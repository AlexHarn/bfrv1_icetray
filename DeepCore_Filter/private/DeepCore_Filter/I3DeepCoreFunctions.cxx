
#include "DeepCore_Filter/I3DeepCoreFunctions.h"

// $Id$


/* *********************************************************
 * GetAverageTime                                         */
/** Calculate the average time of the hit series. This is a 
 *   regular average with no time corrections. A standard
 *   deviation is also calculated and stored in StdDev.
 *
 * \f[ \frac{\sum_{i=0}^{N_{evts}}{t_i}}{N_{evts}} \f]
 *
 * \param geometry The geometry holding the OMKey and DOM positions.
 * \param HitMap Either an I3RecoPulseSeriesMapConstPtr or 
 *               I3DOMLaunchSeriesMapConstPtr containing the 
 *               hits to be processed.
 * \param cog A pointer to an I3Particle. This will be used to 
 *            hold the average time from this calculation.
 * \param StdDev A double used to hold the result of the 
 *               standard deviation calculation.
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::GetAverageTime(const I3Geometry& geometry, 
						boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
						I3Particle* cog,
						double& StdDev,
						bool useFirstOnly,
						std::string Name)
{
  log_debug("Entering GetAverageTime");
  
  int nHits = 0;
  double previousAvgStep = 0.0;
  double previousVarStep = 0.0;
  
  //--------------------------------------------
  //  Compute the average and variance of the first hit times
  //--------------------------------------------
  typename I3Map<OMKey, std::vector<HitType> >::const_iterator MapIter;
  for( MapIter = HitMap->begin(); MapIter != HitMap->end(); ++MapIter)
    {
      //------------------------------------
      // Get the hit series from the map
      //------------------------------------
      const std::vector<HitType> HitSeries = MapIter->second;
      log_trace( "HitSeries size: %zu", HitSeries.size() );
      
      // Check that the series is not empty...
      if( HitSeries.size() == 0 ) {
	log_warn( "HitSeries is empty on String: %i OM: %i! Continuing with next series...", MapIter->first.GetString(), MapIter->first.GetOM() );
	continue;
      }
      
      //------------------------------------
      // If the user wants to do first hits only, do that
      //------------------------------------
      if (useFirstOnly){
	AddHit_AverageTime(HitSeries.front(), previousAvgStep, previousVarStep, nHits);
      }
      //------------------------------------
      // Otherwise, cycle over all hits in the hit series
      //------------------------------------
      else {
	typename std::vector<HitType>::const_iterator SeriesIter;
	for( SeriesIter = HitSeries.begin(); SeriesIter != HitSeries.end(); ++SeriesIter){
	  AddHit_AverageTime(*SeriesIter, previousAvgStep, previousVarStep, nHits);
	}
      }

      log_trace("Currently have (average, variance, hits) = (%f, %f, %i)", previousAvgStep, previousVarStep/(nHits-1), nHits);
    }
  
  //--------------------------------------------
  // Set the average, std deviation
  //--------------------------------------------
  cog->SetTime(previousAvgStep);
  
  if (nHits > 1) {
    StdDev = sqrt( previousVarStep / (nHits-1) );
  }
  else {
    // This is critical for the veto region, as we trigger only on DC.
    log_warn("There are fewer than 2 DOMs hit in %s. Setting standard deviation to 0", Name.c_str());
    StdDev = 0;
  }

  log_debug("Exiting GetAverageTime");  

  return;
}

/* *********************************************************
 * AddHit_AverageTime                                     */
/** Add the current hit to the average and variance.
 *
 * \param hit The current hit to be added to the average
 *            and variance calculations.
 * \param previousAvgStep The average from the previous step.
 *                        It will be used to store the updated 
 *                        average.
 * \param previousVarStep The variance from the previous step.
 *                        It will be used to store the updated
 *                        variance.
 * \param nHits The number of hits processed so far.
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::AddHit_AverageTime(const HitType& hit,
						    double& previousAvgStep,
						    double& previousVarStep,
						    int& nHits)
{

  //------------------------------------
  // Initialize the current step variables
  //------------------------------------
  double currentAvgStep = 0.0;
  double currentVarStep = 0.0;
  

  //------------------------------------
  // Get the Hit time
  //------------------------------------
  double firstHitTime = GetTime(hit);
  
  // Check that the time is not unreasonable...
  if( std::isnan(firstHitTime) || std::isinf(firstHitTime) ) {
    log_debug( "Got NAN or INF for hit time! Continuing with next hit..." );
    return;
  }
  
  //------------------------------------
  // Increase the series counter
  //------------------------------------
  ++nHits;
  
  //------------------------------------
  // Do the calculation 
  //------------------------------------
  double incrementalStep = firstHitTime - previousAvgStep;
  currentAvgStep = previousAvgStep + (incrementalStep / nHits);
  currentVarStep = previousVarStep + (nHits - 1) * incrementalStep * incrementalStep/nHits;
  
  // Get ready for the next iteration
  previousAvgStep = currentAvgStep;
  previousVarStep = currentVarStep;
  

}

/*****************************************************
 * GetCoG                                            */
