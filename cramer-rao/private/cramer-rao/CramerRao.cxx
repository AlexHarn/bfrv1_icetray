#include "cramer-rao/CramerRao.h"
#include "recclasses/CramerRaoParams.h"
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3Double.h>
#include <icetray/I3Frame.h>
#include <fstream>

#include <gsl/gsl_linalg.h>

#include <vector>

// This macro is used to export the module name to the framework
I3_MODULE(CramerRao);

CramerRao::CramerRao(const I3Context& context) : 
  I3ConditionalModule(context),
  inputHits_("Hits"),
  inputTrack_("Track"),
  outputResult_("CramerRao")
{
  // specify that this module has one outbox named 'OutBox'
  AddOutBox("OutBox");

  AddParameter("InputResponse",
	       "Name of the hit or pulse series map that we will take in",
	       inputHits_);

  AddParameter("InputTrack",
	       "Name of the track, the cramer-rao calculation is based on",
	       inputTrack_);

  AddParameter("OutputResult",
	       "Name of the result in the frame",
	       outputResult_);

  pathToTable_=getenv("I3_SRC");
  pathToTable_=pathToTable_.append("/cramer-rao/resources/data/");
  AddParameter("PathToTable",
               "set the path to the digamma tables",
               pathToTable_);

  allHits_=true;//default
  AddParameter("AllHits",
	       "Shall I use all Hits (allHits_=true) or only first hit per DOM (allHits_=false)?",
	       allHits_);

  doubleOutput_=false;//default
  AddParameter("DoubleOutput",
	       "Shall I output the most effective Value (cramer_rao_theta) as I3Double, so it can be booked in flat-ntuples?",
	       doubleOutput_);

  z_dependent_scatter_=true; //default
  AddParameter("z_dependent_scatter",
               "Shall I use one fixed value for the in ice scatter coefficient (false) or a table with z-dependence (better, true)",
               z_dependent_scatter_);

}

void CramerRao::Configure()
{
  GetParameter("InputResponse",
	       inputHits_);

  GetParameter("InputTrack",
	       inputTrack_);

  GetParameter("OutputResult",
	       outputResult_);

  GetParameter("AllHits",
	       allHits_);

  GetParameter("z_dependent_scatter",
               z_dependent_scatter_);

  GetParameter("PathToTable",
	       pathToTable_);

  GetParameter("DoubleOutput",
	       doubleOutput_);

  log_trace("Will calculate CramerRao based on the %s hit series",
	    inputHits_.c_str());
  log_trace("Will output a result named %s",
	    outputResult_.c_str());

  /*------read the digamma function (T(d))-------*/
  std::ifstream digamma;
  std::string digammafile(pathToTable_);
  if (z_dependent_scatter_) digammafile.append("digamma_conv_0-200");
   else  digammafile.append("digamma-table");
  digamma.open (digammafile.c_str());
  if (!digamma.is_open()) 
    log_fatal("Can't find DigammaFile");

  std::string line;
  if (z_dependent_scatter_){ //read z-dependent digamma function
   for(int i=0;i<400;i++){
    for(int j=0;j<122;j++){    
     std::getline (digamma,line,'\n');
     std::stringstream(line) >> gamma_value_[i][j];
    } 
   }
  } else //read z-fix digamma function
  {
   for(int i=0;i<400;i++){
    std::getline (digamma,line);
    std::stringstream(line) >> gamma_value_[i][0];    
   }
  }
  digamma.close();  

  

}

struct throw_gsl_errors {
	throw_gsl_errors()
	{
		old_handler_ = gsl_set_error_handler(&handler);
	}
	~throw_gsl_errors()
	{
		gsl_set_error_handler(old_handler_);
	}
	static void handler(const char * reason, const char * file, int line, int gsl_errno)
	{
		std::ostringstream ss;
		ss << "GSL error ("<<file<<":"<<line<<"): " << reason;
		throw std::runtime_error(ss.str().c_str());
	}
	gsl_error_handler_t *old_handler_;
};

