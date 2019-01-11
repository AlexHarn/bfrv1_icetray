/*
 * class: I3GulliverIPDFFactory
 *
 * Version $Id$
 *
 * Date: Fri Jan 20 21:40:47 CST 2006
 *
 * (c) IceCube Collaboration
 */
#ifndef I3GULLIVER_IPDF_PANDEL_FACTORY_H_INCLUDED
#define I3GULLIVER_IPDF_PANDEL_FACTORY_H_INCLUDED

// standard library
#include <string>

// forward declarations
#include "icetray/IcetrayFwd.h"

// superclasses

#include "icetray/I3ServiceFactory.h"
#include "lilliput/likelihood/I3GulliverIPDFPandel.h"

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

/**
 * @brief This class installs a I3GulliverIPDFPandel.
 *
 * I3GulliverIPDFPandel supports three parameters:
 * - <VAR>"InputReadout"</VAR>
 * - <VAR>"Likelihood"</VAR>
 * - <VAR>"PEProb"</VAR>
 * - <VAR>"IceModel"</VAR>
 * - <VAR>"IceFile"</VAR>
 * - <VAR>"AbsorptionLength"</VAR>
 * - <VAR>"JitterTime"</VAR>
 * - <VAR>"NoiseProbability"</VAR>
 *
 * @todo maybe add methods to retrieve PDF values (instead of event llh)
 *
 * @version $Id$
 * @author boersma
 */
class I3GulliverIPDFPandelFactory : public I3ServiceFactory {
public:

    // Constructors and destructor

    I3GulliverIPDFPandelFactory(const I3Context& context);

    virtual ~I3GulliverIPDFPandelFactory();

    // public member functions

    /**
     * Installed this objects service into the specified services object.
     *
     * @param services the I3Services into which the service should be installed.
     * @return true if the services is successfully installed.
     */
    virtual bool InstallService(I3Context& services);

    /**
     * Configure service prior to installing it. 
     */
    virtual void Configure();

private:

    // private constructors, destructor and assignment

    // stop defaults
    I3GulliverIPDFPandelFactory( const I3GulliverIPDFPandelFactory& rhs);
    I3GulliverIPDFPandelFactory operator= (const I3GulliverIPDFPandelFactory& rhs);

    // options
    std::string likelihood_;   /// SPEAll, SPE1st(default), MPE or PSA
    std::string peprob_;       /// GConvolute(default), UnConvoluted, UPatch
    std::string intLookupFile_;/// OBSOLETE lookup file for GConvolute+MPE
    std::string icefile_;      /// for I3Medium (default: empty)
    int icemodel_;             /// 0,1,2(default),3,4
    double absorption_;        /// override absorption value
    double jitter_;            /// override "jitter" value (usually larger than actual PMT jitter)
    double mpeTimingError_;    /// for MPE: separate time cal error from jitter
    std::string inputreadout_; /// where to get the recopulses
    double noiseProb_;         /// noise prob (noise rate times some time window; should be dimensionless)
    std::string eventTypeString_; /// Muon (default) or {Point|Directional}Cascade
    bool cascade_;             /// true if we should assume cascades
    int intCalcType_;          /// plain, table or numeric for GaussConvolutedPandel

    // misc
    I3PhotonicsServicePtr psp_;            /// pointer to photonics service
    I3EventLogLikelihoodBasePtr eventllh_; /// pointer to singleton service object
    unsigned int ncontext_;                /// counter of the number of installs
    std::string name_;                     /// identifier/label/stepname of this factory in the script

    SET_LOGGER("I3GulliverIPDFPandelFactory");

private:
    template<typename ice>
        I3EventLogLikelihoodBasePtr GetLLHwithHypothesis();
    template<typename ice,typename eventtype>
        I3EventLogLikelihoodBasePtr GetPeProbLLH();
    template<typename spptype, typename eventtype>
        I3EventLogLikelihoodBasePtr GetLLH( boost::shared_ptr<spptype> spp );
    template<typename spptype, typename eventtype>
        I3EventLogLikelihoodBasePtr GetNoisyLLH( boost::shared_ptr<spptype> spp );
};

typedef I3GulliverIPDFPandelFactory I3GulliverIPDFFactory;

#endif /* I3GULLIVER_IPDF_PANDEL_FACTORY_H_INCLUDED */