/** Calculate the center of gravity of the hits. If the
 *   useFirstOnly option is set to true, only the first
 *   hit on each DOM will be used in the calculation. 
 *   Otherwise, all hits will be included. The result
 *   is stored in cog.
 *
 * \param geometry The geometry holding the OMKey and DOM positions.
 * \param HitMap Either an I3RecoPulseSeriesMapConstPtr or 
 *               I3DOMLaunchSeriesMapConstPtr containing the 
 *               hits to be processed.
 * \param cog A pointer to an I3Particle. This will be used to 
 *            hold the center of gravity from this calculation.
 * \param useFirstOnly A boolean specifying whether to limit
 *                     the CoG calculation to first hits only.
 * \param useChargeWeightCoG A boolean specifying whether to weight
 *                           the hits used in the CoG calculation 
 *                           by their charge
 *//***************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::GetCoG(const I3Geometry& geometry,
					boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
					I3Particle* cog,
					bool useFirstOnly,
					bool useChargeWeightCoG) {
  
  log_debug("Entering CalculateCoG");
  
  // Variables needed for this step
  double xsum = 0;
  double ysum = 0;
  double zsum = 0;
  double weight_total = 0;
  
  // Iterator for the hits
  typename I3Map<OMKey, std::vector<HitType> >::const_iterator MapIter;
  for(MapIter=HitMap->begin(); MapIter != HitMap->end(); ++MapIter)
    {
      //--------------------------------------------
      // Grab the DOM
      //--------------------------------------------
      const OMKey& dom = MapIter->first;
      
      // Get the hit position
      I3OMGeoMap::const_iterator Geo_iter = geometry.omgeo.find(dom);
      if(Geo_iter==geometry.omgeo.end())
	{
	  log_trace("  Current OMKey (%i, %i) not found in geometry",
		    MapIter->first.GetOM(), 
		    MapIter->first.GetString());
	  continue;
	}
      const I3Position& dom_position = Geo_iter->second.position;
      log_debug("Looking at OM %i on string %i.", MapIter->first.GetOM(), MapIter->first.GetString());
      
      //--------------------------------------------
      // The old DeepCoreVeto behaviour used only the first hit on each DOM. 
      //--------------------------------------------
      if (useFirstOnly){
	if (useChargeWeightCoG){
	  double charge = GetCharge(MapIter->second.front());
          xsum+=dom_position.GetX() * charge;
          ysum+=dom_position.GetY() * charge;
          zsum+=dom_position.GetZ() * charge;
	  weight_total+= charge;
	}
	else{
          xsum+=dom_position.GetX();
          ysum+=dom_position.GetY();
          zsum+=dom_position.GetZ();
	  ++weight_total;
	}
      }
      // If the user wants, we can try looping over every hit on each DOM.
      else {
	typename std::vector<HitType>::const_iterator Series_iter;
	for(Series_iter=MapIter->second.begin(); Series_iter!=MapIter->second.end(); ++Series_iter){
	  if (useChargeWeightCoG){
            double charge = GetCharge(*Series_iter);
            xsum+=dom_position.GetX() * charge;
            ysum+=dom_position.GetY() * charge;
            zsum+=dom_position.GetZ() * charge;
	    weight_total+= charge;
	  }
	  else{
            xsum+=dom_position.GetX();
            ysum+=dom_position.GetY();
            zsum+=dom_position.GetZ();
	    ++weight_total;
	  }
	}
      }
      
      log_trace("  Current sums: (x, y, z, weight total) = (%f, %f, %f, %f)", xsum, ysum, zsum, weight_total);
    }

  //--------------------------------------------
  // Set position to the correct value and return
  //--------------------------------------------
  cog->SetPos(xsum/weight_total, ysum/weight_total, zsum/weight_total);
  log_debug("CoG position: (x, y, z) = (%f, %f, %f)", cog->GetX(), cog->GetY(), cog->GetZ());
  return;
}

/* *********************************************************
 * GetCoGTime                                              */
