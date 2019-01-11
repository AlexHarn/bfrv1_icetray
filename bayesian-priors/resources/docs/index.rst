.. _bayesian-priors:

===============
Bayesian Priors
===============

This project contains Gulliver likelihood services which add a bias (zenith
weight) to reconstructions. These biased reconstructions do not produce a better
estimate of the track direction, but rather are useful for distinguishing
up-going tracks for down-going tracks.
This is refered to as a "Bayesian" reconstruction because multiplying a
liklihood by a function which only depends on reconstruction parameters
(as opposed to data) is reminiscent of a Baysian prior, not due to any correct
application of Bayes' Theroem.

**Maintainer** : Kevin Meagher
   
.. toctree::
   :maxdepth: 1

   release_notes


Bayesian Priors
---------------

Bayes law states:

.. math::
  P(H|E) = \frac{P(E|H)P(H)}{P(E)}

Where :math:`H` is the track hypothesis and :math:`E` is the hit pattern observed in the detector. :math:`P(E)` is a normalization and can be ignored.
:math:`P(E|H)` is the probability of observing the hit pattern given a track hypothesis and :math:`P(H)` is a probability of observing a track :math:`H` independent of the observed hit pattern :math:`E`. The likelihood, :math:`\Lambda(E;H)` from IceCube reconstructions can be substituted for :math:`P(E|H)`, thus multiplying a reconstruction likelihood by a probability which depends only on :math:`H` is referred to as a bayesian reconstruction.

:math:`P(H)` is referred to as a prior probability. In Bayesian inference, the prior probability distribution is probability that would express one's beliefs about this quantity before some evidence is taken into account. In this context it is the probability of observing a track completely independent of of the hit pattern observed in the detector. Adding the prior to the adds a substantial bias to reconstruction, but greatly improves the ability to distinguish up-going tracks from down-going tracks, especially near the horizon.

