/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3SimpleFitter.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <boost/make_shared.hpp>

#include "gulliver-modules/I3SimpleFitter.h"

#include "dataclasses/I3Vector.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"

#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3LogLikelihoodFit.h"
#include "gulliver/utilities/ordinal.h"

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Logging.h"
#include "icetray/I3Units.h"

I3_MODULE(I3SimpleFitter);

//-----------------------------------------------------------------------------
I3SimpleFitter::I3SimpleFitter(const I3Context& ctx) : I3ConditionalModule(ctx)
{
    AddOutBox("OutBox");

    AddParameter("Minimizer",
                 "Minimizer service to use",
                 minimizer_);

    AddParameter("Parametrization",
                 "Parametrization service to use",
                 parametrization_);

    AddParameter("LogLikelihood",
                 "Log-likelihood service to use",
                 likelihood_);

    AddParameter("SeedService",
                 "Seed service to use",
                 seedService_);

    AddParameter("OutputName",
                 "Name of output I3Particle, and prefix for any fit parameter "
                 "frame objects",
                 "");

    storagePolicy_ = ONLY_BEST_FIT;
    storagePolicyString_ = "OnlyBestFit";
    AddParameter("StoragePolicy",
                 "Select whether you would like to have: "
                 "(1) \"OnlyBestFit\" (default): only a single result (the "
                 "fit with the best likelihood, storing the I3Particle, "
                 "I3LogLikelihoodFitParams and if relevant the nonstd part); "
                 "(2) \"AllFitsAndFitParams\": the best fit PLUS two vectors "
                 "with the fits (I3Particle+I3LogLikelihoodFitParams) for "
                 "each seed; "
                 "(3) \"AllFitsAndFitParamsNotInVectors\": same as before but "
                 "instead of storing the results with all seeds in vectors, "
                 "they are stored individually with an index number appended "
                 "to the fit name; "
                 "(4) \"AllResults\": the best fit PLUS three vectors with "
                 "the full fits (I3Particle+nonstd+I3LogLikelihoodFitParams) "
                 "for each seed (NOT YET IMPLEMENTED); "
                 "(5) \"AllResultsNotInVectors\": same as before but instead "
                 "of storing the results with all seeds in vectors, they are "
                 "stored individually with an index number appended to the "
                 "fit name (NOT YET IMPLEMENTED).",
                 storagePolicyString_);

    nonStdName_ = "";
    AddParameter("NonStdName",
                 "Name to use to store the non-standard part of the event "
                 "hypothesis (if any). Be sure to choose a unique name. "
                 "Default is the fit name + \"Params\".",
                 nonStdName_ );

    traceModeString_ = "None";
    AddParameter("TraceMode",
                 "For detailed debugging of the minimization process, you "
                 "may want to look at the full history of fit parameter "
                 "values that were tried by the minimizer, together with the "
                 "likelihood function values and (if applicable) the "
                 "gradients. Possible option values: \"None\" (no tracing), "
                 "\"All\" (get traces of fits from all available seeds) and "
                 "\"Single\" (only the trace of the best track is kept). In "
                 "case none of the fits converge, only the last trace will be "
                 "reported in \"Single\" mode.",
                 traceModeString_);

    eventNr_ = 0;
    nSeeds_ = 0;
    nSuccessFits_ = 0;
    nSuccessEvents_ = 0;
}