/** Calculate the average time-of-flight corrected time 
 *   of the hit series. This uses time corrections based
 *   on the position of the hit relative to the center of
 *   gravity. 
 *
 * \f[ \frac{\sum_{i=0}^{N_{evts}}{t_i - \frac{\vec{r_hit} - \vec{r_cog}}{c_ice} }}{N_{evts}} \f]
 *
 * \param geometry The geometry holding the OMKey and DOM positions.
 * \param HitMap Either an I3RecoPulseSeriesMapConstPtr or 
 *               I3DOMLaunchSeriesMapConstPtr containing the 
 *               hits to be processed.
 * \param cog A pointer to an I3Particle. This will be used to 
 *            hold the time from this calculation.
 * \param useFirstOnly A boolean specifying whether to limit
 *                     the CoG calculation to first hits only.
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::GetCoGTime(const I3Geometry& geometry, 
					    boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
					    I3Particle* cog,
					    bool useFirstOnly)
{
  log_debug("Entering GetCoGTime");
  
  int nHits = 0;
  double average = 0;
  
  //--------------------------------------------
  //  Compute the average and variance of the first hit times
  //--------------------------------------------
  typename I3Map<OMKey, std::vector<HitType> >::const_iterator MapIter;
  for( MapIter = HitMap->begin(); MapIter != HitMap->end(); ++MapIter)
    {
      //------------------------------------
      // Get the hit position
      //------------------------------------
      const OMKey& dom = MapIter->first;
      I3OMGeoMap::const_iterator Geo_iter = geometry.omgeo.find(dom);
      if(Geo_iter==geometry.omgeo.end())
	{
	  log_trace("  Current OMKey not found in geometry");
	  continue;
	}
      const I3Position& dom_position = Geo_iter->second.position;
      double distance = (cog->GetPos()-dom_position).Magnitude();
      
      //------------------------------------
      // Get the hit series from the map
      //------------------------------------
      const std::vector<HitType> HitSeries = MapIter->second;
      log_trace( "  HitSeries size: %zu", HitSeries.size() );
      
      // Check that the series is not empty...
      if( HitSeries.size() == 0 ) {
	log_warn( "HitSeries on is empty! Continuing with next series..." );
	continue;
      }
      
      //------------------------------------
      // If the user wants to do first hits only, do that
      //------------------------------------
      if (useFirstOnly)
	AddHit_CoGTime(HitSeries.front(), distance, average ,nHits);
      //------------------------------------
      // Otherwise, cycle over all hits in the hit series
      //------------------------------------
      else {
	typename std::vector<HitType>::const_iterator SeriesIter;
	for( SeriesIter = HitSeries.begin(); SeriesIter != HitSeries.end(); ++SeriesIter){
	  AddHit_CoGTime(*SeriesIter, distance, average ,nHits);
	}
      }
     
      log_trace("Current values: (Average, nHits) = (%f, %i)", average, nHits);
      
    }
      

  //--------------------------------------------
  // Set the average
  //--------------------------------------------
  cog->SetTime(average);
  
  return;
}


/* *********************************************************
 * AddHit_CoGTime                                          */
/** Add the current hit to the CoG time calculation.
 *
 * \param hit The current hit to be added to the CoG time
 *             calculation.
 * \param distance The distance from the CoG to the DOM.
 * \param average The current average. Will be used to store
 *                the updated average from this function.
 * \param nHits The number of hits processed so far.
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::AddHit_CoGTime(const HitType& hit,
						const double distance,
						double& average,
						int& nHits)
{
  double firstHitTime = GetTime(hit);

  // Check that the time is not unreasonable...
  if( std::isnan(firstHitTime) || std::isinf(firstHitTime) ) {
    log_debug( "Got NAN or INF for hit time! Continuing with next series..." );
    return;
  }
  
  //------------------------------------
  // Increase the series counter and correct for time-of-flight from CoG
  //------------------------------------
  ++nHits;
  double correctedHitTime = std::abs(firstHitTime - distance/(I3Constants::c/I3Constants::n_ice));
  
  //------------------------------------
  // Do the calculation 
  //------------------------------------
  average += (correctedHitTime - average)/nHits;
  
}


/* *********************************************************
 * ReduceHits                                              */
