/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#include "utility.h"
#include "tpx/I3IceTopBaseline.h"

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Units.h>
#include <dataclasses/physics/I3Waveform.h>

#include <boost/foreach.hpp>
#include <cassert>
#include <cmath>
#include <string>
#ifndef NDEBUG
#include <sstream>
#endif
#include <vector>


class I3IceTopBaselineModule : public I3ConditionalModule {
public:
  I3IceTopBaselineModule(const I3Context &ctx)
    : I3ConditionalModule(ctx)
  {
    waveformsName_ = "";
    AddParameter("Waveforms", "Waveforms to use for baseline calculations", waveformsName_);
    baselineRange_.push_back(0);
    baselineRange_.push_back(0);
    AddParameter("BaselineRange",
		 "Range of bin numbers, which are considered the baseline. Numbers are inclusive."
		 "They can be negative. In that case bins are counted from the last bin. "
		 "For instance, -1 is the last bin, while 0 is the first bin. "
		 "There is no difference between 0 and -0.",
		 baselineRange_);
    source_ = I3Waveform::ATWD;
    AddParameter("Source", "Should we look at ATWD or FADC waveforms?", source_);
    outputName_ = "";
    AddParameter("Output", "Name of the output I3IceTopBaselineSeriesMap", outputName_);
  }

  void Configure();

  void DAQ(I3FramePtr frame);

private:
  std::string waveformsName_;
  std::vector<int> baselineRange_;
  I3Waveform::Source source_;
  std::string outputName_;

  I3IceTopBaseline calculateBaseline(const I3Waveform &waveform);

  SET_LOGGER("I3IceTopBaselineModule");
};


I3_MODULE(I3IceTopBaselineModule);


void I3IceTopBaselineModule::Configure()
{
  GetParameter("Waveforms", waveformsName_);
  if (waveformsName_.length() == 0)
    log_fatal("You have to specify a name of the input waveforms.");

  GetParameter("BaselineRange", baselineRange_);
  if (baselineRange_.size() != 2)
    log_fatal("BaselineRange has to be a list of exactly two bin numbers.");

  GetParameter("Source", source_);

  GetParameter("Output", outputName_);
  if (outputName_.length() == 0)
    log_fatal("A name for the Output has to be configured.");
}


void I3IceTopBaselineModule::DAQ(I3FramePtr frame)
{
  I3WaveformSeriesMapConstPtr waveforms =
    frame->Get<I3WaveformSeriesMapConstPtr>(waveformsName_);
  if (!waveforms) {
    log_info("Waveforms %s not in frame. Skipping.", waveformsName_.c_str());
    PushFrame(frame);
    return;
  }

  log_debug("Found waveforms ... proceeding.");

  I3IceTopBaselineSeriesMapPtr baselines(new I3IceTopBaselineSeriesMap);

  typedef std::pair<OMKey, I3WaveformSeries> I3WaveformSeriesPair;
  BOOST_FOREACH(const I3WaveformSeriesPair &wfp, *waveforms) {
    I3IceTopBaselineSeries baselineSeries;
    BOOST_FOREACH(const I3Waveform &waveform, wfp.second) {
      if (waveform.GetDigitizer() == source_ && waveform.IsHLC()) {    // only makes sense for full waveforms
	I3IceTopBaseline baseline(calculateBaseline(waveform));
#ifndef NDEBUG
	std::string chip = "";
	std::stringstream channel;
	if (source_ == I3Waveform::ATWD) {
	  if (baseline.sourceID == 0) chip = "a";
	  else if (baseline.sourceID == 1) chip = "b";
	  channel << " channel " << (int)baseline.channel;
	}
	log_trace("DOM %d-%d, %s%s%s:", wfp.first.GetString(), wfp.first.GetOM(),
		  source_ == I3Waveform::ATWD ? "ATWD" : "FADC",
		  chip.c_str(), channel.str().c_str());
	log_trace("Baseline: %g mV", (double)baseline.baseline/I3Units::mV);
	log_trace("Slope:    %g mV/ns", (double)baseline.slope*I3Units::ns/I3Units::mV);
	log_trace("RMS:      %g mV", (double)baseline.rms/I3Units::mV);
#endif
	baselineSeries.push_back(baseline);
      }
    }
    if (baselineSeries.size() > 0)
      (*baselines)[wfp.first] = baselineSeries;
  }

  if (baselines->size() > 0)
    frame->Put(outputName_, baselines);

  PushFrame(frame);
}


