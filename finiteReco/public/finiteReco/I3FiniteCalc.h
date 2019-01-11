/**                                                                                                                          
 * @brief declaration of the I3FiniteCalc class                                                                              
 *                                                                                                                           
 * @file I3FiniteCalc.h                                                                                                      
 * @version $Revision$                                                                                               
 * @date $Date$                                                                
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>                                                                                  
 *
 * This class calculates some values useful to determine whether a event is starting or not. These values are stored
 * within an I3FiniteCuts object. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3FiniteCuts>)
*/

#ifndef I3FINITECALC_H_INCLUDED
#define I3FINITECALC_H_INCLUDED

#include "recclasses/I3FiniteCuts.h"
#include "icetray/I3FrameObject.h"
#include "icetray/I3Logging.h"
#include "icetray/serialization.h"
#include "dataclasses/Utility.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Calculator.h"

#include <cmath>

class I3FiniteCalc{
 public:
  I3FiniteCalc(const I3Geometry& geo,
               const I3Particle& track,
               I3RecoPulseSeriesMapConstPtr pulsemap,
               const double radius,
               const bool chMode = false);
  
  ~I3FiniteCalc(){}
  
  struct CylinderGeo{
    double timeStart;
    double timeStopp;
    OMKey  omStart;
    OMKey  omStopp;
    double distStart;
    double distStopp;
  };

  /************************************************************************/
  /* cuts                                                                 */
  /************************************************************************/
  double GetSdet() const;
  double GetFiniteCut()const;
  double GetLend() const;  
  double GetLstart() const;  
  I3FiniteCuts GetCuts() const;
  
  /************************************************************************/
  /* length calculations                                                  */
  /************************************************************************/
  double GetEventLength() const;
  double GetEventLengthMax() const;
  double GetDetectorLength() const;
  
  /************************************************************************/
  /* start and stop point calculations                                    */
  /************************************************************************/  
  I3Position GetEventStart() const;
  I3Position GetEventStop() const;
  I3Position GetEventStartMax() const;
  I3Position GetEventStopMax() const;
  I3Position GetDetectorStart() const;
  I3Position GetDetectorStop() const;
  
  /************************************************************************/
  /* end calculations                                                     */
  /************************************************************************/
  CylinderGeo FindDetectorEnds() const;  
  CylinderGeo FindEventEnds() const;  
  CylinderGeo FindEventEndsMax() const;
  
 private:
//  I3FiniteCalc();  
  void CheckHit(const OMKey& key, 
                const I3Position& omPos, 
                const double& time, 
                const I3Particle& cylinder,
                CylinderGeo& geo) const; 
  /************************************************************************/
  /* small helpers                                                        */
  /************************************************************************/
  static double GetDistPerp(const I3Particle& cylinder,
                            const I3Position& omPos){
    return I3Calculator::ClosestApproachDistance(cylinder,omPos);
  }
  static double GetDistChPos(const I3Particle& cylinder,
                             const I3Position& omPos){
    I3Position chPos;
    double chTime, chDist, angle_ori;
    I3Calculator::CherenkovCalc(cylinder,
                                omPos,
                                chPos,
                                chTime,
                                chDist,
                                angle_ori );
    return (chPos.GetX() - cylinder.GetPos().GetX())/cylinder.GetDir().GetX();
  }
  static double GetDistPara(const I3Particle& cylinder,
                            const I3Position& omPos){
    return I3Calculator::DistanceAlongTrack(cylinder,omPos);
  }

  
  const I3Geometry& geometry_;
  const I3Particle& cylinder_;
  I3RecoPulseSeriesMapConstPtr pulsemap_;
  const double cylinderR_;
  bool chMode_;
  CylinderGeo detector_;
  bool haveEvent_;
  CylinderGeo event_;
  CylinderGeo eventMax_;
};

I3_POINTER_TYPEDEFS(I3FiniteCalc);

#endif
