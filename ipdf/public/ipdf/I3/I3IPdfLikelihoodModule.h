#ifndef IPDF_I3IPDFLIKELIHOODMODULE_H
#define IPDF_I3IPDFLIKELIHOODMODULE_H

/**
 *
 * @brief Definition of the I3IPdfLikelihoodModule class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3IPdfLikelihoodModule.h
 * @version $Revision: 1.3 $
 * @date $Date$
 * @author Simon Robbins
 */

#include "ipdf/AllOMsLikelihood.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/Pandel/UPatchPEP.h"
#include "ipdf/Pandel/GConvolutePEP.h"
#include "ipdf/I3/TMinuitMinimizer.h"
#include "ipdf/I3/I3DetectorConfiguration.h"

#include "icetray/I3Module.h"
#include "icetray/I3Logging.h"
#include "dataclasses/physics/I3Particle.h"
#include <TObject.h>

namespace IPDF { class I3DetectorResponse; class InfiniteMuon; }

  /**
   * @brief Example implementation of muon likelihood reconstruction 
   * within the Icecube / I3 framework and using IPDF.
   *
   * Much of framework side implementation inspired by muon-llh-reco.
   *
   * @todo fix getting seed (make the seed name a "parameter")
   *
   * @todo make it work with both RecoHits and RecoPulses (easy)
   */
class I3IPdfLikelihoodModule : public I3Module {
public:
  typedef IPDF::Pandel::GConvolutePEP<IPDF::Pandel::H2> 
      PandelPDF;

  /// @brief Standard ctor to comply with icetray framework
  I3IPdfLikelihoodModule(const I3Context& ctx);
  
  ~I3IPdfLikelihoodModule();

  /// @brief Framework required method
  void Configure();

  /// @brief Framework required method: the "main routine"
  void Physics(I3FramePtr frame);

  /// @brief Framework method: for updating to new geometry
  void Geometry(I3FramePtr frame);

  SET_LOGGER( "ipdf" );

protected:

  /// @brief Internal implementation: Perform reco depending on I3 response type
  template<class Readout>
  I3ParticlePtr DoReco(const Readout&, const I3ParticleConstPtr&);

  /// @brief Internal implementation: ensure track is in principle fitable
  bool checkTrack(const I3ParticleConstPtr& track);

  /// @brief Internal implementation: get seed track from previous module
  /// 
  /// If we fail to find the seed track and we are doing simulation, then 
  /// the seed is taken from the MC truth.
  I3ParticleConstPtr retrieveSeed(const I3FramePtr&) const;

  /// @brief Internal implementation: do the likelihood fitting
  IPDF::InfiniteMuon maxLikelihood(
		      const IPDF::I3DetectorResponse&,
		      I3ParticleConstPtr);

private:  
  /// @brief default, assignment, and copy constructor declared private
  I3IPdfLikelihoodModule();
  /// @brief default, assignment, and copy constructor declared private
  I3IPdfLikelihoodModule(const I3IPdfLikelihoodModule&);
  /// @brief default, assignment, and copy constructor declared private
  I3IPdfLikelihoodModule& operator=(const I3IPdfLikelihoodModule&);

  // Source of the input data
  std::string inputDataReadout_;
  std::string inputSeedName_;
  bool printConfig_;

  boost::shared_ptr<IPDF::I3DetectorConfiguration> dgeom_;

  // The likelihood:
  IPDF::AllOMsLikelihood<IPDF::Likelihood::SPE1st<PandelPDF> > likelihood_;

  // The interface to TMinuit
  IPDF::TMinuitMinimizer<5> minimizer_;

};


#endif //IPDF_I3IPDFLIKELIHOODMODULE_H
