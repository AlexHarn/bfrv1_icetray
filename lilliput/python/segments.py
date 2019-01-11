from icecube import icetray, lilliput, gulliver, paraboloid
from icecube.icetray import I3Units


def add_minuit_simplex_minimizer_service(tray, minimizer=None):
    """Add Minuit (Simplex) minimizer service to tray.

    We need only one Minuit minimizer service in the tray, this can be
    used by all `gulliver` fitters that need it.

    Parameters
    ----------
    minimizer : I3MinimizerBase or str, optional
        If not `None`, use external minimizer service.

    Returns
    -------
    I3MinimizerBase or str
        Minimizer service; default: ``default_simplex``

    """
    if minimizer is None:
        minimizer = "default_simplex"

        if (minimizer not in tray.tray_info.factory_configs and
                minimizer not in tray.context):
            tray.context[minimizer] = lilliput.I3GulliverMinuit(
                name=minimizer,
                tolerance=0.001,
                max_iterations=1000,
                algorithm="SIMPLEX")

    return minimizer


def add_simple_track_parametrization_service(tray, parametrization=None):
    """Add track parametrization service to tray.

    We need only one track parametrization service in the tray, this can
    be used by all `gulliver` fitters that need it.

    Parameters
    ----------
    parametrization : I3ParametrizationBase or str, optional
        If not `None`, use external track parametrization service.

    Returns
    -------
    I3ParametrizationBase or str
        Track parametrization service; default: ``default_simpletrack``

    """
    if parametrization is None:
        parametrization = "default_simpletrack"

        if (parametrization not in tray.tray_info.factory_configs and
                parametrization not in tray.context):
            tray.Add("I3SimpleParametrizationFactory", parametrization,
                     StepX=20.*icetray.I3Units.m,
                     StepY=20.*icetray.I3Units.m,
                     StepZ=20.*icetray.I3Units.m,
                     StepZenith=0.1*I3Units.radian,
                     StepAzimuth=0.2*I3Units.radian,
                     BoundsX=[-2000.*I3Units.m, 2000.*I3Units.m],
                     BoundsY=[-2000.*I3Units.m, 2000.*I3Units.m],
                     BoundsZ=[-2000.*I3Units.m, 2000.*I3Units.m])

    return parametrization


def add_pandel_likelihood_service(tray, pulses, domllh="SPE1st",
                                  noiserate=10.*I3Units.hertz):
    """Add Pandel service to tray.

    For every desired Pandel configuration we need at most one instance
    of the Pandel service in the tray. This service can be used by all
    `gulliver` fitters that need it.

    With this function you can do *some* configuration, but not all. If
    you want to do more fancy configuration, then you'll need to add the
    service yourself instead of using this convenience function.

    Parameters
    ----------
    pulses : str
        Name of pulse map
    domllh : {"SPE1st", "SPEAll", "SPEqAll", "MPE"}, optional
        Likelihood function
    float noiserate : float, optional
        Noise rate

    Returns
    -------
    str
        Name of Pandel service

    """
    noisetxt = "{0:.1f}Hz".format(noiserate)
    llh_name = "_".join(["Std", domllh, pulses, noisetxt])

    if (llh_name not in tray.tray_info.factory_configs and
            llh_name not in tray.context):
        if domllh not in ["SPE1st", "SPEAll", "SPEqAll", "MPE"]:
            raise ValueError("Invalid domllh name {0}; should be one of "
                             "SPE1st, SPEAll, SPEqAll, MPE.".format(domllh))

        if domllh == "MPE":
            jitter = 4.*I3Units.ns
        else:
            jitter = 15.*I3Units.ns

        tray.Add("I3GulliverIPDFPandelFactory", llh_name,
                 InputReadout=pulses,
                 EventType="InfiniteMuon",
                 Likelihood=domllh,
                 PEProb="GaussConvolutedFastApproximation",
                 JitterTime=jitter,
                 NoiseProbability=noiserate)

    return llh_name


