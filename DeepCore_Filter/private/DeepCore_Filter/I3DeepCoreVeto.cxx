// $Id$

#include "DeepCore_Filter/I3DeepCoreVeto.h"
#include "icetray/I3Int.h"
/*  ******************************************************************** */
/** I3DeepCoreVeto Constructor                                           
 *//******************************************************************** */
template <class Response>
I3DeepCoreVeto<Response>::I3DeepCoreVeto(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  // Tell the framework that we have an outbox...
  AddOutBox("OutBox");
  
  // Deepcore hits - hit information including only the DOMs in the deepcore volume.
  optFiducialHitSeries_ = "";
  AddParameter("InputFiducialHitSeries", "Name of Hit Series to be used for fiducial (DeepCore) hits. Include information for those DOMs inside the Deepcore volume", 
               optFiducialHitSeries_);

  // Icecube hits - hit information including only the DOMs outside deepcore
  optVetoHitSeries_ = "";
  AddParameter("InputVetoHitSeries", "Name of Hit Series to be used for veto hits. Include information for those DOMs outside of Deepcore", 
               optVetoHitSeries_);

  // Name of the object to push to the frame which has the
  // number of hits satisfying the DeepCore Filter. If left
  // blank nothing is pushed to the frame.
  optVetoHitsName_ = "";
  AddParameter("VetoHitsName", "Name of variable to put in the frame which stores the number of hits satisfying the DeepCore Filter.",
               optVetoHitsName_);

  // Name of the object to push to the frame which has the
  // number of photoelectrons (PEs) satisfying the DeepCore Filter. 
  // If left blank nothing is pushed to the frame.
  optVetoChargeName_ = "";
  AddParameter("VetoChargeName", "Name of variable to put in the frame which stores the number of PE satisfying the DeepCore Filter.",
               optVetoChargeName_);
  
  optMinHitsToVeto_ = 1;
  AddParameter("MinHitsToVeto", "Number of acceptable hits within the veto time window. (Strictly) Greater than this will be vetoed", 
               optMinHitsToVeto_);

  // Name the veto bool that will be pushed to the frame
  optDecisionName_ = "DeepCoreVeto";
  AddParameter("DecisionName", 
	       "Name of the veto decision bool which is output to the frame.",
	       optDecisionName_);

  optFirstHitOnly_ = true;
  AddParameter("FirstHitOnly",
	       "Use only the first hit on each DOM instead of looping over all hits. Leave as default to recreate old DeepCoreVeto behaviour",
	       optFirstHitOnly_);

  optChargeWeightCoG_ = false;
  AddParameter("ChargeWeightCoG",
	       "Weight the center of gravity (CoG) calculation by the charge on the DOMs. ",
	       optChargeWeightCoG_);


  // Decide which results should be pushed to the frame
  optParticleName_ = "";
  AddParameter("ParticleName", "Name of the I3Particle containing the CoG, time used for the veto. If blank, will not add particle to frame",
	       optParticleName_);

  log_debug("Exiting I3DeepCoreVeto<Response> constructor.");
} //end I3DeepCoreVeto<Response> constructor




/* ******************************************************************** */
/** I3DeepCoreVeto<Response> Destructor                                  
*//******************************************************************* */
template <class Response>
I3DeepCoreVeto<Response>::~I3DeepCoreVeto()
{
}


/* ******************************************************************** */
/** I3DeepCoreVeto<Response> Configure                                   
 *
 * \param InputFiducialHitSeries The name of the hit series containing DeepCore hits.
 * \param InputVetoHitSeries The name of the hit series containing IceCube hits.
 * \param DecisionName The name to be given to the final decision in the frame.
 * \param ParticleName The name to be given to an I3Particle holding the 
 *                     center of gravity and associated time. If blank, the
 *                     particle will not be written to the frame.
 * \param FirstHitOnly Use only the first hit on each DOM instead of all hits.
 *                     Set this to true for the old behavior of the DeepCoreVeto.
 * \param ChargWeightCoG Weight the hits used int he CoG calculation by the charge.
 *                       Feature extracted charge is subject to change based on
 *                       Feature Extractor and settings used, so beware.
 *//******************************************************************* */
