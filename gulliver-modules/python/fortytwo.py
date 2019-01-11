"""Test suite for Gulliver

Implementation of an IceTray module to test reproducibility of
processing results.

Frame object values from current processing are compared with values
obtained with the same script in the past and/or on some other platform.

The primary (and currently only) use case is to include this module in
the resource/test scripts that are run by the build bots.
Failure to reproduce results results in un-green checkboxes.

If this ever turns out to become more popular then it should move to
some project in offline-software. Then it should also be split up,
each class in a separate file.

"""
# TODO Find a less clutteresque way to use the official logging.

# TODO Right now the test verdicts are binary: good or bad.  Maybe we should
# also allow a purgatory verdict (fishy) for resutls that are a little off but
# not too much.

# TODO Ponder whether a subdirectory with HDF files is a better way to store
# reference data than a pickle file. Advantage: avoids the get_test_values
# methods. Disadvantage: more files, extra dependencies. Storing reference data
# in an i3-file could also be considered, but the nice thing of a pickle is
# that it is completely independent from the system that we want to test.

import os
import sys

import numpy

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.gulliver

if sys.version_info[0] >= 3:
    import pickle
else:
    import cPickle as pickle


# Copy of numpy.isclose, which is not available on numpy 1.4 and older (e.g.
# build bots).
def isclose(a, b, rtol=1.e-5, atol=1.e-8, equal_nan=False):
    """Copied from a modern `numpy` distribution.

    We are using this copy here because the build bots
    builds.icecube.wisc.edu are still using a very old `numpy`
    distribution which apparently does not know about the ``isclose``
    function. Once they are updated, we cane remove this copy and just
    use ``numpy.isclose`` directly.

    Modifcation for integer types: just require a and b to be equal,
    ignore tolerances.

    """
    def within_tol(x, y, atol, rtol):
        err = numpy.seterr(invalid="ignore")
        try:
            result = numpy.less_equal(abs(x-y), atol + rtol * abs(y))
        finally:
            numpy.seterr(**err)
        if numpy.isscalar(a) and numpy.isscalar(b):
            result = bool(result)
        return result

    x = numpy.array(a, copy=False, subok=True, ndmin=1)
    y = numpy.array(b, copy=False, subok=True, ndmin=1)
    if "int" in x.dtype.name and "int" in y.dtype.name:
        return x == y

    xfin = numpy.isfinite(x)
    yfin = numpy.isfinite(y)

    if numpy.all(xfin) and numpy.all(yfin):
        return within_tol(x, y, atol, rtol)
    else:
        finite = xfin & yfin
        cond = numpy.zeros_like(finite)

        # Because we're using boolean indexing, x & y must be the same shape.
        # Ideally, we'd just do x, y = broadcast_arrays(x, y). It's in
        # lib.stride_tricks, though, so we can't import it here.
        x = x * numpy.ones_like(cond)
        y = y * numpy.ones_like(cond)

        # Avoid subtraction with infinite/nan values.
        cond[finite] = within_tol(x[finite], y[finite], atol, rtol)

        # Check for equality of infinite values.
        cond[~finite] = (x[~finite] == y[~finite])

        if equal_nan:
            # Make NaN == NaN.
            cond[numpy.isnan(x) & numpy.isnan(y)] = True

        return cond

# Add isclose function to old numpy distributions.
if not hasattr(numpy, "isclose"):
    numpy.isclose = isclose


# ----IceTray modules----------------------------------------------------------
def MultiplicityCutModule(frame, pulses, nchmin=0, nchmax=10000, nstrmin=0,
                          nstrmax=10000):
    if frame.Stop != icecube.icetray.I3Frame.Physics:
        return True
    if pulses not in frame:
        return False

    pulsemap = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
        frame, pulses)
    domlist = list(pulsemap.keys())

    nch = len(domlist)
    if nch < nchmin or nch > nchmax:
        return False

    stringset = set([dom.string for dom in domlist])
    nstr = len(stringset)
    return (nstr >= nstrmin and nstr <= nstrmax)


