/**
 * @file I3LaputopLikelihood.h
 * @brief declaration of the I3LaputopLikelihood class
 *
 * (c) 2005 * the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author kath
 *
 */

#ifndef I3LAPUTOPLIKELIHOOD_H_INCLUDED
#define I3LAPUTOPLIKELIHOOD_H_INCLUDED

// standard library stuff
#include <string>

// framework stuff
#include "icetray/IcetrayFwd.h"

// Gulliver stuff
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3EventLogLikelihoodBase.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "dataclasses/I3Double.h"
#include "dataclasses/StationKey.h"

//#include "toprec/tankPulse.h"
// For Snow Correction Services
#include "toprec/I3SnowCorrectionService.h"  /* <-- includes tankPulse */
#include "recclasses/I3LaputopParams.h"     /* <-- includes curvature function stuff */

/*  This has been moved to tankPulse.h, so that everybody can get ahold of it
struct tankPulse {
  OMKey omkey;  // needed for LC retriggering
  double x;
  double y;
  double z;
  double t;
  double width;
  double logvem;  // because don't really need vem in THIS llh, for saturated charges : log10(saturatedCharge/VEM)
  double snowdepth;
  bool usepulsetime; //for cutting on highly fluctuating pulses inside laputop (and still using them for charge)
};
*/

/**
 * @class I3LaputopLikelihood
 * @brief Likelihood for icetop event, from expected charge function (i.e. DLP) and expected timing function (i.e. parabola-with-gaussian-nose)
 * 
 * @sa I3EventLogLikelihoodBase
 */
