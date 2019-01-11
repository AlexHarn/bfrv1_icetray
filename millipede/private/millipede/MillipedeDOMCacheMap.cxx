#include <icetray/I3Units.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3TimeWindow.h>
#include <dataclasses/I3DOMFunctions.h>
#include <photonics-service/I3PhotonicsService.h>
#include <millipede/Millipede.h>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <vector>
#include <stack>
#include <cmath>
#include <numeric>

#ifndef NDEBUG
#include <dataclasses/external/CompareFloatingPoint.h>
#endif

#include <cholmod.h>

using namespace std; // Get C99 math functions back

void
MillipedeDOMCacheMap::UpdateParams(I3GeometryConstPtr geometry,
    I3CalibrationConstPtr calib, I3DetectorStatusConstPtr status)
{
	bool recompute = false;

	if (geometry != geometry_) {
		geometry_ = geometry;
		recompute = true;
	}
	if (calib != calib_) {
		calib_ = calib;
		recompute = true;
	}
	if (status != status_) {
		status_ = status;
		recompute = true;
	}

	if (!recompute)
		return;

	// Clear map
	BOOST_FOREACH(MillipedeDOMCacheMap::value_type &pair, (*this)) {
		delete [] pair.second.time_bin_edges;
		delete [] pair.second.charges;
		delete [] pair.second.noise;
	}
	clear();

	// Are we missing something? Missing bad OMs is OK
	if (!geometry_ || !calib_ || !status_)
		return;

	// Recompute everything now we have all the parameters
	for (I3OMGeoMap::const_iterator i = geometry_->omgeo.begin();
	    i != geometry_->omgeo.end(); i++) {
		if (i->second.omtype != I3OMGeo::IceCube)
			continue;

		MillipedeDOMCache cache;
		cache.geo = i->second;
		cache.nbins = -1;
		cache.time_bin_edges = NULL;
		cache.charges = NULL;
		cache.noise = NULL;
		cache.amplitudes_only = true;		

		// Only in-ice DOMs
		if (cache.geo.omtype != I3OMGeo::IceCube)
			continue;
		
		// Check if the DOM is on
		std::map<OMKey, I3DOMStatus>::const_iterator domStatus =
		    status_->domStatus.find(i->first);
		// Absent can mean off, too
		if (domStatus == status_->domStatus.end())
			continue;
		// Skip DOMs that are off
		double voltage = domStatus->second.pmtHV;
		if (fabs(voltage) < 100.*I3Units::V || !isfinite(voltage))
			continue;

		// Compute calibration params
		std::map<OMKey, I3DOMCalibration>::const_iterator domCalib =
		    calib_->domCal.find(i->first);
		if (domCalib == calib_->domCal.end()) {
			log_warn("OM (%d,%d) has no calibration, skipping",
			    i->first.GetString(),
			    i->first.GetOM());
			continue;
		}

		cache.light_scale = MeanSPECharge(domStatus->second,
		    domCalib->second);
		double rde = domCalib->second.GetRelativeDomEff();
		if (isfinite(rde))
			cache.light_scale *= rde;

		cache.noise_rate = 850*I3Units::hertz;
		if (isfinite(domCalib->second.GetDomNoiseRate()))
			cache.noise_rate = domCalib->second.GetDomNoiseRate();

		if (!isfinite(cache.noise_rate) || cache.noise_rate <= 0 ||
		    cache.noise_rate > 10000*I3Units::hertz) {
			log_warn("OM (%d,%d) has an invalid noise rate "
			    "(%e Hz)! Assuming this is bogus, clamping to 850 "
			    " Hz.", domCalib->first.GetString(),
			    domCalib->first.GetOM(),
			    cache.noise_rate/I3Units::hertz);
			cache.noise_rate = 850*I3Units::hertz;
		}

		(*this)[i->first] = cache;
	}
}

static int
rebin_min_entries(int raw_bins, const double *raw_bin_edges,
    const double *raw_charges, double *bin_edges,
    double *charges, double PEPerBin, double min_width)
{
	int p, q;
	for (p = 0, q = 0; p < raw_bins; p++, q++) {
		bin_edges[q] = raw_bin_edges[p];
		charges[q] = raw_charges[p];

		// Combine bins until the either the charge threshold or maximum
		// bin width is exceeded, or an an invalid bin is encountered.
		// Empty bins have an no maximum width, since the original
		// binning scheme never produces consecutive empty bins.
		// TODO: make the maximum width settable?
		while (p > 0 && p < raw_bins-1 &&
		    (charges[q] < PEPerBin ||
		     raw_bin_edges[p+2]-bin_edges[q] < min_width) &&
		    raw_bin_edges[p+2]-bin_edges[q] < 200.)
			charges[q] += raw_charges[++p];
	}
	assert(q <= raw_bins);
	assert(bin_edges[q-1] < raw_bin_edges[raw_bins]);
	bin_edges[q] = raw_bin_edges[raw_bins];
	
	return q;
}

