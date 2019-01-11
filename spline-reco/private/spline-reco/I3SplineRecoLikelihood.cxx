/**
 *
 * Implementation of I3SplineRecoLikelihood
 *
 * $Id$
 *
 * (c) 2012
 * The IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3SplineRecoLikelihood.cxx
 * @version $Revision: 93523 $
 * @date $Date: 2012-09-26 11:16:32 +0200 (Wed, 26 Sep 2012) $
 * @author Kai Schatto <KaiSchatto@gmx.de>
 * @brief This file contains the implementation of the I3SplineRecoLikelihood class,
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

#include "spline-reco/I3SplineRecoLikelihood.h"

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

#include "dataclasses/physics/I3MCTreePhysicsLibrary.hh"
#include "gulliver/I3Gulliver.h"


double
I3SplineRecoLikelihood::GetMPEModifier()
{
if (EnergyDependentMPE == false || enEstimate_ < 1.) return 1.;

double logEn = log10(enEstimate_);

if (logEn <= 3) return 0.4;
if (logEn > 3   && logEn <= 3.5) return 0.2*logEn - 0.2;
if (logEn > 3.5 && logEn <= 4)   return 0.4*logEn - 0.9;
if (logEn > 4   && logEn <= 5.5) return 0.2*logEn - 0.1;
if (logEn > 5.5) return 1.;

return 1.;
}

double
I3SplineRecoLikelihood::GetEnergyDependentPostJitter()
{
if (EnergyDependentJitter==false || enEstimate_ < 1.) return postJitter; 

double logEn = log10(enEstimate_);

if (logEn <= 2) return 9;
if (logEn > 2    && logEn <= 3)    return -4*logEn + 17;
if (logEn > 3    && logEn <= 4)    return -1*logEn + 8;
if (logEn > 4    && logEn <= 4.5)  return -2*logEn + 12;
if (logEn > 4.5  && logEn <= 5.75) return -0.8*logEn + 6.6;
if (logEn > 5.75 && logEn <= 7.25) return 2;
if (logEn > 7.25 && logEn <= 7.5)  return -4*logEn + 31;
if (logEn > 7.5) return 1.;
return 1.;
}

unsigned int
I3SplineRecoLikelihood::GetMultiplicityNCh (I3RecoPulseSeriesMapConstPtr pulses)
{
    unsigned int Nch = pulses->size ();
    return Nch;
}

unsigned int
I3SplineRecoLikelihood::GetMultiplicityNPulses (I3RecoPulseSeriesMapConstPtr pulses)
{
    unsigned int NPulses = 0;
    I3RecoPulseSeriesMap::const_iterator idom;
    for (idom = pulses->begin (); idom != pulses->end (); idom++)
    {
        NPulses += idom->second.size ();

    }
    return NPulses;
}

unsigned int
I3SplineRecoLikelihood::GetMultiplicityNPulsesChargeWeight (I3RecoPulseSeriesMapConstPtr pulses)
{
    double charge = 0.;
    unsigned int NPulses = 0;
    I3RecoPulseSeriesMap::const_iterator idom;
    for (idom = pulses->begin (); idom != pulses->end (); idom++)
    {
        std::vector< I3RecoPulse >::const_iterator ihit;
        for (ihit = idom->second.begin (); ihit != idom->second.end (); ihit++)
        {
	  if (!std::isnan (ihit->GetCharge ()))
            {
                charge += ihit->GetCharge ();
            }
        }
    }
    NPulses = (int) (charge + 0.5);
    return NPulses;
}

unsigned int
I3SplineRecoLikelihood::GetMultiplicity ()
{
    if (llhChoice_ == SPE1st || llhChoice_ == MPE)
        return GetMultiplicityNCh(pulses_);
    else if (llhChoice_ == MPEAll)
        return GetMultiplicityNPulses(pulses_);
    else if (llhChoice_ == SPEAll)
    {
        if (chargeWeight)
            return GetMultiplicityNPulsesChargeWeight(pulses_);
        else
            return GetMultiplicityNPulses(pulses_);
    }
    else
        return -1;
}

void
I3SplineRecoLikelihood::SetGeometry (const I3Geometry & f)
{

}

double
I3SplineRecoLikelihood::MeanEstimatedEnergy (const I3Frame & frame)
{
    /* @brief calculate mean of all available energy estimators */
    double meanEnergy = 0.;
    int count = 0;
    for (unsigned int i = 0; i < E_estimator_names.size (); i++)
    {
        if (E_estimator_names[i] == "MCTruth")
        {
            if (frame.Has ("I3MCTree"))
            {
                if (I3MCTreePhysicsLibrary::GetMostEnergeticTrack (frame.Get < I3MCTree > ("I3MCTree")))
                {
                    I3Particle truth = I3MCTreePhysicsLibrary::GetMostEnergeticTrack (frame.Get < I3MCTree > ("I3MCTree"));
                    double value = truth.GetEnergy ();
                    if (!std::isnan (value) && !std::isinf (value) && value > 0)
                    {
                        meanEnergy += value;
                        count += 1;
                    }
                }
            }
            else
            {
                log_warn ("'MCTruth' in E_estimator_names, but I3Frame has no key 'I3MCTree'!");
            }
        }
        else
        {
            if (frame.Has (E_estimator_names[i]))
            {
                I3ParticleConstPtr E_estimator = frame.Get < I3ParticleConstPtr > (E_estimator_names[i]);
                double value = E_estimator->GetEnergy ();
                if (!std::isnan (value) && !std::isinf (value) && value > 0)
                {
                    meanEnergy += value;
                    count += 1;
                }
            }
        }
    }
    if (count > 0)
        meanEnergy = meanEnergy / count;
    return meanEnergy;
}

