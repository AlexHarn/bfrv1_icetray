/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3ParaboloidFitter.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

// tools, standard lib stuff
#include "icetray/IcetrayFwd.h"
#include <string>
#include <sstream>
#include <cmath>

// my header
#include "paraboloid/I3ParaboloidFitter.h"

// IceTray
I3_MODULE(I3ParaboloidFitter);
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "dataclasses/physics/I3EventHeader.h"

// dataclasses & gulliver
#include "dataclasses/geometry/I3Geometry.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "lilliput/parametrization/I3SimpleParametrization.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/utilities/ordinal.h"
#include "paraboloid/I3ParaboloidFitParams.h"


//----------------------------------------------------------------------------

I3ParaboloidFitter::I3ParaboloidFitter(const I3Context& ctx) : I3ConditionalModule(ctx){

    AddOutBox("OutBox");

    // initialize event counter, seedLlh_
    eventNr_ = 0;
    ndf_ = 0;
    zollCounter_ = 0;
    seedLlh_ = NAN;

    // options

    AddParameter( "Minimizer", "Name of or pointer to minimizer service to use.\n"
                  "If this option is left empty (or if the VertexStepSize\n"
                  "option is set to zero), then NO vertex refitting will be\n"
                  "done at the paraboloid grid points. This makes the\n"
                  "paraboloid fit much faster, but also probably less\n"
                  "accurate.",
                  minimizer_ );

    AddParameter( "LogLikelihood", "Name of or pointer to LogLikelihood service to use",
                  eventLLH_ );

    AddParameter( "SeedService", "Name of or pointer to seed service",
                  seedService_ );

    AddParameter( "OutputName",
                  "Name of the output I3Particle, and prefix for any fit "
                  "parameter frame objects",
                  "" );

    AddParameter( "GridPointVertexCorrection",
                  "Name of or pointer to  seed service to use for vertex corrections done "
                  "on grid points before trying to refit the vertex. "
                  "This aims to increase the convergence rate for all fits "
                  "in the grid. Default behavior (empty string): "
                  "no vertex correction for grid points.",
                  gridSeedService_ );

    nMaxMissingGridPoints_ = 0;
    AddParameter( "MaxMissingGridPoints",
                  "For each point at the paraboloid grid, the vertex is"
                  "refitted.  If that fit fails (minimizer does not converge)"
                  "then this causes a missing grid point. A workaround is"
                  "to use just the likelihood value of the seed grid point"
                  "(seed track with direction of grid point). With this"
                  "configuration parameter you specify how many grid points"
                  "with such a workaround are allowed before declaring the"
                  "paraboloid fit a failure. Set it to -1 for unlimited points"
                  "with workarounds. Default is 0 (no workarounds, paraboloid"
                  "fails when any grid point fit does not converge).",
                  nMaxMissingGridPoints_ );

    vertexStepSize_ = 10.0*I3Units::m;
    AddParameter( "VertexStepSize",
                  "Stepsize for vertex optimization at each gridpoint\n"
                  "If this option is set to zero (or if the Minimizer\n"
                  "option is left empty), then NO vertex refitting will be\n"
                  "done at the paraboloid grid points. This makes the\n"
                  "paraboloid fit much faster, but also probably less\n"
                  "accurate.",
                  vertexStepSize_ );

    zenithReach_ = 10.0*I3Units::degree;
    AddParameter( "ZenithReach", "How far to probe in zenith.\n"
                  "NOTE: Jon and Chad found that for IceCube, this parameter\n"
                  "should be set to a much smaller value than the default,\n"
                  "for instance 2 degrees.",
                  zenithReach_ );

    azimuthReach_ = 10.0*I3Units::degree;
    AddParameter( "AzimuthReach", "How far to probe in azimuth.\n"
                  "NOTE: Jon and Chad found that for IceCube, this parameter\n"
                  "should be set to a much smaller value than the default,\n"
                  "for instance 2 degrees.",
                  azimuthReach_ );

    nSteps_ = 3;
    AddParameter( "NumberOfSteps", "Number of steps (FIXME: better description)",
                  nSteps_ );

    nSamplingPoints_ = 8;
    AddParameter( "NumberOfSamplingPoints", "Nr of sampling points ( FIXME: better description)",
                  nSamplingPoints_ );

    AddParameter( "MCTruthName", "Name of the MC list or tree. "
                  "Default: no MC comparison.",
                  mcName_ );

}

