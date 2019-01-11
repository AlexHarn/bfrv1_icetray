/*
 * class: I3BasicSeedServiceFactory
 *
 * Version $Id$
 *
 * Date: 17 Feb 2004
 *
 * (c) IceCube Collaboration
 */

// gulliver thingies
#include "lilliput/seedservice/I3BasicSeedServiceFactory.h"
#include "lilliput/seedservice/I3BasicSeedService.h"

// icetray thingies
#include "icetray/IcetrayFwd.h"
I3_SERVICE_FACTORY(I3BasicSeedServiceFactory)

// option names
static const char* fgname_optionname = "FirstGuess";
static const char* fgnames_optionname = "FirstGuesses";
static const char* inputreadout_optionname = "InputReadout";
static const char* fixedE_optionname = "FixedEnergy";
static const char* energynch_optionname = "NChEnergyGuessPolynomial";
static const char* posshift_optionname = "PositionShiftType";
static const char* timeshift_optionname = "TimeShiftType";
static const char* alttimeshift_optionname = "AltTimeShiftType";
static const char* speed_optionname = "SpeedPolice";
static const char* tresmax_optionname = "MaxMeanTimeResidual";
static const char* chargefraction_optionname = "ChargeFraction";
static const char* addalt_optionname = "AddAlternatives";
static const char* onlyalt_optionname = "OnlyAlternatives";

// Constructors

