#include "PSSTestModule.h"
#include "gulliver/I3EventHypothesis.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"

PSSTestModule::PSSTestModule(const I3Context &context):I3Module(context){
    AddOutBox("OutBox");
    AddParameter("IPDFName","Name of ipdf service", ipdfName_ );
    AddParameter("TrackName","Name of track", trackName_ );
    n_ = 0;
}

PSSTestModule::~PSSTestModule(){}

void PSSTestModule::Configure(){
    GetParameter("IPDFName", ipdfName_ );
    GetParameter("TrackName", trackName_ );
    ipdf_ = context_.Get<I3EventLogLikelihoodBasePtr>(ipdfName_);
    if (!ipdf_){
        log_fatal("llh service \"%s\" not found", ipdfName_.c_str());
    }
}

void PSSTestModule::Geometry(I3FramePtr frame){
    const I3Geometry& geo = frame->Get<I3Geometry>();
    ipdf_->SetGeometry(geo);
    PushFrame(frame, "OutBox");
}

void PSSTestModule::Physics(I3FramePtr frame){
    n_+=1;
    ipdf_->SetEvent(*frame);
    I3EventHypothesis eh(*(frame->Get<I3ParticleConstPtr>(trackName_)));
    double llh = ipdf_->GetLogLikelihood(eh);
    log_trace("%d. got llh=%g",n_,llh);
    PushFrame(frame, "OutBox");
}

void PSSTestModule::Finish(){
    if (n_>0){
        log_trace("saw %d physics frames", n_);
    } else {
        log_fatal("saw ZERO NADA physics frames");
    }
}

I3_MODULE(PSSTestModule);