/** Remove all hits outside of one standard deviation from 
 *   the average (uncorrected) time. The result will be 
 *   stored in newHitMap.
 *
 * \param geometry The geometry holding the OMKey and DOM positions.
 * \param HitMap Either an I3RecoPulseSeriesMapConstPtr or 
 *               I3DOMLaunchSeriesMapConstPtr containing the 
 *               hits to be processed.
 * \param cog A pointer to an I3Particle holding the CoG
 *            position and time.
 * \param StdDev A double holding the result of the standard 
 *               deviation calculation.
 * \param newHitMap The map where the hits within one standard
 *                  deviation from the average time will be
 *                  stored.
 * \param useFirstOnly A boolean specifying whether to limit
 *                     the calculation to first hits only
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::ReduceHits(const I3Geometry& geometry,
					    boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
					    I3Particle* cog, 
					    double StdDev,
					    I3Map<OMKey, std::vector<HitType> >& newHitMap,
					    bool useFirstOnly)
{
  log_trace("Entering ReduceHits");
  newHitMap.clear();

  typename I3Map<OMKey, std::vector<HitType> >::const_iterator MapIter;
  for(MapIter = HitMap->begin(); MapIter != HitMap->end(); ++MapIter)
    {
      //--------------------------------------------
      // Grab the DOM
      //--------------------------------------------
      const OMKey& dom = MapIter->first;
      
      // Get the hit position
      I3OMGeoMap::const_iterator Geo_iter = geometry.omgeo.find(dom);
      if(Geo_iter==geometry.omgeo.end())
	{
	  log_trace("Current OMKey not found in geometry");
	  continue;
	}

      //------------------------------------
      // Create the vector for the current hit series
      //------------------------------------
      const std::vector<HitType> HitSeries = MapIter->second;
      std::vector<HitType> thisSeries;

      // Check that the series is not empty...
      if( HitSeries.size() == 0 ) {
	log_warn( "HitSeries is empty on String: %i OM: %i! Continuing with next series...", MapIter->first.GetString(), MapIter->first.GetOM() );
	continue;
      }
      
      //------------------------------------
      // If the user wants to do first hits only, do that
      //------------------------------------
      if (useFirstOnly){
	//------------------------------------
	// Only record the hit information if it's timing is in the correct range.
	//------------------------------------
	if( !( std::abs( GetTime(HitSeries.front()) - cog->GetTime()) > StdDev ) )
	  thisSeries.push_back(HitSeries.front());
      }
      //------------------------------------
      // Otherwise, cycle over all hits in the hit series
      //------------------------------------
      else {
	typename std::vector<HitType>::const_iterator SeriesIter;
	for( SeriesIter = HitSeries.begin(); SeriesIter != HitSeries.end(); ++SeriesIter){

	  //------------------------------------
	  // Only record the hit information if it's timing is in the correct range.
	  //------------------------------------
	  if( !( std::abs( GetTime(*SeriesIter) - cog->GetTime()) > StdDev ) )
	  thisSeries.push_back(*SeriesIter);
	}
      }

      //------------------------------------
      // Only add the hit series if it contains hits
      //------------------------------------
      if (thisSeries.size() > 0)
	newHitMap[dom] = thisSeries;
      
      log_trace("Added %zu hits for (string, om) = (%i, %i).", thisSeries.size(), dom.GetString(), dom.GetOM());
      
    } // end loop - SeriesIter.
  
  log_debug("Added %zu hits within one sigma of mean deep core time.", newHitMap.size());
  
  return;
} // end ReduceHits


/* *********************************************************
 * CountHitsInWindow                                       */
/** Count the number of hits in the velocity window. Hits 
 *   are "causal" if their separation from the center of
 *   gravity is nearly lightlike with the hit being before
 *   the CoG in time.
 *
 * \param geometry The geometry holding the OMKey and DOM positions.
 * \param HitMap Either an I3RecoPulseSeriesMapConstPtr or 
 *               I3DOMLaunchSeriesMapConstPtr containing the 
 *               hits to be processed.
 * \param cog A pointer to an I3Particle. This will be used to 
 *            hold the time from this calculation
 * \param nVetoWindowHits The number of hits that are causal
 *                        with the CoG.
 * \param nVetoWindowHits The charge in PE of hits that are causal
 *                        with the CoG.
 * \param useFirstOnly A boolean specifying whether to limit
 *                     the CoG calculation to first hits only.
 *//*********************************************************/