CramerRaoParams CramerRao::GetParams(const I3Geometry& geometry, const I3RecoPulseSeriesMap& hits, const I3Particle& track)
{  

  CramerRaoParams result;

  if (hits.size() <=5 )
    {
      log_info("Insuffient Hits");
      result.status=CramerRaoParams::InsufficientHits;
      return result;
    }

  std::vector<double> data(16, 0.); // Fisher information matrix
  
  // trackparameters
  double x0=track.GetX();
  double y0=track.GetY();
  double z0=track.GetZ();
  double Theta=track.GetZenith();
  double Phi=track.GetAzimuth();
  if(std::isnan(x0)||std::isnan(y0)||std::isnan(z0)||std::isnan(Theta)||std::isnan(Phi)){
    log_info("Track is empty\n");
    result.status=CramerRaoParams::MissingInput;
    return result;
  }
 
  double cT=cos(Theta),sT=sin(Theta),tT=tan(Theta),cP=cos(Phi),sP=sin(Phi);
  double OM_X,OM_Y,OM_Z;  //DOM position
  double OM_String=0;
  bool IsOneString=true;
  double dsquare;//distance square
  double deriv[5];//(first derivatives of distanceSquare. 0->derivative wrt Theta;1->wrt Phi;2->wrt x;3->wrt y;4->wrt z)

  
  //loop all hits --------------------------------------------------------------------------------
  for(I3RecoPulseSeriesMap::const_iterator iter = hits.begin() ; 
      iter != hits.end() ; 
      iter++)
    {

      I3OMGeoMap::const_iterator foundGeometry = 
	geometry.omgeo.find(iter->first);
      if(foundGeometry == geometry.omgeo.end())
	{
	  log_fatal("OMKey (%d,%d) is not located in the geometry",
		    iter->first.GetString(),
		    iter->first.GetOM());
	}

      OM_X=foundGeometry->second.position.GetX();
      OM_Y=foundGeometry->second.position.GetY();
      OM_Z=foundGeometry->second.position.GetZ();
      if(OM_String!=0&&OM_String!=foundGeometry->first.GetString()){
	IsOneString=false;

      }
      OM_String=foundGeometry->first.GetString();

      //calculate square distance between DOM and Track:
      dsquare=pow((OM_X-x0+z0*cP*tT)*sP*sT-(OM_Y-y0+z0*sP*tT)*cP*sT,2)+pow((OM_Y-y0+z0*sP*tT)*cT-(OM_Z)*sP*sT,2)+pow((OM_Z)*cP*sT-(OM_X-x0+z0*cP*tT)*cT,2);
      double distance=sqrt(dsquare);


      //calculate derivatives                                                                                                             
      //theta:                                                                                                                            
      deriv[0]=2* (cT* pow((y0-OM_Y)* cP+(-x0+OM_X)* sP,2)* sT+((-z0+OM_Z)* cP* cT+(-x0+OM_X)* sT)* ((x0-OM_X)* cT+(-z0+OM_Z)* cP* sT)+((z0-OM_Z)* cT* sP+(y0-OM_Y)* sT)* ((-y0+OM_Y)* cT+(z0-OM_Z)* sP* sT));
      //phi:                                                                                                                              
      deriv[1]=2* ((y0-OM_Y)* cP+(-x0+OM_X)* sP)* sT* ((-z0+OM_Z)* cT+((-x0+OM_X)* cP+(-y0+OM_Y)* sP)* sT);
      //x:                                                                                                                                
      deriv[2]=2* ((x0-OM_X)* pow(cT,2)+(-z0+OM_Z)* cP* cT* sT+sP* ((-y0+OM_Y)* cP+(x0-OM_X)* sP)* pow(sT,2));
      //y:                                                                                                                                
      deriv[3]=2* ((y0-OM_Y)* pow(cT,2)+(-z0+OM_Z)* cT* sP* sT+cP* ((y0-OM_Y)* cP+(-x0+OM_X)* sP)* pow(sT,2));


      //distance-index for the digamma table
      int index=static_cast<int>(2*distance-0.5);//the table has a value for every 0.5m, starting with 0.5m(index=0)

      //depth-index for the digamma table
      int d_index=static_cast<int>(round((1948.07-1345-OM_Z)/10));//table has a value for every 10m, from 1345m to 2555m (measured from surface (1948.07))
      if (d_index<0) d_index=0; //if DOM curiously is higher than 1345m we use the value for 1345m
      if (d_index>=122) d_index=121; // if DOM curiously is deeper than 2555m we use the value for 2555m
      if (!z_dependent_scatter_) d_index=0; //if we do not use z dependent scatter value
      
      double T;

      if (index<400&&index>=0){
	if(index==0)
	  index=1;//there is no gamma_value_[index-1] for index==0
	T=gamma_value_[index][d_index]+(index/2-distance)*(gamma_value_[index-1][d_index]-gamma_value_[index][d_index])/0.5;//linear interpolation
      }
      else if(index<0)//this shouldn't happen
	{
	  log_info("This is strange: distance<0 \n");
	  result.status=CramerRaoParams::OtherProblems;
	  return result;
	}
      else
	T=0.033/distance;//approximation for large distances
	//	T=0;
	//analytical convoluted including distance infos:
	//	T=0.033/sqrt(dsquare)+0.03227/(sqrt(dsquare)-33.3)+0.008667/(sqrt(dsquare)-66.6);
      
      //calculate total charge in per DOM
      double charge(0);
      for (I3RecoPulseSeriesMap::value_type::second_type::const_iterator pulse_iter = iter->second.begin(); pulse_iter!=iter->second.end(); pulse_iter++)
	{
	  charge+=pulse_iter->GetCharge();
	}

      for(int m=0;m<4;m++){
	for(int k=0;k<4;k++){
	  if(allHits_== true )
	    data[k+4*m]+=1/(4*dsquare)* deriv[k] * deriv[m]*T*charge;
	  else
	    data[k+4*m]+=1/(4*dsquare)* deriv[k] * deriv[m]*T;
	}
      }
      
    } //Loop over hits finished --------------------------------------------------------------------

  throw_gsl_errors thrower;
  //Invert Matrix and export cramer rao values
  try {
    gsl_matrix_view A = gsl_matrix_view_array(&(data[0]), 4, 4);

    // invert A (and thus the array "data") in place
    // NB: we use smart pointers here to safely free gsl objects when exceptions are thrown
    boost::shared_ptr<gsl_matrix> Ainv(gsl_matrix_alloc(4, 4), gsl_matrix_free);
    boost::shared_ptr<gsl_permutation> p(gsl_permutation_alloc(4), gsl_permutation_free);
    int signum;
    gsl_linalg_LU_decomp(&A.matrix, p.get(), &signum);
    gsl_linalg_LU_invert(&A.matrix, p.get(), Ainv.get());
    gsl_matrix_memcpy(&A.matrix, Ainv.get()); // replace data in A by Ainv

    result.cramer_rao_theta = sqrt(data[0]);
    result.cramer_rao_phi = sqrt(data[5]);
    result.variance_theta = data[0];
    result.variance_phi = data[5];
    result.variance_x = data[10];
    result.variance_y = data[15];
    result.covariance_theta_phi = data[1];
    result.covariance_theta_x = data[2];
    result.covariance_theta_y = data[3];
    result.covariance_phi_x = data[6];
    result.covariance_phi_y = data[7];
    result.covariance_x_y = data[11];

    result.status=CramerRaoParams::OK;
  }
  catch (std::runtime_error &err){
    log_info_stream("Matrix not invertible ("<<err.what()<<")");
    result.status=CramerRaoParams::SingularMatrix;
  }
  if(IsOneString){
    log_info("OneStringEvent, no reasonable azimuthal resolution");
    result.status=CramerRaoParams::OneStringEvent;
  }
  return result;
}

