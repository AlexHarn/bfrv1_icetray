/**
 *
 * Definition of I3SplineRecoLikelihood
 *
 * $Id$
 *
 * Copyright (C) 2012
 * The IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3SplineRecoLikelihood.h
 * @version $Revision$
 * @date $Date$
 * @author Kai Schatto <KaiSchatto@gmx.de>
 * @brief This file contains the definition of the I3SplineRecoLikelihood class,
 *        which is a modification of Jake Feintzeig's splineReco module to perform basic spline reconstructions.
 *
 *        See https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction for more information.
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

#ifndef I3SPLINERECOLIKELIHOOD_H
#define I3SPLINERECOLIKELIHOOD_H

#include <string>
#include <vector>

#include "gulliver/I3EventLogLikelihoodBase.h"
#include "gulliver/I3EventHypothesis.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "photonics-service/I3PhotonicsService.h"

class I3SplineRecoLikelihood : public I3EventLogLikelihoodBase
{
    public:
        I3SplineRecoLikelihood () :
            ps(I3PhotonicsServicePtr()),
            stochastics_ps(I3PhotonicsServicePtr()),
            random_noise_ps(I3PhotonicsServicePtr()),
            noiseModel("none"),
            floorWeight(1e-2),
            modelStochastics(false),
            E_estimator_names(std::vector<std::string>()),
            confidence_level(0),
            chargeCalcStep(0),
            cutMode("late"),
            EnergyDependentMPE(false),
            EnergyDependentJitter(false),
            preJitter(0.),
            postJitter(0.),
            pulses_name("RecoPulseSeries"),
            llhChoice("SPE1st"),
            noiseRate(10.*I3Units::hertz),
            chargeWeight(false),
            jitterWidth_(3.),           /// calculation width of jitter in multiples of sigma(preJitter/postJitter)
            posResLimit_(40.),          /// convolving only if tres < posResLimit;
            convDataLength_(21),        /// must be odd. Number of sample points
            halfSupport_( (int) ((convDataLength_ - 1.) / 2.) ),
            numSteps_(4),               /// accuracy of approximation
            enEstimate_(0.),
            timeWindow_(0.),
            startTime_(0.),
            endTime_(0.),
            chargeCalcCount_(0),
            llhChoice_(SPE1st)
    { }
        virtual ~ I3SplineRecoLikelihood () { }

        /// Gulliver interface
        void SetEvent (const I3Frame &);
        void SetGeometry (const I3Geometry & f);
        double GetLogLikelihood (const I3EventHypothesis &);
        double GetLogLikelihood (const I3EventHypothesis &, I3EventHypothesis *, bool solve_for_energies, double weight=1);
        unsigned int GetMultiplicity ();
        const std::string GetName () const
        {
            return std::string("I3SplineRecoLikelihood");
        }
        
        enum LlhChoice
        {
            SPE1st = 1,
            SPEAll = 2,
            MPE = 3,
            MPEAll = 4
        };
        #define I3SPLINERECOLIKELIHOOD_LlhChoice (SPE1st)(SPEAll)(MPE)(MPEAll)

    private:
        double GetMPEModifier();
        double GetEnergyDependentPostJitter();
        unsigned int GetMultiplicityNCh (I3RecoPulseSeriesMapConstPtr pulses);
        unsigned int GetMultiplicityNPulses (I3RecoPulseSeriesMapConstPtr pulses);
        unsigned int GetMultiplicityNPulsesChargeWeight (I3RecoPulseSeriesMapConstPtr pulses);

        double MeanEstimatedEnergy (const I3Frame & frame);
        double CalculateTimeWindow (I3RecoPulseSeriesMapConstPtr pulses);

        double BuildWeightedDensityFunction (double bare_pes, double stoch_pes, double bareDF, double rDF, double floorDF, double afterDF, double stochDF=0.);

        /** Those two functions construct pdf/cdf by merging different pdfs/cdfs depending on options like "modelStochastics" and "noiseModel".
         * Bare muon pes and stochastic spline pes are needed for weighting.
         * the result is the normalized pdf/cdf value at tres
         */
        double BuildWeightedPdf (double tres, double bare_pes, double stoch_pes);
        double BuildWeightedCdf (double tres, double bare_pes, double stoch_pes, double hitTime);

        /// returns cdf/pdf from given spline at tres
        double GetCdf (double tres, I3PhotonicsServicePtr & spline);
        double GetPdf (double tres, I3PhotonicsServicePtr & spline);

        /// wrapper to return approximative fast gauss convoluted cdf/pdf (makes no sense for cdf, though): Prejitter.
        double ConvolvedCdf (double tres, I3PhotonicsServicePtr & spline, double jitter);
        double ConvolvedPdf (double tres, I3PhotonicsServicePtr & spline, double jitter);

        /// returns MPE/SPE. Use photon_counter (k) in loop for MPEAll
        double BuildMPE (double npe, double tres, int photon_counter, double mean_pes, double stochMean_pes, double noiseprob, double hitTime);
        double BuildSPE (double tres, double mean_pes, double stochMean_pes, double noiseprob, double charge);

        /// wrapper to return approximative fast gauss convoluted MPE/SPE: Postjitter
        double ConvolvedMPE (double npe, double tres, int photon_counter, double mean_pes, double stochMean_pes, double noiseprob, double jitter, double hitTime);
        double ConvolvedSPE (double tres, double mean_pes, double stochMean_pes, double noiseprob, double charge, double jitter);

        /// fast approximative gaussian convolution
        void GaussianIIR1D (float *data, long length, float sigma, int numsteps);

        /// fitted noise weights for noise modelling. See http://butler.physik.uni-mainz.de/~amauser/download/bachelor_joschua.pdf
        void GetNoiseWeights (double &physicsWeight, double &randomWeight, double &afterWeight, double &preLateWeight, double expected_pes);

        /// Kolmogorov-Smirnov limits
        double KS0Limit (double npulses, int level);

        /// returns only the charge of a dom that fits to pdf by performing Kolmogorov-Smirnov-Test
        double GetApprovedCharge (I3RecoPulseSeriesMap::const_iterator idom, const I3Particle p, const double geotime, const double mean_pes, const double stochMean_pes);

        /** calculates n!/((k-1)!*(n-k)!) avoiding rounding errors due to huge factorials. Could be faster, too, because it's cancelling down before calculation.
         * Loops and picks one number from denominator and one number from numerator at a time and multiplies the quotient to the result.
         */
        long double MPECoeff (int n, int k);


    public:
        /// basic spline, probably bare muon
        I3PhotonicsServicePtr ps;

        /// stochastics spline
        I3PhotonicsServicePtr stochastics_ps;

        /// noise spline = bare spline with jitter = 1000ns (I really should change that, and use a parametrisation - Kai Schatto)
        I3PhotonicsServicePtr random_noise_ps;

        /// noise and mixStochastics options
        std::string noiseModel;
        double floorWeight;
        bool modelStochastics;

        /// energy estimators for pes calculation
        std::vector < std::string > E_estimator_names;

        /// Kolmogorov Smirnov options
        int confidence_level;
        int chargeCalcStep;
        std::string cutMode;

        bool EnergyDependentMPE;
        bool EnergyDependentJitter;

        /// jitter options
        double preJitter;
        double postJitter;

        std::string pulses_name;
        std::string llhChoice;
        double noiseRate;
        bool chargeWeight;

    private:
        I3RecoPulseSeriesMapConstPtr pulses_;
        I3GeometryConstPtr geometry_;
        /** stores the dom charge calculated by Kolmogorov-Smirnov Test or in case of KS-test switched off just the dom charge
         * avoids recalculation every iteration
         */
        std::map < OMKey, double > domCharge_;

        /// reasonable gauss conv settings
        const double jitterWidth_;		/// calculation width of jitter in multiples of sigma(preJitter/postJitter)
        const double posResLimit_;		/// convolving only if tres < posResLimit;
        const int convDataLength_;		/// must be odd. Number of sample points
        const int halfSupport_;
        const int numSteps_;			/// accuracy of approximation

        double enEstimate_;
        double timeWindow_;
        double startTime_;
        double endTime_;

        int chargeCalcCount_;

        
        LlhChoice llhChoice_;
};

I3_POINTER_TYPEDEFS (I3SplineRecoLikelihood);

#endif //I3SPLINERECOLIKELIHOOD
