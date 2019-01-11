/**
    copyright  (C) 2006
    the icecube collaboration
    $Id: I3EHEEventSelector 2006-12-18 syoshida $

    @version $Revision: 1.1 $
    @date $Date: 2006-12-18 21:23:15 +0900
    @author Shigeru Yoshida

    I3EHEEventSelector

    This class selects an EHE MC/real event satisfying criteria 
    specified by a given set of parameters.
*/

#ifndef I3EHEEVENTSELECTOR_H_INCLUDED
#define I3EHEEVENTSELECTOR_H_INCLUDED

#include <string>
#include "icetray/I3TrayHeaders.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/Utility.h"
#include "icetray/I3Units.h"

// dataclasses

// DOM Launch stuff
#include "dataclasses/physics/I3DOMLaunch.h" 
// Portia stuff
#include "recclasses/I3PortiaEvent.h"
#include "recclasses/I3PortiaPulse.h"
//Firstguess stuff
#include "recclasses/I3OpheliaFirstGuessTrack.h"
// EHECriteia stuff
#include "ophelia/util/EHECriteria.h"

class I3EHEEventSelector : public I3ConditionalModule {

 public:
  SET_LOGGER("I3EHEEventSelector");

  // constructor and destructor
  I3EHEEventSelector(const I3Context& ctx);
  virtual ~I3EHEEventSelector();

  void Configure();
  void Physics(I3FramePtr frame);

 private:


// default assigment and copy constructors declared private
  
  I3EHEEventSelector();
  I3EHEEventSelector(const I3EHEEventSelector&);
  I3EHEEventSelector& operator=(const I3EHEEventSelector &);

  // This pickes p-frame of "in-ice split"
  bool PassesCriteriaOnEventHeader(I3EventHeaderConstPtr eventHeader);

  bool PassesCriteriaOnJulietParticle(const I3MCTree& mc_tree);

  bool PassesCriteriaOnInIceDOMLaunch(I3DOMLaunchSeriesMapConstPtr inIceLaunch);

  bool PassesCriteriaOnPortiaPulse(I3PortiaEventConstPtr portia_event, 
		   bool baseTimeWindowEvent);

  bool PassesCriteriaOnFirstGuessTrack(I3OpheliaFirstGuessTrackConstPtr fgTrack);

  bool PassesEHECriteria(I3PortiaEventConstPtr portia_event, 
                 I3PortiaPulseMapConstPtr atwd_pulse,
                 I3PortiaPulseMapConstPtr fadc_pulse,
	 I3OpheliaFirstGuessTrackConstPtr fgTrack);



  ////////////////////////////////////////////////////////////
  /**
   * Whether you set criteria on I3EventHeader.
   */
  bool setCriteriaOnEventHeader_;

  /**
   * Whether you set criteria on Juliet Particle - MC Truth Particle
   */
  bool setCriteriaOnJulietParticle_;

  /**
   * Range of particle energy
   */
  double energyMin_, energyMax_;

  /**
   * Particle Type
   */
  int    particleType_;

  /**
   * Particle Geometry range
   */
  double cosZenithMin_, cosZenithMax_;
  double azimuthMin_, azimuthMax_;

  /**
   * Distance between the brighest cascade vertex and i3center
   */
  double distanceOftheHighestCascade_;



  ////////////////////////////////////////////////////////////
  /**
   * Whether you set criteria on InIce DOM launch
   */
  bool setCriteriaOnInIceDOMLaunch_;

  /**
   * range of minimum NDOMs with DOM launches
   */
  int nDOMLaunchMin_;

  ////////////////////////////////////////////////////////////
  /**
   * Whether you set criteria on Portia Pulse
   */
  bool setCriteriaOnPortiaPulse_;

  /**
   * Whether The base time window cleaning for Portia pulse criteria
   */
  bool baseTimeWindow_;

  /**
   * range of NPEs 
   */
  double lowestNPEs_, highestNPEs_;

  /**
   * range of NDOMs (picks up either the one with FADCs or ATWDs or both)
   */
  int nDOMmin_;



  ////////////////////////////////////////////////////////////
  /**
   * Whether you set criteria on EHE first guess
   */
  bool setCriteriaOnEHEFirstGuess_;

  /**
   * range of geometry
   */
  double fg_cosZenithMin_, fg_cosZenithMax_;
  double fg_azimuthMin_, fg_azimuthMax_;
  double fg_cogzMin_, fg_cogzMax_;
  double fg_bdomzMin_, fg_bdomzMax_;


  /**
   * Distance between the COB vertex and i3center
   */
  double distanceOfCOB_;

  /**
   * Line fit velocity cut
   */
  double velocityMin_;


  ////////////////////////////////////////////////////////////
  /**
   * Whether you set criteria on EHE criteria
   */
  bool setEHECriteria_;
  EHECriteria *eheCriteria_;

  /** Whether you use the result after the basetime window cleaning*/
  bool baseTimeWindowEvent_;


  //std::string inputJulietParticleName_;
  std::string subEventStreamName_;
  std::string inputI3MCTreeName_;
  std::string inputInIceDOMLaunchName_;
  std::string inputAtwdPulseName_;
  std::string inputFadcPulseName_;
  std::string inputPortiaEventName_;
  std::string inputFirstguessName_;
  std::string eheCriteriaFileName_;

};

#endif
