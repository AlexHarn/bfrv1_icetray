/**
 * class: NoiseEngine
 * @version $Id: $
 * @date: $Date: $
 * @author Michael Larson <mjlarson@crimson.ua.edu>
 * (c) 2011,2012 IceCube Collaboration
 *
 * The NoiseEngine filter module is designed to identify triggers caused by 
 * random detector noise. It does this by using the TrackEngine algorithm,
 * which maps all possible pairs of hits with apparent velocities inside of
 * a given window and maximum time separation onto a binned Healpix unit sphere.
 * If more than some threshold number of pairs land in a single bin, then 
 * the event passes.
 * Thus, the filter looks for any hint of directionality. All parameters are
 * designed to be intentionally easy for any physics events to pass, which
 * results in a dramatic increase in data/MC agreement at very low NCh.
 * For more information and results from IC79, see the Berkeley talk here:
 *  https://events.icecube.wisc.edu/contributionDisplay.py?sessionId=32&contribId=114&confId=43
 */

#include "NoiseEngine/NoiseEngine.h"

/*  ******************************************************************** */
/** NoiseEngine Constructor                                           
 *//******************************************************************** */
NoiseEngine::NoiseEngine(const I3Context& ctx) : I3ConditionalModule(ctx)
{
  optStartVelocity_ = 0.1 * (I3Units::m/I3Units::ns);
  AddParameter("StartVelocity", "The minimum implied speed (m/ns) for each pair of hits (Default: 0.1 m/ns)", optStartVelocity_);
  
  optEndVelocity_ = 1 * (I3Units::m/I3Units::ns);
  AddParameter("EndVelocity", "The maximum implied speed (m/ns) for each pair of hits (Default: 1. m/ns)", optEndVelocity_);

  optTimeWindowLength_ = 750 * I3Units::ns;
  AddParameter("TimeWindowLength", "The length of the sliding time window use to choose pairs", optTimeWindowLength_);
    
  optHealpixOrder_ = 1;
  AddParameter("HealpixOrder", "The order of the Healpix map used. 0 = 12 bins, 1 = 48 bins, 2 = 192 bins, 3 = 768 bins.", optHealpixOrder_);

  optNChLimit_ = 20;
  AddParameter("NChLimit", "The maximum number of hit DOMs to process. If more DOMs are hit than this, the event automatically passes to avoid wasting processing time.", optNChLimit_);
  
  optThreshold_ = 3;
  AddParameter("Threshold", "The number of hit pairs needed in one bin to pass the trigger.", optThreshold_);
  
  optHitSeriesName_ = "CleanInIceRawData";
  AddParameter("HitSeriesName", "The name of the hit series to process in the frame.", optHitSeriesName_);

  optOutputName_ = "NoiseEngine";
  AddParameter("OutputName", "The name of the boolean decision to be pushed to the frame", optOutputName_);

  optBinsName_ = "";
  AddParameter("BinsName", "The name of the I3Vector containing the Healpix bin contents to be pushed to the frame. If blank, bin contents will not be pushed to the frame", optBinsName_);

  optVelocityName_ = "";
  AddParameter("VelocityName", "The name of the I3Vector containing the velocities of selected pairs to be pushed to the frame. If blank, velocities will not be pushed to the frame", optVelocityName_);

  optTimerName_ = "";
  AddParameter("TimerName", "The name of the I3Double to push to the frame holding the amount of time (for testing purposes).", optTimerName_);

  optChargeWeight_ = true;
  AddParameter("ChargeWeight", "If using I3RecoPulses, would you like to weight each pair by their average charge?", optChargeWeight_);
  
}

/*  ******************************************************************** */
/** NoiseEngine Destructor
 *//******************************************************************** */
NoiseEngine::~NoiseEngine()
{
}