double
I3SplineRecoLikelihood::CalculateTimeWindow (I3RecoPulseSeriesMapConstPtr pulses)
{
    /* @brief calculate time window size by looping all pulses */
    I3RecoPulseSeriesMap::const_iterator idom;
    std::vector < I3RecoPulse >::const_iterator ihit;
    // loop over DOMs
    bool firstPulse = true;
    for (idom = pulses->begin (); idom != pulses->end (); idom++)
    {
        // loop hits
        for (ihit = idom->second.begin (); ihit != idom->second.end (); ihit++)
        {
            double time = ihit->GetTime ();
            if (firstPulse){
                firstPulse = false;
                startTime_ = time;
                endTime_ = time;
            }
            startTime_ = std::min(startTime_, time);
            endTime_ = std::max(endTime_, time);
        }
    }
    return endTime_-startTime_; // ns
}

void
I3SplineRecoLikelihood::SetEvent (const I3Frame & frame)
{
    if (!frame.Has ("I3Geometry"))
    {
        log_fatal ("Frame has no 'I3Geometry'!");
    }
    geometry_ = frame.Get<I3GeometryConstPtr>();

    //clear domCharge_
    domCharge_.clear ();
    chargeCalcCount_ = 0;

    //calculate mean energy of all given energy estimators if needed
    enEstimate_ = 0.;
    if (modelStochastics || noiseModel != "none" || EnergyDependentMPE || EnergyDependentJitter)
    {
        enEstimate_ = MeanEstimatedEnergy(frame);
    }

    if (llhChoice == "SPE1st")
    {
        llhChoice_ = SPE1st;
    }
    else if (llhChoice == "SPEAll")
    {
        llhChoice_ = SPEAll;
    }
    else if (llhChoice == "MPE")
    {
        llhChoice_ = MPE;
    }
    else if (llhChoice == "MPEAll")
    {
        llhChoice_ = MPEAll;
    }
    else {
        log_fatal ("Likelihood choice '%s' not understood! Use one of \"SPE1st/SPEAll/MPE\".", llhChoice.c_str());
    }

    if (frame.Has (pulses_name))
    {
        pulses_ = frame.Get < I3RecoPulseSeriesMapConstPtr > (pulses_name);
    }
    else
    {
        log_error ("Frame has no input pulses named '%s'!", pulses_name.c_str());
        // create new empty pulse map to prevent seg faults and assertion errors:
        I3RecoPulseSeriesMapPtr newMap (new I3RecoPulseSeriesMap);
        pulses_ = newMap;
    }
    //for noise normalisation
    //calculate time window size by looping all pulses
    timeWindow_ = CalculateTimeWindow(pulses_);
}

double
I3SplineRecoLikelihood::GetLogLikelihood (const I3EventHypothesis & hypo)
{
    return GetLogLikelihood (hypo, NULL, false);
}

