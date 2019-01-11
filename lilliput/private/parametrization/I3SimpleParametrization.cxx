/**
 *
 * @brief implementation of the I3SimpleParametrization class
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3SimpleParametrization.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

// icetray stuff
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "icetray/I3SingleServiceFactory.h"
#include "icetray/I3Logging.h"

// Gulliver stuff
#include "lilliput/parametrization/I3SimpleParametrization.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3Gulliver.h"

static const std::vector<double> nobounds(2,0.);
const std::vector< double > I3SimpleParametrization::no_steps_(I3SimpleParametrization::PAR_N,0.);
const std::vector< std::vector<double> > I3SimpleParametrization::all_nobounds_(I3SimpleParametrization::PAR_N,nobounds);

using std::endl;

const std::string
I3SimpleParametrization::parNames_[I3SimpleParametrization::PAR_N] = {
    "T",
    "X",
    "Y",
    "Z",
    "Zenith",
    "Azimuth",
    "LinE",
    "LogE",
    "LinL",
    "LogL",
    "Speed",
};

void I3SimpleParametrization::FitParameterConfig(
        simplepar_t ipar, std::string description,
        double eglim, double egval, std::string unitname ){
    std::string parname = parNames_[ipar];
    std::string stepname = std::string("Step") + parname;
    std::string absname = std::string("Bounds") + parname;
    std::string relname = std::string("RelativeBounds") + parname;
    std::string stepdescr = std::string("Stepsize for ") + description;
    std::ostringstream absdescr;
    std::string unitstr;
    if ( !unitname.empty() ){
        unitstr = "*I3Units.";
        unitstr += unitname;
    }
    absdescr << "Lower&upper (absolute) limits for the " << description << endl
             << "Leaving this empty or setting lower=upper means: no bounds" << endl
             << "(unless you specify relative bounds)" << endl
             << "See also: RelativeBounds" + parname;
    std::ostringstream reldescr;
    reldescr << "Lower&upper (relative) limits for " << description
             << ", relative to the seed value." << endl
             << "Leaving this empty or setting lower=upper means: no bounds" << endl
             << "(unless you specify absolute bounds, "
             << "see Bounds" << parname << ")" << endl
             << "The lower/upper limit should be negative/positive, respectively." << endl
             << "For instance, if you specify [" << (-eglim) << unitstr
             << ",+" << eglim << unitstr << "] " << "here," << endl
             << "and for some event the seed value is "
             << egval << unitstr << ", " << endl
             << "then the fit boundaries will be "
             << "[" << (egval-eglim) << unitstr << ","
             << (egval+eglim) << unitstr << "],"  << endl
             << "If you specify both absolute and relative boundaries then the" << endl
             << "the overlap of relative and absolute bounds will be used;" << endl
             << "if for a particular event this yields lower and upper limits" << endl
             << "that are less than two stepsizes apart then the relative" << endl
             << "boundaries will be stretched (only for that event) to force" << endl
             << "a range of at least two stepsizes.";

    AddParameter( stepname, stepdescr, steps_[ipar] );
    AddParameter( absname, absdescr.str(), absBounds_[ipar] );
    AddParameter( relname, reldescr.str(), relBounds_[ipar] );
}


I3SimpleParametrization::I3SimpleParametrization(
            const std::string name,
            const std::vector<double> &steps,
            const std::vector< std::vector<double> > &absbounds,
            const std::vector< std::vector<double> > &relbounds,
            int vertexmode,bool trace):
        I3ServiceBase(name),
        I3ParametrizationBase( I3EventHypothesisPtr( ) ),
        vertexMode_(vertexmode),
        steps_(steps),
        // absBounds_(all_nobounds),
        // relBounds_(all_nobounds),
        absBounds_(absbounds),
        relBounds_(relbounds),
        wantTrace_(trace) {
    log_debug( "(%s) CONSTRUCTOR making simple track",GetName().c_str() );


    if ( steps_.size() != PAR_N ){
        log_fatal( "(%s) PROGRAMMING ERROR: steps vector has wrong length "
                   "(%zu, should be %d)", GetName().c_str(), steps_.size(), PAR_N );
    }
    if ( ( absBounds_.size() != PAR_N ) || ( relBounds_.size() != PAR_N ) ){
        log_fatal( "(%s) PROGRAMMING ERROR: abs/rel vector of bounds has wrong length "
                   "(%zu/%zu, should be %d)", GetName().c_str(), 
                   absBounds_.size(),relBounds_.size(), PAR_N );
    }

    InitializeFitParSpecs();

    assert(parspecs_.size() == free_.count());

    log_debug( "(%s) CONSTRUCTOR made simple track parametrization with "
               "%zu free parameters",
               GetName().c_str(), parspecs_.size() );
}

I3SimpleParametrization::I3SimpleParametrization(
        const I3Context& context) : I3ServiceBase(context),
                                    I3ParametrizationBase( I3EventHypothesisPtr( ) ),
                                    vertexMode_( VERTEX_Default ),
                                    steps_( no_steps_ ),
                                    absBounds_( all_nobounds_ ),
                                    relBounds_( all_nobounds_ ),
                                    wantTrace_(false) {

    // for contained tracks: vertex=start or vertex=stop?
    AddParameter( "VertexMode",
                  "For contained tracks, the xyz parameters are by default\n"
                  "associated with the starting point position, but in some\n"
                  "cases you might want to prefer that it is the stopping\n"
                  "position. One use case is is to fit the length (and/or\n"
                  "direction) with a fixed stop point (rather than a fixed\n"
                  "start point). In that case you would configure \"Stop\"\n"
                  "here, and choose the xyz stepsizes zero and the stepsize\n"
                  "of length (and/or direction) nonzero.  Then the start\n"
                  "point is computed from the stop point, the length and the\n"
                  "direction, while the stop point remains the same as the\n"
                  "stop point of the seed.\n"
                  "To select the start point, or for non-ContainedTrack\n"
                  "fits, use the default (empty string or \"Default\").",
                  vertexModeString_ );

    AddParameter( "ParticleTrace",
                  "If enabled, store the seed particle and the list of all I3Particle objects "
                  "created during a fit (converted from parameter values generated by the "
                  "minimizer) and make this available as 'Diagnostics' information. The "
                  "generic fitter modules I3SimpleFitter and I3IterativeFitter will store this "
                  "information. See resources/examples/particletrace.py for an example. This "
                  "functionality is complementary to gulliver's lower level tracing.",
                  wantTrace_ );

    // limits
    FitParameterConfig( PAR_T, "vertex time", 200., 10000., "ns" );
    FitParameterConfig( PAR_X, "X coordinate of the vertex position", 200., 300., "m" );
    FitParameterConfig( PAR_Y, "Y coordinate of the vertex position", 200., 300., "m" );
    FitParameterConfig( PAR_Z, "Z coordinate of the vertex position", 200., 300., "m" );
    FitParameterConfig( PAR_Zen, "zenith angle", 10., 75., "degree" );
    FitParameterConfig( PAR_Azi, "azimuth angle", 10., 135., "degree" );
    FitParameterConfig( PAR_LinE, "energy (linear parametrization)", 10, 25., "TeV" );
    FitParameterConfig( PAR_LogE, "log10(energy/GeV) (logarithmic parametrization)", 0.5, 3.0, "" );
    FitParameterConfig( PAR_LinL, "length (linear parametrization)", 100, 250., "m" );
    FitParameterConfig( PAR_LogL, "log10(length) (logarithmic parametrization)", 0.5, 2.0, "" );
    double c_m_ns = I3Constants::c/(I3Units::m/I3Units::ns);
    FitParameterConfig( PAR_Speed, "Speed (e.g. for relativistic monopoles)", 0.1*c_m_ns, 0.8*c_m_ns, "m/ns" );

}


I3SimpleParametrization::~I3SimpleParametrization(){}

/// python convenience: set step at any time after construction
void I3SimpleParametrization::SetStep(I3SimpleParametrization::simplepar_t i, double step, bool verbose){
    if (step >= 0.){
        steps_[i] = step;
        InitializeFitParSpecs(verbose);
    } else {
        log_error("got illegal step values for %s: step=%f (legal would be: step>=0)",
                GetParName(i).c_str(), step);
    }
}

    /// python convenience: set absolute bounds at time after construction
void I3SimpleParametrization::SetAbsBounds(I3SimpleParametrization::simplepar_t i, double absmin, double absmax, bool verbose){
    if (absmin<=absmax){
        std::vector<double> bounds;
        bounds.push_back(absmin);
        bounds.push_back(absmax);
        absBounds_[i] = bounds;
        InitializeFitParSpecs(verbose);
    } else {
        log_error("got illegal values for bounds on %s: min=%f max=%f (legal would be: min<=max)",
                GetParName(i).c_str(), absmin, absmax);
    }
}

/// python convenience: set relative bounds at time after construction
void I3SimpleParametrization::SetRelBounds(I3SimpleParametrization::simplepar_t i, double relmin, double relmax, bool verbose){
    if (relmin<=0. && 0.<=relmax){
        std::vector<double> bounds;
        bounds.push_back(relmin);
        bounds.push_back(relmax);
        relBounds_[i] = bounds;
        InitializeFitParSpecs(verbose);
    } else {
        log_error("got illegal values for bounds on %s: min=%f (should be <=0) max=%f (should be >=0)",
                GetParName(i).c_str(), relmin, relmax);
    }
}

bool I3SimpleParametrization::InitChainRule(bool wantgradient)
{
    if (wantgradient) {
        gradient_ = I3EventHypothesisPtr(new I3EventHypothesis);
        // Zero the parameters we plan to use
        gradient_->particle->SetPos(I3Position(0,0,0));
        gradient_->particle->SetDir(I3Direction(0,0));
        gradient_->particle->SetTime(0);
        gradient_->particle->SetEnergy(0);
        gradient_->particle->SetLength(0);
        gradient_->particle->SetSpeed(0);
            par_gradient_.resize(free_.count(),NAN);
        }
    return (true);
}

// get the configuration wishes from the runscript
void I3SimpleParametrization::Configure() {

    std::string Step("Step");
    std::string AbsBounds("Bounds");
    std::string RelBounds("RelativeBounds");
    for ( int ipar=0; ipar<PAR_N; ++ipar ){
        std::string stepname = std::string("Step") + parNames_[ipar];
        std::string absname = std::string("Bounds") + parNames_[ipar];
        std::string relname = std::string("RelativeBounds") + parNames_[ipar];
        GetParameter(stepname, steps_[ipar]);
        GetParameter(absname, absBounds_[ipar]);
        GetParameter(relname, relBounds_[ipar]);
    }
    GetParameter( "VertexMode", vertexModeString_ );
    GetParameter( "ParticleTrace", wantTrace_ );

    // for contained tracks only: associate "vertex" to stop or start point for fitting?
    if ( ( vertexModeString_ == "Stop") || ( vertexModeString_ == "Stopping") ){
        vertexMode_ = I3SimpleParametrization::VERTEX_Stop;
    } else if ( ( vertexModeString_ == "Start") || ( vertexModeString_ == "Starting") ){
        vertexMode_ = I3SimpleParametrization::VERTEX_Start;
    } else if ( ( vertexModeString_.empty()) || ( vertexModeString_ == "Default") ){
        vertexMode_ = I3SimpleParametrization::VERTEX_Default;
    } else {
        log_fatal( "(%s) unknown VertexMode: \"%s\"",
                   GetName().c_str(), vertexModeString_.c_str() );
    }

    InitializeFitParSpecs();

}

void I3SimpleParametrization::InitializeFitParSpecs(bool verbose){

    // tell what we got, do some basic checks, handle boundaries
    if (verbose){
        ParInfo( "T", "ns", I3Units::ns, PAR_T );
        ParInfo( "X", "m", I3Units::m, PAR_X );
        ParInfo( "Y", "m", I3Units::m, PAR_Y );
        ParInfo( "Z", "m", I3Units::m, PAR_Z );
        ParInfo( "Zenith", "radian", I3Units::radian, PAR_Zen );
        ParInfo( "Azimuth", "radian", I3Units::radian, PAR_Azi );
        ParInfo( "L", "meter", I3Units::meter, PAR_LinL, PAR_LogL );
        ParInfo( "E", "GeV", I3Units::GeV, PAR_LinE, PAR_LogE );
        ParInfo( "Speed", "m/ns", I3Units::m/I3Units::ns, PAR_Speed );
    }

    // initialize parameter array and init specs
    free_.reset();
    parspecs_.clear();
    for ( unsigned int ipar = 0; ipar<PAR_N; ipar++ ){
        free_[ipar] = (steps_[ipar] > 0);
        if ( free_[ipar] ){
            //log_trace( "(%s) par %zu is \"%s\" with step %f",
                       //GetName().c_str(), parspecs_.size(), parNames_[ipar].c_str(), steps_[ipar] );
            I3FitParameterInitSpecs specs(parNames_[ipar]);
            specs.stepsize_ = steps_[ipar];
            specs.minval_   = absBounds_[ipar][0];
            specs.maxval_   = absBounds_[ipar][1];
            if ( relBounds_[ipar] != nobounds ){
                relativeBounds_[parspecs_.size()] = ipar;
            }
            parspecs_.push_back(specs);
        }
    }
    par_.resize(free_.count(),NAN);
}

// check configuration for a given parameter
void I3SimpleParametrization::ParInfo(
                    const char *varname,
                    const char* unitname, double unitval,
                    int iparlin, int iparlog ){
    assert(iparlin<I3SimpleParametrization::PAR_N);
    assert(iparlog<I3SimpleParametrization::PAR_N);
    std::vector<double> dummy(nobounds);
    std::vector<double> &bounds = absBounds_[iparlin];
    std::vector<double> &logbounds = (iparlog>=0) ? absBounds_[iparlog]: dummy;

    if ( bounds.empty() ) bounds = nobounds;
    if ( logbounds.empty() ) logbounds = nobounds;
    if ( bounds.size() != 2 ){
        log_fatal( "(%s) corrupt parameter configuration for lin %s "
                   "wrong number of bounds: %zu (should be 2)",
                   GetName().c_str(), varname, bounds.size() );
    }
    if ( logbounds.size() != 2 ){
        log_fatal( "(%s) corrupt parameter configuration for log %s "
                   "wrong number of bounds: %zu (should be 2)",
                   GetName().c_str(), varname, bounds.size() );
    }
    double lowerbound = 0.;
    double upperbound = 0.;
    double step = 0.;


    const char *linlog = "";
    double stepLin = steps_[iparlin];
    double stepLog = (iparlog>=0) ? steps_[iparlog] : 0.;
    if ( (stepLin*stepLog!=0.) ){
        log_fatal("(%s) you can't set both linear and logarithmic stepsize nonzero"
                  "(got for %s: lin step %g%s, log step %g%s",
                  GetName().c_str(), varname,
                  stepLin/unitval, unitname,
                  stepLog/unitval, unitname);
    } else if ( stepLin!=0. ){
        if ( logbounds != nobounds )
            log_fatal("(%s) lin stepsize, log bounds for %s",
                      GetName().c_str(), varname );
        linlog = ((iparlin==PAR_LinL)||(iparlin==PAR_LinE)) ? "Lin" : "";
        step = stepLin;
        lowerbound = bounds[0];
        upperbound = bounds[1];
    } else if ( stepLog!=0. ){
        if ( bounds != nobounds )
            log_fatal("(%s) log stepsize, lin bounds for %s",
                      GetName().c_str(), varname );
        linlog = "Log";
        step = stepLog;
        lowerbound = logbounds[0];
        upperbound = logbounds[1];
    } else if ( (bounds==nobounds) && (logbounds==nobounds) ){
        // not a free parameter at all
        log_debug( "(%s) %s is fixed (at its seed value).",
                   GetName().c_str(), varname );
    } else {
        log_fatal( "(%s) %s is fixed (at its seed value) "
                   "but got nontrivial bounds!",
                   GetName().c_str(), varname );
    }

    if ( (step<0.) || ! std::isfinite(step) ){
        log_fatal( "(%s) corrupt stepsize for %s: %g (should be >=0.)",
                   GetName().c_str(), varname, step );
    }
    
    if ( (lowerbound>upperbound) || (!std::isfinite(lowerbound))
                                 || (!std::isfinite(upperbound))
                                 || (!std::isfinite(stepLin))
                                 || (!std::isfinite(stepLog)) ){
        log_fatal( "(%s) corrupt parameter configuration for %s: "
                   "Bounds%s%s=[%f,%f], Step%s%s=%f",
                   GetName().c_str(), varname,
                   linlog, varname, lowerbound, upperbound,
                   linlog, varname, step );
    }

    if ( lowerbound<upperbound ){
        if ( upperbound-lowerbound < 2*step ){
            log_fatal("(%s) if you set bounds, they should be wider than twice the stepsize"
                      "(got Step%s%s=%f and Bounds%s%s=(%f,%f)",
                      GetName().c_str(), linlog, varname, step, linlog, varname, lowerbound, upperbound );
        }
        log_info( "(%s) %s is free with Step%s%s = %f%s, min=%f%s, max=%f%s",
                  GetName().c_str(), varname,
                  varname, linlog,
                  step/unitval, unitname,
                  lowerbound/unitval, unitname,
                  upperbound/unitval, unitname );
    } else if (step>0){
            log_info( "(%s) %s is free with Step%s%s=%f%s, no bounds",
                      GetName().c_str(), varname,
                      linlog, varname, step/unitval, unitname );
    }

}


/// this should calculate datamembers of EmissionHypothesis from the values in par_
void I3SimpleParametrization::UpdatePhysicsVariables(){

    I3Particle& simpletrack = *(hypothesis_->particle);
    I3Direction dir( simpletrack.GetDir() );
    double newzenith = dir.GetZenith();
    double newazimuth = dir.GetAzimuth();
    bool setdir = false;
    bool setpos = false;
    I3Position xyz;
    if ( ! (    (simpletrack.GetShape() == I3Particle::ContainedTrack )
             || (vertexMode_ == VERTEX_Default) ) ){
        log_fatal( "(%s) non-Contained seed track, but VERTEX MODE set to a non-default value",
                   GetName().c_str());
    }
    switch (vertexMode_){
        case VERTEX_Default: xyz = simpletrack.GetPos(); break;
        case VERTEX_Start: xyz = simpletrack.GetStartPos(); break;
        case VERTEX_Stop: xyz = simpletrack.GetStopPos(); break;
        default: log_fatal("(%s) unknown vertex mode %d",GetName().c_str(),vertexMode_); break;
    }

    unsigned int ipar = 0;
    if ( free_[PAR_T] ){
        simpletrack.SetTime( par_[ipar] );
        ++ipar;
    }
    if ( free_[PAR_X] ){
        xyz.SetX( par_[ipar] );
        ++ipar;
        setpos = true;
    }
    if ( free_[PAR_Y] ){
        xyz.SetY( par_[ipar] );
        ++ipar;
        setpos = true;
    }
    if ( free_[PAR_Z] ){
        xyz.SetZ( par_[ipar] );
        ++ipar;
        setpos = true;
    }
    if ( free_[PAR_Zen] ){
        newzenith = par_[ipar];
        ++ipar;
        setdir = true;
        if ( vertexMode_ == VERTEX_Stop ) setpos = true;
    }
    if ( free_[PAR_Azi] ){
        newazimuth = par_[ipar];
        ++ipar;
        setdir = true;
        if ( vertexMode_ == VERTEX_Stop ) setpos = true;
    }
    if ( free_[PAR_LinE] ){
        simpletrack.SetEnergy( par_[ipar] );
        ++ipar;
    }
    if ( free_[PAR_LogE] ){
        simpletrack.SetEnergy( exp(M_LN10*par_[ipar])*I3Units::GeV );
        ++ipar;
    }
    if ( free_[PAR_LinL] ){
        double newlength = par_[ipar];
        simpletrack.SetLength( newlength );
        if ( vertexMode_ == VERTEX_Stop ) setpos = true;
        ++ipar;
    }
    if ( free_[PAR_LogL] ){
        double newlength = exp(M_LN10*par_[ipar]) * I3Units::m;
        simpletrack.SetLength( newlength );
        if ( vertexMode_ == VERTEX_Stop ) setpos = true;
        ++ipar;
    }
    if ( free_[PAR_Speed] ){
        simpletrack.SetSpeed( par_[ipar] );
        ++ipar;
    }

    // direction changed
    if ( setdir ){
        simpletrack.SetDir( newzenith, newazimuth );
        I3Gulliver::AnglesInRange(simpletrack, GetName());
    }

    // vertex changed
    if ( setpos ){
        if ( vertexMode_ == VERTEX_Stop ){
            // For fits with contained tracks, the "position" is the
            // position of the starting point. But it could be that
            // the user would e.g. like the stop point to be fixed
            // (while fitting the length and/or direction). 
            const I3Direction& dir = simpletrack.GetDir();
            double length = simpletrack.GetLength();
            simpletrack.SetPos( xyz.GetX() - length * dir.GetX(),
                                xyz.GetY() - length * dir.GetY(),
                                xyz.GetZ() - length * dir.GetZ() );
        } else {
            simpletrack.SetPos( xyz );
        }
        if ( simpletrack.GetShape() == I3Particle::ContainedTrack ){
            log_trace("\n(%s) xyz=%s start=(%.1f,%.1f,%.1f) stop=(%.1f,%.1f,%.1f) L=%.1f=%.1f",
                      GetName().c_str(),
                      ((vertexMode_==VERTEX_Stop)?"Stop":
                       (vertexMode_==VERTEX_Start)?"Start":"Default"),
                      simpletrack.GetStartPos().GetX(),
                      simpletrack.GetStartPos().GetY(),
                      simpletrack.GetStartPos().GetZ(),
                      simpletrack.GetStopPos().GetX(),
                      simpletrack.GetStopPos().GetY(),
                      simpletrack.GetStopPos().GetZ(),
                      simpletrack.GetLength(),
                      (simpletrack.GetStartPos()-simpletrack.GetStopPos()).Magnitude() );
        }
    }

    // did we get all?
    assert(ipar == free_.count());

    if ( wantTrace_ ){
        trace_->push_back(simpletrack);
    }

}

/**
 * This should calculate the values in par_ from datamembers of EmissionHypothesis
 * If relevant it should also update stepsizes.
 */