//-----------------------------------------------------------------------------
void I3SimpleFitter::Configure()
{
    GetParameter("Minimizer", minimizer_);
    GetParameter("Parametrization", parametrization_);
    GetParameter("LogLikelihood", likelihood_);
    GetParameter("SeedService", seedService_);
    GetParameter("OutputName", fitName_);
    GetParameter("StoragePolicy", storagePolicyString_);
    GetParameter("NonStdName", nonStdName_);
    GetParameter("TraceMode", traceModeString_);

    if (minimizer_)
    {
        log_debug("(%s) Configured minimizer: \"%s\"",
                  GetName().c_str(), minimizer_->GetName().c_str());
    }
    else
    {
        log_fatal("(%s) Problem with setting minimizer service.",
                  GetName().c_str());
    }

    if (parametrization_)
    {
        log_debug("(%s) Configured parametrization: \"%s\"",
                  GetName().c_str(), parametrization_->GetName().c_str());
    }
    else
    {
        log_fatal("(%s) Problem with setting parametrization service.",
                  GetName().c_str());
    }

    if (likelihood_)
    {
        log_debug("(%s) Configured log-likelihood: \"%s\"",
                  GetName().c_str(), likelihood_->GetName().c_str());
    }
    else
    {
        log_fatal("(%s) Problem with setting likelihood service.",
                  GetName().c_str());
    }

    if (!seedService_)
    {
        log_fatal("(%s) Problem with setting seed service.",
                  GetName().c_str());
    }

    fitterCore_ = boost::make_shared<I3Gulliver>(
        parametrization_, likelihood_, minimizer_, GetName());

    assert(fitterCore_);

    log_debug("(%s) Configured result mode: \"%s\"",
              GetName().c_str(), storagePolicyString_.c_str());

    if (storagePolicyString_ == "OnlyBestFit")
    {
        storagePolicy_ = ONLY_BEST_FIT;
    }
    else if (storagePolicyString_ == "AllFitsAndFitParams")
    {
        storagePolicy_ = ALL_FITS_AND_FITPARAMS_IN_VECTORS;
    }
    else if (storagePolicyString_ == "AllFitsAndFitParamsNotInVectors")
    {
        storagePolicy_ = ALL_FITS_AND_FITPARAMS_NOT_IN_VECTORS;
    }
    else if ((storagePolicyString_ == "AllResults") ||
             (storagePolicyString_ == "AllResultsNotInVectors"))
    {
        storagePolicy_ = (storagePolicyString_ == "AllResults")
            ? ALL_RESULTS_IN_VECTORS : ALL_RESULTS_NOT_IN_VECTORS;

        log_warn("(%s) nonstd vector saving not yet implemented; you will "
                 "only get I3Particles and I3LogLikelihoodFitParams. Use the "
                 "AllFitsAndFitParams storage policy to avoid this warning.",
                 GetName().c_str());
    }
    else
    {
        log_fatal("(%s) Problem with setting result mode to \"%s\".",
                  GetName().c_str(), storagePolicyString_.c_str());
    }

    log_debug("(%s) Configured trace mode: \"%s\"",
              GetName().c_str(), traceModeString_.c_str());

    if (traceModeString_ == "None")
    {
        traceMode_ = TRACE_NONE;
    }
    else if (traceModeString_ == "All")
    {
        traceMode_ = TRACE_ALL;
    }
    else if (traceModeString_ == "Single")
    {
        traceMode_ = TRACE_SINGLE;
    }
    else
    {
        log_fatal("(%s) Problem with setting trace mode to \"%s\".",
                  GetName().c_str(), traceModeString_.c_str());
    }

    if (fitName_ == "")
    {
        log_warn("(%s) Parameter \"OutputName\" was not set. Falling back to "
                 "old behavior and using instance name. Please update your "
                 "scripts, the fallback will be disabled in the next version.",
                 GetName().c_str());

        fitName_= GetName();
    }

    if (nonStdName_ == "")
        nonStdName_ = fitName_ + I3LogLikelihoodFit::NONSTD_SUFFIX;
}

//-----------------------------------------------------------------------------
void I3SimpleFitter::Geometry(I3FramePtr frame)
{
    log_debug("(%s) Welcome to the Geometry method of I3SimpleFitter.",
              GetName().c_str());

    const I3Geometry& geometry = frame->Get<I3Geometry>();
    fitterCore_->SetGeometry(geometry);

    PushFrame(frame, "OutBox");
    log_debug("(%s) Leaving I3SimpleFitter Geometry.", GetName().c_str());
}