This technique was first developed by the NEVOD neutrino detector collaboration who were able to extract an atmospheric neutrino from a background of 10\ :sup:`10` atmospheric muons in a small (6×6times×7.5 m\ :sup:`3`) neutrino detector on the surface\ [#]_. This technique was independently developed for the AMANDA experiment\ [#]_\ [#]_\ [#]_. 


Pure Bayesian priors
^^^^^^^^^^^^^^^^^^^^

The pure Bayesian priors (in IceCube/AMANDA reconstruction) try to model
the zenith distribution of downgoing muons (bundles) induced by cosmic rays,
and/or atmospheric neutrinos. We have currently three functions which attempt
to model the zenith distribution of downgoing muons:

 - :js:data:`I3PolynomialZenithWeightServiceFactory`: a polynomial of arbitrary (default: 7th) order in :math:`\cos(zenith)`
 - :js:data:`I3TableZenithWeightServiceFactory`: a table in :math:`cos(zenith)`, from which the weight is interpolated
 - :js:data:`I3PowExpZenithWeightServiceFactory`: a 3-parameter function :math:`w=a0*pow(costh,a1)*exp(-a2/costh)`

These function represent a `Zenith` distribution of down-going cosmic rays as observed by the AMANDA detector\ [#]_. The atmospheric neutrino flux is modeled as a constant value all over sky. 
The atmospheric neutrino flux is not constant as a function of zenith but it is close enough that this approximation should be fine

In this purely Bayesian approach, an up-going track should only
be found by the minimizer if its regular likelihood is so good that it
is found by the minimizer even with the bias terms added (which favor
down-going tracks), i.e. its likelihood should be about five or six
orders of magnitude better than the best down going track.

However, it turns out that also a significant number of downgoing
corsika tracks "survive" this reconstruction as an upgoing track.

Bayesian priors to restrict phase space
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Bayesian priors can also be used to enforce e.g. a particular zenith
range for the reconstruction, e.g. with the :js:data:`I3ConstantZenithWeightServiceFactory`
prior. This can also be achieved in other ways, for instance via the
definition of the range of minimizer parameters, but this then also
necessitates fixes in the seed generation. The phase space selection
used to be done with a constant large positive penalty term to :math:`-\log(\Lambda)`,
but it turns out that this still results in solutions in the "forbidden
zone", because of local minima. A different approach is to apply
a penalty term which a very steep slope towards the allowed zone,
so that whenever the seed track or a minimizer-generated track is in
the forbidden zone, it will always "roll" back to the allowed zone
of phase space (think of the minimizer solution as a marble rolling
in a hilly log-likelihood landscape).

For all zenith weights implemented in this project it is possible to
specify both a constant penalty value :math:`c` and a "slope" value :math:`s`.
In the forbidden range the penalty is then :math:`c+s*abs(CosZenDiff)`,
where :math:`CosZenDiff` denotes the difference in :math:`\cos(zenith)` between the
:math:`\cos(zenith)` of the current event hypothesis and the closest allowed
:math:`\cos(zenith)` value.

Both
^^^^

What turns out to work quite well for finding good upgoing muon
tracks is to define a reconstruction in which we simply assume that
the event was a down going muon, to apply both the zenith weight
which models the CR spectrum (e.g. :js:data:`I3PowExpZenithWeightServiceFactory`), as well
as the phase space enforcing terms, i.e. a sloped penalty term for
upgoing tracks.  The likelihood difference of this reconstruction
and an unbiased reconstruction of the same event, with an upgoing
result, can be used as a discriminator between true upgoing events
and downgoing events misreconstructed as upgoing.

Bayesian priors usage
---------------------

This project only implements the priors. To use them in a Gulliver
reconstruction, you instantiate them as a service in the tray with
an ``tray.AddService(...)`` line in your python script, just like you
do for your regular likelihood function (e.g. :js:data:`I3GulliverIPDFPandelFactory`).
Then you combine the prior and the regular likelihood function using
the :js:data:`I3EventLogLikelihoodCombinerFactory` service from the gulliver project.
See the ``bayesianfit.py`` example script under ``resources/scripts``
to get the idea.


Use as a Quality Cut
^^^^^^^^^^^^^^^^^^^^^
In IceCube, the Bayesian reconstruction is used distinguish, up-going tracks
from down-going tracks, specifically  to calculate 
a quality parameter for cutting background in up-going muon analyses,
either as a direct cut or fed to a BDT. The so-called ``bayesratio``
is calculated as :math:`\log(\Lambda_{bayes})-\log(\Lambda_{normal})`.
Typal cut values for ``bayesratio`` are around 25-30\ [#]_. 


Avoiding Inappropriate Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Bayes Reconstruction is a biased estimator of a muon tracks location, which means its expectation value is not equal to its true value. Thus, it is highly undesirable for use as the best guess for the location of a track. For upgoing tracks, the Bayesian reconstruction is often very far from the unbiased reconstruction, it is unlikely to be to be mistaken as a good reconstruction. However, for down-going only analyses the bias is small but still present. 

Recall from statistics that estimators are usually judged on three criteria: consistancy, bias, and variance.  It it a widly know property that in some cases a biased estimator will have a smaller variance than an unbiased estimator\ [#]_.
In the case of the Bayesian reconstruction the zenith distribution is biased and the variance of the zenith distribution in smaller than for a non-zenith weighted reconstruction. Since the parameter ``deltaangle``, the average angular distance between the Monte Carlo truth direction and the reconstructed direction, is essencially the combination of the zenith variance and the azimuth variance, it too will be smaller than the ``deltaangle`` for the unbiased reconstruction. 
This situation has led to more than one IceCube collaborator trying to use Bayesian reconstructions in a down-going analysis on the sole justification that ``deltaangle`` was smaller. Or even worse, incorrectly claiming that the Bayesian reconstruction was more accurate because it incorperates the prior distribution, which contains more information. Please do not make either of these mistakes. You almost certainly do not want to use a biased reconstruction. 


See Also
--------

.. toctree::
   :maxdepth: 1
	      
   Python API Reference </python/icecube.bayesian_priors>
   C++ API Reference </cpp/bayesian-priors>
   IceTray Inspect Reference </inspect/bayesian_priors>
    

References
---------------

.. [#] V. M. Aynutdinov, et al., 
 "Technique of Neutrino-Induced Muon Detection on the Earth Surface,"
 Physics of Particles and Nuclei Letters 109 (2001) 43
 http://www.iaea.org/inis/collection/NCLCollectionStore/_Public/33/016/33016351.pdf

.. [#] T. R. de Young, "Observation of Atmospheric Muon Neutrinos with AMANDA",
 Ph.D. thesis",
 University of Wisconsin at Madison, Madison, Wisconsin, 2001, 
 (discussed in Section 5.2 pp. 44-47).
 http://icecube.berkeley.edu/manuscripts/20010501xx-ty_thesis.pdf

.. [#] G. C. Hill, 
 "Bayesian event reconstruction and background rejection in neutrino detectors",
 Proceedings of the 27th International Cosmic Ray Conference, Hamburg, 2001, 
 Vol. 3, pp. 1279–1282, HE.267.
 `astro-ph/0106081v1 <http://arxiv.org/pdf/astro-ph/0106081v1.pdf>`_

.. [#] T. R. de Young and G. C. Hill for the AMANDA Collaboration,
 "Application of Bayesian Statistics to Muon Track Reconstruction in AMANDA,"
 Proceedings of the Conference On Advanced Statistical Techniques In Particle Physics, Durham, UK, 18-22 March 2002, 235-241
 http://www.ippp.dur.ac.uk/Workshops/02/statistics/proceedings/deyoung.pdf

.. [#] Paolo Desiati and Karen Bland for the AMANDA Collaboration,
 "Response of AMANDA-II to Cosmic Ray Muons,"
 Proceedings of the 28th International Cosmic Ray Conference, Tsukuba, Japan, 31    July - 7 August 2003, HE 2.3, 1373-1376.
 http://www-rccn.icrr.u-tokyo.ac.jp/icrc2003/PROCEEDINGS/PDF/342.pdf

.. [#] For example see:
       https://wiki.icecube.wisc.edu/index.php/IC-22_Point_Source_Study/Cuts
       http://umdgrb.umd.edu/~kjmeagher/IceCube/IC40_GRB_Analysis/Cuts/index.html
       https://wiki.icecube.wisc.edu/index.php/IC40_Diffuse_Analysis_Event_Selection_and_Data/MC_comparison
       https://wiki.icecube.wisc.edu/index.php/IC-40_Point_Source_Analysis/Internal/Downgoing0405
       https://wiki.icecube.wisc.edu/index.php/IC59_PS_Event_Selection_Cuts#bayes

.. [#] \R. J. Barlow "Statistics: A Guide to the Use of Statistical Methods in the Physical Sciences," Wiley, 1989, chapter 5.       
