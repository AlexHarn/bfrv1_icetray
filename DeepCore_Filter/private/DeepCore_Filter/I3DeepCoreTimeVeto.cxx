// $Id$

#include "DeepCore_Filter/I3DeepCoreTimeVeto.h"

/* ******************************************************************** */
/* I3DeepCoreTimeVeto Constructor                                       
 *//******************************************************************* */
template <class Response>
I3DeepCoreTimeVeto<Response>::I3DeepCoreTimeVeto(const I3Context& ctx) : I3ConditionalModule(ctx)
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

  // Name the veto bool that will be pushed to the frame
  optDecisionName_ = "Veto";
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
  AddParameter("ParticleName", "Name of the I3Particle containing the CoG, time used for the veto. If blank, will not add particle to frame. DeepCore will append _dc, IceCube will append _ic",
	       optParticleName_);

  optTimeThreshold_ = 500*I3Units::ns;
  AddParameter("TimeThreshold", "How much earlier(in ns) the IceCube vertex time is allowed to be than the DeepCore. Default to 500 ns.",
	       optTimeThreshold_*I3Units::ns);

  log_debug("Exiting I3DeepCoreTimeVeto<Response> constructor.");
} //end I3DeepCoreTimeVeto<Response> constructor




/* ******************************************************************** */
/* I3DeepCoreTimeVeto<Response> Destructor                              
 *//******************************************************************* */
template <class Response>
I3DeepCoreTimeVeto<Response>::~I3DeepCoreTimeVeto()
{
}


/* ******************************************************************** */
/* I3DeepCoreTimeVeto<Response> Configure                               */   
/**
 * \param InputDeepCoreHitSeries The name of the hit series containing DeepCore hits.
 * \param InputIceCubeHitSeries The name of the hit series containing IceCube hits.
 * \param DecisionName The name to be given to the final decision in the frame.
 * \param ParticleName The name to be given to the I3Particles holding the 
 *                     centers of gravity and associated times. The DeepCore
 *                     CoG will use this name with "_dc" appended. The
 *                     IceCube CoG will append "_ic". If blank, the
 *                     particles will not be written to the frame. 
 * \param FirstHitOnly Use only the first hit on each DOM instead of all hits.
 *                     Set this to true for the old behavior of the DeepCoreTimeVeto.
 * \param TimeThreshold The amount of time that the IceCube CoG may be earlier
 *                      than the DeepCore CoG to still return a true result.
 *//******************************************************************* */
template <class Response>
void I3DeepCoreTimeVeto<Response>::Configure()
{
  // Get the parameter values the user set in the steering file.
  GetParameter("InputFiducialHitSeries",optFiducialHitSeries_);
  GetParameter("InputVetoHitSeries",optVetoHitSeries_);
  GetParameter("DecisionName",optDecisionName_);
  GetParameter("ParticleName", optParticleName_);
  GetParameter("TimeThreshold", optTimeThreshold_);
  GetParameter("FirstHitOnly", optFirstHitOnly_);
  GetParameter("ChargeWeightCoG", optChargeWeightCoG_);
  
  log_info ("Input: FiducialHitSeries=%s",optFiducialHitSeries_.c_str());
  log_info ("Input: VetoHitSeries=%s",optVetoHitSeries_.c_str());
  log_info ("Input: DecisionName=%s",optDecisionName_.c_str());
  log_info ("Input: ParticleName=%s", optParticleName_.c_str());
  log_info ("Input: TimeThreshold=%f ns", optTimeThreshold_);
  log_info ("Input: FirstHitOnly=%i", optFirstHitOnly_);
  log_info ("Input: ChargeWeightCoG=%i", optChargeWeightCoG_);
}




/* ******************************************************************** */ 
/* I3DeepCoreTimeVeto<Response> Physics:                                */
/** Determine if the frame contains an event that triggered deep core.   
 *   If it did, calculate the center of gravity position and average      
 *   corrected time for the DeepCore and IceCube hits. If the average
 *   corrected time of IceCube hits is earlier than the DeepCore 
 *   corrected time by the amount of time specified in TimeThreshold,
 *   reject the event (ie, return false).
 *   Finally, output the appropriate information and push the frame.
 *
 * \param frame The frame to be processed
 *//******************************************************************* */ 