//-----------------------------------------------------------------------------
void I3SimpleFitter::Physics(I3FramePtr frame)
{
    ++eventNr_;

    log_debug("(%s) Welcome to the Physics method of SimpleFitter.",
              GetName().c_str());
    log_debug("(%s) This is the %s physics frame.",
              GetName().c_str(), ordinal(eventNr_));

    I3VectorI3ParticlePtr allFits = boost::make_shared<I3VectorI3Particle>();

    I3LogLikelihoodFitParamsVectPtr params =
        boost::make_shared<I3LogLikelihoodFitParamsVect>();

    std::vector<I3VectorDoublePtr> traces;

    unsigned int nseeds = seedService_->SetEvent(*frame);
    log_debug("(%s) Got %d seed(s).", GetName().c_str(), nseeds);

    I3LogLikelihoodFitPtr bestFit;

    if (nseeds == 0)
    {
        log_debug("(%s) No seed(s) available.", GetName().c_str());

        bestFit = boost::make_shared<I3LogLikelihoodFit>(
            seedService_->GetDummy());

        bestFit->hypothesis_->particle->SetFitStatus(I3Particle::MissingSeed);
    }
    else
    {
        nSeeds_ += nseeds;
        bestFit = Fit(frame, nseeds, allFits, params, traces);
    }

    frame->Put(fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX,
               bestFit->hypothesis_->particle);

    if (bestFit->hypothesis_->nonstd)
    {
        frame->Put(nonStdName_, bestFit->hypothesis_->nonstd);
    }

    if (bestFit->minidiagnostics_)
    {
        frame->Put(fitName_ + "_" + minimizer_->GetName(),
                   bestFit->minidiagnostics_);
    }

    if (bestFit->paradiagnostics_)
    {
        frame->Put(fitName_ + "_" + parametrization_->GetName(),
                   bestFit->paradiagnostics_);
    }

    if (bestFit->llhdiagnostics_)
    {
        frame->Put(fitName_ + "_" + likelihood_->GetName(),
                   bestFit->llhdiagnostics_);
    }

    frame->Put(fitName_ + I3LogLikelihoodFit::FITPARAMS_SUFFIX,
               bestFit->fitparams_);

    if (storagePolicy_ == ALL_FITS_AND_FITPARAMS_IN_VECTORS ||
        storagePolicy_ == ALL_RESULTS_IN_VECTORS)
    {
        frame->Put(fitName_ + I3LogLikelihoodFit::PARTICLEVECT_SUFFIX,
                   allFits);
        frame->Put(fitName_ + I3LogLikelihoodFit::FITPARAMSVECT_SUFFIX,
                   params);

        if (storagePolicy_ == ALL_RESULTS_IN_VECTORS)
            log_debug("(%s) nonstd vector saving not yet implemented.",
                      GetName().c_str());
    }

    if (traceMode_ == TRACE_ALL)
    {
        if (traces.empty())
        {
            traces.resize(1);
        }

        for (unsigned int i = 0; i < traces.size(); i++)
        {
            std::ostringstream suffix;
            suffix << "_TRACE" << std::setw(4) << std::setfill('0') << i;
            frame->Put(fitName_ + suffix.str(), traces[i]);
        }
    }

    if (traceMode_ == TRACE_SINGLE)
    {
        frame->Put(fitName_ + "_TRACE", traces[0]);
    }

    PushFrame(frame, "OutBox");
    log_debug("(%s) Leaving I3SimpleFitter Physics.", GetName().c_str());
}

//-----------------------------------------------------------------------------
I3LogLikelihoodFitPtr I3SimpleFitter::Fit(
    I3FramePtr frame, unsigned int nseeds,
    I3VectorI3ParticlePtr allFits,
    I3LogLikelihoodFitParamsVectPtr params,
    std::vector<I3VectorDoublePtr>& traces)
{
    std::vector<I3LogLikelihoodFit> goodFits;
    for (unsigned int i = 0; i < nseeds; ++i)
    {
        I3EventHypothesis seed = seedService_->GetSeed(i);
        assert(seed.particle->GetShape() != I3Particle::Null);

        I3LogLikelihoodFitPtr fit = Fit(frame, seed);

        if (fit->hypothesis_->particle->GetFitStatus() == I3Particle::OK)
        {
            goodFits.push_back(*fit);
            ++nSuccessFits_;
        }

        if ((traceMode_ == TRACE_SINGLE && i == 0) ||
                traceMode_ == TRACE_ALL)
        {
            traces.push_back(fitterCore_->GetTrace());
        }

        std::ostringstream prefix;
        prefix << i;

        switch (storagePolicy_)
        {
            case ONLY_BEST_FIT:
                break;
            case ALL_RESULTS_IN_VECTORS:
                // TODO not implemented yet
            case ALL_FITS_AND_FITPARAMS_IN_VECTORS:
                allFits->push_back(*(fit->hypothesis_->particle));
                params->push_back(*(fit->fitparams_));
                break;
            case ALL_RESULTS_NOT_IN_VECTORS:
                if (fit->hypothesis_->nonstd)
                {
                    frame->Put(fitName_ + prefix.str() +
                               I3LogLikelihoodFit::NONSTD_SUFFIX,
                               fit->hypothesis_->nonstd);
                }
            case ALL_FITS_AND_FITPARAMS_NOT_IN_VECTORS:
                frame->Put(fitName_ + prefix.str() +
                           I3LogLikelihoodFit::PARTICLE_SUFFIX,
                           fit->hypothesis_->particle);

                frame->Put(fitName_ + prefix.str() +
                           I3LogLikelihoodFit::FITPARAMS_SUFFIX,
                           fit->fitparams_);
        }
    }

    I3LogLikelihoodFitPtr bestFit;

    if (goodFits.size() > 0)
    {
        std::sort(goodFits.begin(), goodFits.end());

        if (storagePolicy_ == ONLY_BEST_FIT)
        {
            // shallow copy works just fine here
            bestFit = boost::make_shared<I3LogLikelihoodFit>(
                goodFits.front());
        }
        else
        {
            // we need a deep copy, otherwise the I3Writer will complain
            // about writing the same object twice: once as the best fit
            // and once as one of the results with several seeds
            I3EventHypothesis bestHypothesis =
                seedService_->GetCopy(*(goodFits.begin()->hypothesis_));

            bestFit = boost::make_shared<I3LogLikelihoodFit>(
                bestHypothesis, *(goodFits.begin()->fitparams_));
        }

        const I3LogLikelihoodFit& worstFit = goodFits.back();

        log_debug(
            "(%s) Got %zu solution(s): "
            "best fit has llh=%.2f and shape=%s; "
            "worst fit has llh=%.2f, status=%s, and shape=%s.",
            GetName().c_str(),
            goodFits.size(),
            bestFit->fitparams_->logl_,
            bestFit->hypothesis_->particle->GetShapeString().c_str(),
            worstFit.fitparams_->logl_,
            worstFit.hypothesis_->particle->GetFitStatusString().c_str(),
            worstFit.hypothesis_->particle->GetShapeString().c_str());

        ++nSuccessEvents_;
    }
    else
    {
        bestFit = boost::make_shared<I3LogLikelihoodFit>(
            seedService_->GetSeed(0));

        bestFit->hypothesis_->particle->SetFitStatus(
            I3Particle::FailedToConverge);
    }

    return bestFit;
}

