/**
 * @file I3HalfSphereParametrization.h
 * @brief declaration of the I3HalfSphereParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author David Boersma <boersma@icecube.wisc.edu>
 */

#ifndef I3HALFSPHEREPARAMETRIZATION_H_INCLUDED
#define I3HALFSPHEREPARAMETRIZATION_H_INCLUDED

// standard library stuff
#include <string>
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceBase.h"

// Gulliver stuff
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3FitParameterInitSpecs.h"

/**
 * @class I3HalfSphereParametrization
 * @brief A parametrization which restricts the direction phase space
 *        to a the hemisphere centered around the seed track direction.
 */
class I3HalfSphereParametrization : public I3ServiceBase,
                                    public I3ParametrizationBase {
public:

    /// constructor with full initialization, for unit tests
    I3HalfSphereParametrization ( const std::string &name,
                                  double ddir,
                                  double dxyz,
                                  double dLogE,
                                  double dt );

    /// constructor for use in icetray scripts
    I3HalfSphereParametrization( const I3Context& context);

    /// destructor
    virtual ~I3HalfSphereParametrization();

    /// configure
    void Configure();

    bool InitChainRule(bool wantgradient);
    void ApplyChainRule();

    /// this should calculate datamembers of the I3Particle from the values in par
    void UpdatePhysicsVariables();

    /**
     * This should calculate the values in par_ from datamembers of the I3Particle
     * If relevant it should also update stepsizes.
     */
    void UpdateParameters();

    /// name to use for log_messages
    const std::string GetName() const { return I3ServiceBase::GetName(); }

    /// macro which sets the name to use to configure log4cplus.conf
    SET_LOGGER( "I3HalfSphereParametrization" );

private:

    /// inhibit the default constructor
    I3HalfSphereParametrization();

    /// inhibit the copy constructor
    I3HalfSphereParametrization( const I3HalfSphereParametrization& );

    /// inhibit the assignment operator
    I3HalfSphereParametrization operator= (const I3HalfSphereParametrization& rhs);

    /// initialize the FitParameterInitSpecs and parameter vectors
    void InitializeFitSpecs();

    /// seed direction cosines
    double seedX_;
    double seedY_;
    double seedZ_;

    /// direction cosines of two directons perpendicular to seed direction
    double perp1X_;
    double perp1Y_;
    double perp1Z_;
    double perp2X_;
    double perp2Y_;
    double perp2Z_;

    /// direction stepsize
    double stepDir_;

    /// whether to parametrize energy...
    bool varLogE_;

    /// ...and with what stepsize
    double stepLogE_;

    /// whether to parametrize the vertex...
    bool varXYZ_;

    /// ...and with what stepsize
    double stepXYZ_;

    /// whether to parametrize the time...
    bool varTime_;

    /// ...and with what stepsize
    double stepTime_;

};

I3_POINTER_TYPEDEFS( I3HalfSphereParametrization );

#endif /* I3HALFSPHEREPARAMETRIZATION_H_INCLUDED */