// Find a binning that optimally reflects statistically significant changes
// in the underlying rate.
// Based on Scargle's algorithm, as outlined in
// http://adsabs.harvard.edu/abs/2012arXiv1207.5578S
static int
rebin_bayesian_blocks(int raw_bins, const double *raw_bin_edges,
    const double *raw_charges, double *bin_edges, double *charges,
    double ncp_prior, double min_width)
{
	std::vector<double> best(raw_bins,
	    -std::numeric_limits<double>::infinity());
	std::vector<int> last(raw_bins, 0);
	
	// sum of raw_charges[:k+1]
	double total = 0;
	for (int k = 1; k < raw_bins-1; k++) {
		total += raw_charges[k];
		// sum of raw_charges[:j]
		double subtotal = 0.;
		for (int j = 0; j < k+1; j++) {
			// sum of raw_charges[j:k+1] (contents of proposedblock)
			double counts = total - subtotal;
			// width of proposed block
			double width = raw_bin_edges[k+1]-raw_bin_edges[j];
			// the fitness of the block is a saturated Poisson
			// likelihood, penalized by a constant factor for each
			// extra block
			double fitness = (counts > 0 ? counts*(std::log(counts)
			    - std::log(width)) : 0) + (j > 0 ? best[j-1] : 0) -
			    ncp_prior - min_width/width;
			if (fitness > best[k]) {
				best[k] = fitness;
				last[k] = j;
			}
			subtotal += raw_charges[j];
		}
	}
	
	// Recover changepoints by iteratively peeling off the last block
	std::stack<int> change_points;
	for (int i = raw_bins; i > 0; i = last[i-1])
		change_points.push(i);
	
	// Fill contents of blocks into output
	int p, q;
	for (p = 0, q = 0; p < raw_bins; q++, change_points.pop()) {
		assert(!change_points.empty());
		bin_edges[q] = raw_bin_edges[p];
		charges[q] = 0;
		while (p < change_points.top())
			charges[q] += raw_charges[p++];
	}
	bin_edges[q] = raw_bin_edges[p];
	
	assert(q <= raw_bins);
	return q;
}

static
std::pair<std::vector<double>, std::vector<double>>
bin_pulses(I3RecoPulseSeries::const_iterator begin, I3RecoPulseSeries::const_iterator end, const I3TimeWindow &readout_window)
{
	std::vector<double> bin_edges;
	std::vector<double> charges;
	
	bin_edges.reserve(2*std::distance(begin, end)+1);
	charges.reserve(2*std::distance(begin, end));
	
	double orphan_charge = 0;
	bin_edges.push_back(readout_window.GetStart());
	charges.push_back(0);
	for (auto p = begin; p != end; p++) {
		// Add one bin containing the pulse followed by a second that continues
		// to the next pulse.
		// If two pulses are extracted within 0.5ns of each other,
		// they are encoded to the same time and sdst decoder sets
		// the width of the first pulse to 0 to prevent them from overlapping.
		// Add the charge of all 0 width pulses to first following pulse with
		// non 0 width
		if (p->GetWidth() > 0) {
			bin_edges.push_back(p->GetTime());
			bin_edges.push_back(p->GetTime() + p->GetWidth());
			charges.push_back(p->GetCharge() + orphan_charge);
			charges.push_back(0);
			orphan_charge = 0;
		} else {
			orphan_charge += p->GetCharge();
		}
	}
	bin_edges.push_back(readout_window.GetStop());
	
	// Clean any zero or negative width bins resulting from pulse
	// width round-off error. Serious problems resulting from
	// this are checked for above.
	for (size_t i = 0; i < charges.size(); i++) {
		if (bin_edges[i+1] <= bin_edges[i]) {
			// Make sure we only contract null bins
			i3_assert(charges[i] == 0);
			
			// Move all bins after the null bin one slot
			// left, eliminating the null bin we're on
			bin_edges.erase(bin_edges.begin()+i);
			charges.erase(charges.begin()+i);
			
			// Recheck the new bin
			i--;
		}
	}

	return std::make_pair(std::move(bin_edges), std::move(charges));
}