def add_seed_service(tray, pulses, seeds=[], tstype="TFirst"):
    """Add seed service to tray.

    For every desired seeding configuration we need at most one instance
    of the basic seed service in the tray. This service can be used by
    all `gulliver` modules that need it.

    Parameters
    ----------
    pulses : str
        Name of pulse map
    seeds : list
        Seed names
    tstype : str, optional
        Use ``"TNone"`` if seed has a reliable vertex time.

    Returns
    -------
    str
        Name of seed service

    """
    seed_name = "_".join(["Std", "SeedService", pulses, tstype] + seeds)

    if (seed_name not in tray.tray_info.factory_configs and
            seed_name not in tray.context):
        tray.Add("I3BasicSeedServiceFactory", seed_name,
                 InputReadout=pulses,
                 FirstGuesses=seeds,
                 TimeShiftType=tstype)

    return seed_name


@icetray.traysegment
def I3SinglePandelFitter(tray, name, fitname="", pulses="OfflinePulses",
                         seeds=[], minimizer=None, parametrization=None,
                         domllh="SPE1st", tstype="TFirst",
                         noiserate=10.*I3Units.hertz, If=None):
    """Run a single Pandel fit.

    Use standard settings as for IC40-IC79 and bulk ice properties.

    Log-likelihood functions:

        * SPE1st: use time of first pulse in each DOM.
        * SPEAll: use times of all pulses in each DOM.
        * SPEqAll: use charge-weighted times of all pulses in each DOM.
        * MPE: use time of first pulse and total charge in each DOM.

    Parameters
    ----------
    fitname : str
        Fit name
    pulses : str
        Name of pulse map
    minimizer : I3MinimizerBase or str, optional
        Minimizer service
    parametrization : I3ParametrizationBase or str, optional
        Track parametrization service
    seeds : list
        Seed names
    domllh : {"SPE1st", "SPEAll", "SPEqAll", "MPE"}, optional
        Likelihood function
    tstype : str, optional
        Use ``"TNone"`` if seed has a reliable vertex time.
    noiserate : float, optional
        Noise rate

    Returns
    -------
    tuple
        Names of services and fit for re-usage convenience

    Warnings
    --------
    Although SPEAll and SPEqAll were thought to be fundamentally
    correct, they work surprisingly worse than SPE1st.

    Notes
    -----
    The default noise rate of 10Hz does not have any physical basis. It
    was set at the time when we used only HLC pulses and based on flawed
    logic (the HLC dark noise rate was estimated to be of order 1Hz and
    then 10Hz seemed sort of conservative). Studies on low filtering
    level indicate that higher noise level reduce the rate of
    mis-reconstructed background, while at high cut level the effect of
    this setting on well reconstructed neutrino events seems to be
    minor. New studies and developments are underway (as of November
    2011).

    """
    icetray.logging.log_trace("name={0}".format(name), unit="lilliput")

    if len(seeds) == 0:
        icetray.logging.log_fatal("{0} is missing a seed list.".format(name),
                                  unit="I3SinglePandelFitter")

    minimizer = add_minuit_simplex_minimizer_service(
        tray, minimizer=minimizer)

    parametrization = add_simple_track_parametrization_service(
        tray, parametrization=parametrization)

    likelihood = add_pandel_likelihood_service(
        tray, pulses=pulses, domllh=domllh, noiserate=noiserate)

    seed = add_seed_service(
        tray, pulses=pulses, seeds=seeds, tstype=tstype)

    tray.Add("I3SimpleFitter", name,
             SeedService=seed,
             Parametrization=parametrization,
             LogLikelihood=likelihood,
             Minimizer=minimizer,
             OutputName=fitname,
             If=If)

    return minimizer, parametrization, likelihood, seed, name


