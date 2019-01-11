/**
 *
 * Implementation of I3SplineRecoLikelihoodFactory
 *
 * $Id$
 *
 * (c) 2012
 * The IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3SplineRecoLikelihoodFactory.cxx
 * @version $Revision: 93523 $
 * @date $Date: 2012-09-26 11:16:32 +0200 (Wed, 26 Sep 2012) $
 * @author Kai Schatto <KaiSchatto@gmx.de>
 * @brief This file contains the implementation of the I3SplineRecoLikelihoodFactory service factory,
 *        which is a modification of Jake Feintzeig's splineReco module to perform basic spline reconstructions.
 *
 *        See https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction for more information.
 *
 *        This class, I3SplineRecoLikelihoodFactory, is the one with the interface to IceTray. Use this class as
 *        service in your IceTray scripts, with the name of an I3PhotoSplineServiceFactory in the parameter `PhotonicsService'.
 *        Give the name of this service to your fitter, e.g. SimpleFitter, in parameter `LogLikelihood'.
 *        Look at the example scripts in the resources directory.
 *
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "spline-reco/I3SplineRecoLikelihoodFactory.h"

#include "icetray/I3Units.h"
#include "photonics-service/I3PhotonicsService.h"

I3_SERVICE_FACTORY (I3SplineRecoLikelihoodFactory);

I3SplineRecoLikelihoodFactory::I3SplineRecoLikelihoodFactory (const I3Context & ctx):
    I3ServiceFactory (ctx),
    llh_ (new I3SplineRecoLikelihood)
{
    AddParameter ("PhotonicsService", "Photonics service for level2 muon splines. This is a *MUST*.", I3PhotonicsServicePtr ());
    AddParameter ("PhotonicsServiceRandomNoise", "Photonics service for level2 random noise spline. "
            "Is used if \"NoiseModel\" != \"none\".", I3PhotonicsServicePtr ());
    AddParameter ("PhotonicsServiceStochastics", "Photonics service for level2 stochastics spline. "
            "Activate with \"ModelStochastics\".", I3PhotonicsServicePtr ());
    AddParameter ("Pulses", "Name of pulse series to use (*MUST*).", "RecoPulseSeries");
    AddParameter ("Likelihood", "One of \"SPE1st/SPEAll/MPE/MPEAll\". Default: \"SPE1st\".", "SPE1st");
    AddParameter ("NoiseModel", "What noise modelling shall be done? One of \"none/flat/HLC/SRT\". "
            "No noise modelling with \"none\" (default). "
            "If it is not \"none\", \"PhotonicsServiceRandomNoise\" and \"FloorWeight\" must be set. "
            "Note: Noise modelling of the PDF/CDF only done if \"E_Estimators\" are set!", "none");
    AddParameter ("FloorWeight", "Height of the constant noise floor (in PEs, correct?). "
            "Used if \"NoiseModel\" != \"none\".", 1.e-2);
    AddParameter ("ModelStochastics", "Use stochastics spline? Need to set PhotonicsService with \"PhotonicsServiceStochastics\". "
            "Note: Stochastics modelling only done if \"E_Estimators\" are set!", false);
    AddParameter ("NoiseRate", "Rate of noise hits in Hz. Needs to be set also if NoiseModel is \"none\". Defaults to 10 Hertz.", 10.*I3Units::hertz);
    AddParameter ("E_Estimators", "Names of I3Particles containing desired energy estimation. "
            "Mean energy of all given estimators is calculated. "
            "Only used if \"ModelStochastics\" or \"NoiseModel\" != \"none\".", llh_->E_estimator_names);
    AddParameter ("ChargeWeightedLLH", "Use charge weighted spe llh? (Only used when NoiseModel SPEAll is selected.)", false);
    AddParameter ("PreJitter", "Jitter applied directly to time residual of inner photospline PDF. "
            "The photospline PDF is convolved over this jitter.", 0.);
    AddParameter ("PostJitter", "Jitter applied to time residual of outer hit llh (MPE/SPE), "
            "The MPE/SPE (containing PreJitter convolved PDF) is convolved over the PostJitter.", 0.);
    AddParameter ("KSConfidenceLevel", "Confidence Level for Kolmogorov-Smirnov tested charge sum. "
            "KSConfidenceLevel should be from [0,1,2,3,4,5] where 5 = 80\% CL, 4 = 85\%, 3 = 90\%, 2 = 95\% or 1 = 99\%. "
            "The total charge is compared with the KS tested charge if SPEAll or MPEAll is used. "
            "For KSConfidenceLevel = 0, the total charge is used without KS test (Default).", 0);
    AddParameter ("ChargeCalcStep", "Iterations between two charge calculations. "
            "ChargeCalcStep = 0 calculates the KS approved charge only once at the first GetLogLlh call, "
            "ChargeCalcStep > 0 calculates it every ChargeCalcStep minimization steps.", 0);
    AddParameter ("CutMode", "There are 3 different KS models: default/normalized/late. "
            "Tests showed, that \"late\" with KSConfidenceLevel = 5 works best for nugen MC.", "late");
    AddParameter ("EnergyDependentMPE", "Charge in MPE calculation is raised by an energy dependent exponent", false);
    AddParameter ("EnergyDependentJitter", "post jitter is chosen depending on energy for MPE reco", false);
}

bool
I3SplineRecoLikelihoodFactory::InstallService (I3Context & ctx)
{
    return ctx.Put < I3EventLogLikelihoodBase > (llh_, GetName ());
}

void
I3SplineRecoLikelihoodFactory::Configure ()
{
    GetParameter ("PhotonicsService", llh_->ps);
    GetParameter ("PhotonicsServiceRandomNoise", llh_->random_noise_ps);
    GetParameter ("PhotonicsServiceStochastics", llh_->stochastics_ps);
    GetParameter ("Pulses", llh_->pulses_name);
    GetParameter ("Likelihood", llh_->llhChoice);
    GetParameter ("NoiseModel", llh_->noiseModel);
    GetParameter ("FloorWeight", llh_->floorWeight);
    GetParameter ("ModelStochastics", llh_->modelStochastics);
    GetParameter ("NoiseRate", llh_->noiseRate);
    GetParameter ("E_Estimators", llh_->E_estimator_names);
    GetParameter ("ChargeWeightedLLH", llh_->chargeWeight);
    GetParameter ("PreJitter", llh_->preJitter);
    GetParameter ("PostJitter", llh_->postJitter);
    GetParameter ("KSConfidenceLevel", llh_->confidence_level);
    GetParameter ("ChargeCalcStep", llh_->chargeCalcStep);
    GetParameter ("CutMode", llh_->cutMode);
    GetParameter ("EnergyDependentMPE", llh_->EnergyDependentMPE);
    GetParameter ("EnergyDependentJitter", llh_->EnergyDependentJitter);
}
