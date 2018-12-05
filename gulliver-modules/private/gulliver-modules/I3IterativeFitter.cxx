/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3IterativeFitter.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include <boost/make_shared.hpp>
#include <gsl/gsl_qrng.h>

#include "gulliver-modules/I3IterativeFitter.h"

#include "dataclasses/I3Direction.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Logging.h"
#include "icetray/I3Units.h"

#include "gulliver/I3EventHypothesis.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3LogLikelihoodFit.h"
#include "gulliver/utilities/ordinal.h"

#include "phys-services/I3RandomService.h"

I3_MODULE(I3IterativeFitter);

static const char* minimizer_optionname = "Minimizer";
static const char* param_optionname = "Parametrization";
static const char* llh_optionname = "LogLikelihood";
static const char* seedservice_optionname = "SeedService";
static const char* tweakservice_optionname = "IterationTweakService";
static const char* randomservice_optionname = "RandomService";
static const char* coszenithrange_optionname = "CosZenithRange";
static const char* niter_optionname = "NIterations";

// TODO not implemented yet
// static const char* minangdiff_optionname = "MinimumAngularDifference";
// static const char* nsol_optionname = "NmaxSolutions";

static const double pi = std::acos(-1.);

//-----------------------------------------------------------------------------
I3IterativeFitter::I3IterativeFitter(const I3Context& ctx)
    : I3ConditionalModule(ctx)
{
    AddOutBox("OutBox");

    AddParameter(minimizer_optionname,
                 "Minimizer service to use",
                 minimizer_);

    AddParameter(param_optionname,
                 "Parametrization service to use",
                 parametrization_);

    AddParameter(llh_optionname,
                 "Log-likelihood service to use",
                 likelihood_);

    AddParameter(seedservice_optionname,
                 "Seed service to use (providing first guess track, with "
                 "sensible vertex tweaking). If you do not set a separate "
                 "\"IterationTweakService\" then this service will also be "
                 "used for the iterations. That may be fine for crude seeding "
                 "with the old line fit. But if your input seed has reliable "
                 "vertex time, for instance with the improved linefit or with "
                 "another likelihood fit, then you may want use two different "
                 "seed services: one for the first guess track *without* "
                 "vertex time tweaking, one for the iterations *with* vertex "
                 "time tweaking.",
                 seedService_);

    AddParameter(tweakservice_optionname,
                 "Seed service to use for vertex corrections done on "
                 "iteration seeds with pseudo-random direction. Default "
                 "behavior: use the seed service as no vertex correction for "
                 "grid points.",
                 tweakService_);

    nonStdName_ = "";
    AddParameter("NonStdName",
                 "Name to use to store the nonstandard part of the event "
                 "hypothesis (if any). Be sure to choose a unique name. "
                 "Default is the fitname + \"Params\".",
                 nonStdName_);

    AddParameter(randomservice_optionname,
                 "Name of random service: \"SOBOL\", \"NIEDERREITER2\", or "
                 "I3RandomService. The current implementation supports only "
                 "the I3Context-based approach in case of the latter.",
                 randomServiceName_);

    AddParameter("OutputName",
                 "Name of output I3Particle and prefix for any fit parameter "
                 "frame objects",
                 "");

    cosZenithRange_.resize(2);
    cosZenithRange_[0] = -1.;
    cosZenithRange_[1] = 1.;
    AddParameter(coszenithrange_optionname,
                 "Range of cos zenith (vector with two doubles), for "
                 "generating random directions. Default is [-1, 1].",
                 cosZenithRange_);

    nIterations_ = 32;
    AddParameter(niter_optionname,
                 "Number of iterations",
                 nIterations_);

    // TODO not implemented yet
    // minDifference_ = 0.0;
    // AddParameter(minangdiff_optionname,
    //              "Minimum angulare difference between stored solutions",
    //              minDifference_);
    //
    // nMaxSolutions_ = 1;
    // AddParameter(nsol_optionname,
    //              "Maximum number of solutions."
    //              nMaxSolutions_);

    eventNr_ = 0;
    nSeeds_ = 0;
    nSuccessFits_ = 0;
    nSuccessEvents_ = 0;
}