I3ParaboloidFitter::~I3ParaboloidFitter(){
    if (zollCounter_>0){
        log_error( "(%s) encountered %u frames (out of %u) with a 'good' seed,  but NDF<=0",
                  GetName().c_str(), zollCounter_, eventNr_ );
        log_error( "(%s) For events with NDF<=0 your input fits (using the same likelihood) should already have failed.",
                  GetName().c_str() );
        log_error( "(%s) This error could happen if for instance you feed different pulsemaps to the likelihood service "
                  "when running paraboloid, than you did when performing the original likelihood fit. "
                  "That would be bad, because really, the paraboloid fit does not make ANY sense if you do not "
                  "use *exactly* the same likelihood definition as for the original fit.", GetName().c_str());
        log_error( "(%s) Or it could happen if (maybe due to a typo) your input seed is a different fit altogether, "
                   "e.g. a linefit, in which case a paraboloid fit is completely meaningless.", GetName().c_str());
        log_error( "(%s) (If you REALLY know what you are doing, "
                   "or if you happen to be running the 'zollamt.py' script, "
                  "then the following admonition does not apply.)", GetName().c_str());
        log_error( "(%s) FIX YOUR SCRIPT(S)!!!", GetName().c_str() );
    }
}

//----------------------------------------------------------------------------
void I3ParaboloidFitter::Configure(){

    GetParameter( "Minimizer", minimizer_ );
    GetParameter( "LogLikelihood", eventLLH_ );
    GetParameter( "SeedService", seedService_ );
    GetParameter( "OutputName", fitName_ );
    GetParameter( "GridPointVertexCorrection", gridSeedService_ );
    GetParameter( "MaxMissingGridPoints", nMaxMissingGridPoints_ );
    GetParameter( "VertexStepSize", vertexStepSize_ );
    GetParameter( "ZenithReach", zenithReach_ );
    GetParameter( "AzimuthReach", azimuthReach_ );
    GetParameter( "NumberOfSteps", nSteps_ );
    GetParameter( "NumberOfSamplingPoints", nSamplingPoints_ );
    GetParameter( "MCTruthName", mcName_ );

    if (fitName_ == ""){
        log_warn( "Parameter \"OutputName\" of I3SimpleFitter instance \"%s\" was not set! "
                  "Falling back to old behavior and using instance name. "
                  "Please update your scripts, the fallback will be disabled in the next version.",
                  GetName().c_str() );
        fitName_= GetName();
    }

    bool fatal = false;
    
    if (vertexStepSize_ > 0. && !minimizer_){
	log_fatal( "Module \"%s\" configured no minimizer "
		   "and vertexstepsize=%.1fm. "
		   "You must set the vertexstepsize to <=0.0 "
		   "to run with out vertex refitting.",
		   GetName().c_str(), vertexStepSize_ );
	fatal=true;
    }

    if ( !gridSeedService_ ){
      log_notice( "Module \"%s\": configured with parameter GridPointVertexCorrection=\"\","
		  "grid seeds will NOT get vertex corrections before the "
		  "vertex fit. ", GetName().c_str() );
    }
    
    log_info( "Module \"%s\": configured with vertex stepsize %f meters",
	      GetName().c_str(), vertexStepSize_/I3Units::m);
    
    if ((zenithReach_>0.0) && (zenithReach_ < M_PI)){
      log_info( "Module \"%s\": configured with zenith reach %f degrees",
		GetName().c_str(), zenithReach_/I3Units::degree);
    }else{
      log_info( "Module \"%s\": configured with zenith reach %f degrees, "
		"The value must be between 0 and 180 degrees",
		GetName().c_str(), zenithReach_/I3Units::degree);
      fatal=true;
    }
    
    if (( nSamplingPoints_ < 100   ) &&
	( nSamplingPoints_ > 0     ) &&
	( 0 == (nSamplingPoints_%4))){
      log_info( "Module \"%s\": configured with number of sampling points %d",
		GetName().c_str(), nSamplingPoints_);
    }else{
      log_error( "Module \"%s\": configured with number of sampling points %d, "
		 "The value must be between 0 and 100 as well as divisable by 4",
		 GetName().c_str(), nSamplingPoints_);
      fatal=true;
    }
    if ((nSteps_ > 0) && (nSteps_ < 11) ){
      log_info( "Module \"%s\": configured with number of steps %d",
		GetName().c_str(), nSteps_);
    }else{
      log_error( "Module \"%s\": configured with number of steps %d, "
		 "The value must be between 1 and 10",
		 GetName().c_str(), nSteps_);
      fatal=true;
    }
    if (!eventLLH_) {
        log_info("No likelihood service configured");
        fatal=true;
    }
    if (!seedService_) {
        log_info("No seed service configured");
        fatal=true;
    }
    // Optional parameters are a special case. Figure out of the user meant to
    // leave it unconfigured, or just screwed up.
    if (!gridSeedService_){
        std::string gridSeedServiceName;
        GetParameter("GridPointVertexCorrection", gridSeedServiceName);
        if (!gridSeedServiceName.empty()) {
            log_info_stream("No grid seed service named "<<gridSeedServiceName<<" found");
            fatal=true;
        }
    }
    
    if ( fatal ){
      log_fatal( "Module \"%s\" encounterd one or more errors while configuring the "
		 "parameters. Check the error messages above to see which ones. ",
		 GetName().c_str() );
      return;
    }

    if ( vertexStepSize_ > 0. ){
        const std::vector<double> nobounds(2,0.0);
        std::vector<double> steps(I3SimpleParametrization::PAR_N,0.);
        steps[I3SimpleParametrization::PAR_X] = vertexStepSize_;
        steps[I3SimpleParametrization::PAR_Y] = vertexStepSize_;
        steps[I3SimpleParametrization::PAR_Z] = vertexStepSize_;
        std::vector< vector<double> > absbounds(I3SimpleParametrization::PAR_N,nobounds);
        std::vector< vector<double> > relbounds(I3SimpleParametrization::PAR_N,nobounds);
        vertexParametrization_ =
            I3SimpleParametrizationPtr(
                    new I3SimpleParametrization(
                        GetName()+"_vertexpar", steps, absbounds, relbounds ));
        assert( vertexParametrization_ );

        fitterCore_ = I3GulliverPtr( 
            new I3Gulliver( 
                vertexParametrization_,
                eventLLH_,
                minimizer_,
                GetName()
            )
        );
    }

    if ( mcName_.empty() ){
        log_info( "(%s) No MC tree/list name provided, so no truth comparison.",
                  GetName().c_str() );
    } else {
        log_info( "(%s) Truth comparison with MC tree/list \"%s\".",
                  GetName().c_str(), mcName_.c_str() );
    }

    if ( nMaxMissingGridPoints_ < 0 ){
        log_info( "(%s) No limit on number of grid points with nonconverging "
                  "fits; for failed grid points the likelihood of the grid "
                  "seed is used.",
                  GetName().c_str() );
        nMaxMissingGridPoints_ = INT_MAX;
    } else {
        log_info( "(%s) Max. %d grid point(s) with nonconverging fits will "
                  "be allowed; for failed grid points the likelihood of the "
                  "grid seed is used.",
                  GetName().c_str(), nMaxMissingGridPoints_ );
    }

}

