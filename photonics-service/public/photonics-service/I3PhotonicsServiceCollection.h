
#ifndef I3PHOTONICSSERVICE_COLLECTION_H
#define I3PHOTONICSSERVICE_COLLECTION_H
/**
 *@file
 *@brief I3PhotonicsServiceCollection
 *       A collection of I3PhotonicsService instances, dispatched to by OMType
 *
 *@author Jakob van Santen
 *(c) the IceCube Collaboration
 */


#include "photonics-service/I3PhotonicsService.h"
#include "dataclasses/geometry/I3OMGeo.h"
#include <icetray/I3ServiceBase.h>

class I3PhotonicsServiceCollection : public I3PhotonicsService {
    public:
        I3PhotonicsServiceCollection(std::map<I3OMGeo::OMType, I3PhotonicsServicePtr> &services);
        virtual ~I3PhotonicsServiceCollection();

        virtual void SelectModule(const I3OMGeo &);

        virtual void SelectSource( double &meanPEs,
                                   double &emissionPointDistance,
                                   double &geoTime,
                                   PhotonicsSource const &source);

        virtual void SelectSource( double &meanPEs,
                                   double gradients[6],
                                   double &emissionPointDistance,
                                   double &geoTime,
                                   PhotonicsSource const &source,
                                   bool getAmp=true);

        virtual void GetTimeDelay(double random, double &timeDelay);

        virtual void GetTimeDelays(I3RandomServicePtr random,
                                   double *timeDelays, int n);
        
        virtual void GetProbabilityDensity(double &density, double timeDelay);

        virtual bool GetProbabilityQuantiles(double *time_edges,
                                             double t_0,
                                             double *amplitudes,
                                             size_t n_bins);

        virtual bool GetProbabilityQuantiles(double *time_edges,
                                             double t_0,
                                             double *amplitudes,
                                             double gradients[][6],
                                             size_t n_bins);

        virtual bool GetMeanAmplitudeGradient(double gradient[6]);
        
        virtual bool GetMeanAmplitudeHessian(double gradient[6], double hessian[6][6]);

        virtual bool GetProbabilityQuantileGradients(double *time_edges, double t_0,
            double gradients[][6], size_t n_bins);

        virtual bool GetProbabilityQuantileHessians(double *time_edges,
            double t_0, double values[], double gradients[][6], double hessians[][6][6], size_t n_bins);

    private:
        std::map<I3OMGeo::OMType, I3PhotonicsServicePtr> services_;
        I3PhotonicsService *current_;
};

I3_POINTER_TYPEDEFS(I3PhotonicsServiceCollection);


#endif // #ifdef I3PHOTONICSSERVICE_COLLECTION_H
