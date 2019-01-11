..
.. copyright  (C) 2011-2012
.. The Icecube Collaboration
..
.. $Id: index.rst 115550 2014-01-15 17:29:03Z jvansanten $
..
.. @version $Revision: 115550 $
.. @date $LastChangedDate: 2014-01-15 18:29:03 +0100 (Wed, 15 Jan 2014) $
.. @author Markus Voge <voge@physik.uni-bonn.de> $LastChangedBy: jvansanten $

.. _spline-reco-main:

Spline-reco
=================

Muon track reconstruction with spline tables.
This project is maintained by Markus Voge <voge@physik.uni-bonn.de>.

`Generated doxygen for this project <../../doxygen/spline-reco/index.html>`_

.. toctree::
   :maxdepth: 3

   release_notes

Intro
^^^^^

Spline-reco is a project providing muon track reconstruction that
makes use of tables containing the ice properties. Those tables
contain the parameters of spline fits to the actual ice
properties (absorption and scattering length), hence the name
spline-reco. See project *photospline* for more infos on the spline
tables. Spline-reco also has the option to model module noise which
may be found to enhance resolution of the reconstruction.

Spline-reco was found to deliver an angular reconstruction quality
superior to the previous de-facto standard, MPE reconstruction using
Pandel functions to construct the likelihood. Spline-reco provides a
better likelihood function, based on the photospline tables. The
likelihood is used by a fitter module that does the actual fitting by
minimizing it. You can choose if you want to construct an MPE (multi
photo-electron) or SPE (single photo-electron) likelihood, amongst
other options. You can find more information as well as comparisons of
angular resolution to other reconstruction algorithms, using different
settings, on this wiki page:
https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction.
There, you also find information on adequate spline tables to use for muon
reconstruction.

This project provides a service factory I3SplineRecoLikelihoodFactory
that constructs an instance of the I3SplineRecoLikelihood class
internally. Use the I3SplineRecoLikelihoodFactory as input for a
fitter module, e.g. install it as "LogLikelihood" in the
"I3SimpleFitter" or "I3IterativeFitter" from project
*gulliver-modules*. There are two example scripts provided: one
simple, fast running script, and one that is more elaborate and
computing intensive.

Running spline-reco
^^^^^^^^^^^^^^^^^^^

Please look at the example scripts in
*$I3_SRC/spline-reco/resources/examples* for a quick start using
spline-reco in an icetray script. Basically, one installs the
*I3SplineRecoLikelihoodFactory* and uses it as LogLikelihood in an
*I3SimpleFitter* or *I3IterativeFitter*.

Options
^^^^^^^

Here is an explanation of the parameter options of the
I3SplineRecoLikelihoodFactory.

1. PhotonicsService
      Name of the I3PhotoSplineServiceFactory. This is a *must*,
      because the spline-reco likelihood is based on the spline
      tables. Configure the I3PhotoSplineServiceFactory and set which
      tables you want to use.

      Look at the scripts in resources/examples/ and on
      https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction
      for information about what spline tables to use and how to
      produce own tables.

2. PhotonicsServiceRandomNoise
      Name of an I3PhotoSplineServiceFactory that is used for random
      noise modelling.

      You might want to apply a 1000 ns convolution
      to bare muon spline tables, take a look at
      resources/examples/spline-reco_slow.py.

3. PhotonicsServiceStochastics
      Name of an I3PhotoSplineServiceFactory that is used to model
      stochastic energy losses.

      Look at the scripts in resources/examples/ and on
      https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction
      for information about what spline tables to use and how to
      produce own tables.

4. Pulses
      Name of the I3RecoPulseSeriesMap to use. This is a *must* as
      well.

5. Likelihood
      Specify which likelihood kind you want to use. Options are: MPE, SPE1st,
      MPEAll, SPEAll. MPE means "multi photo-electron". It uses only the time
      of the first pulse, but also the total charge on the DOM (integrating all
      pulses or number of PEs). SPE1st means "first single photo-electron",
      i.e. only the first pulse of each DOM is considered, the total charge is
      ignored (treated as if only a single PE hit the PMT). MPE and SPE1st both
      use only the first pulse time on each DOM. MPEAll uses all pulse times
      and the total charge on the DOM. SPEAll also uses all pulse times, but
      ignores charge. If **ChargeWeightedLLH** is set to True, SPEAll
      also considers the charge of each individual pulse.

      Default is SPE1st, but recommendation is to use MPE, as it seems
      to give good results.

