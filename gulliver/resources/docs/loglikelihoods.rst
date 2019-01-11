================================================
How to implement your own log-likelihood service
================================================

Overview
--------

The likelihood function describes the physical and statistical
properties of the emission hypothesis for which you are trying to
constrain some parameters, as well as the detector that provides you
with data. For a Gulliver reconstruction this description is
implemented in a likelihood service. Such a service is specifically
targeted at one or more particular emission hypotheses.

For the likelihood of the data, given a particular event hypothesis with
specific numerical values for its variables, there are several aspects
to consider:

#. How much light is emitted, from where, in which direction(s)?
#. How does this light propagate through the detector medium?

   * How many photoelectrons will be registered at each DOM?
   * What is the distribution of the photoelectron detection times?

#. How much uncorrelated data (noise) can we expect in each DOM?

All these aspects need to be dealt with in a statistical way, i.e. with
probabilities and probability densities. We also need to consider
limitations on computing resources. For a fast reconstruction used in
the online filtering at the Pole we need to make many simplifications,
whereas for a reconstruction on a couple of events in the final level
data sample we can make it in principle as fancy as we like.

Apart from the data you may also have some prior knowledge/expectation
of the possible values of the physical variables that you are trying to
fit. Example of "prior likelihoods":

* The normalized zenith distribution of all muons triggering IceCube,
  both from down-going cosmic ray muons and from atmospheric muon
  neutrinos.
* The probability distribution of the distance of the light emitting
  phenomenon to the "cloud of hits".

Gulliver provides a likelihood combiner service, which allows you to
combine e.g. an unbiased likelihood function (likelihood of the event
data, given a hypothesis) and a prior (likelihood of the hypothesis,
regardless of the event data).

Interface
---------

Event log-likelihood services for use with Gulliver-based modules must
be implemented as subclasses of :class:`I3EventLogLikelihoodBase`.

SetGeometry
^^^^^^^^^^^

This method serves to set up the geometry information relevant for the
data that is going to be processed in a particular processing job. In
the implementation of your likelihood calculations you may of course use
the (smart pointer to the object of class) :class:`I3Geometry` directly,
but in some cases it may be more desirable to store the information in a
customized format.

This method usually gets called only once per job, namely once for every
G-frame, but you may e.g. have a final level data file for one or
several years that has several of those. You get a pointer to the actual
geometry in every P-frame, but if you use a customized format then it
would be inefficient to do this conversion every time.

Some likelihood services (such as ``millipede``) also want to use the
calibration and detector status information, so to be consistent the
:class:`I3EventLogLikelihoodBase` should also have a ``SetCalibration``
and ``SetDetectorStatus`` method. It was decided not to do that (in
order to avoid a too baroque interface), but maybe that is something
that should be reconsidered.

Event data
^^^^^^^^^^

In the ``SetEvent`` method the event data should be collected from the
frame. (For a prior likelihood this is a no-op.)

Usually, for IceCube reconstructions (these docs are getting written in
2014), this means that you get an :class:`I3RecoPulseSeriesMap` (or a
mask) and store it in any format that allows efficient computation of
likelihoods later on. So why does this method come with a
:class:`I3Frame` argument? Well:

* In the early days of IceCube, we also considered likelihoods based on
  AMANDA data (mudaq or TWR daq), on calibrated pdaq waveforms, or on
  "reco hits".

  .. note::

    A :class:`I3RecoHit` and a :class:`I3RecoPulse` are almost the same
    thing, but a :class:`I3RecoHit` does not have a charge and was
    thought to represent exactly one measured photoelectron, which is an
    interesting idea but turned out to be a bit too theoretical for the
    practical reality.

* Gulliver is also used for KM3NET, where the event data comes in data
  structures other than pulse maps.
* In the future, Gulliver may be used for PINGU and/or high-energy
  extensions, which may develop yet another data structure for the event
  data.

