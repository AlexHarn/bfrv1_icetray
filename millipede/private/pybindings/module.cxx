
#include <icetray/load_project.h>
#include <millipede/Millipede.h>
#include <millipede/converter/MillipedeFitParamsConverter.h>
#include <tableio/converter/pybindings.h>

#include <icetray/python/dataclass_suite.hpp>

namespace bp = boost::python;

static boost::shared_ptr<cholmod_common> python_common;

void
register_MillipedeFitParams()
{
	bp::class_<MillipedeFitParams, MillipedeFitParamsPtr,
	    bp::bases<I3LogLikelihoodFitParams> >("MillipedeFitParams")
	    .def(bp::dataclass_suite<MillipedeFitParams>())
	    .def_readwrite("qtotal", &MillipedeFitParams::qtotal)
	    .def_readwrite("predicted_qtotal",
	      &MillipedeFitParams::predicted_qtotal)
	    .def_readwrite("squared_residuals",
	      &MillipedeFitParams::squared_residuals)
	    .def_readwrite("chi_squared", &MillipedeFitParams::chi_squared)
	    .def_readwrite("chi_squared_dof",
	      &MillipedeFitParams::chi_squared_dof)
	;
}

struct array_proxy {
	boost::shared_ptr<MillipedeDOMCache> base_;
	double *ptr_;
	int size_;
	array_proxy(boost::shared_ptr<MillipedeDOMCache> b, double *ptr, int size)
	    : base_(b), ptr_(ptr), size_(size) {};
	bp::dict array_interface()
	{
		bp::dict d;
		d["shape"]   = bp::make_tuple(size_);
		d["typestr"] = "<f8";
		d["data"]    = bp::make_tuple((long long)(ptr_), false);
		
		return d;
	}
};

boost::shared_ptr<array_proxy>
get_edges(boost::shared_ptr<MillipedeDOMCache> self)
{
	return boost::make_shared<array_proxy>(self, self->time_bin_edges, self->nbins+1);
}

boost::shared_ptr<array_proxy>
get_charges(boost::shared_ptr<MillipedeDOMCache> self)
{
	return boost::make_shared<array_proxy>(self, self->charges, self->nbins);
}

bp::list
get_valid(boost::shared_ptr<MillipedeDOMCache> self)
{
	bp::list l;
	for (int i=0; i < self->nbins; i++)
		l.append(bool(self->valid[i]));
	
	return l;
}

// Self-destroying cholmod pointers.
namespace detail {

template <typename T>
struct cholmod_deleter {
	typedef int (*deleter_t)(T**, cholmod_common *c);
	cholmod_common *common;
	deleter_t deleter;
	cholmod_deleter(deleter_t d, cholmod_common *c) : common(c), deleter(d) {}
	void operator()(T *ptr)
	{
		if (ptr != NULL)
			deleter(&ptr, common);
	}
};

template <typename T>
typename cholmod_deleter<T>::deleter_t
get_deleter(T *object);

template <>
cholmod_deleter<cholmod_dense>::deleter_t
get_deleter(cholmod_dense*) { return cholmod_l_free_dense; }
template <>
cholmod_deleter<cholmod_sparse>::deleter_t
get_deleter(cholmod_sparse*) { return cholmod_l_free_sparse; }

template <typename T>
boost::shared_ptr<T>
make_shared(T *ptr, cholmod_common *c)
{
	return boost::shared_ptr<T>(ptr,
	    cholmod_deleter<T>(get_deleter(ptr), c));
}

}

I3MatrixPtr
to_I3Matrix(boost::shared_ptr<const cholmod_sparse> sparse)
{
	I3MatrixPtr matrix;
	if (!sparse)
		return matrix;
	
	i3_assert(sparse->packed);
	// Column delimiters
	long *Ap = static_cast<long*>(sparse->p);
	// Row indices
	long *Ai = static_cast<long*>(sparse->i);
	// Values
	double *Ax = static_cast<double*>(sparse->x);
	
	matrix.reset(new I3Matrix(sparse->nrow, sparse->ncol, 0));
	size_t idx = 0;
	for (size_t j=0; j < sparse->ncol; j++) {
		size_t i = Ap[j], end = Ap[j+1];
		for ( ; i < end; i++, idx++)
			(*matrix)(Ai[idx], j) = Ax[idx];
	}
	
	return matrix;
}

