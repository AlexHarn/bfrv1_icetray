#ifndef I3RECOLLH_H_INCLUDED
#define I3RECOLLH_H_INCLUDED

/**
 * @brief Provides a Gulliver likelihood service for muon based reconstructions
 *
 * (c) 2018 the IceCube Collaboration
 *
 * @file I3RecoLLH.h
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include "dataclasses/geometry/I3Geometry.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "rpdf/pandel.h"
#include "rpdf/geometry.h"

/**
 * @class I3RecoLLH
 * @brief A gulliver likelihood service for reconstructions which use
 *        the Pandel function.
 *
 * It calculates the
 * likelihood of a muon track hypothesis using an analytic description of light
 * propagation through the ice. This analytic description is referred to as the
 * Pandel function. This implementation assumes all ice in the detector has
 * uniform optical properties.
 *
 */

class I3RecoLLH : public I3EventLogLikelihoodBase {
 private:

  /**
   * Structure for caching the information needed to calculate the likelihood.
   * This data is the same regardless of the hypothesis so it is computed once
   * per event and cached so it doesn't have to be calculated for each iteration
   */
  struct I3HitCache
  {
    ///Total number of Photoelectrons observed by the DOM in the event
    double total_npe;
    ///The time of the first pulsse
    double first_pulse_time;
    ///position of the DOM
    I3Position pos;
  };

  ///A descriptive string representing the instance of this class
  std::string name_;
  ///The name of the I3FrameObject to read from the frame
  const std::string inputReadout_;
  ///The name of the likelihood function to use: SPE1st or MPE
  const std::string likelihood_;
  ///The name of the Photoelectron probability to use: FastConvoluted or Unconvoluted
  const std::string peprob_;
  ///The time scale of the DOM jitter to consider when reconstructing
  const double jitter_;
  ///The frequency of noise hits to use in the reconstruction
  const double noise_;

  ///The ice model object to store the optical properties in
  const rpdf::IceModel ice_model_;
  ///Instance of the DOM likelihood function object
  rpdf::DOMLikelihoodFunction dom_likelihood_func_;
  ///Instance of the Photoelectron probability function object
  std::shared_ptr<rpdf::PhotoElectronProbability> pe_prob_;
  ///A pointer to the geometry to store
  I3GeometryConstPtr geoptr_;
  ///a list of the pertinent information for each hit in an event stored in a vector for fast access
  std::vector<I3HitCache> hit_cache_;

 public:
  /**
   * @brief Create a new I3RecoLLH object
   *
   * @param input_readout The name of the I3RecoPulseSeriesMap to read from the frame
   * @param likelihood The name of the DOM likelihood algorithm to use.
   *        Options are "GaussConvoluted" or "UnconvolutedPandel"
   * @param peprob The name of the photoelectron probability calculation to use
   *        Options are "SPE1st" or "MPE"
   * @param jitter the width of the Gaussian which is convoluted with the pandel function
   * @param noise The frequency of noise hits to use in the reconstruction
   * @param ice_model A struct containing the description of optical properties of the ice
   */
  I3RecoLLH( const std::string &input_readout,
             const std::string &likelihood,
     const std::string &peprob,
     const double jitter,const double noise,
       const rpdf::IceModel &ice);
  virtual ~I3RecoLLH();

  /**
   * This is called when the reader gets a new geometry.
   * It saves the geometry for SetEvent() to save the location
   * of each DOM in the hit cache
   *
   * @param geo the Geometry to use to calculate likelihoods
   */
  void SetGeometry( const I3Geometry &geo);

  /**
   * Called when a new event occurs, this reads the new event
   * and the pulse map in the frame.  and calls SetHitCache()
   *
   * @param frame the frame to extract the I3RecoPulseSeriesMap from
   */
  void SetEvent( const I3Frame &frame );

  /**
   * Calculates the hit cache from the geometry and given pulse map
   *
   * @param pulse_map map containing the pulses for calculating the likelihood
   */
  void SetPulseMap(const I3RecoPulseSeriesMap& pulse_map);

  /**
   * Calculate the log likelihood of an event hypothesis
   * for the current event
   *
   * @param event_hypothesis the gulliver event hypothesis (which contains an
   *        I3Particle) with which to calculate the likelihood
   *
   * @returns the likelihood of the hypothesis
   */
  double GetLogLikelihood( const I3EventHypothesis &event_hypothesis);

  /**
   * @returns the multiplicity of the event in question:
   * the number of hit DOMs
   */
  unsigned int GetMultiplicity();

  /**
   * changes the name of this particular instance of I3RecoLLH
   */
  void SetName(std::string name);

  /**
   * @returns a string which describes this particular instance of I3RecoLLH
   */
  const std::string GetName() const;

  SET_LOGGER( "I3RecoLLH" );
};

#endif