double
I3SplineRecoLikelihood::GetLogLikelihood (const I3EventHypothesis & hypo, I3EventHypothesis * gradient, bool fit_energy, double weight)
{

    double llh = 0.;

    // Get event hypothesis
    const I3Particle & p = *(hypo.particle);
    const I3Direction & dir = p.GetDir ();
    double zenith = dir.GetZenith () / I3Units::degree;
    double azimuth = dir.GetAzimuth () / I3Units::degree;
    const I3Position & vertex = p.GetPos ();
    const double x = vertex.GetX ();
    const double y = vertex.GetY ();
    const double z = vertex.GetZ ();
    const int sourceType = 0;
    double energy = 0.01;
    double length = 0.;		// for infinite muon

    // If zenith/azimuth are out of range, transform
    // them to make likelihood periodic
    // Taken from I3Gulliver::AnglesInRange
    // For final fit, gulliver will correct the angles
    // using the same routine
    zenith = fmod (zenith, 360.);
    if (zenith < 0.)
    {
        zenith += 360.;
    }
    if (zenith > 180.)
    {
        zenith *= -1.;
        zenith += 360.;
        azimuth -= 180.;
    }
    azimuth = fmod (azimuth, 360.);
    if (azimuth < 0.)
    {
        azimuth += 360.;
    }

    // Error checks
    if ( std::isnan (vertex.GetX ()) ){ log_fatal ("X of seed particle vertex is NaN."); }
    if ( std::isnan (vertex.GetY ()) ){ log_fatal ("Y of seed particle vertex is NaN."); }
    if ( std::isnan (vertex.GetZ ()) ){ log_fatal ("Z of seed particle vertex is NaN."); }
    if ( std::isnan (dir.GetZenith ()) ){ log_fatal ("Zenith of seed particle direction is NaN."); }
    if ( std::isnan (dir.GetAzimuth ()) ){ log_fatal ("Azimuth of seed particle direction is NaN."); }

    // Quick debugging
    log_debug ("Source properties:");
    log_debug ("x,y,z=%5.3f,%5.3f,%5.3f", x, y, z);
    log_debug ("zenith,azimuth=%5.3f,%5.3f", zenith, azimuth);

    // Initialize source
    //I pass an energy of 0.01 GeV here to "switch off" the lowE stochastics scaling of photonics service
    PhotonicsSource source(x, y, z, zenith, azimuth, 1, length, energy, sourceType);

    I3RecoPulseSeriesMap::const_iterator idom;
    std::vector < I3RecoPulse >::const_iterator ihit;

    log_debug ("llh choice is %d", llhChoice_);

    // Start loop over DOMs
    for (idom = pulses_->begin (); idom != pulses_->end (); idom++)
    {
        if (idom->second.empty())
            continue;
        double mean_pes, epointdistance, geotime;

        /**
         * the mean pes returned from my (Kai Schatto's) stochastics splines is the mean light produced by the high energy stochastic energy losses of a muon scaled down to 1Gev muon energy
         * so just multiply with your estimated muon energy in GeV to get the expected pes
         */
        double stochMean_pes = 0.;

        // Tell photospline where the receiver is
        I3OMGeoMap::const_iterator omGeo = geometry_->omgeo.find (idom->first);
        if (omGeo == geometry_->omgeo.end())
            log_fatal_stream(idom->first << " is not part of the geometry");
        ps->SelectModule (omGeo->second);

        // Tell photospline where the receiver is
        if (modelStochastics || noiseModel != "none")
            stochastics_ps->SelectModule (omGeo->second);
        if (noiseModel != "none")
            random_noise_ps->SelectModule (omGeo->second);

        double dummy;
        // Get direct time from photonics, with new getAmp=false
        // @Jake: Why do we need mean_pes for MPE and MPEAll?
        if (llhChoice_ == MPE || llhChoice_ == MPEAll || modelStochastics || noiseModel != "none")
        {
            ps->SelectSource (mean_pes, NULL, epointdistance, geotime, source, true);
            if (modelStochastics || noiseModel != "none")
                stochastics_ps->SelectSource (stochMean_pes, NULL, dummy, dummy, source, true);
            if (noiseModel != "none")
                random_noise_ps->SelectSource (dummy, NULL, dummy, dummy, source, true);
        }
        else
        {
            ps->SelectSource (mean_pes, NULL, epointdistance, geotime, source, false);
        }

        // For getAmp=false, if the coordinates are in the table
        // range, it returns meanPEs=0...if the coords are out of
        // the range, it returns meanPEs=-1
        if (mean_pes >= 0 && stochMean_pes >= 0)
        {

            double noiseprob = noiseRate;
            log_trace ("noiseRate = %f", noiseRate);
            double npe = 0;

            // get the Kolmogorov Smirnov approved charge. By default the KS-test is switched off,
            // and this simply returns the dom charge.
            if (llhChoice_ == MPE || llhChoice_ == MPEAll || llhChoice_ == SPEAll)
            {
                npe = GetApprovedCharge (idom, p, geotime, mean_pes, stochMean_pes);
            }

            log_trace ("got amplitude of %5.3f from table", mean_pes);
            // For SPE1st and SPE, do LLH with first hit on DOM
            if (llhChoice_ == SPE1st || llhChoice_ == MPE)
            {
                log_trace ("LLh Choice is 'SPE1st' or 'MPE'");
                I3RecoPulse firstHit = idom->second[0];
                double time = firstHit.GetTime ();
                log_trace ("time is %5.3f, geotime is %5.3f", time, geotime);
                double tres = time - geotime - p.GetTime ();
                if (llhChoice_ == MPE)
                {
                    log_trace ("llh = %f", llh);
                    log_trace ("calculating MPE");
                    llh += log (ConvolvedMPE (pow(npe,GetMPEModifier()), tres, 1, mean_pes, stochMean_pes, noiseprob, GetEnergyDependentPostJitter(), time));
                    log_trace ("llh = %f", llh);
                }		// MPE loop

                if (llhChoice_ == SPE1st)
                {
                    log_trace ("llh = %f", llh);
                    log_trace ("calculating SPE");
                    llh += log (ConvolvedSPE (tres, mean_pes, stochMean_pes, noiseprob, 0, postJitter));
                    log_trace ("llh = %f", llh);
                }		// SPE1st loop
            }			// SPE1st or MPE loop
            else if (llhChoice_ == SPEAll || llhChoice_ == MPEAll)
            {
                log_trace ("LLH choice is 'SPEAll' or 'MPEAll'");
                int photon_counter = 0;
                double runningcharge = 0;
                // hit loop
                for (ihit = idom->second.begin (); ihit != idom->second.end (); ihit++)
                {
                    photon_counter++;
                    double time = ihit->GetTime ();
                    log_trace ("time is %5.3f, geotime is %5.3f", time, geotime);
                    double tres = time - geotime - p.GetTime ();

                    if (llhChoice_ == SPEAll)
                    {
                        // Calculate SPE llh
                        log_trace ("llh = %f", llh);
                        log_trace ("calculating SPEAll for hit");
                        double charge = 0;
                        if (chargeWeight || confidence_level > 0)
                        {
			  if (!std::isnan (ihit->GetCharge ()))
                            {
                                charge = ihit->GetCharge ();
                                runningcharge += charge;
                            }
                        }
                        // Check if this hit wasn't refused by KS test
                        if (runningcharge <= npe)
                            llh += log (ConvolvedSPE (tres, mean_pes, stochMean_pes, noiseprob, charge, postJitter));
                        log_trace ("llh = %f", llh);
                    }
                    else if (llhChoice_ == MPEAll)
                    {
                        log_trace ("llh = %f", llh);
                        log_trace ("calculating MPEAll for hit");
                        if (confidence_level > 0)
                        {
                            runningcharge += ihit->GetCharge ();
                        }
                        // Check if this hit wasn't refused by KS test
                        if (runningcharge <= npe)
                            llh += log (ConvolvedMPE (npe, tres, photon_counter, mean_pes, stochMean_pes, noiseprob, postJitter, time));
                        log_trace ("llh = %f", llh);
                    }		// MPEAll if statement
                }		// loop over hits
            }			// SPEAll/MPEAll if statement
        }
        else
        {			//mean PEs if statement
            log_debug ("bad SelectSource call, mean_pes is %5.6f, stochMean_pes is %5.6f.", mean_pes, stochMean_pes);
            if (llhChoice_ == SPE1st || llhChoice_ == MPE)
            {
                log_trace ("llh = %f", llh);
                log_trace ("adding noise rate %f once", noiseRate);
                llh += log (noiseRate);
                log_trace ("llh = %f", llh);
            }
            if (llhChoice_ == SPEAll || llhChoice_ == MPEAll)
            {
                for (ihit = idom->second.begin (); ihit != idom->second.end (); ihit++)
                {
                    log_trace ("llh = %f", llh);
                    log_trace ("adding noise rate %f for hit", noiseRate);
                    llh += log (noiseRate);
                    log_trace ("llh = %f", llh);
                }
            }
        }
    }				// DOMs loop
    log_debug ("llh is %f", llh);

    if (!finite (llh))
    {
        log_warn ("got bad llh=%f", llh);
        llh = -1000000000;
    }

    return llh;
}

