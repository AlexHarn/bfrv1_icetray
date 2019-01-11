/**
 * @file I3DoubleMuonParametrization.cxx
 * @brief implementation of the I3DoubleMuonParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <icetray/I3SingleServiceFactory.h>
#include <gulliver/I3Gulliver.h>
#include <double-muon/I3DoubleMuonParametrization.h>
#include <cmath>

static const std::string vstep_optionname = "VertexStepsize";
static const std::string astep_optionname = "AngleStepsize";

const double I3DoubleMuonParametrization::defaultVertexStepsize_ = 20*I3Units::ns;
const double I3DoubleMuonParametrization::defaultAngleStepsize_ = 0.1*I3Units::radian;

I3DoubleMuonParametrization::I3DoubleMuonParametrization(
                                     std::string name,
                                     double vstep,
                                     double astep ):
    I3ServiceBase(name),
    I3ParametrizationBase(I3EventHypothesisPtr() ),
    vertexStepsize_(vstep),
    angleStepsize_(astep){}

/// constructor I3Tray
I3DoubleMuonParametrization::I3DoubleMuonParametrization(const I3Context &c):
    I3ServiceBase(c),
    I3ParametrizationBase(I3EventHypothesisPtr()),
    vertexStepsize_(defaultVertexStepsize_),
    angleStepsize_(defaultAngleStepsize_){

    AddParameter( vstep_optionname,
                  "Stepsize for the vertices",
                  vertexStepsize_ );

    AddParameter( astep_optionname,
                  "Stepsize for the angles",
                  angleStepsize_ );

}

void I3DoubleMuonParametrization::Configure(){
    GetParameter( vstep_optionname, vertexStepsize_ );
    GetParameter( astep_optionname, angleStepsize_ );

    if ( vertexStepsize_ <= 0 ){
        log_fatal( "(%s) vertex stepsize should be positive, I got %g m",
                   GetName().c_str(), vertexStepsize_/I3Units::m );
    }
    if ( angleStepsize_ <= 0 ){
        log_fatal( "(%s) angle stepsize should be positive, I got %g radians",
                   GetName().c_str(), angleStepsize_/I3Units::rad );
    }

    // regular xyzza parameters, fix them
    const std::string xyzzanames[5] = {"X","Y","Z","Zenith","Azimuth"};
    const double xyzzasteps[5] = {
        20*I3Units::m, 20*I3Units::m, 20*I3Units::m,
        0.1*I3Units::rad, 0.2*I3Units::rad };
    // do bounds...
    // constrain vertex position to +/- 2km per coordinate
    // constrain angles to avoid running to infinity
    // (Constraining to 0-pi and 0-2pi radians would be bad, because
    //  the minimizer does not know that angles are cyclic. Letting
    //  them completely free sometimes results in angles so large that
    //  2pi is less than the floating point precision.)

    const double xyzzamin[5] = { -2000.,-2000.,-2000.,-10000.,-20000.};
    const double xyzzamax[5] = { +2000.,+2000.,+2000.,+10000.,+20000.};

    for ( int i=0; i<5; ++i ){
        I3FitParameterInitSpecs specs(xyzzanames[i]+"1");
        specs.stepsize_ = xyzzasteps[i];
        specs.minval_ = xyzzamin[i];
        specs.maxval_ = xyzzamax[i];
        parspecs_.push_back(specs);
    }
    for ( int i=0; i<5; ++i ){
        I3FitParameterInitSpecs specs(xyzzanames[i]+"2");
        specs.stepsize_ = xyzzasteps[i];
        specs.minval_ = xyzzamin[i];
        specs.maxval_ = xyzzamax[i];
        parspecs_.push_back(specs);
    }

    par_.resize(parspecs_.size(),NAN);
}

/// compute event hypothesis from minimizer parameters
void I3DoubleMuonParametrization::UpdatePhysicsVariables(){
    I3ParticlePtr muon1 = hypothesis_->particle;
    I3ParticlePtr muon2 =
        boost::dynamic_pointer_cast<I3Particle>(hypothesis_->nonstd);
    assert(muon2);

    muon1->SetPos( par_[0], par_[1], par_[2] );
    muon1->SetDir( par_[3], par_[4] );
    muon2->SetPos( par_[5], par_[6], par_[7] );
    muon2->SetDir( par_[8], par_[9] );

    I3Gulliver::AnglesInRange(*muon1,GetName());
    I3Gulliver::AnglesInRange(*muon2,GetName());

    log_trace( "(%s) update vars: p0=%g p1=%g p2=%g p3=%g p4=%g",
               GetName().c_str(),
               par_[0], par_[1], par_[2], par_[3], par_[4] );
    log_trace( "(%s) update vars: p5=%g p6=%g p7=%g p8=%g p9=%g",
               GetName().c_str(),
               par_[5], par_[6], par_[7], par_[8], par_[9] );
}

/// compute minimizer parameters from event hypothesis
void I3DoubleMuonParametrization::UpdateParameters(){
    const I3ParticlePtr muon1 = hypothesis_->particle;
    const I3Position& pos1 = muon1->GetPos();
    const I3Direction& dir1 = muon1->GetDir();
    I3ParticlePtr muon2 =
        boost::dynamic_pointer_cast<I3Particle>(hypothesis_->nonstd);
    if (!muon2){
        log_fatal( "(%s) incomplete/wrong event hypothesis; wrong seed service?",
                   GetName().c_str() );
    }
    const I3Position& pos2 = muon2->GetPos();
    const I3Direction& dir2 = muon2->GetDir();

    par_[0] = pos1.GetX();
    par_[1] = pos1.GetY();
    par_[2] = pos1.GetZ();
    par_[3] = dir1.GetZenith();
    par_[4] = dir1.GetAzimuth();
    par_[5] = pos2.GetX();
    par_[6] = pos2.GetY();
    par_[7] = pos2.GetZ();
    par_[8] = dir2.GetZenith();
    par_[9] = dir2.GetAzimuth();

}

typedef
I3SingleServiceFactory<I3DoubleMuonParametrization,I3ParametrizationBase>
I3DoubleMuonParametrizationServiceFactory;
I3_SERVICE_FACTORY( I3DoubleMuonParametrizationServiceFactory );
