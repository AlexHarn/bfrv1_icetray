#ifndef INCLUDE_RPDF_PANDEL_H
#define INCLUDE_RPDF_PANDEL_H

/**
 * @brief functions for calculating photoelectron probabilities for muon reconstructions
 *
 * @copyright (C) 2018 The Icecube Collaboration
 *
 * @file pandel.h
 * @author Kevin Meagher
 * @date January 2018
 *
 */

#include <boost/function.hpp>
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3RandomService.h"
#include "rpdf/geometry.h"

namespace rpdf{

  /**
   * Pure function implementation of the Pandel PDF: the probability of observing
   * a hit at t_res a distance eff_distance away from the track
   *
   * @param t_res the time residual of the of a photon which hits a DOM
   * @param eff_distance the distance of the DOM from the hypothesis track accounting
   * for extra scatters caused by the PMT looking away from the track
   * @param ice_model a struct containing the description of optical properties of the ice
   *
   * @returns the probability density of a photon arriving at t=t_res
   */
  double pandel_pdf(const double t_res, const double eff_distance, const IceModel& ice_model);

  /**
   * Sample a photon arrival time from the Pandel PDF.
   *
   * @param eff_distance the distance of the DOM from the hypothesis track accounting
   * for extra scatters caused by the PMT looking away from the track
   * @param ice_model a struct containing the description of optical properties of the ice
   * @param rng an I3RandomService to use to sample random numbers
   *
   * @returns a time residual relative to the direct Cherenkov time at which the photon hits
   * a DOM (see t_res above).
   */
  double pandel_sample(const double eff_distance, const IceModel& ice_model, I3RandomService &rng);

  /**
   * Pure function implementation of the Pandel complementary cumulative distribution
   * function: the probability of observing a hit at t_res or later a distance
   * eff_distance away from the track
   *
   * @param t_res the time residual of the of a photon which hits a DOM
   * @param eff_distance the distance of the DOM from the hypothesis track accounting
   * for extra scatters caused by the PMT looking away from the track
   * @param ice_model struct containing the description of optical properties of the ice
   *
   * @return the probability of a photon arriving at t>=t_res
   */
  double pandel_sf (const double t_res,const double eff_distance, const IceModel& ice);

  double gslConvoluted1F1Diff(const double xi, const double eta);
  double gslConvolutedU(const double xi, const double eta);
  double fastConvolutedHyperg(const double xi, const double eta);

  /**
   * @brief Base Class for Photoelectron Probabilities. I3RecoLLH uses a plugable
   * system to pick which PE probability to use. Those object inherent from this class
   */
  class PhotoElectronProbability{
  public:
    virtual ~PhotoElectronProbability() {}

    /**
     * @brief virtual function for the probability density of a photon arriving at t=t_res
     * at a distance d from the track
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @returns the probability density of a photon arriving at t=t_res
     */
    virtual double pdf(const double t, const double d)const=0;
    /**
     * virtual function for complementary cumulative distribution function:
     * the probability of observing a hit at t_res or later a distance eff_distance away from the track
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @return the probability of a photon arriving at t>=t_res
     */
    virtual double sf (const double t, const double d)const=0;
  };

  /**
   * @brief Contains functions for calculating the convoluted Pandel function
   * and it's survival function using approximations of these functions
   * which were optimized to be as fast as possible.
   *
   * Intended to be used with I3RecoLLH parameter PEProb="GaussConvoluted"
   */
  class FastConvolutedPandel: public PhotoElectronProbability{
    const double jitter_;
    const rpdf::IceModel ice_model_;
  public:

    /**
     * Creates a FastConvolutedPandel object for calculating probabilities
     * from the convoluted pandel function
     *
     * @param jitter the width of the Gaussian which is convoluted with the
     *        pandel function
     * @param ice_model a struct containing the description of optical properties of the ice
     */
  FastConvolutedPandel(const double jitter,const rpdf::IceModel& ice_model):
    jitter_(jitter),ice_model_(ice_model){;}

    /**
     * @brief Calculate the convoluted pandel function using the approximation by
     * van Eindhoven et al, by breaking it into 5 regions and evaluating a different
     * approximation in each region
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @returns the probability density of a photon arriving at t=t_res
     */
    virtual double pdf(const double t_res, const double eff_distance)const;

    /**
     * @brief Calculate complementary cumulative distribution function of the
     * convoluted pandel function using an approximate integral by D. Chirkin.
     *
     * The approximation replaces the Gaussian with a box of the same first and second
     * moment. An additional term is added to account for the behavior at the
     * singularity at t=0. This approximation is very accurate. See
     * @href https://icecube.wisc.edu/~dima/work/WISC/cpdf/a.pdf for more information
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @return the probability of a photon arriving at t>=t_res
     */
    virtual double sf (const double t_res, const double eff_distance)const;
  };

  /**
   * @brief contains functions for calculating the pandel function without any
   * convolution. Intended for use with I3RecoLLH with parameter
   * PEProb="UnconvolutedPandel"
   */
  class UnconvolutedPandel: public PhotoElectronProbability{
    const rpdf::IceModel ice_model_;
  public:

    /**
     * Construct an UnconvolutedPandel object
     *
     * @param ice_model ice struct containing the description of the ice
     */
  UnconvolutedPandel(const rpdf::IceModel& ice_model):
    ice_model_(ice_model){}

    /**
     * @brief Calculate the unconvouted pandel function using boost's implementation
     * of the gamma distribution
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @returns the probability density of a photon arriving at t=t_res
     */
    virtual double pdf(const double t, const double d)const;
    /**
     * calculate complementary cumulative distribution function of the unconvouted pandel
     * function using gsl's implementation of the gamma distribution
     *
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     *
     * @return the probability of a photon arriving at t>=t_res
     */
    virtual double sf (const double t, const double d)const;
  };

  struct SPEfunc {
    /**
     * @brief Boost.Function for calculating the SPE1st likelihood for an individual DOM.
     *
     * It takes the time of the first from the hit series and simply returns the
     * PDF for that hit.
     * Intended for use with I3RecoLLH parameter DOMLikelihood=SPE1st.
     *
     * @param p the class representing the photoelectron probability calculation
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     *        for extra scatters caused by the PMT looking away from the track
     * @param Npe the number of photoelectrons observed by this DOM in this event
     *
     * @returns the SPE1st Likelihood for this DOM
     */
    double operator()(const PhotoElectronProbability& p, const double t_res,
                       const double eff_distance, const double Npe) const;
  };

  struct MPEfunc {
    /**
     * @brief Boost.Function for calculating the MPE likelihood for an individual DOM
     *
     * MPE is the likelihood of observing the first photon at t=t_res given that
     * the DOM saw Npe photoelectrons total. This is intended for use with
     * I3RecoLLH parameter DOMLikelihood=MPE.
     *
     * @param p the class representing the photoelectron probability calculation
     * @param t_res the time residual of the of a photon which hits a DOM
     * @param eff_distance the distance of the DOM from the hypothesis track accounting
     * for extra scatters caused by the PMT looking away from the track
     * @param Npe the number of photoelectrons observed by this DOM in this event
     *
     * @returns the MPE likelihood for this DOM
     */
    double operator()(const PhotoElectronProbability& p, const double t_res,
                       const double eff_distance, const double Npe) const;
  };
  ///function declaration for the DOM Likelihoods
  typedef boost::function<
    double (const PhotoElectronProbability& p, const double t_res,
            const double d_eff,const double Npe)
    > DOMLikelihoodFunction;
}
#endif
