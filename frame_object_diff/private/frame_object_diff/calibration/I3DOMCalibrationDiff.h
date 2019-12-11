/**
 *
 * copyright  (C) 2015
 * the icecube collaboration
 * @version n$Id: I3Geometry.h 88543 2012-05-22 04:54:48Z claudio.kopper $
 * @file I3DOMCalibrationDiff.h
 * @date $Date: 2012-05-21 23:54:48 -0500 (Mon, 21 May 2012) $
 */

#ifndef I3DOMCALIBRATIONDIFF_H_INCLUDED
#define I3DOMCALIBRATIONDIFF_H_INCLUDED

#include <bitset>

#include "dataclasses/calibration/I3Calibration.h"
#include "frame_object_diff/I3MapDiff.h"

static const unsigned i3domcalibrationdiff_version_ = 0;

/**
 * Store the difference between two I3DOMCalibration objects.
 */
class I3DOMCalibrationDiff
{
public:
  /**
   * The bitset, listing which objects are different from the base.
   * This also gets used during serialization, to tell what to save/load.
   */
  std::bitset<29> bits;

  I3DOMCalibrationDiff() { }

  /**
   * Create a Diff against a bse, for the cur (cal) object.
   */
  I3DOMCalibrationDiff(const I3DOMCalibration& base,
      const I3DOMCalibration& cal);
  I3DOMCalibrationDiff(I3DOMCalibrationConstPtr base,
      I3DOMCalibrationConstPtr cal);

  /**
   * Unpack the Diff, returning a shared pointer to
   * the original object.
   *
   * Takes the base that was originially provided to the
   * constructor as input.
   */
  I3DOMCalibrationPtr Unpack(const I3DOMCalibration& base) const;
  I3DOMCalibrationPtr Unpack(I3DOMCalibrationConstPtr base) const;

  bool operator==(const I3DOMCalibrationDiff& other) const;

private:
  /**
   * Shared initialization between both constructors.
   */
  void Init_(const I3DOMCalibration& base,
      const I3DOMCalibration& cal);

  // Mirrors of the values in I3DOMCalibration
  // TODO: Think about just storing an actual I3DOMCalibration
  //       object instead.
  double droopTimeConstants_[2];
  double temperature_;
  double fadcGain_;
  LinearFit fadcBaselineFit_;
  double fadcBeaconBaseline_;
  double fadcDeltaT_;
  double frontEndImpedance_;
  TauParam tauparameters_;
  double ampGains_[3];
  QuadraticFit atwdFreq_[2];
  double atwdBins_[2][I3DOMCalibration::N_ATWD_CHANNELS][I3DOMCalibration::N_ATWD_BINS];
  LinearFit pmtTransitTime_;
  LinearFit hvGainRelation_;
  std::string domcalVersion_;
  double atwdBeaconBaselines_[2][I3DOMCalibration::N_ATWD_CHANNELS];
  double atwdDeltaT_[2];
  LinearFit speDiscrimCalib_;
  LinearFit mpeDiscrimCalib_;
  LinearFit pmtDiscrimCalib_;
  double relativeDomEff_;
  double noiseRate_;
  double noiseThermalRate_;
  double noiseDecayRate_;
  double noiseScintillationMean_;
  double noiseScintillationSigma_;
  double noiseScintillationHits_;
  SPEChargeDistribution combinedSPEFit_;
  double meanATWDCharge_;
  double meanFADCCharge_;

  /**
   * A shared pointer to the unpacked data, so we don't
   * have to regenerate this for subsequent calls.
   */
  mutable I3DOMCalibrationPtr unpacked_;

  friend class icecube::serialization::access;
  template <class Archive> void serialize(Archive & ar, unsigned version);
  
  friend std::ostream& operator<<(std::ostream& oss, const I3DOMCalibrationDiff& m);
};

std::ostream& operator<<(std::ostream& oss, const I3DOMCalibrationDiff& m);

I3_POINTER_TYPEDEFS(I3DOMCalibrationDiff);
I3_CLASS_VERSION(I3DOMCalibrationDiff, i3domcalibrationdiff_version_);

typedef MapDiff<OMKey, I3DOMCalibrationDiff, I3DOMCalibration> I3DOMCalibrationMapDiff;
I3_POINTER_TYPEDEFS(I3DOMCalibrationMapDiff);
I3_CLASS_VERSION(I3DOMCalibrationMapDiff, mapdiff_version_);

#endif // I3DOMCALIBRATIONDIFF_H_INCLUDED