template <class Response>
void I3DeepCoreVeto<Response>::Configure()
{
  // Get the parameter values the user set in the steering file.
  GetParameter("InputFiducialHitSeries",optFiducialHitSeries_);
  GetParameter("InputVetoHitSeries",optVetoHitSeries_);
  GetParameter("MinHitsToVeto",optMinHitsToVeto_);
  GetParameter("DecisionName",optDecisionName_);
  GetParameter("ParticleName", optParticleName_);
  GetParameter("FirstHitOnly", optFirstHitOnly_);
  GetParameter("ChargeWeightCoG", optChargeWeightCoG_);
  GetParameter("VetoChargeName", optVetoChargeName_);
  GetParameter("VetoHitsName", optVetoHitsName_);

  log_info ("Input: FiducialHitSeries=%s",optFiducialHitSeries_.c_str());
  log_info ("Input: VetoHitSeries=%s",optVetoHitSeries_.c_str());
  log_info ("Input: optMinHitsToVeto=%d",optMinHitsToVeto_);
  log_info ("Input: DecisionName=%s",optDecisionName_.c_str());
  log_info ("Input: ParticleName=%s", optParticleName_.c_str());
  log_info ("Input: FirstHitOnly=%i", optFirstHitOnly_);
  log_info ("Input: ChargeWeightCoG=%i", optChargeWeightCoG_);
  log_info ("Input: VetoChargeName=%s", optVetoChargeName_.c_str());
  log_info ("Input: VetoHitsName=%s", optVetoHitsName_.c_str());
}




/* ******************************************************************** */ 
/* I3DeepCoreVeto<Response> Process:                                    */
/** Determine if the frame contains an event that triggered DeepCore.
 *   If it did, calculate the center of gravity position and average
 *   corrected time for the DeepCore hits. Then, using functions from 
 *   I3DeepCoreFunctions, decide if the event should be vetoed or not.
 *   Veto is 'true' if event should be kept, false otherwise.
 *   Finally, output the appropriate information and push the frame.
 *
 * \param frame The frame that will be processed.
 *//******************************************************************* */ 

template <class Response>
void I3DeepCoreVeto<Response>::Physics(I3FramePtr frame)
{
      I3DeepCoreVeto<Response>::ProcessFrame(frame);
}

template <class Response>
void I3DeepCoreVeto<Response>::DAQ(I3FramePtr frame)
{
      I3DeepCoreVeto<Response>::ProcessFrame(frame);
}