class I3FortyTwo(icecube.icetray.I3ConditionalModule):
    """Check processing results

    A module to check that processing results (e.g. track fits,
    multiplicities) are the same on different platforms and do not
    change over time due to changes in the processing software.
    Reference results for any number of processing/testing scripts can
    be stored in a single picklefile. This module only cares about the
    final answers of other module(s) and that they have the expected
    value(s). Hence the name.

    An I3FortyTwo module operates in two different modes:

    - 'reference': test results of the current run of the script are
       stored in a pickle file. This pickle file can be stored in SVN
       and then be used by for instance the build bots.
    - 'check': test results from the latest reference run are read in
      from the pickle file. They are also computed for the current run,
      and tested for equality (or equivalence). If any (too large)
      deviations then the Finish() method will throw an exception in
      order to trigger the build bots to un-green the appropriate
      checbox on builds.icecube.wisc.edu.

    """
    def __init__(self, context):
        icecube.icetray.I3ConditionalModule.__init__(self, context)

        icecube.icetray.logging.log_trace("Initializing...", unit="I3FortyTwo")

        self.AddOutBox("OutBox")

        # Not setting the default yet for key and picklefilename because that
        # would give weird results when running icetray-inspect.
        self.AddParameter("key",
                          "Key under which the results will be stored in a "
                          "dictionary in the pickle file. If you leave this "
                          "empty then the name of the processing script "
                          "(basename of sys.argv[0]) will be used.",
                          "")
        self.AddParameter("filename",
                          "Full path of the pickle file to read/write "
                          "reference results. If you leave this empty then a "
                          "file called 42.dat in the same directory as the "
                          "processing script will be used.",
                          "")
        self.AddParameter("checklist",
                          "List of I3FrameChecker objects; if this list is "
                          "empty then the module will run in `check` mode: "
                          "read checklist from pickle file, compare test "
                          "values in current run with the reference values. "
                          "Otherwise the module will run in 'reference' mode: "
                          "let the frame checker objects compute the "
                          "reference test values, which are then stored in "
                          "the dictionary in the pickle file. The pickle file "
                          "should then be uploaded/updated in SVN (only "
                          "experts should do this). Make sure to also "
                          "upload/update the new/modified processing script "
                          "to SVN, but with the checklist option *empty* (so "
                          "that build bots will run the module in 'check' "
                          "mode)! It might be useful to keep the checklist "
                          "used in the latest reference run in a comment "
                          "line.",
                          [])

        icecube.icetray.logging.log_trace("Initialization done.",
                                          unit="I3FortyTwo")

        self.nframe = 0
        self.trouble = []

    def Configure(self):
        icecube.icetray.logging.log_trace("Configuring...", unit="I3FortyTwo")

        self.rundir = os.path.abspath(os.path.dirname(sys.argv[0]))

        icecube.icetray.logging.log_debug("rundir = %s" % self.rundir,
                                          unit="I3FortyTwo")

        self.key = self.GetParameter("key")
        if len(self.key) == 0:
            self.key = os.path.basename(sys.argv[0])

        icecube.icetray.logging.log_debug("key = %s" % self.key,
                                          unit="I3FortyTwo")

        self.filename = self.GetParameter("filename")
        if len(self.filename) == 0:
            self.filename = os.path.join(self.rundir, "42.dat")

        icecube.icetray.logging.log_info("filename = %s" % self.filename,
                                         unit="I3FortyTwo")

        self.checklist = self.GetParameter("checklist")
        if not isinstance(self.checklist, list):
            icecube.icetray.logging.log_fatal(
                "checklist should be of type list, not %s."
                % type(self.checklist), unit="I3FortyTwo")

        nchecks = len(self.checklist)
        if nchecks > 0:
            self.mode = "reference"

            icecube.icetray.logging.log_debug(
                "Going to write reference data for %s to file %s, %d checks."
                % (self.key, self.filename, nchecks), unit="I3FortyTwo")
        else:
            self.mode = "check"

        icecube.icetray.logging.log_info("mode = %s" % self.mode,
                                         unit="I3FortyTwo")

        if os.path.isfile(self.filename):
            icecube.icetray.logging.log_debug(
                "Pickle file %s exists; trying to read..." % self.filename,
                unit="I3FortyTwo")

            self.ReadPickle()
        elif self.mode == "reference":
            icecube.icetray.logging.log_info(
                "Pickle file %s not found; starting with empty dict."
                % self.filename, unit="I3FortyTwo")

            self.checkdict = {}
        else:
            icecube.icetray.logging.log_fatal(
                "Check of %s not possible: pickle file %s not found."
                % (self.key, self.filename), unit="I3FortyTwo")

        icecube.icetray.logging.log_trace("Configuring done.",
                                          unit="I3FortyTwo")
        return

    def ReadPickle(self):
        icecube.icetray.logging.log_debug("Getting pickle from %s..."
                                          % self.filename, unit="I3FortyTwo")

        with open(self.filename, "rb") as data:
            if sys.version_info[0] >= 3:
                self.checkdict = pickle.load(data, encoding="latin1")
            else:
                self.checkdict = pickle.load(data)

        icecube.icetray.logging.log_trace("Getting pickle done.",
                                          unit="I3FortyTwo")

        if self.key in self.checkdict:
            if self.mode == "check":
                icecube.icetray.logging.log_debug(
                    "Found reference data for %s." % self.key,
                    unit="I3FortyTwo")

                self.checklist = self.checkdict[self.key]
                self.num_reference_events =\
                    self.checklist[0].num_reference_events()
            else:
                icecube.icetray.logging.log_notice(
                    "Found reference data for %s, going to overwrite them."
                    % self.key, unit="I3FortyTwo")
        elif self.mode == "check":
                icecube.icetray.logging.log_fatal(
                    "Check of %s not possible: key %s not found in the pickle "
                    "file %s. Maybe you need to run a reference run first, or "
                    "SVN update the script and/or the pickle file. If you "
                    "have no idea what this is all about: please contact the "
                    "author of the script."
                    % (self.key, self.key, self.filename), unit="I3FortyTwo")
        return

    def Physics(self, frame):
        icecube.icetray.logging.log_trace("Processing frame %d in %s mode..."
                                          % (self.nframe, self.mode),
                                          unit="I3FortyTwo")

        if self.mode == "check":
            if self.nframe >= self.num_reference_events:
                self.nframe += 1

                icecube.icetray.logging.log_warn(
                    "Only %d reference events in pickle file %s; skip "
                    "frame %d."
                    % (self.num_reference_events, self.filename, self.nframe),
                    unit="I3FortyTwo")

                self.PushFrame(frame)
                return

            self.Check(frame)
        else:
            self.Reference(frame)

        icecube.icetray.logging.log_trace("Processing of frame %d in mode %s "
                                          "done." % (self.nframe, self.mode),
                                          unit="I3FortyTwo")

        self.nframe += 1
        self.PushFrame(frame)
        return

    def Check(self, frame):
        icecube.icetray.logging.log_trace("Check...", unit="I3FortyTwo")

        for checker in self.checklist:
            problem = checker.compare(self.nframe, frame)
            if problem is not None:
                self.trouble.append("Frame %d: %s" % (self.nframe, problem))
        return

    def Reference(self, frame):
        icecube.icetray.logging.log_trace("Reference...", unit="I3FortyTwo")

        for checker in self.checklist:
            checker.reference(self.nframe, frame)
        return

    def Finish(self):
        icecube.icetray.logging.log_trace("Finishing...", unit="I3FortyTwo")

        for checker in self.checklist:
            checker.finish()

        if len(self.trouble) > 0:
            icecube.icetray.logging.log_error(
                "\nSome checks failed:\n%s\nPlease investigate."
                % "\n".join(self.trouble), unit="I3FortyTwo")

            icecube.icetray.logging.log_fatal("Some checks failed.",
                                              unit="I3FortyTwo")

        if self.mode == "reference":
            if self.key in self.checkdict:
                icecube.icetray.logging.log_notice(
                    "Overwriting reference data for %s in pickle file %s."
                    % (self.key, self.filename), unit="I3FortyTwo")
            else:
                icecube.icetray.logging.log_notice(
                    "Adding new reference data for %s to pickle file %s."
                    % (self.key, self.filename), unit="I3FortyTwo")

            self.checkdict[self.key] = self.checklist
            with open(self.filename, "w") as data:
                pickle.dump(self.checkdict, data)

        elif self.nframe > self.num_reference_events:
            icecube.icetray.logging.log_warn(
                "Only %d reference events are available in pickle file %s, %d "
                "extra frames were ignored. Maybe the reference data need to "
                "be updated."
                % (self.num_reference_events, self.filename,
                   self.nframe-self.num_reference_events),
                unit="I3FortyTwo")

        elif self.nframe < self.num_reference_events:
            icecube.icetray.logging.log_fatal(
                "Only %d events were checked, %d reference events are "
                "available in pickle file %s."
                % (self.nframe, self.num_reference_events, self.filename),
                unit="I3FortyTwo")

        else:
            icecube.icetray.logging.log_notice("Congratulations: checks %s "
                                               "passed successfully."
                                               % self.key, unit="I3FortyTwo")
        return


