/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3LaputopFitter.cxx
 * @version $Revision$
 * @date $Date$
 * @author kath
 */

/***
DORKY -> The Story of the LaputopFitter as set in a story described by Kath to myself, Emily Dvorak.
emily.dvorak@mines.sdsmt.edu	06.27.2017
***/


// tools, standard lib stuff
#include "icetray/IcetrayFwd.h"
#include <string>
#include <list>
#include <functional>

// my header
#include "toprec/I3LaputopFitter.h"

// IceTray stuff
I3_MODULE(I3LaputopFitter);
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Bool.h"

// dataclasses & gulliver
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3ParametrizationBase.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3MinimizerBase.h"
#include "gulliver/I3EventHypothesis.h"
#include "gulliver/utilities/ordinal.h"

#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "phys-services/I3Calculator.h"

#include "toprec/I3LaputopLikelihood.h"
#include <recclasses/I3LaputopParams.h>
#include "toprec/SnowCorrectionDiagnostics.h"  // <--- this one needed in order to write out diagnostics
//----------------------------------------------------------------------------

namespace {
    void FillParams(I3LaputopParamsPtr params,
                    const I3EventHypothesis& hypoth,
                    const I3LogLikelihoodFitParams& fitparams,
                    I3LaputopLikelihoodPtr lapuLlh) {
      // The Llh is *not* the negLlh, while logl_ is the negLlh (I3Gulliver.cxx L.306), hence the minus
      params->FillFromI3Particle(*(hypoth.particle));
      params->logLikelihood_ = -fitparams.logl_;
      params->ndf_ = fitparams.ndof_;
      params->nMini_ = fitparams.nmini_;
      double chi2_charge(0.), chi2_time(0.);
      lapuLlh->CalcChi2(hypoth, chi2_charge, chi2_time);  // returns chi2/ndf for charge and time(if fCurv) !!
      params->chi2_LDF_ = chi2_charge;
      params->chi2_Time_ = chi2_time;
      if (lapuLlh->GetFunction() == "dlp")
        params->typeLDF_ = Laputop::LDF::DLP;
      else
        params->typeLDF_ = Laputop::LDF::None;
      if (lapuLlh->GetCurvature() == "gausspar")
        params->typeFrontDelay_ = Laputop::FrontDelay::GaussParabola;
      else
        params->typeFrontDelay_ = Laputop::FrontDelay::None;
      params->typeEnergy_ = Laputop::Energy::IC73SpectrumPaper;
    }
}

I3LaputopFitter::I3LaputopFitter(const I3Context& ctx) : I3ConditionalModule(ctx){

    AddOutBox("OutBox");

    // initialize counters
    eventNr_ = 0;
    nSeeds_ = 0;
    nSuccessFits_ = 0;
    nSuccessEvents_ = 0;

    // More defaults: must have at least one param/likelihood, one step
    nSteps_ = 1;
    parName2_ = "";
    parName3_ = "";
    parName4_ = "";
    
    // options

    AddParameter( "Minimizer", "Name of minimizer service to use",
	          miniName_ );

    AddParameter( "NSteps", "Number of steps (from 1 to 4)", nSteps_ );

    AddParameter( "Parametrization1", "Parametrization service (1st iteration)", parName1_ );
    AddParameter( "Parametrization2", "Parametrization service (2nd iteration)", parName2_ );
    AddParameter( "Parametrization3", "Parametrization service (3rd iteration)", parName3_ );
    AddParameter( "Parametrization4", "Parametrization service (4th iteration)", parName4_ );

    AddParameter( "LogLikelihoodService", "LogLikelihood service to use (1st iteration", llhName_ );

    LDFFunctions_.clear();
    AddParameter( "LDFFunctions",
		  "List of LDF functions for steps \n"
		  "Available: \"dlp\"", LDFFunctions_);

    CurvFunctions_.clear();
    AddParameter( "CurvFunctions",
		  "List of Curvature functions for steps \n"
		  "Available: \"gausspar\" or (empty string)", CurvFunctions_);

    staticRCuts_.clear();
    AddParameter( "StaticCoreCutRadii","List of radii at which to do a static (permanent) cut at the start of each step",staticRCuts_);

    staticTCuts_.clear();
    AddParameter( "StaticTimeCutResiduals","List of time residuals beyond which to do a static (permanent) cut at the start of each step",staticTCuts_);

    AddParameter( "SeedService", "Name of seed service",
	          seedServiceName_ );

    // different storage policies
    storagePolicy_ = ONLY_BEST_FIT;
    storagePolicyString_ = "OnlyBestFit";
    AddParameter( "StoragePolicy",
                  "Select whether you'd like to have\n"
                  "(1) \"OnlyBestFit\" (default) only a single result\n"
                  "(the fit with the best likelihood, storing the I3Particle,\n"
                  "I3LogLikelihoodFitParams and if relevant the nonstd part); or\n"
  		  "(2) \"Intermediate\" : Store result of intermediate likelihoods \n"
 		  "",
	          storagePolicyString_ );
}

