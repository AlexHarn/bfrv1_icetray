#include <serialization/bitset.hpp>
#include <icetray/serialization.h>
#include <frame_object_diff/calibration/I3DOMCalibrationDiff.h>

I3DOMCalibrationDiff::I3DOMCalibrationDiff(const I3DOMCalibration& base,
    const I3DOMCalibration& cal)
{
  Init_(base,cal);
}

I3DOMCalibrationDiff::I3DOMCalibrationDiff(I3DOMCalibrationConstPtr base,
    I3DOMCalibrationConstPtr cal)
{
  Init_(*base,*cal);
}

void
I3DOMCalibrationDiff::Init_(const I3DOMCalibration& base,
    const I3DOMCalibration& cal)
{
  bits.set();
  if (CompareFloatingPoint::Compare_NanEqual(base.droopTimeConstants_[0],
          cal.droopTimeConstants_[0]) &&
      CompareFloatingPoint::Compare_NanEqual(base.droopTimeConstants_[1],
          cal.droopTimeConstants_[1]))
    bits[0] = 0;
  else {
    droopTimeConstants_[0] = cal.droopTimeConstants_[0];
    droopTimeConstants_[1] = cal.droopTimeConstants_[1];
  }
  if (CompareFloatingPoint::Compare_NanEqual(base.temperature_,cal.temperature_))
    bits[1] = 0;
  else
    temperature_ = cal.temperature_;
  if (CompareFloatingPoint::Compare_NanEqual(base.fadcGain_,cal.fadcGain_))
    bits[2] = 0;
  else
    fadcGain_ = cal.fadcGain_;
  if (base.fadcBaselineFit_ == cal.fadcBaselineFit_)
    bits[3] = 0;
  else
    fadcBaselineFit_ = cal.fadcBaselineFit_;
  if (CompareFloatingPoint::Compare_NanEqual(base.fadcBeaconBaseline_,
      cal.fadcBeaconBaseline_))
    bits[4] = 0;
  else
    fadcBeaconBaseline_ = cal.fadcBeaconBaseline_;
  if (CompareFloatingPoint::Compare_NanEqual(base.fadcDeltaT_,cal.fadcDeltaT_))
    bits[5] = 0;
  else
    fadcDeltaT_ = cal.fadcDeltaT_;
  if (CompareFloatingPoint::Compare_NanEqual(base.frontEndImpedance_,cal.frontEndImpedance_))
    bits[6] = 0;
  else
    frontEndImpedance_ = cal.frontEndImpedance_;
  if (base.tauparameters_ == cal.tauparameters_)
    bits[7] = 0;
  else
    tauparameters_ = cal.tauparameters_;
  if (CompareFloatingPoint::Compare_NanEqual(base.ampGains_[0],cal.ampGains_[0]) &&
      CompareFloatingPoint::Compare_NanEqual(base.ampGains_[1],cal.ampGains_[1]) &&
      CompareFloatingPoint::Compare_NanEqual(base.ampGains_[2],cal.ampGains_[2]))
    bits[8] = 0;
  else {
    ampGains_[0] = cal.ampGains_[0];
    ampGains_[1] = cal.ampGains_[1];
    ampGains_[2] = cal.ampGains_[2];
  }
  if (base.atwdFreq_[0] == cal.atwdFreq_[0] &&
      base.atwdFreq_[1] == cal.atwdFreq_[1])
    bits[9] = 0;
  else {
    atwdFreq_[0] = cal.atwdFreq_[0];
    atwdFreq_[1] = cal.atwdFreq_[1];
  }
  if (std::equal(&base.atwdBins_[0][0][0], &base.atwdBins_[0][0][0] +
      2*I3DOMCalibration::N_ATWD_CHANNELS*I3DOMCalibration::N_ATWD_BINS,
      &cal.atwdBins_[0][0][0]),CompareFloatingPoint::Compare_NanEqual)
    bits[10] = 0;
  else
    std::copy(&cal.atwdBins_[0][0][0], &cal.atwdBins_[0][0][0] +
        2*I3DOMCalibration::N_ATWD_CHANNELS*I3DOMCalibration::N_ATWD_BINS,
        &atwdBins_[0][0][0]);
  if (base.pmtTransitTime_ == cal.pmtTransitTime_)
    bits[11] = 0;
  else
    pmtTransitTime_ = cal.pmtTransitTime_;
  if (base.hvGainRelation_ == cal.hvGainRelation_)
    bits[12] = 0;
  else
    hvGainRelation_ = cal.hvGainRelation_;
  if (base.domcalVersion_ == cal.domcalVersion_)
    bits[13] = 0;
  else
    domcalVersion_ = cal.domcalVersion_;
  if (std::equal(&base.atwdBeaconBaselines_[0][0],
      &base.atwdBeaconBaselines_[0][0] + 
      2*I3DOMCalibration::N_ATWD_CHANNELS,
      &cal.atwdBeaconBaselines_[0][0]),CompareFloatingPoint::Compare_NanEqual)
    bits[14] = 0;
  else
    std::copy(&cal.atwdBeaconBaselines_[0][0],
        &cal.atwdBeaconBaselines_[0][0] +
        2*I3DOMCalibration::N_ATWD_CHANNELS,
        &atwdBeaconBaselines_[0][0]);
  if (CompareFloatingPoint::Compare_NanEqual(base.atwdDeltaT_[0],cal.atwdDeltaT_[0]) &&
      CompareFloatingPoint::Compare_NanEqual(base.atwdDeltaT_[1],cal.atwdDeltaT_[1]))
    bits[15] = 0;
  else {
    atwdDeltaT_[0] = cal.atwdDeltaT_[0];
    atwdDeltaT_[1] = cal.atwdDeltaT_[1];
  }
  if (base.speDiscrimCalib_ == cal.speDiscrimCalib_)
    bits[16] = 0;
  else
    speDiscrimCalib_ = cal.speDiscrimCalib_;
  if (base.mpeDiscrimCalib_ == cal.mpeDiscrimCalib_)
    bits[17] = 0;
  else
    mpeDiscrimCalib_ = cal.mpeDiscrimCalib_;
  if (base.pmtDiscrimCalib_ == cal.pmtDiscrimCalib_)
    bits[18] = 0;
  else
    pmtDiscrimCalib_ = cal.pmtDiscrimCalib_;
  if (CompareFloatingPoint::Compare_NanEqual(base.relativeDomEff_,cal.relativeDomEff_))
    bits[19] = 0;
  else
    relativeDomEff_ = cal.relativeDomEff_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseRate_,cal.noiseRate_))
    bits[20] = 0;
  else
    noiseRate_ = cal.noiseRate_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseThermalRate_,cal.noiseThermalRate_))
    bits[21] = 0;
  else
    noiseThermalRate_ = cal.noiseThermalRate_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseDecayRate_,cal.noiseDecayRate_))
    bits[22] = 0;
  else
    noiseDecayRate_ = cal.noiseDecayRate_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseScintillationMean_,
      cal.noiseScintillationMean_))
    bits[23] = 0;
  else
    noiseScintillationMean_ = cal.noiseScintillationMean_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseScintillationSigma_,
      cal.noiseScintillationSigma_))
    bits[24] = 0;
  else
    noiseScintillationSigma_ = cal.noiseScintillationSigma_;
  if (CompareFloatingPoint::Compare_NanEqual(base.noiseScintillationHits_,
      cal.noiseScintillationHits_))
    bits[25] = 0;
  else
    noiseScintillationHits_ = cal.noiseScintillationHits_;
  if (base.combinedSPEFit_ == cal.combinedSPEFit_)
    bits[26] = 0;
  else
    combinedSPEFit_ = cal.combinedSPEFit_;
  if (CompareFloatingPoint::Compare_NanEqual(base.meanATWDCharge_,
      cal.meanATWDCharge_))
    bits[27] = 0;
  else
    meanATWDCharge_ = cal.meanATWDCharge_;
  if (CompareFloatingPoint::Compare_NanEqual(base.meanFADCCharge_,
      cal.meanFADCCharge_))
    bits[28] = 0;
  else
    meanFADCCharge_ = cal.meanFADCCharge_;
}