class Pcount(icecube.icetray.I3ConditionalModule):
    """Progress meter

    Silly for default test, but useful if you use the cmdline options to
    e.g. run this on a larger input file. There is an I3EventCounter in
    phys-services, but that is clunky. It requires an XML files service.

    """
    def __init__(self, context):
        icecube.icetray.I3ConditionalModule.__init__(self, context)

        self.AddOutBox("OutBox")

        self.nevents = 10
        self.AddParameter("num_pframes",
                          "How many P-frames to look at.",
                          self.nevents)

        self.pmod = 1
        self.AddParameter("pmod",
                          "How often to report process (modulo, set to zero "
                          "to keep it quiet).",
                          self.pmod)

        self.count = 0

    def Configure(self):
        self.nevents = self.GetParameter("num_pframes")
        self.pmod = self.GetParameter("pmod")

        icecube.icetray.logging.log_notice("Going to stop the run after %d "
                                           "P-frames." % self.nevents,
                                           unit="Pcount")
        return

    def DAQ(self, frame):
        self.PushFrame(frame)
        return

    def Physics(self, frame):
        self.count += 1
        if self.count == self.nevents:
            icecube.icetray.logging.log_notice("%d P-frames done, requesting "
                                               "suspension..." % self.nevents,
                                               unit="Pcount")

            self.RequestSuspension()
            return

        if self.pmod > 0 and 0 == (self.count % self.pmod):
            icecube.icetray.logging.log_info(
                "COUNT: %d events done, %d to go..."
                % (self.count, self.nevents-self.count), unit="Pcount")

        self.PushFrame(frame)
        return