long double
I3SplineRecoLikelihood::MPECoeff (int n, int k)
{
    long double result = 1.;
    double factor = k > 0 ? (double)k : 1.;
    if (k > n-k) k = n-k;
    double pos = n;
    while (pos > n - k && pos > 0)
    {
        double denom = k - (n - pos);
        result *= pos / denom;
        pos--;
    }
    result *= factor;
    return result;
}

double
I3SplineRecoLikelihood::BuildWeightedDensityFunction (double bare_pes, double stoch_pes, double bareDF, double rDF, double floorDF, double afterDF, double stochDF)
{
    double df = bareDF;

    if (enEstimate_ > 0)
    {
        double low_pes = 0.;

        // calc expected pes
        if (modelStochastics || noiseModel != "none")
        {
            // scale high E stochastics pes with energy
            stoch_pes *= enEstimate_;
            // C. Wiebusch's parametrisation for low E stochastics
            low_pes = (0.1720 + 0.0324 * log (enEstimate_)) * bare_pes;
        }

        //merge bare DF with stochastics DF
        if (modelStochastics)
        {
            df = ((stoch_pes + low_pes) * stochDF + bare_pes * bareDF) / (stoch_pes + low_pes + bare_pes);
        }

        //noise modelling
        if (noiseModel != "none")
        {
            double physicsDF = df;
            double expected_pes = bare_pes + stoch_pes + low_pes;

            //model random noise: constant noise floor + bump caused by hit cleaning
            double bumpWeight = 1.;
            if (noiseModel == "flat") bumpWeight = 0.;
            double randomDF = (bumpWeight * rDF * (1. - exp (-expected_pes)) + floorDF * floorWeight) / (bumpWeight * (1. - exp (-expected_pes)) + floorWeight);

            //get weights
            double physicsWeight = 1.;
            double randomWeight = 0.;
            double afterWeight = 0.;
            double preLateWeight = 0.;
            GetNoiseWeights (physicsWeight, randomWeight, afterWeight, preLateWeight, expected_pes);
            df = (physicsDF * physicsWeight + randomDF * randomWeight + physicsDF * preLateWeight + afterDF * afterWeight) / (physicsWeight + randomWeight + afterWeight + preLateWeight);
        }
    }
    return df;
}