I3IterativeFitter::~I3IterativeFitter()
{ }

//-----------------------------------------------------------------------------
void I3IterativeFitter::Configure()
{
    GetParameter(minimizer_optionname, minimizer_);
    GetParameter(param_optionname, parametrization_);
    GetParameter(llh_optionname, likelihood_);
    GetParameter(seedservice_optionname, seedService_);
    GetParameter(tweakservice_optionname, tweakService_);
    GetParameter(randomservice_optionname, randomServiceName_);
    GetParameter(coszenithrange_optionname, cosZenithRange_);
    GetParameter(niter_optionname, nIterations_);
    GetParameter("NonStdName", nonStdName_);
    GetParameter("OutputName", fitName_);
    // TODO not implemented yet
    // GetParameter(nsol_optionname, nMaxSolutions_);
    // GetParameter(minangdiff_optionname, nMaxSolutions_);

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

    // TODO not implemented yet
    // log_debug("(%s) Will store max. %d solutions (after sorting by "
    //           "likelihood).", GetName().c_str(), nMaxSolutions_);
    //
    // if (nMinDifference_ > 0.)
    // {
    //     log_debug("(%s) Solutions should be at least %f degrees apart.",
    //               GetName().c_str(), minDifference_/I3Units::degree);
    // }
    // else
    // {
    //     log_debug("(%s) No minimum angular difference between solutions is "
    //               "imposed.", GetName().c_str());
    // }

    if (!tweakService_)
        tweakService_ = seedService_;

    assert(tweakService_);

    // determine how (quasi-)random directions should be generated
    if (randomServiceName_.empty())
    {
        randomServiceName_ = "SOBOL";
    }

    log_debug("(%s) Configured random service: \"%s\"", GetName().c_str(),
              randomServiceName_.c_str());

    if (randomServiceName_ == "SOBOL")
    {
        InitGridPoints(true);
    }
    else if (randomServiceName_ == "NIEDERREITER2")
    {
        InitGridPoints(false);
    }
    else
    {
        randomService_ = context_.Get<I3RandomServicePtr>(randomServiceName_);

        if (!randomService_)
        {
            log_fatal("(%s) Problem with setting random service to \"%s\".",
                      GetName().c_str(), randomServiceName_.c_str());
        }
    }

    // sanity check the configured cosine zenith angle range.
    if (cosZenithRange_.size() != 2)
    {
        log_fatal("(%s) %s option got %zu arguments; should be 2.",
                  GetName().c_str(), coszenithrange_optionname,
                  cosZenithRange_.size());
    }

    if (cosZenithRange_[0] < -1. || cosZenithRange_[1] > 1.)
    {
        log_fatal("(%s) Invalid cos(zenith) range [%f, %f]. The maximum "
                  "allowed range is [-1, 1].", GetName().c_str(),
                  cosZenithRange_[0], cosZenithRange_[1]);
    }

    log_debug("(%s) Configured cos(zenith) range: [%f, %f]", GetName().c_str(),
              cosZenithRange_[0], cosZenithRange_[1]);

    log_debug("(%s) Configured iteration(s) per seed %d", GetName().c_str(),
              nIterations_);

    if (fitName_.empty())
    {
      log_fatal("(%s) Parameter \"OutputName\" was not set.",
                GetName().c_str());
    }

    if (nonStdName_.empty())
    {
        nonStdName_ = fitName_ + I3LogLikelihoodFit::NONSTD_SUFFIX;
    }
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::Geometry(I3FramePtr frame)
{
    log_debug("(%s) Welcome to Geometry method.", GetName().c_str());

    const I3Geometry& geometry = frame->Get<I3Geometry>();
    fitterCore_->SetGeometry(geometry);
    PushFrame(frame, "OutBox");

    log_debug("(%s) Leaving Geometry method.", GetName().c_str());
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::InitGridPoints(bool sobol)
{
    gridPoints_.resize(nIterations_);

    gsl_qrng* generator = sobol ? gsl_qrng_alloc(gsl_qrng_sobol, 2)
                                : gsl_qrng_alloc(gsl_qrng_niederreiter_2, 2);

    std::vector<std::pair<double, double>>::iterator point;
    for (point = gridPoints_.begin(); point != gridPoints_.end(); ++point)
    {
        double value[2];
        gsl_qrng_get(generator, value);
        point->first = value[0];
        point->second = value[1];
    }

    gsl_qrng_free(generator);
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::SetGridDir(I3ParticlePtr particle,
                                   const std::pair<double, double>& offnorm,
                                   unsigned int index)
{
    // copy sobol gridpoint
    std::pair<double, double> normdir = gridPoints_[index];

    // offset by seed (normalized) direction
    normdir.first += offnorm.first;
    normdir.second += offnorm.second;

    // bring (normalized) direction back in range (0..1);
    normdir.first = normdir.first - std::floor(normdir.first);
    normdir.second = normdir.second - std::floor(normdir.second);

    // set new particle direction
    double new_coszen = cosZenithRange_[0] * (1. - normdir.first) +
                        cosZenithRange_[1] * normdir.first;

    particle->SetDir(std::acos(new_coszen), normdir.second*2.*pi);
}

//-----------------------------------------------------------------------------
std::pair<double, double> I3IterativeFitter::GetNormDir(const I3Direction& dir)
{
    double coszen = -dir.GetZ();

    double norm_coszen = (coszen - cosZenithRange_[0]) /
                         (cosZenithRange_[1] - cosZenithRange_[0]);

    norm_coszen = norm_coszen - std::floor(norm_coszen);

    double norm_azi = 0.5 * dir.GetAzimuth() / pi;
    norm_azi = norm_azi - std::floor(norm_azi);

    return std::pair<double, double>(norm_coszen, norm_azi);
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::Physics(I3FramePtr frame)
{
    ++eventNr_;

    log_debug("(%s) Welcome to Physics method.", GetName().c_str());

    log_debug("(%s) This is the %s physics frame.",
              GetName().c_str(), ordinal(eventNr_));

    unsigned int nseeds = seedService_->SetEvent(*frame);
    log_debug("(%s) Got %d seed(s).", GetName().c_str(), nseeds);

    if (tweakService_)
    {
        tweakService_->SetEvent(*frame);
    }

    int ndof = fitterCore_->SetEvent(*frame);

    I3LogLikelihoodFitPtr bestFit;

    if (nseeds == 0 || ndof <= 0)
    {
        log_debug("(%s) No seed(s) available.", GetName().c_str());

        bestFit = boost::make_shared<I3LogLikelihoodFit>(
            seedService_->GetDummy());

        bestFit->hypothesis_->particle->SetFitStatus(I3Particle::MissingSeed);
    }
    else if (ndof <= 0)
    {
        log_debug("(%s) Not enough hits: ndof=%d", GetName().c_str(), ndof);

        bestFit = boost::make_shared<I3LogLikelihoodFit>();

        bestFit->hypothesis_->particle->SetFitStatus(
            I3Particle::InsufficientHits);
    }
    else
    {
        nSeeds_ += nseeds;
        bestFit = BestFit(nseeds);
    }

    assert(
        !(bestFit->hypothesis_->particle->GetFitStatus() == I3Particle::OK &&
          bestFit->hypothesis_->particle->GetShape() == I3Particle::Null));

    frame->Put(fitName_ + I3LogLikelihoodFit::PARTICLE_SUFFIX,
               bestFit->hypothesis_->particle );

    log_debug("(%s) Stored fit has status=%s and shape=%s.", GetName().c_str(),
              bestFit->hypothesis_->particle->GetFitStatusString().c_str(),
              bestFit->hypothesis_->particle->GetShapeString().c_str());

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

    PushFrame(frame, "OutBox");
    log_debug("(%s) Leaving Physics method.", GetName().c_str());
}

//-----------------------------------------------------------------------------
I3LogLikelihoodFitPtr I3IterativeFitter::BestFit(unsigned int nseeds)
{
    std::vector<I3LogLikelihoodFit> goodFits;
    for (unsigned int i = 0; i < nseeds; ++i)
    {
        I3EventHypothesis seed = seedService_->GetSeed(i);
        assert(seed.particle->GetShape() != I3Particle::Null);

        I3LogLikelihoodFitPtr initialFit =
            boost::make_shared<I3LogLikelihoodFit>(seed);

        assert(
            initialFit->hypothesis_->particle->GetShape() != I3Particle::Null);

        // first fit with unmodified seed (only if within zenith range)
        double coszen = -seed.particle->GetDir().GetZ();

        if (cosZenithRange_[0] <= coszen  && coszen <= cosZenithRange_[1])
        {
            Fit(i + 1, 0, initialFit, goodFits);
        }

        // then fit iteratively with randomized directions
        std::pair<double, double> seedNormDir = GetNormDir(
            seed.particle->GetDir());

        for (unsigned int j = 0; j < nIterations_; ++j)
        {
            I3LogLikelihoodFitPtr fit = boost::make_shared<I3LogLikelihoodFit>(
                seedService_->GetCopy(*(initialFit->hypothesis_)));

            // vertex tweaking (in particular space vertex tweaking) should be
            // done before rotating, in order to keep the vertex (and the track
            // as a whole) as close to the COG of the pulses as possible.
            tweakService_->Tweak(*(fit->hypothesis_));

            // get new random direction
            if (randomService_)
            {
                double zenith = std::acos(randomService_->Uniform(
                    cosZenithRange_[0], cosZenithRange_[1]));

                double azimuth = randomService_->Uniform(0., 2.*pi);

                fit->hypothesis_->particle->SetDir(zenith, azimuth);
            }
            else
            {
                SetGridDir(fit->hypothesis_->particle, seedNormDir, j);
            }

            // vertex space/time correction (e.g.)
            tweakService_->Tweak(*(fit->hypothesis_));
            assert(fit->hypothesis_->particle->GetShape() != I3Particle::Null);

            Fit(i + 1, j + 1, fit, goodFits);
        }
    }

    I3LogLikelihoodFitPtr bestFit;

    if (goodFits.size() > 0)
    {
        std::sort(goodFits.begin(), goodFits.end());
        bestFit = boost::make_shared<I3LogLikelihoodFit>(goodFits.front());

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
        bestFit = boost::make_shared<I3LogLikelihoodFit>();

        bestFit->hypothesis_->particle->SetFitStatus(
            I3Particle::FailedToConverge);
    }

    return bestFit;
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::Fit(int seed, int iteration, I3LogLikelihoodFitPtr fit,
                            std::vector<I3LogLikelihoodFit>& goodFits)
{
    // if new fit fails put the old one back
    I3EventHypothesis backup = seedService_->GetCopy(*(fit->hypothesis_));

    bool success = fitterCore_->Fit(fit);

    if (success)
    {
        ++nSuccessFits_;

        assert(fit->hypothesis_->particle->GetFitStatus() == I3Particle::OK);
        assert(fit->hypothesis_->particle->GetShape() != I3Particle::Null);

        goodFits.push_back(*fit);

        assert(goodFits.back().hypothesis_->particle->GetShape() !=
               I3Particle::Null);
    }
    else
    {
        assert(fit->hypothesis_->particle->GetFitStatus() != I3Particle::OK);

        // set fit back to old fit
        fit->hypothesis_->particle = backup.particle;
        fit->hypothesis_->nonstd = backup.nonstd;
    }

    log_debug("(%s) Seed %d, iteration %d: status=%s, ndof=%d, and nmini=%d",
              GetName().c_str(), seed, iteration,
              fit->hypothesis_->particle->GetFitStatusString().c_str(),
              fit->fitparams_->ndof_,
              fit->fitparams_->nmini_);
}

//-----------------------------------------------------------------------------
void I3IterativeFitter::Finish()
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