/* ******************************************************************** */
/** NoiseEngine Configure
 *
 * \param ChargeWeight Apply charge-weighting for reco pulse pairs of hits?
 *                     At the moment, uses the average charge of the two hits.
 * \param HealpixOrder The order of the Healpix map used. 
 *                     0 = 12 bins, 1 = 48 bins, 2 = 192 bins, 3 = 768 bins.
 * \param HitSeriesName Name of the DOMLaunch/RecoPulse series to use.
 * \param NChLimit To save on processing time, the user can set the maximum
 *                     nchannel to be processed. Defaults to 20.
 * \param OutputName The name of the I3Bool decision to be pushed to the frame.
 * \param StartVelocity The minimum apparent velocity between hits to use.
 *                     This is intentionally low (0.1 m/ns) to keep as many
 *                     physics events as possible.
 * \param EndVelocity The maximum apparent velocity between hits to use.
 *                     This is intentionally high (1.0 m/ns) to keep as many
 *                     physics events as possible.
 * \param TimeWindowLength The size of the sliding time window to use
 *                     The time window maximizes the number of hits 
 * \param Threshold The minimum number of pairs mapped to one healpix bin
 *                     to pass this filter. 
 * \param TimerName If given, the name of an I3Double put into the frame
 *                  holding the amount of processing time taken for each
 *                  even. Included for testing purposes.
 *//******************************************************************* */
void NoiseEngine::Configure()
{
  GetParameter("StartVelocity", optStartVelocity_);
  GetParameter("EndVelocity", optEndVelocity_);
  GetParameter("TimeWindowLength", optTimeWindowLength_);
  GetParameter("HealpixOrder", optHealpixOrder_);
  GetParameter("NChLimit",   optNChLimit_);
  GetParameter("Threshold", optThreshold_);
  GetParameter("HitSeriesName", optHitSeriesName_);
  GetParameter("OutputName", optOutputName_);
  GetParameter("TimerName", optTimerName_);
  GetParameter("ChargeWeight", optChargeWeight_);
  GetParameter("BinsName", optBinsName_);
  GetParameter("VelocityName", optVelocityName_);
}


/* ******************************************************************** */ 
/* NoiseEngine::Physics                                                 */
/** Find the time window that maximizes the number of hits, then produce
 * a map of all possible hit pairs that satisfy the time window and velocity
 * window requirements. Map those pairs to a healpix map and check whether
 * any of the healpix bins have more pairs than threshold. If so, then the 
 * event passes. Otherwise, the event fails.
 *
 * \param frame The frame that will be processed.
 *//******************************************************************* */ 
