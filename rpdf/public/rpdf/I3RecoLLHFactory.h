#ifndef I3RECOLLHFACTORY_H_INCLUDED
#define I3RECOLLHFACTORY_H_INCLUDED

/**
 * @brief Provides a Gulliver likelihood service for muon based reconstructions
 *
 * (c) 2018 the IceCube Collaboration
 *
 * @file I3RecoLLHFactory.h
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include "icetray/I3ServiceFactory.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "rpdf/geometry.h"

/**
 * @class I3RecoLLHFactory
 * A I3Service factory to install an I3RecoLLH into the context
 */
class I3RecoLLHFactory : public I3ServiceFactory {
 public:

  I3RecoLLHFactory(const I3Context& context);
  virtual ~I3RecoLLHFactory();

  /**
   * Installs this service in the context
   */
  virtual bool InstallService(I3Context& services);
  /**
   * Gets the parameters defined by the steering file from the context
   * and initializes the instance of I3RecoLLH
   */
  virtual void Configure();

 private:

  ///The name of the I3FrameObject to read from the frame
  std::string inputreadout_;
  ///The name of the likelihood function to use: SPE1st or MPE
  std::string likelihood_;
  ///The time scale of the DOM jitter to consider when reconstructing
  double jitter_;
  ///The frequency of noise hits to use in the reconstruction
  double noiseProb_;
  ///the name to install the I3RecoLLH instance in the context
  std::string name_;
  ///The name of the Photoelectron probability to use: FastConvoluted or Unconvoluted
  std::string peprob_;
  ///pointer to the instance of the object installed in the context
  I3EventLogLikelihoodBasePtr llh_;
  ///instance of the ice model parameters to use
  rpdf::IceModel ice_model_;

  SET_LOGGER("I3RecoLLHFactory");
};

#endif
