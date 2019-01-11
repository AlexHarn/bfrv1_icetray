#include <I3Test.h>

#include "vemcal/I3VEMCalData.h"
#include <icetray/I3Tray.h>
#include <icetray/I3Module.h>


TEST_GROUP(I3VEMCalData);


class VEMCalPutter : public I3Module {
public:
  VEMCalPutter(const I3Context &ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void DAQ(I3FramePtr frame)
  {
    I3VEMCalDataPtr vemcal(new I3VEMCalData);
    vemcal->runID = 1;

    I3VEMCalData::MinBiasHit mbHit;
    mbHit.str = 61;
    mbHit.om = 21;
    mbHit.chip = 1;  // == chip b
    mbHit.channel = 1;
    mbHit.charge_dpe = 1234;
    vemcal->minBiasHits.push_back(mbHit);

    I3VEMCalData::HGLGhit hglgHit;
    hglgHit.str = 61;
    hglgHit.hg_om = 23;
    hglgHit.hg_chip = 0;
    hglgHit.hg_channel = 2;
    hglgHit.hg_charge_pe = 200;
    hglgHit.lg_om = 24;
    hglgHit.lg_chip = 1;
    hglgHit.lg_channel = 0;
    hglgHit.lg_charge_pe = 200;
    hglgHit.deltat_2ns = 10;
    vemcal->hglgHits.push_back(hglgHit);

    frame->Put("VEMCalData", vemcal);
    PushFrame(frame);
  }
};

I3_MODULE(VEMCalPutter);


class VEMCalChecker : public I3Module {
public:
  VEMCalChecker(const I3Context &ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void DAQ(I3FramePtr frame)
  {
    ENSURE(frame->Has("VEMCalData"), "VEMCalData not in frame");

    const I3VEMCalData &vemcal = frame->Get<I3VEMCalData>("VEMCalData");
    ENSURE_EQUAL(vemcal.minBiasHits.size(), 1u, "Expected 1 min bias hit");

#define ENSURE_RESTORE(key, value) ENSURE_EQUAL(key, value, "Failed to restore " #key)

    const I3VEMCalData::MinBiasHit &mbHit(vemcal.minBiasHits[0]);
    ENSURE_RESTORE(mbHit.str, 61);
    ENSURE_RESTORE(mbHit.om, 21);
    ENSURE_RESTORE(mbHit.chip, 1);
    ENSURE_RESTORE(mbHit.channel, 1);
    ENSURE_RESTORE(mbHit.charge_dpe, 1234);

    ENSURE_EQUAL(vemcal.hglgHits.size(), 1u, "Expected 1 HGLG hit pair");
    
    const I3VEMCalData::HGLGhit &hglgHit(vemcal.hglgHits[0]);
    ENSURE_RESTORE(hglgHit.str, 61);
    ENSURE_RESTORE(hglgHit.hg_om, 23);
    ENSURE_RESTORE(int(hglgHit.hg_chip), 0);
    ENSURE_RESTORE(hglgHit.hg_channel, 2);
    ENSURE_RESTORE(hglgHit.hg_charge_pe, 200);
    ENSURE_RESTORE(hglgHit.lg_om, 24);
    ENSURE_RESTORE(hglgHit.lg_chip, 1);
    ENSURE_RESTORE(hglgHit.lg_channel, 0);
    ENSURE_RESTORE(hglgHit.lg_charge_pe, 200);
    ENSURE_RESTORE(hglgHit.deltat_2ns, 10);
  }
};

I3_MODULE(VEMCalChecker);


TEST(resurrection)
{
  const char *fileName = "vemcal_test_I3VEMCalData.i3";

  I3Tray tray1;
  tray1.AddModule("I3InfiniteSource", "Source")
    ("Stream", I3Frame::DAQ);
  tray1.AddModule("VEMCalPutter", "Putter");
  tray1.AddModule("I3Writer", "Writer")
    ("FileName", fileName);
  tray1.Execute(1);

  I3Tray tray2;
  tray2.AddModule("I3Reader", "Reader")
    ("FileName", fileName);
  tray2.AddModule("VEMCalChecker", "Checker");
  tray2.Execute();
  std::remove(fileName);
}