double
I3SplineRecoLikelihood::BuildWeightedPdf (double tres, double bare_pes, double stoch_pes)
{
    double barePdf = ConvolvedPdf (tres, ps, preJitter);
    double rPdf = 0., floorPdf = 0., afterPdf = 0.;
    if (noiseModel != "none"){
        rPdf = ConvolvedPdf (tres, random_noise_ps, 0);
        floorPdf = 1. / timeWindow_;
        afterPdf = ConvolvedPdf (tres - 2000, random_noise_ps, 0); // model after pulses
    }
    double stochPdf = 0.;
    if (modelStochastics){
        stochPdf = ConvolvedPdf (tres, stochastics_ps, preJitter);
    }
    return BuildWeightedDensityFunction(bare_pes, stoch_pes, barePdf, rPdf, floorPdf, afterPdf, stochPdf);
}

double
I3SplineRecoLikelihood::BuildWeightedCdf (double tres, double bare_pes, double stoch_pes, double hitTime)
{
    double bareCdf = ConvolvedCdf (tres, ps, preJitter);
    double rCdf = 0., floorCdf = 0., afterCdf = 0.;
    if (noiseModel != "none"){
        rCdf = ConvolvedCdf (tres, random_noise_ps, 0);
        floorCdf = (hitTime - startTime_)/timeWindow_;
        afterCdf = ConvolvedCdf (tres - 2000, random_noise_ps, 0); // model after pulses
    }
    double stochCdf = 0.;
    if (modelStochastics){
        stochCdf = ConvolvedCdf (tres, stochastics_ps, preJitter);
    }
    return BuildWeightedDensityFunction(bare_pes, stoch_pes, bareCdf, rCdf, floorCdf, afterCdf, stochCdf);
}

double
I3SplineRecoLikelihood::ConvolvedCdf (double tres, I3PhotonicsServicePtr & spline, double jitter)
{
    /* @brief just return the cdf as i don't have a routine to calc a cdf of a gauss convolved pdf */
    return GetCdf (tres, spline);
}

double
I3SplineRecoLikelihood::GetCdf (double tres, I3PhotonicsServicePtr & spline)
{
    double time_edges[2] = { -10000, tres };
    double cdf_array[1] = { 0. };
    double cdf = 0.;

    if (spline->GetProbabilityQuantiles (time_edges, 0, cdf_array, NULL, 1))
    {
        cdf = cdf_array[0];
    }

    return std::max (0., std::min (1., cdf));
}

double
I3SplineRecoLikelihood::ConvolvedPdf (double tres, I3PhotonicsServicePtr & spline, double jitter)
{
    // only do the convolution in an interval where it makes a difference -> speed up
    if (jitter == 0 || tres > posResLimit_ || tres < -jitterWidth_ * jitter - 5.)
        return GetPdf (tres, spline);
    else
    {
        float probConvData[convDataLength_];
        for (int i = 0; i < convDataLength_; i++)
        {
            probConvData[i] = GetPdf (tres + (jitter * jitterWidth_ * double(i - halfSupport_) / (double)halfSupport_), spline);
        }
        GaussianIIR1D (probConvData, convDataLength_, halfSupport_ / jitterWidth_, numSteps_);
        return probConvData[halfSupport_];
    }
}

double
I3SplineRecoLikelihood::GetPdf (double tres, I3PhotonicsServicePtr & spline)
{
    double pdf = 0;
    spline->GetProbabilityDensity (pdf, tres);
    if (pdf < 0)
    {
        log_trace ("Photorec call failed!");
        pdf = 0;
    }

    return pdf;
}

