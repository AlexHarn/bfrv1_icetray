/**
 *
 * @brief implementation of the I3HalfSphereParametrization class
 *
 * (c) 2007
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3HalfSphereParametrization.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// Gulliver stuff
#include "lilliput/parametrization/I3HalfSphereParametrization.h"
#include "gulliver/I3EventHypothesis.h"

// icetray stuff
#include "icetray/I3Logging.h"
#include "icetray/I3SingleServiceFactory.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "phys-services/I3Calculator.h"
#include "gulliver/I3Gulliver.h"

I3HalfSphereParametrization::I3HalfSphereParametrization (
            const std::string &name,
            double ddir,
            double dxyz,
            double dLogE,
            double dt ):
        I3ServiceBase( name ),
        I3ParametrizationBase( I3EventHypothesisPtr( ) ),
        stepDir_(ddir),
        varLogE_(dLogE>0),
        stepLogE_(dLogE),
        varXYZ_(dxyz>0),
        stepXYZ_(dxyz),
        varTime_(dt>0),
        stepTime_(dt) {
    log_debug( "(%s) unit test CONSTRUCTOR making halfsphere parametrization",
               name.c_str() );
    InitializeFitSpecs();
}

I3HalfSphereParametrization::I3HalfSphereParametrization(
        const I3Context& context) : I3ServiceBase(context),
                                    I3ParametrizationBase( I3EventHypothesisPtr( ) ),
                                    stepDir_(0.3*I3Units::radian),
                                    stepLogE_(0),
                                    stepXYZ_(30*I3Units::m),
                                    stepTime_(0){

    log_debug( "(%s) CONSTRUCTOR making halfsphere parametrization",
               GetName().c_str() );


    AddParameter( "DirectionStepsize",
                  "Stepsize for direction, should be positive.",
                  stepDir_ );
    AddParameter( "LogEnergyStepsize",
                  "Stepsize for log10(E), should be >=0 "
                  "(if 0, then energy remains fixed).",
                  stepLogE_ );
    AddParameter( "VertexStepsize",
                  "Stepsize for (x,y,z), should be >=0 "
                  "(if 0, then the vertex position remains fixed).",
                  stepXYZ_ );
    AddParameter( "TimeStepsize",
                  "Stepsize for vertex time, should be >=0 "
                  "(if 0, then the vertex time remains fixed).",
                  stepTime_ );

}

void I3HalfSphereParametrization::Configure(){

    GetParameter( "DirectionStepsize", stepDir_ );
    GetParameter( "LogEnergyStepsize", stepLogE_ );
    GetParameter( "VertexStepsize", stepXYZ_ );
    GetParameter( "TimeStepsize", stepTime_ );

    varLogE_ = (stepLogE_>0);
    varXYZ_ = (stepXYZ_>0);
    varTime_ = (stepTime_>0);

    InitializeFitSpecs();
}

void I3HalfSphereParametrization::InitializeFitSpecs(){

    if ( stepDir_ > 0 ){
        log_debug( "(%s) direction stepsize %frad",
                   GetName().c_str(),
                   stepDir_/I3Units::radian );
    } else {
        log_debug( "(%s) direction stepsize must be positive, I got %frad. "
                   "The direction will be fixed to the seed value.",
                    GetName().c_str(),
                    stepDir_/I3Units::radian );
    }

    // fill vector of parameter specs, only for the variables which
    // are actually configured to be free (stepsize positive)

    I3FitParameterInitSpecs specs("");

    if (stepDir_ > 0) {
        specs.name_ = "dir1";
        specs.stepsize_ = stepDir_;
        specs.minval_   = 0.;
        specs.maxval_   = 0.;
        parspecs_.push_back(specs);
        specs.name_ = "dir2";
        parspecs_.push_back(specs);
    }

    if ( varLogE_ ){
        specs.name_ = "logE";
        specs.stepsize_ = stepLogE_;
        parspecs_.push_back(specs);
    }

    if ( varXYZ_ ){
        const char* xyz[3] = {"x","y","z"};
        for ( int i=0; i<3; ++i ){
            specs.name_ = xyz[i];
            specs.stepsize_ = stepXYZ_;
            parspecs_.push_back(specs);
        }
    }

    if ( varTime_ ){
        specs.name_ = "Time";
        specs.stepsize_ = stepTime_;
        parspecs_.push_back(specs);
    }

    // set parameter vector to correct size
    par_.resize( parspecs_.size() );
    log_debug( "(%s) got %zu fittable parameters",
              GetName().c_str(), par_.size() );
}

I3HalfSphereParametrization::~I3HalfSphereParametrization(){}

bool I3HalfSphereParametrization::InitChainRule(bool wantgradient)
{
    if (wantgradient) {
        gradient_ = I3EventHypothesisPtr(new I3EventHypothesis);
        // Zero the parameters we plan to use.
        gradient_->particle->SetPos(I3Position(0,0,0));
        gradient_->particle->SetDir(I3Direction(0,0));
        gradient_->particle->SetTime(0);
        gradient_->particle->SetEnergy(0);
        par_gradient_.resize(par_.size(),NAN);
        }
    return true;
}

/* calculate the values in par_gradient_, using the current gradient_ and hypothesis_. */
void I3HalfSphereParametrization::ApplyChainRule()
{
    I3Particle& simpletrack = *(hypothesis_->particle);
    I3Particle& gradient = *(gradient_->particle);
    const I3Direction& dir = gradient.GetDir();
    const I3Position& xyz = gradient.GetPos();

    double p0(0), p1(0);
    if (stepDir_ > 0) {
        assert(par_.size()>=2);
        p0 = par_[0];
        p1 = par_[1];
    }

    unsigned int ipar = 0;

    // Calculate matrix to transform p0', p1' into theta', phi'. Ugly.
    double newdirx = seedX_ + p0 * perp1X_ + p1 * perp2X_;
    double newdiry = seedY_ + p0 * perp1Y_ + p1 * perp2Y_;
    double newdirz = seedZ_ + p0 * perp1Z_ + p1 * perp2Z_;
    double r = sqrt(newdirx*newdirx + newdiry*newdiry + newdirz*newdirz);

    // dL/dp0 = dL/dtheta * dtheta/dp0 + dL/dphi * dphi/dp0
    if (stepDir_ > 0) {
    par_gradient_[ipar++] = dir.GetZenith()*(-newdirz*(newdirx*perp1X_ +
     newdiry*perp1Y_) + perp1Z_*(newdirx*newdirx + newdiry*newdiry))/
     (hypot(newdirx, newdiry)*r*r) +
    dir.GetAzimuth()*(perp1Y_*newdirx - perp1X_*newdiry)/
     (newdirx*newdirx + newdiry*newdiry);
    par_gradient_[ipar++] = dir.GetZenith()*(-newdirz*(newdirx*perp2X_ +
     newdiry*perp2Y_) + perp2Z_*(newdirx*newdirx + newdiry*newdiry))/
     (hypot(newdirx, newdiry)*r*r) +
    dir.GetAzimuth()*(perp2Y_*newdirx - perp2X_*newdiry)/
     (newdirx*newdirx + newdiry*newdiry);
    }

    if ( varXYZ_ ){
        assert(par_gradient_.size()>=ipar+3);
    par_gradient_[ipar++] = xyz.GetX();
    par_gradient_[ipar++] = xyz.GetY();
    par_gradient_[ipar++] = xyz.GetZ();
    }
    if ( varLogE_ ){
        assert(par_gradient_.size()>ipar);
    par_gradient_[ipar++] = gradient.GetEnergy()*simpletrack.GetEnergy();
    }
    if ( varTime_ ){
        assert(par_gradient_.size()>ipar);
    par_gradient_[ipar++] = gradient.GetTime();
    }

}

