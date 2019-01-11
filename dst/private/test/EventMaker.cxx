/**
 * A drop-in replacement for I3EmptyEventService from phys-services.
 */

#include "icetray/I3Module.h"
#include "dataclasses/physics/I3EventHeader.h"
#include <boost/make_shared.hpp>

class EventMaker : public I3Module {
public:
	EventMaker(const I3Context &ctx) : I3Module(ctx), run_number_(0), event_number_(0)
	{
		AddParameter("EventRunNumber", "", run_number_); 
		AddParameter("EventTimeNnanosec", "", 0);
		AddParameter("EventTimeYear", "", 2007);
		AddOutBox("OutBox");
	}
	void Configure()
	{
		int year, event_time_ns;
		
		GetParameter("EventRunNumber", run_number_); 
		GetParameter("EventTimeNnanosec", event_time_ns);
		GetParameter("EventTimeYear", year);
		
		event_time_ = I3Time(year, event_time_ns);
	}
	void Physics(I3FramePtr frame)
	{
		I3EventHeaderPtr header = boost::make_shared<I3EventHeader>();
		header->SetStartTime(event_time_);
		header->SetRunID(run_number_);
		header->SetSubRunID(0);
		header->SetEventID(event_number_++);
		
		frame->Put<I3EventHeader>(header);
		
		PushFrame(frame);
	}
private:
	size_t run_number_, event_number_;
	I3Time event_time_;
};

I3_MODULE(EventMaker);