void
I3SplineRecoLikelihood::GetNoiseWeights (double &physicsWeight, double &randomWeight, double &afterWeight, double &preLateWeight, double expected_pes)
{
    /* @brief fitted noise weights for noise modelling. See http://butler.physik.uni-mainz.de/~amauser/download/bachelor_joschua.pdf */
    double log10_pes = log10 (expected_pes);
    if (noiseModel == "HLC")
    {
        physicsWeight = 1 - exp (-pow (10., 0.355311 - 0.810614 * atan (1.37116 - 0.799182 * log10_pes) + 0.462819 * atan (16.6328 + 5.23089 * log10_pes)));
        randomWeight = 1 - exp (-pow (10., std::min (0., -1.04595 - 1.38939 * atan (1.3419 + 0.659969 * log10_pes))));
        afterWeight = 1 - exp (-pow (10., 0.257231 + 1.04697 * log10_pes - 2.21043 * atan (0.803398 + 0.449565 * log10_pes)));
        preLateWeight = 1 - exp (-pow (10., 0.0805558 + 0.928096 * log10_pes - 1.0686 * atan (1.31006 + 0.835417 * log10_pes)));
    }
    else if (noiseModel == "flat")
    {
        physicsWeight = 1 - exp (-pow (10., 0.296574 + 1.6467 * log10_pes + 5.08968 * atan(0.00385278 - 0.257861 *log10_pes)));
        randomWeight = 1 - exp (-pow (10., std::min (-0.042, -1.13461 - 1.03123 * atan(0.3965 + 0.740068 * log10_pes))));
        afterWeight = 1 - exp (-pow (10., -0.554393 + 1.63425 *log10_pes - 3.50506 *atan(0.122375 + 0.333922 *log10_pes)));
        preLateWeight = 1 - exp (-pow (10., -0.181343 + 1.51783 *log10_pes - 3.14974 * atan(0.232648 + 0.324715 *log10_pes)));
    }
    else if (noiseModel == "SRT")
    {
        physicsWeight = 1 - exp (-pow (10., 0.774234 - 1.02385 * atan (0.969486 - 0.577865 * log10_pes) + 0.193763 * atan (16.2363 + 4.76944 * log10_pes)));
        randomWeight = 1 - exp (-pow (10., std::min (0., -1.36638 - 1.10099 * atan (1.08653 + 0.850798 * log10_pes))));
        afterWeight = 1 - exp (-pow (10., -0.186922 + 0.946511 * log10_pes - 1.08804 * atan (1.73241 + 0.760878 * log10_pes)));
        preLateWeight = 1 - exp (-pow (10., 0.144828 + 0.928378 * log10_pes - 1.11346 * atan (1.3868 + 0.780568 * log10_pes)));
    }
    else
        log_warn ("Noise model not found. There will be no noise in reco!");
}

double
I3SplineRecoLikelihood::ConvolvedMPE (double npe, double tres, int photon_counter, double mean_pes, double stochMean_pes, double noiseprob, double jitter, double hitTime)
{
    // only do the convolution in an interval where it makes a difference -> speed up
    if (jitter == 0 || tres > posResLimit_ || tres < -jitterWidth_ * jitter - 5.)
    {
        log_trace ("Do no convolution. Just run BuildMPE once.");
        double mpe = BuildMPE (npe, tres, photon_counter, mean_pes, stochMean_pes, noiseprob, hitTime);
        log_trace ("MPE = %f is returned from ConvolvedMPE", mpe);
        return mpe;
    }
    else
    {
        log_trace ("Do convolution. Run BuildMPE several times.");
        float llhConvData[convDataLength_];
        for (int i = 0; i < convDataLength_; i++)
        {
            llhConvData[i] = BuildMPE (npe, tres + (jitter * jitterWidth_ * double(i - halfSupport_) / (double)halfSupport_), photon_counter, mean_pes, stochMean_pes, noiseprob, hitTime);
        }
        log_trace ("Now the convolution.");
        GaussianIIR1D (llhConvData, convDataLength_, halfSupport_ / jitterWidth_, numSteps_);
        return llhConvData[halfSupport_];
    }
}

double
I3SplineRecoLikelihood::ConvolvedSPE (double tres, double mean_pes, double stochMean_pes, double noiseprob, double charge, double jitter)
{
    // only do the convolution in an interval where it makes a difference -> speed up
    if (jitter == 0 || tres > posResLimit_ || tres < -jitterWidth_ * jitter - 5.)
        return BuildSPE (tres, mean_pes, stochMean_pes, noiseprob, charge);
    else
    {
        float llhConvData[convDataLength_];
        for (int i = 0; i < convDataLength_; i++)
        {
            llhConvData[i] = BuildSPE (tres + (jitter * jitterWidth_ * double(i - halfSupport_) / (double)halfSupport_), mean_pes, stochMean_pes, noiseprob, charge);
        }
        GaussianIIR1D (llhConvData, convDataLength_, halfSupport_ / jitterWidth_, numSteps_);
        return llhConvData[halfSupport_];
    }
}