std::vector<std::pair<std::vector<double>, std::vector<double>>>
MillipedeDOMCacheMap::BinPulses(const I3RecoPulseSeries &pulses, const I3TimeWindowSeries &readouts, double PEPerBin, double BayesianBlockSigma, double min_width)
{
	std::vector<std::pair<std::vector<double>, std::vector<double>>> histograms;
	
	auto begin(pulses.cbegin()), end(pulses.cend());
	for (auto &readout : readouts) {
		auto pulse_time_sort = [](const I3RecoPulse &p, double t) { return p.GetTime() < t; };
		begin = std::lower_bound(begin, end, readout.GetStart(), pulse_time_sort);
		end = std::lower_bound(begin, end, readout.GetStop(), pulse_time_sort);
		
		std::vector<double> bin_edges, charges;
		
		if (PEPerBin < 0) {
			bin_edges = {readout.GetStart(), readout.GetStop()};
			charges = {std::accumulate(begin, end, 0.,
			    [](double q, const I3RecoPulse &p) { return q+p.GetCharge(); })};
		} else {
			// do stuff with pulses and readout
			std::vector<double> raw_bin_edges, raw_charges;
			std::tie(raw_bin_edges, raw_charges) = bin_pulses(begin, end, readout);
			
			i3_assert(raw_bin_edges.size() >= 2u);
			i3_assert(raw_charges.size() >= 1u);
			i3_assert(std::is_sorted(raw_bin_edges.begin(), raw_bin_edges.end()));
			
			bin_edges.resize(raw_bin_edges.size());
			charges.resize(raw_charges.size());
			
			int nbins = 0;
			if (std::isfinite(BayesianBlockSigma)) {
				// Combine bins until the PE/ns rates in each
				// bin are different in a statistically
				// significant way. For each proposed partition,
				// compare the saturated Poisson log-likelihood
				// for the entire block against the sum of
				// log-likelihoods for each sub-partition, and
				// choose the configuration with the greater
				// likelihood. Unless the entire block is empty,
				// the partition will have the greater
				// likelihood; ncp_prior is the minimum
				// log-likelihood difference required to
				// partition the block, controlling the
				// complexity of the final binning scheme.
				double ncp_prior =
				    BayesianBlockSigma*BayesianBlockSigma/2.;
				nbins = rebin_bayesian_blocks(raw_charges.size(),
				    raw_bin_edges.data(), raw_charges.data(),
				    bin_edges.data(), charges.data(), ncp_prior,
				    min_width);
			} else {
				// Combine bins until each contains at least PEPerBin of charge
				nbins = rebin_min_entries(raw_charges.size(),
				    raw_bin_edges.data(), raw_charges.data(),
				    bin_edges.data(), charges.data(), PEPerBin,
				    min_width);
			}
			charges.resize(nbins);
			bin_edges.resize(nbins+1);
			
			i3_assert(std::is_sorted(bin_edges.begin(), bin_edges.end()));
			
			double raw_qtot = std::accumulate(raw_charges.begin(), raw_charges.end(), 0.);
			double qtot = std::accumulate(charges.begin(), charges.end(), 0.);
			
			i3_assert(charges.size() <= raw_charges.size());
			i3_assert(bin_edges.size() <= raw_bin_edges.size());
			i3_assert(raw_qtot == qtot);
			
		}
		
		histograms.emplace_back(std::move(bin_edges), std::move(charges));
		
		begin = end;
		end = pulses.cend();
	}

	return histograms;
}

