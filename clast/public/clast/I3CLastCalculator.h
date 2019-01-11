/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id: I3CLastCalculator.h 18552 2006-04-19 15:45:25Z grullon $
 *
 * @file I3CLastCalculator.h
 * @version $Revision: 1.3 $
 * @date $Date: 2006-04-19 10:45:25 -0500 (Wed, 19 Apr 2006) $
 * @author Pat Toale <toale@phys.psu.edu> (based on TensorOfInertia code by Sean Grullon <grullon@icecube.wisc.edu>)
 */
#ifndef I3CLASTCALCULATOR_H_INCLUDED
#define I3CLASTCALCULATOR_H_INCLUDED

#include <gsl/gsl_eigen.h>

#include "dataclasses/I3Matrix.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Position.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Constants.h"

/**
 *
 * @class I3CLastCalculator
 * @brief Core calculation class for I3CLastModule module
 *
 * This class contains a series of functions to calculate the tensor
 * of inertia and related quantities.  With this, you can calculate
 * the inertia tensor of the hits, diagonalize the inertia tensor,
 * and correct the direction of the longest eigenvector.
 *
 * In addition, a simplified version of the cfirst algorithm is
 * included so that the start time of the cascade can be estimated
 * by looking at the times of hits that are close to the light
 * cone assuming that the vertex is at the center of gravity.  This
 * implementation differs from the cfirst one in that it will return
 * the time of the first hit in the event for an events that do not
 * have a sufficient number of hits on/near the light cone.  In
 * this way, a seed will always have a start time -- something that
 * is required by subsequent likelihood fits.
 * 
 * And, finally, this code contains routines that parameterize the
 * estimate of the cascade energy from the nchannels for IceCube
 * AMANDA.  These parameterization was obtained from cascade MC
 * data.
 *
 */

class I3CLastCalculator {
  
 public:
  
  I3CLastCalculator(double ampWeight, int ampOpt, 
                    double directHitRadius, double directHitWindow, int directHitThreshold,
                    double e0, double e1, double e2, double e3, double e4, double e5,
                    double a0, double a1, double a2, double a3, double a4, double a5)
  {

    ampWeight_       = ampWeight;
    ampOpt_          = ampOpt;
    rMax_            = directHitRadius;
    directHitWindow_ = directHitWindow;
    threshold_       = directHitThreshold;

    e0_ = e0;                               // Q->Energy parameterization coefficients for I3
    e1_ = e1;
    e2_ = e2;
    e3_ = e3;
    e4_ = e4;
    e5_ = e5;

    a0_ = e0;                               // Q->Energy parameterization coefficients for AM
    a1_ = e1;
    a2_ = e2;
    a3_ = e3;
    a4_ = e4;
    a5_ = e5;

    return;

  } // end of constructor function for I3CLastCalculator object

  /* 
   *Calculates the Tensor of Inertia
   */
  I3Matrix CalculateTOI(I3RecoPulseSeriesMapConstPtr pulse_series, const I3OMGeoMap& om_geo, const I3Position& cogPosition)
  {

    //General ToI formulation:
    //Diagonals: I_ii = sum_k[m_k*(r^2 - r_i^2)] / sum(m_k)
    //Off-diagonals: I_ij = sum_k[m_k*(-r_i*-r_j)] / sum(m_k)
    //Here our m_k is our per-dom charge
    //and our r_i r_j terms are between 
    //the dom and center of gravity (cog)

    //Initialise I3Matrix to 3x3 of 0.
    I3Matrix inertiatensor(3,3,0.);
    double ampsum=0.0;
    double charge=0.0;

    //Iterate over pulse map for each ToI contribution
    I3RecoPulseSeriesMap::const_iterator iter;
    iter=pulse_series->begin();
    while (iter != pulse_series->end()) {

      charge = 0.;
      const std::vector<I3RecoPulse> pulsevect=iter->second;
        
      if (pulsevect.empty()) {
        iter++;
        continue;
      }
        
      for (unsigned ii=0; ii < pulsevect.size(); ii++) {
        I3RecoPulse pulse = pulsevect[ii];
        if (std::isnan(pulse.GetCharge()) || std::isinf(pulse.GetCharge()) ) {
          log_warn("Got a nan or an inf pulse charge.  Setting charge to 0 instead.  Something could be screwy with a DOM calibration!!");
          pulse.SetCharge(0.);
        }

        //with ampOpt=0, count any charge less than 2 as exactly 1
        //all other cases just count charge as-is
        double amp_tmp = 0.;
        if (ampOpt_ == 0) {
          amp_tmp = (pulse.GetCharge() >= 2.0) ? pulse.GetCharge() : 1;
        } else if (ampOpt_ == 1) {
          amp_tmp = pulse.GetCharge();
        } else {
          log_warn("Incorrect AmplitudeOption: setting amplitude to 0!" );
        }   

        //accumulate total charge
        ampsum += amp_tmp;

        //option to weight pulse included, is "mass" put into the ToI
        charge += pow(amp_tmp,ampWeight_);
      }

      //get current omkey and check exists
      const OMKey om = iter->first;
      I3OMGeoMap::const_iterator iter2 = om_geo.find(om);
      if (iter2 == om_geo.end()) {
        log_warn("in CalculateTOI(), missing DOM in OMGeoMap ... skipping OM (%2.2d,%2.2d) in pulseseries",
                 om.GetString(),om.GetOM());
        continue;
      }
      
      //get dom position
      const I3Position ompos = (iter2->second).position;
      
      //compute distance between cog and current dom
      double r[3];
      r[0] = ompos.GetX()-cogPosition.GetX();
      r[1] = ompos.GetY()-cogPosition.GetY();
      r[2] = ompos.GetZ()-cogPosition.GetZ();
      double rsquared = r[0]*r[0] + r[1]*r[1] + r[2]*r[2];
      double delta = 1.0;
      
      // the following nested for loop adds the various values to the
      // appropriate component of the inertia tensor.
      
      for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
          //diagonals have rsquared, off-diag not
          delta = (i == j) ? 1.0 : 0.0; 
          //compute inertia tensor element
          inertiatensor(i,j)+=charge*(delta*rsquared-r[i]*r[j]);
        }
      }
    
