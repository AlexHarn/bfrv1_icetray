#include <gsl/gsl_eigen.h>

#include <dataclasses/geometry/I3Geometry.h>
#include <icetray/OMKey.h>
#include <tensor-of-inertia/I3TensorOfInertiaCalculator.hpp>


I3Matrix I3TensorOfInertiaCalculator::CalculateTOI(I3RecoPulseSeriesMapConstPtr pulse_series, 
                                                   const I3OMGeoMap& om_geo, 
                                                   const I3Position& cogPosition)
{
  I3Matrix inertiatensor(3,3,0.);
  double ampsum=0.0;
  
  I3RecoPulseSeriesMap::const_iterator iter;
  iter=pulse_series->begin();
  while(iter != pulse_series->end()) {
    
    const std::vector<I3RecoPulse> pulsevect=iter->second;
    
    if(pulsevect.empty()==true) {
      iter++;
      continue;
    }
    
    for (unsigned ii=0; ii < pulsevect.size(); ii++) 
      {
        I3RecoPulse pulse = pulsevect[ii];
        if (std::isnan(pulse.GetCharge()) > 0 || std::isinf(pulse.GetCharge()) > 0) {
          log_warn("Got a nan or an inf pulse charge.  Setting charge to 0 instead.  "
                   "Something could be screwy with a DOM calibration!!");
          pulse.SetCharge(0.);
        }
        
        double amp_tmp = pulse.GetCharge();
        ampsum+=amp_tmp;
        double amp=pow(amp_tmp,ampWeight_);
        const OMKey om = iter->first;
        I3OMGeoMap::const_iterator iter2 = om_geo.find(om);
        assert(iter2 != om_geo.end());
        const I3Position ompos = (iter2->second).position;
        
        double r[3];
        r[0] = ompos.GetX()-cogPosition.GetX();
        r[1] = ompos.GetY()-cogPosition.GetY();
        r[2] = ompos.GetZ()-cogPosition.GetZ();
        double rsquared=r[0]*r[0]+r[1]*r[1]+r[2]*r[2];
        double delta=1.0;
	
        // the following nested for loop adds the various values to the
        // appropriate component of the inertia tensor.
	
	for (int i=0; i < 3; i++)
	  {
	    for (int j=0; j < 3; j++)
	      {
		delta = ( i == j) ? 1.0 : 0.0;
		inertiatensor(i,j)+=amp*(delta*rsquared-r[i]*r[j]);
	      }
	  }
      }
    iter++;
  }
  if(ampsum==0) {
    ampsum=1;
  }
  
  for (int i=0; i < 3; i++)
    {
      for (int j=0; j < 3; j++)
        {
          inertiatensor(i,j)/=ampsum;
        }
    }
  return inertiatensor;
}

/*
 * Corrects the direction of a reconstructed particle.
 */