void
MillipedeDOMCacheMap::UpdateData(const I3TimeWindow &readout_window,
    const I3RecoPulseSeriesMap &pulses, const I3TimeWindowSeriesMap &exclusions,    double PEPerBin, double BayesianBlockSigma, double min_width, bool useUnhit)
{
	// Check for madness
	for (I3RecoPulseSeriesMap::const_iterator ps = pulses.begin();
	    ps != pulses.end(); ps++) {
		if (this->find(ps->first) == this->end())
			log_fatal("Pulse on OM(%d,%d) that is not part of the "
			    "detector configuration", ps->first.GetString(),
			    ps->first.GetOM());
		
		for (auto p = ps->second.begin(); p != ps->second.end(); p++) {
			// Sanity checks: pulses must be ordered,
			// non-overlapping, have defined widths,
			// and occur between the beginning and end
			// of the event
			if (p->GetTime() < readout_window.GetStart())
				log_fatal("Pulse time (%f ns) before readout "
				    "window start (%f ns)",
				    p->GetTime()/I3Units::ns,
				    readout_window.GetStart()/I3Units::ns);
			// Note: pulse + width > window is OK since the last
			// bin edge is truncated to the width of the window
			// below. The resulting bin would have zero width
			// only if pulse == window.
			if (p->GetTime() >= readout_window.GetStop())
				log_fatal("Pulse time (%f+%f ns) after readout "
				    "window end (%f ns) on OM(%d,%d)",
				    p->GetTime()/I3Units::ns,
				    p->GetWidth()/I3Units::ns,
				    readout_window.GetStop()/I3Units::ns,
				    ps->first.GetString(), ps->first.GetOM());
			i3_assert(p->GetWidth() > 0);
			i3_assert((p+1) == ps->second.end() ||
			    p->GetTime() + 0.9*p->GetWidth() <=
			    (p+1)->GetTime());
		}

	}

	// With no madness, update pulse map
	for (MillipedeDOMCacheMap::iterator i = this->begin();
	    i != this->end(); i++) {
		MillipedeDOMCache &cache = i->second;

		cache.nbins = 1;
		cache.amplitudes_only = (PEPerBin < 0);
		if (cache.time_bin_edges)
			delete [] cache.time_bin_edges;
		if (cache.charges)
			delete [] cache.charges;
		if (cache.noise)
			delete [] cache.noise;
		cache.valid.clear();

		I3RecoPulseSeriesMap::const_iterator ps = pulses.find(i->first);
		I3TimeWindowSeries effective_readouts;
		effective_readouts.push_back(readout_window);
		{
			I3TimeWindowSeriesMap::const_iterator tws =
			    exclusions.find(i->first);
			// The effective readout windows are the set difference of the
			// global readout window and exclusion windows
			if (tws != exclusions.end()) {
				effective_readouts = effective_readouts & (~(tws->second));
			}
		}
		
		// Bin pulse charges in time in the face of user options and
		// exclusion windows
		I3RecoPulseSeries dummy;
		std::vector<std::pair<std::vector<double>, std::vector<double>>>
		    histograms = BinPulses(ps == pulses.end() ? dummy : ps->second,
		    effective_readouts, PEPerBin, BayesianBlockSigma, min_width);
		
		// the gap between histograms is an invalid bin
		cache.nbins = histograms.empty() ? 0 : histograms.size()-1;
		for (const auto &hist : histograms) {
			cache.nbins += hist.second.size();
		}
		
		cache.time_bin_edges = new double[cache.nbins == 0 ? 0 : cache.nbins+1];
		cache.charges = new double[cache.nbins];
		cache.valid.resize(cache.nbins, true);
		
		// Concatenate binned readouts, inserting an invalid bin between
		auto edge = cache.time_bin_edges;
		auto charge = cache.charges;
		for (const auto &hist : histograms) {
			if (edge != cache.time_bin_edges) {
				// insert an invalid bin
				cache.valid[std::distance(cache.charges, charge)] = false;
				*(charge++) = 0.;
			}
			edge = std::copy(hist.first.begin(), hist.first.end(), edge);
			charge = std::copy(hist.second.begin(), hist.second.end(), charge);
		}
		
		if (!useUnhit) {
			if (std::accumulate(cache.charges, cache.charges+cache.nbins, 0.) == 0.)
				cache.valid.reset();
		}
		
		// Create noise records
		cache.noise = new double[cache.nbins];
		for (int j = 0; j < cache.nbins; j++) {
			assert(cache.time_bin_edges[j+1] -
			    cache.time_bin_edges[j] > 0);

			if (!cache.valid[j])
				continue;
			cache.noise[j] = cache.noise_rate *
			  (cache.time_bin_edges[j+1] - cache.time_bin_edges[j]);
			assert(isfinite(cache.noise[j]));
			assert(cache.noise[j] > 0);
		}
	}
}

MillipedeDOMCacheMap::~MillipedeDOMCacheMap()
{
	BOOST_FOREACH(MillipedeDOMCacheMap::value_type &pair, (*this)) {
		delete [] pair.second.time_bin_edges;
		delete [] pair.second.charges;
		delete [] pair.second.noise;
	}
	clear();
}

struct photo_source {
        PhotonicsSource source;
        I3PhotonicsServicePtr service;
        const I3Particle *particle;
};