      iter++;
    }

    //avoid div0 error
    if (ampsum == 0) {
      log_warn("total charge is 0 over pulsemap, avoiding div0 error");
      ampsum = 1;
    }

    //divide out tensor by total charge as we 
    //have weighted by individual charge in ToI above
    for (int i=0; i < 3; i++) {
      for (int j=0; j < 3; j++) {
        inertiatensor(i,j)/=ampsum;
      }
    }

    return inertiatensor;

  } // end of member function I3CLastCalculator::CalculateTOI()
  
  /*
   * Corrects the direction of a reconstructed particle.
   */
  int CorrectDirection(I3RecoPulseSeriesMapConstPtr pulse_series, const I3OMGeoMap& om_geo,
                       const I3Position& cog, const std::vector<double>& minevect)
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
      
    //Iterate over all doms
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
      
        //get corresponding geometry for OMKey
        const OMKey om = iter->first;
        I3OMGeoMap::const_iterator iter2 = om_geo.find(om);
        if (iter2 == om_geo.end()) {
           log_warn("in CorrectDirection(), missing DOM in OMGeoMap ... skipping OM (%2.2d,%2.2d) in pulseseries",
                    om.GetString(),om.GetOM());
           continue;
        }

        //get OM Position and calculate difference vector to COG
        const I3Position three = (iter2->second).position;
      
        double omx = three.GetX() - cog.GetX();
        double omy = three.GetY() - cog.GetY();
        double omz = three.GetZ() - cog.GetZ();
        double omnorm = sqrt(omx*omx + omy*omy + omz*omz);
        if(omnorm==0) {
          omnorm=1;
        }
      
        // get the projection of the OM position along the longest axis.
        // first take length of eigenvector
        double minevect_mag = sqrt(minevect[0]*minevect[0]+minevect[1]*minevect[1]+minevect[2]*minevect[2]);
        // then take dot product while making sure that eigenvector is unit length
        double projom=(minevect[0]*omx+minevect[1]*omy+minevect[2]*omz)/(minevect_mag);
      
        // get (potentially modified) pulse charge as in ToI calculation
        if (std::isnan(pulse.GetCharge() > 0)) {
          pulse.SetCharge(0);
        }
        double amp_tmp = 0.;
        if (ampOpt_ == 0){
          amp_tmp=(pulse.GetCharge() >= 2.0) ? pulse.GetCharge() : 1;
        } else if (ampOpt_ == 1){
          amp_tmp=pulse.GetCharge();
        } else {
          log_warn("Incorrect AmplitudeOption: setting amplitude of pulse to 0!" );
        }

        // get (potentially weighted) charge squared for covariance
        double ampsquared=pow(amp_tmp,ampWeight_)*pow(amp_tmp,ampWeight_);
      
        //update  total covariance variables
        ampsum+=ampsquared;
        projomsum+=projom*ampsquared;
        timesum+=time*ampsquared;
        projomsumsquared+=projom*projom*ampsquared;
        projomtimesum+=time*projom*ampsquared;
      }
      iter++;
        
    }
      
    if(ampsum==0.0){
      log_warn("Variable ampsum is 0!");
      ampsum=1;
    }
      
    double cov = (ampsum*projomsumsquared-projomsum*projomsum);
      
    // get the covariance of the projected OM length along the longest axis with the sum of the OM hit times,
    // weighted by the square of the amplitude. 
    if (cov != 0.) {
      cov=(ampsum*projomtimesum-projomsum*timesum)/cov;
    } else {
      cov=(ampsum*projomtimesum-projomsum*timesum)/ampsum;
    }
      
    int corr;

    //return the sign of the covariance
    if (cov < 0)
      corr=-1;
    else 
      corr=1;
  
    return corr;
      
  } // end of member function I3CLastCalculator::CorrectDirection()
  
  /*
   *diagonalizes the Tensor of Inertia
   */
  std::vector<double> diagonalizeTOI(I3Matrix& inertiatensor, double& mineval, double& eval2, double& eval3, double& evalratio)
  {
    i3_assert((inertiatensor.size1()==3) && (inertiatensor.size2()==3));

    gsl_eigen_symmv_workspace *workspace = gsl_eigen_symmv_alloc(3);
    //vector of eigenvalues
    gsl_vector *eval = gsl_vector_alloc(3);
    //matrix of eigenvectors
    gsl_matrix *evec = gsl_matrix_alloc(3, 3);
    //will hold ToI
    gsl_matrix *A = gsl_matrix_alloc(3, 3);

    //set A to ToI
    for (std::size_t i=0;i<3;++i)
      for (std::size_t j=0;j<3;++j)
      {
        gsl_matrix_set(A, i, j, inertiatensor(i,j));
      }

    //get eigenvecs/vals
    gsl_eigen_symmv(A, eval, evec, workspace);
    //sort the eigenvalues and eigenvectors simultaneously (ascending)
    gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_ASC);
    gsl_matrix_free(A);

    //expose eigenvalues outside function
    mineval = gsl_vector_get(eval, 0);
    eval2   = gsl_vector_get(eval, 1);
    eval3   = gsl_vector_get(eval, 2);

    // parameter evalratio provides a useful cut for the sphericity of an event
    const double evalsum=mineval+eval2+eval3;
    if (evalsum!=0.0){
      evalratio=mineval/evalsum;
    } //expose evalratio

    std::vector<double> minevect(3, NAN);
    //get long-axis eigenvector (corresponding to smallest eigenvalue)
    {
      gsl_vector_view evec_i = gsl_matrix_column(evec, 0);
      for (std::size_t i=0;i<3;++i)
      {
        minevect[i] = gsl_vector_get(&evec_i.vector, i);
      }
    }

    //free memory
    gsl_matrix_free(evec);
    gsl_vector_free(eval);
    gsl_eigen_symmv_free(workspace);

    return minevect;

  } // end of member function I3CLastCalculator::diagonalizeTOI()

  double CalculateTime(I3RecoPulseSeriesMapConstPtr pulseMap, const I3OMGeoMap& omMap, const I3Position& cogPosition)
  {

    //initialise variables
    //start with large time values if we're testing for earlier ones
    double earliestVertexTime = 1e10;
    int nDirect = 0;

    int maxNDirect = -1;
    double timeOfMaxNDirect = 1e10;
    double earliestThresholdTime = 1e10;

    bool haveThresholdTime = false;
    bool haveDirectTime    = false;

    // loop over all DOMs with reco pulses
    I3RecoPulseSeriesMap::const_iterator mapIter1;
    for (mapIter1 = pulseMap->begin(); mapIter1 != pulseMap->end(); mapIter1++) {
      const OMKey& omKey1 = mapIter1->first;
      const I3RecoPulseSeries& pulseSeries1 = mapIter1->second;

      // get the time of the first pulse
      if (pulseSeries1.size() == 0) continue;
      double t1 = pulseSeries1[0].GetTime();

      // get the distance from the DOM to the COG
      I3OMGeoMap::const_iterator omGeo1 = omMap.find(omKey1);
      if (omGeo1 == omMap.end()) {
        log_warn("in CalculateTime(), missing DOM in OMGeoMap ... skipping OM (%2.2d,%2.2d) in pulseseries",
                 omKey1.GetString(),omKey1.GetOM());
        continue;
      }
      double r1 = (cogPosition-(omGeo1->second).position).Magnitude();

      // this is our trial vertex time
      double vertexTime = t1 - r1/I3Constants::c_ice;

      // keep track of the earliest (this is the fallback solution)
      if (vertexTime < earliestVertexTime) earliestVertexTime = vertexTime;
      
      // require that this DOM is within some radius of the COG
      if (r1 > rMax_) continue;

      // now loop over all other DOMs
      I3RecoPulseSeriesMap::const_iterator mapIter2;
      for (mapIter2 = pulseMap->begin(); mapIter2 != pulseMap->end(); mapIter2++) {
        // make sure not the same DOM
        if (mapIter2 == mapIter1) continue;

        const OMKey& omKey2 = mapIter2->first;
        const I3RecoPulseSeries& pulseSeries2 = mapIter2->second;

        // get the time of the first pulse
        if (pulseSeries2.size() == 0) continue;
        double t2 = pulseSeries2[0].GetTime();

        // get the distance from the DOM to the COG
        I3OMGeoMap::const_iterator omGeo2 = omMap.find(omKey2);
        if (omGeo2 == omMap.end()) {
          log_warn("Missing DOM in OMGeoMap");
          continue;
        }
        double r2 = (cogPosition-(omGeo2->second).position).Magnitude();

        // require that this DOM is within some radius of the COG
        if (r2 > rMax_) continue;

        // project the time to the COG
        double projectedTime = t2 - r2/I3Constants::c_ice;

        // calculate the time difference
        double deltaT = projectedTime - vertexTime;

        // ensure that the time difference is positive (basically to enforce a time ordering)
        if (deltaT < 0) continue;

        // now see if it counts as a direct hit
        if (deltaT < directHitWindow_) nDirect++;

      }

      // allow for three possible times: 
      // - the preferred time (earliestThresholdTime) for the earliest 
      // overall vertex time with nDirect hits above threshold
      // - the time (timeOfMaxNDirect) with the most direct hits following
      // - a final fallback time (earliestVertexTime) for the earliest overall vertex time
      // regardless of nearby direct hits

      // update timeOfMaxNDirect
      if (nDirect > maxNDirect) {
        maxNDirect = nDirect;
        timeOfMaxNDirect = vertexTime;
        haveDirectTime = true;
      }

      // update earliestThresholdTime
      if (nDirect >= threshold_) {
        if (vertexTime < earliestThresholdTime) {
          earliestThresholdTime = vertexTime;
        }
        haveThresholdTime = true;
      }

    }

    // finally figure out which time to return
    if (haveThresholdTime)
      return earliestThresholdTime;
    else if (haveDirectTime)
      return timeOfMaxNDirect;
    else
      return earliestVertexTime;

  } // end of member function I3CLastCalculator::CalculateTime()

  double CalculateEnergy_From_I3Hits(I3RecoPulseSeriesMapConstPtr pulseMap)
  {

    double npe = 0;
    I3RecoPulseSeriesMap::const_iterator mapIter;
    for (mapIter = pulseMap->begin(); mapIter != pulseMap->end(); mapIter++) {
      const I3RecoPulseSeries& pulseSeries = mapIter->second;
      I3RecoPulseSeries::const_iterator seriesIter;
      for (seriesIter = pulseSeries.begin(); seriesIter != pulseSeries.end(); seriesIter++) {
        const I3RecoPulse& pulse = *seriesIter;
        if (std::isnan(pulse.GetCharge()) > 0 || std::isinf(pulse.GetCharge()) > 0) {
          log_warn("in CalculateEnergy(), got a nan or an inf pulse charge for OM (%2.2d,%2.2d) - skipping it",
                   (mapIter->first).GetString(),(mapIter->first).GetOM());
          continue;
        }
        npe += pulse.GetCharge();
      }
    }

    double logQ = log10(npe);
    double logE = e0_ + logQ*(e1_ + logQ*(e2_ + logQ*(e3_ + logQ*(e4_ + logQ*e5_))));

    return pow(10, logE);

  } // end of member function I3CLastCalculator::CalculateEnergy_From_I3Hits()

  double CalculateEnergy_From_AMHits(I3RecoPulseSeriesMapConstPtr pulseMap)
  {

    double npe = 0;
    I3RecoPulseSeriesMap::const_iterator mapIter;
    for (mapIter = pulseMap->begin(); mapIter != pulseMap->end(); mapIter++) {
      const I3RecoPulseSeries& pulseSeries = mapIter->second;
      I3RecoPulseSeries::const_iterator seriesIter;
      for (seriesIter = pulseSeries.begin(); seriesIter != pulseSeries.end(); seriesIter++) {
        const I3RecoPulse& pulse = *seriesIter;
        npe += pulse.GetCharge();
      }
    }

    double logQ = log10(npe);
    double logE = a0_ + logQ*(a1_ + logQ*(a2_ + logQ*(a3_ + logQ*(a4_ + logQ*a5_))));

    return pow(10, logE);

  } // end of member function I3CLastCalculator::CalculateEnergy_From_AMHits()

 private:

  double ampWeight_;
  int    ampOpt_;

  double rMax_;
  double directHitWindow_;
  double threshold_;

  double e0_;
  double e1_;
  double e2_;
  double e3_;
  double e4_;
  double e5_;

  double a0_;
  double a1_;
  double a2_;
  double a3_;
  double a4_;
  double a5_;

}; // end of class I3CLastCalculator class

#endif  // I3CLASTCALCULATOR_H_INCLUDED
