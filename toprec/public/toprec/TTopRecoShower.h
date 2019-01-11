/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file TTopRecoShower.h
 * @version $Revision: $
 * @date $Date$
 * @author $Author:  $
 */

#ifndef __TTOPRECOSHOWER_H_
#define __TTOPRECOSHOWER_H_


// I'm going to add some stuff to assist in the "gulliverization"
// effort.  It should break anything for anyone else, but just
// in case, here's a way to turn it off.
#define FOR_GULLIVER 1

#include "icetray/OMKey.h"
#include "dataclasses/StationKey.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3TankGeo.h"
#include "phys-services/I3Calculator.h"
#include "dataclasses/I3Constants.h"

#if FOR_GULLIVER
#include "dataclasses/physics/I3RecoPulse.h"
#endif

#include <map>
#include <vector>
#include <string>


struct showerPulse {
  OMKey omkey;
  double x;
  double y;
  double z;
  double t;
  double width;
  double vem;
  double logvem;
  bool usepulsecharge;
  bool usepulsetime;
  double snowdepth;   // this can be used by reconstructions to correct VEM's for snow
};

struct noShowerPulse{
  StationKey stationkey;  
  double x;
  double y;
  double z;
  double avgsnowdepth;
};

class I3Frame;

/**
 * @brief This class is intended to contain all the pulse information of an event which is going to be reconstructed.
 *        The data (position, vem, time) of each hit as to be provided to the class or it can be read automatically
 *        from the frame. It can also hold first-guess-variables like core or direction information. It has to be
 *        a TObject in order to be usable in TMinuit::SetObjectFit().
 */

class TTopRecoShower
{
 public:
  TTopRecoShower();
  ~TTopRecoShower();
    
  /**
   * Reads the icetop data from the frame 
   */
  bool ReadData(I3FramePtr frame, std::string pulselabel, std::string corelabel = "", std::string planelabel = "",
		std::string badStationLabel = "", double software_threshold = -1.);

#if FOR_GULLIVER
  /**
   * Reads the icetop data from stuff that's already been gotten from the frame 
   * (and the seed) 
   */
  int ReadDataFromSeed(const I3Frame &frame, I3ParticleConstPtr seed, 
			//I3RecoPulseSeriesMapPtr pulse_series_map,
			std::string datalabel,
			std::string badStationLabel = "", double software_threshold = -1.);

  // This one changes the track info in a TTopRecoShower (leaves all the pulses
  // not-pulses, etc. unchanged)
  bool UpdateTrackOnly(I3ParticleConstPtr newtrack); 
#endif

  /**
   * Adds a single pulse
   */
  void AddPulse(OMKey omKey, double x, double y, double z, double time,
                double width, double vem, bool usepulsecharge = true, bool usepulsetime = true,
		double snowdepth = 0); 

  /**
   * Adds a single no-pulse (Station without a trigger)
   */
  void AddNoPulse(StationKey stationKey, double x, double y, double z, double avgsnowdepth = 0);
    
  /**
   * Returns the pulses.
   */
  std::vector<showerPulse> GetPulses() {return pulseData;};

  /**
   * Returns the stations without a pulse.
   */
  std::vector<noShowerPulse> GetNoPulses() {return noPulseData;};

  /**
   * Set the shower direction.
   */
  void SetDirection(I3Direction inDir) {dir=inDir; nx=dir.GetX(); ny=dir.GetY();};
  void SetNXNY(double nX, double nY) {nx=nX; ny=nY; dir = I3Direction(nx, ny, -sqrt(1. - nx*nx - ny*ny));};
  void SetPlaneParams(double T0, double X0, double Y0) {t_0 = T0; x_0 = X0; y_0 = Y0;};

  I3Direction GetDir() {return dir;};
  double GetNX() {return nx;};
  double GetNY() {return ny;};
  double GetT0() {return t_0;};
  double GetX0() {return x_0;};
  double GetY0() {return y_0;};
  double GetZenith() {return dir.GetZenith();};
  double GetAzimuth() {return dir.GetAzimuth();};
	 
  double GetArtificialVEMShift() {return artificial_shift;};

  /**
   * Get a starting value for log10(S125) assuming \f$\beta=3\f$ and the seed core and direction.
   */
  double GetLogS125Start();

  /**
   * Set the shower core.
   */
  void SetCore(I3Position pos) {x_c=pos.GetX(); y_c=pos.GetY(); z_c=pos.GetZ();};
  void SetCore(double xc, double yc)/*, double zc = GetMeans(0.).z) */
  {x_c=xc; y_c=yc; z_c=GetMeans().z;};
  void SetTime(double tc) {t_c = tc;};

  double GetX_C() {return x_c;};
  double GetY_C() {return y_c;};
  double GetZ_C() {return z_c;};
  double GetT_C() {return t_c;};


  /**
   *  Specifies which Hit is going to be  used/excluded from the analysis 
   */
  void UsePulseCharge(unsigned int i, bool usePulse=true)
  {if(i<pulseData.size()) pulseData.at(i).usepulsecharge = usePulse;};
  void UsePulseTime(unsigned int i, bool usePulse=true)
  {if(i<pulseData.size()) pulseData.at(i).usepulsetime = usePulse;};
  bool IsUsedPulseCharge(unsigned int i)
  {return pulseData.at(i).usepulsecharge;};
  bool IsUsedPulseTime(unsigned int i)
  {return pulseData.at(i).usepulsetime;};
     
