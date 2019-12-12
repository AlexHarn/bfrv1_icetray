#include <vector>

#include <iostream>
#include <cmath>

#include "I3Test.h"
#include "icetray/I3Tray.h"

#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "icetray/OMKey.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include "photonics-service/I3PhotoSplineService.h"

#include "gulliver-modules/I3SimpleFitter.h"
#include "gulliver/I3LogLikelihoodFitParams.h"

#include "spline-reco/I3SplineRecoLikelihood.h"

TEST_GROUP(SplineRecoTest);

const unsigned n_frames=10;

const std::string pulsesName = "TestPulsesForSpline";
const std::string splineRecoName = "SplineRecoMPEFit";
const std::string testParticleName = "TestParticle";
// paths to bare muon splines
const std::string splineDir = std::string(getenv("I3_TESTDATA"))+"/photospline/";
const std::string timingSpline = splineDir+"ems_z0_a0.pt.prob.fits";
const std::string amplitudeSpline = splineDir+"ems_z0_a0.pt.abs.fits";

std::vector<OMKey> OMKeys()
{
  std::vector<OMKey> omkeys;
  for (int om=1; om <=6; om++){
    OMKey omkey(-1,om);
    omkeys.push_back(omkey);
  }
  return omkeys;
}

namespace SplineRecoTest {

  class RunProcess : public I3Module 
  {
  private:
    bool has_geometry_;

  public:
    RunProcess(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Configure(){
      has_geometry_=false;
    }
    void Process(){
      if (not has_geometry_){
	has_geometry_=true;

	I3GeometryPtr geometry(new I3Geometry);
	const std::vector<OMKey> &omkeys=OMKeys();

	for (size_t om=0; om < omkeys.size(); om++){
	  double x= 10.*(om+1);
	  double y= 5.*(om+1)*std::pow(-1,om);
	  double z= 9.*(om+1);

	  I3Position pos(x,y,z);
	  const OMKey &omkey= omkeys.at(om);
	  I3OMGeo domgeo;
	  geometry->omgeo[omkey]=domgeo;
	  geometry->omgeo[omkey].position = pos;
	}
	I3Frame::Stream geo_stream= I3Frame::Geometry;
	I3FramePtr geo_frame(new I3Frame(geo_stream));
	geo_frame->Put("I3Geometry",geometry);
	PushFrame(geo_frame);
      }
      I3Frame::Stream frame_stream= I3Frame::Physics;
      I3FramePtr frame(new I3Frame(frame_stream));
      PushFrame(frame);
    }
  };

  class CreateTrack : public I3Module {
  public:
    CreateTrack(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      I3ParticlePtr part(new I3Particle());
      part->SetTime(42.);
      part->SetDir(0.0308507, 3.04555);
      part->SetPos(-120.945, 30.9894, 335.954);
      part->SetEnergy(100*I3Units::GeV);
      part->SetSpeed(I3Constants::c);
      part->SetLength(NAN);
      part->SetType(I3Particle::unknown);
      part->SetPdgEncoding(0);
      part->SetShape(I3Particle::InfiniteTrack);
      part->SetFitStatus(I3Particle::OK);
      part->SetLocationType(I3Particle::Anywhere);
      frame->Put(testParticleName, part);
      PushFrame(frame, "OutBox");
    }
  };
  
  
  //Generate some test pulses
  class CreatePulseSeries : public I3Module {
  public:
    CreatePulseSeries(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    
    void Physics(I3FramePtr frame) {

      const std::vector<OMKey> &omkeys=OMKeys();
      OMKey om1=omkeys.at(0);
      OMKey om2=omkeys.at(1);
      OMKey om3=omkeys.at(2);
      OMKey om4=omkeys.at(3);
      OMKey om5=omkeys.at(4);
      OMKey om6=omkeys.at(5);

      I3RecoPulseSeriesMapPtr newMap(new I3RecoPulseSeriesMap);
      vector<I3RecoPulse> hits1, hits2, hits3, hits4, hits5, hits6;
      I3RecoPulse hit1, hit2, hit3, hit4, hit5, hit6;
      
      hit1.SetCharge( 1);
      hit1.SetTime(100);
      hit2.SetCharge(2);
      hit2.SetTime(200);
      hit3.SetCharge(3);
      hit3.SetTime(300);
      hit4.SetCharge(11);
      hit4.SetTime(400);
      hit5.SetCharge(10);
      hit5.SetTime(500);
      hit6.SetCharge(10);
      hit6.SetTime(1000);
      
      hits1.push_back(hit1);
      hits1.push_back(hit2);
      hits1.push_back(hit3);
      hits2.push_back(hit2);
      hits3.push_back(hit3);
      hits4.push_back(hit4);
      hits5.push_back(hit5);
      hits6.push_back(hit6);
      
      (*newMap)[om1] = hits1;
      (*newMap)[om2] = hits2;
      (*newMap)[om3] = hits3;
      (*newMap)[om4] = hits4;
      (*newMap)[om5] = hits5;
      (*newMap)[om6] = hits6;
            
      frame->Put(pulsesName, newMap);
      PushFrame(frame);
    }
  };
  