I3DOMCalibrationPtr
I3DOMCalibrationDiff::Unpack(const I3DOMCalibration& base) const
{
  if (unpacked_)
    return unpacked_;
  
  unpacked_ = I3DOMCalibrationPtr(new I3DOMCalibration());
  if (bits[0]) {
    unpacked_->droopTimeConstants_[0] = droopTimeConstants_[0];
    unpacked_->droopTimeConstants_[1] = droopTimeConstants_[1];
  } else {
    unpacked_->droopTimeConstants_[0] = base.droopTimeConstants_[0];
    unpacked_->droopTimeConstants_[1] = base.droopTimeConstants_[1];
  }
  if (bits[1])
    unpacked_->temperature_ = temperature_;
  else
    unpacked_->temperature_ = base.temperature_;
  if (bits[2])
    unpacked_->fadcGain_ = fadcGain_;
  else
    unpacked_->fadcGain_ = base.fadcGain_;
  if (bits[3])
    unpacked_->fadcBaselineFit_ = fadcBaselineFit_;
  else
    unpacked_->fadcBaselineFit_ = base.fadcBaselineFit_;
  if (bits[4])
    unpacked_->fadcBeaconBaseline_ = fadcBeaconBaseline_;
  else
    unpacked_->fadcBeaconBaseline_ = base.fadcBeaconBaseline_;
  if (bits[5])
    unpacked_->fadcDeltaT_ = fadcDeltaT_;
  else
    unpacked_->fadcDeltaT_ = base.fadcDeltaT_;
  if (bits[6])
    unpacked_->frontEndImpedance_ = frontEndImpedance_;
  else
    unpacked_->frontEndImpedance_ = base.frontEndImpedance_;
  if (bits[7])
    unpacked_->tauparameters_ = tauparameters_;
  else
    unpacked_->tauparameters_ = base.tauparameters_;
  if (bits[8]) {
    unpacked_->ampGains_[0] = ampGains_[0];
    unpacked_->ampGains_[1] = ampGains_[1];
    unpacked_->ampGains_[2] = ampGains_[2];
  } else {
    unpacked_->ampGains_[0] = base.ampGains_[0];
    unpacked_->ampGains_[1] = base.ampGains_[1];
    unpacked_->ampGains_[2] = base.ampGains_[2];
  }
  if (bits[9]) {
    unpacked_->atwdFreq_[0] = atwdFreq_[0];
    unpacked_->atwdFreq_[1] = atwdFreq_[1];
  } else {
    unpacked_->atwdFreq_[0] = base.atwdFreq_[0];
    unpacked_->atwdFreq_[1] = base.atwdFreq_[1];
  }
  if (bits[10])
    std::copy(&atwdBins_[0][0][0], &atwdBins_[0][0][0] +
        2*I3DOMCalibration::N_ATWD_CHANNELS*I3DOMCalibration::N_ATWD_BINS,
        &unpacked_->atwdBins_[0][0][0]);
  else
    std::copy(&base.atwdBins_[0][0][0], &base.atwdBins_[0][0][0] +
        2*I3DOMCalibration::N_ATWD_CHANNELS*I3DOMCalibration::N_ATWD_BINS,
        &unpacked_->atwdBins_[0][0][0]);
  if (bits[11])
    unpacked_->pmtTransitTime_ = pmtTransitTime_;
  else
    unpacked_->pmtTransitTime_ = base.pmtTransitTime_;
  if (bits[12])
    unpacked_->hvGainRelation_ = hvGainRelation_;
  else
    unpacked_->hvGainRelation_ = base.hvGainRelation_;
  if (bits[13])
    unpacked_->domcalVersion_ = domcalVersion_;
  else
    unpacked_->domcalVersion_ = base.domcalVersion_;
  if (bits[14])
    std::copy(&atwdBeaconBaselines_[0][0],
        &atwdBeaconBaselines_[0][0] + 2*I3DOMCalibration::N_ATWD_CHANNELS,
        &unpacked_->atwdBeaconBaselines_[0][0]);
  else
    std::copy(&base.atwdBeaconBaselines_[0][0],
        &base.atwdBeaconBaselines_[0][0] + 
        2*I3DOMCalibration::N_ATWD_CHANNELS,
        &unpacked_->atwdBeaconBaselines_[0][0]);
  if (bits[15]) {
    unpacked_->atwdDeltaT_[0] = atwdDeltaT_[0];
    unpacked_->atwdDeltaT_[1] = atwdDeltaT_[1];
  } else {
    unpacked_->atwdDeltaT_[0] = base.atwdDeltaT_[0];
    unpacked_->atwdDeltaT_[1] = base.atwdDeltaT_[1];
  }
  if (bits[16])
    unpacked_->speDiscrimCalib_ = speDiscrimCalib_;
  else
    unpacked_->speDiscrimCalib_ = base.speDiscrimCalib_;
  if (bits[17])
    unpacked_->mpeDiscrimCalib_ = mpeDiscrimCalib_;
  else
    unpacked_->mpeDiscrimCalib_ = base.mpeDiscrimCalib_;
  if (bits[18])
    unpacked_->pmtDiscrimCalib_ = pmtDiscrimCalib_;
  else
    unpacked_->pmtDiscrimCalib_ = base.pmtDiscrimCalib_;
  if (bits[19])
    unpacked_->relativeDomEff_ = relativeDomEff_;
  else
    unpacked_->relativeDomEff_ = base.relativeDomEff_;
  if (bits[20])
    unpacked_->noiseRate_ = noiseRate_;
  else
    unpacked_->noiseRate_ = base.noiseRate_;
  if (bits[21])
    unpacked_->noiseThermalRate_ = noiseThermalRate_;
  else
    unpacked_->noiseThermalRate_ = base.noiseThermalRate_;
  if (bits[22])
    unpacked_->noiseDecayRate_ = noiseDecayRate_;
  else
    unpacked_->noiseDecayRate_ = base.noiseDecayRate_;
  if (bits[23])
    unpacked_->noiseScintillationMean_ = noiseScintillationMean_;
  else
    unpacked_->noiseScintillationMean_ = base.noiseScintillationMean_;
  if (bits[24])
    unpacked_->noiseScintillationSigma_ = noiseScintillationSigma_;
  else
    unpacked_->noiseScintillationSigma_ = base.noiseScintillationSigma_;
  if (bits[25])
    unpacked_->noiseScintillationHits_ = noiseScintillationHits_;
  else
    unpacked_->noiseScintillationHits_ = base.noiseScintillationHits_;
  if (bits[26])
    unpacked_->combinedSPEFit_ = combinedSPEFit_;
  else
    unpacked_->combinedSPEFit_ = base.combinedSPEFit_;
  if (bits[27])
    unpacked_->meanATWDCharge_ = meanATWDCharge_;
  else
    unpacked_->meanATWDCharge_ = base.meanATWDCharge_;
  if (bits[28])
    unpacked_->meanFADCCharge_ = meanFADCCharge_;
  else
    unpacked_->meanFADCCharge_ = base.meanFADCCharge_;
  return unpacked_;
}