  /**
   * Sets the usepulsecharge-flag of pulses with NAN-charges, negative charges or charges>3000 VEM to false.
   */

  unsigned int SortOutBadCharges();
   
  /**
   * Sets the usepulsetime-flag of pulses with NAN-times to false.
   */

  unsigned int SortOutBadTimes();
   
  /**
   * Sets the usepulsetime- and usepulsecharge-flag of pulses with NAN-widths to false.
   */

  unsigned int SortOutBadWidths();
 
  /**
   * Returns the number of pulses.
   */
  size_t GetNPulses() { return pulseData.size(); }

  /**
   * Returns the number of used pulses.
   */
  unsigned int GetNPulsesUsedCharge();
  unsigned int GetNPulsesUsedTime();
 
  /**
   * Returns the number of used stations.
   */
  unsigned int GetNStationsUsedCharge();
  unsigned int GetNStationsUsedTime();
   
  /**
   * Returns the number of non-fired stations.
   */
  unsigned int GetNNoPulses() {return noPulseData.size();};


  /**
   * Delete pulse \#i.
   */
  void ErasePulse(unsigned int i) {pulseData.erase(pulseData.begin()+i);}; 

    
  /**
   * Deletes all the pulses stored in this object
   */
  void ClearHits();
    
     
  /**
   * Returns the distance of pulse \#i to the shower axis.  
   */
  double GetDistToAxis(unsigned int i){
    return GetDistToAxisIt(pulseData.begin()+i);};
   
  /**
   * Returns the distance of no-pulse (untriggered station) \#i to the shower axis.  
   */
  double GetNoDistToAxis(unsigned int i){
    return GetNoDistToAxisIt(noPulseData.begin()+i);};

  /**
   * Returns the distance of iterator's pulse to the shower axis.
   */

  double GetDistToAxisIt(std::vector<showerPulse>::iterator it){
    double abs_x_sq = (it->x-x_c)*(it->x-x_c)
      + (it->y-y_c)*(it->y-y_c)
      + (it->z-z_c)*(it->z-z_c);
    double n_prod_x = nx*(it->x-x_c)
      + ny*(it->y-y_c)
      - sqrt(1. - nx*nx - ny*ny)*(it->z-z_c);
    return sqrt(abs_x_sq - n_prod_x * n_prod_x);
  }

  /**
   * Returns the distance of iterator's no-pulse to the shower axis.
   */
  double GetNoDistToAxisIt(std::vector<noShowerPulse>::iterator it){
    double abs_x_sq = (it->x-x_c)*(it->x-x_c)
      + (it->y-y_c)*(it->y-y_c)
      + (it->z-z_c)*(it->z-z_c);
    double n_prod_x = nx*(it->x-x_c) 
      + ny*(it->y-y_c)
      - sqrt(1. - nx*nx - ny*ny)*(it->z-z_c);
    return sqrt(abs_x_sq - n_prod_x * n_prod_x);
  }

  /**
   * Get time distance to shower plane.
   */

  double GetDistToPlane(int i){
    return GetDistToPlaneIt(pulseData.begin()+i);
  }  

  double GetDistToPlaneIt(std::vector<showerPulse>::iterator it){
    double t_plane = t_0 + (nx*(it->x-x_0) + ny*(it->y-y_0) - sqrt(1. - nx*nx - ny*ny)*(it->z-z_c))/I3Constants::c;
    return t_plane - it->t;
  }  


  /**
   * The COG of the logs of the radii with respect to a shower core x/y.
   */
  double GetLogRadCOG(double (*sigma_func)(double r, double logq));
 
  /**
   * Returns the loudest available station.
   */
  int GetLoudestStation() {return loudest_station;}
    
  /**
   * Prints out the hit informations of all the pulses stored in this object
   */
#ifdef __clang__
// this shadows TObject::Print(Option_t *option="") const.
// silence the clang warning
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
  void Print();
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    
    
  /**
   * Correct the pulse times for the height of the tanks.
   */
  bool CorrectForHeight();
 
  /**
   * Artificially shift all VEM's
   */
  bool ApplyArtificialVEMShift(double f);

  /**
   * Return a showerPulse that contains the mean of all elements.
   * If power is specified, the points are weighted with a power of the charge.
   * If nTanks is &gt;0 only the first nTanks tank signals are used.
   */
  showerPulse GetMeans(double power = 0., short int nTanks = -1);

 
   
 private:
   
  /**
   * Pulses in the array.
   */
  std::vector<showerPulse> pulseData;

  /**
   * Stations without a pulse.
   */
  std::vector<noShowerPulse> noPulseData;

  /**
   * Shower Direction
   */
  I3Direction dir;
  double nx, ny, t_0, x_0, y_0;

  /**
   * Shower Core.
   */
  double x_c, y_c, z_c, t_c; 

  /**
   * Make sure the height correction is only applied once.
   */
  bool height_corrected;

  /**
   * Make sure the VEM correction is only applied once.
   * Positive number indicates the shift if it has; negative number
   * indicates it is uncorrected.
   */
  double artificial_shift;

  /**
   * Loudest station of that event.
   */
  int loudest_station;

public:
  SET_LOGGER("I3TopLateralFit");
};

#endif