Like with the geometry, you can choose to just keep a pointer to the
:class:`I3RecoPulseSeriesMap` or store the data in a custom format. In
either case you need to make sure that after a call to the ``SetEvent``
method the service can return a reasonable value for the *multiplicity*
(``GetMultiplicity`` method). This is a quantity that depends on the
specifics of your likelihood model, it might for instance be the number
of hit DOMs. It is used by Gulliver for two things:

#. Decide whether to perform a fit or not: the number of degrees of
   freedom (multiplicity minus the number of free fit variables) must be
   positive.
#. Compute the *reduced likelihood* (the likelihood of the final fit
   result divided by the number of degrees of freedom).

Log-likelihood
^^^^^^^^^^^^^^

The main job of a likelihood service is of course to compute (log-)
likelihoods. Make sure that the likelihood function is indeed a proper
function, meaning that the return value really only depends on the event
data and on the event hypothesis. If it depends on any other state
variable (like random numbers, the wall clock time, the Dow Jones index)
then the minimizer will get confused and not be able to perform a
reliable fit.

The log-likelihood may be any finite floating point number, positive or
negative. Larger values (less negative or more positive) indicate
"good" likelihood, lower values (more negative or less positive)
indicate "worse" likelihood. There is sometimes confusion about this
because the minimizer is **minimizing** the **negative** log-likelihood.
Remember: the event log-likelihood service should return
:math:`+\log(\mathcal{L})`, Gulliver will multiply that with :math:`-1`
before giving it to the minimizer. (Same holds for the gradient.)

The log-likelihood should also never be ``NAN``. There is really no good
excuse to ever return ``NAN``; make sure that your implementation is
indeed robust enough that it never does that. If the event hypothesis is
of the wrong type or it has data members that are ``NAN`` while they
should not, then throw a ``log_fatal`` (with an informative error
message), because this should never happen unless one or more of the
Gulliver services are somehow wrongly configured, in which case the job
should be aborted in order to force the user to fix the script. If the
likelihood service *does* return ``NAN`` then Gulliver will catch that
and terminate the job with a much less informative error message.

Gradients
^^^^^^^^^

A likelihood service may or may not support gradients. The availability
of gradients should be specified in the documentation and through the
boolean ``HasGradients`` method.

During job configuration, Gulliver asks the minimizer service whether it
wants to use gradients, and if so, whether both the parametrization
service (chain rule) and the likelihood service support that.

The actual gradient consists of partial derivatives, one derivative for
*every* variable in the event hypothesis. Hence, the value of the
gradient is also stored in an object of :class:`I3EventHypothesis`! For
instance, the partial derivative with respect to the :math:`x`-component
of the position of the ``particle`` data member of the event hypothesis
is stored in the :math:`x`-component of the position of the ``particle``
data member of the gradient. If the event hypothesis has a ``nonstd``
data member that is used in the likelihood calculation, then the
gradient should have a data member of the same type and the gradient
calculation should deliver values for the partial derivatives in that
data member as well. *All* data members that are irrelevant for the
likelihood calculation should be set to zero.

Diagnostics
^^^^^^^^^^^

Any Gulliver service may or may not provide diagnostics. The diagnostics
are completely specific to the service and may be stored in any kind of
:class:`I3FrameObject`. This can be e.g. a simple :class:`I3Bool`, a
dedicated struct-like or more fancy class (if you do that, please be
nice and also provide py-bindings), or e.g. a lazy
:class:`I3MapStringDouble`.

Items that could possibly be of interest in the diagnostics:

* More sophisticated multiplicity information (e.g. number of time bins
  for ``millipede``-like likelihoods)
* Number of noise DOMs
    DOMs with pulses that seem unrelated to the event hypothesis.
* Number of signal DOMs
    DOMs with pulses that seem sufficiently related to the event
    hypothesis.
* Number of times that the calculation had to work around some problem,
  e.g. when evaluating a likelihood based on spline tables and some
  kinematic situation was out of reach of those tables.
