/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/STTools/I3RUsageTimer.cxx
 * @date $Date$
 * @brief This file contains the implementation of the I3RUsageTimer class
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
#include "STTools/I3RUsageTimer.h"

//##############################################################################
namespace sttools {

//______________________________________________________________________________
void
I3RUsageTimer::
ResetTotalTimes()
{
    totSysTime_ = 0;
    totUsrTime_ = 0;
    totWCTime_  = 0;
}

//______________________________________________________________________________
bool
I3RUsageTimer::
Start(bool resetTotTimes)
{
    if(isActive_)
    {
        log_error(
            "The timer is still active! It will not be restarted.");
        return true;
    }

    if(resetTotTimes)
    {
        ResetTotalTimes();
    }

    if(! ((gettimeofday(&wctStart_, NULL)    == 0) &&
          (getrusage(RUSAGE_SELF, &ruStart_) == 0)
         )
      )
    {
        log_error(
            "Could not get resource usage via getrusage and gettimeofday "
            "call for starting the timer!");
        isActive_ = false;
        return false;
    }

    isActive_ = true;
    return true;
}

//______________________________________________________________________________
bool
I3RUsageTimer::
Stop()
{
    if(!isActive_)
    {
        log_error(
            "The timer is not active, so it cannot be stopped!");
        return false;
    }

    if(! ((getrusage(RUSAGE_SELF, &ruStop_) == 0) &&
          (gettimeofday(&wctStop_, NULL) == 0)
         )
      )
    {
        log_error(
            "Could not get resource usage via getrusage and gettimeofday "
            "call for stopping the timer!");
        return true;
    }

    totSysTime_ += Delta(ruStart_.ru_stime, ruStop_.ru_stime);
    totUsrTime_ += Delta(ruStart_.ru_utime, ruStop_.ru_utime);
    totWCTime_  += Delta(wctStart_, wctStop_);

    isActive_ = false;
    return false;
}

//______________________________________________________________________________
double
I3RUsageTimer::
Delta(const struct timeval& startTime, const struct timeval& stopTime)
{
    const double start = (double)startTime.tv_sec*I3Units::second +
                         (double)startTime.tv_usec*I3Units::microsecond;
    const double stop  = (double)stopTime.tv_sec*I3Units::second +
                         (double)stopTime.tv_usec*I3Units::microsecond;
    return (stop - start);
}

//______________________________________________________________________________
I3RUsagePtr
I3RUsageTimer::
GetCurrentRUsage()
{
    I3RUsagePtr rup = boost::make_shared<I3RUsage>();

    if(! isActive_)
    {
        // We assume here, that the time was measured through the Start and Stop
        // calls before. If not, the result will be quite arbitrary.
        rup->systemtime    = Delta(ruStart_.ru_stime, ruStop_.ru_stime);
        rup->usertime      = Delta(ruStart_.ru_utime, ruStop_.ru_utime);
        rup->wallclocktime = Delta(wctStart_, wctStop_);
    }
    else
    {
        struct rusage ruIntermediate;
        struct timeval wctIntermediate;
        if((getrusage(RUSAGE_SELF, &ruIntermediate) == 0) &&
           (gettimeofday(&wctIntermediate, NULL) == 0)
          )
        {
            rup->systemtime    = Delta(ruStart_.ru_stime, ruIntermediate.ru_stime);
            rup->usertime      = Delta(ruStart_.ru_utime, ruIntermediate.ru_utime);
            rup->wallclocktime = Delta(wctStart_, wctIntermediate);
        }
    }

    return rup;
}

//______________________________________________________________________________
I3RUsagePtr
I3RUsageTimer::
GetTotalRUsage()
{
    I3RUsagePtr rup = boost::make_shared<I3RUsage>();
    rup->systemtime    = totSysTime_;
    rup->usertime      = totUsrTime_;
    rup->wallclocktime = totWCTime_;
    return rup;
}

//______________________________________________________________________________
std::string
convertI3RUsageToString(const I3RUsage& rusage)
{
    std::stringstream ss;
    ss.precision(2);
    ss << std::fixed <<
        rusage.systemtime/I3Units::second << "s system, " <<
        rusage.usertime/I3Units::second << "s user, " <<
        rusage.wallclocktime/I3Units::second << "s wallclock";
    return ss.str();
}

}/*sttools*/
//##############################################################################