boost::shared_ptr<cholmod_sparse>
from_I3Matrix(I3MatrixPtr m)
{
	if (python_common == NULL) {
		python_common.reset(new cholmod_common, &cholmod_l_finish);
		if (!cholmod_l_start(python_common.get()))
			log_fatal("Couldn't initialize cholmod_common!");
	}
	
	// Dress up our matrix to look like a cholmod_dense
	cholmod_dense dense;
	dense.nrow  = m->size1();
	dense.ncol  = m->size2();
	dense.d     = m->size1();
	dense.nzmax = m->data().size();
	dense.xtype = CHOLMOD_REAL;
	dense.dtype = 0;
	dense.x     = (double*)&m->data()[0];
	
	cholmod_sparse *sparse = cholmod_l_dense_to_sparse(&dense, 1,
	    python_common.get());
	
	return detail::make_shared(sparse, python_common.get());
}

/**
 * A Millipede that can be poked at from Python.
 */
class PyPyMillipede : public I3MillipedeService {
public:

	PyPyMillipede(const I3Context &ctx) : I3MillipedeService(ctx) {}	
	MillipedeDOMCacheMap& GetDOMCache() { return domCache_; }
	void DatamapFromFrame(const I3Frame &frame) {
		Configure();
		I3MillipedeService::DatamapFromFrame(frame);
	}
	boost::shared_ptr<cholmod_sparse> GetResponseMatrixPtr(const std::vector<I3Particle> &sources, boost::shared_ptr<cholmod_sparse> *gradient=NULL)
	{
		cholmod_sparse *g;
		cholmod_sparse *r = I3MillipedeService::GetResponseMatrix(sources, gradient ? &g : NULL);
		if (gradient)
			*gradient = detail::make_shared(g, &c);
		return detail::make_shared(r, &c);
	}
	bp::object GetResponseMatrix(const std::vector<I3Particle> &sources, bool with_gradient=false)
	{
		boost::shared_ptr<cholmod_sparse> r, g;
		r = GetResponseMatrixPtr(sources,
		    with_gradient ? &g : NULL);
		if (with_gradient)
			return bp::make_tuple(r, g);
		else
			return bp::object(r);
	}
	void SolveEnergyLosses(std::vector<I3Particle> &sources,
	    boost::shared_ptr<cholmod_sparse> response_matrix, boost::shared_ptr<cholmod_sparse> gradients) {
		Millipede::SolveEnergyLosses(domCache_, sources,
		    response_matrix.get(), gradients.get(), 
		    regularizeMuons_, regularizeStochastics_, &c);
	}
	double FitStatistics(const std::vector<I3Particle> &sources,
	    boost::shared_ptr<cholmod_sparse> response_matrix, MillipedeFitParamsPtr params, double energy_epsilon)
	{
		return Millipede::FitStatistics(domCache_, sources, energy_epsilon, 
		    response_matrix.get(), params.get(), &c);
	}
	I3MatrixPtr GetDataVector()
	{
		int j = 0;
		
		for (MillipedeDOMCacheMap::const_iterator i = domCache_.begin();
		    i != domCache_.end(); i++)
			j += i->second.valid.count();
		I3MatrixPtr matrix(new I3Matrix(j,1));
		
		j = 0;
		for (MillipedeDOMCacheMap::const_iterator i = domCache_.begin();
		    i != domCache_.end(); i++) {
			for (int k = 0; k < i->second.nbins; k++) {
				if (!i->second.valid[k])
					continue;
				matrix->operator()(j,0) = i->second.charges[k];
				j++;
			}
		}
		
		return matrix;
	}
	I3MatrixPtr GetNoiseVector()
	{
		int j = 0;
		
		for (MillipedeDOMCacheMap::const_iterator i = domCache_.begin();
		    i != domCache_.end(); i++)
			j += i->second.valid.count();
		I3MatrixPtr matrix(new I3Matrix(j,1));
		
		j = 0;
		for (MillipedeDOMCacheMap::const_iterator i = domCache_.begin();
		    i != domCache_.end(); i++) {
			for (int k = 0; k < i->second.nbins; k++) {
				if (!i->second.valid[k])
					continue;
				matrix->operator()(j,0) = i->second.noise[k];
				j++;
			}
		}
		
		return matrix;
	}
	void SetParameter(const std::string &key, bp::object value)
	{
		this->configuration_->Set(key, value);
	}
	bp::object GetParameter(const std::string &key)
	{
		bp::object val;
		I3MillipedeService::GetParameter(key, val);
		return val;
	}
	I3Configuration GetConfiguration()
	{
		return I3MillipedeService::GetConfiguration();
	}
};

