// $Id$

#ifndef I3DEEPCOREFUNCTIONS_INCLUDED
#define I3DEEPCOREFUNCTIONS_INCLUDED

#include "icetray/I3Frame.h"
#include "icetray/I3Bool.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3DOMLaunch.h"

/**
 * @brief Helper module containing the methods used in the
 *         DeepCoreVeto and DeepCoreTimeVeto.
 */
template <class HitType>
class DeepCoreFunctions
{
 public:
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
  static void GetAverageTime(const I3Geometry& geometry, 
			     boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
			     I3Particle* cog,
			     double& StdDev,
			     bool useFirstOnly,
			     std::string Name);
  
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
  static void AddHit_AverageTime(const HitType& hit,
				 double& previousAvgStep,
				 double& previousVarStep,
				 int& nHits);
  
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
  static void GetCoG(const I3Geometry& geometry,
		     boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
		     I3Particle* cog,
		     bool useFirstOnly,
		     bool useChargeWeightCoG);
  
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
  static void GetCoGTime(const I3Geometry& geometry, 
			 boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
			 I3Particle* cog,
			 bool useFirstOnly);
  
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
  static void AddHit_CoGTime(const HitType& hit,
			     const double distance,
			     double& average,
			     int& nHits);
  
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
  static void ReduceHits(const I3Geometry& geometry,
			 boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap, 
			 I3Particle* cog,
			 double StdDev,
			 I3Map<OMKey, std::vector<HitType> >& newHitMap,
			 bool useFirstOnly);
  
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
  static void CountHitsInWindow(const I3Geometry& geometry, 
				boost::shared_ptr<const I3Map<OMKey, std::vector<HitType> > > HitMap,
				I3Particle* cog,
				int& nVetoWindowHits,
				double& nVetoWindowCharge,
				bool useFirstOnly);
  
  // private:
/* *********************************************************
 * InWindow                                                */
/** Determine if event is in the veto window.
 *   if 0.25 nm/n <= particle_speed <= 0.40 nm/n.
 *
 * \param particle_speed The speed at which a particle moved 
 *                       if a hit was caused by an event at the Cog.
 *//*******************************************************/
  static bool InWindow(double particle_speed);

/* *******************************************/
/** Helper function to get the charge of an I3RecoPulse.
 * 
 * \param pulse An I3RecoPulse with charge information.
 * *******************************************/
  //Helper function to get the time of the hits
  static double GetCharge(const I3RecoPulse &pulse);

/* *******************************************/
/** Helper function to get the charge of an I3DOMLaunch.
 * 
 * \param launch An I3DOMLaunch with charge information.
 * *******************************************/
  static double GetCharge(const I3DOMLaunch &launch);

/* *******************************************/
/** Helper function to get the time of an I3RecoPulse.
 * 
 * \param pulse An I3RecoPulse with timing information.
 * *******************************************/  
  //Helper function to get the time of the hits
  static double GetTime(const I3RecoPulse &pulse);

/* *******************************************/
/** Helper function to get the start time of an I3DOMLaunch
 * 
 * \param launch An I3DOMLaunch with timing information
 * *******************************************/
  static double GetTime(const I3DOMLaunch &launch);
  //Logger
  SET_LOGGER("I3DeepCoreFunctions");
};

#endif
