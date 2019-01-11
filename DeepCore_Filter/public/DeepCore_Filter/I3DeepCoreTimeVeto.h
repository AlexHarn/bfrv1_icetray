// $Id$

#ifndef I3DEEPCORETIMEVETO_H_INCLUDED
#define I3DEEPCORETIMEVETO_H_INCLUDED

#include "DeepCore_Filter/I3DeepCoreFunctions.h"
#include "icetray/I3ConditionalModule.h"

/**
 * @brief IceTray module to perform an Icecube DeepCore Time Veto
 */
template <class Response>
class I3DeepCoreTimeVeto : public I3ConditionalModule
{
 public:

  /**
   * Constructor: builds an instance of the module, with the
   * context provided by IceTray.
   */
  I3DeepCoreTimeVeto(const I3Context& ctx);

  /**
   * Destructor: deletes the module
   */
  ~I3DeepCoreTimeVeto();

  /**
   * \brief Configure: Grabs options from the python processing script.
   */
  void Configure();

  /**
   * \brief Physics: Process the event and decide the veto status.
   */
  void Physics(I3FramePtr frame);

  /**
   * \brief endProcessing: Push the veto decision to the frame and the frame to the outbox.
   */
  void endProcessing(I3FramePtr frame, bool veto);

private:

  // Parameters and Options
  bool optChargeWeightCoG_;
  bool optFirstHitOnly_;
  std::string optFiducialHitSeries_;
  std::string optVetoHitSeries_;
  std::string optDecisionName_;
  std::string optParticleName_;
  double optTimeThreshold_;

  SET_LOGGER("I3DeepCoreTimeVeto");

};  // end of class I3DeepCoreTimeVeto

#endif