void
register_PyPyMillipede()
{
	bp::class_<cholmod_sparse, boost::shared_ptr<cholmod_sparse>,
	    boost::noncopyable >("cholmod_sparse", bp::no_init)
	    .def("__init__", bp::make_constructor(from_I3Matrix))
	    .def("to_I3Matrix", &to_I3Matrix)
	;
	
	bp::implicitly_convertible<boost::shared_ptr<cholmod_sparse>,  boost::shared_ptr<const cholmod_sparse> >();
	
	bp::class_<PyPyMillipede, boost::shared_ptr<PyPyMillipede>, boost::noncopyable>("PyPyMillipede", bp::init<const I3Context&>())
	    .def("SetParameter", &PyPyMillipede::SetParameter)
	    .def("GetParameter", &PyPyMillipede::GetParameter)
	    .def("GetConfiguration", &PyPyMillipede::GetConfiguration)
	    .def("DatamapFromFrame", &PyPyMillipede::DatamapFromFrame)
	    .def("GetDataVector", &PyPyMillipede::GetDataVector)
	    .def("GetNoiseVector", &PyPyMillipede::GetNoiseVector)
	    .def("GetResponseMatrix", &PyPyMillipede::GetResponseMatrix,
	        (bp::arg("sources"), bp::arg("with_gradient")=false))
	    .def("SolveEnergyLosses", &PyPyMillipede::SolveEnergyLosses,
	        (bp::args("sources", "response_matrix"), bp::arg("gradients")=I3MatrixPtr()))
	    .def("FitStatistics", &PyPyMillipede::FitStatistics,
	        (bp::args("datamap", "sources", "response_matrix"), bp::arg("params")=MillipedeFitParamsPtr(), bp::arg("energy_epsilon")=0))
	    .add_property("domCache", bp::make_function(&PyPyMillipede::GetDOMCache, bp::return_internal_reference<>()))
	;
}

void
register_MillipedeDOMCache()
{
	bp::class_<array_proxy, boost::shared_ptr<array_proxy>, boost::noncopyable>("array_proxy", bp::no_init)
	    .add_property("__array_interface__", &array_proxy::array_interface)
	;
	
	bp::class_<MillipedeDOMCacheMap, boost::shared_ptr<MillipedeDOMCacheMap> >("MillipedeDOMCacheMap")
	    .def(bp::std_map_indexing_suite<MillipedeDOMCacheMap>())
	    .def("UpdateParams", &MillipedeDOMCacheMap::UpdateParams)
	    .def("UpdateData", &MillipedeDOMCacheMap::UpdateData)
	;
	
	bp::class_<MillipedeDOMCache, boost::shared_ptr<MillipedeDOMCache> >("MillipedeDOMCache")
	    .def_readwrite("geo", &MillipedeDOMCache::geo)
	    .def_readwrite("light_scale", &MillipedeDOMCache::light_scale)
	    .def_readwrite("noise_rate", &MillipedeDOMCache::noise_rate)
	    .def_readwrite("amplitudes_only", &MillipedeDOMCache::amplitudes_only)
	    .add_property("valid", &get_valid)
	    .def("_get_time_bin_edges", &get_edges)
	    .def("_get_charges", &get_charges)
	;
}

void
register_tableio_converters()
{
	I3CONVERTER_NAMESPACE(millipede);
	I3CONVERTER_EXPORT_DEFAULT(MillipedeFitParamsConverter,
	    "Dumps the fit statistics from Millipede reconstructions");
}

I3_PYTHON_MODULE(millipede) {
	load_project("millipede", false);
	bp::import("icecube.gulliver");

	register_MillipedeFitParams();
	register_MillipedeDOMCache();
	register_PyPyMillipede();
	register_tableio_converters();
}


