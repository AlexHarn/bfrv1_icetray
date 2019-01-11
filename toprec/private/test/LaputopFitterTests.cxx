#include <I3Test.h>
#include <icetray/I3Tray.h>
#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"

#include "lilliput/minimizer/I3GulliverMinuit.h"
#include "recclasses/I3LaputopParams.h"
#include "toprec/I3LaputopFitter.h"


// I stole these from an IT73 event, from my little testfile "Level2a_Laputop_IT73_0718_116195.i3"
// The pulseseries IceTopVEMPulses_0 in the first event.
// It contains 18 pulses.
I3RecoPulseSeriesMapPtr testPSM() {
  I3RecoPulseSeriesMapPtr psm(new I3RecoPulseSeriesMap);
  OMKey key;
  I3RecoPulse pulse;
  I3RecoPulseSeries ps;
  key.SetString(43);  key.SetOM(61);
  pulse.SetTime(10572.57);  pulse.SetCharge(3.0177093);  pulse.SetWidth(153.87358);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(43);  key.SetOM(63);
  pulse.SetTime(10611.964);  pulse.SetCharge(0.31476855);  pulse.SetWidth(128.18964);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(52);  key.SetOM(61);
  pulse.SetTime(10569.023);  pulse.SetCharge(1.2578508);  pulse.SetWidth(222.78705);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(52);  key.SetOM(63);
  pulse.SetTime(10589.027);  pulse.SetCharge(0.45404792);  pulse.SetWidth(133.80554);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(53);  key.SetOM(61);
  pulse.SetTime(10326.345);  pulse.SetCharge(2.4605699);  pulse.SetWidth(153.76886);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(53);  key.SetOM(63);
  pulse.SetTime(10342.751);  pulse.SetCharge(1.076555);  pulse.SetWidth(155.85727);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(54);  key.SetOM(61);
  pulse.SetTime(10197.892);  pulse.SetCharge(0.1957562);  pulse.SetWidth(156.05147);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(54);  key.SetOM(63);
  pulse.SetTime(10191.37);  pulse.SetCharge(0.58932847);  pulse.SetWidth(134.95946);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(61);  key.SetOM(61);
  pulse.SetTime(10305.588);  pulse.SetCharge(6.5178876);  pulse.SetWidth(132.24896);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(61);  key.SetOM(63);
  pulse.SetTime(10319.602);  pulse.SetCharge(2.2484291);  pulse.SetWidth(152.80693);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(62);  key.SetOM(61);
  pulse.SetTime(10143.537);  pulse.SetCharge(27.503639);  pulse.SetWidth(142.91077);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(62);  key.SetOM(64);   // This one is high-gain...
  pulse.SetTime(10145.445);  pulse.SetCharge(24.468243);  pulse.SetWidth(139.28784);  pulse.SetFlags(3);
  pulse.SetTime(10145.445);  pulse.SetCharge(100000);  pulse.SetWidth(139.28784);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(63);  key.SetOM(61);
  pulse.SetTime(9872.1543);  pulse.SetCharge(0.84486663);  pulse.SetWidth(129.31296);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(63);  key.SetOM(63);
  pulse.SetTime(9899.5029);  pulse.SetCharge(0.37883031);  pulse.SetWidth(166.07344);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(69);  key.SetOM(61);
  pulse.SetTime(10129.423);  pulse.SetCharge(0.35165673);  pulse.SetWidth(219.39558);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(69);  key.SetOM(63);
  pulse.SetTime(10134.228);  pulse.SetCharge(0.42961508);  pulse.SetWidth(178.56526);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(75);  key.SetOM(61);
  pulse.SetTime(10126.95);  pulse.SetCharge(NAN);  pulse.SetWidth(130.26392);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(75);  key.SetOM(63);
  pulse.SetTime(10110.982);  pulse.SetCharge(0.62996495);  pulse.SetWidth(156.70247);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  return psm;
  }

TEST_GROUP(FrontToBack);

class LaputopPrep : public I3Module {
 public:
  LaputopPrep(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {

    // Add some pulses
    I3RecoPulseSeriesMapConstPtr psmptr = testPSM();
    ENSURE(psmptr->size() == 18);
    frame->Put("FakePulseSeriesMap",psmptr);

    // Add some seed tracks
    I3ParticlePtr p(new I3Particle);
    p->SetDir(0.8, 0.7);
    p->SetPos(-200.0, 200.0, 1945.0);
    p->SetTime(10000);
    p->SetFitStatus(I3Particle::OK);
    frame->Put("Seed", p);
    PushFrame(frame);
  }
};

class LaputopTest : public I3Module {
 public:
  LaputopTest(const I3Context& ctx) : I3Module(ctx)
  {
    AddOutBox("OutBox");
  }