I3BasicSeedServiceFactory::I3BasicSeedServiceFactory(const I3Context& context) :
        I3ServiceFactory(context){

    nContext_ = 0;

    AddParameter( fgname_optionname,
                  "Names of result of first guess module.\n",
                  fgName_ );

    AddParameter( fgnames_optionname,
                  "Names of results of several first guess modules.\n",
                  fgNames_ );

    AddParameter( inputreadout_optionname,
                  "Name of hits/pulses to use for vertex correction\n"
                  "(leave empty if you don't want any corrections)",
                  inputReadout_);

    fixedEnergy_ = NAN;
    AddParameter( fixedE_optionname,
                  "Most FG methods do not provide an energy estimate, "
                  "with this option you can give the energy some reasonable "
                  "starting value.\n"
                  "NOTE(0): 'Fixed' refers to the seed energy always being the "
                  "same value, independent of any event variales (e.g. on NCh). "
                  "So you *can* use this option if you actually want to fit "
                  "the energy: you configure that in the parametrization "
                  "service, not in the seed service.\n"
                  "NOTE(1): You cannot set both this option and the "
                  "NChEnergyGuessPolynomial option.\n"
                  "NOTE(2): If the FG already gives a value for the energy,"
                  "then setting the FixedEnergy or NChEnergyGuessPolynomial "
                  "option will clobber that value.\n"
                  "Default: leave FG energy as it is, regardless.",
                  fixedEnergy_ );

    energyGuessPolynomial_.clear();
    AddParameter( energynch_optionname,
                  "Most FG methods do not provide an energy estimate,"
                  "with this option you can fix the energy value. The"
                  "argument to this option should be vector of floats (use"
                  "proper units, e.g. I3Units.GeV)), which will be used to"
                  "obtain an energy estimate from NCh, via a polynomial: "
                  "log10(E/GeV) = p0+p1*x+p2*x*x+... with x=log(NCh). "
                  "You can get the efficients p0, p1, p2... from a fit to\n"
                  "a profile plot of log10(MCLeadMuon/GeV):log10(NHits).\n"
                  "For instance, for IC80 it seems that [3.627338, 0.299288, 0.437605]\n"
                  "works well.\n"
                  "NOTE(1): You cannot set both this option and the "
                  "FixedEnergy option.\n"
                  "NOTE(2): If the FG already gives a value for the energy,"
                  "then setting the FixedEnergy or NChEnergyGuessPolynomial "
                  "option will clobber that value.\n"
                   "Default: leave FG energy as it is, regardless.",
                  energyGuessPolynomial_ );

    timeShiftType_ = I3BasicSeedService::TUndefined;
    timeShiftTypeName_ = "TMean";
    AddParameter( timeshift_optionname,
                  "Indicate how you'd like the vertex *time* of the fg corrected: "
                  "such that mean or minimum time residual of the hits/pulses "
                  "is zero (use \"TMean\" or \"TFirst\", resp.), or no "
                  "correction at all (\"TNone\"). There are two *new* vertex "
                  "time methods: "
                  "\"TChargeFraction\" and \"TDirectChargeFraction\"."
                  "The former chooses the vertex such that some (configurable) "
                  "charge-weighted fraction of all pulses has a positive "
                  "time residual, the latter does the same but using only the "
                  "first hit/pulse in every DOM. What's best depends both "
                  "on the detector (AMANDA or IceCube) and on the first guess, "
                  "e.g.: DirectWalk first guess: TNone; "
                  "linefit first guess for AMANDA: TMean; "
                  "linefit first guess for IceCube: TFirst.",
                  timeShiftTypeName_ );

    posShiftType_ = I3BasicSeedService::PUndefined;
    posShiftTypeName_ = "COG";
    AddParameter( posshift_optionname,
                  "Indicate how you'd like the vertex *position* of the fg "
                  "corrected. Currently this is only relevant for infinite muon "
                  "tracks, for other I3Particle shapes the vertex position from "
                  "the first guess is taken as is. For infinite muons, if you "
                  "provide an InputReadout, then the vertex position of an "
                  "infinite track is by default (option value \"COG\") shifted "
                  "along the track to the closest point to the COG of the "
                  "hits/pulses.  Setting this option to \"None\" or empty string "
                  "will disable the correction of the vertex position.",
                  posShiftTypeName_ );

    chargeFraction_ = 0.9;
    AddParameter( chargefraction_optionname,
                  "If you use the \"TChargeFraction\" or "
                  "\"TDirectChargeFraction\" vertex time correction method, "
                  "then you can configure with this option the fraction "
                  "of the charge that should have a *positive* time residual.",
                  chargeFraction_ );

    altTimeShiftType_ = I3BasicSeedService::TUndefined;
    altTimeShiftTypeName_ = "TFirst";
    AddParameter( alttimeshift_optionname,
                  "This option is only relevant if you configure 'alternative "
                  "seeds' (add e.g. reverse fg guess track). You may want to "
                  "have different vertex tweaking for the original seed and "
                  "the alternative seeds: if the first guess is the result "
                  "of some LLH then you don't want to do any vertex tweaking "
                  "on that, but the alternative seeds are probably worthless "
                  "without vertex tweaking.",
                  altTimeShiftTypeName_ );

    speedPolice_ = true;
    AddParameter( speed_optionname,
                  "By default, the basic seed service will force the speed of "
                  "infinite tracks to be 1.0c and that of cascades to be 0. "
                  "If you would like to keep the speed value as provided by "
                  "the first guess (e.g. linefit sets a nontrivial speed value "
                  "since release V02-00-07), then set this flag to False.",
                  speedPolice_ );

    maxTResMean_ = 1000.0*I3Units::ns;
    AddParameter( tresmax_optionname,
                  "This option only has effect if you set TimeShift to \"TFirst\":\n"
                  "Occasionally an event has a weird little early cluster of hits;\n"
                  "that would totally sabotage this time correction. Hence for this\n"
                  "correction hits with a tres less than [optionvalue] below the mean\n"
                  "are ignored; in other words, it is guaranteed that *after* the time\n"
                  "correction, the mean time residual is between 0 and [optionvalue].\n"
                  "if you don't want this, set this option to NAN or a negative time.",
                  maxTResMean_ );

    addAlternativesString_ = "None";
    AddParameter( addalt_optionname,
                  "Add simple alternative seeds for each first guess;"
                  "argument is a string; possibilities: \"None\" (default,"
                  "no alternatives), \"Reverse\" (add a track the same"
                  "vertex in the opposite direction), \"Cubic\" (add 5"
                  "more tracks: the track and four perpendicular tracks).",
                  addAlternativesString_ );

    onlyAlternatives_ = false;
    AddParameter( onlyalt_optionname,
                  "If set to true: use *only* the alternative seeds, omit "
                  "the seeds that are directly based on the input tracks or "
                  "first guesses.",
                  onlyAlternatives_ );

}

// Destructors

I3BasicSeedServiceFactory::~I3BasicSeedServiceFactory(){
    log_trace( "The I3BasicSeedService service \"%s\" was installed in %d contexts",
              name_.c_str(), nContext_ );
}

// Member functions

bool
I3BasicSeedServiceFactory::InstallService(I3Context& ctx){

    ++nContext_;
    if ( seedService_ ){
        log_trace( "using existing instantiation for %s", name_.c_str() );
        return ctx.Put< I3SeedServiceBase >( seedService_, name_ );
    }

    log_debug( "making new instantiation for %s", name_.c_str() );

    assert( timeShiftType_ != I3BasicSeedService::TUndefined );
    seedService_ =
        I3SeedServiceBasePtr(
                new I3BasicSeedService(
                    name_, fgNames_, inputReadout_, fixedEnergy_,
                    energyGuessPolynomial_,
                    posShiftType_, timeShiftType_, altTimeShiftType_,
                    speedPolice_, maxTResMean_, chargeFraction_,
                    addAlternatives_, onlyAlternatives_ ));
    assert( seedService_ );
    bool success = ctx.Put< I3SeedServiceBase >( seedService_, name_ );
    if ( success ){
        log_debug( "(%s) basic seed service got installed fine.",
                   name_.c_str() );
    } else {
        log_fatal( "(%s) unknown problem when installing basic seed service.",
                   name_.c_str() );
    }
    return success;
}