void I3ParaboloidFitter::Geometry(I3FramePtr frame){
    log_debug("(%s) Welcome to the Geometry method of ParaboloidFitter!",
                GetName().c_str());
    const I3Geometry& inicegeo = frame->Get< I3Geometry >();
    eventLLH_->SetGeometry(inicegeo);
    PushFrame( frame, "OutBox" );
    log_debug( "(%s) Leaving I3Paraboloid Geometry().",
               GetName().c_str() );
}

//----------------------------------------------------------------------------
void I3ParaboloidFitter::Physics(I3FramePtr frame){

    ++eventNr_;
    log_debug( "(%s) Welcome to the physics method of ParaboloidFitter!",
              GetName().c_str());
    log_debug( "(%s) This is the %s event.",
              GetName().c_str(), ordinal(eventNr_));
    // assert(fitterCore_);
    assert(seedService_);

    // get/set seed
    unsigned int nseeds = seedService_->SetEvent(*frame);
    log_debug( "(%s) got %d seed(s)",
              GetName().c_str(), nseeds );
    I3EventHypothesisPtr hypothesis( (nseeds == 0) ? seedService_->GetDummyPtr( )
                                                   : seedService_->GetSeedPtr( 0 ) );

    // prepare fit result object
    I3ParaboloidFitParamsPtr pbfParams( new I3ParaboloidFitParams);
    I3LogLikelihoodFitPtr fitptr( new I3LogLikelihoodFit( hypothesis, pbfParams ) );


    // Go for it!

    // Execute do-while loop ONE time.  if anything goes wrong, then:
    //   1) Set pbfStatus_ to indicate failure, and
    //   2) Break out of loop.

    do {
        if ( nseeds == 0 ){
            log_debug( "(%s) no seed available...", GetName().c_str() );
            fitptr->hypothesis_->particle->SetFitStatus( I3Particle::MissingSeed );
            pbfParams->pbfStatus_ = I3ParaboloidFitParams::PBF_NO_SEED;
            seedLlh_ = NAN;
	    break;
        } else {
            if ( nseeds > 1 ){
	        log_debug( "(%s) Got multiple seeds (%u), using only the first one!",
                          GetName().c_str(), nseeds );
            }
        }

        if ( ! NDFOK(*frame,*hypothesis) ){
            hypothesis->particle->SetFitStatus( I3Particle::InsufficientHits );
            // maybe we should introduce an INSUFFICIENT HITS value ParaboloidFitStatus as well
            pbfParams->pbfStatus_ = I3ParaboloidFitParams::PBF_FAILED_PARABOLOID_FIT;
            log_trace( "(%s) fit status set to %d=\"%s\", pbfstatus=%d",
                       GetName().c_str(),
                       hypothesis->particle->GetFitStatus(),
                       hypothesis->particle->GetFitStatusString().c_str(),
                       pbfParams->pbfStatus_ );
	    break;
        }

	// for convenience
	const I3Position pos = fitptr->hypothesis_->particle->GetPos();
	const I3Direction dir = fitptr->hypothesis_->particle->GetDir();
	int return_result;

        log_debug( "(%s) starting paraboloid with seed x=%f y=%f z=%f t=%f "
                  "theta=%f phi=%f energy=%f length=%f fitstat=%s(%d)",
                  GetName().c_str(),
                  pos.GetX(), pos.GetY(), pos.GetZ(),
                  fitptr->hypothesis_->particle->GetTime(),
                  dir.GetZenith(), dir.GetAzimuth(),
                  fitptr->hypothesis_->particle->GetEnergy(),
                  fitptr->hypothesis_->particle->GetLength(),
                  fitptr->hypothesis_->particle->GetFitStatusString().c_str(),
                  fitptr->hypothesis_->particle->GetFitStatus() );

        // map llh landscape around seed
        return_result = GetParaboloidDatapoints(*frame,*hypothesis);
	if (return_result != 1) {
	    log_debug("Failed: GetParaboloidDatapoints =%d",return_result);
	    pbfParams->pbfStatus_ = 
	        I3ParaboloidFitParams::PBF_INCOMPLETE_GRID;
            fitptr->hypothesis_->particle->SetFitStatus(I3Particle::GeneralFailure);
	    break;
	}

        // do the parabola fit
        double detM=NAN; // hm. calculated but not used
        return_result = paraFit_.lin_reg_parabola_2_sym(
				       paraPol_, &chi2_, &detM);
	if (return_result != 1) {
	    log_debug("Failed: paraFit_.lin_reg_parabola_2_sym =%d",
		     return_result);
	    pbfParams->pbfStatus_ = 
	        I3ParaboloidFitParams::PBF_FAILED_PARABOLOID_FIT;
            fitptr->hypothesis_->particle->SetFitStatus(I3Particle::GeneralFailure);
	    break;
	}

        // turn the polynomial representation into the standard representation
        paraStd_ = paraPol_;
        // TODO: error handling?

        // get fit quality estimate
	return_result = GetErrorsFromCurvature(*pbfParams);
	if (return_result == 0) {
	    log_debug("Failed: GetErrorsFromCurvature =%d",return_result);
	    pbfParams->pbfStatus_ = 
	        I3ParaboloidFitParams::PBF_SINGULAR_CURVATURE_MATRIX;
            fitptr->hypothesis_->particle->SetFitStatus(I3Particle::GeneralFailure);
	    break;
	}


	// okay, fit was completed, so store other results
	StoreResults(*pbfParams);
        // TODO: error handling ?


	// additional checks: is fit good enough to use?

        // guilty until proven innocent
        fitptr->hypothesis_->particle->SetFitStatus(I3Particle::GeneralFailure);

	if (pbfParams->pbfErr1_<=0 && pbfParams->pbfErr2_<=0) {
	  pbfParams->pbfStatus_ = 
	    I3ParaboloidFitParams::PBF_NON_POSITIVE_ERRS;
	  break;
	}
	if (pbfParams->pbfErr1_<=0) {
	  pbfParams->pbfStatus_ = 
	    I3ParaboloidFitParams::PBF_NON_POSITIVE_ERR_1;
	  break;
	}
	if (pbfParams->pbfErr2_<=0) {
	  pbfParams->pbfStatus_ = 
	    I3ParaboloidFitParams::PBF_NON_POSITIVE_ERR_2;
	  break;
	}

	if (pbfParams->pbfErr1_<1e-5 || pbfParams->pbfErr2_<1e-5) {
	  pbfParams->pbfStatus_ = 
	    I3ParaboloidFitParams::PBF_TOO_SMALL_ERRS;
	  break;
	}

        // hypothesis seems fine
        fitptr->hypothesis_->particle->SetFitStatus(I3Particle::OK);
        GetLogLHFitParams( *frame, fitptr );

        // Getting this far means success! Everything passed!
        pbfParams->pbfStatus_ = I3ParaboloidFitParams::PBF_SUCCESS;

    } while (0);  // i.e. don't do the 'loop' a second time.

    GetTruth(*pbfParams, frame);
    // TODO: error handling ?

    frame->Put( fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX,
                fitptr->hypothesis_->particle );
    if ( fitptr->hypothesis_->nonstd ){
        frame->Put( fitName_ + I3LogLikelihoodFit::NONSTD_SUFFIX,
                    fitptr->hypothesis_->nonstd );
    }
    frame->Put( fitName_ + I3LogLikelihoodFit::FITPARAMS_SUFFIX,
                fitptr->fitparams_ );

    PushFrame( frame, "OutBox" );
    log_debug( "(%s) Leaving I3ParaboloidFitter Physics().",
               GetName().c_str() );
}

