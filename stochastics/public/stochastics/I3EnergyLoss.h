#ifndef I3ENERGYLOSS_H_INCLUDED
#define I3ENERGYLOSS_H_INCLUDED

#include <icetray/I3FrameObject.h>      //must inherit from FrameObject to be put into the frame
#include "dataclasses/Utility.h"
#include "dataclasses/physics/I3Particle.h"

static const unsigned i3energyloss_version_ = 1;

class I3EnergyLoss : public I3FrameObject
{
 
 public:
  //the params
  double Eloss_1000;
  double Eloss_1500;
  double Eloss_1600;
  double Eloss_1700;
  double Eloss_1800;
  double Eloss_1900;
  double Eloss_2000;
  double Eloss_2100;
  double Eloss_2200;
  double Eloss_2300;
  double Eloss_2400;
  double Eloss_3000;

  double primMassEstimate; 
  double primEnergyEstimate;
  double primMassEstimate_err; 
  double primEnergyEstimate_err;
  
  int nHEstoch;
  double avStochEnergy;
  double avRelStochEnergy;
  double highestStochEnergy;
  double highestRelStochEnergy;
  double totalStochEnergy;
  double totalRelStochEnergy;
  double chi2;
  double chi2_red;
  double avStochDepth;

  I3Particle::FitStatus status;

 I3EnergyLoss() :
  Eloss_1000(NAN), Eloss_1500(NAN), Eloss_1600(NAN),
  Eloss_1700(NAN), Eloss_1800(NAN), Eloss_1900(NAN),
  Eloss_2000(NAN), Eloss_2100(NAN), Eloss_2200(NAN),
  Eloss_2300(NAN), Eloss_2400(NAN), Eloss_3000(NAN),
  primMassEstimate(NAN), primEnergyEstimate(NAN),
  primMassEstimate_err(NAN), primEnergyEstimate_err(NAN),
  nHEstoch(-1), avStochEnergy(NAN), avRelStochEnergy(NAN), 
    highestStochEnergy(NAN), highestRelStochEnergy(NAN),
    totalStochEnergy(NAN), totalRelStochEnergy(NAN),
    chi2(NAN), chi2_red(NAN),
    avStochDepth(NAN), status(I3Particle::NotSet) {};
  
  virtual ~I3EnergyLoss() {};

 private:

  //serialization routine
  friend class icecube::serialization::access;         //provide boost access to my privates
  template <class Archive> void serialize(Archive& ar, unsigned version);

};

std::ostream& operator<<(std::ostream&, const I3EnergyLoss&);

//gives some shared_ptr typedefs...
I3_CLASS_VERSION(I3EnergyLoss,i3energyloss_version_);
I3_POINTER_TYPEDEFS(I3EnergyLoss);

#endif
    
