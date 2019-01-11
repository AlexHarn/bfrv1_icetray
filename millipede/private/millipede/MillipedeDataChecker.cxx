#include <numeric>
#include <millipede/Millipede.h>

/// @brief: Check pulses and binning thereof for crippling errors
class MillipedeDataChecker : public I3MillipedeConditionalModule {
public:
	using I3MillipedeConditionalModule::I3MillipedeConditionalModule;
	void Physics(I3FramePtr frame)
	{
		if (!frame->Has(pulses_name_)) {
			PushFrame(frame);
			return;
		}
		
		const I3RecoPulseSeriesMap& pulsemap =
		    frame->Get<I3RecoPulseSeriesMap>(pulses_name_);
		
		I3TimeWindowSeries readout_window;
		{
			I3TimeWindowConstPtr rw =
			    frame->Get<I3TimeWindowConstPtr>(readout_window_name_);
			if (!rw)
				log_fatal("No readout window named \"%s\" present in frame",
				    readout_window_name_.c_str());
			if (!std::isfinite(rw->GetLength()))
				log_fatal("Readout window \"%s\" not of finite length",
				    readout_window_name_.c_str());
			readout_window.push_back(*rw);
		}
	
		I3TimeWindowSeriesMap exclusions;
		for(const std::string &mapname : exclusions_name_) {
			I3TimeWindowSeriesMapConstPtr exclusions_segment =
			    frame->Get<I3TimeWindowSeriesMapConstPtr>(mapname);
			I3VectorOMKeyConstPtr excludedoms =
			    frame->Get<I3VectorOMKeyConstPtr>(mapname);
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
				for(const OMKey &key : *excludedoms)
					exclusions[key].push_back(I3TimeWindow());
			}
		}
		
		DatamapFromFrame(*frame);

		// Verify mapping of pulses to bins
		for (auto &entry : pulsemap) {
			auto &cache = domCache_.find(entry.first)->second;
			auto &pulses = entry.second;
			for (int i=0; i < cache.nbins; i++) {
				if (!cache.valid[i])
					continue;
			
				// charge in the bin must be equal to the sum of charges of pulses
				// that start in [left,right)
				auto begin = std::lower_bound(pulses.begin(), pulses.end(), cache.time_bin_edges[i],
				    [](const I3RecoPulse &p, double t) { return p.GetTime() < t; });
				auto end = std::lower_bound(pulses.begin(), pulses.end(), cache.time_bin_edges[i+1],
				    [](const I3RecoPulse &p, double t) { return p.GetTime() < t; });
				auto qtot = std::accumulate(begin, end, 0.,
				    [](double q, const I3RecoPulse &p) { return q+p.GetCharge(); });
				i3_assert(qtot == cache.charges[i]);
			}
		}
	
		// Verify construction of time bins
		for (const auto &entry : domCache_) {
			auto &cache = entry.second;
		
			// Since first and last bins are never coalesced, there can be
			// either 0, 1, or >= 3 bins, but never 2
			i3_assert(cache.nbins >= 0 && cache.nbins != 2);
		
			for (int i=0; i < cache.nbins; i++) {
				// all bins must have strictly positive width
				i3_assert(cache.time_bin_edges[i+1] > cache.time_bin_edges[i]);
				// Bins may be coalesced if all of the following conditions are
				// true:
				// a) they are neither the first nor the last
				// b) there are more than 3 bins (so (a) can apply)
				// c) they are not marked invalid, nor border an invalid bin
				if ((i > 0) && (i+1 < cache.nbins) && (cache.nbins > 3) &&
				    (cache.valid[i] && cache.valid[i-1] && cache.valid[i+1])) {
					if (!std::isfinite(timeBinSigma_)) {
						// the bin must have either minimum charge or minimum 
						// width (unless the combined bin would be longer than
						// 200 ns)
						i3_assert(cache.charges[i] >= timeBinPhotons_ ||
						    cache.time_bin_edges[i+1] - cache.time_bin_edges[i] >= minWidth_ ||
							cache.time_bin_edges[i+2] - cache.time_bin_edges[i] >= 200.);
					} else {
						// fitness score: saturated Poisson log-likelihood +
						// penalty term against narrow bins
						auto sat_llh = [](double n, double dt, double minWidth)
						    { return (n > 0 ? n*std::log(n/dt) : 0) - minWidth/dt; };
						double l1 = sat_llh(cache.charges[i],
						    cache.time_bin_edges[i+1] - cache.time_bin_edges[i],
						    minWidth_);
						double l2 = sat_llh(cache.charges[i+1],
						    cache.time_bin_edges[i+2] - cache.time_bin_edges[i+1],
						    minWidth_);
						double lcomb = sat_llh(cache.charges[i]+cache.charges[i+1],
						    cache.time_bin_edges[i+2] - cache.time_bin_edges[i],
						    minWidth_);
						// the fitness of neighboring bins must exceed the
						// combined fitness by at least sigma^2/2
						i3_assert(l1+l2-lcomb > timeBinSigma_*timeBinSigma_/2);
					}
				}
			
				if (cache.valid[i]) {
					i3_assert(std::isfinite(cache.noise[i]));
					i3_assert(cache.noise[i] > 0);
				}
			}
		}
	
		for (const auto &entry : domCache_) {
			auto &cache = entry.second;
			auto tws = exclusions.find(entry.first);
			if (tws != exclusions.end()) {
				for (auto &window : tws->second) {
					if (std::isinf(window.GetStart()) && std::isinf(window.GetStop())) {
						i3_assert(cache.nbins == 0);
						continue;
					}
				
					double *begin = cache.time_bin_edges;
					double *end = begin+cache.nbins+1;
					auto left = std::lower_bound(begin, end, window.GetStart());
					auto right = std::lower_bound(begin, end, window.GetStop());
					i3_assert(left != end);
					i3_assert(right != end);
					i3_assert(*left == window.GetStart());
					i3_assert(*right == window.GetStop());
				
					i3_assert(right-left == 1);
					i3_assert(cache.valid[left-begin] == false);
				}
				i3_assert(cache.nbins == 0 || cache.valid.count() == cache.nbins - tws->second.size());
				
			} else {
				i3_assert(cache.nbins >= 1);
				i3_assert(cache.valid.count() == unsigned(cache.nbins));
			}
			if (cache.nbins > 0) {
				i3_assert(readout_window.front().GetStart() == cache.time_bin_edges[0]);
				i3_assert(readout_window.front().GetStop() == cache.time_bin_edges[cache.nbins]);
			}
		}
	
		PushFrame(frame);
	}
private:
	SET_LOGGER("MillipedeDataChecker");
};

I3_MODULE(MillipedeDataChecker);

