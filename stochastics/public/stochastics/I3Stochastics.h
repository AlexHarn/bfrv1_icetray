#ifndef I3STOCHASTICS_H_INCLUDED
#define I3STOCHASTICS_H_INCLUDED

#include "icetray/I3Module.h"
#include "icetray/I3ConditionalModule.h"

#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"

#include <vector>
#include <bitset>
#include <string>
#include <icetray/I3Logging.h>

struct energyloss {

  double zenith; // of segment
  double energy;
  double slantDepth;
};

class I3Stochastics : public I3ConditionalModule
{
  
 public:
  
  //Function prototypes
  I3Stochastics(const I3Context& context);
  
  void Configure();
  
  void Physics(I3FramePtr frame);
  
  
 private : 
  
 
  //Friend functions so they can access the private variables
  friend void elbert_simple_fcn(int &nPar, double *grad, double &value, double *params, int iflag);
  
  friend double eloss_llh(std::vector<energyloss> inputData, double *par); //also eloss NonSimple?
  friend double muonBundleEnergyLoss(double slantDepth, double zenith, double *par);

  //internal variable, which needs to used inside the likelihood function
  static std::vector<energyloss> energyLossData_; 
  
  std::string inputName_;
  std::string outputName_;
  std::string outputName_red_;
  // selection related variables
  double a_sel_;
  double b_sel_;
  double c_sel_;
  std::string type_sel_;

  // minimization related settings
  bool verbose_;
  std::string minimizer_;
  int freePar_;
  double tolerance_;
  int nIterations_;

  std::bitset<6> freeParams_;
  int fixParams_;

  SET_LOGGER("I3Stochastics");
};

// Likelihood functions declared outside class scope for friend functions
// TMinuit interface
void elbert_simple_fcn(int &nPar, double *grad, double &value, double *params, int iflag);

double eloss_llh(std::vector<energyloss> inputData, double *par); //also eloss NonSimple?
double muonBundleEnergyLoss(double slantDepth, double zenith, double *par);



#endif
