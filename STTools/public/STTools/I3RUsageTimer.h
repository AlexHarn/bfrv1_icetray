/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/I3RUsageTimer.h
 * @date $Date$
 * @brief This file contains the definition of the I3RUsageTimer class
 *        within the sttools namespace.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef STTOOLS_I3RUSAGETIMER_H_INCLUDED
#define STTOOLS_I3RUSAGETIMER_H_INCLUDED

#include <sys/time.h>
#include <sys/resource.h>

#include <boost/make_shared.hpp>

#include "icetray/I3Logging.h"
#include "icetray/I3Units.h"
#include "icetray/I3PhysicsTimer.h"

//##############################################################################
namespace sttools {

//==============================================================================
class I3RUsageTimer
{
  public:
    //__________________________________________________________________________
    I3RUsageTimer()
      : internalTotRUsage_(boost::make_shared<I3RUsage>()),
        totSysTime_(internalTotRUsage_->systemtime),
        totUsrTime_(internalTotRUsage_->usertime),
        totWCTime_(internalTotRUsage_->wallclocktime),
        isActive_(false)
    {
        ResetTotalTimes();
    }

    //__________________________________________________________________________
    I3RUsageTimer(
        double& totSysTime, double& totUsrTime, double& totWCTime,
        bool startImmediately=false,
        bool resetTotalTimes=false
    )
      : totSysTime_(totSysTime),
        totUsrTime_(totUsrTime),
        totWCTime_(totWCTime),
        isActive_(false)
    {
        if(startImmediately)
            Start(resetTotalTimes);
    }

    //__________________________________________________________________________
    I3RUsageTimer(
        I3RUsage& rusage,
        bool startImmediately=false,
        bool resetTotalTimes=false
    )
      : totSysTime_(rusage.systemtime),
        totUsrTime_(rusage.usertime),
        totWCTime_(rusage.wallclocktime),
        isActive_(false)
    {
        if(startImmediately)
            Start(resetTotalTimes);
    }

    //__________________________________________________________________________
    /** This constructor is needed for the pybindings. We will store the given
     *  shared pointer to the I3RUsage object within the timer class, so the
     *  I3RUsage object will not get destroyed at any time.
     */
    I3RUsageTimer(
        I3RUsagePtr rusage,
        bool startImmediately=false,
        bool resetTotalTimes=false
    )
      : internalTotRUsage_(rusage),
        totSysTime_(internalTotRUsage_->systemtime),
        totUsrTime_(internalTotRUsage_->usertime),
        totWCTime_(internalTotRUsage_->wallclocktime),
        isActive_(false)
    {
        if(startImmediately)
            Start(resetTotalTimes);
    }

    //__________________________________________________________________________
    ~I3RUsageTimer()
    {
        if(isActive_)
            Stop();
    }

    //__________________________________________________________________________
    /** Resets the total system, user, and wallclock times to zero.
     */
    void ResetTotalTimes();

    //__________________________________________________________________________
    /** Starts the timer.
     *
     *  @param resetGlobalTotTimes If set to ``true``, the total system,
     *      user, and wallclock time will be reset to zero.
     *
     *  @returns The status of the timer (``true`` if it is active, ``false``
     *      otherwise (i.e. an error occurred).
     */
    bool Start(bool resetTotTimes=false);

    //__________________________________________________________________________
    /** Stops the timer. On success, it updates the system, user, and
     *  wallclock time by adding the measuring time duration.
     *
     *  @returns The status of the timer (``true`` if it is still active after
     *      this call (i.e. an error occurred), ``false`` otherwise.
     */
    bool Stop();

    //__________________________________________________________________________
    /** @returns The current resource usage by returning a shared pointer to a
     *      new I3RUsage frame object, which could be put into an I3Frame.
     *      If there was an error, a NULL shared pointer is returned instead.
     *
     *  @note The method assumes, that the Start and Stop methods had been
     *      called before. If not, the values of the returned I3RUsage object
     *      are arbitrary.
     */
    I3RUsagePtr
    GetCurrentRUsage();

    //__________________________________________________________________________
    /** @returns The total resource usage as a shared pointer to a newly created
     *      I3RUsage frame object, which could be put into an I3Frame.
     */
    I3RUsagePtr
    GetTotalRUsage();

  protected:
    //__________________________________________________________________________
    /// A shared pointer to an I3RUsage object for storing the total system,
    /// user, and wallclock time if no external storage was provided.
    I3RUsagePtr internalTotRUsage_;
    /// The reference to the total used system time.
    double& totSysTime_;
    /// The reference to the total used user time.
    double& totUsrTime_;
    /// The reference to the total used wallclock time.
    double& totWCTime_;

    //__________________________________________________________________________
    /// The resource usage at the start time of the timer.
    struct rusage ruStart_;
    /// The resource usage at the stop time of the timer.
    struct rusage ruStop_;
    /// The wallclock timeval at the start time of the timer.
    struct timeval wctStart_;
    /// The wallclock timeval at the stop time of the timer.
    struct timeval wctStop_;

    //__________________________________________________________________________
    /// Flag if the timer is currently active and is measuring the time.
    bool isActive_;

    //__________________________________________________________________________
    /** Returns the time difference in IceCube units between the stop and
     *  start times given by two ``struct timeval`` variables.
     */
    double
    Delta(const struct timeval& startTime, const struct timeval& stopTime);

  private:
    //__________________________________________________________________________
    SET_LOGGER("I3RUsageTimer");
};

//==============================================================================

//______________________________________________________________________________
/** Converts an I3RUsage object into a std::string.
 */
std::string
convertI3RUsageToString(const I3RUsage& rusage);

}/*sttools*/
//##############################################################################

#endif//STTOOLS_I3RUSAGETIMER_H_INCLUDED
