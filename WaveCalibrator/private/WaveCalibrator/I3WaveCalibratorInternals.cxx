/**
 *  $Id$
 *  
 *  Copyright (C) 2011
 *  Jakob van Santen <vansanten@wisc.edu>
 *  and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  
 */

#include "WaveCalibrator/I3WaveCalibrator.h"
#include "dataclasses/I3DOMFunctions.h"
#include "icetray/I3Units.h"

#include <boost/foreach.hpp>

#define ATWD_NBINS 128u
#define ATWD_NCHANNELS 3u
#define ADC_MAX 1023

void
I3WaveCalibrator::CalibrateLaunches(const OMKey &om, const I3DOMLaunchSeries &launches,
    const I3OMGeo &geo, const I3DOMCalibration &calib, const I3DOMStatus &status,
    I3WaveformSeriesMap &waveform_map, I3TimeWindowSeriesMap &errata)
{
	std::vector<I3WaveformSeries> atwd_bundles(launches.size());
	I3WaveformSeries fadcs(launches.size());
	
	I3DOMLaunchSeries::const_iterator launch_it = launches.begin();
	std::vector<I3WaveformSeries>::iterator atwd_bundle_it = atwd_bundles.begin();
	I3WaveformSeries::iterator fadc_it = fadcs.begin();

	for ( ; launch_it != launches.end(); launch_it++, atwd_bundle_it++, fadc_it++) {
		if (launch_it->GetLCBit()) {
			/* Calibrate ATWD channel(s) */
			std::vector<I3Waveform> &atwds = *atwd_bundle_it;
			for (unsigned i = 0; i < ATWD_NCHANNELS; i++) {
				atwds.push_back(I3Waveform());
				if (!CalibrateATWD(om, *launch_it, calib, status,
				    atwds.back(), i))
					atwds.pop_back();
			}

			/* Calibrate FADC */
			CalibrateFADC(om, *launch_it, calib, status, *fadc_it);

		} else if (geo.omtype == I3OMGeo::IceCube) {
			/* InIce-style SLC readout (3 bins from the FADC) */
			CalibrateFADC_SLC(om, *launch_it, calib, status, *fadc_it);
		} else {
			/* IceTop-style SLC readout (sum over the ATWD) */
			CalibrateATWD_SLC(om, *launch_it, calib, status, *fadc_it);
		}
	}

	/* Apply droop correction to all of the waveforms in one go. */
	I3TimeWindowSeries erratum;
	if (correct_droop_)
		CorrectPedestalDroop(atwd_bundles, fadcs, erratum, calib);
	
	/* Pack waveforms into output */
	std::vector<I3Waveform> waveforms_out;
	atwd_bundle_it = atwd_bundles.begin();
	BOOST_FOREACH(I3Waveform &fadc, fadcs) {
		BOOST_FOREACH(I3Waveform &atwd, *atwd_bundle_it) {
			CompactifyWaveform(atwd);
			waveforms_out.push_back(atwd);
			assert(waveforms_out.back().GetWaveform().size() > 0);
		}
		atwd_bundle_it++;
		
		if (fadc.GetWaveform().size() > 0) {
			CompactifyWaveform(fadc);
			waveforms_out.push_back(fadc);
		}
	}

	/* Errata that extend to +inf really exire at the last bin present. */
	BOOST_FOREACH(I3TimeWindow &w, erratum) {
		if (waveforms_out.size() > 0 &&
		    w.GetStop() == std::numeric_limits<double>::infinity()) {
			const I3Waveform &last = waveforms_out.back();
			w = I3TimeWindow(w.GetStart(), last.GetStartTime() +
			    last.GetBinWidth()*last.GetWaveform().size());
		    }
	}
	
	waveform_map[om].swap(waveforms_out);
	if (erratum.size() > 0)
		errata[om].swap(erratum);
}

