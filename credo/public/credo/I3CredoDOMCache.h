/*
 *  @Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#ifndef I3CREDODOMCACHE_H_INCLUDED
#define I3CREDODOMCACHE_H_INCLUDED

#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Position.h"

/**
 * @class I3CredoDOMCache
 * @brief cache DOM-wise LLH intermediate results
 */
class I3CredoDOMCache  {
    public:
        enum OMType {
            UNSPECIFIED = 0,
            ICECUBEDOM = 10,
            AMANDA_ELECTRICAL = 20,
            AMANDA_OPTICAL    = 30,
            DEEPCOREDOM = 40
        };

        /// constructor
        I3CredoDOMCache(const I3Position &pos,
                        double totalnpe, double pnpe, double base, double outofbound, 
                        double fpTime, I3RecoPulseSeriesMap::const_iterator iter, 
                        OMType omtype, bool saturated, double relativedomefficiency):
            position(pos),
            npe(totalnpe),
            nPoissonPE(pnpe),
            baseContribution(base),
            outOfBoundContribution(outofbound),
            firstPulseTime(fpTime),
            type(omtype),
            i_pmap(iter),
            llhcontrib(0),
            expectedAmpliude(0),
            amplitudeCorrection(1),
            saturated(saturated),
            relativedomefficiency(relativedomefficiency)
            {}

        I3CredoDOMCache(){ Reset(); }
        // destructor
        ~I3CredoDOMCache(){}

        // reset all datamembers
        void Reset(); 
         
        // datamembers: 
        I3Position position;           // copy position of the DOM to the cache 
        double npe;                    // total charge in all pulses -> number of photoelectrons
        double nPoissonPE;             // total charge in all pulses that should be treated poissonian
        double baseContribution;       // contribution of this DOM to the llh independent of the hypothesis
        double outOfBoundContribution; // contribution of this DOM if it's outside the active volume
        double firstPulseTime;         // eventually needed for pdf integration
        OMType type;                   // type of this DOM

        // iterator to the pulseseries (to avoid a costly pulsemap_.find(omkey) call)
        // won't get serialized
        I3RecoPulseSeriesMap::const_iterator i_pmap;

        // the following entries are useful for diagnostics and depend on the hypothesis
        double llhcontrib;             // contribution of this DOM to the llh.
        double expectedAmpliude;       // photonics prediction without correction.
        double amplitudeCorrection;    // correction factor returned by GetPhotonicsCorrectionFactor

        bool saturated;                 // saturation detected
        double relativedomefficiency;   // quantumefficiency of the dom

     private:
        friend class icecube::serialization::access;
        template <class Archive> void serialize(Archive& ar, unsigned version);
};




#endif
