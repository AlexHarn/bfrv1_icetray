/**
 *  $Id$
 *
 *  Copyright (C) 2004 - 2008
 *  Troy D. Straszheim  <troy@icecube.umd.edu>
 *  Simon Patton, John Pretz, and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "icetray/I3Module.h"

#include <sys/time.h>
#include <sys/resource.h>

#include <boost/bind.hpp>
#include <boost/preprocessor.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

#include "icetray/I3TrayInfo.h"
#include "icetray/I3Context.h"
#include "icetray/I3Tray.h"
#include "icetray/I3PhysicsUsage.h"
#include "icetray/IcetrayFwd.h"
#include "icetray/I3Frame.h"
#include "icetray/I3FrameMixing.h"
#include "icetray/impl.h"

#ifdef MEMORY_TRACKING
#include "icetray/memory.h"
#endif

const double I3Module::min_report_time_ = 10;

class ModuleTimer
{
  double& sys;
  double& user;
  struct rusage stop, start;
   bool fail;
 public:
   ModuleTimer(double& s, double& u) : sys(s), user(u)
  {
    fail = (getrusage(RUSAGE_SELF, &start) == -1);
  }
  ~ModuleTimer()
  {
    if (getrusage(RUSAGE_SELF, &stop) != -1 && !fail)
      {
	user += (stop.ru_utime.tv_sec - start.ru_utime.tv_sec);
	user += double(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) / 1E+06;

	sys += (stop.ru_stime.tv_sec - start.ru_stime.tv_sec);
	sys += double(stop.ru_stime.tv_usec - start.ru_stime.tv_usec) / 1E+06;
      }
  }
};

I3Module::I3Module(const I3Context& context)
  : context_(context), inbox_()
{
  nphyscall_ = ndaqcall_ = 0;
  sysphystime_ = userphystime_ = 0;
  sysdaqtime_ = userdaqtime_ = 0;
  i3_log("%s done", __PRETTY_FUNCTION__);
}

I3Module::~I3Module()
{
  // only print if more than 10 seconds used.  This is kind of an
  // arbitrary number.
  if (userphystime_ + sysphystime_ > min_report_time_)
    log_info("%40s: %6u calls to physics %9.2fs user %9.2fs system",
	   GetName().c_str(), nphyscall_, userphystime_, sysphystime_);
  if (userdaqtime_ + sysdaqtime_ > min_report_time_)
    log_info("%40s: %6u calls to DAQ %9.2fs user %9.2fs system",
	   GetName().c_str(), ndaqcall_, userdaqtime_, sysdaqtime_);
}

void
I3Module::Flush()
{
  for (outboxmap_t::iterator iter = outboxes_.begin();
       iter != outboxes_.end();
       iter++)
    {
      I3ModulePtr nextmodule = iter->second.second;
      FrameFifoPtr fifo = iter->second.first;

      if (nextmodule)
	{
	  while (fifo->size())
	    {
	      nextmodule->Do(&I3Module::Process_);
	    }
	}
    }
}

void
I3Module::Do(void (I3Module::*f)())
{
#ifdef MEMORY_TRACKING
  memory::set_scope(GetName());
#endif
  try {
    (this->*f)();
  } catch (...) {
    log_error("%s: Exception thrown", GetName().c_str());
    throw;
  }

  for (outboxmap_t::iterator iter = outboxes_.begin();
       iter != outboxes_.end();
       iter++)
    {
      I3ModulePtr nextmodule = iter->second.second;
      FrameFifoPtr fifo = iter->second.first;

      log_trace("%s: %zu frames in fifo %s",
		GetName().c_str(), fifo->size(), iter->first.c_str());

      if (nextmodule)
	{
	  if (f == &I3Module::Process_)
	    {
	      while (fifo->size())
		nextmodule->Do(f);
	    }
	  else if (f == &I3Module::Finish)
	    {
	      while (fifo->size())
		nextmodule->Do(&I3Module::Process_);
	      nextmodule->Do(f);
	    }
	  else
	    nextmodule->Do(f);
	}
    }
#ifdef MEMORY_TRACKING
  memory::set_scope("icetray");
#endif
}

void I3Module::Configure(){}

void
I3Module::Configure_()
{
  Configure();
  // give the user the opportunity to configure the outboxes
  // any way they wish.  if they haven't added an outbox
  // then add one for them.
  if(outboxes_.empty())
    AddOutBox("OutBox");
}

void
I3Module::Register(const I3Frame::Stream& when, boost::function<void(I3FramePtr)> what)
{
  methods_[when] = what;
}

void
I3Module::Process_()
{
  i3_log("%s", __PRETTY_FUNCTION__);

  if (inbox_)
    log_trace("%s: %zu frames in inbox", GetName().c_str(), inbox_->size());

  I3FramePtr frame = PeekFrame();

  log_trace("frame=%p", frame.get());
  if (!frame)
    {
      log_trace("no frame, calling this->Process() in case we're a driving module");
      this->Process();
      return;
    }

  if (ShouldDoProcess(frame)) {
    if (frame->GetStop() == I3Frame::Physics)
      ModuleTimer mt(sysphystime_, userphystime_);
    else if (frame->GetStop() == I3Frame::DAQ)
      ModuleTimer mt(sysdaqtime_, userdaqtime_);

    this->Process();
  } else {
    PopFrame();
    PushFrame(frame);
  }
}

void
I3Module::Process()
{
  i3_log("%s", __PRETTY_FUNCTION__);

  if (inbox_)
    log_trace("%zu frames in inbox", inbox_->size());

  I3FramePtr frame = PopFrame();

  if (!frame)
    return;

  methods_t::iterator miter = methods_.find(frame->GetStop());

  if (miter != methods_.end())
    {
      miter->second(frame);
      return;
    }

  if(frame->GetStop() == I3Frame::Physics && ShouldDoPhysics(frame))
    {
      ++nphyscall_;
      Physics(frame);
    }
  else if(frame->GetStop() == I3Frame::Geometry && ShouldDoGeometry(frame))
    Geometry(frame);
  else if(frame->GetStop() == I3Frame::Calibration && ShouldDoCalibration(frame))
    Calibration(frame);
  else if(frame->GetStop() == I3Frame::DetectorStatus && ShouldDoDetectorStatus(frame))
    DetectorStatus(frame);
  else if(frame->GetStop() == I3Frame::Simulation && ShouldDoSimulation(frame))
    Simulation(frame);
  else if(frame->GetStop() == I3Frame::DAQ && ShouldDoDAQ(frame)) {
    ++ndaqcall_;
    DAQ(frame);
  } else if(ShouldDoOtherStops(frame))
    OtherStops(frame);
}

void
I3Module::AddOutBox(const std::string& s)
{
  // we *could* check for double-adds of outboxes here, but this isn't likely
  // to catch very many buggy situations, and it breaks lots of code,
  // for instance if you switch from I3Module to I3ConditionalModule, you'd
  // have to remove your AddOutBox()

  if (outboxes_.find(s) != outboxes_.end())
    return;

  outboxes_[s].first = FrameFifoPtr(new FrameFifo);
}

void
I3Module::AddParameter(const std::string& name, const std::string& description)
{
  configuration_.Add(name, description);
}

I3FramePtr
I3Module::PopFrame()
{
  if (!inbox_)
    return I3FramePtr();

  if (inbox_->begin() == inbox_->end())
    return I3FramePtr();

  I3FramePtr frame = inbox_->back();
  inbox_->pop_back();
  return frame;
}

inline void
I3Module::SyncCache(std::string outbox, I3FramePtr frame)
{
	if (cachemap_.find(outbox) == cachemap_.end())
		cachemap_[outbox] = boost::make_shared<I3FrameMixer>();

	boost::shared_ptr<I3FrameMixer> cache_ = cachemap_[outbox];
	cache_->Mix(*frame);
}

void
I3Module::PushFrame(I3FramePtr frameptr, const std::string& name)
{
  // Send to outbox
  outboxmap_t::iterator iter = outboxes_.find(name);

  if (iter == outboxes_.end())
    log_fatal("Module \"%s\" attempted to push a frame onto an outbox "
	      "name \"%s\" which either doesn't exist or isn't connected "
	      "to anything.  Check steering file.",
	      GetName().c_str(), name.c_str());

  if(iter->second.second){ //Only do the push if this outbox goes somewhere
    SyncCache(name, frameptr);
    iter->second.first->push_front(frameptr);
    log_trace_stream(GetName() << " pushed frame onto fifo \"" << name << '"');
  }
}

void
I3Module::PushFrame(I3FramePtr frameptr)
{
  // Send to all outboxes
  for (outboxmap_t::iterator iter = outboxes_.begin();
       iter != outboxes_.end();
       iter++)
    {
      if(iter->second.second){ //Only do the push if this outbox goes somewhere
        SyncCache(iter->first, frameptr);
        iter->second.first->push_front(frameptr);
        log_trace_stream(GetName() << " pushed frame onto fifo \"" << iter->first << '"');
      }
    }
}

void
I3Module::Finish()
{
  i3_log("%s", __PRETTY_FUNCTION__);
}

void
I3Module::Physics(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoPhysics(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::Geometry(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoGeometry(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::Calibration(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoCalibration(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::DAQ(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoDAQ(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::DetectorStatus(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoDetectorStatus(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::Simulation(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoSimulation(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::OtherStops(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  PushFrame(frame);
}

bool
I3Module::ShouldDoOtherStops(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::RequestSuspension() const
{
  i3_log("%s", __PRETTY_FUNCTION__);
  boost::shared_ptr<I3Tray> tray=context_.Get<boost::shared_ptr<I3Tray> >("I3Tray");
  if(tray)
    tray->RequestSuspension();
  else
    log_warn("Suspension was requested, but this module does not seem to belong to a tray to suspend");
}


I3PhysicsUsage
I3Module::ReportUsage()
{
  I3PhysicsUsage pu;
  pu.systime = sysphystime_ + sysdaqtime_;
  pu.usertime = userphystime_ + userdaqtime_;
  pu.ncall = nphyscall_ + ndaqcall_;
  return pu;
}

I3FramePtr
I3Module::PeekFrame()
{
  if (!inbox_)
    return I3FramePtr();
  if (inbox_->begin() == inbox_->end())
    return I3FramePtr();
  return inbox_->back();
}

bool
I3Module::ShouldDoProcess(I3FramePtr frame)
{
  i3_log("%s", __PRETTY_FUNCTION__);
  return true;
}

void
I3Module::ConnectOutBox(const std::string& outBoxName, I3ModulePtr targetModule)
{
  // Make sure the outbox exists.
  // This can be correct when the module doesn't add its outboxes until Configure()
  if (outboxes_.find(outBoxName) == outboxes_.end()) {
    AddOutBox(outBoxName);
    log_warn_stream("module \"" << GetName() <<
                    "\" doesn't have an out box named \"" << outBoxName << '"');
  }
  outboxes_[outBoxName].second = targetModule;
  // Make the target module know that our outbox is its inbox
  targetModule->inbox_ = outboxes_[outBoxName].first;
}

bool
I3Module::AllOutBoxesConnected() const
{
  for (outboxmap_t::const_iterator iter = outboxes_.begin();
       iter != outboxes_.end();
       iter++){
    if (!iter->second.second){
      log_error_stream("Module \"" << name_  << "\" outbox \""
		       << iter->first << "\" not connected");
      return false;
    }
  }
  return true;
}

bool
I3Module::HasOutBox(const std::string& outBoxName) const
{
  return outboxes_.find(outBoxName)!=outboxes_.end();
}

bool
I3Module::HasInBox() const
{
  return (bool)inbox_;
}