  class SetValueToNaN : public I3Module {
  public:
    SetValueToNaN(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
      AddParameter("ValueName", "Name of the particle property you want to set to NaN.", "x");
    }
    void Configure() {
      GetParameter("ValueName", value_name_);
    }
    void Physics(I3FramePtr frame) {
      I3Particle * particle = new I3Particle( frame->Get<I3Particle>(testParticleName) );
      I3Position vertex(particle->GetPos());
      if (value_name_ == "x"){ vertex.SetX(NAN); }
      else if (value_name_ == "y"){ vertex.SetY(NAN); }
      else if (value_name_ == "z"){ vertex.SetZ(NAN); }
      particle->SetPos(vertex);
      double zenith = particle->GetZenith();
      double azimuth = particle->GetAzimuth();
      if (value_name_ == "zen"){ zenith = NAN; }
      else if (value_name_ == "azi"){ azimuth = NAN; }
      particle->SetDir(zenith, azimuth);
      frame->Delete(testParticleName);
      frame->Put(testParticleName, I3ParticlePtr(particle));
      PushFrame(frame, "OutBox");
    }
  private:
    std::string value_name_;
  };
  
  // Test if spline-reco properly handles the case where the I3Geometry
  // isn't in the frame
  class NoI3Geometry : public I3Module {
  public:
    NoI3Geometry(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      I3SplineRecoLikelihood splineLlh;
      /***** check if missing I3Geometry is handled ******/
      bool fatal_thrown = false;
      try {
	splineLlh.SetEvent(*frame);
      }
      catch (std::runtime_error&) {
	// good, we probably had a log_fatal
	fatal_thrown = true;
      } catch (...) {
	FAIL("The frame has no 'I3Geometry', and spline-reco should have thrown log_fatal, but it didn't! Sth. else went wrong!");
      }
      if (!fatal_thrown){
	FAIL("The frame has no 'I3Geometry', and spline-reco should have thrown log_fatal, but it didn't!");
      }
      PushFrame(frame);
    }
  };
  
  // Test if spline-reco fatals when given an unknown Llh name
  class WrongLlh : public I3Module {
  public:
    WrongLlh(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      I3SplineRecoLikelihood splineLlh;
      /***** check if wrong Likelihood name is handled ******/
      bool fatal_thrown = false;
      try {
	splineLlh.llhChoice = "XxXBlablaXxX";
	splineLlh.SetEvent(*frame);
      }
      catch (std::runtime_error&) {
	// good, we probably had a log_fatal
	fatal_thrown = true;
      } catch (...) {
	FAIL("spline-reco was configured with wrong 'Likelihood', and spline-reco should have thrown log_fatal, but it didn't! Sth. else went wrong!");
      }
      if (!fatal_thrown){
	FAIL("spline-reco was configured with wrong 'Likelihood', and spline-reco should have thrown log_fatal, but it didn't!");
      }
      PushFrame(frame);
    }
  };