template <class Response>
void I3DeepCoreVeto<Response>::ProcessFrame(I3FramePtr frame)
// void I3DeepCoreVeto<Response>::DAQ(I3FramePtr frame)
{
  if (!frame->Has(optFiducialHitSeries_) ||
   !frame->Has(optVetoHitSeries_)) {
    PushFrame(frame);
    return;
  }

  log_debug("Entering I3DeepCoreVeto<Response>::ProcessFrame()... ");
  
  // Get geometry and MCTree from frame.
  const I3Geometry& geometry = frame->Get<I3Geometry>();

  //-------------------------------------------------------
  // Check for and get the hit series
  //-------------------------------------------------------

  // Get the Deepcore hits. Stop processing if nonexistent.
  if (! frame->Has(optFiducialHitSeries_)){
    log_warn("No hit series with name %s is present in the frame! Skipping event", optFiducialHitSeries_.c_str());
    PushFrame(frame,"OutBox");
    return;
  }
  boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > DCHitMap = 
    frame->Get<boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > >(optFiducialHitSeries_);

  // Get the IceCube hits. Stop processing if nonexistent.
  if (! frame->Has(optVetoHitSeries_)){
    log_warn("No hit series with name %s is present in the frame! Skipping event", optVetoHitSeries_.c_str());
    PushFrame(frame,"OutBox");
    log_debug("Exiting DeepCoreVeto::Physics().");
    return;
  }
  boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > ICHitMap = 
    frame->Get<boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > >(optVetoHitSeries_);

  //-------------------------------------------------------
  // Check for hits in each hit series
  //-------------------------------------------------------

  // Check to make sure that there are actually hits in DeepCore. If not, stop processing.
  // if (DCHitMap->size() == 0){
  if (!DCHitMap){
    // if (DCHitMap->size() == 0){
    log_warn("No hits in Fiducial hit series. Are you running SMT3?");
    endProcessing(frame, false);
    return;
  }
  
  // Check to make sure that there are actually hits in IceCube. If not, we have a winner
  // if (ICHitMap->size() == 0){
  if (!ICHitMap){
    log_trace("No hits in Veto hit series. Passing event");
    endProcessing(frame, true);
    return;
  }

  //-------------------------------------------------------
  // Get the hits, CoG, time we want to use
  //-------------------------------------------------------
  double StdDev = 0;
  I3Particle* DCcog = new I3Particle();
  I3Map<OMKey, std::vector<Response> > reducedHits;
  
  // First pass: Get the average hit time in DeepCore
  DeepCoreFunctions<Response>::GetAverageTime(geometry, DCHitMap, DCcog, StdDev, optFirstHitOnly_, optFiducialHitSeries_);

  // Remove any hits beyond 1 StdDev
  DeepCoreFunctions<Response>::ReduceHits(geometry, DCHitMap, DCcog, StdDev, reducedHits, optFirstHitOnly_);

  boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > reducedHitsPtr(new const I3Map<OMKey, std::vector<Response> >(reducedHits));

  // Make sure we have hits left. If not, the hits we had were probably noise...?
  if (reducedHitsPtr->size() == 0){
    log_debug("Hits in DeepCore hit series appear scattered. Maybe a lot of noise?");
    endProcessing(frame, false);
    return;
  }

  // Second pass: Get CoG position, time in DeepCore  
  DeepCoreFunctions<Response>::GetCoG(geometry, reducedHitsPtr, DCcog, optFirstHitOnly_, optChargeWeightCoG_);
  DeepCoreFunctions<Response>::GetCoGTime(geometry, reducedHitsPtr, DCcog, optFirstHitOnly_);

  //-------------------------------------------------------
  // Count the number of hits and PE consistent with speed of light travel
  // If more than the threshold (1 hit), the veto is failed
  //-------------------------------------------------------
  int nVetoWindowHits = 0;
  double nVetoWindowCharge = 0;
  bool veto;

  DeepCoreFunctions<Response>::CountHitsInWindow(geometry, ICHitMap, DCcog, nVetoWindowHits, nVetoWindowCharge, optFirstHitOnly_);

  log_debug("Number of veto window hits: %i", nVetoWindowHits);
  log_debug("Charge of veto window hits: %f", nVetoWindowCharge);
 
  if (nVetoWindowHits > optMinHitsToVeto_)
    veto = false;
  else
    veto = true;

  
  //-------------------------------------------------------
  // Push everything to the frame and say goodbye
  //-------------------------------------------------------
  if (optVetoHitsName_.size()){
    I3IntPtr v_ptr(new I3Int(nVetoWindowHits));
    frame->Put( optVetoHitsName_, v_ptr);
  }

  if (optVetoChargeName_.size()){
    I3DoublePtr v_ptr(new I3Double(nVetoWindowCharge));
    frame->Put( optVetoChargeName_, v_ptr);
  }

  if (optParticleName_.size()){
    I3ParticlePtr p_ptr(DCcog);
    frame->Put( optParticleName_, p_ptr);
  }

  endProcessing(frame, veto);

}// end physics.


/* ******************************************************************** */ 
/* endProcessing                                                        */
/** Create the I3BoolPtr from veto, put the result in the frame, and
 *   push the frame to the outbox.                               
 *
 *  \param frame The frame that is being processed.
 *  \param veto The veto decision, which will be pushed to the frame.
 *//******************************************************************* */ 
template <class Response>
void I3DeepCoreVeto<Response>::endProcessing(I3FramePtr frame, bool veto){
  
  I3BoolPtr veto_ptr(new I3Bool(veto));
  frame->Put( optDecisionName_, veto_ptr);
  
  PushFrame(frame,"OutBox");

  log_debug("Exiting DeepCoreVeto::Physics().");
  return;
}

// Register the module's existence with icetray
I3_MODULE(I3DeepCoreVeto<I3DOMLaunch>);
I3_MODULE(I3DeepCoreVeto<I3RecoPulse>);


