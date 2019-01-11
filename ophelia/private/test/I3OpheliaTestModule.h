#ifndef I3OPHELIA_TEST_MODULE_H_INCLUDED
#define I3OPHELIA_TEST_MODULE_H_INCLUDED

#include<icetray/I3Module.h>

class I3OpheliaTestModule : public I3Module
{

 public:

  I3OpheliaTestModule(const I3Context& context);

  void Configure();

  void Physics(I3FramePtr frame);

};

#endif

