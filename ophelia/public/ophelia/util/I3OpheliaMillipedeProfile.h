/**
    copyright  (C) 2006
    the icecube collaboration
    $Id: I3OpheliaMillipedeProfile 2012-3-10 syoshida $

    @version $Revision: 1.1 $
    @date $Date: 2012-3-10 15:23:15 +0900
    @author Shigeru Yoshida

    I3OpheliaMilllipedeProfile

    This class outputs the millipede results stored in
    I3VectorI3Particle to standard out. The output text format
    follows the topdrawer (grafig) style, for graphics.
*/

#ifndef I3OPHELIAMILLIPEDEPROFILE_H_INCLUDED
#define I3OPHELIAMILLIPEDEPROFILE_H_INCLUDED

#include <stdio.h>
#include <vector>

#include "icetray/I3TrayHeaders.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/Utility.h"
#include "icetray/I3Units.h"

// dataclasses
#include "dataclasses/physics/I3Particle.h" 

// DOM Launch stuff
#include "dataclasses/physics/I3DOMLaunch.h" 

class I3OpheliaMillipedeProfile : public I3ConditionalModule {

 public:
  SET_LOGGER("I3OpheliaMillipedeProfile");

  // constructor and destructor
  I3OpheliaMillipedeProfile(const I3Context& ctx);
  virtual ~I3OpheliaMillipedeProfile();

  void Configure();
  void Physics(I3FramePtr frame);

 private:


// default assigment and copy constructors declared private
  
  I3OpheliaMillipedeProfile();
  I3OpheliaMillipedeProfile(const I3OpheliaMillipedeProfile&);
  I3OpheliaMillipedeProfile& operator=(const I3OpheliaMillipedeProfile &);

  /**
   * Take the I3Particle<vector> which describes the series of casacades
   * along the event estimated by millipede. Stored energy loss profile in the internal vectors. 
   */
  void ExtractMillipedeResults(I3VectorI3ParticleConstPtr cascadeSeries);

  /**
   * Outputs the extracted energy loss profile to stdout.
   */
  void OutputProfile();


  /** Millipede results parameters */
  double energyDeposit_; // total energy deposit in the fidutial volume
  std::vector<double> dedx_;
  std::vector<I3Position> vertex_;

  std::string inputMillipedeParticlesName_;

};

#endif
