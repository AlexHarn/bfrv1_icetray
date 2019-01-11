#include <I3Test.h>
#include <icetray/I3Tray.h>
#include "paraboloid/I3ParaboloidFitParams.h"

TEST_GROUP(I3ParaboloidFitParamsTest)

void cmp_params(const I3ParaboloidFitParams &p1,
		const I3ParaboloidFitParams &p2){
  ENSURE_EQUAL(p1,p2, "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfStatus_   ,p2.pbfStatus_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfZen_      ,p2.pbfZen_      , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfAzi_      ,p2.pbfAzi_      , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfErr1_     ,p2.pbfErr1_     , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfErr2_     ,p2.pbfErr2_     , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfRotAng_   ,p2.pbfRotAng_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfCenterLlh_,p2.pbfCenterLlh_, "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfLlh_      ,p2.pbfLlh_      , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfZenOff_   ,p2.pbfZenOff_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfAziOff_   ,p2.pbfAziOff_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfCurv11_   ,p2.pbfCurv11_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfCurv12_   ,p2.pbfCurv12_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfCurv22_   ,p2.pbfCurv22_   , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfChi2_     ,p2.pbfChi2_     , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfDetCurvM_ ,p2.pbfDetCurvM_ , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfSigmaZen_ ,p2.pbfSigmaZen_ , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfSigmaAzi_ ,p2.pbfSigmaAzi_ , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfCovar_    ,p2.pbfCovar_    , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfTrOffZen_ ,p2.pbfTrOffZen_ , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfTrOffAzi_ ,p2.pbfTrOffAzi_ , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfTrZen_    ,p2.pbfTrZen_    , "Deserialized fit status is different");
  ENSURE_EQUAL(p1.pbfTrAzi_    ,p2.pbfTrAzi_    , "Deserialized fit status is different");
}

TEST(SerializeSingleElement)
{
  
  I3ParaboloidFitParams p1,p2;

  p1.pbfStatus_   = I3ParaboloidFitParams::PBF_SUCCESS;
  p1.logl_        =  1;
  p1.rlogl_       =  2;
  p1.ndof_        =  3;
  p1.nmini_       =  4;
  p1.pbfZen_      =  5;
  p1.pbfAzi_      =  6;
  p1.pbfErr1_     =  7;
  p1.pbfErr2_     =  8;
  p1.pbfRotAng_   =  9;
  p1.pbfCenterLlh_= 10;
  p1.pbfLlh_      = 11;
  p1.pbfZenOff_   = 12;
  p1.pbfAziOff_   = 13;
  p1.pbfCurv11_   = 14;
  p1.pbfCurv12_   = 15;
  p1.pbfCurv22_   = 16;
  p1.pbfChi2_     = 17;
  p1.pbfDetCurvM_ = 18;
  p1.pbfSigmaZen_ = 19;
  p1.pbfSigmaAzi_ = 20;
  p1.pbfCovar_    = 21;  
  p1.pbfTrOffZen_ = 22;
  p1.pbfTrOffAzi_ = 23;
  p1.pbfTrZen_    = 24;
  p1.pbfTrAzi_    = 25;
  
  std::ostringstream oarchive_stream;
  /* This is the archive type instantiated by I3_SERIALIZABLE */
  icecube::archive::portable_binary_oarchive oarchive(oarchive_stream);
  oarchive << p1;
  
  std::istringstream iarchive_stream(oarchive_stream.str());
  icecube::archive::portable_binary_iarchive iarchive(iarchive_stream);
  iarchive >> p2;
  
  cmp_params(p1,p2);
  
}

class ParaboloidFitParamsTestModule : public I3Module
{
public:
  ParaboloidFitParamsTestModule(const I3Context& context): I3Module(context)
  {
    AddOutBox("OutBox");
  }
  
  void Physics(I3FramePtr frame)
  {
    const I3ParaboloidFitParams &p1 =frame->Get<I3ParaboloidFitParams>("MPEFitParaboloidFitParams");
    std::ostringstream oarchive_stream;
    
    icecube::archive::portable_binary_oarchive oarchive(oarchive_stream);
    oarchive << p1;
    
    std::istringstream iarchive_stream(oarchive_stream.str());
    icecube::archive::portable_binary_iarchive iarchive(iarchive_stream);
    
    I3ParaboloidFitParams p2;
    iarchive >> p2;
    
    cmp_params(p1,p2);
    
    PushFrame(frame,"OutBox");
  }
};

I3_MODULE(ParaboloidFitParamsTestModule);

TEST(SerializeFramesFromNuGen)
{
  I3Tray tray;
  
  std::string filename(getenv("I3_TESTDATA"));
  filename+="/sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2";
  
  tray.AddModule("I3Reader","reader")
    ("FileName",filename);
  
  tray.AddModule("ParaboloidFitParamsTestModule","client");
  
  tray.Execute();
  
}
