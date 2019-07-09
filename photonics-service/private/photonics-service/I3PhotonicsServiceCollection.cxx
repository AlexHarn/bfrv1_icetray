
#include <photonics-service/I3PhotonicsServiceCollection.h>

I3PhotonicsServiceCollection::I3PhotonicsServiceCollection(
    std::map<I3OMGeo::OMType, I3PhotonicsServicePtr> &services)
    : services_(services), current_(NULL)
{
  std::map<I3OMGeo::OMType, I3PhotonicsServicePtr>::iterator serviceIter;
  for(serviceIter=services.begin(); serviceIter!=services.end(); ++serviceIter)
    log_info_stream("Adding I3PhotonicsService for module type " << serviceIter->first);
}
         
I3PhotonicsServiceCollection::~I3PhotonicsServiceCollection() {};

void
I3PhotonicsServiceCollection::SelectModule(const I3OMGeo &geo)
{
    std::map<I3OMGeo::OMType, I3PhotonicsServicePtr>::const_iterator
        current = services_.find(geo.omtype);
    if (current == services_.end()) {
        log_fatal_stream("Unknown OM type " << geo.omtype);
    } else {
        current_ = current->second.get();
        current_->SelectModule(geo);
    }
}

namespace {

inline I3PhotonicsService* check(I3PhotonicsService *ptr)
{
    if (ptr == NULL)
        log_fatal("No module selected");
    return ptr;
}

}

void 
I3PhotonicsServiceCollection::SelectSource( double &meanPEs,
                           double &emissionPointDistance,
                           double &geoTime,
                           PhotonicsSource const &source)
{
    check(current_)->SelectSource(meanPEs, emissionPointDistance, geoTime, source);
}

void 
I3PhotonicsServiceCollection::SelectSource( double &meanPEs,
                           double gradients[6],
                           double &emissionPointDistance,
                           double &geoTime,
                           PhotonicsSource const &source,
                           bool getAmp)
{
    check(current_)->SelectSource(meanPEs, gradients, emissionPointDistance, geoTime, source, getAmp);
}

void 
I3PhotonicsServiceCollection::GetTimeDelay(double random, double &timeDelay)
{
    check(current_)->GetTimeDelay(random, timeDelay);
}

void 
I3PhotonicsServiceCollection::GetTimeDelays(I3RandomServicePtr random,
                           double *timeDelays, int n)
{
    check(current_)->GetTimeDelays(random, timeDelays, n);
}
void 
I3PhotonicsServiceCollection::GetProbabilityDensity(double &density, double timeDelay)
{
    check(current_)->GetProbabilityDensity(density, timeDelay);
}

bool 
I3PhotonicsServiceCollection::GetProbabilityQuantiles(double *time_edges,
                                     double t_0,
                                     double *amplitudes,
                                     size_t n_bins)
{
    return check(current_)->GetProbabilityQuantiles(time_edges, t_0, amplitudes, n_bins);
}

bool 
I3PhotonicsServiceCollection::GetProbabilityQuantiles(double *time_edges,
                                     double t_0,
                                     double *amplitudes,
                                     double gradients[][6],
                                     size_t n_bins)
{
    return check(current_)->GetProbabilityQuantiles(time_edges, t_0, amplitudes, gradients, n_bins);
}

bool 
I3PhotonicsServiceCollection::GetMeanAmplitudeGradient(double gradient[6])
{
    return check(current_)->GetMeanAmplitudeGradient(gradient);
}

bool 
I3PhotonicsServiceCollection::GetMeanAmplitudeHessian(double gradient[6], double hessian[6][6])
{
    return check(current_)->GetMeanAmplitudeHessian(gradient, hessian);
}

bool 
I3PhotonicsServiceCollection::GetProbabilityQuantileGradients(double *time_edges, double t_0,
    double gradients[][6], size_t n_bins)
{
    return check(current_)->GetProbabilityQuantileGradients(time_edges, t_0, gradients, n_bins);
}

bool 
I3PhotonicsServiceCollection::GetProbabilityQuantileHessians(double *time_edges,
    double t_0, double values[], double gradients[][6], double hessians[][6][6], size_t n_bins)
{
    return check(current_)->GetProbabilityQuantileHessians(time_edges, t_0, values, gradients, hessians, n_bins);
}