double
I3SplineRecoLikelihood::BuildMPE (double npe, double tres, int photon_counter, double mean_pes, double stochMean_pes, double noiseprob, double hitTime)
{
    // Calculate MPE llh
    double llh = 1.;
    double prob = BuildWeightedPdf (tres, mean_pes, stochMean_pes);
    log_trace ("in BuildMPE");
    log_trace ("BuildMPE's llh = %f", llh);
    if (finite (prob) && prob > 0)
    {
        if (npe < 1.5)
        {
            log_trace ("npe = %f < 1.5", npe);
            llh = prob + noiseprob;
            log_trace ("BuildMPE's llh = %f", llh);
        }
        else
        {
            double cdf = BuildWeightedCdf (tres, mean_pes, stochMean_pes, hitTime);
            if (cdf > 1) cdf = 1;
            if (cdf < 0) cdf = 0;
            double tint = 1.0 - cdf;
            int tpe ((unsigned int) (npe + 0.5));
            if (tint >= 1.0)
            {
                log_trace ("tint = %f > 1.0", tint);
                llh = tpe * prob + noiseprob;
                log_trace ("BuildMPE's llh = %f", llh);
            }
            else if (tint > 0.0)
            {
                log_trace ("tint = %f > 0.0, < 1.0", tint);
                if (photon_counter > 1)
                {
                    log_trace ("photon_counter = %d > 1", photon_counter);
                    double coeff2 = MPECoeff (tpe, photon_counter);
                    llh = coeff2 * prob * pow ((double) tint, (double) tpe - photon_counter) * pow ((double) cdf, (double) photon_counter - 1) + noiseprob;
                    log_trace ("BuildMPE's llh = %f", llh);
                }
                else
                {
                    log_trace ("photon_counter = %d <= 1", photon_counter);
                    llh = tpe * prob * pow ((double) tint, (double) tpe - photon_counter) + noiseprob;
                    log_trace ("BuildMPE's llh = %f", llh);
                }
            }
            else
            {
                log_trace ("tint = %f <= 0.0", tint);
                llh = noiseRate;
                log_trace ("BuildMPE's llh = %f", llh);
            }
        }
    }
    else
    {
        log_trace ("prob = %f <= 0.0 or not finite", prob);
        llh = noiseRate;
        log_trace ("BuildMPE's llh = %f", llh);
    }
    if (llh <= 0)
    {
        log_trace ("BuildMPE's llh = %f <= 0.0", llh);
        llh = noiseRate;
        log_trace ("BuildMPE's llh = %f", llh);
    }
    log_trace ("BuildMPE's llh = %f is returned", llh);
    return llh;
}

double
I3SplineRecoLikelihood::BuildSPE (double tres, double mean_pes, double stochMean_pes, double noiseprob, double charge)
{
    // Calculate SPE llh
    double llh = 1.;
    double prob = BuildWeightedPdf (tres, mean_pes, stochMean_pes);
    if (finite (prob) && (prob > 0))
    {
        if (chargeWeight && llhChoice_ == SPEAll)
        {
            llh *= pow (prob + noiseprob, charge);	//is this correct?
        }
        else
        {
            llh *= prob + noiseprob;
        }
    }
    else
    {
        log_trace ("prob from splines wasn't finite and positive");
        llh *= noiseRate;
    }
    if (llh <= 0){
        log_trace ("SPE llh %f <= 0", llh);
        llh = noiseRate;
    }
    return llh;
}

double
I3SplineRecoLikelihood::KS0Limit (double npulses, int level)
{
    /**
     * first  column = level 5 corresponds to 80% confidence level
     *
     * second column = level 4 corresponds to 85% confidence level
     *
     * third  column = level 3 corresponds to 90% confidence level
     *
     * fourth column = level 2 corresponds to 95% confidence level
     *
     * fifth  column = level 1 corresponds to 99% confidence level
     */
    static const float ks_test[20][5] = {
        {.900, .925, .950, .975, .995},
        {.684, .726, .776, .842, .929},
        {.565, .597, .642, .708, .828},
        {.494, .525, .564, .624, .733},
        {.446, .474, .510, .565, .669},
        {.410, .436, .470, .521, .618},
        {.381, .405, .438, .486, .577},
        {.358, .381, .411, .457, .543},
        {.339, .360, .388, .432, .514},
        {.322, .342, .368, .410, .490},
        {.307, .326, .352, .391, .468},
        {.295, .313, .338, .375, .450},
        {.284, .302, .325, .361, .433},
        {.274, .292, .314, .349, .418},
        {.266, .283, .304, .338, .404},
        {.258, .274, .295, .328, .392},
        {.250, .266, .286, .318, .387},
        {.244, .259, .278, .309, .371},
        {.237, .252, .272, .301, .363},
        {.231, .246, .264, .294, .356}
    };
    static const double a[5] = { 1.07, 1.14, 1.22, 1.36, 1.63 };
    double limit;

    if(npulses<0)
    {
        npulses=0;
    }
    if (npulses <= 20)
    {
        int index=(int) (npulses + 0.5) - 1;
        if(index < 0)
        {
            index=0;
        }
        limit = ks_test[index][5 - level];
    }
    else
    {
        limit = a[5 - level] / sqrt (npulses);
    }
    return limit;
}