  // Test if spline-reco properly handles the case where the PulseSeries
  // isn't in the frame
  class NoPulses : public I3Module {
  public:
    NoPulses(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      bool hasParticle = frame->Has(splineRecoName);
      bool hasFitParams = frame->Has(splineRecoName+"FitParams");
      ENSURE(hasParticle, "spline-reco should have put an I3Particle into the frame.");
      ENSURE(hasFitParams, "spline-reco should have put I3LogLikelihoodFitParams into the frame.");
      I3Particle particle = frame->Get<I3Particle>(splineRecoName);
      ENSURE(particle.GetFitStatus() == I3Particle::FailedToConverge, "spline-reco fit status should not have converged.");
      PushFrame(frame);
    }
  };

  class SomethingIsNaN : public I3Module {
  public:
    SomethingIsNaN(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      I3SplineRecoLikelihood splineLlh;
      splineLlh.pulses_name = pulsesName;
      splineLlh.llhChoice = "MPE";
      splineLlh.SetEvent(*frame);
      I3Particle particle = frame->Get<I3Particle>(testParticleName);
      I3EventHypothesis hyp(particle);
      bool fatal_thrown = false;
      try {
	splineLlh.GetLogLikelihood(hyp);
      }
      catch (std::runtime_error&) {
	// good, we probably had a log_fatal
	fatal_thrown = true;
      } catch (...) {
	FAIL("spline-reco was configured with seed particle having a NaN value, and spline-reco should have thrown log_fatal, but it didn't! Sth. else went wrong!");
      }
      if (!fatal_thrown){
	FAIL("spline-reco was configured with seed particle having a NaN value, and spline-reco should have thrown log_fatal, but it didn't!");
      }
      PushFrame(frame);
    }
  };
  
  // Test if spline-reco calculates the correct likelihood values
  class CheckLikelihood : public I3Module {
  public:
    CheckLikelihood(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    
    void Physics(I3FramePtr frame) {
            
      // setup SplineRecoLikelihood
      I3SplineRecoLikelihood splineLlh;
      splineLlh.pulses_name = pulsesName;
      splineLlh.llhChoice = "MPE";
      splineLlh.SetEvent(*frame);
      I3Particle particle = frame->Get<I3Particle>(testParticleName);
      I3EventHypothesis hyp(particle);
      I3PhotonicsServicePtr ps_(new I3PhotoSplineService(amplitudeSpline, timingSpline, 4));
      splineLlh.ps = ps_;
      
      double epsilon = 0.00001;
      
      double llh = splineLlh.GetLogLikelihood(hyp);
      ENSURE_DISTANCE(llh, -110.5240844637142, epsilon);
          
      splineLlh.llhChoice = "MPEAll";
      splineLlh.SetEvent(*frame);
      llh = splineLlh.GetLogLikelihood(hyp);
      ENSURE_DISTANCE(llh, -147.365445951619, epsilon);
      
      splineLlh.llhChoice = "SPE1st";
      splineLlh.SetEvent(*frame);
      llh = splineLlh.GetLogLikelihood(hyp);
      ENSURE_DISTANCE(llh, -110.5240844637142, epsilon); 
      
      splineLlh.llhChoice = "SPEAll";
      splineLlh.SetEvent(*frame);
      llh = splineLlh.GetLogLikelihood(hyp);
      ENSURE_DISTANCE(llh, -147.365445951619, epsilon);
      
      PushFrame(frame);
    }
  };
  