void I3BasicSeedServiceFactory::Configure() {

    name_ = GetName();
    GetParameter( fgname_optionname, fgName_ );
    GetParameter( fgnames_optionname, fgNames_ );
    GetParameter( inputreadout_optionname, inputReadout_ );
    GetParameter( fixedE_optionname, fixedEnergy_ );
    GetParameter( energynch_optionname, energyGuessPolynomial_ );
    GetParameter( posshift_optionname, posShiftTypeName_ );
    GetParameter( timeshift_optionname, timeShiftTypeName_ );
    GetParameter( alttimeshift_optionname, altTimeShiftTypeName_ );
    GetParameter( speed_optionname, speedPolice_ );
    GetParameter( tresmax_optionname, maxTResMean_ );
    GetParameter( chargefraction_optionname, chargeFraction_ );
    GetParameter( addalt_optionname, addAlternativesString_ );
    GetParameter( onlyalt_optionname, onlyAlternatives_ );

    // list of first guesses
    if ( ! fgName_.empty() ){
        fgNames_.push_back( fgName_ );
    }
    if ( 0 == fgNames_.size() ){
        log_warn( "(%s) NOTE: No first guesses provided.\n"
                  "If this is a seed service for 'grid seeds' in paraboloid,\n"
                  "or the 'iteration tweak service' in the iterative fitter,\n"
                  "then that is fine, otherwise you'll have a problem.",
                   name_.c_str() );
    }
    std::vector< std::string >::const_iterator iseed;
    std::string seedlist;
    for ( iseed = fgNames_.begin(); iseed != fgNames_.end(); ++iseed ){
        seedlist += *iseed;
        seedlist += ", ";
    }
    log_debug( "(%s) %s=%s",
              name_.c_str(), fgnames_optionname,
              seedlist.substr(0,seedlist.size()-2).c_str() );

    // hits/pulses (for time/vertex shift)
    log_debug( "(%s) %s=%s",
              name_.c_str(), inputreadout_optionname, inputReadout_.c_str() );

    // energy checks
    if ( std::isnan(fixedEnergy_) && energyGuessPolynomial_.size() > 0 ){
        std::ostringstream oss;
        oss << "Always overrideing FG energy with Ench;\nlog10(Ench)=";
        std::string xpower = "";
        for ( std::vector<double>::iterator ifitp=energyGuessPolynomial_.begin();
                                            ifitp<energyGuessPolynomial_.end();
                                          ++ifitp ){
            oss << ((*ifitp>=0)?"+":"") << (*ifitp) << xpower;
            xpower += "*x";
        }
        oss << " (where x=log10(Nch))";
        log_debug("%s",oss.str().c_str());
    } else if ( std::isnan(fixedEnergy_) ){
        log_debug( "(%s) Always using energy value from FG.",
                  name_.c_str() );
    } else if (energyGuessPolynomial_.size() > 0 ){
        log_fatal( "(%s) You cannot set *both* \"%s\" and \"%s\"",
                  name_.c_str(), fixedE_optionname, energynch_optionname );
    } else {
        log_debug( "(%s) Always overriding FG energy value with E=%g GeV",
                  name_.c_str(),
                  fixedEnergy_/I3Units::GeV );
    }
    if ( (energyGuessPolynomial_.size() > 0) && inputReadout_.empty() ){
        log_fatal( "\n(%s) When you specify an energy polynomial with \"%s\" "
                   "\nthen you should also provide an input readout with \"%s\"",
                  name_.c_str(), energynch_optionname, inputreadout_optionname );
    }

    // whether to generate extra seeds
    if ( addAlternativesString_ == "None" ){
        if ( onlyAlternatives_ ){
            log_warn( "(%s) '%s'. Nice. Very funny.",
                      GetName().c_str(), addAlternativesString_.c_str() );
            onlyAlternatives_ = false;
        }
        addAlternatives_ = I3BasicSeedService::SeedAlt_None;
    } else if ( addAlternativesString_ == "Reverse" ){
        addAlternatives_ = I3BasicSeedService::SeedAlt_Reverse;
    } else if ( addAlternativesString_ == "Tetrahedron" ){
        addAlternatives_ = I3BasicSeedService::SeedAlt_Tetrahedron;
    } else if ( addAlternativesString_ == "Cube" ){
        addAlternatives_ = I3BasicSeedService::SeedAlt_Cube;
    } else {
        log_fatal( "\n(%s) Unknown alternative seed \"%s%s\"",
                   GetName().c_str(),
                   (onlyAlternatives_?"Only":""),
                   addAlternativesString_.c_str() );
    }

    // vertex time correction
    // would be nice if GetParameter would work with an enum.
    if      ( timeShiftTypeName_ == "TMean" ) timeShiftType_ = I3BasicSeedService::TMean;
    else if ( timeShiftTypeName_ == "TFirst" ) timeShiftType_ = I3BasicSeedService::TFirst;
    else if ( timeShiftTypeName_ == "TChargeFraction" ) timeShiftType_ = I3BasicSeedService::TChargeFraction;
    else if ( timeShiftTypeName_ == "TDirectChargeFraction" ) timeShiftType_ = I3BasicSeedService::TDirectChargeFraction;
    else if ( timeShiftTypeName_ == "TNone" ) timeShiftType_ = I3BasicSeedService::TNone;
    else {
        log_fatal( "\n(%s) Unknown time correction type: %s "
                   "\n(try setting the \"%s\" option to TMean, TFirst, "
                   "\nTChargeFraction, TDirectChargeFraction or TNone)",
                   name_.c_str(),
                   timeShiftTypeName_.c_str(), timeshift_optionname );
    }
    if      ( posShiftTypeName_ == "COG" ){
        posShiftType_ = I3BasicSeedService::PCOG;
    } else if ( posShiftTypeName_.empty() || ( posShiftTypeName_ == "None" ) ){
        posShiftType_ = I3BasicSeedService::PNone;
    } else {
        log_fatal( "\n(%s) Unknown position correction type: %s "
                   "\n(try setting the \"%s\" option to None, COG, "
                   "\nor empty string)",
                   name_.c_str(),
                   posShiftTypeName_.c_str(), posshift_optionname );
    }

    // vertex time correction
    // would be nice if GetParameter would work with an enum.
    if ( altTimeShiftTypeName_ == "TMean" ) altTimeShiftType_ = I3BasicSeedService::TMean;
    else if ( altTimeShiftTypeName_ == "TFirst" ) altTimeShiftType_ = I3BasicSeedService::TFirst;
    else if ( altTimeShiftTypeName_ == "TChargeFraction" ) altTimeShiftType_ = I3BasicSeedService::TChargeFraction;
    else if ( altTimeShiftTypeName_ == "TDirectChargeFraction" ) altTimeShiftType_ = I3BasicSeedService::TDirectChargeFraction;
    else if ( altTimeShiftTypeName_ == "TNone" ){
        altTimeShiftType_ = I3BasicSeedService::TNone;
        if ( addAlternatives_ != I3BasicSeedService::SeedAlt_None ){
            log_warn( "(%s) It's not advisable to use alternatives seeds "
                      "without vertex time tweaking "
                      "(try setting the \"%s\" option to TMean, TFirst, "
                      "TChargeFraction, TDirectChargeFraction or TNone)",
                       name_.c_str(), alttimeshift_optionname );
        }
    } else {
        log_fatal( "\n(%s) Unknown time correction type: %s "
                   "\n(try setting the \"%s\" option to TMean, TFirst, "
                   "\nTChargeFraction, TDirectChargeFraction or TNone)",
                   name_.c_str(),
                   timeShiftTypeName_.c_str(), alttimeshift_optionname );
    }

    // vertex time correction: protection boundary for TFirst
    if ( maxTResMean_ < 0) maxTResMean_ = NAN;
    if (    ( altTimeShiftType_ == I3BasicSeedService::TFirst )
         || ( timeShiftType_ == I3BasicSeedService::TFirst ) ){
        if ( maxTResMean_ > 0 ){
            log_debug( "(%s) for TFirst, the mean time residual will be "
                      "bounded to %.1fns",
                      name_.c_str(), maxTResMean_/I3Units::ns );
            log_debug( "(%s) That means: IF the lowest time residual is "
                      "more than %.1fns earlier than the mean of all "
                      "time residuals, THEN the vertex time will NOT be "
                      "shifted such that the lowest time residual is equal "
                      "to zero, BUT RATHER such that the mean time residual "
                      "is equal to %s=%.1f.",
                      name_.c_str(),
                      maxTResMean_/I3Units::ns,
                      tresmax_optionname,
                      maxTResMean_/I3Units::ns );
        } else {
            log_warn( "(%s) for TFirst, the mean time residual will be "
                      "unbounded (no protection agains very early hits)",
                      name_.c_str() );
        }
    }

    // vertex time correction: check charge fraction value.
    if ( ( chargeFraction_ < 0. ) || ( chargeFraction_ > 1. ) ){
        log_fatal( "\n(%s) Illegal charge fraction value %g, "
                   "should be between 0 and 1.",
                   name_.c_str(), chargeFraction_ );
    }

}
