/**
 * @brief declaration of the I3GulliverFinitePhPnh class
 *
 * @file I3GulliverFinitePhPnh.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class is implemented as an I3PDFService to be used with gulliver. It calculates the probability for a given track to fit to the hit pattern. The probability for each DOM to be hit or not hit is taken into account. These probabilities can be obtained in different ways, all deriving from PhPnhProbBase. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef LILLIPUT_I3GULLIVERFINITEPHPNH_H_INCLUDED
#define LILLIPUT_I3GULLIVERFINITEPHPNH_H_INCLUDED

#include <string>
#include <vector>

#include "finiteReco/probability/PhPnhProbBase.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3PDFBase.h"


class I3GulliverFinitePhPnh : public I3EventLogLikelihoodBase {
public:
    I3GulliverFinitePhPnh( const std::string &name,
                           const std::string &inputreadout,
                           const double& noiserate,
                           const double& defaultEventDuration,
                           const double& rCylinder,
                           const bool& FlagStringLLH,
                           const std::vector<int> selectedStrings,
                           const PhPnhProbBasePtr prob,
                           const bool& useOnlyFirstHit,
                           const double& probMultiDet = 0.5);
    ~I3GulliverFinitePhPnh();
    void SetGeometry( const I3Geometry &geo ) {};
    void SetEvent( const I3Frame &f );
    unsigned int GetMultiplicity();
    double GetLogLikelihood( const I3EventHypothesis &p );
    const std::string GetName() const { return llhName_; }
    
    /// Internal struct holding simple hits
    struct I3MHit {
      /// number of hits
      int n;
      /// hit time
      double t;
      /// hit amplitude
      double a;
      /// geometrical hit position
      I3Position pos;
    };
private:
    bool IsStringSelected(const int& stringNr);
    std::map<OMKey,I3MHit> hits_;
    std::string name_;
    std::string inputReadout_;

    std::string llhName_; 
    double noiseRate_;
    double defaultEventDuration_;
    double rCylinder_;
    bool flagStringLLH_;
    std::vector<int> selectedStrings_;
    PhPnhProbBasePtr prob_;

    int  multi_;
    bool useOnlyFirstHit_;
    double probMultiDet_;
    double minProb_;
    I3GulliverFinitePhPnh();
    double GetProbCylinder( const I3Particle& track, const I3Position& pos, const int& Nhit);
    double GetLogLikelihoodString( const I3Particle& track );
    double GetLogLikelihoodOM( const I3Particle& track );
    I3GulliverFinitePhPnh(const I3GulliverFinitePhPnh&);
    I3GulliverFinitePhPnh operator= (const I3GulliverFinitePhPnh& rhs);
};

I3_POINTER_TYPEDEFS( I3GulliverFinitePhPnh );

#endif /* LILLIPUT_I3GULLIVERFINITEPHPNH_H_INCLUDED */