/// this should calculate datamembers of EmissionHypothesis from the values in par_
void I3HalfSphereParametrization::UpdatePhysicsVariables(){

    I3ParticlePtr simpletrack = hypothesis_->particle;

    unsigned int ipar = 0;
    if (stepDir_ > 0) {
        assert(par_.size()>=2);
        double p0 = par_[ipar++];
        double p1 = par_[ipar++];
        double newdirx = seedX_ + p0 * perp1X_ + p1 * perp2X_;
        double newdiry = seedY_ + p0 * perp1Y_ + p1 * perp2Y_;
        double newdirz = seedZ_ + p0 * perp1Z_ + p1 * perp2Z_;

        // no need to normalize, I3Direction will take care of that
        simpletrack->SetDir(newdirx,newdiry,newdirz);
        I3Gulliver::AnglesInRange((*simpletrack), GetName());
    }

    if ( varXYZ_ ){
        assert(par_.size()>=ipar+3);
        simpletrack->SetPos(par_[ipar],par_[ipar+1],par_[ipar+2]);
        ipar += 3;
    }
    if ( varLogE_ ){
        assert(par_.size()>ipar);
        simpletrack->SetEnergy( exp(par_[ipar]) );
        ipar += 1;
    }
    if ( varTime_ ){
        assert(par_.size()>ipar);
        simpletrack->SetTime( par_[ipar] );
    }

}