bool I3ParaboloidFitter::NDFOK(const I3Frame &f, I3EventHypothesis &hypothesis){
    eventLLH_->SetEvent(f);
    ndf_ = eventLLH_->GetMultiplicity() - 5;
    if ( ndf_ <= 0 ){
        const unsigned int zollMax=100;
        if (zollCounter_++<zollMax){
            log_error( "(%s) Event has bad multiplicity: NDF=%d.",
                       GetName().c_str(), ndf_ );
            log_error( "(%s) This is really bad: your 'seed' fit should already have failed on this event, "
                       "but the seed service apparently found that it had OK fit status.",
                       GetName().c_str() );
            log_error( "(%s) One way to land in this miserable situation is if you configured paraboloid with "
                       "different pulsemap or a different likelihood configuration than the input llh fit.",
                       GetName().c_str() );
            log_error( "(%s) (Note that in that case *all* paraboloid fits are bad, "
                       "not only the ones which happen to get NDF<=0)", GetName().c_str() );
            log_error( "(%s) Proceed only if you really know what you are doing, or else: FIX YOUR SCRIPTS",
                       GetName().c_str() );
            if ( f.Has("I3EventHeader") ){
                std::ostringstream oss;
                oss << f.Get<I3EventHeader>("I3EventHeader");
                log_error( "(%s) Offending event: %s",
                          GetName().c_str(), oss.str().c_str() );
            }
        }
        if (zollCounter_ == zollMax){
            log_warn( "(%s) This error won't be repeated for the remaining offending events.",
                      GetName().c_str() );
        }
    }
    return (ndf_>0);
}

