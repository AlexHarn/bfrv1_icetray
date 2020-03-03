#ifndef I3SNOWSERVICE_H_INCLUDED
#define I3SNOWSERVICE_H_INCLUDED

/**
 *
 * @file I3SnowCorrectionService.h
 * @brief declaration of the I3SnowCorrectionService classes
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision: 90223 $
 * @date $Date$
 * @author kath
 *
 */

#include <string>
#include <icetray/I3Logging.h>
#include <icetray/I3ServiceBase.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/I3Position.h>
#include "toprec/tankPulse.h"
#include "toprec/SnowCorrectionDiagnostics.h"
#include <recclasses/I3LaputopParams.h>
#include <cmath>

/**
 * @class I3SnowCorrectionServiceBase
 * @brief Service (generic base class) to provide snow attenuation to IceTop reconstructors
 *
 */
class I3SnowCorrectionServiceBase : public I3ServiceBase {

 public:
  
  // constructors and destructors
  I3SnowCorrectionServiceBase(const std::string& name) : I3ServiceBase(name) {}
  I3SnowCorrectionServiceBase(const I3Context &c) : I3ServiceBase(c) {}
  virtual ~I3SnowCorrectionServiceBase() {}
  
  /// Do the calculation!  These vary from derived class to derived class
  /// Default = do nothing. (The "Null" snow correction)
  virtual double AttenuationFactor(const I3Position& pos,
                                   double snowDepth,
                                   const I3Particle& hypoth,
                                   const I3LaputopParams& params)
    const
  { return 1.0; }

  // Fill the "snow diagnostics" object; this will be called by Laputop at the end.
  virtual void FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diag, 
                                   I3ParticleConstPtr hypoth,
                                   I3LaputopParamsConstPtr paramPtr)
    const
  {}

  // for Laputop
  double CalculateAttenuatedLogS(double oldlogS,
                                 const tankPulse& tp, 
                                 I3ParticleConstPtr hypoth,
                                 I3LaputopParamsConstPtr paramPtr)
    const
  { return oldlogS + std::log10(AttenuationFactor(I3Position(tp.x, tp.y, tp.z), tp.snowdepth, *hypoth, *paramPtr)); }
  
  // A helper function for general use: (copied from somewhere else)
  double DistToAxis(const I3Particle& part,
                    const I3Position& pos)
    const
  {
    I3Position v = pos - part.GetPos();
    const double d_axis = v * part.GetDir();
    const double ground_r2 = v.Mag2();
    return sqrt(ground_r2 - d_axis * d_axis);
  }

 protected:

  SET_LOGGER( "Laputop" );

};


//---------------- SIMPLE ("LAMBDA") SNOW CORRECTION ---------------------
/**
 * @class I3SimpleSnowCorrectionService
 * @brief This one just attenuates by exp(-slantdepth/lambda), where "lambda" is a configurable parameter
 *
 */
class I3SimpleSnowCorrectionService : public I3SnowCorrectionServiceBase {
 public:
  
  /// default constructor for unit tests
  I3SimpleSnowCorrectionService(const std::string& name, double lambda);
  /// constructor I3Tray
  I3SimpleSnowCorrectionService(const I3Context &c);
  /// destructor
  virtual ~I3SimpleSnowCorrectionService(){}
  
  void Configure();
  
  /// Setter function for Lambda
  void ResetLambda(double newlambda);
  double GetLambda() const { return fLambda_; }
  
  /// Do the calculation!
  virtual double AttenuationFactor(const I3Position&,
                                   double,
				                           const I3Particle&,
                                   const I3LaputopParams&) const;

  virtual void FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr,
                                   I3ParticleConstPtr,
                                   I3LaputopParamsConstPtr) const;

 private:

  static const std::string LAMBDA_TAG;
  static const double DEFAULT_SIMPLE_LAMBDA;
  
  // Internal variables
  double fLambda_;

};

//---------------- BORS ---------------------
/**
 * @class I3BORSSnowCorrectionService
 * @brief This one implements Kath's BORS function
 *
 */
class I3BORSSnowCorrectionService : public I3SnowCorrectionServiceBase {
 public:
  
  /// default constructor for unit tests
  I3BORSSnowCorrectionService(const std::string& name, bool fEMonly);
  /// constructor I3Tray
  I3BORSSnowCorrectionService(const I3Context &c);
  /// destructor
  virtual ~I3BORSSnowCorrectionService(){}
  
  void Configure();

  /// Do the calculation!
  virtual double AttenuationFactor(const I3Position&,
                                   double,
                                   const I3Particle&,
                                   const I3LaputopParams&) const;

  virtual void FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diag,
                                   I3ParticleConstPtr hypoth,
                                   I3LaputopParamsConstPtr paramPtr) const;

  // Additional functions
  double DominantExponentialSlope(double r, double t) const;
  double TurnoverExponentialSlope(double r, double t) const;
  double Turnover_c0(double r, double t) const;
  double FractionEM(double r, double logS125) const;
  double T_from_beta_zenith(double beta, double zenith) const;

  // Getter functions (may be useful for diagnostics?
  double GetTStage() const { return t_; }

  // Setter function: so you can send it tstage directly, and
  // don't have to compute T from beta/zenith (for instance, Javier's two-LDF)
  // You'd want to call this BEFORE calling CalculateAttenuatedS.
  // WARNING: may do strange things if one service is being called by multiple modules!
  void SetTStage(double myt) {
    t_ = myt;
    t_set_externally_ = 1;
  }
  void UnsetTStage() { t_set_externally_ = 0; }

 private:

  static const std::string EM_ONLY_TAG;

  // User parameters
  bool fEMonly_;

  // Internal variables
  mutable double t_;
  bool t_set_externally_;

};

//---------------- RADE ---------------------
/**
 * @class I3RadeBasicSnowCorrectionService
 * @brief This one implements Kath's second attempt at a radius-dependent function
 *
 */
class I3RadeBasicSnowCorrectionService : public I3SnowCorrectionServiceBase {
 public:
  
  /// default constructor/destructors for unit tests
  I3RadeBasicSnowCorrectionService(const std::string& name);
  I3RadeBasicSnowCorrectionService(const I3Context &c);
  virtual ~I3RadeBasicSnowCorrectionService(){}
  
  void Configure(){}  // Nothing special

  /// Standard functions
  virtual double AttenuationFactor(const I3Position&,
                                   double snowDepth,
                                   const I3Particle&,
                                   const I3LaputopParams&) const;

  virtual void FillSnowDiagnostics(SnowCorrectionDiagnosticsPtr diag,
                                   I3ParticleConstPtr hypoth,
                                   I3LaputopParamsConstPtr paramPtr) const;

  // Additional functions
  double Lambda(double r, double s125) const;

 private:
    // (none)
};

I3_POINTER_TYPEDEFS( I3SnowCorrectionServiceBase );
I3_POINTER_TYPEDEFS( I3SimpleSnowCorrectionService );
I3_POINTER_TYPEDEFS( I3BORSSnowCorrectionService );
I3_POINTER_TYPEDEFS( I3RadeBasicSnowCorrectionService );

#endif