int I3TensorOfInertiaCalculator::CorrectDirection(I3RecoPulseSeriesMapConstPtr pulse_series, 
                                                  const I3OMGeoMap& om_geo,                       
                                                  const I3Position& cog, 
                                                  const std::vector<double>& minevect)
{
  // Because of the inherit ambiguity in the direction of the track defined
  // by the eigenvector, we need to figure out the right direction.
  // This is accomplished by picking the direction in which the average 
  // OM hit time is latest.  I calculate the covariance of the
  // OM hit times with the projection of the OM position along the longest
  // axis.  This covariance is weighted by the square of the Charge.
  // If the covariance is negative, it means we have the wrong 
  // direction.  
  
  // variables to hold covariance information
  double ampsum=0.0;
  double projomsum=0.0;
  double timesum=0.0;
  double projomsumsquared=0.0;
  double projomtimesum=0.0;
  
  I3RecoPulseSeriesMap::const_iterator iter;
  iter=pulse_series->begin();
  
  while(iter != pulse_series->end()) {
    
    const std::vector<I3RecoPulse> pulsevect=iter->second;
    
    if(pulsevect.empty()==true) {
      iter++;
      continue;
    }
    
    for (unsigned ii=0; ii < pulsevect.size(); ii++) {
      
      I3RecoPulse pulse = pulsevect[ii];
      
      double time = pulse.GetTime();
      
      log_debug("Got time %f in OM %i on string %i.", time, 
		iter->first.GetOM(), iter->first.GetString());
      
      const OMKey om = iter->first;
      I3OMGeoMap::const_iterator iter2 = om_geo.find(om);
      assert(iter2 != om_geo.end());
      const I3Position three = (iter2->second).position;
      
      double omx = three.GetX() - cog.GetX();
      double omy = three.GetY() - cog.GetY();
      double omz = three.GetZ() - cog.GetZ();
      double omnorm = sqrt(omx*omx + omy*omy + omz*omz);
      if(omnorm==0) {
	omnorm=1;
      }
      
      // get the direction cosine between OM position and axis, 
      // and then the projection of the OM position
      // along the longest axis.
      double minevect_mag = sqrt(minevect[0]*minevect[0]+minevect[1]*minevect[1]+minevect[2]*minevect[2]);
      double dircos=(minevect[0]*omx+minevect[1]*omy+minevect[2]*omz)/(omnorm*minevect_mag);
      double projom=dircos*omnorm;
      
      if (std::isnan(pulse.GetCharge() > 0)) {
	pulse.SetCharge(0);
      }
      double amp_tmp = pulse.GetCharge();      
      double ampsquared=pow(amp_tmp,ampWeight_)*pow(amp_tmp,ampWeight_);
      
      ampsum+=ampsquared;
      projomsum+=projom*ampsquared;
      timesum+=time*ampsquared;
      projomsumsquared+=projom*projom*ampsquared;
      projomtimesum+=time*projom*ampsquared;
    }
    iter++;
    
  }
  
  if(ampsum==0.0){
    ampsum=1;
  }
  
  double cov = (ampsum*projomsumsquared-projomsum*projomsum);
  
  // get the covariance of the projected OM length along the longest axis with 
  // the sum of the OM hit times, weighted by the square of the amplitude. 
  if (cov != 0.)
    {
      cov=(ampsum*projomtimesum-projomsum*timesum)/cov;
    }
  else 
    {
      cov=(ampsum*projomtimesum-projomsum*timesum)/ampsum;
    }
  
  int corr;
  
  if (cov < 0)
    corr=-1;
  else 
    corr=1;
  
  return corr;
  
}

/*
 *diagonalizes the Tensor of Inertia
 */

eval_tuple_t I3TensorOfInertiaCalculator::DiagonalizeTOI(I3Matrix& inertiatensor, 
                                                         double& mineval, 
                                                         double& eval2, 
                                                         double& eval3, 
                                                         double& evalratio)    
{
  i3_assert((inertiatensor.size1()==3) && (inertiatensor.size2()==3));
  
  gsl_eigen_symmv_workspace *workspace = gsl_eigen_symmv_alloc(3);
  gsl_vector *eval = gsl_vector_alloc(3);
  gsl_matrix *evec = gsl_matrix_alloc(3, 3);
  gsl_matrix *A = gsl_matrix_alloc(3, 3);
  
  for (std::size_t i=0;i<3;++i)
    for (std::size_t j=0;j<3;++j)
      {
        gsl_matrix_set(A, i, j, inertiatensor(i,j));
      }
  
  gsl_eigen_symmv(A, eval, evec, workspace);
  // sort the eigenvalues and eigenvectors simultaneously (ascending)
  gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_ASC);
  gsl_matrix_free(A);
  
  mineval = gsl_vector_get(eval, 0);
  eval2   = gsl_vector_get(eval, 1);
  eval3   = gsl_vector_get(eval, 2);
  
  // parameter evalratio provides a useful cut for the sphericity of an event
  double evalsum=mineval+eval2+eval3;
  if (evalsum!=0.0){
    evalratio=mineval/evalsum;
  }
  
  std::vector<double> minevect(3, NAN);
  {
    gsl_vector_view evec_i = gsl_matrix_column(evec, 0);
    for (std::size_t i=0;i<3;++i)
      minevect[i] = gsl_vector_get(&evec_i.vector, i);
  }
  
  std::vector<double> eval2evect(3, NAN);
  {
    gsl_vector_view evec_i = gsl_matrix_column(evec, 1);
    for (std::size_t i=0;i<3;++i)
      eval2evect[i] = gsl_vector_get(&evec_i.vector, i);
  }
  
  std::vector<double> eval3evect(3, NAN);
  {
    gsl_vector_view evec_i = gsl_matrix_column(evec, 2);
    for (std::size_t i=0;i<3;++i)
      eval3evect[i] = gsl_vector_get(&evec_i.vector, i);
  }
  
  gsl_matrix_free(evec);
  gsl_vector_free(eval);
  gsl_eigen_symmv_free(workspace);
  
  return eval_tuple_t(minevect,eval2evect,eval3evect);
}
