import matplotlib
import numpy

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.gulliver


class MapSpecs(object):
    def __init__(self, parspecs, nsteps, stepsize):
        self.steps = [p.stepsize for p in parspecs]
        self.initvals = [p.initval for p in parspecs]
        self.names = [p.name for p in parspecs]
        self.npar = len(self.names)

        self.axes = [
            i + (step*stepsize)*numpy.arange(-int(nsteps/2.), int(nsteps/2.))
            for step, i in zip(self.steps, self.initvals)
            ]

        self.axbounds = [
            (ax[0]-(step*stepsize)/2., ax[-1]+(step*stepsize)/2.)
            for step, ax in zip(self.steps, self.axes)
            ]


class GulliView(icecube.icetray.I3ConditionalModule):
    """Likelihood space visualization

    This module displays (or save PNGs for) the two-dimensional slices
    of the likelihood space around the seed.

    """
    @property
    def LogLikelihood(self):
        """LogLikelihood service to use"""
        pass

    @property
    def Parametrization(self):
        """Parametrization service to use"""
        pass

    @property
    def SeedService(self):
        """Seed service to use"""
        pass

    @property
    def WithGradients(self):
        """Plot LogLikelihood gradients along with function values."""
        pass

    @property
    def NSteps(self):
        """Number of steps to take along each dimension"""
        pass

    @property
    def StepSize(self):
        """Size of each step in unit of the parametrization's step size
        """
        pass

    @property
    def Filename(self):
        """Output filename base for plots; if None, run interactively.
        """
        pass

    def __init__(self, ctx):
        icecube.icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")

        self.par = ""
        self.AddParameter("Parametrization",
                          self.__class__.Parametrization.__doc__, self.par)

        self.llh = ""
        self.AddParameter("LogLikelihood",
                          self.__class__.LogLikelihood.__doc__, self.llh)

        self.seeder = ""
        self.AddParameter("SeedService",
                          self.__class__.SeedService.__doc__, self.seeder)

        self.gradients = False
        self.AddParameter("WithGradients",
                          self.__class__.WithGradients.__doc__, self.gradients)

        self.nsteps = 20
        self.AddParameter("NSteps",
                          self.__class__.NSteps.__doc__, self.nsteps)

        self.stepsize = 0.5
        self.AddParameter("StepSize",
                          self.__class__.StepSize.__doc__, self.stepsize)

        self.filename = None
        self.AddParameter("Filename",
                          self.__class__.Filename.__doc__, self.filename)

    def Configure(self):
        self.par = icecube.gulliver.I3ParametrizationBase.from_context(
            self.context, self.GetParameter("Parametrization"))
        self.llh = icecube.gulliver.I3EventLogLikelihoodBase.from_context(
            self.context, self.GetParameter("LogLikelihood"))
        self.seeder = icecube.gulliver.I3SeedServiceBase.from_context(
            self.context, self.GetParameter("SeedService"))

        self.gulliver = icecube.gulliver.I3Gulliver(
            "GulliView", self.llh, self.par)

        self.gradients = self.GetParameter("WithGradients")
        self.nsteps = self.GetParameter("NSteps")
        self.stepsize = self.GetParameter("StepSize")

        self.filename = self.GetParameter("Filename")
        if self.filename is not None:
            matplotlib.use("Agg")

        self.frame_number = 0

    def Physics(self, frame):
        self.seeder.SetEvent(frame)
        seed = self.seeder.GetSeed(0)

        if self.gradients:
            if not self.llh.HasGradient():
                icecube.icetray.loggoing.log_fatal(
                    "Log-likelihood '%s' does not support gradients!"
                    % self.llh.GetName(), unit="GulliView")

            if not self.par.InitChainRule(True):
                icecube.icetray.loggoing.log_fatal(
                    "Parametrization '%s' does not implement the chain rule!"
                    % self.par.GetName(), unit="GulliView")

            self.gulliver.UseGradients(True)

        self.llh.SetEvent(frame)
        self.par.SetEvent(frame)

        parspecs = self.par.GetParInitSpecs(seed)
        specs = MapSpecs(parspecs, self.nsteps, self.stepsize)

        # Map out the likelihood space in 2-d slices.
        llhmap = self.map_llh_space(specs)

        # And plot it.
        self.plot_llh_space(specs, llhmap)

        self.PushFrame(frame)
        self.frame_number += 1

    def map_llh_space(self, specs):
        images = [[None for j in range(specs.npar)] for i in range(specs.npar)]

        mn = numpy.inf
        mx = -numpy.inf

        if self.gradients:
            def eval_llh(params):
                vals = icecube.icetray.vector_double()
                vals.extend(params)
                llh, grad = self.gulliver(vals, True)
                return -1*llh, -1*numpy.array(grad)
        else:
            def eval_llh(params):
                vals = icecube.icetray.vector_double()
                vals.extend(params)
                return -1*self.gulliver(vals, False)

        # Map out the likelihood in 2-d slices.
        for i in range(0, specs.npar):
            for j in range(i):
                p1, p2 = numpy.meshgrid(specs.axes[i], specs.axes[j])
                llh = numpy.zeros(p1.shape)

                if self.gradients:
                    grad = numpy.zeros(p1.shape + (specs.npar,))

                params = list(specs.initvals)
                for k in range(p1.shape[0]):
                    for l in range(p1.shape[1]):
                        params[i] = p1[k, l]
                        params[j] = p2[k, l]

                        if self.gradients:
                            llh[k, l], grad[k, l] = eval_llh(params)
                        else:
                            llh[k, l] = eval_llh(params)

                if self.gradients:
                    images[i][j] = (llh, grad)
                else:
                    images[i][j] = llh

                if llh.min() < mn:
                    mn = llh.min()
                if llh.max() > mx:
                    mx = llh.max()

        return images, mn, mx

    def plot_llh_space(self, specs, llhmap):
        import matplotlib.pyplot
        matplotlib.rc("font", size=10)
        matplotlib.rc("figure", autolayout=False)

        fig = matplotlib.pyplot.figure(figsize=(12, 8))

        gs = matplotlib.gridspec.GridSpec(
                nrows=specs.npar-1, ncols=specs.npar-1,
                left=0.05, bottom=0.05, right=0.97, top=0.95,
                wspace=0.25, hspace=0.25)

        images, mn, mx = llhmap
        im = None

        # Now, show each slice as an image.
        for i in range(specs.npar):
            for j in range(i):
                ax = fig.add_subplot(gs[i-1, j])

                llh = grad = None
                if self.gradients:
                    llh, grad = images[i][j]
                else:
                    llh = images[i][j]

                im = ax.imshow(llh.T, aspect="auto", vmin=mn, vmax=mx,
                               interpolation="nearest", origin="lower",
                               extent=specs.axbounds[j] + specs.axbounds[i])

                # Plot arrows on top of the.
                if self.gradients:
                    X, Y = numpy.meshgrid(specs.axes[j], specs.axes[i])
                    U = grad[:, :, j]*specs.steps[j]
                    V = grad[:, :, i]*specs.steps[i]

                    ax.quiver(X, Y, U.T, V.T, pivot="middle", units="xy")

                # Tuck axis labels inside each plot.
                ax.set_xlabel(specs.names[j], ha="right", va="bottom",
                              bbox=dict(facecolor=(1, 1, 1, 0.5),
                                        edgecolor=None))
                ax.set_ylabel(specs.names[i], ha="right", va="top",
                              bbox=dict(facecolor=(1, 1, 1, 0.5),
                                        edgecolor=None))

                ax.xaxis.set_label_coords(0.98, 0.03)
                ax.yaxis.set_label_coords(0.02, 0.96)

                ax.axvline(specs.initvals[j], c="k", ls=":")
                ax.axhline(specs.initvals[i], c="k", ls=":")

        # Make a colorbar in part of one of the unused patches.
        ax = fig.add_subplot(gs[0:min(specs.npar-1, 2), -1])

        cb = fig.colorbar(im, ax=ax, orientation='vertical', fraction=0.25)
        cb.set_label("$\\ln L$")

        # Trash original axes.
        fig.delaxes(ax)

        # Showtime!
        if self.filename is None:
            matplotlib.pyplot.show()
        else:
            fig.savefig("%s_%03d.png" % (self.filename, self.frame_number))
            matplotlib.pyplot.close(fig)