// almost literally copied from sieglinde
int I3ParaboloidFitter::GetParaboloidDatapoints(const I3Frame &f, const I3EventHypothesis &hypothesis){

    // mostly copied from sieglinde/reconstruction/SLParaboloidLogLH.cc

    // temporary data point added to ParabolaFit list
    static ParaboloidImpl::XYTupel datapoint;
    // azimuth/zenith coordinates relative to the seeding track
    // (if rotated to pi/2;pi)
    double azi_off, zen_off;
    // azimuth/zenith coordinates in standard CO system
    double azimuth, zenith;
    // temporary vector representing the seed track
    I3Direction v; // this used to be a TVector3

    // we start on the seeding track, shift x,y,z,t and minimize x,y,z
    // do the initial trafo of the tracks time (projection on cherenkov cone
    // run the minimizer once on the seeding track, so that x,y,z are ok)

    I3EventHypothesis seedcp = seedService_->GetCopy(hypothesis);
    I3LogLikelihoodFitPtr fitptr(new I3LogLikelihoodFit(seedcp));

    // do it

    if ( fitterCore_ ){
        bool success = fitterCore_->Fit( fitptr );

        // store results
        if ( ! success ){
            assert( fitptr->hypothesis_->particle->GetFitStatus() != I3Particle::OK );
            log_debug( "(%s) bummer! status=%s(%d), ndof=%d nmini=%d",
                      GetName().c_str(),
                      fitptr->hypothesis_->particle->GetFitStatusString().c_str(),
                      fitptr->hypothesis_->particle->GetFitStatus(),
                      fitptr->fitparams_->ndof_,
                      fitptr->fitparams_->nmini_ );
            return -1;
        }
    } else {
        fitptr->fitparams_->logl_ = -1.0*eventLLH_->GetLogLikelihood( seedcp );
        fitptr->fitparams_->rlogl_ = fitptr->fitparams_->logl_/ndf_;
        fitptr->fitparams_->ndof_ = ndf_;
        fitptr->fitparams_->nmini_ = 0;
    }

    // clear all internal ParabolaFit variables
    paraFit_.Clear();

    // fill the center point into the datapoints structure
    datapoint.x[0] = 0;
    datapoint.x[1] = 0;
    seedLlh_ = datapoint.y = fitptr->fitparams_->logl_;
    paraFit_.push_back(datapoint);

    // extract zenith and azimuth from the seeding track
    zenith  = seedcp.particle->GetDir().GetZenith();
    azimuth = seedcp.particle->GetDir().GetAzimuth();
    v = I3Direction(zenith,azimuth);
     // start net by using the seeding fit information
    if (grid_.Init(nSamplingPoints_, nSteps_, azimuthReach_, zenithReach_, v) < 0){
        log_debug( "initializing grid failed" );
        return -1;
    }

    // if the grid seed service needs hit information, it should get it now...
    if ( gridSeedService_ ){
        gridSeedService_->SetEvent(f);
    }

    int nFailures = 0;
    // get next point
    while ( grid_.GridNext(azi_off, zen_off, azimuth, zenith) > 0 ){

        // new copy from central gridpoint
        fitptr->hypothesis_ = seedService_->GetCopyPtr( seedcp );

        // put in the zenith+azimuth values from the grid
        fitptr->hypothesis_->particle->SetDir(zenith,azimuth);

        // the grid datapoints are based on the coordinates rotated to the equator
        datapoint.x[0] = zen_off;
        datapoint.x[1] = azi_off;

        double llhpre=0;
        double llhpost=0;
        if ( nMaxMissingGridPoints_ > 0 ){
            llhpre = -1.0 * eventLLH_->GetLogLikelihood(*(fitptr->hypothesis_));
        }

        // do vertex spacetime tweaks, if wanted.
        if ( gridSeedService_ ){
            gridSeedService_->Tweak(*(fitptr->hypothesis_));
            if ( nMaxMissingGridPoints_ > 0 ){
                llhpost = -1.0 * eventLLH_->GetLogLikelihood(*(fitptr->hypothesis_));
            }
        } else {
            llhpost = llhpre;
        }


        if ( fitterCore_ ){
            bool success = fitterCore_->Fit( f, fitptr );

            // store results
            if ( success ){

                datapoint.y = fitptr->fitparams_->logl_;

            } else {

                assert( fitptr->hypothesis_->particle->GetFitStatus() != I3Particle::OK );
                log_debug( "(%s) bummer! status=%s(%d), ndof=%d nmini=%d",
                          GetName().c_str(),
                          fitptr->hypothesis_->particle->GetFitStatusString().c_str(),
                          fitptr->hypothesis_->particle->GetFitStatus(),
                          fitptr->fitparams_->ndof_,
                          fitptr->fitparams_->nmini_ );

                ++nFailures;
                if ( nFailures > nMaxMissingGridPoints_ ) return -1;

                // workaround: use llh value from grid seed
                // (with or without tweak, whichever is best)
                // hm. maybe nFailures is an interesting pbf parameter?
                datapoint.y = (llhpre<llhpost) ? llhpre: llhpost;

            }
        } else {
            datapoint.y = -1.0*eventLLH_->GetLogLikelihood( *(fitptr->hypothesis_) );
        }

        paraFit_.push_back(datapoint);
    }

    return 1;
}