static int
MillipedeAddOMSourcePairToMatrix(MillipedeDOMCacheMap::const_iterator om,
    std::vector<photo_source>::const_iterator src, double dom_efficiency,
    cholmod_triplet *basis_trip, cholmod_triplet *gradient_triplet,
    int i, int j)
{
	double mean_pes, epointdistance, geotime;
	double amp_gradients[6];

	if (om->second.valid.count() == 0)
		return 0;

	src->service->SelectSource(mean_pes,
	    (gradient_triplet == NULL) ? NULL : amp_gradients,
	    epointdistance, geotime, src->source);

	if (mean_pes <= 0)
		return 0;

	mean_pes *= om->second.light_scale;
	mean_pes *= dom_efficiency;

	if (gradient_triplet != NULL) {
		// Correct for the odd coordinates of gradients
		// For amplitudes, they are X, Y, Z, zen, azi, E
		// For quantiles, it is X, Y, Z, t, zen, azi
		// Adopt the quantile coordinate system, since
		// we don't care about E

		amp_gradients[5] = amp_gradients[4];
		amp_gradients[4] = amp_gradients[3];
		amp_gradients[3] = 0; // dAmp/dt = 0
		for (int k = 0; k < 6; k++)
			amp_gradients[k] *= dom_efficiency*
			    om->second.light_scale;
	}

	if (om->second.nbins == 1) {
		((long *)(basis_trip->i))[basis_trip->nnz] = i;
		((long *)(basis_trip->j))[basis_trip->nnz] = j;
		((double *)(basis_trip->x))[basis_trip->nnz] = mean_pes;
		basis_trip->nnz++;
		if (gradient_triplet != NULL) {
			for (int k = 0; k < 6; k++) {
				if (amp_gradients[k] == 0)
					continue;
				((long *)(gradient_triplet->i))
				    [gradient_triplet->nnz] = i;
				((long *)(gradient_triplet->j))
				    [gradient_triplet->nnz] = 6*j + k;
				((double *)(gradient_triplet->x))
				    [gradient_triplet->nnz] = amp_gradients[k];
				gradient_triplet->nnz++;
			}
		}
	} else {
		double quantiles[om->second.nbins];
		double qgradients[om->second.nbins][6];
		src->service->GetProbabilityQuantiles(om->second.time_bin_edges,
		    geotime + src->particle->GetTime(), quantiles,
		    (gradient_triplet == NULL) ? NULL : qgradients,
		    om->second.nbins);
		int r = -1;
		for (int q = 0; q < om->second.nbins; q++) {
			if (!om->second.valid[q])
				continue;

			r++;
			((long *)(basis_trip->i))[basis_trip->nnz] = i+r;
			((long *)(basis_trip->j))[basis_trip->nnz] = j;
			((double *)(basis_trip->x))[basis_trip->nnz] =
			    mean_pes*quantiles[q];
			if (quantiles[q] <= 0)
				continue;
			basis_trip->nnz++;

			if (gradient_triplet == NULL)
				continue;
			for (int k = 0; k < 6; k++) {
				double qg = amp_gradients[k]*quantiles[q] +
				    qgradients[q][k]*mean_pes;
				if (qg == 0)
					continue;
				((long *)(gradient_triplet->i))
				    [gradient_triplet->nnz] = i+r;
				((long *)(gradient_triplet->j))
				    [gradient_triplet->nnz] = 6*j+k;
				((double *)(gradient_triplet->x))
				    [gradient_triplet->nnz] = qg;

				gradient_triplet->nnz++;
			}
		}
	}

	return om->second.valid.count();
}