bool
I3WaveCalibrator::CalibrateATWD(const OMKey &om, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status,
    I3Waveform &atwd, int channel)
{
	const size_t nbins = GetATWDSize(launch, channel);
	if (nbins == 0)
		return false;
	
	/* Read in the digitizer counts. */
	std::vector<WaveformBin> bins(nbins);
	ReadATWD(bins, launch, calib, status, channel);
	
	/* Set up a waveform. */
	std::vector<double> &waveform = atwd.GetWaveform();
	waveform.resize(nbins);
	
	int8_t last_channel = -1;
	double baseline(0.), gain(0.);
	const unsigned atwd_id = (launch.GetWhichATWD() == I3DOMLaunch::ATWDa) ? 0 : 1;
	unsigned i = 0;

	BOOST_FOREACH(WaveformBin &bin, bins) {
		if (bin.channel != last_channel) {
			last_channel = bin.channel; 
			baseline = calib.GetATWDBeaconBaseline(atwd_id, last_channel);
			gain = calib.GetATWDGain(last_channel);
		}
		
		/* Apply the voltage calibration. */
		double slope = calib.GetATWDBinCalibSlope(atwd_id, bin.channel, i);
		waveform[i++] = slope*(bin.counts - baseline)/gain;
	}
	
	/* Extract bin struct into StatusCompound. */
	ExtractWaveformStatus(bins, atwd.GetWaveformInformation());
	
	/* Set sampling rate. */
	atwd.SetBinWidth(1./ATWDSamplingRate(atwd_id, status, calib));
	
	/* Correct for PMT transit time. */
	atwd.SetStartTime(GetStartTime(launch, calib, status, false /* fadc */));
	
	/* Set source info. */
	atwd.SetDigitizer(I3Waveform::ATWD);
	atwd.SetSourceIndex(atwd_id);
	atwd.SetHLC(true);
	
	return true;
}

bool
I3WaveCalibrator::CalibrateATWD_SLC(const OMKey &om, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status, I3Waveform &atwd)
{
	const unsigned channel = launch.GetWhichATWDChargeStamp();
	const unsigned atwd_id = (launch.GetWhichATWD() == I3DOMLaunch::ATWDa) ? 0 : 1;
	
	/*
	 * XXX HACK: since each bin of the ATWD is an independent digitizer, we
	 * can't apply our voltage calibration to the sum of ATWD bins directly.
	 * Instead, we would have to form an average of all the bin gains weighted
	 * by the pedestal-subtracted counts in each bin. Since we don't have the
	 * individual counts, however, we have to make something up.
	 * 
	 * Following DOMcalibrator, take the average gain of bins 6--50.
	 */
	double mean_slope = 0;
	for (unsigned bin = 6; bin < 51; bin++)
		mean_slope += calib.GetATWDBinCalibSlope(atwd_id, channel, bin);
	mean_slope /= 45.0;
	
	/*
	 * Emit a single waveform bin.
	 *
	 * NB: RawATWDChargeStamp is the sum over ATWD_NBINS samples in units of
	 * ADC counts. To preserve the meaning of an I3Waveform for this style of
	 * readout, we output the _average_ voltage, averaged over the ATWD_NBINS
	 * samples; the corresponding width of the averaged waveform is
	 * ATWD_NBINS * binsSize.
	 */
	const double total_counts = launch.GetRawATWDChargeStamp();
	const double total_baseline = ATWD_NBINS*calib.GetATWDBeaconBaseline(atwd_id, channel);
	atwd.GetWaveform().push_back(mean_slope*(total_counts - total_baseline)/calib.GetATWDGain(channel) / ATWD_NBINS); // 
	
	/* Mark channels as appropriate. */
	I3Waveform::StatusCompound stat;
	stat.GetInterval().first = 0;
	stat.GetInterval().second = 1;
	stat.SetChannel(channel);
	stat.SetStatus(I3Waveform::VIRGINAL); /* We have no information to the contrary. */
	atwd.GetWaveformInformation().push_back(stat);
	
	/* 
	 * Set the bin width to the width of the ATWD readout window; this is
	 * also the time over which the average voltage was calculated.
	 */
	atwd.SetBinWidth(ATWD_NBINS/ATWDSamplingRate(atwd_id, status, calib));
	
	/* Correct for PMT transit time. */
	atwd.SetStartTime(GetStartTime(launch, calib, status, false /* fadc */));
	
	atwd.SetDigitizer(I3Waveform::ATWD);
	atwd.SetSourceIndex(atwd_id);
	atwd.SetHLC(false);
	
	return true;
}

