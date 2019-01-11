.. _cscd-llh:

CscdLlh
*******

:Authors: 
  Michael Greene <mgreene@phys.psu.edu>
  project maintainer: Tomasz Palczewski <tomas.j.palczewski@ua.edu>


.. toctree::
   :maxdepth: 2

   release_notes


CscdLlh Overview
==================

The CscdLlh module performs a log-likelihood reconstruction of neutrino-induced cascades.  A steering file parameter is used to specify a probability density function (PDF) that describes the probability p(**v**; **a**) that an OM hit with time and position **v** = (t, x, y, z) would result from a cascade characterized by a set of parameters **a**.  For each event, CscdLlh passes a list of OM hits to a minimizer, which finds the parameters **a** that give the maximum probability
 .. math::
  \mathcal{L} = \Pi_i \, p_i(\vec{v_i}; \vec{a})
or, equivalently, the minimum negative log-likelihood
 .. math::
  - ln(\mathcal{L}) = - \, \Sigma_i \, \ln(p_i(\vec{v_i}; \, \vec{a})),

where the index i runs over the OM hits. The cascade parameters made available to the minimizer are (at, ax, ay, az, :math:`\theta`, :math:`\phi`, E), where at, ax, ay, az are the vertex time and position, :math:`\theta`, :math:`\phi` are the polar and azimuthal angles of the incident neutrino's velocity, and E is the neutrino energy. No PDF's describing directionality have been implemented yet; the angles :math:`\theta`, :math:`\phi` must therefore always be fixed. The other parameters may be fixed or free.

Class Summary
===============

The main module is I3CscdLlhModule. I3CscdLlhModule obtains a cascade vertex from the RecoResultDict that has been calculated by a previous reconstruction module (e.g., CFirst).  This "first guess" vertex is used to seed the minimizer.

Configuration parameters, hit data, etc., are passed from the I3CscdLlhModule to an I3CscdLlhFitter.  The main function of the I3CscdLlhFitter is to instantiate and initialize minimizer and pdf objects.

The minimizer is implemented by the I3CscdLlhMinimizer class.  This is a wrapper around ROOT's TMinuit class, which is itself an implementation of the Minuit minimization package developed at CERN.  A future version of CscdLlh may allow for other minimizers to be used; one possible design would be to introduce an I3CscdLlhAbsMinimizer base class from which other minimizers (e.g., I3CscdLlhMinuitMinimizer) are derived.

PDF's currently available for use by CscdLlh are:
 
* I3CscdLlhUPandel: A single-photoelectron patched Pandel (UPandel) time-likelihood PDF;
* I3CscdLlhUPandelMpe: A multi-photoelectron patched Pandel (UPandel) time-likelihood PDF;
* I3CscdLlhHitNoHit: A single-photoelectron Hit/No-hit position-likelihood PDF;
* I3CscdLlhHitNoHitMpe: A multi-photoelectron Hit/No-hit position-likelihood PDF;

Follow the HTML links for documentation on the individual PDF's.  PDF classes must be subclassed from I3CscdLlhAbsPdf.

Steering File Parameters
==============================

The log-likelihood minimization can be configured using the following steering file parameters:

* param InputType [DEFAULT="RecoHit"] 
       Type of input hits. Options are "RecoHit" and "RecoPulse"
* param RecoSeries [DEFAULT="Muon-DAQ Hits"] RecoSeries name

* param FirstLE [DEFAULT=true] 
       If true, use the first leading edge with a weight factor.  If false, use all leading edges with no weights.

* param AmpWeightPower [DEFAULT=0] 
       Hits are weighted with an amplitude raised to this power, if "FirstLE" is true.  The "amplitude" is set equal to the number of hits in the RecoHitsSeries for the OM.  The AmpWeightPower is usually set to either 0.0 (weight=1 for all hits) or 1.0 (weight=amplitude).  Weights :math:`$w_i` are used by defining the negative log-likelihood as :math:`- \, ln(\mathcal{L}) = - \, \Sigma_i \, w_i \, \ln(p_i(\vec{v_i}; \, \vec{a}))`

* param SeedKey [DEFAULT=""] 
       The dictionary key that holds the first guess vertex position and time.  The key is specified in a previous reconstruction module, e.g., CFirst.

* param SeedWithOrigin [DEFAULT=false] 
       If true, use the detector origin and t=0 as a first guess for the vertex position and time.  The "SeedKey" will be ignored.  To be used only for testing purposes.
       
* param EnergySeed [DEFAULT=NaN] 
       An initial value for the cascade energy (GeV).  
* param EnergyCalibration [DEFAULT=1.4] 
       This is the I_0 in the analytic expression for the mean number of PEs

* param MinHits [DEFAULT=10] 
       Minimum number of hits in the event.  If there are fewer hits, a reconstruction will not be attempted.

* param MinuitVerbosityLevel [DEFAULT=-1] 
       Minuit output message level.  Allowed values are from -1 to 3, inclusive.  Higher values give more messages.

* param MinuitPrintWarnings [DEFAULT=0] 
       Output Minuit warnings.

* param MinuitStrategy [DEFAULT=2] 
       Allowed values are 0, 1, and 2.  0 is faster, 2 is more reliable, 1 is intermediate.

* param CalculateGradient [DEFAULT=false] 
       Calculate the first derivatives of the minimized funtion.  If set to true, the PDF must implement the calculation.  If set to false, Minuit will calculate the gradient using finite differences.

* param minAlgorithm [DEFAULT=MIGRAD] 
       The minimization algorithm.  Options are "MIGRAD", "SIMPLEX", and "MINIMIZE".
