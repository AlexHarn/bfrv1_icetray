#ifndef MUONL3_IC86_RECOPULSETRUNCATIONFIXER_H_INCLUDED
#define MUONL3_IC86_RECOPULSETRUNCATIONFIXER_H_INCLUDED

#include <math.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Units.h>

class RecoPulseTruncationFixer : public I3ConditionalModule{
  public:
	RecoPulseTruncationFixer(const I3Context& context);
	void Configure();
	void Physics(const I3FramePtr frame);

  private:
	std::string inputPulsesName_;
	std::string outputPulsesName_;
};

I3_MODULE(RecoPulseTruncationFixer);

#endif // MUONL3_IC86_RECOPULSETRUNCATIONFIXER_H_INCLUDED