void NoiseEngine::Physics(I3FramePtr frame)
{
  log_trace("Entering NoiseEngine::Process()... ");

  // Start the timer
  timeval startTimer, endTimer;
  double elapsedTime;
  gettimeofday(&startTimer, NULL);

  // Geometry first
  if (! frame->Has("I3Geometry") ) 
    log_fatal("No I3Geometry available!");
  const I3Geometry& geometry = frame->Get<I3Geometry>();

  // Get the hit series
  if (! frame->Has(optHitSeriesName_))
    {
      log_warn("No hitSeriesMap with the name %s is present in the frame!",
	       optHitSeriesName_.c_str());
      PushFrame(frame, "OutBox");
      return;
    }
  I3RecoPulseSeriesMapConstPtr HitMap = frame->Get<I3RecoPulseSeriesMapConstPtr>(optHitSeriesName_);

  bool trigger = false;
  std::vector<double> binEntries, velocities;
  // Check the size of the hit series to compare against optNChLimit_.
  if ( static_cast<int>( HitMap->size() ) > optNChLimit_){
    log_debug("The hit series contains %i hits compared to the limit of %i hits. Trigger is automatically true.", static_cast<int>(HitMap->size()), optNChLimit_);
    trigger = true;
  }
  else
    {
      // Find the start time with the maximum number of hits in the window
      double startTime = PickStartTime(HitMap);
      
      // Catch the event if there are no hits...
      if (startTime != -1){
	// Check all of the possible hit pairs with this window length
	std::vector<HitPair> allPairs = GetHitPairs(geometry,
						    HitMap,
						    startTime,
						    velocities);
	
	// Create the necessary variables for each time window
	Healpix_Base hp_base(optHealpixOrder_, RING);
	
	binEntries = CheckTrigger(allPairs,
				  hp_base,
				  optThreshold_,
				  trigger);
	
      } // end catch for no hits
    } // end check against optNChLimit_
  
  //----------------------------------
  // Start pushing things to the frame
  //----------------------------------
  // Check whether the bins should be pushed. If so, then do it
  if (optBinsName_.size()){
    I3VectorDoublePtr binsPtr( new I3VectorDouble(binEntries.begin(),
						  binEntries.end() ) );
    frame->Put(optBinsName_, binsPtr);
  }

  // Check whether the bins should be pushed. If so, then do it
  if (optVelocityName_.size()){
    I3VectorDoublePtr velocitiesPtr( new I3VectorDouble(velocities.begin(),
						  velocities.end() ) );
    frame->Put(optVelocityName_, velocitiesPtr);
  }

  // Push the result to the frame.
  I3Bool *trigger_decision = new I3Bool(trigger);
  I3BoolPtr trigger_ptr(trigger_decision);
  frame->Put(optOutputName_, trigger_ptr);

  // Push the processing time to the frame.  
  gettimeofday(&endTimer, NULL);
  elapsedTime = (endTimer.tv_sec - startTimer.tv_sec);
  elapsedTime += (endTimer.tv_usec - startTimer.tv_usec) / 1.0e6;

  if (optTimerName_.size()){
    I3DoublePtr timer_ptr(new I3Double(elapsedTime));
    frame->Put(optTimerName_, timer_ptr);
  }

  PushFrame(frame, "OutBox");
  log_debug("Exiting NoiseEngine");

}

/* ******************************************************************** */ 
/* NoiseEngine::PickStartTime                                           */
/** Find the maximum number of hits for a given time window and return the 
 * starting time
 *
 * \param hitmap The DOMLaunch/RecoPulse map to process
 *//******************************************************************* */ 
double NoiseEngine::PickStartTime(I3RecoPulseSeriesMapConstPtr hitmap)
{
  log_trace("Entering NoiseEngine::PickStartTime()... ");
  std::vector<double> times;

  // Most interested in multiple DOMs being hit, since we exclude same-DOM hits for the fit.
  I3RecoPulseSeriesMap::const_iterator map_iter;
  for (map_iter = hitmap->begin(); map_iter != hitmap->end(); ++map_iter){
    times.push_back(map_iter->second.begin()->GetTime());
  }
  
  // This is possible with hit cleaning, so better check to make sure...
  if (times.size() == 0)
    return -1;

  sort(times.begin(), times.end());

  std::vector<double> numInWindow(times.size(), 0);
  for (int i=0; i< static_cast<int>(times.size()); ++i){
    double sum = 0;
    for (int j=i; j< static_cast<int>(times.size()); ++j){
      if (times[j] - times[i] > optTimeWindowLength_)
	break;

      sum++;
    }
    numInWindow[i] = sum;
  }

  std::vector<double>::iterator maximum = max_element(numInWindow.begin(), numInWindow.end());
  int index = distance(numInWindow.begin(), maximum); // STL algorithm

  return times[index];
}

/* ******************************************************************** */ 
/* NoiseEngine::GetHitPairs                                             */
/** The meat of the NoiseEngine algorithm. Forms all possible hit pairs
 * given the time window and apparent velocity constraints.
 *
 * \param geometry   The geometry associated with the hitmap
 * \param hitmap     The DOMLaunch/RecoPulse map to process
 * \param startTime  The start of the time window
 * \param velocities The apparent velocity constraints
 *//******************************************************************* */ 