/**
 * This should calculate the values in par_ from datamembers of EmissionHypothesis
 * If relevant it should also update stepsizes.
 */
void I3HalfSphereParametrization::UpdateParameters(){
    I3ParticlePtr simpletrack = hypothesis_->particle;
    const I3Direction& dir = simpletrack->GetDir();

    unsigned int ipar = 0;
    // by definition
    if (stepDir_ > 0) {
        assert(par_.size()>=2);
        par_[ipar++] = 0.;
        par_[ipar++] = 0.;
    }

    // set reference directions
    seedX_ = dir.GetX();
    seedY_ = dir.GetY();
    seedZ_ = dir.GetZ();
    std::pair<I3Direction,I3Direction> sideways =
        I3Calculator::GetTransverseDirections(dir);
    const I3Direction &perp1 = sideways.first;
    const I3Direction &perp2 = sideways.second;
    perp1X_ = perp1.GetX();
    perp1Y_ = perp1.GetY();
    perp1Z_ = perp1.GetZ();
    perp2X_ = perp2.GetX();
    perp2Y_ = perp2.GetY();
    perp2Z_ = perp2.GetZ();

    // check orthnormality
    assert( fabs( perp1X_*perp2X_ + perp1Y_*perp2Y_ + perp1Z_*perp2Z_ ) < 0.01 );
    assert( fabs( perp1X_*perp1X_ + perp1Y_*perp1Y_ + perp1Z_*perp1Z_ - 1) < 0.01 );
    assert( fabs( perp2X_*perp2X_ + perp2Y_*perp2Y_ + perp2Z_*perp2Z_ - 1) < 0.01 );

    if ( varXYZ_ ){
        assert(par_.size()>ipar);
        const I3Position &pos = simpletrack->GetPos();
        par_[ipar++] = pos.GetX();
        par_[ipar++] = pos.GetY();
        par_[ipar++] = pos.GetZ();
    }
    if ( varLogE_ ){
        assert(par_.size()>ipar);
        double energy = simpletrack->GetEnergy();
        if ( ! ( std::isfinite(energy) && (energy>0) ) ){
            energy = 1.0*I3Units::TeV;
        }
        par_[ipar] = log(energy);
        ipar += 1;
    }
    if ( varTime_ ){
        assert(par_.size()>ipar);
        par_[ipar] = simpletrack->GetTime();
    }

}

typedef
I3SingleServiceFactory<I3HalfSphereParametrization,I3ParametrizationBase>
I3HalfSphereParametrizationFactory;
I3_SERVICE_FACTORY( I3HalfSphereParametrizationFactory );
