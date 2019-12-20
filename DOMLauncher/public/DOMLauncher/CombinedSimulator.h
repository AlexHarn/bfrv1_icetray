#ifndef DOMLAUNCHER_COMBINED_SIMULATOR_H_INCLUDED
#define DOMLAUNCHER_COMBINED_SIMULATOR_H_INCLUDED

// I3 headers
#include "icetray/I3ConditionalModule.h"
#include "icetray/OMKey.h"
#include "sim-services/I3SumGenerator.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "phys-services/I3GSLRandomService.h"
#include "simclasses/I3MCPulse.h"
#include <simclasses/I3MCPE.h>
#include <dataclasses/physics/I3ParticleID.h>
#include <simclasses/I3ParticleIDMap.hpp>

//DOMLauncher headers
#include "domlauncherutils.h"

namespace dlud = domlauncherutils::detail;
/**
 * class: CombinedSimulator (Gyllenstierna)
 *
 * @version $Id: $
 *
 * @date: $Date: $
 *
 * @author Matti Jansson <matti.jansson@fysik.su.se>
 *
 * @brief A module that combines simulation of the detector response
 *   with the PMT response simulator.
 *
 * (c) 2011,2012 IceCube Collaboration
 */
class CombinedSimulator : public I3ConditionalModule{
  public:

  CombinedSimulator(const I3Context&);

  ~CombinedSimulator() {};

  void Configure();
  void DAQ(I3FramePtr);
  void Calibration(I3FramePtr);
  void Geometry(I3FramePtr);
  void DetectorStatus(I3FramePtr);

 private:
  ///The name of the I3MCPESeriesMap to process.
  std::string inputHitsName_;
  ///Whether the charges of hits should be randomized
  bool useSPEDistribution_;
  ///Whether the times of hits should be randomly perturbed
  bool usePMTJitter_;
  ///The probability that a pulse arrives early
  double prePulseProbability_;
  ///The probability that a pulse arrives late
  double latePulseProbability_;
  ///The probability that a pulse produces an accompanying afterpulse
  double afterPulseProbability_;
  ///Whether the weights of the hits should be modified to account for saturation
  bool applySaturation_;
  ///Whether hits very close in time should be merged
  bool mergeHits_;
  ///Whether to merge hits repeatedly during generation to attempt to limit transient memory use
  bool lowMem_;
    ///The random service fetched from the tray
  boost::shared_ptr<I3RandomService> randomService_;
  ///A generic distribution of charges produced by single photon hits
  boost::shared_ptr<I3SumGenerator> genericChargeDistribution_;
  ///The charge distributions to be used for each DOM for single photon hits
  std::map<OMKey, boost::shared_ptr<I3SumGenerator> > chargeDistributions_;

  ///The most recently seen calibration data
  boost::shared_ptr<const I3Calibration> lastCalibration_;

  /// Extracts Discriminator Over Threshold (DOT) times from triggerStream_ and
  /// puts in Frame as an MCPulseSeriesMap.
  void DOTOutput(I3FramePtr, const domlauncherutils::DCStream& );
  void InitilizeDOMMap();

  /// Is DOM Map initialized
  bool domMapInitialized_;

  /// SuperNova Mode :
  /// Outputs discriminator over threshold (DOT) times
  bool snMode_;

  ///Does not reset the DOMs with each new frame.
  bool multiFrameEvents_;

  /// Pulse templates used in simulation are interpolated for better speed
  /// with the cost of worse precision and larger memory consumption.
  bool tabulatePTs_;

  /// If pulses in the past should be merged. Speeds up considerably if the
  /// number of pulses to process per DOM and frame is large.
  bool mergePulses_;

  /// If drooped SPE pulse templates should be used.
  bool droopedPulseTemplates_;
  
  /// If beacon launches should be simulated.
  bool beaconLaunches_;
  /// Rate of beacon launches.
  double beaconLaunchRate_;
  
  /// The name of the DOMLaunchSeriesMaps to be produced.
  std::string domLaunchMapName_;

  /// The name of the MCPulseSeriesMap to be processed.
  std::string mcPulseSeriesName_;

  I3Map<OMKey, I3OMGeo> domGeo_;
  std::map<OMKey, I3DOMCalibration> domCal_;
  std::map<OMKey, I3DOMStatus> domStatus_;

  std::map<OMKey, boost::shared_ptr<I3DOM> > domMap_;
  
  std::map<OMKey, boost::shared_ptr<I3DOM> > activeDOMsMap_;
  
  domlauncherutils::I3DOMGlobals globalSimState;
  
  ///The ID of the random service to use
  std::string randomServiceName_;
    
  ///Computes the amount of charge, relative to the charge produced by an ideal, single
  ///  photoelectron, produced by a hit whose weight is an integer number of initial
  ///  photoelectrons
  ///
  ///\param w The weight of the hit in photons
  ///\param speDistribution The amplification distribution from which to sample 
  ///\return  The amount of charge produced by the PMT for this hit, in units of the ideal charge
  ///         produced by a single photoelectron
  double normalHitWeight(unsigned int w, const boost::shared_ptr<I3SumGenerator>& speDistribution);

  ///Generates a random time jitter value for a hit
  ///\return A random time offset, in nanoseconds
  double PMTJitter();

