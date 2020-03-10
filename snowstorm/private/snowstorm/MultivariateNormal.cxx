
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#include <icetray/I3Logging.h>
#include <phys-services/I3RandomService.h>
#include "snowstorm/MultivariateNormal.h"

namespace {

struct set_gsl_error_handler {
    set_gsl_error_handler() : old_handler(gsl_set_error_handler_off()) {}
    ~set_gsl_error_handler()
    {
        gsl_set_error_handler(old_handler);
    }
    gsl_error_handler_t* old_handler;
};

}

namespace snowstorm {

std::vector<double>
MultivariateNormal::Sample(I3RandomService &rng) const
{
    // draw variates from a unit gaussian
    std::vector<double> x(covarianceL_.size1());
    for (auto &v : x)
        v = rng.Gaus(0,1);

    // transform to final space: x = Lv + mu
    {
        gsl_matrix_const_view L = gsl_matrix_const_view_array(&(covarianceL_.data()[0]), covarianceL_.size1(), covarianceL_.size2());
        gsl_vector_view xv = gsl_vector_view_array(x.data(), x.size());
        gsl_blas_dtrmv(CblasLower, CblasNoTrans, CblasNonUnit, &L.matrix, &xv.vector);
    }
    std::transform(mean_.begin(), mean_.end(), x.begin(), x.begin(), std::plus<double>());

    return x;
}

double
MultivariateNormal::LogPDF(const std::vector<double> &x) const
{
    i3_assert(x.size() == mean_.size());
    std::vector<double> work(x.size());
    std::transform(x.begin(), x.end(), mean_.begin(), work.begin(), std::minus<double>());

    double result = -std::numeric_limits<double>::infinity();
    {
        gsl_matrix_const_view L = gsl_matrix_const_view_array(&(covarianceL_.data()[0]), covarianceL_.size1(), covarianceL_.size2());
        gsl_vector_view workv = gsl_vector_view_array(work.data(), work.size());

        double mahalanobis;
        // compute L^-1 * (x-mu)
        gsl_blas_dtrsv(CblasLower, CblasNoTrans, CblasNonUnit, &L.matrix, &workv.vector);
        // compute (x-mu)' * L^-1 * (x-mu)
        gsl_blas_ddot(&workv.vector, &workv.vector, &mahalanobis);
        
        // compute log [ sqrt(|Sigma|) ] = sum_i log covarianceL_{ii}
        double norm = 0.5*covarianceL_.size1()*std::log(2.0*M_PI);
        for (unsigned i=0; i < covarianceL_.size1(); i++)
            norm += std::log(gsl_matrix_get(&L.matrix, i, i));

        result = -0.5*mahalanobis - norm;
    }

    return result;
}

template <class Archive>
void MultivariateNormal::serialize(Archive & ar, unsigned version)
{
    if (version > 0)
        log_fatal_stream("Version "<<version<<" is from the future");

    ar & make_nvp("snowstorm::Distribution", base_object<Distribution>(*this));
    ar & make_nvp("covarianceL", covarianceL_);
    ar & make_nvp("mean", mean_);
}

MultivariateNormal::MultivariateNormal(const I3Matrix &covariance, const std::vector<double> &mean) : covarianceL_(covariance), mean_(mean)
{
    i3_assert(covariance.size1() > 0 && "covariance is not empty");
    i3_assert(covariance.size1() == covariance.size2() && "covariance matrix is square");
    i3_assert(covariance.size1() == mean.size() && "mean is compatible with covariance");
    // compute Cholesky factorization. this will fail if the covariance is not
    // symmetric and positive definite.
    gsl_matrix_view m = gsl_matrix_view_array(&(covarianceL_.data()[0]), covarianceL_.size1(), covarianceL_.size2());
    set_gsl_error_handler muzzle;
    if (gsl_linalg_cholesky_decomp(&m.matrix) != GSL_SUCCESS) {
        covarianceL_.clear();
        log_fatal_stream("Decomposition failed; is the covariance really positive definite?");
    }
}

MultivariateNormal::MultivariateNormal() {}
MultivariateNormal::~MultivariateNormal() {}

}

I3_SERIALIZABLE(snowstorm::MultivariateNormal);