@icetray.traysegment
def I3IterativePandelFitter(tray, name, fitname="", pulses="OfflinePulses",
                            n_iterations=4, minimizer=None,
                            parametrization=None, seeds=[], domllh="SPE1st",
                            tstype="TFirst", noiserate=10.*I3Units.hertz,
                            If=None):
    """Run an iterative Pandel Fit.

    Use standard settings as for IC40-IC79 and bulk ice properties.

    Parameters
    ----------
    fitname : str
        Fit name
    pulses : str
        Name of pulse map
    n_iterations : int, optional
        Number of iterations
    minimizer : I3MinimizerBase or str, optional
        Minimizer service
    parametrization : I3ParametrizationBase or str, optional
        Track parametrization service
    seeds : list
        Seed names
    domllh : {"SPE1st", "SPEAll", "SPEqAll", "MPE"}, optional
        Likelihood function
    tstype : str, optional
        Use ``"TNone"`` if seed has a reliable vertex time.
    noiserate : float, optional
        Noise rate

    Returns
    -------
    tuple
        Names of services and fit for re-usage convenience

    """
    icetray.logging.log_trace("name={0}".format(name), unit="lilliput")

    if len(seeds) == 0:
        icetray.logging.log_fatal("{0} is missing a seed list.".format(name),
                                  unit="I3IterativePandelFitter")

    minimizer = add_minuit_simplex_minimizer_service(
        tray, minimizer=minimizer)

    parametrization = add_simple_track_parametrization_service(
        tray, parametrization=parametrization)

    likelihood = add_pandel_likelihood_service(
        tray, pulses=pulses, domllh=domllh, noiserate=noiserate)

    seed = add_seed_service(
        tray, pulses=pulses, seeds=seeds, tstype=tstype)

    if tstype != "TFirst":
        tweaker = add_seed_service(tray, pulses, [], "TFirst")
    else:
        tweaker = ""

    tray.Add("I3IterativeFitter", name,
             SeedService=seed,
             Parametrization=parametrization,
             IterationTweakService=tweaker,
             LogLikelihood=likelihood,
             Minimizer=minimizer,
             NIterations=n_iterations,
             RandomService="SOBOL",
             CosZenithRange=[-1., 1.],
             OutputName=fitname,
             If=If)

    return minimizer, parametrization, likelihood, seed, name


@icetray.traysegment
def I3ParaboloidPandelFitter(tray, name, fitname="", pulses="OfflinePulses",
                             minimizer=None, parametrization=None,
                             inputtrack=None, domllh="SPE1st",
                             input_tstype="TFirst", grid_tstype="TFirst",
                             noiserate=10.*I3Units.hertz, If=None):
    """Run a Paraboloid fit with a Pandel likelihood.

    Use standard settings as used for OnlineL2 and bulk ice properties
    This tray segment only works for `gulliver` Pandel track fits.

    Parameters
    ----------
    fitname : str
        Fit name
    pulses : str
        Name of pulse map as for input track
    minimizer : I3MinimizerBase or str, optional
        Minimizer service
    parametrization : I3ParametrizationBase or str, optional
        Track parametrization service
    inputtrack : str
        Name of input track
    domllh : {"SPE1st", "SPEAll", "SPEqAll", "MPE"}, optional
        Likelihood as for input track
    input_tstype : str, optional
        Vertex time strategy as for input track
    grid_tstype : str, optional
        Use ``"TFirst"`` or ``"TChargeFraction"`` for grid.
    noiserate : float
        Noise rate as for input track

    Returns
    -------
    tuple
        Names of services and fit for re-usage convenience

    Notes
    -----
    The default vertex time strategy of ``"TFirst"`` for the input track
    was chosen because it is the standard for online level 2 and
    level 3. Supposing the input track has already an excellent timing,
    ``"TNone"`` should be the logical and optimal choice. According to
    some incomplete testing there should not be any dramatic differences
    between the two strategies.

    """
    icetray.logging.log_trace("name={0}".format(name), unit="lilliput")

    minimizer = add_minuit_simplex_minimizer_service(
        tray, minimizer=minimizer)

    likelihood = add_pandel_likelihood_service(
        tray, pulses=pulses, domllh=domllh, noiserate=noiserate)

    seed = add_seed_service(
        tray, pulses=pulses, seeds=[inputtrack], tstype=input_tstype)

    if input_tstype == grid_tstype:
        grid_seed = seed
    else:
        grid_seed = add_seed_service(
            tray, pulses=pulses, seeds=[], tstype=grid_tstype)

    # Most values are copied from OnlineL2.
    tray.Add("I3ParaboloidFitter", name,
             SeedService=seed,
             LogLikelihood=likelihood,
             MaxMissingGridPoints=1,
             VertexStepSize=5.*I3Units.m,
             ZenithReach=2.*I3Units.degree,
             AzimuthReach=2.*I3Units.degree,
             GridpointVertexCorrection=grid_seed,
             Minimizer=minimizer,
             NumberOfSamplingPoints=8,
             NumberOfSteps=3,
             MCTruthName="",
             OutputName=fitname,
             If=If)

    return minimizer, likelihood, seed, grid_seed, name
