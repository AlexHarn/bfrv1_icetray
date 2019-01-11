/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include "icetray/I3Units.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"

#include <vector>
#include <cfloat>
#include "gsl/gsl_cdf.h"
#include "gsl/gsl_randist.h"

#include "credo/I3CredoFunctions.h"

double I3Credo::GetPhotonicsCorrectionFactor(I3CredoDOMCache::OMType type, 
                                             double ampprediction, double distance, 
                                             bool atwdonly) {

    // these parametrizations are obtained from a study that tries to match for IC40
    // the sum of all pulse charges to the amplitude prediction given by photonics
    // see https://docushare.icecube.wisc.edu/dsweb/Get/Document-50066/111-Slides-middell-credo_madison09.pdf
    if ((type == I3CredoDOMCache::ICECUBEDOM) || (type == I3CredoDOMCache::DEEPCOREDOM)) {
        if (atwdonly) {
            if ( std::isnan(distance) || (distance < 0))
                return 1.;
            else
                // dist dependent atwd correction
                return 200*gsl_ran_gaussian_pdf(distance, 120); // 0.75 * 0.95*gauss(dist,120)
        }else{        
          if ( std::isnan(ampprediction) || (ampprediction <= 0)) { 
            // parametrization is a function of log(ampprediction). avoid nans
            return 1.0;
          }else {          
            double logexpected = log10(ampprediction);
            double fp(0);
            if ( (0 < logexpected ) && ( logexpected <=  2.5) )
              fp = 0.2;
            else if ( (2.5 < logexpected ) && ( logexpected <  3.25) )
              fp = .2 + .2*(logexpected-2.5)/.75;
            else if ( logexpected >= 3.25 )
              fp = 0.4;
            
            return pow(10, logexpected - fp) / ampprediction;
          }
        }
    }
    return 0.; 
}

/*****************************************************************************/

I3RecoPulseSeriesMapPtr I3Credo::GetPulseMapWithGaps(I3RecoPulseSeriesMapConstPtr inputpulses,
                                                     bool onlyATWD) {

    I3RecoPulseSeriesMapPtr newpulsemap = I3RecoPulseSeriesMapPtr(new I3RecoPulseSeriesMap());

    I3RecoPulseSeriesMap::const_iterator i_pmap;
    std::vector<I3RecoPulse>::const_iterator currentPulse, lastPulse;
    
    for (i_pmap = inputpulses->begin(); i_pmap != inputpulses->end(); ++i_pmap) {
        std::vector<I3RecoPulse> newpulseseries = std::vector<I3RecoPulse>();

        bool firstpulse = true;
        double minoffset = 10; // ns
        double emptypulsewidth = 100; // ns


        for (currentPulse = i_pmap->second.begin(); currentPulse != i_pmap->second.end(); ++currentPulse) {
            if (firstpulse) {
                lastPulse = currentPulse;
                newpulseseries.push_back(*currentPulse);
                firstpulse = false;
            }
            else {
                FillOneGap(newpulseseries, lastPulse, currentPulse->GetTime(), minoffset, emptypulsewidth);
                newpulseseries.push_back(*currentPulse);
                lastPulse = currentPulse;
            }
        }
        (*newpulsemap)[i_pmap->first] = newpulseseries; 
    }
    return newpulsemap;
}

/******************************************************************************/


void I3Credo::FillOneGap(I3RecoPulseSeries& pulseseries,              
                         I3RecoPulseSeries::const_iterator& lastPulse,
                         double tnext, double minoffset, double emptypulsewidth) {        

    double timediff = tnext - lastPulse->GetTime() - lastPulse->GetWidth();
    // check if this gap is big enough to fill it (> minoffset)
    if ( timediff > minoffset) {
        // if so, fill it up to tnext with pulses which are not
        // wider than emptypulsewidth
        do {
            I3RecoPulse emptypulse = I3RecoPulse();
            emptypulse.SetTime(lastPulse->GetTime() + lastPulse->GetWidth());
            if (timediff > emptypulsewidth) {
                emptypulse.SetWidth(emptypulsewidth);
                timediff -= emptypulsewidth;
            }
            else {
                emptypulse.SetWidth(timediff);
                timediff = 0;
            }
            emptypulse.SetCharge(0.);
            pulseseries.push_back(emptypulse);
            lastPulse = --(pulseseries.end()); 
        } 
        while (timediff > 0);
    }
}

/******************************************************************************/

double I3Credo::GetPDFIntegralEstimateGaussianQuadrature(double min_time_residual, 
                                                         double max_time_residual,
                                                         I3PhotonicsServicePtr pdf,
                                                         const PhotonicsSource& source) {


    // assume that the photonics service and the source are set up correctly
    // from the calling function
     
    // perform a gaussian quadrature integration - copied from Numerical Recipes
    // tests suggest that this yields a good estimate for an integral over the pdf
    const double x[] = {0.1488743389816312, 0.4333953941292472, 0.6794095682990244,
                        0.8650633666889845, 0.9739065285171717};
    const double w[] = {0.2955242247147529, 0.2692667193099963, 0.2190863625159821,
                        0.1494513491505806, 0.06667134430868819};
    
    double expected_amplitude; // to be filled py pdf call but not further used
    double xm,xr,dx,s; 
    double prob_left=0; 
    double prob_right=0;

    xm = 0.5*(min_time_residual+max_time_residual);
    xr = 0.5*(max_time_residual-min_time_residual);
    s  = 0;
    for (int i=0; i < 5; i++) {
        dx = xr*x[i];
        
	    pdf->GetPhotorecInfo(expected_amplitude,prob_left,xm-dx,source);
	    pdf->GetPhotorecInfo(expected_amplitude,prob_right,xm+dx,source);

        if ( !(std::isfinite(prob_left) && (prob_left > 0)) )
            prob_left = 0;
        if ( !(std::isfinite(prob_right) && (prob_right > 0)) )
            prob_right = 0;

        s += w[i]*(prob_left + prob_right);
    }
    return s*xr;
}

/******************************************************************************/

double I3Credo::GetPDFIntegralEstimatePandel(double distance,
                                             double min_time_residual,
                                             double max_time_residual) {

    // integrate the pandel function = gamma distribution
    // FIXME: move the b calculation to a member variable
    double a = distance / 47*I3Units::m;
    double b = 1. / ( 1. / 450*I3Units::ns + I3Constants::c / I3Constants::n_ice_group / 98*I3Units::m);
    
    return gsl_cdf_gamma_P(max_time_residual, a, b) - gsl_cdf_gamma_P(min_time_residual, a, b);
}

/******************************************************************************/


// this methods was copied from photorec-llh. At some point it should be moved
// to phys-services.
double I3Credo::DistSquared( const I3Position& vertex, const I3Position& pos ){
    double dx = pos.GetX() - vertex.GetX();
    double dy = pos.GetY() - vertex.GetY();
    double dz = pos.GetZ() - vertex.GetZ();
    double dsqr = dx * dx + dy * dy + dz * dz;
    return dsqr;
}