double
I3SplineRecoLikelihood::GetApprovedCharge (I3RecoPulseSeriesMap::const_iterator idom, const I3Particle p, const double geotime, const double mean_pes, const double stochMean_pes)
{
    /**
     * Calculates the total charge in DOM idom for particle p.  If
     * confidence_level is set to either 5 = 80%, 4 = 85%, 3 = 90%, 2 = 95% or 1
     * = 99% the last pulse is cut off until either a Kolmogorov Smirnov Test of
     * measured pulses versus CDF yields a confidence level greater than
     * cofidence_level or there is only one pulse left. There are 3 different KS
     * models "late", "normalized" and "default". Tests showed, that "late" with
     * confidence_level = 5 works best for nugen MC.  chargeCalcStep = 0
     * calculates the KS approved charge only once at the first GetLogLlh call,
     * chargeCalcStep > 0 calculates every chargeCalcStep minimization steps.
     * For confidence_level = 0 the total is returned without KS test. this is
     * the default.
     */
    chargeCalcCount_++;
    if (chargeCalcStep <= 0)
    {
        if (domCharge_.find (idom->first) != domCharge_.end ())
        {			//check if domCharge_ has been calculated
            return domCharge_.find (idom->first)->second;	//if true return domCharge_
        }
    }
    else if (chargeCalcCount_ % chargeCalcStep != 0)
    {
        if (domCharge_.find (idom->first) != domCharge_.end ())
        {
            return domCharge_.find (idom->first)->second;
        }
    }
    // else do the calculation
    log_trace ("Calculating approved DOM charge");

    double totalcharge = 0.;
    std::vector < I3RecoPulse >::const_iterator ipulse;
    for (ipulse = idom->second.begin (); ipulse != idom->second.end (); ipulse++)
    {			//loop over all pulses in DOM idom and sum the charge
      if (!std::isnan (ipulse->GetCharge ()))
        {
            totalcharge += ipulse->GetCharge ();
        }
    }
    if (confidence_level > 0 && confidence_level < 6)
    {			//if confidence level is set do Kolmogorov Smirnov Test on pulses
        std::map < double, double > CdfValue;	//<time,CDF>
        double kolmogorov_maximum = 0.;
        double runningcharge = 0.;
        std::vector < I3RecoPulse >::const_iterator LastPulse = idom->second.end ();
        for (ipulse = idom->second.begin (); ipulse != LastPulse; ipulse++)
        {			//run test with all pulses, fill map CdfValue so CDF has to be calculated only once
            runningcharge += ipulse->GetCharge ();
            double tres = ipulse->GetTime () - geotime - p.GetTime ();
            double Cdf = BuildWeightedCdf (tres, mean_pes, stochMean_pes, ipulse->GetTime ());
            CdfValue[ipulse->GetTime ()] = Cdf;
            double diff;
            if (cutMode == "late")
                diff = Cdf - runningcharge / totalcharge;
            else
                diff = fabs (Cdf - runningcharge / totalcharge);
            kolmogorov_maximum = std::max (diff, kolmogorov_maximum);
        }
        while (kolmogorov_maximum > KS0Limit (totalcharge, confidence_level) && --LastPulse != idom->second.begin ())
        {			//if confidence level is not met subtract the last pulse until it is but always keep the first pulse
            totalcharge -= LastPulse->GetCharge ();
            kolmogorov_maximum = 0.;
            runningcharge = 0.;
            for (ipulse = idom->second.begin (); ipulse != LastPulse; ipulse++)
            {
                runningcharge += ipulse->GetCharge ();
                double Cdf = CdfValue.find (ipulse->GetTime ())->second;
                double CdfMaximum;
                if (cutMode == "normalized")
                    CdfMaximum = CdfValue.find ((LastPulse - 1)->GetTime ())->second;
                else
                    CdfMaximum = 1;
                double diff;
                if (cutMode == "late")
                    diff = Cdf / CdfMaximum - runningcharge / totalcharge;
                else
                    diff = fabs (Cdf / CdfMaximum - runningcharge / totalcharge);
                kolmogorov_maximum = std::max (diff, kolmogorov_maximum);
            }
        }
    }
    domCharge_[idom->first] = totalcharge;	//Set the domCharge_ value for this DOM so it won't have to be calculated again
    return totalcharge;	//return domCharge_ value
}
