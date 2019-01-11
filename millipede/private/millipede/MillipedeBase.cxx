#include <icetray/I3Frame.h>
#include <icetray/I3Units.h>
#include <photonics-service/I3PhotonicsService.h>
#include <millipede/Millipede.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

template<class Base>
I3MillipedeBase<Base>::I3MillipedeBase(const I3Context &context)
    : Base(context)
{
	cholmod_l_start(&c);

	Base::AddParameter("MuonPhotonicsService",
	    "Photonics service for Cerenkov emissions (muons)", muon_p);
	Base::AddParameter("CascadePhotonicsService",
	    "Photonics service for shower emissions", cascade_p);
	Base::AddParameter("Pulses", "Name of pulse series to use",
	    "RecoPulseSeries");
	Base::AddParameter("ReadoutWindow", "Name of the I3TimeWindow in the "
	    "frame giving the first and last possible time of a pulse. If "
	    "blank (the default), will use the name of the pulse series with "
	    "\"TimeRange\" appended to the end.", "");
	std::vector<std::string> bad_doms_defaults;
	bad_doms_defaults.push_back("BadDomsList");
	bad_doms_defaults.push_back("CalibrationErrata");
	bad_doms_defaults.push_back("SaturationWindows");
	Base::AddParameter("ExcludedDOMs", "Set of keys containing lists "
	    "either of OMKeys or I3TimeWindowSeriesMaps describing potentially "
	    "unreliable data that should be excluded from the fit",
	    bad_doms_defaults);
	Base::AddParameter("PartialExclusion", "If true, exclude only marked "
	    "time ranges from excluded DOMs. Otherwise, ignore entire readout "
	    "from any DOM in one of the exclusion lists", true);
	Base::AddParameter("MuonRegularization",
	    "Coefficient for regularization term for ionization component of "
	    "muon energy losses (0 to disable)", 0);
	Base::AddParameter("ShowerRegularization",
	    "Coefficient for regularization term for stochastic energy losses "
	    "(0 to disable)", 0);
	Base::AddParameter("PhotonsPerBin", "Number of photons to use per time "
	    "bin. Smaller values use more timing information, but can bias the "
	    "solution when solving for multiple sources. Setting PhotonsPerBin "
	    "to -1 disables the use of timing.", 5);
	Base::AddParameter("BinSigma", "Signficance threshold for time bins. "
	    "If set to a finite number, PhotonsPerBin will be ignored, and "
	    "time bins will be combined using the Bayesian Blocks algorithm "
	    "until the mean PE rate in each bin differs from its neighbors at "
	    "this significance level.", NAN);
	Base::AddParameter("MinTimeWidth", "Minimum width of time bins. "
	    "Implemented as a hard cutoff when using fixed binning and a prior "
	    "for Bayesian Blocks. Timing structure less than this width should "
	    "be ignored in the fit.", 8*I3Units::ns);
	Base::AddParameter("DOMEfficiency", "Fraction of DOM surface "
	    "unshadowed by cable and active", 0.99);
	Base::AddParameter("UseUnhitDOMs", "Take all DOMs into account in the "
	    "matrix unfolding, which is much slower but potentially more "
	    "accurate.", true);
}

template<class Base>
I3MillipedeBase<Base>::~I3MillipedeBase()
{
	cholmod_l_finish(&c);
}

template<class Base>
void
I3MillipedeBase<Base>::Configure()
{
	Base::GetParameter("MuonPhotonicsService", muon_p);
	Base::GetParameter("CascadePhotonicsService", cascade_p);
	Base::GetParameter("Pulses", pulses_name_);
	Base::GetParameter("ReadoutWindow", readout_window_name_);
	if (readout_window_name_ == "")
		readout_window_name_ = pulses_name_ + "TimeRange";
	Base::GetParameter("ExcludedDOMs", exclusions_name_);
	Base::GetParameter("PartialExclusion", partial_exclusion_);
	Base::GetParameter("MuonRegularization", regularizeMuons_);
	Base::GetParameter("ShowerRegularization", regularizeStochastics_);
	Base::GetParameter("PhotonsPerBin", timeBinPhotons_);
	Base::GetParameter("BinSigma", timeBinSigma_);
	Base::GetParameter("MinTimeWidth", minWidth_);
	// Can't exclude only part of waveforms in amplitude-only mode
	if (timeBinPhotons_ < 0)
		partial_exclusion_ = false;
	Base::GetParameter("DOMEfficiency", domEfficiency_);
	Base::GetParameter("UseUnhitDOMs", useUnhit_);
}

template<class Base>
void
I3MillipedeBase<Base>::DatamapFromFrame(const I3Frame &frame)
{
	domCache_.UpdateParams(frame.Get<I3GeometryConstPtr>(),
	    frame.Get<I3CalibrationConstPtr>(),
	    frame.Get<I3DetectorStatusConstPtr>());

	if (domCache_.size() == 0)
		log_fatal("No active DOMs in data map!");

	const I3RecoPulseSeriesMap& pulses =
	    frame.Get<I3RecoPulseSeriesMap>(pulses_name_);

	I3TimeWindowConstPtr readout_window =
	    frame.Get<I3TimeWindowConstPtr>(readout_window_name_);
	if (!readout_window)
		log_fatal("No readout window named \"%s\" present in frame",
		    readout_window_name_.c_str());
	if (!std::isfinite(readout_window->GetLength()))
		log_fatal("Readout window \"%s\" not of finite length",
		    readout_window_name_.c_str());
	
	I3TimeWindowSeriesMap exclusions;
	BOOST_FOREACH(const std::string &mapname, exclusions_name_) {
		I3TimeWindowSeriesMapConstPtr exclusions_segment =
		    frame.Get<I3TimeWindowSeriesMapConstPtr>(mapname);
		I3VectorOMKeyConstPtr excludedoms =
		    frame.Get<I3VectorOMKeyConstPtr>(mapname);
		if (exclusions_segment && partial_exclusion_) {
			for (I3TimeWindowSeriesMap::const_iterator i =
			    exclusions_segment->begin(); i !=
			    exclusions_segment->end(); i++)
				exclusions[i->first] = exclusions[i->first] |
				    i->second;
		} else if (exclusions_segment && !partial_exclusion_) {
			for (I3TimeWindowSeriesMap::const_iterator i =
			    exclusions_segment->begin(); i !=
			    exclusions_segment->end(); i++)
				exclusions[i->first].push_back(I3TimeWindow());
		} else if (excludedoms) {
			BOOST_FOREACH(const OMKey &key, *excludedoms)
				exclusions[key].push_back(I3TimeWindow());
		}
	}
	if ((std::isfinite(timeBinSigma_)) && (timeBinPhotons_ < 0))
		log_fatal("Cannot do amplitude fit (PhotonsPerBin=\"%f\") and \
			fit using timing (BinSigma=\"%f\" at the same time. Either \
			remove BinSigma or set PhotonsPerBin to positive values.",
			timeBinPhotons_, timeBinSigma_);

	domCache_.UpdateData(*readout_window, pulses, exclusions,
	    timeBinPhotons_, timeBinSigma_, minWidth_, useUnhit_);
}

template class I3MillipedeBase<I3Module>;
template class I3MillipedeBase<I3ConditionalModule>;
template class I3MillipedeBase<I3ServiceBase>;

