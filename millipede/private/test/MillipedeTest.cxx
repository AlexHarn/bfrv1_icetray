#include <I3Test.h>

#include <icetray/I3Tray.h>
#include <icetray/I3Module.h>
#include <icetray/I3TrayHeaders.h>
#include <icetray/Utility.h>

#include <photonics-service/I3PhotonicsService.h>
#include <photonics-service/I3DummyPhotonicsService.h>
#include <millipede/Millipede.h>

#include <stdlib.h>
#include <boost/foreach.hpp>

class MillipedeTestModule : public I3Module {
	public:
		MillipedeTestModule(const I3Context &);
		void Physics(I3FramePtr);
	private:
		I3PhotonicsServicePtr dummy_photonics;
		MillipedeDOMCacheMap datamap;
		cholmod_common c;
};

I3_MODULE(MillipedeTestModule);

TEST_GROUP(Millipede);

TEST(MatrixTest) {
	I3Tray tray;

	boost::python::import("icecube.dataio");
	tray.AddModule("I3Reader", "reader")("Filename",
	    std::string(getenv("I3_TESTDATA")) + "/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
	tray.AddModule("MillipedeTestModule", "millipede_amps");
	
	tray.Execute();
	
}

MillipedeTestModule::MillipedeTestModule(const I3Context &ctx) : I3Module(ctx)
{
	cholmod_l_start(&c);
	dummy_photonics = I3PhotonicsServicePtr(new I3DummyPhotonicsService);

	AddOutBox("OutBox");
}

void
MillipedeTestModule::Physics(I3FramePtr frame)
{
	I3RecoPulseSeriesMap pulses;

	cholmod_sparse *response_matrix;
	
	// Amplitudes only, for dummy compat
	datamap.UpdateParams(frame->Get<I3GeometryConstPtr>(),
	    frame->Get<I3CalibrationConstPtr>(),
	    frame->Get<I3DetectorStatusConstPtr>());
	ENSURE(datamap.size() > 0);
	I3TimeWindow readout_window(0, 10*I3Units::microsecond);
	I3TimeWindowSeriesMap exclusions;
	datamap.UpdateData(readout_window, pulses, exclusions, -1, NAN, 0,
	    true);
	ENSURE(datamap.size() > 0);

	I3Vector<I3Particle> sources;
	for (unsigned n_cascades = 0; n_cascades < 5; n_cascades++) {
		response_matrix = Millipede::GetResponseMatrix(datamap,
		    sources, 0.1, dummy_photonics, dummy_photonics, NULL, &c);
		ENSURE(response_matrix != NULL);

		// Check there are non-zero entries if there should be
		if (n_cascades > 0)
			ENSURE(((long *)response_matrix->p)[response_matrix->ncol] != ((long *)response_matrix->p)[0]);
		else
			ENSURE(((long *)response_matrix->p)[response_matrix->ncol] == ((long *)response_matrix->p)[0]);
		ENSURE(response_matrix->ncol == n_cascades);
		ENSURE(response_matrix->nrow == datamap.size());

		I3Particle new_cascade;
		new_cascade.SetType(I3Particle::Hadrons);
		new_cascade.SetShape(I3Particle::Cascade);
		new_cascade.SetLocationType(I3Particle::InIce);
		new_cascade.SetPos(100*n_cascades, 100*n_cascades,
		    100*n_cascades);
		new_cascade.SetDir(0.1*n_cascades, 0.1*n_cascades,
		    0.1*n_cascades);
		new_cascade.SetTime(50*n_cascades);

		sources.push_back(new_cascade);
		cholmod_l_free_sparse(&response_matrix, &c);
	}
	
}

static I3FramePtr
GetPotemkinGCD(std::vector<OMKey> &oms)
{
	I3FramePtr frame(new I3Frame);
	
	I3CalibrationPtr calibration(new I3Calibration);
	I3DetectorStatusPtr status(new I3DetectorStatus);
	I3GeometryPtr geometry(new I3Geometry);
	
	BOOST_FOREACH(const OMKey &key, oms) {
		I3DOMCalibration &calib = calibration->domCal[key];
		I3DOMStatus &stat = status->domStatus[key];
		I3OMGeo &geo = geometry->omgeo[key];
		
		geo.omtype = I3OMGeo::IceCube;
		calib.SetRelativeDomEff(1.0);
		calib.SetDomNoiseRate(850*I3Units::hertz);
		stat.pmtHV = 1e3*I3Units::V;
	}
	
	frame->Put(calibration);
	frame->Put(status);
	frame->Put(geometry);
	
	return frame;
}

static MillipedeDOMCacheMap
GetPotemkinDOMCacheMap(std::vector<OMKey> &oms)
{
	MillipedeDOMCacheMap cachemap;
	I3FramePtr frame = GetPotemkinGCD(oms);
	
	I3GeometryConstPtr geometry = frame->Get<I3GeometryConstPtr>();
	I3CalibrationConstPtr calibration = frame->Get<I3CalibrationConstPtr>();
	I3DetectorStatusConstPtr status = frame->Get<I3DetectorStatusConstPtr>();
	cachemap.UpdateParams(geometry, calibration, status);
	
	return cachemap;
}

TEST(PulseBinning) {
	OMKey key(1,1);
	std::vector<OMKey> oms;
	oms.push_back(key);
	
	MillipedeDOMCacheMap cachemap = GetPotemkinDOMCacheMap(oms);
	
	I3RecoPulseSeriesMap pulsemap;
	I3RecoPulseSeries &pulses = pulsemap[key];
	I3RecoPulse pulse;
	pulse.SetFlags(I3RecoPulse::LC & I3RecoPulse::ATWD);
	pulse.SetWidth(25.);
	
	// Three pulses: the first should coalesce into one bin,
	// the third will be split out.
	double t1(1e4), t2(1e4 + 2.*25.), t3(1e4 + 15.*25.);
	
	pulse.SetTime(t1);
	pulse.SetCharge(8.);
	pulses.push_back(pulse);
	
	pulse.SetTime(t2);
	pulses.push_back(pulse);
	
	pulse.SetTime(t3);
	pulses.push_back(pulse);
	
	I3TimeWindow readout_window(0, 2e4);
	I3TimeWindowSeriesMap exclusions;
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	
	MillipedeDOMCache &cache = cachemap[key];
	
	ENSURE_EQUAL(cache.nbins, 5);
	
	ENSURE_EQUAL(cache.time_bin_edges[0], 0.);
	ENSURE_EQUAL(cache.charges[0], 0.);
	
	ENSURE_EQUAL(cache.time_bin_edges[1], t1);
	ENSURE_EQUAL(cache.charges[1], 16.);
	
	ENSURE_EQUAL(cache.time_bin_edges[2], t2 + pulse.GetWidth());
	ENSURE_EQUAL(cache.charges[2], 0.);
	
	ENSURE_EQUAL(cache.time_bin_edges[3], t3);
	ENSURE_EQUAL(cache.charges[3], 8.);
	
	ENSURE_EQUAL(cache.time_bin_edges[4], t3 + pulse.GetWidth());
	ENSURE_EQUAL(cache.charges[4], 0.);
	
	ENSURE_EQUAL(cache.time_bin_edges[5], 2e4);
	
	// Add an exclusion that eliminates the last pulse and covers the rest of the readout window
	exclusions[key].push_back(I3TimeWindow(pulses.back().GetTime(), readout_window.GetStop()));
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	ENSURE_EQUAL(cache.nbins, 3);
	ENSURE_EQUAL(cache.time_bin_edges[3], pulses.back().GetTime(), "Last bin stops at the beginning of the exclusion window");
	for (int i=0; i<cache.nbins; i++)
		ENSURE(cache.valid[i]);
	
	// Add an exclusion window that doesn't cover any pulses. This should simply
	// shorten the readout window.
	exclusions.clear();
	exclusions[key].push_back(I3TimeWindow(0, 150.));
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	ENSURE_EQUAL(cache.nbins, 5);
	ENSURE_EQUAL(cache.time_bin_edges[0], 150.);
	
	// and again, but in between pulses
	exclusions.clear();
	exclusions[key].push_back(I3TimeWindow(((t2+t3)/2.), ((t2+t3)/2.)+1));
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	ENSURE_EQUAL(cache.nbins, 7);
	ENSURE_EQUAL(cache.valid[2], true);
	ENSURE_EQUAL(cache.valid[3], false);
	ENSURE_EQUAL(cache.time_bin_edges[3], exclusions[key].front().GetStart());
	ENSURE_EQUAL(cache.time_bin_edges[4], exclusions[key].front().GetStop());
	ENSURE_EQUAL(cache.valid[4], true);
	
	// Add an exclusion that just barely eliminates the last pulse
	exclusions.clear();
	exclusions[key].push_back(I3TimeWindow(pulses.back().GetTime(), pulses.back().GetTime()+pulses.back().GetWidth()));
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	ENSURE_EQUAL(cache.nbins, 5);
	ENSURE_EQUAL(cache.time_bin_edges[4], exclusions[key].front().GetStop(), "Excluded bin ends at the proper time");
	ENSURE_EQUAL(cache.valid[3], false);
	
	// Add an exclusion that only eliminates the middle pulse
	exclusions.clear();
	{
		const I3RecoPulse pulse = *(pulses.end()-2);
		// Choose an end time that *doesn't* correspond to a bin edge
		exclusions[key].push_back(I3TimeWindow(pulse.GetTime(), pulse.GetTime()+pulse.GetWidth()+1));
	}
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., NAN, 0, true);
	ENSURE_EQUAL(cache.nbins, 6);
	ENSURE_EQUAL(cache.time_bin_edges[3], exclusions[key].front().GetStop(), "Excluded bin ends at the proper time");
	ENSURE_EQUAL(cache.valid[2], false);
	
	// BinSigma=0 will never combine bins
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., 0, 0, true);
	ENSURE_EQUAL(cache.nbins, 7);
	ENSURE_EQUAL(cache.valid[3], false);
	
	// BinSigma=10 should combine all bins
	cachemap.UpdateData(readout_window, pulsemap, exclusions, 10., 10, 0, true);
	ENSURE_EQUAL(cache.nbins, 3);
	ENSURE_EQUAL(cache.valid[1], false);
}
