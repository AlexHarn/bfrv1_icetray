#include <millipede/Millipede.h>
#include <boost/python.hpp>
#include <boost/foreach.hpp>

class PyMillipede : public I3MillipedeConditionalModule
{
	public:
		PyMillipede(const I3Context &);
		void Configure();
		void Physics(I3FramePtr frame);
	private:
		SET_LOGGER("PyMillipede");
		
		void CallDebug(cholmod_sparse *response_matrix,
		    const I3Vector<I3Particle> &solutions, I3FramePtr frame);

		boost::python::object hypothesis_;
		boost::python::object callback_;
		std::string output_name;
};

I3_MODULE(PyMillipede);

PyMillipede::PyMillipede(const I3Context &context) :
    I3MillipedeConditionalModule(context)
{
	AddParameter("Hypothesis",
	    "Python function returning a source list", 0);
	AddParameter("Callback",
	    "Python function to call with the event data and hypotheses", 0);
	AddParameter("Output", "Name of output", "MillipededEdx");
}

void
PyMillipede::Configure()
{
	I3MillipedeConditionalModule::Configure();

	GetParameter("Output", output_name);
	GetParameter("Hypothesis", hypothesis_);
	GetParameter("Callback", callback_);
}

void
PyMillipede::CallDebug(cholmod_sparse *response_matrix,
    const I3Vector<I3Particle> &sources, I3FramePtr frame)
{
	namespace bp = boost::python;
	
	if (!callback_)
		return;
	
	bp::dict mappy;
	cholmod_dense *response =
	    cholmod_l_sparse_to_dense(response_matrix, &c);
	
	int row = 0;	
	BOOST_FOREACH(const MillipedeDOMCacheMap::value_type &pair, domCache_) {
		bp::dict entry;
		bp::list edges, charges, noise, valid;
		for (int i=0; i < pair.second.nbins; i++) {
			edges.append(pair.second.time_bin_edges[i]);
			charges.append(pair.second.charges[i]);
			noise.append(pair.second.noise[i]);
			valid.append(pair.second.valid[i]);
		}
		edges.append(pair.second.time_bin_edges[pair.second.nbins]);
		
		entry["time_bin_edges"] = edges;
		entry["charges"] = charges;
		entry["valid"] = valid;
		entry["noise"] = noise;
		entry["light_scale"] = pair.second.light_scale;
		entry["geo"] = pair.second.geo;
		
		bp::list bases;
		for (unsigned i=0; i < sources.size(); i++)
			bases.append(bp::list());
		
		// NB: invalid entries do not appear in the response matrix!
		for (int i=0; i < pair.second.nbins; i++) {
			int col = 0;
			if (pair.second.valid[i]) {
				BOOST_FOREACH(const I3Particle &source __attribute__((unused)), sources) {
					// NB: cholmod_dense is in column-major order
					double elem = ((double*)(response->x))
					    [row + col*response->nrow];
					bases[col].attr("append")(elem);
					col++;
				}
				row++;
			} else {
				BOOST_FOREACH(const I3Particle &source __attribute__((unused)), sources)
					bases[col++].attr("append")(0.);
			}
		}
		
		entry["bases"] = bases;
		
		mappy[pair.first] = entry;
	}
	
	cholmod_l_free_dense(&response, &c);
	
	callback_(frame, sources, mappy);
}

void
PyMillipede::Physics(I3FramePtr frame)
{
	if (!frame->Has(pulses_name_)) {
		PushFrame(frame);
		return;
	}

	boost::shared_ptr<I3Vector<I3Particle> > sources(new
	   I3Vector<I3Particle>);
	I3Vector<I3Particle> pysources =
	   boost::python::extract<I3Vector<I3Particle> >(hypothesis_(frame));
	for (unsigned i = 0; i < pysources.size(); i++)
		sources->push_back(pysources[i]);

	// If we don't have any sources, don't do anything
	if (sources->size() == 0) {
		PushFrame(frame);
		return;
	}

	MillipedeFitParamsPtr seed_params(new MillipedeFitParams);
	MillipedeFitParamsPtr params(new MillipedeFitParams);
	cholmod_sparse *response_matrix;

	DatamapFromFrame(*frame);
	response_matrix = GetResponseMatrix(*sources);
	if (response_matrix == NULL)
		log_fatal("Null basis matrix");
	Millipede::FitStatistics(domCache_, *sources, 0, response_matrix,
	    &*seed_params, &c);

	SolveEnergyLosses(*sources, response_matrix);
	Millipede::FitStatistics(domCache_, *sources, 0, response_matrix,
	    &*params, &c);
	CallDebug(response_matrix, *sources, frame);
	cholmod_l_free_sparse(&response_matrix, &c);

	frame->Put(output_name, sources);
	frame->Put(output_name + "FitParams", params);
	frame->Put(output_name + "SeedParams", seed_params);

	PushFrame(frame);
}