std::vector<HitPair> NoiseEngine::GetHitPairs(const I3Geometry& geometry, 
					      I3RecoPulseSeriesMapConstPtr hitmap, 
					      double startTime,
					      std::vector<double>& velocities)
{
  log_trace("Entering NoiseEngine::GetHitPairs");

  std::vector<HitPair> pairList;
  HitPair pair;

  I3OMGeoMap::const_iterator geo_iter;

  I3RecoPulseSeriesMap::const_iterator map1_iter;
  I3RecoPulseSeries::const_iterator series1_iter;
  I3RecoPulseSeriesMap::const_iterator map2_iter;
  I3RecoPulseSeries::const_iterator series2_iter;
  
  // Check all of the possible hit pairs for this time window.
  // Hits must be within a time window
  // Hits must be within a velocity window
  // Hits must have a defined zenith, azimuth (ie, be on different DOMs)

  // Loop over the first hit
  log_trace("Looping over first hits");
  for (map1_iter = hitmap->begin(); map1_iter != hitmap->end(); ++map1_iter){

    // Get the position of the first hit
    geo_iter = geometry.omgeo.find(map1_iter->first);
    const I3Position& domPos1 = geo_iter->second.position;
    for (series1_iter = map1_iter->second.begin(); series1_iter != map1_iter->second.end(); ++series1_iter){

      // Loop over the second hit
      log_trace("Looping over second hits");
      for (map2_iter = map1_iter; map2_iter != hitmap->end(); ++map2_iter){

	// Get the position of the second hit
	geo_iter = geometry.omgeo.find(map2_iter->first);
	const I3Position& domPos2 = geo_iter->second.position;

	log_trace("Looping over hit series");
	for (series2_iter = map2_iter->second.begin(); series2_iter != map2_iter->second.end(); ++series2_iter){

	  // Want to make sure to point from early to late DOM...
	  if (series1_iter->GetTime() < series2_iter->GetTime()){
	    pair.SetAngles( domPos1, domPos2 );
	    pair.SetTimes(series1_iter->GetTime(), series2_iter->GetTime());
	  }
	  else {
	    pair.SetAngles( domPos2, domPos1 );
	    pair.SetTimes(series2_iter->GetTime(), series1_iter->GetTime());
	  }

	  if (optVelocityName_.size() > 0)
	    velocities.push_back( pair.GetVelocity() );

	  // Check the conditions
	  if (!pair.InTimeWindow(startTime, optTimeWindowLength_)) continue;
	  if (!pair.InVelocityWindow(optStartVelocity_, optEndVelocity_)) continue;
	  if (pair.GetAzimuth() == -1 && pair.GetZenith() == -1) continue;

	  // Charge weight if requested
	  if (optChargeWeight_)
	    pair.SetWeight(series1_iter->GetCharge(), series2_iter->GetCharge());
	  else
	    pair.SetWeight(1,1);

	  pairList.push_back(pair);

	  log_trace("pairList now has %i pairs saved.", static_cast<int>(pairList.size()));
	}
	log_trace("Finished that series2");
      } // end loop over second hit
      log_trace("Finished that map2");
    }
    log_trace("Finished that series1");
  } // end loop over first hit
  log_trace("Finished that map1");
  
  return pairList;
}

//---------------------------------------------------------------------------
// Check the trigger conditions. We have a trigger if there are at least
// threshold hits in one bin.
//---------------------------------------------------------------------------
std::vector<double> NoiseEngine::CheckTrigger(std::vector<HitPair> TWPairs,
					      Healpix_Base hp_base,
					      double threshold,
					      bool& decision
					      )
{
  log_trace("Entering NoiseEngine::CheckTrigger");
  decision = false;
  std::vector<double> binEntries(hp_base.Npix(), 0);
  
  for (int i=0; i<static_cast<int>(TWPairs.size()); ++i){
    int bin = hp_base.ang2pix( pointing(TWPairs[i].GetZenith(), TWPairs[i].GetAzimuth()) );
    binEntries[bin] += TWPairs[i].GetWeight();
  }
  
  for(int i = 0; i < static_cast<int>(binEntries.size()); ++i){
    if(binEntries[i] > threshold) {
      decision = true;
      break;
    }
  }
  
  return binEntries;
}

I3_MODULE(NoiseEngine);