I3IceTopBaseline I3IceTopBaselineModule::calculateBaseline(const I3Waveform &waveform)
{
  assert(baselineRange_.size() == 2);

  const std::vector<double> &wf = waveform.GetWaveform();
  int waveformLength = wf.size();
  I3Waveform::Source source = waveform.GetSource();
  int8_t channel = tpx::GetChannel(waveform);
  int sourceIndex = source == I3Waveform::ATWD ? waveform.GetSourceIndex() : 0;
  double deltaT = waveform.GetBinWidth();

  // define range such that the second entry points one bin past the last one for convenience
  const int range[2] =
    { baselineRange_[0] < 0 ? waveformLength + baselineRange_[0] : baselineRange_[0],
      1 + baselineRange_[1] < 0 ? waveformLength + baselineRange_[1] : baselineRange_[1] };
  int count = range[1]-range[0];   // number of bins in baseline

  // sanity check: range must be positive and must not exceed the range of the waveform
  for (int i = 0; i < 2; ++i) {
    if (range[i] < 0 || range[i] > waveformLength) {
      log_error("Baseline range (%d, %d) too large for waveform. "
		"Baseline information will be invalid.",
		range[0], range[1]);
      return I3IceTopBaseline(source, channel, sourceIndex);
    }
  }
  // another sanity check: the range[0] <= range[1]
  if (range[0] >= range[1]) {
    // keep the error message consistent with the parameter definition,
    // which means ranges are inclusive
    log_error("Baseline first bin must not be larger than second bin, but I found (%d, %d)",
	      range[0], range[1]-1);
    return I3IceTopBaseline(source, channel, sourceIndex);
  }

  double sum = 0;    // sum of bin contents
  double sum2 = 0;   // sum of squares for rms calculation
  double sxy = 0;    // for slope fit: S_xy = sum(x_i * y_i / sigma_i) = deltaT*sum(i * y_i)
                     // assuming x_i = i*deltaT, sigma_i = 1

  for (int i = range[0]; i < range[1]; ++i) {
    sum += wf[i];
    sum2 += wf[i]*wf[i];
    sxy += (i-range[0])*wf[i];
  }

  double baseline = sum/count;                        // average
  double rms = sqrt(sum2/count - baseline*baseline);  // spread
  sxy *= deltaT;                                      // see definition of S_xy above

  // Slope calculation
  // NB: All sums from i=0 to i=N-1
  //     x_i = i*deltaT
  //     sigma_i = 1
  // slope = 1/D * (S1*S_xy - S_x*S_y)
  // with:
  // D = S1*S_xx - S_x^2
  // S1 = sum(1/sigma_i^2) = 1
  // S_x = sum(x_i/sigma_i^2) = deltaT * sum(i) = deltaT * N * (N-1)/2
  // S_y = sum(y_i/sigma_i^2) = sum(y_i) = sum
  // S_xx = sum(x_i^2/sigma_i^2) = sum((deltaT*i)^2) = deltaT^2 * (N-1)/6 * N * (2N-1)
  // S_xy = sum(x_iy_i/sigma_i^2) = sum(deltaT*i*y_i), see above
  // N = count
  double sxx = deltaT*deltaT * (count-1)/6 * count * (2*count - 1);
  double sx = deltaT * count * (count-1)/2;
  double D = sxx - sx*sx;
  double slope = (sxy - sx*sum)/D;

  return I3IceTopBaseline(source, channel, sourceIndex, baseline, slope, rms);
}