// almost literally copied from sieglinde
int I3ParaboloidFitter::GetErrorsFromCurvature(I3ParaboloidFitParams &parabParam){
    // dummy
    // should be copy-adapted from sieglinde/reconstruction/SLParaboloidLogLHFit.cc
    int    myreturn;                 // for local return values
    double detA;                     // determinant of A
    double eigenvalue1, eigenvalue2; // eigenvalues of the diagonalized matrix
    double rotation_angle;           // angle by which the original matrix must be rotated to be diagonal
  
    // case of a singular matrix. That should only happen *very* rarely
    detA = ParaboloidImpl::determinant(paraStd_.A);   
    if (detA == 0 || std::isnan(detA)) {
        log_debug("WARNING: hesse_tn6: matrix Afit singular - cannot obtain errors"); 
        // determinant of curvature matrix of parabola fit
        parabParam.pbfDetCurvM_ = 0.;
        return 0;
    } 

    // invert matrix to obtain errors
    bnu::matrix<double> inverse(paraStd_.A);        // inverse matrix of A
    bool ok = ParaboloidImpl::invert_matrix(paraStd_.A,inverse);
    if (!ok){
        log_error("determinant nonzero but still not able to invert matrix...");
        // determinant of curvature matrix of parabola fit
        parabParam.pbfDetCurvM_ = 0.;
        return 0;
    }

    // case of successfull inversion follows

    // determinant of curvature matrix of parabola fit
    parabParam.pbfDetCurvM_ = detA;

    // we determine the errors in zenith and track azimuth and their correlation

    // covariance from inverted curvature matrix
    parabParam.pbfCovar_ = inverse(0,1);

    // The square root of the variance is the standard deviation.
    // We correct for the case that the former could be negative.
    parabParam.pbfSigmaZen_ = copysign(sqrt(fabs(inverse(0,0))), inverse(0,0));
    parabParam.pbfSigmaAzi_ = copysign(sqrt(fabs(inverse(1,1))), inverse(1,1));

    // we try to diagonalize the inverted matrix. If successful we
    // store the eigenvalues and the rotation angle
    myreturn = ParaboloidImpl::diagonalize_sym_2_2_matrix(inverse, eigenvalue1, eigenvalue2, rotation_angle);

    if (myreturn > 0) {
        parabParam.pbfErr1_ = copysign(sqrt(fabs(eigenvalue1)), eigenvalue1);   // error 1 of diagonalized hesse matrix
        parabParam.pbfErr2_ = copysign(sqrt(fabs(eigenvalue2)), eigenvalue2);   // error 2 of diagonalized hesse matrix 
	parabParam.pbfRotAng_ = rotation_angle;                                    // rotation angle to diagonalize hesse matrix
    }
    return myreturn;
}