class I3LaputopLikelihood : public I3EventLogLikelihoodBase,
                                  public I3ServiceBase {
 private:

  // Defaults and Descriptions
  static const std::string DEFAULT_DATA_READOUT_LABEL;
  static const std::string DATA_READOUT_TAG;
  static const std::string DATA_READOUT_DESCRIPTION;

  static const std::string DEFAULT_BADTANKLABEL;
  static const std::string BADTANKLABEL_TAG;
  static const std::string BADTANKLABEL_DESCRIPTION;

  static const int DEFAULT_TRIGGER;
  static const std::string TRIGGER_TAG;
  static const std::string TRIGGER_DESCRIPTION;

  static const double DEFAULT_CORECUT;
  static const std::string CORECUT_TAG;
  static const std::string CORECUT_DESCRIPTION;

  static const double DEFAULT_SOFT_THR;
  static const std::string SOFT_THR_TAG;
  static const std::string SOFT_THR_DESCRIPTION;

  static const std::string DEFAULT_LDF;
  static const std::string LDF_TAG;
  static const std::string LDF_DESCRIPTION;

  static const std::string DEFAULT_CURV;
  static const std::string CURV_TAG;
  static const std::string CURV_DESCRIPTION;

  static const std::string DEFAULT_SNOWSERVICENAME;
  static const std::string SNOWSERVICENAME_TAG;
  static const std::string SNOWSERVICENAME_DESCRIPTION;

  //------ VARIABLES --------
  std::string fBadTankLabel;
  std::string fDataReadoutLabel;

  /*
  bool ehe_;
  double ehe_rmin_;
  double ehe_rmax_;
  */

  // Minimum number of stations hit
  unsigned int fTrigger;

  // Radius at which to make the likelihood constant inside
  double fCoreCut;

  // Minimum VEM to include signals
  double fSoftwareThreshold;

  // Charge and Timing functions
  std::string fLDF;
  std::string fCurv;
  Laputop::FrontDelay::Enum fCurvType; 

  // Name of the Snow Service
  std::string fSnowServiceName;

  // Options for using the saturation llh and atmosphere correction
  bool fCorrectAtm_;
  bool fSaturation_;
  
  // Option for extra cleaning for extreme timing fluctuations within station
  double fMaxIntraStaTimeDiff_;
  unsigned int multiplicity_;
  unsigned int timeFluctuatingTanks_;

  // need nStation for part of the multiplicity for each SetEvent
  unsigned int nStation_;
  
  I3RecoPulseSeriesMapConstPtr pulsemap_;

  // contains Hit and Not Hit tanks which we'll use for the fitting
  std::vector<tankPulse> inputData_;
  std::vector<tankPulse> inputEmptyData_;  // for noHitLlh (same struct)
  std::vector<tankPulse> saturatedData_;  // for saturationLlh

  // The core cut radius... is fCoreCut most of the time, but 
  // it may become dynamic in the future.
  double core_radius_;

  // Environment variables from the frame
  I3DoubleConstPtr pressure;
  I3DoubleConstPtr temperature;

  // Pointer to the Snow Correction Service
  I3SnowCorrectionServiceBasePtr snowservice_;

  // Key tank depths (for diagnostics)
  double snowdepth_39B_;
  double snowdepth_44A_;
  double snowdepth_59A_;
  double snowdepth_74A_;


  //Only for internal use
  // Be completely independant from TTopRecoShower, then use functions below
  // OR only use its structs and functions from below? For the moment we chose above solution!
  // This is computationally the most efficient way!
  double GetDistToAxis(const I3Position& core, const I3Direction &dir,std::vector<tankPulse>::iterator it);
  double GetDistToPlane(const double &time, const I3Position& core, const I3Direction &dir,std::vector<tankPulse>::iterator it);
  
  // Helper functions used by FillInput:
  void UpdateFillCounts(std::set<StationKey> &setA, std::set<StationKey> &setB, OMKey key);
  void PrintPulseFill(tankPulse pulse, std::string description);

  // More helper functions:
  // A generic likelihood-computer for curvature functions:
  double top_curv_gausspar_llh(double r, double deltaT, I3LaputopParamsConstPtr par);


public:

    /// constructor for unit tests
    I3LaputopLikelihood( std::string name="unittest" );

    /// constructor for I3Tray
    I3LaputopLikelihood( const I3Context &context );

    /// cleanup
    virtual ~I3LaputopLikelihood(){}

    /// get configuration parameters
    virtual void Configure();

    /// provide event geometry
    void SetGeometry( const I3Geometry &geo ) {};

    /// provide event data
    void SetEvent( const I3Frame &f );


    /// Get +log(likelihood) for a particular emission hypothesis
    double GetLogLikelihood( const I3EventHypothesis &t );

    /// Calc Chi2 after all the likelihood fitting for the charges and times
    void CalcChi2( const I3EventHypothesis &t, double &charge_chi2, double &time_chi2);

    /// This one is required by gulliver.
    unsigned int GetMultiplicity(){ return multiplicity_; }

    /// tell your name
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

  
    //Fill inputData only ONCE and Reset for each event
    unsigned int FillInput(const I3Frame &f);
    void ResetInput();

    // Here are some setter/getter functions, which I need for unit tests.
    // If I knew how to write unit tests more gracefully, maybe I wouldn't need these...
    std::vector<tankPulse> GetInputData() { return inputData_; }
    std::vector<tankPulse> GetInputEmptyData() { return inputEmptyData_; }  
    std::vector<tankPulse> GetInputSaturatedData() { return saturatedData_; } 
    void SetReadoutName(std::string name) { fDataReadoutLabel = name; }
    void SetSnowService(I3SnowCorrectionServiceBasePtr ss) { snowservice_ = ss; }

    //Some Setters for flexible and efficient use of the same service
    void SetFunction(std::string newLDF){ fLDF=newLDF;}
    void SetCurvature(std::string newCurv);  // This one is defined in the .cxx.  It changes both fCurv and fCurvType.

    std::string GetFunction(void){ return fLDF;}
    std::string GetCurvature(void){ return fCurv;}

    //Tools for PERMANENT cleaning of hits from the structure
    //(These used to exist in the old I3TopLateralFit!)
    // Returns number of tanks cut.
    int CutBadTimingPulses(const I3EventHypothesis &t, double t_res_cut );
    int CutCorePulses(const I3EventHypothesis &t, double rcut);

    // This is a part of the base class; let's use it!
    I3FrameObjectPtr GetDiagnostics(const I3EventHypothesis &);

    SET_LOGGER( "Laputop" );

};


I3_POINTER_TYPEDEFS( I3LaputopLikelihood );
#endif /* I3LAPUTOPLIKELIHOOD_H_INCLUDED */
