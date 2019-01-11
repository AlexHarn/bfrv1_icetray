/*
 *  @Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
 *  based on Sean Grullon's I3PhotorecLogLikelihood
*/

#ifndef I3POISSONGAUSSLOGLIKELIHOOD_H_INCLUDED
#define I3POISSONGAUSSLOGLIKELIHOOD_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <set>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3Units.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "photonics-service/I3PhotonicsService.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "credo/I3CredoEventStatistics.h"
#include "credo/I3CredoDOMCache.h"

class I3Position;
class I3Particle;
class I3Direction;

/**
 * @class I3PoissonGaussLogLikelihood
 * @brief service class to compute the event likelihood using recopulses and photonics-service
 *
 */

class I3PoissonGaussLogLikelihood  : public I3EventLogLikelihoodBase {
    public:
        /**
         * constructors (see datamember docs for info about the args)
         */
        I3PoissonGaussLogLikelihood() : I3EventLogLikelihoodBase() {};

        I3PoissonGaussLogLikelihood(const std::string& serviceName,
                                    I3PhotonicsServicePtr pdf,
                                    const std::string& pulsesrc,
                                    double noiserate,
                                    double eventlength,
                                    double activevolume,
                                    double gaussianErrorConstant,
                                    bool onlyATWD,
                                    bool useBaseContrib,
                                    bool useEmptyPulses,
                                    double light_scale);

        /**
         * destructor
         */
        virtual ~I3PoissonGaussLogLikelihood();

        /**
         * setup saturation behaviour
         * for the limits a value of <= 0 will turn off saturation detection
         */
        void SetSaturation(double minChargeFraction, 
                           double saturationLimit, 
                           bool   photonicsSaturation) {
            minChargeFraction_   = minChargeFraction;
            photonicsSaturation_ = photonicsSaturation;
            saturationLimit_           = saturationLimit;
        }
        
        /**
         * setup bad DOMs
         */
        void SetBadDOMs(const std::string& badDOMListName, const std::vector<OMKey>& badDOMList) {
            badDOMListName_ = badDOMListName;
            badDOMList_.clear();
            // remove duplicate entries in the badDOM list
            std::set<OMKey> uniqueBadDoms(badDOMList.begin(), badDOMList.end());
            badDOMList_.insert(badDOMList_.begin(), uniqueBadDoms.begin(), uniqueBadDoms.end());
        }


        /**
         * Get the geometry from the frame
         */
        void SetGeometry( const I3Geometry &f) {};

        /**
         * Get everything from the frame that is necessary for the reconstruction
         */
        void SetEvent( const I3Frame &f );

        /**
         * returns number of summands in the log likelihood
         */
        unsigned int GetMultiplicity() { return multiplicity_; }

        /**
         * core method: computes the log likelihood for a give hypothesis
         */
        double GetLogLikelihood( const I3EventHypothesis &p );

        /**
         * name, used for log messages and such
         */
        const std::string GetName() const { return serviceName_; }

        /**
         * set different PDF (useful for unit tests)
         */
        void SetPDF( I3PhotonicsServicePtr newpdf ){ pdf_ = newpdf; }

        /**
         * get PDF (useful for unit tests)
         */
        I3PhotonicsServicePtr GetPDF() const { return pdf_; }

        const std::vector<OMKey>& GetStaticBadDOMList() const { return badDOMList_; }
        


        /// cache variables for each DOM, compute only once per event
        // public to allow diagnostics
        std::map<OMKey,I3CredoDOMCache> cacheMap_;

        I3CredoEventStatistics stats_;
    private:

        /**
         * PDF: point to a photorec object (actually, just anything with
         * the I3PDFBase interface)
         */
        I3PhotonicsServicePtr pdf_;

        /**
         * the name of this likelihood service
         */
        std::string serviceName_;

        /**
         * name of I3RecoPulseSeriesMap to use
         */
        std::string inputPulses_;

        /**
         * name of list of bad DOMs in the frame
         */
        std::string badDOMListName_;


        /**
         * own bad DOM list
         */
        std::vector<OMKey> badDOMList_;

        // configuration parameters
        double noiseRate_;       // noise rate (replacing noise prob), should be OM dependent
        bool constEventLength_;  // switch between constant or data-dependent event length
        double activeVolume_;    // max distance DOM - particle, if it's further then it's noise
        bool onlyATWD_;          // flag indicates that pulsesmap_ contains pulses only from the ATWD
        bool useBaseContribution_;     // calculate contributions to the likelihood that don't depend on the hypothesis
        bool useEmptyPulses_;      // fill gaps in the pulsemaps with pulses of zero charge
        double gaussianErrorConstant_; // determines the error of gaussian pdf
        double eventLength_;       // duration of event (can be either a constant, or derived from pulsemap)
        bool useIC40Correction_; // flag use of Eike's IC40 correction of photonics amplitudes
        double light_scale_;      // constant scaling factor to the photonics charge prediction

        double minChargeFraction_; // fraction of the charge of DOM with highest amplitude that a DOM must have to be used in the llh calc. 
        double saturationLimit_;            // hightest total charge of IceCube DOMs
        bool photonicsSaturation_; // consider saturation in the photonics prediction 

        // private variables for intermediary results
        double noiseChargeEvent_;  // expected noise charge per nohit or ATWD+FADC DOM is noiseRate_*eventLength_ (TODO: LC?)
        double noiseChargeATWD_;   // expected noise charge per ATWD DOM is noiseRate_*128*3.5ns 
        double maxPerpDistSqr_;    // square of activeVolume_
        double npeThreshold_;      // pulses with charges over this threshold are treated with the gaussian term
        double gaussianBaseTerm_;  // part of gaussian normalisation 


        I3RecoPulseSeriesMapPtr pulsesmap_;    // pulses

        unsigned int eventCounter_;  // count SetEvent() calls
        unsigned int multiplicity_; /// number of pulses + number of unhit DOMs in current event


        // to be filled by GetDirectTimeDistance
        double directTime_;       /// direct time from a track to an OM
        double distance_;         /// distance from an OM to a track

        // disable some unwanted constructors/operators
        I3PoissonGaussLogLikelihood(const I3PoissonGaussLogLikelihood&);
        I3PoissonGaussLogLikelihood operator= (const I3PoissonGaussLogLikelihood& rhs);

        /**
         * get expected "direct" arrival time and distance to a track
         * fills the private members directTime_ and distance_
         */
        void GetDirectTimeDistance(const I3Particle& p, const I3Position& pos, double pdsqr);

        SET_LOGGER( "I3PoissonGaussLogLikelihood" );
};



I3_POINTER_TYPEDEFS( I3PoissonGaussLogLikelihood );

#endif /* I3POISSONGAUSSLOGLIKELIHOOD_H_INCLUDED */
