/**
    copyright  (C) 2006
    the icecube collaboration
    $Id: I3EHEEventDigest 2006-12-29 syoshida $

    @version $Revision: 1.1 $
    @date $Date: 2006-12-29 15:23:15 +0900
    @author Shigeru Yoshida

    I3EHEEventDigest

    This class digests an EHE MC/real event. Digests are written out
    to standard output so that your analysis prorgram independent of
    the IceTray platform can read out in the down stream :-).
*/

#ifndef I3EHEEVENTDIGEST_H_INCLUDED
#define I3EHEEVENTDIGEST_H_INCLUDED

#include <stdio.h>

#include "icetray/I3TrayHeaders.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/Utility.h"
#include "icetray/I3Units.h"


// dataclasses

// DOM Launch stuff
#include "dataclasses/physics/I3DOMLaunch.h" 
// Portia stuff
#include "recclasses/I3PortiaPulse.h"
#include "recclasses/I3PortiaEvent.h"
//Juliet  stuff
#include "juliet-interface/I3JulietParams.h"
#include "juliet-interface/I3JulietPrimaryParams.h"
#include "juliet-interface/I3JulietUtils.h"
#include "juliet-interface/I3JulietPrimaryParticleSource.h"
// MMCTrack stuff
#include "simclasses/I3MMCTrack.h"


//Firstguess stuff
#include "recclasses/I3OpheliaFirstGuessTrack.h"

class I3EHEEventDigest : public I3ConditionalModule {

 public:
  SET_LOGGER("I3EHEEventDigest");

  // constructor and destructor
  I3EHEEventDigest(const I3Context& ctx);
  virtual ~I3EHEEventDigest();

  void Configure();
  void Physics(I3FramePtr frame);

  /** Distance of in-ice particle start position from the IceCube center - 880m  */
  // This value must be equal to IceCubeCoordinate.elevation
  // in the Juliet. See $I3_SRC/juliet/java_lib/sources/iceCube/uhe/geometry/
  // IceCubeCoordinate.java
  static const double inIceDistance_;

 private:


// default assigment and copy constructors declared private
  
  I3EHEEventDigest();
  I3EHEEventDigest(const I3EHEEventDigest&);
  I3EHEEventDigest& operator=(const I3EHEEventDigest &);

  void DigestJulietParticle(const I3MCTree& mc_tree, const I3JulietParamsTree& params_tree);

  void DigestMMCTrack(const I3MMCTrackList& muonBundle, const I3MCTree& cosmicRayTree);

  void DigestPortiaPulse(I3PortiaEventConstPtr portia_event, 
                        I3PortiaPulseMapConstPtr atwd_pulse,
                         I3PortiaPulseMapConstPtr fadc_pulse,
			 bool baseTimeWindowEvent = true);

  void DigestFirstGuessTrack(I3OpheliaFirstGuessTrackConstPtr fgTrack);

  void DigestRecoTrack(I3ParticleConstPtr i3Track);

  void DigestMillipede(I3VectorI3ParticleConstPtr cascadeSeries);

  void InitializeValuablesInDigest();

  void OutputEventDigest();

  void OutputEventExecutiveSummary();


  unsigned long eventNumber_;
  unsigned long runID_;
  unsigned long eventID_;
  /** 
   * Geometry shift parameter. You can shift (digested) vertex position
   * by these values. For the case when the IceCube geometry was shifted
   * in the MC data production for a partial array such as IC 22 strings
   */
  double shiftX_;
  double shiftY_;
  double shiftZ_;

  ////////////////////////////////////////////////////////////
  /**
   * Whether you digest Juliet Particle - MC Truth Particle
   */
  bool digestJulietParticle_;

  /** Particle spieces defined by JULIeT  */
  int flavor_;
  int doublet_;

  /** Particle direction - MCTruth */
  double nxMC_;
  double nyMC_;
  double nzMC_;

  /** particle energy - MCTruth [GeV]*/
  double energyMC_;

  /** propagation distance from the earth surface [cm]*/
  double distanceFromEarthSurface_;

  /** track length [cm] for weighting the neutrino interaction */
  double trackLength_;
  bool isNeutrino_;
  double enhancementFactor_;

  /** Vertex of the highest energy cascade [cm] */
  double cascadeX_;
  double cascadeY_;
  double cascadeZ_;


  ////////////////////////////////////////////////////////////
  /**
   * Whether you digest MMC Tracks  - MC Truth Particle
   */
  bool digestMMCTrack_;

  /** muon bundle energy sum at Surface [GeV]*/
  double muonEnergyAtSurface_;
  /** Primary Cosmic Ray Energy [GeV] */
  double crEnergy_;
  /** Muon Bundle direction - MCTruth */
  double nxBundle_;
  double nyBundle_;
  double nzBundle_;

  ////////////////////////////////////////////////////////////
  /**
   * Whether you digest Portia Pulse
   */
  bool digestPortiaPulse_;
  bool baseTimeWindow_;

  /** Event-summed NPEs */
  double totalAtwdEstimatedNpe_;
  double totalFadcEstimatedNpe_;
  double totalBestEstimatedNpe_;

  /** Number of DOMs with the Portia pulses */
  int nDOMsWithLaunch_;
  int nDOMsWithATWD_;
  int nDOMsWithFADC_;

  /** DOM timing info */
  double largestNPETime_;
  double earliestTime_;

  /** Millipede results parameters */
  double energyDeposit_; // total energy deposit in the fidutial volume
  I3Position startPosition_;
  I3Position endPosition_;

  ////////////////////////////////////////////////////////////
  /**
   * Whether you digest EHE first guess
   */
  bool digestEHEFirstGuess_;
  bool digestRecoTrack_;

  bool digestMillipede_;

  /** Particle direction - first guess */
  double nxFG_;
  double nyFG_;
  double nzFG_;

  /** Vertex of the Center Of Brightness [cm] */
  double cobX_;
  double cobY_;
  double cobZ_;

  /** Velocity resulted from the first guess line fit [cm/s] */
  double velocity_;

  /** The position of DOMs receiving the largest NPE [cm]*/
  double lnpeDOMX_;
  double lnpeDOMY_;
  double lnpeDOMZ_;

  /** digest executive summary with no MC truth information */
  bool digestExecutiveSummary_;

  std::string eventHeaderName_;
  //std::string inputJulietParticleName_;
  std::string inputI3MCTreeName_;
  std::string inputI3JulietParamsTreeName_;
  std::string inputMMCTrackListName_;
  std::string inputPrimaryCRMCTreeName_;
  std::string inputInIceDOMLaunchName_;
  std::string inputAtwdPulseName_;
  std::string inputFadcPulseName_;
  std::string inputPortiaEventName_;
  std::string inputFirstguessName_;
  std::string inputRecoTrackName_;
  std::string inputMillipedeParticlesName_;

};

#endif