template <class Response>
void I3DeepCoreTimeVeto<Response>::Physics(I3FramePtr frame)
{
  log_debug("Entering I3DeepCoreTimeVeto<Response>::Physics()... ");
  
  // Get geometry from frame.
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
  if (DCHitMap->size() == 0){
    log_warn("No hits in DeepCore hit series. Are you running SMT3?");
    endProcessing(frame, false);
    return;
  }
  
  // Check to make sure that there are actually hits in IceCube. If not, we have a winner
  if (ICHitMap->size() == 0){
    log_trace("No hits in IceCube hit series. Passing event");
    endProcessing(frame, true);
    return;
  }
  
  //-------------------------------------------------------
  // Get the DeepCore hits, CoG, time we want to use
  //-------------------------------------------------------
  double StdDev = 0;
  I3Particle* DCcog = new I3Particle();
  I3Map<OMKey, std::vector<Response> > reducedHits;
  
  // First pass: Get the average hit time, CoG in DeepCore
  DeepCoreFunctions<Response>::GetCoG(geometry, DCHitMap, DCcog, optFirstHitOnly_, optChargeWeightCoG_);
  DeepCoreFunctions<Response>::GetAverageTime(geometry, DCHitMap, DCcog, StdDev, optFirstHitOnly_, optFiducialHitSeries_);
  
  // Remove any hits beyond 1 StdDev
  DeepCoreFunctions<Response>::ReduceHits(geometry, DCHitMap, DCcog, StdDev, reducedHits, optFirstHitOnly_);
  
  boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > DCReducedHitsPtr(new const I3Map<OMKey, std::vector<Response> >(reducedHits));
  
  // Make sure we have hits left. If not, the hits we had were probably noise...?
  if (DCReducedHitsPtr->size() == 0){
    log_debug("Hits in DeepCore hit series appear scattered. Maybe a lot of noise?");
    endProcessing(frame, false);
    return;
  }
  
  // Second pass: Get CoG position, time in DeepCore
  DeepCoreFunctions<Response>::GetCoG(geometry, DCReducedHitsPtr, DCcog, optFirstHitOnly_, optChargeWeightCoG_);
  DeepCoreFunctions<Response>::GetCoGTime(geometry, DCReducedHitsPtr, DCcog, optFirstHitOnly_);
  
  //-------------------------------------------------------
  // Get the IceCube hits, CoG, time we want to use
  //-------------------------------------------------------
  StdDev = 0;
  I3Particle* ICcog = new I3Particle();
  reducedHits.clear();
  
  // First pass: Get the average hit time, CoG in IceCube
  DeepCoreFunctions<Response>::GetCoG(geometry, ICHitMap, ICcog, optFirstHitOnly_, optChargeWeightCoG_);
  DeepCoreFunctions<Response>::GetAverageTime(geometry, ICHitMap, ICcog, StdDev, optFirstHitOnly_, optFiducialHitSeries_);
  
  // Remove any hits beyond 1 StdDev
  DeepCoreFunctions<Response>::ReduceHits(geometry, ICHitMap, ICcog, StdDev, reducedHits, optFirstHitOnly_);
  
  boost::shared_ptr<const I3Map<OMKey, std::vector<Response> > > ICReducedHitsPtr(new const I3Map<OMKey, std::vector<Response> >(reducedHits));
  
  // Make sure we have hits left. If not, the hits we had were probably noise...?
  if (ICReducedHitsPtr->size() == 0){
    log_debug("Hits in IceCube hit series appear scattered. Maybe a lot of noise?");
    endProcessing(frame, true);
    return;
  }
  
  // Second pass: Get CoG position, time in IceCube
  DeepCoreFunctions<Response>::GetCoG(geometry, ICReducedHitsPtr, ICcog, optFirstHitOnly_, optChargeWeightCoG_);
  DeepCoreFunctions<Response>::GetCoGTime(geometry, ICReducedHitsPtr, ICcog, optFirstHitOnly_);
  
  //------------------------------------------------------
  // Compare the hit times to check the veto condition
  //------------------------------------------------------
  bool veto;
  double delta_time = DCcog->GetTime() - ICcog->GetTime();
  
  if (delta_time < optTimeThreshold_)
    veto = true;
  else
    veto = false;
  
  
  //-------------------------------------------------------
  // Push everything to the frame and say goodbye
  //-------------------------------------------------------
  
  if (optParticleName_.size()){ 
    I3ParticlePtr dccog_ptr(DCcog);
    I3ParticlePtr iccog_ptr(ICcog);
    frame->Put( optParticleName_ + "_dc", dccog_ptr);
    frame->Put( optParticleName_ + "_ic", iccog_ptr);
  }
  endProcessing(frame, veto);
  return;
  
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
void I3DeepCoreTimeVeto<Response>::endProcessing(I3FramePtr frame, bool veto){
  I3BoolPtr veto_ptr(new I3Bool(veto));
  frame->Put( optDecisionName_, veto_ptr);
  
  PushFrame(frame,"OutBox");
  
  log_debug("Exiting DeepCoreVeto::Physics().");
  return;
}


// Register the module's existence with the framework.
I3_MODULE(I3DeepCoreTimeVeto<I3DOMLaunch>);
I3_MODULE(I3DeepCoreTimeVeto<I3RecoPulse>);


