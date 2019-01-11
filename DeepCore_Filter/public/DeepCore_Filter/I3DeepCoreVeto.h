// $Id$

#ifndef I3DEEPCOREVETO_H_INCLUDED
#define I3DEEPCOREVETO_H_INCLUDED

#include <vector>

#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "icetray/I3ConditionalModule.h"

/**
 * @brief IceTray module to perform an Icecube DeepCore Veto
 */
template <class Response>
class I3DeepCoreVeto : public I3ConditionalModule
{
 public:

  /**
   * \brief Constructor:  builds an instance of the module, with the
   *         context provided by IceTray.
   */
  I3DeepCoreVeto(const I3Context& ctx);

  /**
   * \brief Destructor: deletes the module
   */
  ~I3DeepCoreVeto();

  /**
   * \brief Configure: Grabs options from the python processing script.
   */
  void Configure();

  /**
   * \brief Physics: Process the event if the input series are in the physics frame
   */
  void Physics(I3FramePtr frame);

  /**
   * \brief DAQ: Process the event if the input series are in the DAQ frame
   */
  void DAQ(I3FramePtr frame);

  /**
   * \brief Process: Process the event and decide the veto status.
   */
  void ProcessFrame(I3FramePtr frame);

  /**
   * \brief endProcessing: Push the veto decision to the frame and the frame to the outbox.
   */
  void endProcessing(I3FramePtr frame, bool veto);

private:

  // Parameters and Options
  bool optChargeWeightCoG_;
  bool optFirstHitOnly_;
  std::string optTreeName_;
  std::string optFiducialHitSeries_;
  std::string optVetoChargeName_;
  std::string optVetoHitSeries_;
  std::string optVetoHitsName_;
  int optMinHitsToVeto_;
  std::string optDecisionName_;
  std::string optParticleName_;

  SET_LOGGER("I3DeepCoreVeto");

};  // end of class I3DeepCoreVeto

#endif