void I3SimpleParametrization::UpdateParameters(){
    I3Particle& simpletrack = *(hypothesis_->particle);
    if ( wantTrace_ ){
        // (re)initialize with seed track
        trace_ = I3VectorI3ParticlePtr(new I3VectorI3Particle(1,simpletrack));
        trace_->reserve(1000); // random guess for max size
    }
    const I3Direction& dir = simpletrack.GetDir();

    // only relevant for contained tracks: 
    // should we associate the xyz coordinates of the track
    // to its starting point or its stopping point?
    I3Position xyz;
    switch (vertexMode_){
        case VERTEX_Default:
            xyz = simpletrack.GetPos();
            break;
        case VERTEX_Start:
            if ( simpletrack.GetShape() != I3Particle::ContainedTrack )
                log_fatal("(%s) vertexmode=start only supported for contained tracks",
                          GetName().c_str());
            xyz = simpletrack.GetStartPos();
            break;
        case VERTEX_Stop:
            if ( simpletrack.GetShape() != I3Particle::ContainedTrack )
                log_fatal("vertexmode=stop only supported for contained tracks");
            xyz = simpletrack.GetStopPos();
            break;
        default:
            log_fatal("unknown vertex mode %d",vertexMode_);
            break;
    }

    // set parameter values
    unsigned int ipar = 0;
    if ( free_[PAR_T] ){
        par_[ipar] = simpletrack.GetTime();
        ++ipar;
    }
    if ( free_[PAR_X] ){
        par_[ipar] = xyz.GetX();
        ++ipar;
    }
    if ( free_[PAR_Y] ){
        par_[ipar] = xyz.GetY();
        ++ipar;
    }
    if ( free_[PAR_Z] ){
        par_[ipar] = xyz.GetZ();
        ++ipar;
    }
    if ( free_[PAR_Zen] ){
        par_[ipar] = dir.GetZenith();
        ++ipar;
    }
    if ( free_[PAR_Azi] ){
        par_[ipar] = dir.GetAzimuth();
        ++ipar;
    }
    if ( free_[PAR_LinE] ){
        double energy = simpletrack.GetEnergy();
        if ( ! (std::isnormal(energy) && (energy > 0) ) ){
            // force positive energy
            energy = 1.0 * I3Units::TeV;
            simpletrack.SetEnergy( energy );
        }
        par_[ipar] = simpletrack.GetEnergy();
        ++ipar;
    }
    if ( free_[PAR_LogE] ){
        double energy = simpletrack.GetEnergy();
        if ( ! (std::isnormal(energy) && (energy > 0) ) ){
            // force positive energy
            energy = 1.0 * I3Units::TeV;
            simpletrack.SetEnergy( energy );
        }
        par_[ipar] = log10( simpletrack.GetEnergy()/I3Units::GeV );
        ++ipar;
    }
    if ( free_[PAR_LinL] ){
        double length = simpletrack.GetLength();
        if ( ! (std::isnormal(length) && (length > 0) ) ){
            // force positive length
            length = 100.0 * I3Units::m;
            simpletrack.SetLength( length );
        }
        par_[ipar] = simpletrack.GetLength();
        ++ipar;
    }
    if ( free_[PAR_LogL] ){
        double length = simpletrack.GetLength();
        if ( ! (std::isnormal(length) && (length > 0) ) ){
            // force positive length
            length = 100.0 * I3Units::m;
            simpletrack.SetLength( length );
        }
        par_[ipar] = log10( simpletrack.GetLength()/I3Units::m );
        ++ipar;
    }
    if ( free_[PAR_Speed] ){
        // FIXME: do we need to do any silly checks here?
        par_[ipar] = simpletrack.GetSpeed();
        ++ipar;
    }
    assert(ipar == free_.count());
    log_debug( "(%s) simple track par x=%f y=%f z=%f t=%f theta=%f phi=%f E=%g L=%f speed=%f",
               GetName().c_str(),
               xyz.GetX(), xyz.GetY(), xyz.GetZ(),
               simpletrack.GetTime(),
               dir.GetZenith(), dir.GetAzimuth(),
               simpletrack.GetEnergy(),
               simpletrack.GetLength(),
               simpletrack.GetSpeed()
            );

    // updating relative bounds (if any) to new seed values
    std::map<int,int>::iterator ibound;
    for ( ibound = relativeBounds_.begin();
            ibound != relativeBounds_.end();
            ibound++ ){

        int iarray=ibound->first;
        int ipar=ibound->second;
        I3FitParameterInitSpecs &specs = parspecs_[iarray];

        // basic operation: compute bounds relative to seed value
        double newminval = par_[iarray] + relBounds_[ipar][0];
        double newmaxval = par_[iarray] + relBounds_[ipar][1];

        // tweak: check if new bounds are compatible with abs bounds (if any)
        const double absmin = absBounds_[ipar][0];
        const double absmax = absBounds_[ipar][1];
        bool mintweak=false;
        bool maxtweak=false;
        if ( absmin < absmax ){ // check if there are abs bounds
            if (newminval < absmin ){
                newminval = absmin;
                mintweak = true;
            }
            if (newmaxval > absmax ){
                newmaxval = absmax;
                maxtweak = true;
            }
        }

        // tweak: inhibit negative values for energy and/or length (FIXME: and speed?)
        if ( ( ( ipar == PAR_LinE ) || ( ipar == PAR_LinL ) ) &&
             ( newminval < 0. ) ){
            newminval = 0.;
            mintweak = true;
        }

        // make sure that after tweaks the bounds interval is still large enough
        if ( newmaxval - newminval < 2*specs.stepsize_ ){
            if (mintweak) newmaxval = newminval + 2*specs.stepsize_;
            if (maxtweak) newminval = newmaxval - 2*specs.stepsize_;
        }

        // make sure that after tweaks the seed value is still well within bounds
        // TODO: need a log_warn here?
        // TODO: should we compare to bounds or to bounds +- stepsize?
        // TODO: should we also do this with abs bounds only?
        if ( newminval > par_[iarray] ) par_[iarray] = newminval + specs.stepsize_;
        if ( newmaxval < par_[iarray] ) par_[iarray] = newmaxval - specs.stepsize_;

        // commit new values
        specs.minval_ = newminval;
        specs.maxval_ = newmaxval;
    }
}

