/*
 *  @Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#ifndef I3CREDOFUNCTIONS_H_INCLUDED
#define I3CREDOFUNCTIONS_H_INCLUDED

#include "dataclasses/physics/I3RecoPulse.h"
#include "photonics-service/I3PhotonicsService.h"
#include "credo/I3CredoDOMCache.h"

class I3Position;
class I3Direction;
class I3Particle;

namespace I3Credo {
    /**
     * fills a I3RecoPulseSeriesMap with pulses of zero charge
     * that fill the gaps between reconstructed pulses.
     * these "empty" pulses provide penalty contributions to the llh that
     * may be helpful to reject wrong hypotheses. Without them
     * the likelihood would be only evaluated at the times
     * where reconstructed pulses exists.
     * Be aware that this can drastically increase the number
     * of summands in the llh calculation and will affect the
     * runtime. 
     */
    I3RecoPulseSeriesMapPtr GetPulseMapWithGaps(I3RecoPulseSeriesMapConstPtr inputpulses,
                                                bool onlyATWD);

    /**
     * helper function to GetPulseMapWithGaps
     * pulseseries: the pulseseries to fill
     * lastPulse: points to the last pulse in pulseseries
     * tnext: fill times [lastpulse->GetTime(),tnext]
     * minoffset: smallest timedifference that should be filled
     * emptypulsewidth : the width of the empty pulses
     */
    void FillOneGap(I3RecoPulseSeries& pulseseries,              
                    I3RecoPulseSeries::const_iterator& lastPulse,
                    double tnext, double minoffset, double emptypulsewidth);



    /*/ 
     * function used to correct the photonics prediction
     */ 
    double GetPhotonicsCorrectionFactor(I3CredoDOMCache::OMType type, double prediction, 
                                        double distance, bool atwdonly);
    
    // this method was copied from photorec-llh. Someday it should be moved
    // to phys-services.
    double DistSquared( const I3Position& vertex, const I3Position& pos );
    

    /**
     * for pulses with large widths it may be necessary to integrate the delay
     * time pdf over a given time interval 
     * these methods are currently not used in the likelihood service
     */

    double GetPDFIntegralEstimateGaussianQuadrature( double min_time_residual, double max_time_residual,
                                                     I3PhotonicsServicePtr pdf,
                                                     const PhotonicsSource& source); 
    
    double GetPDFIntegralEstimatePandel(double distance, double min_time_residual, double max_time_residual);
}

#endif 
