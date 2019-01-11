#ifndef IPDF_IPdfService_H_INCLUDED
#define IPDF_IPdfService_H_INCLUDED

#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"

#include "icetray/I3ServiceFactory.h"
#include "icetray/I3Frame.h"
#include "dataclasses/physics/I3Particle.h"

#include <boost/shared_ptr.hpp>

/**
 * copyright  (C) 2006
 * the icecube collaboration
 * $Id$
 * 
 * @version $Revision: 1.5 $
 * @date $Date$
 * @author robbins <robbins@physik.uni-wuppertal.de>
 *
 * @brief Proposed interface to ipdf for I3
 *
 * @todo Are the Geometry and Physics methods called automatically?
   *	(maybe they should be in the factory?  See F2K stuff)
 * @todo Basic tools in place - now build the configurable "getLikelihood()"
 */
class IPdfService {
public:

  IPdfService();

  virtual ~IPdfService(){};

  void Geometry(const I3FramePtr frame);

  void Physics(const I3FramePtr frame);

  /// @brief Calculate the likelihood value for a given hypothesis
  ///
  /// The detector response is stored internally (set via "Physics()").
  /// The detector configuration is set via "Geometry()".
  template<typename EmissionHypothesis>
  double getLogLikelihood(const EmissionHypothesis&) const { /* todo */ return 0.; }

protected:
  /// @brief Internal implementation: get seed track from previous module
  /// 
  /// If we fail to find the seed track and we are doing simulation, then 
  /// the seed is taken from the MC truth.
  I3ParticleConstPtr retrieveSeed(const I3FramePtr&) const;

  /// @brief Internal implementation: ensure track is in principle fitable
  bool checkTrack(const I3ParticleConstPtr& track);

  /// @brief Internal implementation: install the detector response
  template<class Readout>
  void makeResponse(const Readout&);

private:

  // private copy constructors and assignment
  IPdfService(const IPdfService& );
  IPdfService operator=(const IPdfService& );

  // Source of the input data
  std::string inputDataReadout_;
  std::string inputSeedName_;

  boost::shared_ptr<IPDF::I3DetectorConfiguration> dgeom_;
  typedef IPDF::I3DetectorResponse DResponse;
  boost::shared_ptr<const DResponse> dresponse_;

  SET_LOGGER("ipdf");
};

I3_POINTER_TYPEDEFS(IPdfService);

class IPdfServiceFactory : public I3ServiceFactory {
public:
  IPdfServiceFactory(const I3Context& context) : I3ServiceFactory(context) {}
  bool InstallService(I3Context& c){
    boost::shared_ptr<IPdfService> ipdf(new IPdfService());
    c.Put(ipdf,"ipdf");
    return (bool)ipdf;
  }
};

#if 0
/// NOTE this syntax (for templated classes):
template <typename T>
class I3TemplateServiceFactory 
{ 
  // declaration
};

typedef I3TemplateServiceFactory<int> MyServiceFactoryOfInt;

I3_SERVICE_FACTORY(MyServiceFactoryOfInt);
#endif

#endif //IPDF_IPdfService_H_INCLUDED
