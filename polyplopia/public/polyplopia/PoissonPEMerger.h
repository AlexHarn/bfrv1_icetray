#ifndef DIPLOPIA_POISSON_H_INLCUDED
#define DIPLOPIA_POISSON_H_INLCUDED

#include <string>
#include <icetray/I3Module.h>
#include <icetray/I3Logging.h>
#include <phys-services/I3RandomService.h>
#include <sim-services/I3GeneratorService.h>

/**
 *
 */
class PoissonPEMerger : public I3Module
{
public:
  /**
   *Constructor
   */
  PoissonPEMerger(const I3Context& ctx);

  /**
   *Destructor
   */
  virtual ~PoissonPEMerger() { }

  /**
   * Configures parameters
   */
  void Configure();

  void Finish();

  /**
   * Caches frames and merges MCTrees and MCInfoTrees 
   */
  void DAQ(I3FramePtr frame);

private:
  ///Source of background events to merge
  boost::shared_ptr<I3GeneratorService> backgroundService_;
  ///Source of random numbers
  boost::shared_ptr<I3RandomService> randomService_;
  ///
  std::string mcTreeName_;
  ///Time within which coincidences are to be injected
  double timeWindow_;
  ///Rate at which background events occur
  double backgroundRate_;
  ///Whether the event stream into which events are being merged is already
  ///air showers, since this affects the merging rate needed
  bool base_is_background_;
  
  std::string mcpesName_;
  std::string photonsName_;
  std::string mmcTrackName_;
  
  enum class MergeType{
    Photon,
    PhotoElectron
  };
  
  MergeType mergeType_;
  
  ///Count of events processed
  unsigned int counter_;
  ///Record of merges made
  I3Map<unsigned int,unsigned int> coincidenceHistogram_;

  SET_LOGGER("PoissonPEMerger");
};

#endif
