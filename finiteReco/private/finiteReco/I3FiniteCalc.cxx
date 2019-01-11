/**
 * @brief implementation of the I3FiniteCalc class
 *
 * @file I3FiniteCalc.cxx
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class calculates some parameters useful for the identification of finite tracks. These parameters can be stored within the I3FiniteCuts class (where you can find a short explanation of them). 
 *
 * The calculation of these parameters is placed in this separate class due to the structure of the calculation. For a given track a cylinder is created. The axis of the cylinder is oriented along the track axis. The radius defines which DOMs are included in the calculations. For each DOM the Cherenkov light emission point on the track is estimated. Those points are used for the calculation of the parameters.
 */

#include "finiteReco/I3FiniteCalc.h"
#include <boost/make_shared.hpp>

I3FiniteCalc::I3FiniteCalc(const I3Geometry& geo,
                           const I3Particle& track,
                           I3RecoPulseSeriesMapConstPtr pulsemap,
                           const double radius,
                           const bool chMode):
  geometry_(geo),
  cylinder_(track),
  pulsemap_(pulsemap),
  cylinderR_(radius),
  chMode_(chMode)
{
  detector_  = this->FindDetectorEnds();
  event_     = this->FindEventEnds();
  haveEvent_ = true;
  eventMax_  = this->FindEventEndsMax();
}

// I3FiniteCalc::I3FiniteCalc():
//   geometry_(I3Geometry()),
//   cylinder_(I3Particle()),
//   pulsemap_(boost::make_shared<I3RecoPulseSeriesMap>()),
//   cylinderR_(dummy_double),
//   chMode_(dummy_bool){}


/************************************************************************/
/* length calculations                                                  */
/************************************************************************/
double I3FiniteCalc::GetEventLength() const{
  return event_.distStopp - event_.distStart;
}
double I3FiniteCalc::GetEventLengthMax() const{
  return eventMax_.distStopp - eventMax_.distStart;
}
  
double I3FiniteCalc::GetDetectorLength() const{
  return detector_.distStopp - detector_.distStart;
}

/************************************************************************/
/* Start Stop Point calculations                                        */
/************************************************************************/
static I3Position MoveAlongCylinder(const I3Particle& cylinder,
                                    const double& length){
  I3Position cPos = cylinder.GetPos();
  I3Direction cDir = cylinder.GetDir();
  I3Position pos(cPos.GetX() + length * cDir.GetX(),
                 cPos.GetY() + length * cDir.GetY(),
                 cPos.GetZ() + length * cDir.GetZ());
  return pos;
}

I3Position I3FiniteCalc::GetEventStart() const{
  return MoveAlongCylinder(cylinder_,event_.distStart);
}
I3Position I3FiniteCalc::GetEventStop() const{
  return MoveAlongCylinder(cylinder_,event_.distStopp);
}
I3Position I3FiniteCalc::GetEventStartMax() const{
  return MoveAlongCylinder(cylinder_,eventMax_.distStart);
}
I3Position I3FiniteCalc::GetEventStopMax() const{
  return MoveAlongCylinder(cylinder_,eventMax_.distStopp);
}
I3Position I3FiniteCalc::GetDetectorStart() const{
  return MoveAlongCylinder(cylinder_,detector_.distStart);
}
I3Position I3FiniteCalc::GetDetectorStop() const{
  return MoveAlongCylinder(cylinder_,detector_.distStopp);
}
 
/************************************************************************/
/* cuts                                                                 */
/************************************************************************/

double I3FiniteCalc::GetSdet() const{
  double lN = detector_.distStopp - detector_.distStart;
  double Sdet = 0;
  std::vector<double> lengthAll;
  I3RecoPulseSeriesMap::const_iterator imap;
  for(imap=pulsemap_->begin(); imap!=pulsemap_->end(); imap++){
    OMKey key = imap->first;
    I3OMGeoMap::const_iterator omgeo=geometry_.omgeo.find(key);
    I3Position omPos = omgeo->second.position;
    if(GetDistPerp(cylinder_,omPos) < cylinderR_){
      double dist;
      if(chMode_) dist = GetDistChPos(cylinder_,omPos);
      else dist = GetDistPara(cylinder_,omPos);
      lengthAll.push_back(dist);
    }
  }
  sort(lengthAll.begin(),lengthAll.end());
  
  int N = lengthAll.size();
  for(int j=0; j<N; ++j){
    double lj = lengthAll[j]-detector_.distStart;
    double S = (double)j/(double)N - lj/lN;
    log_trace("j: %i  N: %i  S: %f",j,N,S);
    if (fabs(S)>fabs(Sdet)) Sdet = S;
  }
  // calculation is meaningless for less than 3 hits
  if(N <= 2) Sdet = NAN;
  return Sdet;
}  

double I3FiniteCalc::GetFiniteCut() const{
  double middle = (detector_.distStart + detector_.distStopp)/2;
  double lengthHalf = (detector_.distStopp - detector_.distStart)/2;
  I3RecoPulseSeriesMap::const_iterator imap;
  double cutValue(0);
  for(imap=pulsemap_->begin(); imap!=pulsemap_->end(); imap++){
    OMKey key = imap->first;
    I3OMGeoMap::const_iterator omgeo=geometry_.omgeo.find(key);
    I3Position omPos = omgeo->second.position;
    if(GetDistPerp(cylinder_,omPos) < cylinderR_){
      double dist;
      if(chMode_) dist = GetDistChPos(cylinder_,omPos);
      else dist = GetDistPara(cylinder_,omPos);
      
      cutValue += (dist - middle) / lengthHalf;
    }
  }
  return cutValue / pulsemap_->size();
}

