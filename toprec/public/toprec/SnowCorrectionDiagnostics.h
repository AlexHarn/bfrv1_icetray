#ifndef I3SNOWDIAGNOSTICS_H_INCLUDED
#define I3SNOWDIAGNOSTICS_H_INCLUDED


#include "icetray/I3FrameObject.h"
#include "icetray/serialization.h"

// ----------- A STRUCT FOR DIAGNOSTICS ---------------

class SnowCorrectionDiagnostics : public I3FrameObject {
 public:

  // The stage of shower evolution
  // The "raw output" of the cheap function that calculates this:
  double tstage; 
  // The one that is actually used, restricted to be between -1 and 9 (or something)
  double tstage_restricted;
  
  // Fraction of signal which is EM
  double fEM_30m;
  double fEM_50m;
  double fEM_80m;
  double fEM_100m;
  double fEM_125m;
  double fEM_150m;
  double fEM_200m;
  double fEM_300m;
  double fEM_500m;
  double fEM_1000m;

  // Effective lambda's: EM only (one based on the "raw" t, and one based on the "restricted" t)
  double lambda_EM_30m;
  double lambda_EM_50m;
  double lambda_EM_80m;
  double lambda_EM_100m;
  double lambda_EM_125m;
  double lambda_EM_150m;
  double lambda_EM_200m;
  double lambda_EM_300m;
  double lambda_EM_500m;
  double lambda_EM_1000m;
  double lambda_EM_30m_restricted;
  double lambda_EM_50m_restricted;
  double lambda_EM_80m_restricted;
  double lambda_EM_100m_restricted;
  double lambda_EM_125m_restricted;
  double lambda_EM_150m_restricted;
  double lambda_EM_200m_restricted;
  double lambda_EM_300m_restricted;
  double lambda_EM_500m_restricted;
  double lambda_EM_1000m_restricted;
  
  /*
  // s/s_nosnow = (fEM)*exp(-x/lambdaEM) + (1-fEM)
  // So, the "effective attenuation length" is:
  // exp(-x/lambda_eff) = (fEM)*exp(-x/lambdaEM) + (1-fEM), which depends on "x" so not a simple thing.
  // Maybe I'll code up something like this later.
  double lambda_eff_30m;
  double lambda_eff_50m;
  double lambda_eff_80m;
  double lambda_eff_100m;
  double lambda_eff_125m;
  double lambda_eff_150m;
  double lambda_eff_200m;
  double lambda_eff_300m;
  double lambda_eff_500m;
  double lambda_eff_1000m;
  */

  // Snow depths used for certain "key" tanks: 39B, 44A, 59A, 74A
  // (See "Snowtables" wiki page)
  double snowdepth_39B;
  double snowdepth_44A;
  double snowdepth_59A;
  double snowdepth_74A;
  
  /// Empty boring constructor
 SnowCorrectionDiagnostics() :
    tstage(NAN),
    tstage_restricted(NAN),
    fEM_30m(NAN),
    fEM_50m(NAN),
    fEM_80m(NAN),
    fEM_100m(NAN),
    fEM_125m(NAN),
    fEM_150m(NAN),
    fEM_200m(NAN),
    fEM_300m(NAN),
    fEM_500m(NAN),
    fEM_1000m(NAN),
    lambda_EM_30m(NAN),
    lambda_EM_50m(NAN),
    lambda_EM_80m(NAN),
    lambda_EM_100m(NAN),
    lambda_EM_125m(NAN),
    lambda_EM_150m(NAN),
    lambda_EM_200m(NAN),
    lambda_EM_300m(NAN),
    lambda_EM_500m(NAN),
    lambda_EM_1000m(NAN),
    lambda_EM_30m_restricted(NAN),
    lambda_EM_50m_restricted(NAN),
    lambda_EM_80m_restricted(NAN),
    lambda_EM_100m_restricted(NAN),
    lambda_EM_125m_restricted(NAN),
    lambda_EM_150m_restricted(NAN),
    lambda_EM_200m_restricted(NAN),
    lambda_EM_300m_restricted(NAN),
    lambda_EM_500m_restricted(NAN),
    lambda_EM_1000m_restricted(NAN),
    snowdepth_39B(NAN),
    snowdepth_44A(NAN),
    snowdepth_59A(NAN),
    snowdepth_74A(NAN)
    {}
  
  /// cleanup
  virtual ~SnowCorrectionDiagnostics(){}
  
 protected:
  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive& ar, unsigned version);
};


I3_POINTER_TYPEDEFS( SnowCorrectionDiagnostics );

#endif
