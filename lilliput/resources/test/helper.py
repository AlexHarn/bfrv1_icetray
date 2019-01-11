"""Helper modules for test scripts
"""
import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.gulliver


class COGPrior(icecube.gulliver.I3EventLogLikelihood):
    def __init__(self, name, pulsemap, distance=30.*icecube.icetray.I3Units.m):
        super(COGPrior, self).__init__()

        self.name = name
        self.pulsemap = pulsemap
        self.geometry = None
        self.multiplicity = 0
        self.distance2 = distance**2

        self.count = 0
        self.nbad = 0
        self.ncall = 0
        self.nfit = 0

    def __del__(self):
        if self.count > 0:
            self.ncall += self.count
            self.nfit += 1

        if self.nfit > 0:
            average = self.ncall / float(self.nfit)
            print("COGPrior: {0:d} function calls in {1:d} fits, {2:.1f} on "
                  "average.".format(self.ncall, self.nfit, average))
        else:
            print("COGPrior: No fits?")

    def SetEvent(self, frame):
        if self.geometry and self.pulsemap in frame:
            pulsemap = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
                frame, self.pulsemap)

            self.cog = icecube.common_variables.hit_statistics.calculate_cog(
                self.geometry, pulsemap)

            self.multiplicity = len(pulsemap)
        else:
            self.cog = None
            self.multiplicity = 0
            self.nbad += 1

        if self.count > 0:
            self.ncall += self.count
            self.nfit += 1

        self.count = 0

    def SetGeometry(self, geo):
        self.geometry = geo

    def GetName(self):
        return self.name

    def GetMultiplicity(self):
        return self.multiplicity

    def GetDiagnostics(self, fitresult):
        return None

    def GetLogLikelihood(self, hypothesis):
        if self.cog is None:
            icecube.icetray.logging.log_warn(
                "Cannot compute COG weight without pulse map and geometry.",
                unit="COGPrior")

            return 0

        self.count += 1

        position = hypothesis.particle.pos
        direction = hypothesis.particle.dir

        shift = (self.cog - position) * direction
        closest = position + shift * direction
        distance = self.cog - closest
        distance2 = distance.mag2

        prior = -distance2 / self.distance2
        success = distance * direction

        icecube.icetray.logging.log_trace(
            "Call {0:03d}: success={1}, cog={2}, closest={3}, distance2={4}, "
            "prior={5}".format(self.count, success, self.cog, closest,
                               distance2, prior),
            unit="COGPrior")

        if success > 0.1:
            icecube.icetray.logging.log_fatal("Check failed.", unit="COGPrior")

        return prior

    def GetLogLikelihoodWithGradient(self, hypothesis, grad, weight):
        return None

    def HasGradient(self):
        return False


def Multiplicity(name):
    def check(frame):
        if not frame.Stop == icecube.icetray.I3Frame.Physics:
            return False

        if name not in frame:
            icecube.icetray.logging.log_error(
                "Pulsemap {0} not found, event header is {1}, frame is "
                "{2}.".format(name, frame["I3EventHeader"], frame),
                unit="OKmultiplicity")

            return False

        pulsemap = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
            frame, name)

        nch = len(pulsemap)
        nstr = len(set([omkey.string for omkey, pulseseries in pulsemap]))

        push = (nch >= 8 and nch <= 28 and nstr >= 3)

        icecube.icetray.logging.log_trace(
            "NCh={0:d}, NString={1:d}, OK={2}".format(nch, nstr, push),
            unit="Multiplicity")

        return push

    return check


class PFCounter(icecube.icetray.I3ConditionalModule):
    def __init__(self, contex):
        super(PFCounter, self).__init__(contex)

    def Configure(self):
        self.nleft = 10

    def Physics(self, frame):
        self.nleft -= 1

        if self.nleft <= 0:
            icecube.icetray.logging.log_debug(
                "10 frames processed. Going to request suspension.",
                unit="PFCounter")

            self.RequestSuspension()
        else:
            icecube.icetray.logging.log_debug(
                "{0} frames to go...".format(self.nleft),
                unit="PFCounter")

        self.PushFrame(frame)