void
I3WaveCalibrator::ReadATWD(std::vector<WaveformBin> &bins, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status,
    int channel)
{
	const std::vector<int> &trace = launch.GetRawATWD(channel);
	
	for (unsigned i = 0; i < trace.size(); i++) {
		const int count = trace[i];
		bins[i].counts = count;
		bins[i].channel = channel;
		
		if (count + atwd_saturation_margin_ >= ADC_MAX)
			bins[i].status = I3Waveform::SATURATED;
		else if (count <= 0)
			bins[i].status = I3Waveform::UNDERSHOT;
		else
			bins[i].status = I3Waveform::VIRGINAL;
	}
}

size_t
I3WaveCalibrator::GetATWDSize(const I3DOMLaunch& dl, int channel)
{
	if (channel >= 0)
		return dl.GetRawATWD(channel).size();
	
	size_t nbins = 0;
	for (unsigned i = 0; i < 3; i++)
		if (dl.GetRawATWD(i).size() > nbins)
			nbins = dl.GetRawATWD(i).size();
	
	return nbins;
}

bool
I3WaveCalibrator::CalibrateFADC(const OMKey &om, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status,
    I3Waveform &fadc)
{
	const size_t nbins = launch.GetRawFADC().size();
	if (nbins == 0)
		return false;
		
	/* Get digitizer counts. */
	const std::vector<int> &trace = launch.GetRawFADC();
	
	/* Set up a waveform and channel info */
	std::vector<WaveformBin> bins(nbins);
	std::vector<double> &waveform = fadc.GetWaveform();
	waveform.resize(nbins);
	
	/* Get the voltage calibration. */
	const double baseline = calib.GetFADCBeaconBaseline();
	const double gain = calib.GetFADCGain();
	
	/* Apply the calibration */
	for (unsigned i = 0; i < nbins; i++) {
		waveform[i] = gain*(double(trace[i]) - baseline);
		
		bins[i].channel = 0;
		if (trace[i] + fadc_saturation_margin_ >= ADC_MAX)
			bins[i].status = I3Waveform::SATURATED;
		else if (trace[i] - fadc_undershoot_margin_ <= 0)
			bins[i].status = I3Waveform::UNDERSHOT;
		else
			bins[i].status = I3Waveform::VIRGINAL;
	}
	
	/* Extract bin struct into StatusCompound. */
	ExtractWaveformStatus(bins, fadc.GetWaveformInformation());
	
	/* Set sampling rate. */
	fadc.SetBinWidth(25*I3Units::ns);
	
	/* Correct for PMT transit time. */
	fadc.SetStartTime(GetStartTime(launch, calib, status, true /* fadc */));
	
	/* Set source info. */
	fadc.SetDigitizer(I3Waveform::FADC);
	fadc.SetHLC(true);
	
	return true;
}

void
I3WaveCalibrator::ReadFADC(std::vector<WaveformBin> &bins, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status)
{
	const std::vector<int> &trace = launch.GetRawFADC();
	for (unsigned i = 0; i < trace.size(); i++) {
		bins[i].counts = trace[i];
		bins[i].channel = 0;
		if (trace[i] + fadc_saturation_margin_ >= ADC_MAX)
			bins[i].status = I3Waveform::SATURATED;
		else if (trace[i] - fadc_undershoot_margin_ <= 0)
			bins[i].status = I3Waveform::UNDERSHOT;
		else
			bins[i].status = I3Waveform::VIRGINAL;
	}
}

