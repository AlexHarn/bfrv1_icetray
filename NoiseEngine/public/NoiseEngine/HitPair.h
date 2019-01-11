/**
 * class: HitPair
 * @version $Id: $
 * @date: $Date: $
 * @author Michael Larson <mjlarson@crimson.ua.edu>
 * (c) 2011,2012 IceCube Collaboration
 *
 * Class to hold the azimuth and zenith information. 
 * Use this instead of having multiple parallel arrays.
 */

#ifndef NOISEENGINE_HITPAIR_H_INCLUDED
#define NOISEENGINE_HITPAIR_H_INCLUDED

#include <cmath>
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/physics/I3RecoPulse.h"


/* ******************************************************************** */
/** HitPair
 * A class designed to hold pairs of hits. Holds the time, distance, direction
 * and weight of each pair of hits along with methods to calculate each.
 ******************************************************************** **/
class HitPair
{
 public:

  /* ******************************************************************** */
  /** InTimeWindow
   * Calculates whether this pair of hits lies within the time window 
   * specified by [startTime, startTime + windowLength]
   * 
   * \param startTime The time of the beginning of the window
   * \param windowLength The length of the time window to calculate the end time
   ******************************************************************** **/
  bool InTimeWindow(double startTime, double windowLength) {
    return (firstTime >= startTime) && (lastTime - firstTime <= windowLength);
  };

  /* ******************************************************************** */
  /** InVelocityWindow
   * Check whether the apparent velocity is within the velocity window 
   * defined by [start,end] m/ns
   *
   * \param start The lower bound on the velocity window in m/ns
   * \param end The upper bound on the velocity window in m/ns 
   ******************************************************************** **/
  bool InVelocityWindow(double start, double end) {
    // If the same time (hugely unlikely), then return false
    if (firstTime == lastTime)
      return false;
     
    double velocity = distance/fabs(firstTime-lastTime);
    return ( (velocity > start) && (velocity < end) );
  };

  /* ******************************************************************** */
  /** GetVelocity
   * Returns the apparent velocity
   ******************************************************************** **/
  double GetVelocity(){
    // If the same time (hugely unlikely), then return false
    if (firstTime == lastTime)
      return -1;
     
    double velocity = distance/fabs(firstTime-lastTime);
    return velocity;
  };

  /* ******************************************************************** */
  /** SetAngles
   * Find the distance, azimuth, and zenith angles between two I3Positions.
   * The pointing is from dom1 to dom2, so the assumption is that the earlier
   * hit is the first argument.
   *
   * \param dom1 The position of the earlier hit.
   * \param dom2 The position of the later hit.
   ******************************************************************** **/
  void SetAngles(const I3Position& dom1, const I3Position& dom2) {
    double dx = dom2.GetX() - dom1.GetX();
    double dy = dom2.GetY() - dom1.GetY();
    double dz = dom2.GetZ() - dom1.GetZ();
    distance = (dom2-dom1).Magnitude();
    
    // If the DOMs are the same, just return -1 for both.
    if (distance == 0){
      azimuth = -1;
      zenith = -1;
    }
    else {
      azimuth = atan2(dy, dx);
      zenith = acos(dz/distance);
    }
  };

  /* ******************************************************************** */
  /** SetWeight
   * Calculate the weight for the hit pair. The NoiseEngine
   * user may choose to either use unity or use charge weighting.
   *
   * \param weight1 The weight associated with the first hit.
   * \param weight2 The weight associated with the second hit.
   ******************************************************************** **/
  void SetWeight(double weight1, double weight2){
    weight = (weight1 + weight2)/2.0;
  }

  /* ******************************************************************** */
  /** SetTimes
   * Set the times of the first and second hit. This doesn't provide constraints
   * on the order of the hits, although NoiseEngine always uses the first
   * argument as the earlier hit and the second as the later. 
   *
   * \param hit1 The first time for this pair
   * \param hit2 The second time for this pair
   ******************************************************************** **/
  void SetTimes(double hit1, double hit2){
    firstTime = hit1;
    lastTime = hit2;
  };

  /* ******************************************************************** */
  /** GetFirstTime
   * Returns the first time given to the SetTimes function.
   *
   * \return The first time passed to SetTimes
   ******************************************************************** **/
  double GetFirstTime(){ return firstTime; }

  /* ******************************************************************** */
  /** GetLastTime
   * Returns the second time given to the SetTimes function.
   *
   * \return The second time passed to SetTimes
   ******************************************************************** **/
  double GetLastTime(){ return lastTime; }

  /* ******************************************************************** */
  /** GetAzimuth
   * Returns the azimuth as calculated by the SetAngles function. It points
   * from the first passed DOM to the second in that function.
   *
   * \return The azimuthal angle between DOMs
   ******************************************************************** **/
  double GetAzimuth(){ return azimuth; }

  /* ******************************************************************** */
  /** GetZenith
   * Returns the zenith as calculated by the SetAngles function. It points
   * from the first passed DOM to the second in that function.
   *
   * \return The zenith angle between DOMs
   ******************************************************************** **/
  double GetZenith(){ return zenith; }

  /* ******************************************************************** */
  /** GetWeight
   * Returns the weight calculated by the SetWeight function. 
   *
   * \return The weight
   ******************************************************************** **/
  double GetWeight(){ return weight; }

 private:
  double azimuth, zenith;
  double firstTime, lastTime;
  double distance;
  double weight;  

};

typedef I3Vector<HitPair> HitPairVect;
I3_POINTER_TYPEDEFS(HitPairVect);

#endif
