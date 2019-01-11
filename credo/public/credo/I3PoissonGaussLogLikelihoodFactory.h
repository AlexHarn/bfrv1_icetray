/*
 *  @Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#ifndef I3PHOTORECLOGLIKELIHOODFACTORY_H_INCLUDED
#define I3PHOTORECLOGLIKELIHOODFACTORY_H_INCLUDED


// boost, standard library stuff
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceFactory.h"
#include <string>
#include <vector>
#include "credo/I3PoissonGaussLogLikelihood.h"

/**
 * @class I3PoissonGaussLogLikelihoodFactory
 * @brief This class installs a I3PoissonGaussLogLikelihood
 *
 */
class I3PoissonGaussLogLikelihoodFactory : public I3ServiceFactory {
public:

    // Constructors and destructor

    I3PoissonGaussLogLikelihoodFactory(const I3Context& context);

    virtual ~I3PoissonGaussLogLikelihoodFactory();

    // public member functions

    /**
     * Install this objects service into the specified context
     *
     * @param services the context into which the service should be installed.
     * @return true if the services is successfully installed.
     */
    virtual bool InstallService(I3Context& services);

    /**
     * Configure service prior to installing it. 
     */
    virtual void Configure();

   private:

    // private constructors, destructor and assignment
    I3PoissonGaussLogLikelihoodFactory( const I3PoissonGaussLogLikelihoodFactory& rhs);
    I3PoissonGaussLogLikelihoodFactory operator= (const I3PoissonGaussLogLikelihoodFactory& rhs);

    // options - these private variables store the user input and will be passed
    // to the constructor of the llh service
    // the documentation strings in the corresponding AddParameter(..) calls give
    // some more information on these
    std::string inputPulses_ ; // name of the input I3RecoPulseSeriesMap in the frame
    I3PhotonicsServicePtr pdfService_;   // name of the photonics service in the context 
    double noiseRate_;         // rate of noise hits; will be added to charge prediction
    double eventLength_;       // fixed eventlength or toggle 
    double activeVolume_;      /* maximum distance between an OM and the current hypothesis 
                                  for which the photonics-service is queried */
    double gaussianErrorConstant_; // controls which pulses enter the poisson  and which enter the gaussian llh
    //bool skipWeights_;             // ignore amplitudes in llh calculation
    //bool midPulse_;                // whether to query the tables at the leading edge or at the center of a pulse
    bool onlyATWD_;                // toggle if the input pulsemaps contains only pulses from the ATWD
    bool useBaseContribution_;     // calculate the parts of the llh which do not depend on the hypothesis
    bool useEmptyPulses_;          // whether gaps in the pulsemap should be filled with empty pulses
    double minChargeFraction_;     // introduces a lower charge limit as the fraction of the highest charge in the event 
    double saturationLimit_;  // saturation limit for IceCube DOMs
    bool photonicsSaturation_;     // whether the photonics prediction should be restricted to the saturation levels
    bool useIC40Correction_;      // whether amplitude predictions (photonics) should be scaled according to Eike's IC40 parametrization	
    double light_scale_;          // constant scaling factor to the photonics charge prediction

    std::string badDOMListName_;  // name of the bad dom list in the frame
    std::vector<OMKey> badDOMList_;    // a static bad dom list

    // instance data
    unsigned int ncontext_; // the number of contexts where in which we the llhservice was installed
    I3PoissonGaussLogLikelihoodPtr llhservice_; // a pointer to the configured llhservice

    SET_LOGGER("I3PoissonGaussLogLikelihoodFactory");
};

#endif /* I3PHOTORECLOGLIKELIHOODFACTORY_H_INCLUDED */
