#include <I3Test.h>
#include <icetray/I3Tray.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <simclasses/I3MCPulse.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3DOMFunctions.h>

/* This test checks that running DOMLauncher, WaveCalibrator and wavedeform in sequence
preserves timing to within a small (1/2 digitizer bin) margin, for a variety of light levels.
PMTResponseSimulator is deliberately omitted, since many of its functions vary the timing
of pulses (timing jitter, pre-, late, and afterpulses). */

TEST_GROUP(TimingRoundTrip);

namespace{
static const double slcSpacing=2.e4*I3Units::ns; //this should put each pulse in its own readout
static const double hlcSpacing=100.*I3Units::ns; //this should seperate pulses by ~30 ATWD bins
double hitSpacing(bool LC){
	if(LC)
		return hlcSpacing;
	return slcSpacing;
}
double binSize(bool LC){
	if(LC)
		return 10/3. * I3Units::ns;
	return 25. * I3Units::ns;
}
}

class HitTimingGenerator : public I3Module{
private:
	double hitCharge;
	bool lcMode;
public:
	HitTimingGenerator(const I3Context& context):
	I3Module(context),
	hitCharge(1),
	lcMode(true){
		AddOutBox("OutBox");
		AddParameter("lcMode","Whether to generate triggers with LC",lcMode);
	}
	void Configure(){
		GetParameter("lcMode",lcMode);
	}

	virtual void DAQ(boost::shared_ptr<I3Frame> frame){
		std::cout << "Generating hits with charge " << hitCharge << " which should produce " 
		          << (lcMode?"HLC":"SLC") << " readouts" << std::endl;
		const I3Geometry& geo=frame->Get<I3Geometry>();
		const I3Calibration& cal=frame->Get<I3Calibration>();
		const I3DetectorStatus& det=frame->Get<I3DetectorStatus>();
		const I3Vector<OMKey> bad=frame->Get<I3Vector<OMKey>>("BadDomsList");
		boost::shared_ptr<I3Map<OMKey,std::vector<I3MCPulse>>> hits(new I3Map<OMKey,std::vector<I3MCPulse>>);

		for(I3Map<OMKey, I3OMGeo>::const_iterator domgeo=geo.omgeo.begin(), geoend=geo.omgeo.end(); domgeo!=geoend; domgeo++){
			if(domgeo->first.GetString()<42 || domgeo->first.GetString()>45 || domgeo->first.GetOM()>60) //use the in-ice part of four strings
				continue;
			if(!lcMode && domgeo->first.GetOM()%3) //for SLC mode keep hit DOMs well separated
				continue;
			if(std::find(bad.begin(),bad.end(),domgeo->first)!=bad.end())
				continue;
			double gain=PMTGain(det.domStatus.find(domgeo->first)->second,cal.domCal.find(domgeo->first)->second);
			if(gain==0.0 || std::isnan(gain))
				continue;
			for(unsigned int i=0; i!=4; i++){ //generate a train of four hits
				I3MCPulse hit;
				hit.charge=hitCharge;
				hit.time=i*hitSpacing(lcMode);
				(*hits)[domgeo->first].push_back(hit);
			}
		}

		frame->Put("MCHitSeriesMap",hits);
		//this is useless, but keeps WaveCalibrator quiet
		frame->Put("I3EventHeader",boost::shared_ptr<I3EventHeader>(new I3EventHeader));
		PushFrame(frame);
		hitCharge++; // next time around, use more charge
	}
};
I3_MODULE(HitTimingGenerator)

namespace{
template<typename T=double>
class RunningStat{
public:
	RunningStat():n(0){}
	void clear(){ n=0; }
	void insert(T x){
		n++;
		//Knuth TAOCP vol 2, 3rd edition, page 232
		if (n == 1){
			oldM = newM = x;
			oldS = 0.0;
		}
		else{
			newM = oldM + (x - oldM)/n;
			newS = oldS + (x - oldM)*(x - newM);
			//prepare for next iteration
			oldM = newM; 
			oldS = newS;
		}
	}
	uint64_t count() const{ return n; }
	T mean() const{ return (n ? newM : T(0.)); }
	T variance() const{ return (n ? newS/(n-1) : T(0.)); }
	T stddev() const{ return sqrt(variance()); }
private:
	uint64_t n;
	T oldM, newM, oldS, newS;
};
}

class TimeFidelityChecker : public I3Module{
private:
	bool lcMode;
public:
	TimeFidelityChecker(const I3Context& context):I3Module(context),lcMode(true){
		AddParameter("lcMode","Whether to expect triggers with LC",lcMode);
	}
	void Configure(){
		GetParameter("lcMode",lcMode);
	}

	virtual void DAQ(boost::shared_ptr<I3Frame> frame){
		const double expectedSpacing=hitSpacing(lcMode);
		const I3Map<OMKey,std::vector<I3RecoPulse>>& pulses=frame->Get<I3Map<OMKey,std::vector<I3RecoPulse>>>("WavedeformPulses");
		RunningStat<> stat;
		for(I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator dom=pulses.begin(), end=pulses.end(); dom!=end; dom++){
			for(std::vector<I3RecoPulse>::const_iterator pulse=dom->second.begin(), end=dom->second.end(); pulse!=end; pulse++){
				//std::cout << ' ' << pulse->GetTime();
				double intPart;
				double fracPart=modf(pulse->GetTime()/expectedSpacing,&intPart);
				if(fracPart>0.5)
					fracPart-=1.;
				fracPart*=expectedSpacing;
				//std::cout << " (" << fracPart << ')';
				stat.insert(fracPart);
			}
			//std::cout << '\n';
		}
		std::cout << "Average timing bias: " << stat.mean()/I3Units::ns << " ns with std. dev. " << stat.stddev()/I3Units::ns << " ns" << std::endl;
		ENSURE(std::abs(stat.mean())<binSize(lcMode)/2,"Mean extracted pulse time should be within one half digitizer bin of injected time");
	}
};
I3_MODULE(TimeFidelityChecker)

namespace{
std::string testDataPath(){
 	if(std::getenv("I3_TESTDATA") == NULL)
		FAIL("I3_TESTDATA is not defined!");
	return std::string(std::getenv("I3_TESTDATA"));
}

void runTest(bool lcMode){
	I3Tray tray;
	tray.AddService("I3GSLRandomServiceFactory","rng")
		("Seed",5);
	tray.AddModule("I3InfiniteSource","FrameMaker")
		("Stream",I3Frame::DAQ)
		("Prefix", testDataPath()+"/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
	tray.AddModule("HitTimingGenerator","Hitter")
		("lcMode",lcMode);
	tray.AddModule("DOMLauncher", "Guildenstern")
		("Input","MCHitSeriesMap")
		("Output","InIceRawData");
	tray.AddModule("I3WaveCalibrator","Domcal");
	tray.AddModule("I3Wavedeform","Deform");
	tray.AddModule("TimeFidelityChecker","Checker")
		("lcMode",lcMode);
	tray.Execute(3+20);
}
}

TEST(TimingRoundTrip_HLC){
	runTest(true);
}

TEST(TimingRoundTrip_SLC){
	runTest(false);
}
