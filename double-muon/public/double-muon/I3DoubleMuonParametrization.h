#ifndef I3DOUBLEMUONPARAMETRIZATION_H_INCLUDED
#define I3DOUBLEMUONPARAMETRIZATION_H_INCLUDED

/**
 *
 * @file I3DoubleMuonParametrization.h
 * @brief declaration of the I3DoubleMuonParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <string>
#include <icetray/I3Units.h>
#include <icetray/I3ServiceBase.h>
#include <gulliver/I3ParametrizationBase.h>

/**
 * @class I3DoubleMuonParametrization
 * @brief define fittable variables and their parametrization for double muon events
 *
 * For now: simple standard xyzza parametrization for both muons.
 *
 * Later: fancy stuff to prevent the two tracks from coinciding.
 */
class I3DoubleMuonParametrization : public I3ServiceBase,
                                    public I3ParametrizationBase {
public:

    /// default constructor for unit tests
    I3DoubleMuonParametrization( std::string name="unittest",
                                 double vertexstepsize=20.*I3Units::m,
                                 double anglestepsize=0.2*I3Units::degree );

    /// constructor I3Tray
    I3DoubleMuonParametrization(const I3Context &c);

    /// set parameters (in I3Tray)
    void Configure();

    /// compute event hypothesis from minimizer parameters
    void UpdatePhysicsVariables();

    /// compute minimizer parameters from event hypothesis
    void UpdateParameters();

    /// tell your name
    const std::string GetName() const {
        return this->I3ServiceBase::GetName();
    }

    /// default stepsize for x, y, z
    static const double defaultVertexStepsize_;

    /// default stepsize for zenith (for azimuth it's twice this value)
    static const double defaultAngleStepsize_;

private:

    /// stepsize for x, y, z
    double vertexStepsize_;

    /// stepsize for zenith (for azimuth it's twice this value)
    double angleStepsize_;

    /// log4cplus thingy
    SET_LOGGER( "I3DoubleMuonParametrization" );
};

#endif /* I3DOUBLEMUONPARAMETRIZATION_H_INCLUDED */