//----------------------------------------------------------------------------
void I3LaputopFitter::Configure(){

    // default fitName (none of the other options have defaults yet)
    fitName_ = GetName();
    GetParameter( "NSteps", nSteps_ );
    GetParameter( "Minimizer", miniName_ );
    GetParameter( "Parametrization1", parName1_ );
    GetParameter( "Parametrization2", parName2_ );
    GetParameter( "Parametrization3", parName3_ );
    GetParameter( "Parametrization4", parName4_ );
    GetParameter( "LogLikelihoodService", llhName_ ); 
    GetParameter( "LDFFunctions",LDFFunctions_);
    GetParameter( "CurvFunctions",CurvFunctions_);
    GetParameter( "StaticCoreCutRadii",staticRCuts_);
    GetParameter( "StaticTimeCutResiduals",staticTCuts_);
    
    GetParameter( "SeedService", seedServiceName_ );
    GetParameter( "StoragePolicy", storagePolicyString_ );

    // If the user left these blank, fill these lists with the right number of zeros
    // So the code won't complain later.
    if (staticRCuts_.size() == 0)
      for (unsigned int i=0; i<nSteps_; i++) staticRCuts_.push_back(0);
    if (staticTCuts_.size() == 0)
      for (unsigned int i=0; i<nSteps_; i++) staticTCuts_.push_back(0);


    bool miniOK = context_.Has< I3MinimizerBase >( miniName_ );
    bool par1OK = context_.Has< I3ParametrizationBase >( parName1_ );
    bool llhOK = context_.Has< I3EventLogLikelihoodBase >( llhName_ );
    bool par2OK = 1;
    if (nSteps_ > 1) {
      par2OK = context_.Has< I3ParametrizationBase >( parName2_ );
    }
    bool par3OK = 1;
    if (nSteps_ > 2) {
      par3OK = context_.Has< I3ParametrizationBase >( parName3_ );
    }
    bool par4OK = 1;
    if (nSteps_ > 3) {
      par4OK = context_.Has< I3ParametrizationBase >( parName4_ );
    }

    if (nSteps_ > 4) log_fatal("Hey, nSteps cannot be greater than four. %u", nSteps_);
    if (nSteps_ < 1) log_fatal("Hey, nSteps cannot be less than one. %u", nSteps_);

    bool seedOK = context_.Has< I3SeedServiceBase >( seedServiceName_ );
    bool modeOK = ( (storagePolicyString_ == "OnlyBestFit") ||
                    (storagePolicyString_ == "AllFitsAndFitParams") ||
                    (storagePolicyString_ == "AllFitsAndFitParamsNotInVectors") ||
                    (storagePolicyString_ == "AllResults") ||
                    (storagePolicyString_ == "AllResultsNotInVectors") ||
 		    (storagePolicyString_ == "Intermediate"));

    log_info( "(%s) minimizer from \"%s\": %s",
              GetName().c_str(), miniName_.c_str(), (miniOK?"OK":"PROBLEM") );
    log_info( "(%s) parametrization1 from \"%s\": %s",
              GetName().c_str(), parName1_.c_str(), (par1OK?"OK":"PROBLEM") );
    log_info( "(%s) loglikelihood from \"%s\": %s",
              GetName().c_str(), llhName_.c_str(), (llhOK?"OK":"PROBLEM") );
    if (nSteps_ > 1) {
      log_info( "(%s) parametrization2 from \"%s\": %s",
		GetName().c_str(), parName2_.c_str(), (par2OK?"OK":"PROBLEM") );
    }
    if (nSteps_ > 2) {
      log_info( "(%s) parametrization3 from \"%s\": %s",
		GetName().c_str(), parName3_.c_str(), (par3OK?"OK":"PROBLEM") );
    }
    if (nSteps_ > 3) {
      log_info( "(%s) parametrization4 from \"%s\": %s",
              GetName().c_str(), parName4_.c_str(), (par4OK?"OK":"PROBLEM") );
    }
    log_info( "(%s) seeds from \"%s\": %s",
              GetName().c_str(), seedServiceName_.c_str(), (seedOK?"OK":"PROBLEM") );
    log_info( "(%s) result mode \"%s\": %s",
              GetName().c_str(), storagePolicyString_.c_str(), (modeOK?"OK":"PROBLEM") );

    if ( ! ( miniOK && par1OK && par2OK && par3OK && par4OK 
	     && llhOK && seedOK) ){
        // more than one could be wrong, so we call fatal only at the end.
        if ( !miniOK ) log_error( "(%s) problem with getting minimizer \"%s\"", GetName().c_str(), miniName_.c_str() );
        if ( !par1OK ) log_error( "(%s) problem with getting parametrization1 \"%s\"", GetName().c_str(), parName1_.c_str() );
        if ( !par2OK ) log_error( "(%s) problem with getting parametrization2 \"%s\"", GetName().c_str(), parName2_.c_str() );
        if ( !par3OK ) log_error( "(%s) problem with getting parametrization3 \"%s\"", GetName().c_str(), parName3_.c_str() );
        if ( !par4OK ) log_error( "(%s) problem with getting parametrization4 \"%s\"", GetName().c_str(), parName4_.c_str() );
        if ( !llhOK )  log_error( "(%s) problem with getting llh service \"%s\"", GetName().c_str(), llhName_.c_str() );
        if ( !seedOK ) log_error( "(%s) problem with getting seed service \"%s\"", GetName().c_str(), seedServiceName_.c_str() );
        if ( !modeOK ) log_error( "(%s) problem with setting result mode to \"%s\"", GetName().c_str(), storagePolicyString_.c_str() );
        log_fatal("(%s) fix it!", GetName().c_str() );
        return;
    }


    /***
    DORKY -> The Gulliver is the "Boss" of all the "Servants"  or services.  These are the parameterization and Likliehood services.  
    The boss telss the servants their tasks and gets them organized.
    ***/

    fitterCore1_ = I3GulliverPtr( 
        new I3Gulliver( 
            context_.Get< I3ParametrizationBasePtr >( parName1_ ),
            context_.Get< I3EventLogLikelihoodBasePtr >( llhName_ ),
            //context_.Get< boost::shared_ptr<I3LaputopLikelihood> >( llhName1_ ),
            context_.Get< I3MinimizerBasePtr >( miniName_ ),
            GetName()
        )
    );
    if (nSteps_ > 1) 
    fitterCore2_ = I3GulliverPtr( 
        new I3Gulliver( 
            context_.Get< I3ParametrizationBasePtr >( parName2_ ),
	    context_.Get< I3EventLogLikelihoodBasePtr >( llhName_ ),
            context_.Get< I3MinimizerBasePtr >( miniName_ ),
            GetName()
        )
    );
    if (nSteps_ > 2)
    fitterCore3_ = I3GulliverPtr( 
        new I3Gulliver( 
            context_.Get< I3ParametrizationBasePtr >( parName3_ ),
            context_.Get< I3EventLogLikelihoodBasePtr >( llhName_ ),
            context_.Get< I3MinimizerBasePtr >( miniName_ ),
            GetName()
        )
    );
    if (nSteps_ > 3)
    fitterCore4_ = I3GulliverPtr( 
        new I3Gulliver( 
            context_.Get< I3ParametrizationBasePtr >( parName4_ ),
            context_.Get< I3EventLogLikelihoodBasePtr >( llhName_ ),
            context_.Get< I3MinimizerBasePtr >( miniName_ ),
            GetName()
	 )
    );
    

    if (storagePolicyString_ == "OnlyBestFit"){
        storagePolicy_ = ONLY_BEST_FIT;
    } else if (storagePolicyString_ == "AllFitsAndFitParams"){
        storagePolicy_ = ALL_FITS_AND_FITPARAMS_IN_VECTORS;
    } else if (storagePolicyString_ == "AllFitsAndFitParamsNotInVectors"){
        storagePolicy_ = ALL_FITS_AND_FITPARAMS_NOT_IN_VECTORS;
    } else if (storagePolicyString_ == "Intermediate"){
        storagePolicy_ = INTERMEDIATE;
    } else if ( (storagePolicyString_ == "AllResults") ||
                (storagePolicyString_ == "AllResultsNotInVectors") ){
        storagePolicy_ = (storagePolicyString_ == "AllResults")
            ? ALL_RESULTS_IN_VECTORS
            : ALL_RESULTS_NOT_IN_VECTORS;
        log_warn( "(%s) nonstd vector saving not yet implemented",
                  GetName().c_str() );
        log_warn( "(%s) you'll only get I3Particles and "
                  "I3LogLikelihoodFitParams", GetName().c_str() );
        log_warn( "(%s) Use the AllFitsAndFitParams storage policy to avoid "
                  "this warning", GetName().c_str() );
    } else {
        log_fatal("impossible");
    }

    seedService_ = context_.Get< I3SeedServiceBasePtr >( seedServiceName_ );
    assert( seedService_ );

    if((nSteps_ != LDFFunctions_.size()) || (nSteps_ != CurvFunctions_.size()))
      log_fatal("Number of LDF functions or Curv Functions is different from the number of Steps!");

    if((nSteps_ != staticRCuts_.size()) || (nSteps_ != staticTCuts_.size()))
      log_fatal("Number of Radius or Timing cuts is different from the number of Steps!");

    lapuLlhService_ = boost::dynamic_pointer_cast<I3LaputopLikelihood> (context_.Get<I3EventLogLikelihoodBasePtr> (llhName_));
    assert(lapuLlhService_);
}