  void Physics(I3FramePtr frame) {

    // Test the results of the Laputop fit
    const I3Particle& track = frame->Get<I3Particle>("Laputop");
    printf("(%f %f %f) %f %f, and %f\n", 
	   track.GetX(), track.GetY(), track.GetZ(), track.GetZenith(), track.GetAzimuth(), track.GetTime());
    // KR 1/12/2014: Fit results change somewhat when I3RecoPulse time changes from float->double
    // Old values: (-225.574201 206.271926 1945.0) 0.793286, 0.778915, and 10232.538042
    // New values: (-224.706172 205.866159 1945.0) 0.793182 0.781984, and 10231.552258
    ENSURE_DISTANCE(track.GetX(), -224.706172, 0.0001);  
    ENSURE_DISTANCE(track.GetY(), 205.866159, 0.0001);  
    ENSURE_DISTANCE(track.GetZ(), 1945.0, 0.01);  
    ENSURE_DISTANCE(track.GetZenith(), 0.793182, 0.00001);  
    ENSURE_DISTANCE(track.GetAzimuth(), 0.781984, 0.00001);
    ENSURE_DISTANCE(track.GetTime(), 10231.552258, 0.00001);  
    // Old params:
    //const I3TopLateralFitParams& params = frame->Get<I3TopLateralFitParams>("LaputopParams");
    // New params:
    const I3LaputopParams& params = frame->Get<I3LaputopParams>("LaputopParams");
    printf("[%f %f]\n", 
	   params.GetValue(Laputop::Parameter::Log10_S125), 
	   params.GetValue(Laputop::Parameter::Beta));
    // Again, float->double in I3RecoPulse changes these slightly:
    // Old values: [0.645507 2.773615 1.5]
    // New values: [0.646655 2.766762 1.5]
    ENSURE_DISTANCE(params.GetValue(Laputop::Parameter::Log10_S125), log10(0.646655), 0.00001);  
    ENSURE_DISTANCE(params.GetValue(Laputop::Parameter::Beta), 2.766762, 0.00001);  

    PushFrame(frame);
  }
};

I3_MODULE(LaputopPrep);
I3_MODULE(LaputopTest);

TEST(run_it)
{
  I3Tray tray;
  std::string gcd(getenv("I3_TESTDATA"));
  gcd = gcd+"/sim/GeoCalibDetectorStatus_IC79.55380_corrected.i3.gz";
  tray.AddModule("I3InfiniteSource", "source");
  tray.SetParameter("source","Stream",I3Frame::Physics);
  tray.SetParameter("source","Prefix",gcd);

  tray.AddModule("LaputopPrep", "prepit");

  //tray.AddSegment(LaputopStandard,"Laputop")("pulses", "FakePulseSeriesMap");

  bool fixcore = 0;
  bool fitsnow = 0;
  std::string name = "Laputop";

    // This one is the standard one.
  tray.AddService("I3GulliverMinuitFactory",name+"Minuit");
  tray.SetParameter(name+"Minuit","MinuitPrintLevel",-2);
  tray.SetParameter(name+"Minuit","FlatnessCheck",1);  
  tray.SetParameter(name+"Minuit","Algorithm","SIMPLEX");  
  tray.SetParameter(name+"Minuit","MaxIterations",1000);
  tray.SetParameter(name+"Minuit","MinuitStrategy",2);
  tray.SetParameter(name+"Minuit","Tolerance",0.01);    
  
  // New (Feb 2014): The snow correction service
  tray.AddService("I3SimpleSnowCorrectionServiceFactory",name+"SimpleSnow");
  tray.SetParameter(name+"SimpleSnow","Lambda",1.5);
    
    // The Seed service
  tray.AddService("I3LaputopSeedServiceFactory",name+"ToprecSeed");
  tray.SetParameter(name+"ToprecSeed","InCore", "Seed");
  tray.SetParameter(name+"ToprecSeed","InPlane", "Seed");
  //tray.SetParameter(name+"ToprecSeed","SnowCorrectionFactor", 1.5);   // <--- put your snow correction factor here; NOW IRRELEVANT
  tray.SetParameter(name+"ToprecSeed","Beta",2.6);                    // first guess for Beta
  tray.SetParameter(name+"ToprecSeed","InputPulses", "FakePulseSeriesMap");  // thisll let it first-guess at S125 automatically
    
    // Step 1 AND 2 (they are the same):
  tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam2");
  tray.SetParameter(name+"ToprecParam2","FixCore", fixcore);        
  tray.SetParameter(name+"ToprecParam2","FixTrackDir", 1);
  tray.SetParameter(name+"ToprecParam2","FitSnowCorrection", fitsnow);
  tray.SetParameter(name+"ToprecParam2","IsBeta", 1);
  tray.SetParameter(name+"ToprecParam2","MinBeta", 2.9);   // From toprec... 2nd iteration (DLP, using beta)
  tray.SetParameter(name+"ToprecParam2","MaxBeta", 3.1);
  tray.SetParameter(name+"ToprecParam2","LimitCoreBoxSize", -1); 

   // Step 3:
  tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam3");
  tray.SetParameter(name+"ToprecParam3","FixCore", fixcore);        
  tray.SetParameter(name+"ToprecParam3","FixTrackDir", 0);      // FREE THE DIRECTION!
  tray.SetParameter(name+"ToprecParam3","FitSnowCorrection", fitsnow);
  tray.SetParameter(name+"ToprecParam3","IsBeta", 1);
  tray.SetParameter(name+"ToprecParam3","MinBeta", 2.0);   // From toprec... 3rd iteration (DLP, using beta)
  tray.SetParameter(name+"ToprecParam3","MaxBeta", 4.0);
  tray.SetParameter(name+"ToprecParam3","LimitCoreBoxSize", 15.0);  // these two together mimic the "+/- 3 sigma thing
  tray.SetParameter(name+"ToprecParam3","VertexStepsize",5.0);
  // Use these smaller stepsizes instead of the defaults:
  tray.SetParameter(name+"ToprecParam3","SStepsize", 0.045);        // default is 1
  tray.SetParameter(name+"ToprecParam3","BetaStepsize",0.15);        // default is 0.6    


    // Step 4:
  tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam4");
  tray.SetParameter(name+"ToprecParam4","FixCore", fixcore);        
  tray.SetParameter(name+"ToprecParam4","FixTrackDir", 1);
  tray.SetParameter(name+"ToprecParam4","FitSnowCorrection", fitsnow);
  tray.SetParameter(name+"ToprecParam4","IsBeta", 1);
  tray.SetParameter(name+"ToprecParam4","MinBeta", 1.5);   // From toprec... 4th iteration (DLP, using beta)
  tray.SetParameter(name+"ToprecParam4","MaxBeta", 5.0);
  tray.SetParameter(name+"ToprecParam4","LimitCoreBoxSize", -1);
  // Use these smaller stepsizes instead of the defaults:
  tray.SetParameter(name+"ToprecParam4","VertexStepsize", 4.0);     // default is 20
  tray.SetParameter(name+"ToprecParam4","SStepsize", 0.045);        // default is 1
  tray.SetParameter(name+"ToprecParam4","BetaStepsize",0.15);        // default is 0.6 
      
  // The likelihood service
  tray.AddService("I3LaputopLikelihoodServiceFactory",name+"ToprecLike2");
  tray.SetParameter(name+"ToprecLike2","DataReadout", "FakePulseSeriesMap");
  tray.SetParameter(name+"ToprecLike2","DynamicCoreTreatment", 11.0);     // don't do dynamic core things
  tray.SetParameter(name+"ToprecLike2","SaturationLikelihood", 1);
  tray.SetParameter(name+"ToprecLike2","Curvature","");      // NO timing likelihood (at first; this will be overridden)
  tray.SetParameter(name+"ToprecLike2","SnowServiceName",name+"SimpleSnow");  // New!

  // This module performs the three steps
  std::vector<std::string> vldf; vldf.push_back("dlp");vldf.push_back("dlp");vldf.push_back("dlp");
  std::vector<std::string> vcurv; vcurv.push_back(""); vcurv.push_back("gausspar");vcurv.push_back("gausspar");
  tray.AddModule("I3LaputopFitter",name);
  tray.SetParameter(name, "SeedService",name+"ToprecSeed");
  tray.SetParameter(name, "NSteps",3);            // <--- tells it how many services to look for and perform
  tray.SetParameter(name, "Parametrization1",name+"ToprecParam2");   // the parametrizations
  tray.SetParameter(name, "Parametrization2",name+"ToprecParam3");
  tray.SetParameter(name, "Parametrization3",name+"ToprecParam4");
  tray.SetParameter(name, "StoragePolicy","OnlyBestFit");
  tray.SetParameter(name, "Minimizer",name+"Minuit");
  tray.SetParameter(name, "LogLikelihoodService",name+"ToprecLike2");     // the four likelihoods
  tray.SetParameter(name, "LDFFunctions",vldf);
  tray.SetParameter(name, "CurvFunctions",vcurv);   // VERY IMPORTANT : use time Llh for step 3, but fix direction!
  
  tray.AddModule("LaputopTest", "testit");

  //tray.AddModule("Dump","dumpit");

  
    
  tray.Execute(4);
  
}


				     ;