//-----------------------------------------------------------------------------
I3LogLikelihoodFitPtr I3SimpleFitter::Fit(I3FramePtr frame,
                                          const I3EventHypothesis& seed)
{
    I3LogLikelihoodFitPtr fit = boost::make_shared<I3LogLikelihoodFit>(seed);
    const I3Particle& particle = *(fit->hypothesis_->particle);

    log_debug("(%s) Reconstruct using seed x=%.2fm, y=%.2fm, z=%.2fm, "
              "t=%.2fns, theta=%.2fdeg, phi=%.2fdeg, energy=%.2eGeV, "
              "length=%.2fm, and status=%s.",
              GetName().c_str(),
              particle.GetPos().GetX()/I3Units::m,
              particle.GetPos().GetY()/I3Units::m,
              particle.GetPos().GetZ()/I3Units::m,
              particle.GetTime()/I3Units::ns,
              particle.GetDir().GetZenith()/I3Units::degree,
              particle.GetDir().GetAzimuth()/I3Units::degree,
              particle.GetEnergy()/I3Units::GeV,
              particle.GetLength()/I3Units::m,
              particle.GetFitStatusString().c_str());

    if (traceMode_ != TRACE_NONE)
    {
        fitterCore_->Trace();
    }

    bool success = fitterCore_->Fit(*frame, fit);

    log_debug("(%s) %s: x=%.2fm, y=%.2fm, z=%.2fm, t=%.2fns theta=%.2fdeg, "
              "phi=%.2fdeg, energy=%.2eGeV length=%.2fm, status=%s,",
              GetName().c_str(),
              success ? "Success" : "Failed",
              particle.GetPos().GetX()/I3Units::m,
              particle.GetPos().GetY()/I3Units::m,
              particle.GetPos().GetZ()/I3Units::m,
              particle.GetTime()/I3Units::ns,
              particle.GetDir().GetZenith()/I3Units::degree,
              particle.GetDir().GetAzimuth()/I3Units::degree,
              particle.GetEnergy()/I3Units::GeV,
              particle.GetLength()/I3Units::m,
              particle.GetFitStatusString().c_str());

    log_debug("(%s) llh=%.2f, rllh=%.2f, ndof=%d, nmini=%d",
              GetName().c_str(),
              fit->fitparams_->logl_,
              fit->fitparams_->rlogl_,
              fit->fitparams_->ndof_,
              fit->fitparams_->nmini_);

    // fit status has already been set correctly by fitterCore_
    assert(success ^ (particle.GetFitStatus() != I3Particle::OK));

    return fit;
}

//-----------------------------------------------------------------------------
void I3SimpleFitter::Finish()
{
    log_info("(%s) Finishing after %s physics frame.",
             GetName().c_str(), ordinal(eventNr_));

    if ((eventNr_ == 0) || ( nSuccessEvents_ == 0))
    {
        log_warn("(%s) Saw no events with at least one good fit.",
                 GetName().c_str());
    }

    log_info("(%s) %d seeds, %d good fits, %d events with at least one good "
             "fit.", GetName().c_str(), nSeeds_, nSuccessFits_,
             nSuccessEvents_);
}