  ///Computes the amount by which prepulses are early at a given voltage
  ///\param voltage The operating voltage of the DOM
  ///\return A time offset, in nanoseconds
  double prePulseTimeShift(double voltage);

  ///Computes the charge of a prepulse, relative to an ideal s.p.e. at a given voltage
  ///\param voltage The operating voltage of the DOM
  ///\return The amount of charge produced by the PMT for a prepulse, in units of the ideal
  ///        charge produced by a single photoelectron
  double prePulseWeight(double voltage);

  ///Computes the charge of an early afterpulse, relative to an ideal s.p.e. at a given voltage
  ///\return The amount of charge produced by the PMT for an early afterpulse, in units of the ideal
  ///        charge produced by a single photoelectron
  double earlyAfterPulseWeight();

  ///Alters the properies of the given hit to describe a late pulse
  ///\param hit The existing raw hit to be turned into a late pulse hit
  ///\param speDistribution The speDistribution for the DOM on which the hit is being detected
  ///\param voltage The operating voltage of the PMT
  void createLatePulse(I3MCPulse& hit, const boost::shared_ptr<I3SumGenerator>& speDistribution, double voltage);

  ///Alters the properies of the given hit to describe an afterpulse
  ///\param hit The existing raw hit to be turned into an afterpulse hit
  ///\param speDistribution The speDistribution for the DOM on which the hit is being detected
  ///\param voltage The operating voltage of the PMT
  void createAfterPulse(I3MCPulse& hit, const boost::shared_ptr<I3SumGenerator>& speDistribution, double voltage);

  ///Generate a Fisher-Tippet distributed (alias Gumbel distributed)  random variate
  ///\param location The location parameter of the distribution from which to sample
  ///                 (mean = location + scale * euler_mascheroni)
  ///\param scale The scale parameter of the distribution from which to sample
  ///              (variance = (pi * scale)**2 / 6)
  ///\param logLowerBound Indirectly determines the lower cutoff of the distribution
  ///\param logUpperBound Indirectly determines the upper cutoff of the distribution
  double fisherTippett(double location, double scale, double logLowerBound, double logUpperBound);

  SET_LOGGER("DOMLauncher");
public:
  ///Applies all transformations to a set of hits on a single DOM
  ///\param inputHits The raw hits on the DOM
  ///\param dom The DOM on which the hits occur
  ///\param cal The current calibration information for this DOM
  ///\param status The current status information for this DOM
  ///\return A new set of time ordered hits with weighting applied, alternate
  ///pulse types generated, saturation applied, and time merging applied
  std::pair<std::vector<I3MCPulse>, ParticlePulseIndexMap>
  processHits(const std::vector<I3MCPE>& inputHits, OMKey dom,
              const I3DOMCalibration& cal, const I3DOMStatus& status,
              const ParticlePulseIndexMap& pePedigree);

  ///Reweights the hits in the given series to mimic the effects of saturation in the PMT.
  ///  The 'inverse' saturation parameterization from T. Waldenmeier is used.
  ///\param hits The hit series to be reweighted
  ///\param pmtVoltage The operating voltage of the PMT
  ///\param cal The calibration data for the DOM
  ///\pre The input hits must be time ordered
  void saturate(std::vector<I3MCPulse>& hits, double pmtVoltage, const I3DOMCalibration& cal);

  ///Merges together pulses which are within some small time window, compacting the hit series
  ///  if it is densely filled with hits.
  ///  (Here, 'small' is intended to be relative to the resolution of the feature extractor.)
  ///\param hits The hit series to be compacted
  ///\pre The input hits must be time ordered
  static void timeMergeHits(std::vector<I3MCPE>& hits);
  static void timeMergeHits(std::vector<I3MCPulse>& hits, ParticlePulseIndexMap& aux);

  //Simple accessors and mutators for using this object directly, rather than as part of a tray

  std::string getInputHitsName() const{ return(inputHitsName_); }
  void setInputHitsName(const std::string& in){ inputHitsName_=in; }

  bool getUseSPEDistribution() const{ return(useSPEDistribution_); }
  void setUseSPEDistribution(bool use){ useSPEDistribution_=use; }

  bool getUseJitter() const{ return(usePMTJitter_); }
  void setUseJitter(bool use){ usePMTJitter_=use; }

  double getPrePulseProbability() const{ return(prePulseProbability_); }
  void setPrePulseProbability(double prob){ prePulseProbability_=prob; }

  double getLatePulseProbability() const{ return(latePulseProbability_); }
  void setLatePulseProbability(double prob){ latePulseProbability_=prob; }

  double getAfterPulseProbability() const{ return(afterPulseProbability_); }
  void setAfterPulseProbability(double prob){ afterPulseProbability_=prob; }

  bool getApplySaturation() const{ return(applySaturation_); }
  void setApplySaturation(bool apply){ applySaturation_=apply; }

  bool getMergeHits() const{ return(mergeHits_); }
  void setMergeHits(bool merge){ mergeHits_=merge; }

  bool getLowMem() const{ return(lowMem_); }
  void setLowMem(bool lowMem){ lowMem_=lowMem; }

  boost::shared_ptr<I3RandomService> getRandomService() const{ return(randomService_); }
  void setRandomService(boost::shared_ptr<I3RandomService> randomService){ randomService_=randomService; }

  friend class CombinedSimulatorTestSetup;
};

#endif
