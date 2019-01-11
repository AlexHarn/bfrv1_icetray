#ifndef GULLIVER_I3TESTDUMMYTRACKPARAMETRIZATION_H_INCLUDED
#define GULLIVER_I3TESTDUMMYTRACKPARAMETRIZATION_H_INCLUDED
#include <cassert>
#include <cmath>
#include <iostream>
#include "gulliver/I3ParametrizationBase.h"
#include "icetray/I3Units.h"
#include <cfloat>
// #include "gulliver/test/I3TestDummyTrack.h"


//
// a Cartesian parametrization of our Dummy track
// this is a very contrived example, got nothing to do with any physics...
// this only exists in order to define unit tests, independent of any
// other project.
//
// This is a parametrization of x, length and azimuth.
// Length and azimuth are converted to two cartesian coordinates,
// x is trivially converted.
//
class I3TestDummyTrackParametrization : public I3ParametrizationBase {
public:

    // construct
    // rsf = rho step fraction: stepsize will be a fraction of the seed length
    // ps = phi step (absolute value)
    // xs = x step
    I3TestDummyTrackParametrization( I3EventHypothesisPtr dtp, double rsf, double ps, double xs=10.*I3Units::m ):
    I3ParametrizationBase(dtp),rho_step_fraction_(rsf), phi_step_(ps),x_step_(xs), name_("DummyRho2Phi"){

        // check config
        assert( ( rho_step_fraction_ > 0 ) && ( rho_step_fraction_ < 1 ) );
        assert( ( phi_step_ > 0 ) && ( phi_step_ < 2*M_PI ) );

        // init
        std::string dummy("dummy");
        parspecs_.resize(3,dummy);
        par_.resize(3);
        UpdateParameters();
        parspecs_[0].initval_ = par_[0];
        parspecs_[1].initval_ = par_[1];
        parspecs_[2].initval_ = par_[2];

        // stepsize
        double rho = hypothesis_->particle->GetLength();
        double azi = hypothesis_->particle->GetDir().GetAzimuth();
        // double x = hypothesis_->particle->GetPos().GetX();
        double drho = parspecs_[0].stepsize_ = rho_step_fraction_ * rho;
        double dphi = phi_step_;
        double drcf = drho * cos( azi );
        double rsdf = rho * sin( azi ) * dphi;
        double drsf = drho * sin( azi );
        double rcdf = rho * cos( azi ) * dphi;
        parspecs_[0].stepsize_ = sqrt( rsdf*rsdf + drcf * drcf );
        parspecs_[1].stepsize_ = sqrt( rcdf*rcdf + drsf * drsf );
        parspecs_[2].stepsize_ = x_step_;

    }

    virtual bool InitChainRule(bool wantgradient){
        if (!gradient_){
            gradient_ = I3EventHypothesisPtr(new I3EventHypothesis);
        }
        par_gradient_.clear();
        par_gradient_.resize( par_.size(), 0. );
        if ( wantgradient ){
            I3Particle &g = *(gradient_->particle);
            g.SetPos(1.,0.,0.); // need d(LogL)/d(x)
            g.SetDir(0.,1.); // need d(LogL)/d(azimuth)
            g.SetEnergy(0.);
            g.SetLength(1.); // need d(LogL)/d(length)
            g.SetTime(0.);
        }
        return true;
    }


    virtual void ApplyChainRule(){

        I3Particle &g = *(gradient_->particle);
        I3Particle &p = *(hypothesis_->particle);
        double dLogL_dlength = g.GetLength();
        double dLogL_dazi = g.GetDir().GetAzimuth();
        double dLogL_dx = g.GetPos().GetX();

        double length = p.GetLength();
        // double azi = p.GetDir().GetAzimuth();

        double dazi_dpar0, dazi_dpar1;
        double dlength_dpar0, dlength_dpar1;

        if ( length <= 0. ){
            // singularity
            dlength_dpar0 = 1.;
            dlength_dpar1 = 1.;
            dazi_dpar0 = DBL_MAX;
            dazi_dpar1 = DBL_MAX;
        } else {
            dlength_dpar0 = par_[0]/length;
            dlength_dpar1 = par_[1]/length;
            if ( fabs( par_[0] ) > fabs( par_[1] ) ){
                // atan2 = arctan(par1/par0) = arctan(r)
                // datan2/dr = 1/(1+r^2)
                double r = par_[1]/par_[0];
                double datan2dr = 1.0/(1.0+r*r);
                dazi_dpar0 = -datan2dr*par_[1]/(par_[0]*par_[0]);
                dazi_dpar1 = datan2dr/par_[0];
            } else {
                // atan2 = pi/2 - arctan(par0/par1) = pi/2 - arctan(s)
                // datan2/ds = 1/(1+s^2)
                double s = par_[0]/par_[1];
                double datan2ds = -1.0/(1.0+s*s);
                dazi_dpar0 = datan2ds/par_[1];
                dazi_dpar1 = -datan2ds*par_[0]/(par_[1]*par_[1]);
            }
        }

        assert(par_gradient_.size() == 3);
        par_gradient_[0] = dLogL_dlength * dlength_dpar0 + dLogL_dazi * dazi_dpar0;
        par_gradient_[1] = dLogL_dlength * dlength_dpar1 + dLogL_dazi * dazi_dpar1;
        par_gradient_[2] = dLogL_dx;

        /*
        log_trace("CHECK\n:dLogL/dL=%g dLogL/dA=%g dLogL/dX=%g\n"
                 "par=(%g,%g,%g)\ngrad=(%g,%g,%g)",
                 dLogL_dlength, dLogL_dlength, dLogL_dx,
                par_[0], par_[1], par_[2],
                par_gradient_[0], par_gradient_[1], par_gradient_[2]);
        */
    }

    // destruct
    ~I3TestDummyTrackParametrization(){}

    void UpdatePhysicsVariables(){
        // temporaries necessary for braindead I3Particle interface
        I3Particle &p = *(hypothesis_->particle);
        double zenith = p.GetDir().GetZenith();
        double y = p.GetPos().GetY();
        double z = p.GetPos().GetZ();
        p.SetLength( sqrt( par_[0]*par_[0] + par_[1]*par_[1] ) );
        p.SetDir( zenith, atan2( par_[1], par_[0] ) );
        p.SetPos( par_[2], y, z );
        log_trace( "setting L=%g azimuth=%g X=%g",
                   p.GetLength(),
                   p.GetDir().GetAzimuth(),
                   p.GetPos().GetX() );
    }

    void UpdateParameters(){

        I3Particle &p = *(hypothesis_->particle);
        double l = p.GetLength();
        double azi = p.GetDir().GetAzimuth();
        double x = p.GetPos().GetX();

        if ( l <= 0 ){
            par_[0] = par_[1] = 0;
        } else {
            par_[0] = l * cos( azi );
            par_[1] = l * sin( azi );
        }
        par_[2] = x;


    }

    const std::string GetName() const {
        return name_;
    }

private:

    // prohibit default constructor
    I3TestDummyTrackParametrization();

    double rho_step_fraction_;
    double phi_step_;
    double x_step_;
    std::string name_;

};
typedef boost::shared_ptr<I3TestDummyTrackParametrization> I3TestDummyTrackParametrizationPtr;

#endif /* GULLIVER_I3TESTDUMMYTRACKPARAMETRIZATION_H_INCLUDED */