6. NoiseModel
      Select what noise modelling shall be done. Options are: none,
      flat, HLC, SRT. The default is "none", i.e. no noise modeling is
      done. If it is not set to "none",
      **PhotonicsServiceRandomNoise** and **FloorWeight** must be set
      as well.

.. note::

      Noise modelling of the PDF/CDF is only done if **E_Estimators**
      are set!

7. FloorWeight
      Height of the constant noise floor (in PEs, I think?), only
      needed when **NoiseModel** is not "none".

8. ModelStochastics
      Boolean to select if stochastics spline tables are used or not.
      Default is False.

.. note::

      Stochastics modelling is only done if **E_Estimators** are set!

9. NoiseRate
      Rate of noise hits in Hertz. Also needs to be set if
      **NoiseModel** is set to "none". Default is 10 Hz.

10. E_Estimators
      List/Vector with names of I3Particles that contain energy
      estimation. The arithmetic mean of all provided energy estimates
      is calculated and used in the spline likelihood calculation, for
      the modelling of light from stochastic energy losses. Only used
      if **ModelStochastics** or **NoiseModel** is not "none".

11. ChargeWeightedLLH
      Boolean to control if charge weighted SPE likelihood shall be
      used. This only takes effect if SPEAll is selected for
      **Likelihood**. Default is False.

12. PreJitter
      Jitter applied directly to the time residual of the inner
      photospline PDF. The photospline PDF is convolved over this
      jitter times the jitter width of 3 ns. Convolution is only done
      if time residual is larger than -PreJitter * 3 ns - 5 ns and if
      it's smaller than 40 ns. Default for PreJitter is 0.

13. PostJitter
      Jitter applied to the time residual of the outer hit likelihood
      (i.e. MPE or SPE likelihood). The MPE/SPE likelihood (containing
      the PreJitter convolved photospline PDF) is convolved over this
      jitter times the jitter width of 3 ns. Convolution is only done
      if time residual is larger than -PreJitter * 3 ns - 5 ns and if
      it's smaller than 40 ns. Default for PostJitter is 0.

14. KSConfidenceLevel
      Confidence Level for the Kolmogorov-Smirnov tested charge sum. Using a KS
      test, only certain charge and pulses are approved and used in the MPE,
      MPEAll and SPEAll with ChargeWeight reconstructions (late pulses can be
      chopped off). KSConfidenceLevel should be from [0,1,2,3,4,5] where 5 =
      80% CL, 4 = 85%, 3 = 90%, 2 = 95% or 1 = 99%. For KSConfidenceLevel = 0,
      the total charge is used without KS test (Default). Tests showed that
      KSConfidenceLevel = 5 with **CutMode** = "late" works best for nugen MC.

15. ChargeCalcStep
      Iterations between two charge calculations. ChargeCalcStep = 0
      calculates the KS approved charge (see **KSConfidenceLevel**)
      only once at the first GetLogLlh call, ChargeCalcStep > 0
      calculates it every ChargeCalcStep minimization steps.

16. CutMode
      There are 3 different KS models: default, normalized, late.
      Tests showed that "late" with **KSConfidenceLevel** = 5 works
      best for nugen MC.


Components
^^^^^^^^^^

I3SplineRecoLikelihood
----------------------

Class doing the actual likelihood calculation work. It is however only
used internally and interfaced by the icetray service factory.

I3SplineRecoLikelihoodFactory
-----------------------------

The I3_SERVICE_FACTORY that is installed in the icetray. This is the
interface used by the user. The parameter options are summarized
above.

gaussianiir1d
-------------

Fast 1D Gaussian convolution using IIR approximation. Implements the
fast Gaussian convolution algorithm of Alvarez and Mazorra (1994),
where the Gaussian is approximated by a cascade of first-order
infinite impulsive response (IIR) filters. Boundaries are handled
with half-sample symmetric extension.

This is external C code written by Pascal Getreuer and published under
the GNU General Public License.