template <class HitType>
void DeepCoreFunctions<HitType>::CountHitsInWindow(const I3Geometry& geometry, 
						   boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap,
						   I3Particle* cog,
						   int& nVetoWindowHits,
						   double& nVetoWindowCharge,
						   bool useFirstOnly)
{
  log_debug("Entering CountHitsInWindow");

  nVetoWindowHits   = 0;
  nVetoWindowCharge = 0;

  //------------------------------------
  // Iterator for the hits
  //------------------------------------
  typename I3Map<OMKey, std::vector<HitType> >::const_iterator MapIter;
  for(MapIter=HitMap->begin(); MapIter!=HitMap->end(); ++MapIter)
    {

      //------------------------------------
      // Grab the hit DOM
      //------------------------------------
      const OMKey& dom = MapIter->first;

      // Get the hit position
      I3OMGeoMap::const_iterator Geo_iter = geometry.omgeo.find(dom);
      if(Geo_iter==geometry.omgeo.end())
	{
	  log_warn("  Current OMKey not found in geometry");
	  continue;
	}
      const I3Position& dom_position = Geo_iter->second.position;

      //------------------------------------
      // Calculate the distance to the CoG
      //------------------------------------
      double distance = (cog->GetPos()-dom_position).Magnitude();
      
      //------------------------------------
      // Grab the hit series
      //------------------------------------
      std::vector<HitType> HitSeries = MapIter->second;

      // Check that the series is not empty...
      if( HitSeries.size() == 0 ) {
	log_warn( "HitSeries is empty on String: %i OM: %i! Continuing with next series...", MapIter->first.GetString(), MapIter->first.GetOM() );
	continue;
      }
      
      //------------------------------------
      // If the user wants to do first hits only, do that
      //------------------------------------
      if (useFirstOnly) {
	log_trace( "  OMKey(%i, %i) Time of hit: %f", 
		   MapIter->first.GetOM(), 
		   MapIter->first.GetString(),
		   GetTime(HitSeries.front()));
	double delta_time =  cog->GetTime() - GetTime(HitSeries.front());
	if( InWindow(distance/delta_time)){
	  ++nVetoWindowHits;
	  nVetoWindowCharge += GetCharge(HitSeries.front());
	}
      }
      //------------------------------------
      // Otherwise, cycle over all hits in the hit series
      //------------------------------------
      else {
	typename std::vector<HitType>::const_iterator SeriesIter;
	for( SeriesIter = HitSeries.begin(); SeriesIter != HitSeries.end(); ++SeriesIter){
	  log_trace( "  Time of hit: %f", GetTime(*SeriesIter));  
	  double delta_time =  cog->GetTime() - GetTime(*SeriesIter);
	  if( InWindow(distance/delta_time)){
	    ++nVetoWindowHits;
	    nVetoWindowCharge += GetCharge(*SeriesIter);
	  }
	}
      }  
    }

  log_debug( "Exiting CountHitsInWindow");

  return;
}



/* *********************************************************
 * InWindow                                                */
/** Determine if event is in the veto window.
 *   if 0.25 nm/n <= particle_speed <= 0.40 nm/n.
 *
 * \param particle_speed The speed at which a particle moved 
 *                       if a hit was caused by an event at the Cog.
 *//*******************************************************/
 template <class HitType>
bool DeepCoreFunctions<HitType>::InWindow(double particle_speed)
{
  log_debug("Entering InWindow()");
  
  const double minParticleSpeed = 0.25; //0.834*I3Constants::c;
  const double maxParticleSpeed = 0.40; //1.334*I3Constants::c;

  log_debug("Exiting InWindow()");
  
  return (bool) (minParticleSpeed <= particle_speed) && 
               (particle_speed <= maxParticleSpeed);

}// end InWindow.



/* *******************************************/
/** Helper function to get the charge of an I3RecoPulse.
 * 
 * \param pulse An I3RecoPulse with charge information.
 * *******************************************/
template <class HitType>
double DeepCoreFunctions<HitType>::GetCharge(const I3RecoPulse &pulse){
  return pulse.GetCharge();
}

/* *******************************************/
/** Helper function to get the charge of an I3DOMLaunch.
 * 
 * \param launch An I3DOMLaunch with charge information.
 * *******************************************/
template <class HitType>
double DeepCoreFunctions<HitType>::GetCharge(const I3DOMLaunch &launch){
  return 1.0;
}

/* *******************************************/
/** Helper function to get the time of an I3RecoPulse.
 * 
 * \param pulse An I3RecoPulse with timing information.
 * *******************************************/
template <class HitType>
double DeepCoreFunctions<HitType>::GetTime(const I3RecoPulse &pulse) {
  return pulse.GetTime();
}

/* *******************************************/
/** Helper function to get the start time of an I3DOMLaunch
 * 
 * \param launch An I3DOMLaunch with timing information
 * *******************************************/
template <class HitType>
double DeepCoreFunctions<HitType>::GetTime(const I3DOMLaunch &launch) {
  return launch.GetStartTime();
}




template class DeepCoreFunctions<I3DOMLaunch>;
template class DeepCoreFunctions<I3RecoPulse>;
