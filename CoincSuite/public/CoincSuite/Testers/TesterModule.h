/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file TesterModule.h
 * @version $Revision$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 * @brief Baseclass definitions for any TesterModule
 */

#ifndef TESTERMODULE_H
#define TESTERMODULE_H

#include "CoincSuite/Modules/FrameCombiner.h"

#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/I3String.h"
#include "icetray/OMKey.h"

#include "icetray/I3Units.h"
#include "dataclasses/I3Constants.h"

/** @brief A module class that provides higher order functions for Tester Modules
 * Derived classes must provide an evaluation on the split/unsplit-hypothesis, which will than be stored in the HypoFrame for later evaluation<BR>
 * Normal mode of operation is this:<BR>
 * -> Configuration: Provide a base-set of configurable Parameters "SplitName", "RecoMapName", which are needed operate the FrameRegister<BR>
 * Expand this parameters for the needs of the specific module<BR>
 * -> FramePacket:<BR>
 * The FramePacket is buffered up in the FrameRegister and access to Q/Split/HypoFrames provided.<BR>
 * Each HypoFrame is looped over and its originators are identified and picked from the subevent-stream<BR>
 * The derived Tester-class must implement a function: Result Evaluate((void*) this, HypoFrame, frameA, frameB), which evaluates the unsplit-hypothesis is to be preferred to the split-hypothesis (as present) with one of the following possibilities:<BR>
 * * evaluation of the SplitFrame A against Splitframe B (MutualTest)<BR>
 * * evaluation of the HypoFrame only (HypoTest)<BR>
 * * evaluation of the SplitFrames A and SplitFrame B against the HypoFrame<BR>
 * the result of the evaluation must be returned as either YES, NO, or UNDECIDED, in case no definite decision could be made, e.g. because keys were missing or the situation is unclear<BR>
 * Put this decision into a map in the HypoFrame which can than be later evaluated and frames can get recombined<BR>
 * Put a map into the Q frame, that stores that the specific Tester module has run<BR>
 * Push the augmented FramePackage back into the processing pipeline<BR>
 */
class TesterModule : public FrameCombiner {
private:
  SET_LOGGER("TesterModule");

private: //bookkeeping
  unsigned int n_YES_;
  unsigned int n_NO_;
  unsigned int n_UNDECIDED_;
  
protected: //ADD NEW FUNCTIONALITY
  /// buffers the geometry for arbitrary purpose
  I3GeometryConstPtr geometry_;
  /// Formalizes the decision of a module
  enum Result{
    UNDECIDED = -1,
    YES = 1,
    NO = 0
  };

  ///Convenience converter function
  inline Result Bool2Result(const bool b) const
    {if (b) return(YES); else return(NO); };

  /** Overwritable function which evaluates the criterion
   *  @param tester pointer to the instance of the tester 
   *  @param hypoframe The HypoFrame pointer
   *  @param frameA the frame A that should be evaluated against frame B and/or against the HypoFrame
   *  @param frameB the frame B that should be evaluated against frame A and/or against the HypoFrame
   */
  Result (*Evaluate) (void *tester,
                      I3FrameConstPtr hypoframe,
                      I3FrameConstPtr frameA,
                      I3FrameConstPtr frameB);

public: // hand all inherited functions upwards
  /// Constructor
  TesterModule(const I3Context& context);
  /// Configure function: Add more Parameters
  void Configure();
  /// just use Finish of I3PacketModule: a must do
  void Finish();
  /// buffer the I3Geometry up in geometry
  void Geometry (I3FramePtr frame);
  
  
  /** ACTION:
   * Check if a recombination attempt makes sense
   * Check for all objects needed to run.
   * Check the criteria
   * Check discrimination/veto
   * Mark frames according to decision
   */
  void FramePacket(std::vector<I3FramePtr> &packet);

protected: //ADD NEW FUNCTIONALITY
  /** @brief Bookkeeping: tell the q frame we attempted this recombinations
   * Put 'RecombinationAttempts' into the Qframe so that it knows this module has run
   * @param moduleName name of this Module (should be fetched by I3Module-method GetName())
   */
  void ModuleMarking (const std::string& moduleName);

  /** @brief Bookkeeping: Put all necessary keys in the split- and HypoFrame to decide about recombinations
   * Put 'RecombinationSuccess' key into the HypoFrame so that it knows about the success or failure of the run Tester
   * @param hypoframe the HypoFrame that was tested
   * @param frameA the first frame the HypoFrame has been tested against
   * @param frameB the second frame the HypoFrame has been tested against
   * @param moduleName name of this Module (should be fetched by I3Module-method GetName())
   * @param success was the recombination test successful?
   */
  void TesterMarking (I3FramePtr hypoframe,
                      I3FramePtr frameA,
                      I3FramePtr frameB,
                      const std::string& moduleName,
                      const bool success);
};

#endif