int I3ParaboloidFitter::StoreResults(I3ParaboloidFitParams &parabParam){

    // use the rotation on the shift-value b[2] to obtain the
    // paraboloid minimum in AMANDA coordinates
    bnu::vector<double> b_global(2);
    b_global = paraStd_.b;
    grid_.localnet.Rotate(b_global[1], b_global[0]);

    // now save all the data in structure
    parabParam.pbfCenterLlh_ = seedLlh_;
    parabParam.pbfLlh_       = paraStd_.c;
    parabParam.pbfZenOff_    = paraStd_.b[0];
    parabParam.pbfAziOff_    = paraStd_.b[1];
    parabParam.pbfCurv11_    = paraStd_.A(0,0);
    parabParam.pbfCurv12_    = paraStd_.A(0,1);
    parabParam.pbfCurv22_    = paraStd_.A(1,1);
    parabParam.pbfChi2_      = chi2_;
    parabParam.pbfZen_       = b_global[0];
    parabParam.pbfAzi_       = b_global[1];

    return 1;
}

// helper method to check if a particle traverses a given depth range
// (instead of the InIce LocationType check)
bool I3ParaboloidFitter::IsInDepthRange( const I3Particle &p,
                                         double zmin, double zmax ){
    assert(zmin<zmax);
    switch ( p.GetShape() ){
        case I3Particle::Cascade:
            {
                double zstop=p.GetPos().GetZ();
                return ((zstop>zmin) && (zstop<zmax));
            }
            break;
        case I3Particle::InfiniteTrack:
            if ( cos(p.GetZenith()) != 0 ) return true;
            {
                // horizontal track
                double z=p.GetPos().GetZ();
                return ((z>zmin) && (z<zmax));
            }
            break;
        case I3Particle::StartingTrack:
            {
                double zstart=p.GetStartPos().GetZ();
                double pz=p.GetDir().GetZ();
                if ( pz<0 ) return (zstart>zmin);
                else if ( pz==0 ) return ((zstart>zmin) && (zstart<zmax));
                else if ( pz>0 ) return (zstart<zmax);
            }
            break;
        case I3Particle::StoppingTrack:
            {
                double zstop=p.GetStopPos().GetZ();
                double pz=p.GetDir().GetZ();
                if ( pz>0 ) return (zstop>zmin);
                else if ( pz==0 ) return ((zstop>zmin) && (zstop<zmax));
                else if ( pz<0 ) return (zstop<zmax);
            }
            break;
        case I3Particle::ContainedTrack:
            {
                double zstart=p.GetStartPos().GetZ();
                double zstop=p.GetStopPos().GetZ();
                return (
                  ( (zstart>zmax)&&(zstop <zmin) ) || // through, downward
                  ( (zstop <zmax)&&(zstop >zmin) ) || // stop inside
                  ( (zstart<zmax)&&(zstart>zmin) ) || // start inside
                  ( (zstop >zmax)&&(zstart<zmin) )    // through, upward
                );
            }
            break;
        default:
            break;
    };
    return false;
}