* param Minimizer [DEFAULT=] The function minimizer. Options are "Brent", "GfxMinimizer", "Minuit", "Powell", and "Simplex"
* param MinimizeInLog(E) [DEFAULT=false] Minimizing in log(E) can improve the results when E varies slowly across the range.

* param MaxCalls [DEFAULT=50000] 
       Maximum number of function calls after which the minimizer will stop, even if it has not yet converged.

* param Tolerance [DEFAULT=0.01] 
       Required tolerance on the function value at the minimum.

* param Hesse [DEFAULT=false] 
       Calculate the Hessian error matrix.  Set to true or false.

* param Minos [DEFAULT=false] 
       Calculate the exact (non-linear) parameter error analysis.  Set to true or false.

* param Improve [DEFAULT=false] 
       Search for a new minimum around the current minimum.  Set to true or false.

* param Cutoff distance [DEFAULT=1000] Distance (from vertex) after which hits will not contribute the minimization

* param ResultName [DEFAULT="CscdLlh"] 
       Key for the RecoResultDict entry that holds the result of the reconstruction.
 
* param ParamT [DEFAULT=""] 
       Step size (SS), lower limit (LL), upper limit (UL), and &quot;<i>fix</i>&quot; for the cascade parameters.  To fix a parameter, set <i>fix</i> to "true"; to leave it free, set it to "false".  The string format is &quot;SS, LL, UL, <i>fix</i>&quot;.  To prevent limits from being used, set LL = UL = 0.  Using limits is not recommended, so the strings will usually be something like "0.5, 0.0, 0.0, false".
* param ParamX [DEFAULT=""]  see ParamT
* param ParamY [DEFAULT=""]  see ParamT
* param ParamZ [DEFAULT=""]  see ParamT
* param ParamZenith [DEFAULT=""]  see ParamT
* param ParamAzimuth [DEFAULT=""]  see ParamT
* param ParamEnergy [DEFAULT=""]  see ParamT

* param PDF [DEFAULT=""] 
       The probability density function.  Options are "UPandel", "UPandelMpe", "HitNoHit", and "HitNoHitMpe".
* param PndlHnhWeight [DEFAULT=1] The weight of the Hit/No-hit factor of the PndlHnh PDF. The weight of the UPandel factor is always 1.0.

* param Particle Type [DEFAULT=2] The particle type for photonics calls. Options are 1--muon, 2--ems, 3--hadronic.
* param DriverFile [DEFAULT="level2_cascade_photorec_table.list"] The Driver File (tables list) for photonics
* param Driver File [DEFAULT="level2_cascade_table.list"] The Driver File (tables list) for photonics
* param PhotonicsLevel [DEFAULT=2] The leve of the photonics tables options are 1, or 2.
* param Photonics Verbosity [DEFAULT=0] The verbosity setting for photonics
* param PhotonicsVerbosity [DEFAULT=0] The verbosity setting for photonics

* param GfxMinimizerFileName [DEFAULT="CscdLlhPlots.root"] The graphical minimizer dumps the llh and minimization parameters to aroot file. This setting provides it with a filename. 

* param Legendre Coefficient 0 [DEFAULT=0.0455] The zeroth coefficient for the legendre polynomial
* param Legendre Coefficient 1 [DEFAULT=0.909] The first coefficient for the legendre polynomial
* param Legendre Coefficient 2 [DEFAULT=0.0455] The second coefficient for the legendre polynomial
* param LightSpeedInIce [DEFAULT=ICE_LIGHT_SPEED] The speed of light in ice

* param NoiseProbability [DEFAULT=0.005] The probability that a hit was due to noise
* param SmallestProbability [DEFAULT=1e-9] This parameter defines a lower limit on the probability. probabilities with extremely small probabilities candestroy the minimization with too many NANs.
* param ExcludedOMs [DEFAULT= ] This is a vector of OMs that are to be excluded from contributing to the likelihood. This is necessary for PDFs that need to know about un-hit OMs.

**The following parameters refer to the both SPE and MPE UPandel PDF's:**

* param PandelTau [DEFAULT=450] 
         Pandel time parameter.
* param PandelLambda [DEFAULT=47] 
         Pandel length parameter.
* param PandelLambdaA [DEFAULT=98] 
         The photon absorption length.
* param PandelSigma [DEFAULT=15] 
         Patched Pandel gaussian width, due to PMT jitter.
* param PandelLightSpeed [DEFAULT=I3Constants ::c/I3Constants ::n_ice] The speed of light in ice.
* param PandelMaxDist [DEFAULT=500] If the distance from the hit to the cascade vertex is greater than "PandelMaxDist", the probability will be Set to "PandelSmallProb". If PandelMaxDist is less than or equal to zero, it will be ignored.
* param PandelSmallProb [DEFAULT=1e-10] The minimum probability (a small positive definite number.)

**The following parameters refer to the both SPE and MPE Hit/No-hit PDF's:**

* param HitNoHitNorm [DEFAULT=1.4] 
         A normalization constant for the mean number of photoelectrons.
* param HitNoHitLambdaAttn [DEFAULT=29] 
         The photon attenuation length.
* param HitNoHitNoise [DEFAULT=0.005] 
         The probability that an OM will obtain a hit due to noise.
* param HitNoHitDistCutoff [DEFAULT=0.5] 
         A small distance parameter that is used to handle the divergence in the mean number of photoelectrons for an OM at the cascade vertex.  The distance d is replaced by :math:`\sqrt{d^2 + d_\mathrm{cutoff}^2}`.
* param HitNoHitDead [DEFAULT=0.05] 
         The probability that an OM will not register a hit, regardless of the light intensity.
* param HitNoHitSmallProb [DEFAULT=1e-40] The minimum probability (a small positive definite number.) 1.0e-40


`Generated doxygen for this project <../../doxygen/cscd-llh/index.html>`_

