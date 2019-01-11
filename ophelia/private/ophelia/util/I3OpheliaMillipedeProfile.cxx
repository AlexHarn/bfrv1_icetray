
// This class
#include "ophelia/util/I3OpheliaMillipedeProfile.h"

#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/I3Constants.h"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

I3_MODULE(I3OpheliaMillipedeProfile);

// constructor and destructor

//===================================================================
//* constructor -----------------------------------------------------
I3OpheliaMillipedeProfile::I3OpheliaMillipedeProfile(const I3Context& ctx) : 
  I3ConditionalModule(ctx),
  inputMillipedeParticlesName_("MillipedeAmpsPortiaOphelia")
{


  AddParameter("inMillipedeParticlesName"," ", inputMillipedeParticlesName_);
  AddOutBox("OutBox");

}

//===================================================================
//* destructor -----------------------------------------------------
I3OpheliaMillipedeProfile::~I3OpheliaMillipedeProfile()
{

  dedx_.clear();
  vertex_.clear();
}


//===================================================================
//* configure -----------------------------------------------------
void I3OpheliaMillipedeProfile::Configure(){
  log_info("Configuring the EHE event selector");

  GetParameter("inMillipedeParticlesName", inputMillipedeParticlesName_);

}


//===================================================================
//* physics -----------------------------------------------------
void I3OpheliaMillipedeProfile::Physics(I3FramePtr frame){
  log_info("Entering I3OpheliaMillipedeProfile::Physics()");

  // Take the Millipede particles (I3VectorI3Particle) from the frame
  I3VectorI3ParticleConstPtr cascadeSeries = 
    frame->Get<I3VectorI3ParticleConstPtr>(inputMillipedeParticlesName_);
  if(cascadeSeries) ExtractMillipedeResults(cascadeSeries);


  // Output energy loss profile
  OutputProfile();

  PushFrame(frame,"OutBox");

}//End Physics()


//===================================================================
// Digest the millipede particles stored in I3VectorI3Particle
//===================================================================
void I3OpheliaMillipedeProfile::ExtractMillipedeResults(I3VectorI3ParticleConstPtr cascadeSeries){

  log_info(" -- Entering ExtractMillipedeResults --");

  if(!cascadeSeries){
    log_error("  This event does not have an I3Particle Vector");
    return;
  }

  // initialization
  energyDeposit_ = 0.0;
  dedx_.clear();
  vertex_.clear();

  I3VectorI3Particle::const_iterator secondary = cascadeSeries->begin();
  for (secondary = cascadeSeries->begin(); secondary != cascadeSeries->end(); secondary++) {
    double energy = (*secondary).GetEnergy();
    if(energy>0.0){ // energy is deposited here!
      energyDeposit_ += energy;
      dedx_.push_back(energy);
      I3PositionPtr posPtr(new I3Position((*secondary).GetPos()));
      vertex_.push_back(*posPtr);
    }
  }

}

//===================================================================
// Output the energy loss profile to standard output
// 
// Note that the string "EVENT" is a header to extarct the output
// from all the standrd outs from icetray. You can pick up
// the output from this function by using grep and cut, for example,
// on the unix shell.
//
// ex. python yours.py| egrep EVENT | cut -d ' ' -f2-
//===================================================================
void I3OpheliaMillipedeProfile::OutputProfile(){


  vector<I3Position>::iterator dxIter = vertex_.begin();
  // Track start position
  double x_start = (*dxIter).GetX();
  double y_start = (*dxIter).GetY();
  double z_start = (*dxIter).GetZ();
  dxIter = vertex_.end();dxIter--;
  // Track end position
  double x_end = (*dxIter).GetX();
  double y_end = (*dxIter).GetY();
  double z_end = (*dxIter).GetZ();

  double trackLength = sqrt((x_end-x_start)*(x_end-x_start)+(y_end-y_start)*(y_end-y_start)+
			    (z_end-z_start)*(z_end-z_start));
  // directional vector
  double nz = 0.0;
  if(trackLength>0.0){
    nz = (z_end-z_start)/trackLength;
  }

  //
  // Now writing out the formtatted output
  printf("EVENT titx length along the track [m]\n");
  printf("EVENT tity deposited energy  [TeV]\n");
  printf("EVENT mksz 0.5\n");
 
  dxIter = vertex_.begin();
  vector<double>::iterator deIter = dedx_.begin();
  for(;deIter!=dedx_.end(); deIter++){
    double energy = (*deIter);
    double z =  (*dxIter).GetZ();
    double l = 0.0;
    if(trackLength>0.0){
      l = (z-z_start)/nz;
    }
    printf("EVENT data %lf 0.0 %7.4e 0.0\n",l/I3Units::meter,energy/I3Units::TeV);
    printf("EVENT line %lf %e %lf %e\n",l/I3Units::meter,energy/I3Units::TeV,l/I3Units::meter,0.0);
    dxIter++;
  }


  printf("EVENT mssg total deposited energy (%7.4e) [GeV]\n",energyDeposit_/I3Units::GeV);
  printf("EVENT mssg track length (%7.2lf) [m]\n",trackLength/I3Units::meter);
  printf("EVENT plot\n");
  printf("EVENT disp\n");
  printf("EVENT endg\n");

}