void I3LaputopFitter::Geometry(I3FramePtr frame){
  if(!frame->Has("I3Geometry")) 
    log_fatal("No Geometry in Frame!");
  PushFrame( frame, "OutBox" );
}

void I3LaputopFitter::Calibration(I3FramePtr frame){
  if(!frame->Has("I3Calibration")) 
    log_fatal("No Calibration in Frame!");
  PushFrame( frame, "OutBox" );
}

void I3LaputopFitter::DetectorStatus(I3FramePtr frame){
  if(!frame->Has("I3DetectorStatus")) 
    log_fatal("No DetectorStatus in Frame!");
  PushFrame( frame, "OutBox" );
}




//----------------------------------------------------------------------------
void I3LaputopFitter::Physics(I3FramePtr frame){

    ++eventNr_;
    log_debug( "(%s) Welcome to the physics method of LaputopFitter!",
               GetName().c_str());
    log_debug( "(%s) This is the %s physics frame.",
               GetName().c_str(), ordinal(eventNr_));
    assert(fitterCore1_);
    if (nSteps_ > 1) assert(fitterCore2_);
    if (nSteps_ > 2) assert(fitterCore3_);
    if (nSteps_ > 3) assert(fitterCore4_);
    assert(seedService_);

    // containers to store all fits in the frame (if wanted)
    I3VectorI3ParticlePtr allfits(new I3VectorI3Particle);
    I3LogLikelihoodFitParamsVectPtr allparams(new I3LogLikelihoodFitParamsVect);

    // get/set seed
    /***
    DORKY -> the seedService SetEvent sets up all the pulses for use in calculations.  The seedService works closely with the boss Gulliver.
    ***/
    unsigned int nseeds = seedService_->SetEvent(*frame);
    log_debug( "(%s) got %d seed(s)", GetName().c_str(), nseeds );
    
    if(nseeds > 1 && storagePolicy_ == INTERMEDIATE){
      log_fatal("More than one seed and you want to store all intermediate fits? Will be a mess! \n"
                "Need to be implemented better then! Please change one of the options (1 seed or other storage policy)");
    }
    /***
    DORKY -> Here we set up the houses where the servants do their jobs.  These will be were the successful jobs can be accessed from thei house.
    ***/
    I3EventHypothesis hypothesis( (nseeds == 0) ? seedService_->GetDummy( )
                                         : seedService_->GetSeed( 0 ) );
    I3LogLikelihoodFitPtr storefit( new I3LogLikelihoodFit(hypothesis) );

    // do business
    if ( nseeds == 0 ){
        log_debug( "(%s) no seed available...", GetName().c_str() );
        storefit->hypothesis_->particle->SetFitStatus( I3Particle::MissingSeed ); // also means BAD seed
    } else {
        // this else claus is getting too long.
        // it should be broken up into methods.
        nSeeds_ += nseeds;
        std::list< I3LogLikelihoodFit > solutions;
        for ( unsigned int iseed = 0; iseed < nseeds; ++iseed ){
	    /***
	    DORKY -> Get everyone started. 
	    ***/
            I3EventHypothesis seed = seedService_->GetSeed( iseed );
            assert( seed.particle->GetShape() != I3Particle::Null );
            I3LogLikelihoodFitPtr fitptr( new I3LogLikelihoodFit(seed) );
            assert( fitptr->hypothesis_->particle->GetShape() != I3Particle::Null );

            // convenience refs
            const I3Particle &p = *(fitptr->hypothesis_->particle);
            const I3Position pos = p.GetPos();
            const I3Direction dir = p.GetDir();

            log_debug( "(%s) starting reco with seed x=%.1fm y=%.1fm z=%.1fm t=%.1fns "
                       "theta=%.1fdeg phi=%.1fdeg energy=%.2fTeV length=%.1fm fitstat=%s(%d)",
                       GetName().c_str(),
                       pos.GetX()/I3Units::m, pos.GetY()/I3Units::m, pos.GetZ()/I3Units::m,
                       p.GetTime()/I3Units::ns,
                       dir.GetZenith()/I3Units::degree, dir.GetAzimuth()/I3Units::degree,
                       p.GetEnergy()/I3Units::TeV,
                       p.GetLength()/I3Units::m,
                       p.GetFitStatusString().c_str(),
                       p.GetFitStatus() );


	    // This loop will handle all "N" steps, since they are all basically identical.
	    // Modify one, and modify them all!
	    // In the future, this code could be used to handle an arbitrary number of steps.
	    I3GulliverPtr fitterCores[4] = { fitterCore1_, fitterCore2_, fitterCore3_, fitterCore4_ };
	    std::string names[4] = { "1", "2", "3", "4" };
	    bool success_ultimate = 0;

	    for (unsigned int thisStep = 0; thisStep < nSteps_; thisStep++) {
	      if (thisStep==0 || success_ultimate) {
	      
	      // Setting and checking stuff for new llh service method
	      lapuLlhService_->SetFunction(LDFFunctions_.at(thisStep));
	      lapuLlhService_->SetCurvature(CurvFunctions_.at(thisStep));
	      log_debug("LDF for %s-th iter : %s", names[thisStep].c_str(), (lapuLlhService_->GetFunction()).c_str());
	      log_debug("Curv for %-sth iter : %s (empty means No curv)", names[thisStep].c_str(), (lapuLlhService_->GetCurvature().c_str()));

	      // Similarly for the static (permanent) cutting
	      // Note: this will have no effect for the 1st step, because the hits have not been loaded into
	      // the likelihood service yet...
	      /* KR experiments with removing this whole thing
	      if(staticRCuts_.at(0) >0 || staticTCuts_.at(0) > 0){ 
		I3EventHypothesis copyforcutting = seedService_->GetCopy(*(fitptr->hypothesis_) );
		if (staticRCuts_.at(thisStep) > 0) {
		  lapuLlhService_->CutCorePulses(copyforcutting, staticRCuts_.at(thisStep));
		}
		if (staticTCuts_.at(thisStep) > 0) {
		  lapuLlhService_->CutBadTimingPulses(copyforcutting, staticTCuts_.at(thisStep)); 
		}
	      }
	      */

	      // Do the fitting!
	      bool successi = fitterCores[thisStep]->Fit( *frame, fitptr );
	      /***
		DORKY -> Here is the fitting.  A servant goes and calculates the llh of an event.  
		This is a loop where the servant keeps getting a llh until it is good enough for the event.  
		This is decided by the boss, a minimizer group like Gulliver? 
	      ***/
	      
	      // be noisy about the outcome (or not)
	      const I3Position newpos = fitptr->hypothesis_->particle->GetPos();
	      const I3Direction newdir = fitptr->hypothesis_->particle->GetDir();
	      
	      log_debug( "(%s) %s x=%f y=%f z=%f t=%f "
			 "theta=%.1fdeg phi=%.1fdeg energy=%.3g GeV length=%fm fitstat=%s(%d)",
			 GetName().c_str(),
			 (successi? "bingo!" : "bummer!"),
			 newpos.GetX()/I3Units::m, newpos.GetY()/I3Units::m, newpos.GetZ()/I3Units::m,
			 p.GetTime()/I3Units::ns,
			 newdir.GetZenith()/I3Units::degree, newdir.GetAzimuth()/I3Units::degree,
			 p.GetEnergy()/I3Units::GeV,
			 p.GetLength()/I3Units::m,
			 p.GetFitStatusString().c_str(),
			 p.GetFitStatus() );
	      
	      log_debug( "(%s) %s llh=%f rllh=%f ndof=%d nmini=%d",
			 GetName().c_str(),
			 (successi? "bingo!" : "bummer!"),
			 fitptr->fitparams_->logl_,
			 fitptr->fitparams_->rlogl_,
			 fitptr->fitparams_->ndof_,
			 fitptr->fitparams_->nmini_ );
	      
	      // fit status has already been set correctly by fitterCore_
	      assert( successi ^ (p.GetFitStatus() != I3Particle::OK ) );
	      success_ultimate = successi;

	      if(storagePolicy_ == INTERMEDIATE){
		I3EventHypothesis deep_copy = seedService_->GetCopy(*(fitptr->hypothesis_) );
		// store the intermediate fit
		frame->Put( fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX + "_" + names[thisStep],
			    deep_copy.particle );
		
		if (deep_copy.nonstd){
		  I3LaputopParamsPtr params = boost::dynamic_pointer_cast<I3LaputopParams>(deep_copy.nonstd);
                  FillParams(params, deep_copy, *(fitptr->fitparams_), lapuLlhService_);

		  frame->Put(fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX + "_" + names[thisStep] + I3LogLikelihoodFit::NONSTD_SUFFIX,   // where NONSTD_SUFFIX is "Params"
			     deep_copy.nonstd);  // Has to be reconame+"Params" otherwise topeventbrowser can't find it
		}
	      }
	    }
	    }  // end of loop over the steps


	    if ( success_ultimate ){
	      ++nSuccessFits_;
	    }    
	    solutions.push_back(*fitptr);  // Independent of whether the solution was good or bad, we have the info in the fitptr

        }// End loop over nseeds


	//--------- STORAGE ----------------------

	// At the moment, this code is NOT equipped to deal with storing the results from multiple seeds.
	// It will only store the "best one" (and optionally, the intermediate fits which led to the "best one").
	// If you're an ambitious code-writer and want to take on the challenge, look to I3SimpleFitter for
	// inspiration...

	if ( solutions.size() > 0 ){ 
	  ++nSuccessEvents_;
	  // deep copy, because we already have written this one if INTERMEDIATE, avoid conflicts
	  I3EventHypothesis besthypo = seedService_->GetCopy(*(solutions.begin()->hypothesis_));
	  storefit = I3LogLikelihoodFitPtr( new I3LogLikelihoodFit( besthypo,
	  							    *(solutions.begin()->fitparams_) ) );

	  // KR (Mar 2014) store the diagnostics too!  This is NOT a deep copy!  So we're going to do it only once, for the best fit.
	  if (solutions.begin()->llhdiagnostics_) 
	    storefit->llhdiagnostics_ = solutions.begin()->llhdiagnostics_;

	  log_debug( "(%s) got as final solution, "
		     "best fit has llh=%f and shape=%s ",
		     GetName().c_str(),
		     storefit->fitparams_->logl_,
		     storefit->hypothesis_->particle->GetShapeString().c_str());

	  log_debug("Checking Diagnostics... does solutions have one? %d", (bool)(solutions.begin()->llhdiagnostics_));
	  log_debug("Checking Diagnostics... does storefit have one? %d", (bool)(storefit->llhdiagnostics_));
	}

    }// End if nseeds is NOT zero

    // Some remarks about storing results :
    // Topeventbrowser explicitly looks for reconame_(I3Particle) + "Params"
    // => nonStdName_ should ALWAYS be reconame_ + "Params"
    // "Params" = NONSTD_SUFFIX

    // store the best fit
    frame->Put( fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX,
                storefit->hypothesis_->particle );

    if (storefit->hypothesis_->nonstd){
      I3LaputopParamsPtr params = boost::dynamic_pointer_cast<I3LaputopParams>(storefit->hypothesis_->nonstd);
      FillParams(params, *(storefit->hypothesis_),
        *(storefit->fitparams_), lapuLlhService_);

      frame->Put(fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX + I3LogLikelihoodFit::NONSTD_SUFFIX,  // for now this is fine, TODO : cleanup part 2
		 storefit->hypothesis_->nonstd);  // Has to be reconame+"Params" otherwise topeventbrowser can't find it
    }

    if ( storefit->minidiagnostics_){
        frame->Put( fitName_+"_"+miniName_, storefit->minidiagnostics_ );
    }
    if ( storefit->paradiagnostics_){
      // HEY KATH, SHOULD THERE BE ALL FOUR OF THEM HERE?
        frame->Put( fitName_+"_"+parName1_, storefit->paradiagnostics_ );
    }
    if ( storefit->llhdiagnostics_){
      // HEY KATH, SHOULD THERE BE ALL FOUR OF THEM HERE?
        log_debug("I will be writing a LLH Diagnostics frame object here! %s", (fitName_+"SnowDiagnostics").c_str());
	log_debug("Checking it: tstage is %f", boost::dynamic_pointer_cast<SnowCorrectionDiagnostics>(storefit->llhdiagnostics_)->tstage);
        frame->Put( fitName_+"SnowDiagnostics", storefit->llhdiagnostics_ );
    } 

    //Fit procedure Done : reset:
    lapuLlhService_->ResetInput();

    PushFrame( frame, "OutBox" );
    log_debug( "(%s) Leaving I3LaputopFitter Physics().",
               GetName().c_str() );
}

void I3LaputopFitter::Finish(){
    if ( (eventNr_ > 0) && ( nSuccessEvents_ > 0 ) ){
        log_info( "(%s) finishing after %s physics frame",
                  GetName().c_str(), ordinal(eventNr_) );
        log_info( "(%s) %d seeds, %d good fits, %d events with at least one good fit",
                  GetName().c_str(), nSeeds_, nSuccessFits_, nSuccessEvents_ );
    } else {
        log_warn( "(%s) finishing after %s physics frame",
                  GetName().c_str(), ordinal(eventNr_) );
        log_warn( "(%s) %d seeds, %d good fits, %d events with at least one good fit",
                  GetName().c_str(), nSeeds_, nSuccessFits_, nSuccessEvents_ );
    }
}


