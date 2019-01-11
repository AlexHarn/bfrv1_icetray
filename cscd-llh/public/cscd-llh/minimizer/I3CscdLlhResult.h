/**
 * copyright  (C) 2005
 * the icecube collaboration
 *  $Id$
 *
 * @file I3CscdLlhResult.h
 * @version $Revision: 1.1 $
 * @date $Date$
 * @author mggreene
 */

#ifndef I3_CSCD_LLH_RESULT_H
#define I3_CSCD_LLH_RESULT_H

#include <math.h>

/**
 * @brief The I3CscdLlhResult struct holds the result of the Llh minimization.
 */  

struct I3CscdLlhResult
{
  public:

    int status;
    double t;
    double x, y, z;
    double theta, phi;
    double energy;

    double errT;
    double errX, errY, errZ;
    double errTheta, errPhi;
    double errEnergy;

    double negLlh;

    I3CscdLlhResult() 
    {
      Clear();
    }

    void Clear() 
    {

      status = -1;
      t = NAN;
      x = y = z = NAN;
      theta = phi = NAN;
      energy = NAN;

      errT = NAN;
      errX = errY = errZ = NAN;
      errTheta = errPhi = NAN;
      errEnergy = NAN;

       negLlh = NAN;
    }

}; // struct I3CscdLlhResult

typedef boost::shared_ptr<I3CscdLlhResult> I3CscdLlhResultPtr;
#endif
