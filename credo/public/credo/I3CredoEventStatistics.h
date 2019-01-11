/*
 *  @Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#ifndef I3CREDOEVENTSTATISTICS_H_INCLUDED
#define I3CREDOEVENTSTATISTICS_H_INCLUDED

#include "icetray/IcetrayFwd.h"
#include "icetray/serialization.h"

/**
 * @class
 * @brief I3CredoEventStatistics holds a number of variables and counters that
 *        describe the event.
 */

class I3CredoEventStatistics {
    public:
        I3CredoEventStatistics(); 
        virtual ~I3CredoEventStatistics();

        void Reset();

        // nchannel counter
        unsigned int nCh_hit;        // size of pulsemap
        unsigned int nCh_selected;   // hit DOMs that enter the llh calculation
        unsigned int nCh_minball;    // all DOMs that enter the llh calculation
        
        // npe counter
        double npe_all;                       // in all DOMs: total number of npe
        double npe_selected_all;              // in all selected DOMs: total number of npe
        double npe_selected_poisson;          // in all selected DOMs: npe to be treated as poissonian
        double npe_max;                       // in all DOMs: highest charge of all DOMs

        // npulses counter
        unsigned int nPulses_all_good;          // in all DOMs : number of usable pulses
        unsigned int nPulses_all_bad;           // in all DOMs : number of problematic pulses (nans, etc.)
        unsigned int nPulses_selected_poisson;  // in all selected DOMs: number of poisson pulses
        unsigned int nPulses_selected_gaussian; // in all selected DOMs: number of gaussian pulses
        
        // counter for skipped DOMs
        unsigned int skipped_saturated; // skipped because charge too high
        unsigned int skipped_mincharge; // skipped because charge too low
        unsigned int skipped_noStatus;  // skipped because no entry in DetectorStatus map
        unsigned int skipped_noHV;      // skipped because no high voltage (turned off)
        unsigned int skipped_noCalibration; // skipped because no entry in Calibration map

        // other counters
        unsigned int failed_photorec_calls; // counts how often photonics service returns -1
                                            // additive for several calls to GetLogLikelihood

        double start_time, end_time;    // first and last DOM with pulses

     private:
        friend class icecube::serialization::access;
        template <class Archive> void serialize(Archive& ar, unsigned version);
};

I3_POINTER_TYPEDEFS( I3CredoEventStatistics );


#endif 