cholmod_sparse *
Millipede::GetResponseMatrix(const MillipedeDOMCacheMap &datamap,
    const std::vector<I3Particle> &sources, double dom_efficiency,
    I3PhotonicsServicePtr muon_p, I3PhotonicsServicePtr cascade_p,
    cholmod_sparse **gradients, cholmod_common *c)
{
	// Calculate the number of data bins we have
	int ndata = 0;
	for (MillipedeDOMCacheMap::const_iterator i = datamap.begin();
	    i != datamap.end(); i++) {
		ndata += i->second.valid.count();
	}

	// Bail and return a dummy matrix if no sources
	if (sources.size() == 0)
		return cholmod_l_allocate_sparse(ndata, 0, 0, true, true, 0,
		    CHOLMOD_REAL, c);

	// Allocate base triplet matrices
	cholmod_triplet *basis_trip = cholmod_l_allocate_triplet(
	    ndata, sources.size(), sources.size()*ndata, 0,
	    CHOLMOD_REAL, c);
	basis_trip->nnz = 0;

	cholmod_triplet *gradient_triplet; /* <6 nsources> cols x <data> rows */
	if (gradients != NULL) {
		gradient_triplet = cholmod_l_allocate_triplet(
		    basis_trip->nrow, 6*basis_trip->ncol,
		    6*basis_trip->nzmax, 0, CHOLMOD_REAL, c);
	}

	// Precache photonics sources
	std::vector<photo_source> photo_sources;
	for (std::vector<I3Particle>::const_iterator
	    src = sources.begin(); src != sources.end(); src++) {
		// Check input validity
		assert(src->GetZenith() >= 0 && src->GetZenith() <= M_PI);
		assert(src->GetAzimuth() >= 0 && src->GetAzimuth() < 2*M_PI);

		// Get photonics source
		photo_source ps;
		if (src->IsCascade())
			ps.service = cascade_p;
		else
			ps.service = muon_p;
		ps.source = PhotonicsSource(*src);
		ps.source.length = 0;
		ps.source.E = 1; // GeV
		ps.source.type = 1; // EM cascade scaling
		ps.particle = &*src;

		photo_sources.push_back(ps);
	}

	// Build up the triplet matrix
	int i = 0, j = 0;
	for (std::map<OMKey, MillipedeDOMCache>::const_iterator om =
	    datamap.begin(); om != datamap.end(); om++) {
		if (om->second.valid.count() == 0)
			continue;

		I3Position om_position = om->second.geo.position;
		if (muon_p)
			muon_p->SelectModule(om->second.geo);
		if (cascade_p)
			cascade_p->SelectModule(om->second.geo);
		j = 0;
		for (std::vector<photo_source>::const_iterator
		    src = photo_sources.begin(); src != photo_sources.end();
		    j++, src++) {
			if (!src->service) {
				log_error("Attempting to use "
				    "unconfigured photonics service module!");
				cholmod_l_free_triplet(&basis_trip, c);
				return NULL;
			}

			MillipedeAddOMSourcePairToMatrix(om, src,
			    dom_efficiency, basis_trip,
			    (gradients == NULL) ? NULL : gradient_triplet,
			    i, j);
		}
		i += om->second.valid.count();
	}

	// Check for craziness
	for (unsigned i = 0; i < basis_trip->nnz; i++)
		assert(((double *)(basis_trip->x))[i] > 0);
	
	// Convert basis matrix to sparse form
	cholmod_sparse *basis = cholmod_l_triplet_to_sparse(basis_trip, 0, c);
	cholmod_l_free_triplet(&basis_trip, c);
	
	if (gradients != NULL) {
		*gradients = cholmod_l_triplet_to_sparse(
		    gradient_triplet, 0, c); 
		cholmod_l_free_triplet(&gradient_triplet, c);
	}

	return basis;
}

std::ostream& operator<<(std::ostream& oss, const MillipedeFitParams& p){
	oss << "[ MillipedeFitParams logL : " << p.logl_ << std::endl
	    << "                    rlogL : " << p.rlogl_ << std::endl
	    << "                     Ndof : " << p.ndof_ << std::endl
	    << "                   qtotal : " << p.qtotal << std::endl
	    << "         predicted_qtotal : " << p.predicted_qtotal << std::endl
	    << "        squared_residuals : " << p.squared_residuals << std::endl
	    << "              chi_squared : " << p.chi_squared << std::endl
	    << "          chi_squared_dof : " << p.chi_squared_dof << std::endl
	    << "               logl_ratio : " << p.logl_ratio << std::endl
	    << "]";
	return oss;
}

template <class Archive>
void MillipedeFitParams::serialize(Archive& ar, unsigned version)
{
	icecube::serialization::void_cast_register<MillipedeFitParams, I3FrameObject>(static_cast<MillipedeFitParams *>(NULL), static_cast<I3FrameObject *>(NULL));
	ar & make_nvp("I3LogLikelihoodFitParams",
	    base_object<I3LogLikelihoodFitParams>(*this));
	ar & make_nvp("qtotal", qtotal);
	ar & make_nvp("predicted_qtotal", predicted_qtotal);
	ar & make_nvp("squared_residuals", squared_residuals);
	ar & make_nvp("chi_squared", chi_squared);
	ar & make_nvp("chi_squared_dof", chi_squared_dof);
	if (version > 1)
		ar & make_nvp("logl_ratio", logl_ratio);
}

I3_SERIALIZABLE(MillipedeFitParams);

