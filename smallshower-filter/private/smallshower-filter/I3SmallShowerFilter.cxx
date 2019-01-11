/**
 *  Copyright  (C) 2009
 *  the IceCube Collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author Chen Xu <chen@udel.edu>, Bakhtiyar Ruzybayev <bahtiyar@udel.edu>
 *
 */

#include "smallshower-filter/I3SmallShowerFilter.h"
#include "SmallShowerGeometries.h"
#include <interfaces/I3IcePickModule.h>
#include <interfaces/I3IceForkModule.h>
#include <icetray/I3IcePickInstaller.h>
#include <icetray/I3Int.h>
#include <icetray/I3Bool.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <algorithm>


I3SmallShowerFilter::I3SmallShowerFilter(const I3Context & ctx)
  : I3IcePick(ctx), filterGeometry_(""), pulseKey_("TopEvent_0"), resultName_(""),
    station3_(0), station4_(0)
{
  AddParameter("FilterGeometry",
	       "For each IceCube geometry a different set of geometries is allowed."
	       "IC40, IC59, IC79, and IC86 are implemented",
	       filterGeometry_);
  AddParameter("TopPulseKey",
	       "Name of the IceTop pulses in the frame",
	       pulseKey_);
  AddParameter("NStationResultName",
	       "If not empty, puts the number of stations as an I3Int of this"
	       "name into the frame",
	       resultName_);
}

void I3SmallShowerFilter::Configure()
{
  GetParameter("FilterGeometry", filterGeometry_);
  GetParameter("TopPulseKey", pulseKey_);
  GetParameter("NStationResultName", resultName_);

  station3_ = smallshower_filter::getStation3(filterGeometry_);
  station4_ = smallshower_filter::getStation4(filterGeometry_);
  if (!station3_ || !station4_) {
    log_fatal("Geometry \"%s\" unknown. Please check your configuration.",
	      filterGeometry_.c_str());
  }
}

bool I3SmallShowerFilter::SelectFrame(I3Frame& frame)
{
  using namespace smallshower_filter;

  /*
   * Get pulses, if no pulses are found, you either misconfigured the module
   * or there simply are no pulses in the current frame
   */  
  I3RecoPulseSeriesMapConstPtr top_pulses 
    = frame.Get<I3RecoPulseSeriesMapConstPtr>(pulseKey_);

  if (!top_pulses) {
    log_debug("No pulses found. Did you configure your module correctly?");
    return false;
  }

  bool filterPassed = false;
  int nStation = 0;
  if (top_pulses->size() <= 16) {   // 4 DOMs per station --> 4 stations --> max 16 DOMs
    /*
     * Get the list of triggered stations
     */
    std::set<int> stationID_list;
    for (I3RecoPulseSeriesMap::const_iterator iter = top_pulses->begin();
	 iter != top_pulses->end(); ++iter)
      {
	stationID_list.insert(iter->first.GetString());
      }
    nStation = stationID_list.size();

    log_trace("Number of stations: %d", nStation);
    
    /*
     * Check for validity of combinations
     */
    switch (nStation) {
    case 3:
      for (StationList::const_iterator iter = station3_->begin();
	   iter != station3_->end(); ++iter)
	{
	  if (std::equal(iter->begin(), iter->end(), stationID_list.begin())) {
	    filterPassed = true;
	    break;
	  }
	}
      break;
      
    case 4: 
      for (StationList::const_iterator iter = station4_->begin();
	   iter != station4_->end(); ++iter)
	{
	  if (std::equal(iter->begin(), iter->end(), stationID_list.begin())) {
	    filterPassed = true;
	    break;
	  }
	}
      break;
    }
  }

  log_trace("Event passed: %s", filterPassed ? "true" : "false");

  /*
   * Put result to frame, but only if the filter was passed
   */
  if (filterPassed && (resultName_.size() > 0)) {
    frame.Put(resultName_, I3IntPtr(new I3Int(nStation)));
  }

  return filterPassed;
}

I3_MODULE(I3IcePickModule<I3SmallShowerFilter>);
I3_MODULE(I3IceForkModule<I3SmallShowerFilter>);
I3_SERVICE_FACTORY(I3IcePickInstaller<I3SmallShowerFilter>);