# ----Checker base class-------------------------------------------------------
# TODO: Implement versioning?
class I3FrameChecker(object):
    """This class serves to define the interface for checks on specific
    (types of) objects within a frame, to be used by an ``I3FortyTwo``
    module.

    """
    def __init__(self):
        # The reference_list is a list of lists of reference results.
        self.reflist = []
        self.i = 0

    def num_reference_events(self):
        """Return the number of reference events. All of them should be
        checked.
        """
        if len(self.reflist) == 0:
            return 0
        else:
            return len(self.reflist[0])

    def reference(self, index, frame):
        """This method gets called by the ``I3FortyTwo`` module in
        `reference` mode. The checker object is then installed in the
        processing script and passed to the module via the configurable
        parameters. We compute the test values and store them within
        this checker object in the `reference_list`.
        """
        self.i = index
        test_values = self.get_test_values(index, frame)

        icecube.icetray.logging.log_debug("Reference values for frame %d: %s"
                                          % (index, test_values),
                                          unit="I3FrameChecker")

        if index == 0:
            self.reflist = [[val] for val in test_values]
        elif len(test_values) != len(self.reflist):
            raise RuntimeError("Coding error in %s: number of reference "
                               "values not the same for every "
                               "frame." % type(self))
        else:
            for values, val in zip(self.reflist, test_values):
                values.append(val)

    def compare(self, index, frame):
        """This method gets run in `check` mode. The checker object is
        then instantiated by reading it from a pickle file, so the
        `reference_list` should be filled with the test values from a
        previous run. We compute the test values now again in the
        current run, and check that they are compatible with the values
        stored in `reference_list`.
        """
        self.i = index
        reference_values = [values[index] for values in self.reflist]
        test_values = self.get_test_values(index, frame)

        if not len(reference_values) == len(test_values):
            raise RuntimeError("Coding error in %s: number of reference "
                               "values not the same for every "
                               "frame." % type(self))

        return self.compare_test_values(test_values, reference_values)

    def report_missing(self, key):
        icecube.icetray.logging.log_notice("Object %s not found in frame %d."
                                           % (key, self.i),
                                           unit="I3FrameChecker")

    def get_test_values(self, index, frame):
        """This method must be implemented by a subclasss. For every
        frame the same number of test values are computed, which should
        be returned in the form of a list.
        """
        raise NotImplementedError("get_test_values is a virtual method.")

    def compare_test_values(self, values, reference_values):
        """This method must be implemented by a subclasss. The `values`
        and `reference_values` are both lists of test values as
        computed by the ``get_test_values`` method.

        If some test values are NOT compatible with each other then a
        string should be returned with an informative description of the
        problem(s).

        If the current test values and the reference ones are are all
        compatible with each other then an empty string (or ``None``, or
        ``False``) should be returned.
        """
        raise NotImplementedError("compare_test_values is a virtual method.")

    def finish(self):
        """Optional, write diagnostic stuff at the end."""
        pass