double I3FiniteCalc::GetLend() const{
  return (detector_.distStopp - event_.distStopp) / (detector_.distStopp - detector_.distStart);
}

double I3FiniteCalc::GetLstart() const{
  return (event_.distStart - detector_.distStart) / (detector_.distStopp - detector_.distStart);
}

I3FiniteCuts I3FiniteCalc::GetCuts() const{
  I3FiniteCuts cut;
  cut.Sdet           = this->GetSdet();
  cut.finiteCut      = this->GetFiniteCut();
  cut.Length         = this->GetEventLength();
  cut.Lend           = this->GetLend();
  cut.Lstart         = this->GetLstart();
  cut.DetectorLength = this->GetDetectorLength();
  return cut;
}
  /************************************************************************/
  /* end calculations                                                     */
  /************************************************************************/
void I3FiniteCalc::CheckHit(const OMKey& key, 
                            const I3Position& omPos, 
                            const double& time, 
                            const I3Particle& cylinder,
                            CylinderGeo& geo) const{
  if(GetDistPerp(cylinder,omPos) < cylinderR_){
    double dist;
    if(chMode_) dist = GetDistChPos(cylinder,omPos);
    else dist = GetDistPara(cylinder,omPos);
    
    if(dist < geo.distStart){
      geo.distStart = dist;
      geo.timeStart = time;
      geo.omStart = key;
    }
    if(dist > geo.distStopp){
      geo.distStopp = dist;
      geo.timeStopp = time;
      geo.omStopp = key;
    }
  }
  return;
}

I3FiniteCalc::CylinderGeo I3FiniteCalc::FindDetectorEnds() const{
  CylinderGeo geo;
  geo.distStart=1e300;
  geo.distStopp=-1e300;
  I3OMGeoMap::const_iterator igeo;
  for(igeo=geometry_.omgeo.begin(); igeo!=geometry_.omgeo.end(); ++igeo){
    if(igeo->second.omtype == I3OMGeo::IceCube){
      this->CheckHit(igeo->first,igeo->second.position,NAN,cylinder_,geo);
    }
  }  
  if(geo.distStart > 1e299){
    log_info("No OM close enough for start point");
    geo.distStart = NAN;
  }
  if(geo.distStopp <-1e299){
    log_info("No OM close enough for stop point");
    geo.distStopp = NAN;
  }
  return geo;
}

I3FiniteCalc::CylinderGeo I3FiniteCalc::FindEventEnds() const{
  CylinderGeo geo;
  geo.distStart = 1e300;
  geo.distStopp = -1e300;
  I3RecoPulseSeriesMap::const_iterator imap;
  if(pulsemap_->empty())
    log_debug("Got no hits! Reco will fail!");
  for(imap=pulsemap_->begin(); imap!=pulsemap_->end(); ++imap){
    const OMKey& key = imap->first;
    I3OMGeoMap::const_iterator omgeo=geometry_.omgeo.find(key);
    const I3Position& omPos = omgeo->second.position;
    if(not imap->second.empty()){
      double time = imap->second.begin()->GetTime();
      this->CheckHit(key,omPos,time,cylinder_,geo);
    }
    else{
      log_warn("Empty pulse series, skipping this DOM!");
      continue;
    }
  }
  if(geo.distStart > 1e299){
    log_info("No Hit close enough for start point");
    geo.distStart = NAN;
  }
  if(geo.distStopp <-1e299){
    log_info("No Hit close enough for stop point");
    geo.distStopp = NAN;
  }
  return geo;
}

I3FiniteCalc::CylinderGeo I3FiniteCalc::FindEventEndsMax()const{
  CylinderGeo geo;
  geo.distStart = -1e300;
  geo.distStopp = 1e300;
  geo.timeStart = event_.timeStart;
  geo.timeStopp = event_.timeStopp;
  I3OMGeoMap::const_iterator igeo;
  for(igeo = geometry_.omgeo.begin(); igeo!=geometry_.omgeo.end();++igeo){
    I3Position omPos = igeo->second.position;
    OMKey key = igeo->first;
    if(GetDistPerp(cylinder_,omPos) < cylinderR_){
      double dist;
      if(chMode_) dist = GetDistChPos(cylinder_,omPos);
      else dist = GetDistPara(cylinder_,omPos);
      
      if(dist > geo.distStart && dist <= event_.distStart){
        geo.distStart = dist;
        geo.omStart = key;
      }
      if(dist < geo.distStopp && dist >= event_.distStopp){
        geo.distStopp = dist;
        geo.omStopp = key;
      }
    }
  }
  if(geo.distStart > 1e299){ 
    log_info("No Hit close enough for start point");
    geo.distStart = NAN;
  }
  if(geo.distStopp <-1e299){
    log_info("No Hit close enough for stop point");
    geo.distStopp = NAN;
  }
  return geo;
}
