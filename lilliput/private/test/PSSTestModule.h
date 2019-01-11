#ifndef PSSTESTMODULE_H_INCLUDED
#define PSSTESTMODULE_H_INCLUDED

#include "icetray/I3Module.h"
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include <string>
#include "gulliver/I3EventLogLikelihoodBase.h"

class I3Geometry;

class PSSTestModule: public I3Module {
private:
    std::string ipdfName_;
    std::string trackName_;
    I3EventLogLikelihoodBasePtr ipdf_;
    int n_;

public:
    PSSTestModule(const I3Context &context);
    ~PSSTestModule();
    void Configure();
    void Geometry(I3FramePtr frame);
    void Physics(I3FramePtr frame);
    void Finish();
    SET_LOGGER("PSSTestModule");
};

#endif /* PSSTESTMODULE_H_INCLUDED */