# ----Checker implementations--------------------------------------------------
class DummyChecker(I3FrameChecker):
    """This checker just serves as an example of a very simple
    implementation of an ``I3FrameChecker``. It computes only one test
    value.
    """
    def __init__(self):
        I3FrameChecker.__init__(self)

    def get_test_values(self, index, frame):
        nkeys = len(list(frame.keys()))
        icecube.icetray.logging.log_debug("get_test_values: frame %d with "
                                          "%d keys." % (index, nkeys),
                                          unit="DummyChecker")
        return [nkeys]

    def compare_test_values(self, values, reference_values):
        icecube.icetray.logging.log_debug("compare_test_values: value = %d, "
                                          "reference = %d."
                                          % (values[0], reference_values[0]),
                                          unit="DummyChecker")

        if not values[0] == reference_values[0]:
            return("ERROR: got nkeys = %d, expected %d."
                   % (values[0], reference_values[0]))

        return


# TODO: Compare direction cosines instead of zenith/azimuth; avoids azimuth
# issues with near-vertical fits.
class I3ParticleChecker(I3FrameChecker):
    """Checks fit status and direction of I3Particle.

    Needs a parameter to tweak the tolerance at which floating point
    numbers should be compared (with ``numpy.isclose``).

    """
    # Tolerances for status, type, shape, zenith, azimuth,
    # x, y, z, t, L, log10(E)
    rtols = (0., 0., 0., 1e-3, 1e-3, 1e-2, 1e-2, 1e-2, 1e-2, 1e-2, 0.01)
    atols = (0., 0., 0., 2e-2, 2e-2, 0.1, 0.1, 0.1, 0.1, 0.1, 0.01)

    def __init__(self, particles=[]):
        I3FrameChecker.__init__(self)
        self.particles = particles

    def get_test_values(self, index, frame):
        if 0 == len(self.particles):
            self.particles = [k for k in frame
                              if frame.type_name(k) == "I3Particle"
                              ]

        status_list = []
        type_list = []
        shape_list = []

        zenith_list = []
        azimuth_list = []

        vertex_x_list = []
        vertex_y_list = []
        vertex_z_list = []
        time_list = []

        track_length_list = []
        log_energy_list = []

        for key in self.particles:
            if key not in frame:
                status_list.append(icecube.dataclasses.I3Particle.NotSet)
                type_list.append(icecube.dataclasses.I3Particle.unknown)
                shape_list.append(icecube.dataclasses.I3Particle.Null)
                zenith_list.append(numpy.nan)
                azimuth_list.append(numpy.nan)
                vertex_x_list.append(numpy.nan)
                vertex_y_list.append(numpy.nan)
                vertex_z_list.append(numpy.nan)
                time_list.append(numpy.nan)
                track_length_list.append(numpy.nan)
                log_energy_list.append(numpy.nan)

                self.report_missing(key)
            else:
                particle = frame[key]

                status_list.append(particle.fit_status)
                type_list.append(particle.type)
                shape_list.append(particle.shape)
                zenith_list.append(particle.dir.zenith)
                azimuth_list.append(particle.dir.azimuth)
                vertex_x_list.append(particle.pos.x)
                vertex_y_list.append(particle.pos.y)
                vertex_z_list.append(particle.pos.z)
                time_list.append(particle.time)
                track_length_list.append(particle.length)

                if particle.energy > 0:
                    log_energy_list.append(numpy.log10(particle.energy))
                else:
                    log_energy_list.append(particle.energy)

        return [numpy.array(status_list), numpy.array(type_list),
                numpy.array(shape_list), numpy.array(zenith_list),
                numpy.array(azimuth_list), numpy.array(vertex_x_list),
                numpy.array(vertex_y_list), numpy.array(vertex_z_list),
                numpy.array(time_list), numpy.array(track_length_list),
                numpy.array(log_energy_list)
                ]

    def compare_test_values(self, values, reference_values):
        mask = numpy.ones(len(self.particles), dtype=bool)
        for val, ref, rtol, atol in zip(values, reference_values, self.rtols,
                                        self.atols):
            mask *= numpy.isclose(val, ref, rtol, atol, equal_nan=True)

        if mask.all():
            return None
        else:
            return self.problems(values, reference_values, mask)

    def problems(self, values, reference_values, mask):
        name_list = ["status", "type", "shape", "zenith", "azimuth", "x", "y",
                     "z", "t", "L", "log10(E)"
                     ]

        complaints = []
        for i, particle in enumerate(self.particles):
            if mask[i]:
                continue

            issue = "%s value/reference: " % particle
            for name, val, ref, rtol, atol in zip(
                    name_list, values, reference_values, self.rtols,
                    self.atols):
                issue += "%s = %s/%s " % (name, val[i], ref[i])
                if numpy.isclose(val[i], ref[i], rtol=rtol, atol=atol,
                                 equal_nan=True):
                    issue += "(OK?)"
                else:
                    issue += "(PROBLEM!)"
                issue += "\n"

            complaints.append(issue)

        return "\n".join(complaints)