  // Test if spline-reco calculates pulse multiplicity correctly
  class GetMultiplicity : public I3Module {
  public:
    GetMultiplicity(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      // setup I3SplineRecoLikelihood:
      I3SplineRecoLikelihood splineLlh;
      splineLlh.pulses_name = pulsesName;
      splineLlh.llhChoice = "SPE1st"; // should use NCh
      splineLlh.SetEvent(*frame);
      ENSURE_EQUAL(splineLlh.GetMultiplicity(), 6u, "NChannel of test pulses should be 6.");
      splineLlh.llhChoice = "MPE"; // should use NCh
      splineLlh.SetEvent(*frame);
      ENSURE_EQUAL(splineLlh.GetMultiplicity(), 6u, "NChannel of test pulses should be 6.");
      splineLlh.llhChoice = "MPEAll"; // should use NPulses
      splineLlh.SetEvent(*frame);
      ENSURE_EQUAL(splineLlh.GetMultiplicity(), 8u, "NPulses of test pulses should be 8.");
      splineLlh.chargeWeight = false;
      splineLlh.llhChoice = "SPEAll"; // should use NPulses
      splineLlh.SetEvent(*frame);
      ENSURE_EQUAL(splineLlh.GetMultiplicity(), 8u, "NPulses of test pulses should be 8.");
      splineLlh.chargeWeight = true;
      splineLlh.llhChoice = "SPEAll"; // should use NPulses, cahrge weighted
      splineLlh.SetEvent(*frame);
      ENSURE_EQUAL(splineLlh.GetMultiplicity(), 42u, "Charge weighted NPulses of test pulses should be 42.");
      PushFrame(frame);
    }
  };
  // Test if spline-reco behaves correctly under normal operation
  class NormalOperation : public I3Module {
  public:
    NormalOperation(const I3Context& ctx) : I3Module(ctx) {
      AddOutBox("OutBox");
    }
    void Physics(I3FramePtr frame) {
      bool hasParticle = frame->Has(splineRecoName);
      bool hasFitParams = frame->Has(splineRecoName+"FitParams");
      ENSURE(hasParticle, "spline-reco should have put an I3Particle into the frame.");
      ENSURE(hasFitParams, "spline-reco should have put I3LogLikelihoodFitParams into the frame.");
      I3Particle particle = frame->Get<I3Particle>(splineRecoName);
     
      ENSURE(particle.GetFitStatus() == I3Particle::OK, "spline-reco fit status should have been OK.");
      PushFrame(frame);
    }
  };
  
}

I3_MODULE(SplineRecoTest::RunProcess);
I3_MODULE(SplineRecoTest::CreateTrack);
I3_MODULE(SplineRecoTest::SetValueToNaN);
I3_MODULE(SplineRecoTest::CreatePulseSeries);
I3_MODULE(SplineRecoTest::NoI3Geometry);
I3_MODULE(SplineRecoTest::WrongLlh);
I3_MODULE(SplineRecoTest::NoPulses);
I3_MODULE(SplineRecoTest::SomethingIsNaN);
I3_MODULE(SplineRecoTest::CheckLikelihood);
I3_MODULE(SplineRecoTest::GetMultiplicity);
I3_MODULE(SplineRecoTest::NormalOperation);

void prepareSplineReco(I3Tray& tray) {
  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle");
  // bare muon spline with 4 ns convolution
  tray.AddService("I3PhotoSplineServiceFactory", "spline-reco-test_SplineService4")
    ("AmplitudeTable", amplitudeSpline) // path to amplitude spline (abs)
    ("TimingTable", timingSpline) // path to timing spline (prob)
    ("TimingSigma", 4); // pdf convolution in ns
  // seed service without vertex time shift, PandelMPE as first guess
  // used for the SplineMPE fit
  std::vector<std::string> firstGuesses;
  firstGuesses.push_back(testParticleName);
  tray.AddService( "I3BasicSeedServiceFactory", "spline-reco-test_MPESeedNoShift")("FirstGuesses", firstGuesses);
  // the spline reco likelihood doing a default MPE reco --> faster and worse than with all modifications
  tray.AddService("I3SplineRecoLikelihoodFactory", "spline-reco-test_SplineMPEllh");
  tray.SetParameter("spline-reco-test_SplineMPEllh", "PhotonicsService", "spline-reco-test_SplineService4"); // spline services
  tray.SetParameter("spline-reco-test_SplineMPEllh", "Pulses", pulsesName);
  tray.SetParameter("spline-reco-test_SplineMPEllh", "Likelihood", "MPE");
  tray.SetParameter("spline-reco-test_SplineMPEllh", "NoiseRate", 10*I3Units::hertz);
  tray.AddService("I3GulliverMinuitFactory", "mininame");
  tray.SetParameter("mininame", "Algorithm", "SIMPLEX");
  tray.SetParameter("mininame", "MaxIterations", 1000);
  tray.SetParameter("mininame", "Tolerance", 0.01);
  std::vector<double> bounds;
  bounds.push_back(-2000.*I3Units::m);
  bounds.push_back(2000.*I3Units::m);
  tray.AddService("I3SimpleParametrizationFactory", "paraname");
  tray.SetParameter("paraname", "StepX", 20.*I3Units::m);
  tray.SetParameter("paraname", "StepY", 20.*I3Units::m);
  tray.SetParameter("paraname", "StepZ", 20.*I3Units::m);
  tray.SetParameter("paraname", "StepZenith", 0.1*I3Units::radian);
  tray.SetParameter("paraname", "StepAzimuth", 0.2*I3Units::radian);
  tray.SetParameter("paraname", "BoundsX", bounds);
  tray.SetParameter("paraname", "BoundsY", bounds);
  tray.SetParameter("paraname", "BoundsZ", bounds);
}

void doSplineReco(I3Tray& tray) {
  prepareSplineReco(tray);
  tray.AddModule("I3SimpleFitter", splineRecoName);
  tray.SetParameter(splineRecoName, "OutputName", splineRecoName);
  tray.SetParameter(splineRecoName, "SeedService", "spline-reco-test_MPESeedNoShift");
  tray.SetParameter(splineRecoName, "Parametrization", "paraname");
  tray.SetParameter(splineRecoName, "LogLikelihood", "spline-reco-test_SplineMPEllh");
  tray.SetParameter(splineRecoName, "Minimizer", "mininame");
}

/***************************
Run the Test functions
***************************/

TEST(SetEvent_noI3Geometry) {
    boost::python::import("icecube.icetray");
    boost::python::import("icecube.dataio");
    I3Tray tray;
    //create I3Geometry and Physic frames
    tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
    // Delete the I3Geometry, and see if that's handled correctly
    std::vector<std::string> deletes;
    deletes.push_back("I3Geometry");
    tray.AddModule("Delete", "deleteGeo")("Keys", deletes);
    tray.AddModule("SplineRecoTest::NoI3Geometry", "noI3Geo");
    
    tray.Execute(n_frames);
    
  }

TEST(SetEvent_wrongLlh) {
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");
  I3Tray tray;
  //create I3Geometry and Physic frames
  tray.AddModule("SplineRecoTest::RunProcess","RunProcess");

  tray.AddModule("SplineRecoTest::WrongLlh", "wrongLikelihood");
  
  tray.Execute(n_frames);
  
}

TEST(SetEvent_noPulses) {
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");
  I3Tray tray;
  tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
  // Delete the PulseSeries, and see if that's handled correctly
  std::vector<std::string> deletes;
  deletes.push_back(pulsesName);
  tray.AddModule("Delete", "deletePulses")("Keys", deletes);
  doSplineReco(tray);
  tray.AddModule("SplineRecoTest::NoPulses", "noPulses");
  
  tray.Execute(n_frames);
  
}

TEST(HasGradient) {
  I3SplineRecoLikelihood splineLlh;
  ENSURE(!splineLlh.HasGradient(), "HasGradient() should always return false.");
}

TEST(GetLogLikelihood_checkNaNs) {
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");
  I3Tray tray;
  tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
  std::vector<std::string> deletes;
  deletes.push_back(testParticleName);

  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle_x");
  // Set the vertex X to NaN, and see if that's handled correctly
  tray.AddModule("SplineRecoTest::SetValueToNaN", "xToNaN")("ValueName", "x");
  tray.AddModule("SplineRecoTest::SomethingIsNaN", "vertexXIsNaN");
  tray.AddModule("Delete", "deleteParticleY")("Keys", deletes);
  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle_y");
  // Set the vertex Y to NaN, and see if that's handled correctly
  tray.AddModule("SplineRecoTest::SetValueToNaN", "yToNaN")("ValueName", "y");
  tray.AddModule("SplineRecoTest::SomethingIsNaN", "vertexYIsNaN");
  tray.AddModule("Delete", "deleteParticleZ")("Keys", deletes);
  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle_z");
  // Set the vertex Z to NaN, and see if that's handled correctly
  tray.AddModule("SplineRecoTest::SetValueToNaN", "zToNaN")("ValueName", "z");
  tray.AddModule("SplineRecoTest::SomethingIsNaN", "vertexZIsNaN");
  tray.AddModule("Delete", "deleteParticleZen")("Keys", deletes);
  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle_zen");
  // Set the direction's zen to NaN, and see if that's handled correctly
  tray.AddModule("SplineRecoTest::SetValueToNaN", "zenToNaN")("ValueName", "zen");
  tray.AddModule("SplineRecoTest::SomethingIsNaN", "zenIsNaN");
  tray.AddModule("Delete", "deleteParticleAzi")("Keys", deletes);
  tray.AddModule("SplineRecoTest::CreateTrack", "test_particle_azi");
  // Set the direction's azi to NaN, and see if that's handled correctly
  tray.AddModule("SplineRecoTest::SetValueToNaN", "aziToNaN")("ValueName", "azi");
  tray.AddModule("SplineRecoTest::SomethingIsNaN", "aziIsNaN");

  
  tray.Execute(n_frames);
  
}

TEST(GetLogLikelihood_testWithFakePulses) {
    boost::python::import("icecube.icetray");
    boost::python::import("icecube.dataio");

    //GetIcetrayLogger()->SetLogLevel(I3LOG_TRACE);
    I3Tray tray;
    //create I3Geometry and Physic frames
    tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
    tray.AddModule("SplineRecoTest::CreateTrack", "test_particle");
    tray.AddModule("SplineRecoTest::CreatePulseSeries", "fake_PulseSeries");
    tray.AddModule("SplineRecoTest::CheckLikelihood", "llhTest");
    
    
    tray.Execute(n_frames);
    
}

TEST(GetMultiplicity) {
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");
  I3Tray tray;
  tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
  std::vector<std::string> deletes;
  deletes.push_back(pulsesName);
  tray.AddModule("Delete", "deletePulses")("Keys", deletes);
  tray.AddModule("SplineRecoTest::CreatePulseSeries", "fake_PulseSeries");
  tray.AddModule("SplineRecoTest::GetMultiplicity", "checkMult");
  

  tray.Execute(n_frames);
  
}

TEST(GetName) {
  I3SplineRecoLikelihood splineLlh;
  ENSURE_EQUAL(splineLlh.GetName(), "I3SplineRecoLikelihood", "GetName() should return 'I3SplineRecoLikelihood'.");
}

TEST(NormalOperation) {
  boost::python::import("icecube.icetray");
  boost::python::import("icecube.dataio");
  I3Tray tray;
  tray.AddModule("SplineRecoTest::RunProcess","RunProcess");
  tray.AddModule("SplineRecoTest::CreatePulseSeries", "fake_PulseSeries");
  doSplineReco(tray);
  tray.AddModule("SplineRecoTest::NormalOperation", "normalOps");
  

  tray.Execute(n_frames);
  
}