void CramerRao::Physics(I3FramePtr frame)
{
  // find pulses
  I3RecoPulseSeriesMapConstPtr pulses = frame->Get<I3RecoPulseSeriesMapConstPtr>(inputHits_);
  // finding the geometry. 
  const I3Geometry& geometry = frame->Get<I3Geometry>();
  // finding the track.
  I3ParticleConstPtr track = frame->Get<I3ParticleConstPtr>(inputTrack_);
  

  CramerRaoParamsPtr result(new CramerRaoParams());

  if (!track)
    {
      log_info("Can't find I3Particle \"%s\" in Frame",inputTrack_.c_str());
      result->status=CramerRaoParams::MissingInput;
    }
  else if (pulses)
    {
      *result=GetParams(geometry,*pulses,*track);
    }
  else
    {
      log_info("\"%s\" Does not exist or cast to I3RecoPulseSeriesMap",inputHits_.c_str());
      result->status=CramerRaoParams::MissingInput;
    }

  // putting it in the frame
  frame->Put(outputResult_ + "Params",result);

  if(doubleOutput_){
    I3DoublePtr crzenPtr(new I3Double());
    I3DoublePtr craziPtr(new I3Double());
    crzenPtr->value = result->cramer_rao_theta;
    craziPtr->value = result->cramer_rao_phi;
    frame->Put(outputResult_ + "_cr_zenith",crzenPtr);
    frame->Put(outputResult_ + "_cr_azimuth",craziPtr);
  }
    
  
  // giving the frame back to the framework
  PushFrame(frame,"OutBox");
}
