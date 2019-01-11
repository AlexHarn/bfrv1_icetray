/**
 * copyright (C) 2011
 * the IceCube collaboration
 * $Id$
 *
 * @file TesterModule.cxx
 * @version $Revision: 1.00$
 * @author mzoll <marcel.zoll@fysik.su.se>
 * @date $Date: Nov 30 2011
 */

#include "CoincSuite/Testers/TesterModule.h"

#include "boost/make_shared.hpp"

//_________________________________________________________
TesterModule::TesterModule(const I3Context& context):
  FrameCombiner(context),
  n_YES_(0),
  n_NO_(0),
  n_UNDECIDED_(0)
{};

//______________________________________________________________________________
void TesterModule::Geometry (I3FramePtr frame) {
  geometry_ = frame->Get<I3GeometryConstPtr>("I3Geometry");
  if (!geometry_)
    log_fatal("Unable to find <I3Geometry>(I3Geometry) in Geometry-frame!");

  PushFrame(frame);
};

//______________________________________________________________________________
void TesterModule::Configure() {
  FrameCombiner::Configure();
};

//______________________________________________________________________________
void TesterModule::Finish() {
  FrameCombiner::Finish();
  log_notice_stream(std::endl
  <<"Recombination Decisions by "<<GetName()<<":"<<std::endl
  <<"  YES       : "<< n_YES_ <<std::endl
  <<"  NO        : "<< n_NO_ <<std::endl
  <<"  UNDECIDED : "<< n_UNDECIDED_ <<std::endl);
};

//______________________________________________________________________________
void TesterModule::ModuleMarking (const std::string& moduleName) {
  log_debug("Entering ModuleMarking()");

  // === Mark the Q frame that this Module has been run ===
  I3VectorStringPtr recombAttempts;
  if (FrameRegister_.QFrames_[0].second->Has(splitName_+"RecombAttempts")) {
    recombAttempts= boost::make_shared<I3VectorString>(*FrameRegister_.QFrames_[0].second->Get<I3VectorStringConstPtr>(splitName_+"RecombAttempts"));
    FrameRegister_.QFrames_[0].second->Delete(splitName_+"RecombAttempts");
  }
  else
    recombAttempts = I3VectorStringPtr(new I3VectorString());

  (*recombAttempts).push_back(moduleName);

  FrameRegister_.QFrames_[0].second->Put(splitName_+"RecombAttempts", recombAttempts);
};

//______________________________________________________________________________
void TesterModule::TesterMarking (I3FramePtr hypoframe,
  I3FramePtr frameA,
  I3FramePtr frameB,
  const std::string& moduleName,
  const bool success)
{
  log_debug("Entering TesterMarking()");

  // ===mark the HypoFrame if this very recombination was successful===
  I3MapStringBoolPtr recombSuccess;
  if (hypoframe->Has("CS_RecombSuccess")) {
    recombSuccess=I3MapStringBoolPtr(new I3MapStringBool(*hypoframe->Get<I3MapStringBoolConstPtr>("CS_RecombSuccess")));
    hypoframe->Delete("CS_RecombSuccess");
  }
  else
    recombSuccess= I3MapStringBoolPtr (new I3MapStringBool());

  (*recombSuccess)[moduleName]=success;

  hypoframe->Put("CS_RecombSuccess", recombSuccess);
  return;
};

//_____________________________________________________________________________
void TesterModule::FramePacket(std::vector<I3FramePtr> &packet) {
  log_debug("Entering FramePacket()");

  BuildFrameRegister(packet);
  //test if recombination attempt makes sense
  if (FrameRegister_.GetSplitCount() < 2) {
    log_debug("There is only one or no split; Cannot run; Push everything");
    PushFrameRegister();
    return;
  }
  //Tester can run

  //loop over all HypoFrames and find their originators
  for (uint hypoframe_index=0; hypoframe_index < FrameRegister_.GetNumberHypoFrames(); hypoframe_index++) {
    I3FramePtr hypoframe = FrameRegister_.HypoFrames_[hypoframe_index].second;
    
    I3MapStringVectorDoubleConstPtr created_from = hypoframe->Get<I3MapStringVectorDoubleConstPtr>("CS_CreatedFrom");
    if (not created_from) {
      log_error("Could not find <I3MapStringVectorDouble>('CS_CreatedFrom') in the hypoframe '%s';"
                " will continue with next hypoframe", CoincSuite::FrameIDstring(hypoframe).c_str());
      continue; // to loop over all Hypoframes
    }
    if (created_from->begin()->first != splitName_)
      log_fatal("Configured SplitName does not interlock with the frames the HypoFrame is created from!");

    I3FramePtr frameA = FindCorrIndices(splitName_, (created_from->begin()->second)[0]).second;
    I3FramePtr frameB = FindCorrIndices(splitName_, (created_from->begin()->second)[1]).second;

    if (frameA->Has("CS_ReducedBy") || frameB->Has("CS_ReducedBy")) {
      log_warn("One or both of these SplitFrames were already reduced by this or another HypoFrame");
      //continue; // to loop over all HypoFrames
    }

    const Result result = Evaluate((void*) this, hypoframe, frameA, frameB);
    // final decision
    switch (result) {
      case UNDECIDED: {
        n_UNDECIDED_++;
        log_debug("Result is UNDECIDED: do not do anything with these frames");
        break;
      }
      case YES: {
        n_YES_++;
        log_debug("Result is YES: ready to recombine, mark the frames accordingly");
        TesterMarking (hypoframe, frameA, frameB, GetName(), true);
        break;
      }
      case NO: {
        n_NO_++;
        log_debug("Result is NO: refuse to recombine, mark the frames accordingly");
        TesterMarking (hypoframe, frameA, frameB, GetName(), false);
        break;
      }
      default: {
        log_fatal("Something unforeseen has happened;");
      }
    }
  }

  //finish up
  ModuleMarking(GetName());

  PushFrameRegister();
  log_debug("Exit FramePacket()");
  return;
};