I3DOMCalibrationPtr
I3DOMCalibrationDiff::Unpack(I3DOMCalibrationConstPtr base) const
{
  return Unpack(*base);
}

bool
I3DOMCalibrationDiff::operator==(const I3DOMCalibrationDiff& other) const
{
  if (bits != other.bits)
    return false;

  if (bits[0] &&
      (droopTimeConstants_[0] != other.droopTimeConstants_[0] ||
       droopTimeConstants_[1] != other.droopTimeConstants_[1]))
    return false;
  if (bits[1] && temperature_ != other.temperature_)
    return false;
  if (bits[2] && fadcGain_ != other.fadcGain_)
    return false;
  if (bits[3] && fadcBaselineFit_ != other.fadcBaselineFit_)
    return false;
  if (bits[4] && fadcBeaconBaseline_ != other.fadcBeaconBaseline_)
    return false;
  if (bits[5] && fadcDeltaT_ != other.fadcDeltaT_)
    return false;
  if (bits[6] && frontEndImpedance_ != other.frontEndImpedance_)
    return false;
  if (bits[7] && tauparameters_ != other.tauparameters_)
    return false;
  if (bits[8] &&
      (ampGains_[0] != other.ampGains_[0] ||
       ampGains_[1] != other.ampGains_[1] ||
       ampGains_[2] != other.ampGains_[2]))
    return false;
  if (bits[9] &&
      (atwdFreq_[0] != other.atwdFreq_[0] ||
       atwdFreq_[1] != other.atwdFreq_[1]))
    return false;
  if (bits[10] && !std::equal(&atwdBins_[0][0][0],&atwdBins_[0][0][0] +
          2*I3DOMCalibration::N_ATWD_CHANNELS*I3DOMCalibration::N_ATWD_BINS,
          &(other.atwdBins_[0][0][0])))
    return false;
  if (bits[11] && pmtTransitTime_ != other.pmtTransitTime_)
    return false;
  if (bits[12] && hvGainRelation_ != other.hvGainRelation_)
    return false;
  if (bits[13] && domcalVersion_ != other.domcalVersion_)
    return false;
  if (bits[14] && !std::equal(&atwdBeaconBaselines_[0][0],
      &atwdBeaconBaselines_[0][0] + 2*I3DOMCalibration::N_ATWD_CHANNELS,
      &(other.atwdBeaconBaselines_[0][0])))
    return false;
  if (bits[15] &&
      (atwdDeltaT_[0] != other.atwdDeltaT_[0] ||
       atwdDeltaT_[1] != other.atwdDeltaT_[1]))
    return false;
  if (bits[16] && speDiscrimCalib_ != other.speDiscrimCalib_)
    return false;
  if (bits[17] && mpeDiscrimCalib_ != other.mpeDiscrimCalib_)
    return false;
  if (bits[18] && pmtDiscrimCalib_ != other.pmtDiscrimCalib_)
    return false;
  if (bits[19] && relativeDomEff_ != other.relativeDomEff_)
    return false;
  if (bits[20] && noiseRate_ != other.noiseRate_)
    return false;
  if (bits[21] && noiseThermalRate_ != other.noiseThermalRate_)
    return false;
  if (bits[22] && noiseDecayRate_ != other.noiseDecayRate_)
    return false;
  if (bits[23] && noiseScintillationMean_ != other.noiseScintillationMean_)
    return false;
  if (bits[24] && noiseScintillationSigma_ != other.noiseScintillationSigma_)
    return false;
  if (bits[25] && noiseScintillationHits_ != other.noiseScintillationHits_)
    return false;
  if (bits[26] && combinedSPEFit_ != other.combinedSPEFit_)
    return false;
  if (bits[27] && meanATWDCharge_ != other.meanATWDCharge_)
    return false;
  if (bits[28] && meanFADCCharge_ != other.meanFADCCharge_)
    return false;

  return true;
}

