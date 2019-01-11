/**
 * @file I3DoubleMuonLogLikelihood.h
 * @brief declaration of the I3DoubleMuonLogLikelihood class
 *
 * (c) 2005 * the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#ifndef I3DOUBLEMUONLIKELIHOOD_H_INCLUDED
#define I3DOUBLEMUONLIKELIHOOD_H_INCLUDED

// standard library stuff
#include <string>

// framework stuff
#include <icetray/IcetrayFwd.h>
#include <icetray/I3ServiceBase.h>

// Gulliver stuff
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3EventLogLikelihoodBase.h"

// ipdf stuff
#ifndef IPDF_I3MODULES
#define IPDF_I3MODULES 1
#endif
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Pandel/GConvolutePEP.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Pandel/HitProbability.h"

/**
 * @class I3DoubleMuonLogLikelihood
 * @brief likelihood for double muon event
 * 
 * @sa I3EventLogLikelihoodBase
 */
class I3DoubleMuonLogLikelihood : public I3EventLogLikelihoodBase,
                                  public I3ServiceBase {
private:
    std::string pulseMapName_;
    std::string zenithWeightName_;
    double noiseRate_;
    double logNoiseRate_;
    bool do_mpe_;
    unsigned int multiplicity_;

    I3EventLogLikelihoodBasePtr zenithWeight_;
    boost::shared_ptr<IPDF::I3DetectorConfiguration> ipdfGeometry_;
    boost::shared_ptr<IPDF::I3DetectorResponse> ipdfResponseMap_;

    IPDF::Pandel::GConvolutePEP<IPDF::Pandel::H2> pandelGCPDFH2_;
    IPDF::Likelihood::MPE<IPDF::Pandel::GaussConvolutedPEP<IPDF::Pandel::H2> > mpeGCPDFH2_;

public:

    /// constructor for unit tests
    I3DoubleMuonLogLikelihood( std::string name="unittest",
                               std::string pulses="testpulses",
                               std::string zenithw8="",
                               double noiserate = 700*I3Units::hertz,
                               bool mpe = false );

    /// constructor for I3Tray
    I3DoubleMuonLogLikelihood( const I3Context &context );

    /// cleanup
    virtual ~I3DoubleMuonLogLikelihood(){}

    /// get configuration parameters
    virtual void Configure();

    /// provide event data
    void SetGeometry( const I3Geometry &geo );

    /// provide event data
    void SetEvent( const I3Frame &f );

    /// Get +log(likelihood) for a particular emission hypothesis
    double GetLogLikelihood( const I3EventHypothesis &t );

    /// the zenith weight function does not use event data.
    unsigned int GetMultiplicity(){ return multiplicity_; }

    /// tell your name
    const std::string GetName() const {
        return I3ServiceBase::GetName();
    }

    SET_LOGGER( "I3DoubleMuonLogLikelihood" );

};

#endif /* I3DOUBLEMUONLIKELIHOOD_H_INCLUDED */
