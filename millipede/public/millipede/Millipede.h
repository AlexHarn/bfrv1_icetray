#ifndef MILLIPEDE_H
#define MILLIPEDE_H

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3ServiceBase.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3TimeWindow.h>
#include <dataclasses/I3Matrix.h>
#include <gulliver/I3LogLikelihoodFitParams.h>
#include <photonics-service/I3PhotonicsService.h>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>
#include <boost/dynamic_bitset.hpp>

#include <cholmod.h>

struct MillipedeDOMCache {
	I3OMGeo geo;
	double light_scale;

	int nbins;
	double *time_bin_edges;
	double *charges;
	double *noise;
	boost::dynamic_bitset<uint8_t> valid;

	bool amplitudes_only;
	double noise_rate;
};

class MillipedeDOMCacheMap : public std::map<OMKey, struct MillipedeDOMCache> {
	public:
		~MillipedeDOMCacheMap();

		void UpdateParams(I3GeometryConstPtr geometry,
		    I3CalibrationConstPtr calib,
		    I3DetectorStatusConstPtr status);
		void UpdateData(const I3TimeWindow &readout_window,
		    const I3RecoPulseSeriesMap &pulses,
		    const I3TimeWindowSeriesMap &exclusions,
		    double PEPerBin, double BayesianBlockSigma,
		    double min_width, bool useUnhit);
		
		static
		std::vector<std::pair<std::vector<double>, std::vector<double>>>
		BinPulses(const I3RecoPulseSeries &pulses,
		    const I3TimeWindowSeries &readouts,
		    double PEPerBin, double BayesianBlockSigma, double min_width);
		
	private:
		I3GeometryConstPtr geometry_;
		I3CalibrationConstPtr calib_;
		I3DetectorStatusConstPtr status_;
};

class MillipedeFitParams : public I3LogLikelihoodFitParams {
	public:
		MillipedeFitParams();

		double qtotal;
		double predicted_qtotal;
		double squared_residuals;
		double chi_squared;
		double chi_squared_dof;
		double logl_ratio;
	protected:
		friend class icecube::serialization::access;
		template <class Archive> void serialize(Archive& ar,
		    unsigned version);
};

std::ostream& operator<<(std::ostream& oss, const MillipedeFitParams& d);

I3_POINTER_TYPEDEFS(MillipedeFitParams);
I3_CLASS_VERSION(MillipedeFitParams, 2);

// Main millipede functions that implement all the algorithms
namespace Millipede {
	cholmod_sparse *GetResponseMatrix(const MillipedeDOMCacheMap &datamap,
	    const std::vector<I3Particle> &sources, double dom_efficiency,
	    I3PhotonicsServicePtr muon_p, I3PhotonicsServicePtr cascade_p,
	    cholmod_sparse **gradients /* optional */, cholmod_common *c);
	void SolveEnergyLosses(MillipedeDOMCacheMap &datamap,
	    std::vector<I3Particle> &sources, cholmod_sparse *response_matrix,
	    cholmod_sparse *gradients /* optional */,
	    double muonRegularization, double stochasticRegularization,
	    cholmod_common *c);
	double FitStatistics(const MillipedeDOMCacheMap &datamap,
	    const std::vector<I3Particle> &sources, double energy_epsilon,
	    cholmod_sparse *response_matrix, MillipedeFitParams *params,
	    cholmod_common *c);
	I3MatrixPtr FisherMatrix(const MillipedeDOMCacheMap &datamap,
	    const std::vector<I3Particle> &sources,
	    cholmod_sparse *response_matrix, cholmod_common *c);
	void LLHGradient(const MillipedeDOMCacheMap &datamap,
	    const std::vector<I3Particle> &sources,
	    std::vector<I3Particle> &gradsources, double energy_epsilon, double weight,
	    cholmod_sparse *response_matrix, cholmod_sparse *gradients,
	    cholmod_common *c);
};

// Time-saving base class for modules/services using Millipede. You can
// optionally inherit from this to reduce some boilerplate copying and pasting.
//
// Note: only works for things that inherit from I3ServiceBase, I3Module,
//  or I3ConditionalModule
template<class Base>
class I3MillipedeBase : public Base {
	public:
		I3MillipedeBase<Base>(const I3Context &context);
		virtual ~I3MillipedeBase<Base>();
		void Configure();

	protected:
		void DatamapFromFrame(const I3Frame &frame);
		cholmod_sparse *GetResponseMatrix(
		    const std::vector<I3Particle> &sources,
		    cholmod_sparse **gradients = NULL) {
			return Millipede::GetResponseMatrix(domCache_, sources,
			    domEfficiency_, muon_p, cascade_p, gradients, &c);
		}
		void SolveEnergyLosses(std::vector<I3Particle> &sources,
		    cholmod_sparse *response_matrix, 
		    cholmod_sparse *gradients = NULL) {
			Millipede::SolveEnergyLosses(domCache_, sources,
			    response_matrix, gradients, 
			    regularizeMuons_, regularizeStochastics_, &c);
		}

		I3PhotonicsServicePtr muon_p, cascade_p;
		double regularizeMuons_;
		double regularizeStochastics_;
		MillipedeDOMCacheMap domCache_;
		std::string pulses_name_;
		std::string readout_window_name_;
		std::vector<std::string> exclusions_name_;
		bool partial_exclusion_;
		double domEfficiency_; 
		double timeBinPhotons_;
		double timeBinSigma_;
		double minWidth_;
		bool useUnhit_;

		cholmod_common c;

	private:
		SET_LOGGER("MillipedeBase");
};

// NB: Only the following are instantiated
typedef I3MillipedeBase<I3Module> I3MillipedeModule;
typedef I3MillipedeBase<I3ConditionalModule> I3MillipedeConditionalModule;
typedef I3MillipedeBase<I3ServiceBase> I3MillipedeService;

#endif //MILLIPEDE_H

