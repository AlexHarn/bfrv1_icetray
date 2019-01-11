#include <polyplopia/PolyplopiaUtils.h>
#include <polyplopia/CoincidentI3ReaderService.h>
#include <dataclasses/physics/I3MCTreeUtils.h>
#include <icetray/I3Units.h>
#include <icetray/I3SingleServiceFactory.h>

CoincidentI3ReaderService::~CoincidentI3ReaderService()
{
    reader_.close();
}

CoincidentI3ReaderService::CoincidentI3ReaderService(const I3Context& ctx)
: I3GeneratorService(ctx),
rate_(NAN),
configured_(false)
{
        AddParameter("FileName","Input file from which to read background events","");
}

CoincidentI3ReaderService::CoincidentI3ReaderService(const std::string& filename)
    : path_(filename), 
    rate_(NAN), 
    configured_(false)
{
}

void
CoincidentI3ReaderService::Configure(){
        GetParameter("FileName",path_);
        stager_ = context_.Get<boost::shared_ptr<I3FileStager>>();
}

I3MCTreePtr 
CoincidentI3ReaderService::GetNextEvent() 
{ 
        if (!configured_) { 
                Init(); 
        } 
        I3FramePtr frame = reader_.pop_frame(I3Frame::DAQ); 
        if (!frame) { 
                log_fatal("I have run out of frames!!! %s",path_.c_str() ); 
        } 
        if (!frame->Has("I3MCTree")) { 
                log_fatal("No I3MCTree found in frame on file %s",path_.c_str() ); 
        } 
        I3MCTreeConstPtr tree = I3MCTreeUtils::Get(*frame,"I3MCTree"); 
        return I3MCTreePtr(new I3MCTree(*tree));
}

I3FramePtr 
CoincidentI3ReaderService::GetNextFrame() 
{ 
        if (!configured_) { 
                Init(); 
        } 
        return reader_.pop_frame(I3Frame::DAQ);
}

double 
CoincidentI3ReaderService::GetRate() 
{ 
        if (!configured_) { 
                Init(); 
        } 
        return rate_;
}

int 
CoincidentI3ReaderService::Init() 
{ 

        if (configured_) return 0; 
        if (path_.empty()) {
                log_fatal("No i3 file was given."); 
        }
        
        if (!stager_)
                stager_ = I3TrivialFileStager::create();
    
        file_ref_=stager_->GetReadablePath(path_);
	
        reader_.add_file(*file_ref_);
        I3FramePtr sample_frame = reader_.pop_frame(I3Frame::DAQ); 
        if (!sample_frame->Has("CorsikaWeightMap")) { 
                log_fatal("No CoriskaWeightMap found. Unable to calculate rate"); 
        } 
        if (sample_frame->Has("I3MCPESeriesMap")) { 
                log_warn("File %s appears to already have photon propagation.",
                                path_.c_str()); 
        } 
        
        I3MapStringDouble weights = sample_frame->Get<I3MapStringDouble>("CorsikaWeightMap"); 
        unsigned int bgevents=0; 
        double time_scale = weights["TimeScale"]*I3Units::second; 

        log_info("Calculating event rate..."); 
        while (reader_.more()) {
                ++bgevents; 
                sample_frame = reader_.pop_frame(I3Frame::DAQ); 
        } 
        rate_ = bgevents/time_scale; 
        log_info("TimeScale=%gs, Nevents=%u, Rate=%gHz",time_scale/I3Units::second,bgevents,rate_/I3Units::hertz);

        // jump back to the beginning of the file
        reader_.rewind();
        configured_ = true; 
        return 1;
}

bool 
CoincidentI3ReaderService::Open(std::string path) 
{ 
        path_ = path; 
        if (configured_) { 
                reader_.close(); 
                configured_ = false; 
        } 
        return true;
}

using CoincidentI3ReaderServiceFactory = I3SingleServiceFactory<CoincidentI3ReaderService,I3GeneratorService>;
I3_SERVICE_FACTORY(CoincidentI3ReaderServiceFactory);