int I3ParaboloidFitter::GetTruth(I3ParaboloidFitParams &parabParam, I3FramePtr f){

    if ( mcName_.empty() ){
        return 0;
    }

    if ( ! f->Has( mcName_) ){
        log_fatal( "no MC tree/list available with name \"%s\"",
                   mcName_.c_str() );
    }

    I3MCTreeConstPtr mctree = f->Get<I3MCTreeConstPtr>(mcName_);
    double maxenergy = 0.;
    double tzen = NAN;
    double tazi = NAN;
    I3MCTree::const_iterator iter;

    for ( iter=mctree->begin(); iter!=mctree->end(); ++iter){
        if ( (iter->GetEnergy()>maxenergy) &&
             iter->HasDirection() &&
             IsInDepthRange(*iter,-500*I3Units::m,+500*I3Units::m) ){
            maxenergy = iter->GetEnergy();
            const I3Direction &dir = iter->GetDir();
            tzen = dir.GetZenith();
            tazi = dir.GetAzimuth();
        }
    }
    log_debug( "(%s) got mostEnergetic inice "
               "maxE=%gGeV zenith=%fdeg azimuth=%fdeg",
               GetName().c_str(), maxenergy/I3Units::GeV,
               tzen/I3Units::deg, tazi/I3Units::deg );

    parabParam.pbfTrZen_ = tzen;
    parabParam.pbfTrAzi_ = tazi;
    grid_.localnet.Invert( tazi, tzen ); // yes, azi and zen must be in this order....
    parabParam.pbfTrOffZen_ = tzen;
    parabParam.pbfTrOffAzi_ = tazi;
    return 0;
}

int I3ParaboloidFitter::GetLogLHFitParams( const I3Frame &f,
                                           I3LogLikelihoodFitPtr fitptr){
    // backup, in case fit fails (should be very rare)
    I3EventHypothesis &eh = *(fitptr->hypothesis_);
    I3Particle &p = *(eh.particle);

    if ( ! fitterCore_ ){
        fitptr->fitparams_->logl_ = -1.0*eventLLH_->GetLogLikelihood( eh );
        fitptr->fitparams_->rlogl_ = fitptr->fitparams_->logl_/ndf_;
        fitptr->fitparams_->ndof_ = ndf_;
        fitptr->fitparams_->nmini_ = 0;
        return 1;
    }

    // set direction to the values of the paraboloid minimum
    I3ParaboloidFitParamsPtr params = 
        boost::dynamic_pointer_cast<I3ParaboloidFitParams>( fitptr->fitparams_ );

    p.SetDir( params->pbfZen_, params->pbfAzi_);
    I3EventHypothesisPtr backup = seedService_->GetCopyPtr(eh);

    // fit
    if ( gridSeedService_ ){
        gridSeedService_->Tweak( eh );
    }
    bool success = fitterCore_->Fit( f, fitptr );

    // catch failure
    if ( ! success ){
        log_warn( "(%s) Refitting vertex of track with paraboloid minimum failed.",
                  GetName().c_str() );
        log_warn( "(%s) This should be very rare.",
                  GetName().c_str() );
        log_warn( "(%s) You should get worried if you see this message too often.",
                  GetName().c_str() );

        // set original hypothesis back
        fitptr->hypothesis_ = backup;
        I3EventHypothesis &ehb = *(fitptr->hypothesis_);

        // fill in llh
        // the llh function value at the paraboloid minimum can of course be
        // slightly different from the fitted minimum from the grid.
        fitptr->fitparams_->logl_ = -1.0*eventLLH_->GetLogLikelihood( ehb );

        // fill in the reduced llh
        assert(ndf_>0); // if this still fails: programming error, submit bug report
        fitptr->fitparams_->rlogl_ = fitptr->fitparams_->logl_/ndf_;
    }

    return 1;
}
