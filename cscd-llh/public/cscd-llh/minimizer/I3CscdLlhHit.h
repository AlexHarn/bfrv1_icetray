/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhHit.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_HIT_H
#define I3_CSCD_LLH_HIT_H

#include <math.h>

#include "icetray/OMKey.h"

/**
 * @brief The I3CscdLlhHit struct holds OM hit data.
 */  

struct I3CscdLlhHit 
{
  public:

  /**
   * The hit time and position.
   */
  double t, x, y, z;

  /**
   * Multiply the log-likelihood for the hit by the weight.
   */
  double weight;

  /**
   * omHitCount is the number of hits that the OM got in the event, 
   * in other words, this hit is one of a set hits for some OM.
   */
  int omHitCount;

  /**
   * hitNumber is a zero-based number for an OM's hits,
   * i.e., hitNumber values are 0, 1, ..., omHitCount-1.
   */
  int hitNumber;

  /**
   * Trigger threshold
   */
  double triggerThresh;

  double amplitude;

  /**
   * OM identifier (string, OM).  Usually just used for debugging.
   */
  OMKey omKey;

  I3CscdLlhHit() 
  {
    t = NAN; x = NAN; y = NAN; z = NAN;
    weight = 1.0;
    omHitCount = 1;
    hitNumber = 0;
    triggerThresh = NAN;
  }
  
  I3CscdLlhHit(double time, double posX, double posY, double posZ, 
    double hitWeight, int omCount, int hitNum, OMKey hitDom)
  {
    t = time;
    x = posX;
    y = posY;
    z = posZ;
    weight = hitWeight;
    omHitCount = omCount;
    hitNumber = hitNum;
    omKey = hitDom;
    triggerThresh = NAN;
    amplitude = NAN;
  }

  I3CscdLlhHit(double time, double posX, double posY, double posZ,
    double hitWeight, int omCount, int hitNum, OMKey hitDom, 
    double thresh, double amp)
  {
    t = time;
    x = posX;
    y = posY;
    z = posZ;
    weight = hitWeight;
    omHitCount = omCount;
    hitNumber = hitNum;
    omKey = hitDom;
    triggerThresh = thresh;
    amplitude = amp;
  }

  
  I3CscdLlhHit(double time, double posX, double posY, double posZ, 
    double hitWeight, int omCount, int hitNum)
  {
    t = time;
    x = posX;
    y = posY;
    z = posZ;
    weight = hitWeight;
    omHitCount = omCount;
    hitNumber = hitNum;
    triggerThresh = NAN;
  }
}; // struct I3CscdLlhHit

typedef boost::shared_ptr<I3CscdLlhHit> I3CscdLlhHitPtr;

#endif