# TODO This class is very similar to the I3ParticleChecker class. Maybe some of
# the common code should be absorbed in a convenience method of the
# I3FrameChecker base class.
class I3LogLikelihoodFitParamsChecker(I3FrameChecker):
    """Checks the diagnostics object of gulliver fits.

    Diagnostics:

        * likelihood value
        * reduced likelihood value
        * number of degree of freedom
        * number of minimizer steps

    """
    # Tolerances for logl, rlogl, ndof, nmini
    rtols = (0.01, 0.01, 0., 0.)
    atols = (0.05, 0.05, 0., 100.)

    def __init__(self, params=[]):
        I3FrameChecker.__init__(self)
        self.params = params

    def get_test_values(self, index, frame):
        if 0 == len(self.params):
            self.params = [k for k in frame
                           if frame.type_name(k) == "I3LogLikelihoodFitParams"
                           ]
        logl_list = []
        rlogl_list = []
        ndof_list = []
        nmini_list = []

        for key in self.params:
            if key not in frame:
                logl_list.append(numpy.nan)
                rlogl_list.append(numpy.nan)
                ndof_list.append(0)
                nmini_list.append(0)

                self.report_missing(key)
            else:
                fit_params = frame[key]

                logl_list.append(fit_params.logl)
                rlogl_list.append(fit_params.rlogl)
                ndof_list.append(fit_params.ndof)
                nmini_list.append(fit_params.nmini)

        return [numpy.array(logl_list), numpy.array(rlogl_list),
                numpy.array(ndof_list), numpy.array(nmini_list)
                ]

    def compare_test_values(self, values, reference_values):
        mask = numpy.ones(len(self.params), dtype=bool)
        for val, ref, rtol, atol in zip(values, reference_values, self.rtols,
                                        self.atols):
            mask *= numpy.isclose(val, ref, rtol=rtol, atol=atol,
                                  equal_nan=True)

        if mask.all():
            return None
        else:
            return self.problems(values, reference_values, mask)

    def problems(self, values, reference_values, mask):
        name_list = ["logl", "rlogl", "ndof", "nmini"]

        complaints = []
        for i, par in enumerate(self.params):
            if mask[i]:
                continue

            issue = "%s value/reference: " % par
            for name, val, ref, rtol, atol in zip(
                    name_list, values, reference_values, self.rtols,
                    self.atols):
                issue += "%s = %s/%s " % (name, val[i], ref[i])
                if numpy.isclose(val[i], ref[i], rtol=rtol, atol=atol,
                                 equal_nan=True):
                    issue += "(OK?)"
                else:
                    issue += "(PROBLEM!)"
                issue += "\n"

            complaints.append(issue)

        return "\n".join(complaints)