bool
I3WaveCalibrator::CalibrateFADC_SLC(const OMKey &om, const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status,
    I3Waveform &fadc)
{
	const size_t nbins = launch.GetRawChargeStamp().size();
	if (nbins == 0)
		return false;
	
	/* Apply the voltage calibration. */
	const double baseline = calib.GetFADCBeaconBaseline();
	const double gain = calib.GetFADCGain();
	
	/* 
	 * Clipped SLC launches are rare, but they can happen e.g. in DOMs with
	 * broken LC lines, or in events where muons actually hit the DOM. While we
	 * can't do much for droop correction, it's important to mark the clipped
	 * sections for pulse extraction downstream.
	 * 
	 * NB: SLC charge stamps are packed into 9 bits, dropping the LSB if the
	 * peak is greater than 512. This makes the charge stamps for
	 * high-amplitude SLC hits even by construction, which means that they clip
	 * at 1022 instead of 1023.
	 */
	const int adc_max = ADC_MAX-1;
	fadc.GetWaveform().resize(nbins, 0.0);
	fadc.GetWaveformInformation().push_back(I3Waveform::StatusCompound());
	fadc.GetWaveformInformation().back().GetInterval().first = 0;
	fadc.GetWaveformInformation().back().SetStatus(I3Waveform::VIRGINAL);
	fadc.GetWaveformInformation().back().SetChannel(0);
	for (unsigned i = 0; i < nbins; i++) {
		const int counts = launch.GetRawChargeStamp()[i];
		fadc.GetWaveform()[i] = gain*(counts - baseline);
		
		I3Waveform::Status status;
		if (counts + fadc_saturation_margin_ >= adc_max)
			status = I3Waveform::SATURATED;
		else if (counts - fadc_undershoot_margin_ <= 0)
			status = I3Waveform::UNDERSHOT;
		else
			status = I3Waveform::VIRGINAL;
		if (status != fadc.GetWaveformInformation().back().GetStatus()) {
			/* Close previous interval */
			fadc.GetWaveformInformation().back().GetInterval().second = i;
			/* Open a new one */
			fadc.GetWaveformInformation().push_back(I3Waveform::StatusCompound());
			fadc.GetWaveformInformation().back().GetInterval().first = i;
			fadc.GetWaveformInformation().back().SetStatus(status);
			fadc.GetWaveformInformation().back().SetChannel(0);
		}
	}
	/* Close last interval */
	fadc.GetWaveformInformation().back().GetInterval().second = nbins;
	
	/* Set sampling rate. */
	fadc.SetBinWidth(25*I3Units::ns);
	
	/*
	 * Correct for PMT transit time, setting the waveform start to
	 * the leading edge of the first bin in the charge stamp.
	 */
	fadc.SetStartTime(GetStartTime(launch, calib, status, true /* fadc */) +
	    (std::max(launch.GetChargeStampHighestSample(),1u)-1)*25*I3Units::ns);
	
	/* Set source info. */
	fadc.SetDigitizer(I3Waveform::FADC);
	fadc.SetHLC(false);

	return true;
}

void
I3WaveCalibrator::CompactifyWaveform(I3Waveform &waveform)
{
	std::vector<I3Waveform::StatusCompound> tmp_status;
	std::vector<I3Waveform::StatusCompound> &status = waveform.GetWaveformInformation();
	
	std::vector<I3Waveform::StatusCompound>::const_iterator status_it = status.begin();
	for ( ; status_it != status.end(); status_it++)
		if (status_it->GetChannel() != 0 || status_it->GetStatus() != I3Waveform::VIRGINAL)
			tmp_status.push_back(*status_it);
			
	status.swap(tmp_status);
}

void
I3WaveCalibrator::ExtractWaveformStatus(const std::vector<WaveformBin> &bins,
    std::vector<I3Waveform::StatusCompound> &stati)
{
	const size_t nbins = bins.size();

	/* 
	 * Don't throw out VIRGINAL/channel 0 markers quite yet;
	 * they will be useful for droop correction later.
	 */
	I3Waveform::Status status = bins[0].status;
	int8_t channel = bins[0].channel;
	
	stati.push_back(I3Waveform::StatusCompound());
	stati.back().SetStatus(status);
	stati.back().SetChannel(channel);
	stati.back().GetInterval().first = 0;
	
	unsigned i = 0;
	BOOST_FOREACH(const WaveformBin &bin, bins) {
		if (bin.status != status || bin.channel != channel) {
			status = bin.status;
			channel = bin.channel;
			stati.back().GetInterval().second = i;
			stati.push_back(I3Waveform::StatusCompound());
			stati.back().SetStatus(status);
			stati.back().SetChannel(channel);
			stati.back().GetInterval().first = i;
		}
		i++;
	}
	
	stati.back().GetInterval().second = nbins;
}

double
I3WaveCalibrator::GetStartTime(const I3DOMLaunch &launch,
    const I3DOMCalibration &calib, const I3DOMStatus &status, bool fadc)
{
	const LinearFit &fit = calib.GetTransitTime();
	const double pmtHV = status.pmtHV/I3Units::V;
	const double offset = calib.GetATWDDeltaT(
	    (launch.GetWhichATWD() == I3DOMLaunch::ATWDa) ? 0 : 1);
	const double transit_time = fit.slope/sqrt(pmtHV) + fit.intercept + offset;
	
	if (fadc)
		return (launch.GetStartTime() - transit_time + calib.GetFADCDeltaT());
	else
		return (launch.GetStartTime() - transit_time);
}