template <class Archive>
void 
I3DOMCalibrationDiff::serialize(Archive& ar, unsigned version)
{
  if (version>i3domcalibrationdiff_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3DOMCalibrationDiff class.",
        version, i3domcalibrationdiff_version_);

  ar & make_nvp("bitset",bits);

  if (bits[0])
    ar & make_nvp("droopTimeConstants", droopTimeConstants_);
  if (bits[1])
    ar & make_nvp("temperature", temperature_);
  if (bits[2])
    ar & make_nvp("fadcGain", fadcGain_);
  if (bits[3])
    ar & make_nvp("fadcBaselineFit", fadcBaselineFit_);
  if (bits[4])
    ar & make_nvp("fadcBeaconBaseline", fadcBeaconBaseline_);
  if (bits[5])
    ar & make_nvp("fadcDeltaT", fadcDeltaT_);
  if (bits[6])
    ar & make_nvp("frontEndImpedance", frontEndImpedance_);
  if (bits[7])
    ar & make_nvp("tauparameters", tauparameters_);
  if (bits[8])
    ar & make_nvp("ampGains", ampGains_);
  if (bits[9])
    ar & make_nvp("atwdFreq", atwdFreq_);
  if (bits[10])
    ar & make_nvp("atwdBins", atwdBins_);
  if (bits[11])
    ar & make_nvp("pmtTransitTime", pmtTransitTime_);
  if (bits[12])
    ar & make_nvp("hvGainRelation", hvGainRelation_);
  if (bits[13])
    ar & make_nvp("domcalVersion", domcalVersion_);
  if (bits[14])
    ar & make_nvp("atwdBeaconBaselines", atwdBeaconBaselines_);
  if (bits[15])
    ar & make_nvp("atwdDeltaT", atwdDeltaT_);
  if (bits[16])
    ar & make_nvp("speDiscrimCalib", speDiscrimCalib_);
  if (bits[17])
    ar & make_nvp("mpeDiscrimCalib", mpeDiscrimCalib_);
  if (bits[18])
    ar & make_nvp("pmtDiscrimCalib", pmtDiscrimCalib_);
  if (bits[19])
    ar & make_nvp("relativeDomEff", relativeDomEff_);
  if (bits[20])
    ar & make_nvp("noiseRate", noiseRate_);
  if (bits[21])
    ar & make_nvp("noiseThermalRate", noiseThermalRate_);
  if (bits[22])
    ar & make_nvp("noiseDecayRate", noiseDecayRate_);
  if (bits[23])
    ar & make_nvp("noiseScintillationMean", noiseScintillationMean_);
  if (bits[24])
    ar & make_nvp("noiseScintillationSigma", noiseScintillationSigma_);
  if (bits[25])
    ar & make_nvp("noiseScintillationHits", noiseScintillationHits_);
  if (bits[26])
    ar & make_nvp("combinedSPEFit", combinedSPEFit_);
  if (bits[27])
    ar & make_nvp("meanATWDCharge", meanATWDCharge_);
  if (bits[28])
    ar & make_nvp("meanFADCCharge", meanFADCCharge_);
}