/* calculate the values in par_gradient_, using the current gradient_ and hypothesis_. */
void I3SimpleParametrization::ApplyChainRule()
{
    I3Particle& track = *(hypothesis_->particle);
    I3Particle& gradient = *(gradient_->particle);
    const I3Direction& dir = gradient.GetDir();
    const I3Position& xyz = gradient.GetPos();
    
    // set parameter values
    unsigned int ipar = 0;
    if ( free_[PAR_T] ){
        par_gradient_[ipar] = gradient.GetTime();
        ++ipar;
    }
    if ( free_[PAR_X] ){
        par_gradient_[ipar] = xyz.GetX();
        ++ipar;
    }
    if ( free_[PAR_Y] ){
        par_gradient_[ipar] = xyz.GetY();
        ++ipar;
    }
    if ( free_[PAR_Z] ){
        par_gradient_[ipar] = xyz.GetZ();
        ++ipar;
    }
    if ( free_[PAR_Zen] ){
        par_gradient_[ipar] = dir.GetZenith();
        // I3Gulliver::AnglesInRange() does some coordinate transformations
        // on our parameters. Mostly these involve addition of constants,
        // and so have no effect on the gradients, but in one case the sense 
        // of the zenith angle is reversed, which needs to be dealt with here.
        double raw_zenith = fmod(par_[ipar],2*M_PI);
        if (raw_zenith < 0)
            raw_zenith += 2*M_PI;
        if (raw_zenith > M_PI)
            par_gradient_[ipar] *= -1;
        ++ipar;
    }
    if ( free_[PAR_Azi] ){
        par_gradient_[ipar] = dir.GetAzimuth();
        ++ipar;
    }
    if ( free_[PAR_LinE] ){
        par_gradient_[ipar] = gradient.GetEnergy();
        ++ipar;
    }
    if ( free_[PAR_LogE] ){
        par_gradient_[ipar] = M_LN10*gradient.GetEnergy()*track.GetEnergy();
        ++ipar;
    }
    if ( free_[PAR_LinL] ){
    par_gradient_[ipar] = gradient.GetLength();
        ++ipar;
    }
    if ( free_[PAR_LogL] ){
        par_gradient_[ipar] = M_LN10*gradient.GetLength()*track.GetLength();
        ++ipar;
    }
    if ( free_[PAR_Speed] ){
        par_gradient_[ipar] = track.GetSpeed();
        ++ipar;
    }
    assert(ipar == free_.count());

}

typedef
I3SingleServiceFactory<I3SimpleParametrization,I3ParametrizationBase>
I3SimpleParametrizationFactory;
I3_SERVICE_FACTORY( I3SimpleParametrizationFactory );
