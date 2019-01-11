#include <icetray/I3ConditionalModule.h>
#include <algorithm>
#include <limits>
#include <boost/make_shared.hpp>
#include <boost/math/constants/constants.hpp>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/I3Double.h>
#include <gulliver-bootstrap/BootstrapParams.h>

namespace{
	double differenceAngle(const I3Direction& p1, const I3Direction& p2){
		double cad=cos(p1.GetAzimuth()-p2.GetAzimuth());
		double czd=cos(p1.GetZenith()-p2.GetZenith());
		double czs=cos(p1.GetZenith()+p2.GetZenith());
		double dot=(cad*(czd-czs)+czd+czs)/2;
		if(dot>1.)
			return(0.0);
		if(dot<-1.)
			return(boost::math::constants::pi<double>());
		return(acos(dot));
	}
}

/**
 Computes the average of the various bootstrapped fits, the angular difference 
 between each fit and the average, and the angular difference from the average 
 within which a given fraction of the bootstrapped reconstructions fall. This 
 latter value is intended as an estimate of the angular uncertainty for the 
 reconstruction of the event. FInally, the averaged bootstrapped reconstruction 
 is appended to the collection of bootstrapped reconstructions.
*/
class BootstrapSeedTweak : public I3ConditionalModule{
private:
	std::string bootstrappedRecosName;
	double containmentLevel;
	std::string angularEstimateName;
public:
	BootstrapSeedTweak(const I3Context& context):
	I3ConditionalModule(context),
	containmentLevel(0.5),
	angularEstimateName("BootstrappedAngluarError")
	{
		AddParameter("BootstrappedRecos","Vector of I3Particles resulting from bootstrapped reconstructions",bootstrappedRecosName);
		AddParameter("ContainmentLevel","The fraction of bootstrapped reconstructions which should be contained within the angular error estimate",containmentLevel);
		AddParameter("AngularError","The name for the angular error estimate",angularEstimateName);
		AddOutBox("OutBox");
	}
	
	void Configure(){
		GetParameter("BootstrappedRecos",bootstrappedRecosName);
		GetParameter("ContainmentLevel",containmentLevel);
		GetParameter("AngularError",angularEstimateName);
	}
	
	void Physics(boost::shared_ptr<I3Frame> frame){
		if(!frame->Has(bootstrappedRecosName)){
			PushFrame(frame);
			return;
		}

		// test all bootstrapped fits as candidates and copy the successful ones to the 'recos' vector
		boost::shared_ptr<const I3Vector<I3Particle> > recoCands = frame->Get<boost::shared_ptr<const I3Vector<I3Particle> > >(bootstrappedRecosName);
		boost::shared_ptr<I3Vector<I3Particle> > recos(new I3Vector<I3Particle>());
		for(unsigned int i=0; i<recoCands->size(); i++){
			if((*recoCands)[i].GetFitStatus()==0)
				recos->push_back((*recoCands)[i]);
		}
		if(recoCands->size()!=recos->size()){
			log_warn("Out of %zu bootstrapped fits, only %zu were successful.",recoCands->size(),recos->size());
		}
		if(recos->size()==0){
			log_warn("All bootstrapping fits failed. Cannot estimate angular error.");
			frame->Put(angularEstimateName,boost::make_shared<I3Double>(std::numeric_limits<double>::quiet_NaN()));
			frame->Put(angularEstimateName+"Params",
					   boost::make_shared<BootstrapParams>(BootstrapParams::NoValidFits,recos->size(),recoCands->size()));
			PushFrame(frame);
			return;
		}

		
		I3Position averageDir(0,0,0), averagePos(0,0,0);
		double averageTime=0.0;
		for(unsigned int i=0; i<recos->size(); i++){
			averageDir+=I3Position((*recos)[i].GetDir());
			averagePos+=(*recos)[i].GetPos();
			averageTime+=(*recos)[i].GetTime();
		}
		averageDir/=recos->size();
		averagePos/=recos->size();
		averageTime/=recos->size();
		std::vector<double> differences(recos->size());
		for(unsigned int i=0; i<recos->size(); i++)
			differences[i]=differenceAngle(I3Direction(averageDir),(*recos)[i].GetDir());
		std::sort(differences.begin(),differences.end());
		
		boost::shared_ptr<I3Vector<I3Particle> > extendedRecos(new I3Vector<I3Particle>(*recos));
		extendedRecos->push_back(extendedRecos->back());
		extendedRecos->back().SetPos(averagePos);
		extendedRecos->back().SetDir(I3Direction(averageDir));
		extendedRecos->back().SetTime(averageTime);
		frame->Delete(bootstrappedRecosName);
		frame->Put(bootstrappedRecosName,extendedRecos);
		
		double index=containmentLevel*(differences.size()+1)-1;
		BootstrapParams::ResultStatus status;
		if(index<0.0){
			log_warn_stream("Unable to estimate a " << containmentLevel <<
							" containment radius with " << differences.size() <<
							" reconstructions; rounding up to nearest");
			frame->Put(angularEstimateName,boost::make_shared<I3Double>(differences.front()));
			status=BootstrapParams::Underflow;
		}
		else if(index>=differences.size()-1){
			log_warn_stream("Unable to estimate a " << containmentLevel <<
							" containment radius with " << differences.size() <<
							" reconstructions; rounding down to nearest");
			frame->Put(angularEstimateName,boost::make_shared<I3Double>(differences.back()));
			status=BootstrapParams::Overflow;
		} else {
			double angErr=(differences[ceil(index)]-differences[floor(index)])*(index-floor(index))+differences[floor(index)];
			//std::cout << "Angular error: " << angErr << std::endl;
			frame->Put(angularEstimateName,boost::make_shared<I3Double>(angErr));
			status=BootstrapParams::OK;
		}
		frame->Put(angularEstimateName+"Params",
				   boost::make_shared<BootstrapParams>(status,recos->size(),recoCands->size()));
		
		PushFrame(frame);
	}
};

I3_MODULE(BootstrapSeedTweak);