std::ostream&
operator<<(std::ostream& oss, const I3DOMCalibrationDiff& m)
{
  oss << "[ I3DOMCalibrationDiff  ::\n";

  if (m.bits[0])
    oss << "  droopTimeConstants: (" << m.droopTimeConstants_[0]
        << ", " << m.droopTimeConstants_[1] << ")\n";
  if (m.bits[1])
    oss << "  temperature: " << m.temperature_ << "\n";
  if (m.bits[2])
    oss << "  fadcGain: " << m.fadcGain_ << "\n";
  if (m.bits[3])
    oss << "  fadcBaselineFit: " << m.fadcBaselineFit_ << "\n";
  if (m.bits[4])
    oss << "  fadcBeaconBaseline: " << m.fadcBeaconBaseline_ << "\n";
  if (m.bits[5])
    oss << "  fadcDeltaT: " << m.fadcDeltaT_ << "\n";
  if (m.bits[6])
    oss << "  frontEndImpedance: " << m.frontEndImpedance_ << "\n";
  if (m.bits[7])
    oss << "  tauparameters: " << m.tauparameters_ << "\n";
  if (m.bits[8])
    oss << "  ampGains: (" << m.ampGains_[0] << ", "
        << m.ampGains_[1] << ", "
        << m.ampGains_[2] << ")\n";
  if (m.bits[9])
    oss << "  atwdFreq: (" << m.atwdFreq_[0] << ", "
        << m.atwdFreq_[1] << ")\n";
  if (m.bits[10])
    oss << "  atwdBins\n";
  if (m.bits[11])
    oss << "  pmtTransitTime: " << m.pmtTransitTime_ << "\n";
  if (m.bits[12])
    oss << "  hvGainRelation: " << m.hvGainRelation_ << "\n";
  if (m.bits[13])
    oss << "  domcalVersion: " << m.domcalVersion_ << "\n";
  if (m.bits[14])
    oss << "  atwdBeaconBaselines\n";
  if (m.bits[15])
    oss << "  atwdDeltaT: (" << m.atwdDeltaT_[0] << ", "
        << m.atwdDeltaT_[1] << ")\n";
  if (m.bits[16])
    oss << "  speDiscrimCalib: " << m.speDiscrimCalib_ << "\n";
  if (m.bits[17])
    oss << "  mpeDiscrimCalib: " << m.mpeDiscrimCalib_ << "\n";
  if (m.bits[18])
    oss << "  pmtDiscrimCalib: " << m.pmtDiscrimCalib_ << "\n";
  if (m.bits[19])
    oss << "  relativeDomEff: " << m.relativeDomEff_ << "\n";
  if (m.bits[20])
    oss << "  noiseRate: " << m.noiseRate_ << "\n";
  if (m.bits[21])
    oss << "  noiseThermalRate: " << m.noiseThermalRate_ << "\n";
  if (m.bits[22])
    oss << "  noiseDecayRate: " << m.noiseDecayRate_ << "\n";
  if (m.bits[23])
    oss << "  noiseScintillationMean: " << m.noiseScintillationMean_ << "\n";
  if (m.bits[24])
    oss << "  noiseScintillationSigma: " << m.noiseScintillationSigma_ << "\n";
  if (m.bits[25])
    oss << "  noiseScintillationHits: " << m.noiseScintillationHits_ << "\n";
  if (m.bits[26])
    oss << "  combinedSPEFit: " << m.combinedSPEFit_ << "\n";
  if (m.bits[27])
    oss << "  meanATWDCharge: " << m.meanATWDCharge_ << "\n";
  if (m.bits[28])
    oss << "  meanFADCCharge: " << m.meanFADCCharge_ << "\n";

  oss << "]\n";

  return oss;
}

I3_SERIALIZABLE(I3DOMCalibrationDiff);

I3_SERIALIZABLE(I3DOMCalibrationMapDiff);
