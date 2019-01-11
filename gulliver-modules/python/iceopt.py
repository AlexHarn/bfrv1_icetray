"""IceOpt module

This module provides an `optparse.OptionParser` wrapper for IceTray
script. It can be used to parse input i3-files to the scripts, specify
common output files (ROOT, HDF5, ...) and so on.

"""
import glob
import optparse
import os


class IceTrayScriptOptions(optparse.OptionParser):
    """IceTray wrapper for the standard `optparse.OptionParser` class

    The `iceopt.IceTrayScriptOptions` class is a wrapper for the
    standard `optparse.OptionParser` class, and it predefines a bunch of
    very common options in typical IceTray scripts: GCD file, input
    file(s), ROOT output file, verbosity, output file, and number of
    events to be processed.

    Input files are checked for existence and ``i3(.gz)`` suffix. A
    default input file can be defined that will only be used in case the
    user defines no input file at all (apart from maybe a GCD file).
    All input files (GCD and others) are collected in a list named
    `infiles`, which has the ``-g`` and ``-i`` files in front, if
    provided.

    You can add more options using the regular `add_option` method from
    `optparse.OptionParser`. If you plan to do so, then make sure to set
    ``moreopts = True`` when instantiating the
    `iceopt.IceTrayScriptOptions` object, and call `parse` after you
    have added all your custom options. If you leave
    ``moreopts = False`` (default) then the options will be parsed
    immediately during instantiation.

    """
    def __init__(self, name, definput="", defgcd="", defpulses="OfflinePulses",
                 defverbose=False, defnevents=0, moreopts=False):
        """Defines standard options.

        Standard options are parsed immediately, unless the user
        indicated that more will be added: ``moreopts = True``.

        """
        optparse.OptionParser.__init__(
            self, "usage: %prog [options] [i3 input files]")

        self.infiles = []
        self.definput = definput
        self.outfile = name + ".i3.gz"
        self.rootfile = name + ".root"
        self.hdf5file = name + ".hdf"
        self.xmlfile = name + ".xml"
        self.nevents = defnevents
        self.verbose = defverbose
        self.pulses = defpulses

        optparse.OptionParser.add_option(
            self,
            "-i", "--infile",
            default="",
            dest="INFILE",
            help="read input to INFILE (.i3.gz format)")
        optparse.OptionParser.add_option(
            self,
            "-g", "--gcdfile",
            default=defgcd,
            dest="GCDFILE",
            help="read geo-cal-detstat to GCDFILE (.i3.gz)")
        optparse.OptionParser.add_option(
            self,
            "-p", "--pulses",
            default=defpulses,
            dest="PULSES",
            help="name of pulses to use in reconstructions")
        optparse.OptionParser.add_option(
            self,
            "-r", "--rootfile",
            default=self.rootfile,
            dest="ROOTFILE",
            help="write output to flat-ntuple OUTFILE (.root format)")
        if self.verbose:
            optparse.OptionParser.add_option(
                self,
                "-q", "--quiet",
                default=True,
                action="store_false",
                dest="VERBOSE",
                help="do not babble about what's going on")
        else:
            optparse.OptionParser.add_option(
                self,
                "-v", "--verbose",
                default=False,
                action="store_true",
                dest="VERBOSE",
                help="be noisy about what's going on")
        optparse.OptionParser.add_option(
            self,
            "-o", "--outfile",
            default=self.outfile,
            dest="OUTFILE",
            help="write output to I3Writer OUTFILE (.i3 format)")
        optparse.OptionParser.add_option(
            self,
            "-n", "--nevents",
            type="int",
            default=self.nevents,
            dest="NEVENTS",
            help="number of events to process (default: all)")

        self.__special_options__ = []
        if not moreopts:
            self.parse()

    def add_option(self, shortname="-", longname="--", *args, **kwargs):
        if shortname == "-h":
            return optparse.OptionParser.add_option(self, shortname, longname,
                                                    *args, **kwargs)

        # print "shortname = ", shortname, "longname = ", longname
        # print "args = ", args
        # print "kwargs = ", kwargs

        if "dest" in kwargs:
            dest = kwargs["dest"]

            assert dest not in self.__dict__,\
                ("option %s already exists" % dest)

            assert dest not in self.__special_options__,\
                ("option %s already exists" % dest)

            self.__special_options__.append(dest)

        optparse.OptionParser.add_option(self, shortname, longname, *args,
                                         **kwargs)

    def noise(self, msg):
        """Printing messages only if allowed to do so."""
        if self.verbose:
            print(msg)

    def checksuf(self, filename="", descr="input file"):
        """Check file suffix: ``.i3``, ``.i3.gz`` or ``.bz2``.

        Parameters
        ----------
        filename : str, optional
            Check suffix of `filename`.
        descr : str, optional
            Description for `filename`

        Raises
        ------
        optparse.OptionParser.error
            In case of a non-excepted suffix

        """
        if (filename[-3:] != ".i3" and
                filename[-6:] != ".i3.gz" and
                filename[-7:] != ".i3.bz2" and
                filename[-4:] != ".tar" and
                filename[-7:] != ".tar.gz" and
                filename[-8:] != ".tar.bz2"):
            self.error("%s %s has wrong suffix (should be .i3, .i3.gz,"
                       ".i3.bz2, .tar, .tar.gz or .tar.bz2)."
                       % (descr, filename))

    def checkaccess(self, filename="", descr="input file"):
        """Check file accessibility.

        Parameters
        ----------
        filename : str, optional
            Check accessibility of `filename`.
        descr : str, optional
            Description for `filename`

        Raises
        ------
        optparse.OptionParser.error
            In case of a non-existing file

        """
        if os.access(filename, os.R_OK) == False:
            self.error("%s named %s does not exist or is inaccessible!"
                       % (descr, filename))

    def check(self, filename="", descr="input file"):
        """Combination of `checksuf` and `checkaccess`"""
        self.checksuf(filename, descr)
        self.checkaccess(filename, descr)

    def parse(self):
        """Parsing, standard checks, collect input files in a list."""
        # Use base class parser method.
        options, args = optparse.OptionParser.parse_args(self)

        # Set noise level.
        self.verbose = options.VERBOSE

        # Harvest files and check input.
        morefiles = []
        if len(args) != 0:
            badargs = []
            for a in args:
                fglob = glob.glob(a)
                if len(fglob) > 0:
                    morefiles.extend(fglob)
                else:
                    badargs.append(a)

            if len(badargs) > 0:
                msg = "Got undefined options or misspelled input files: %s."\
                       % " ".join(badargs)
                self.error(msg)

            for f in morefiles:
                self.checksuf(f)

        # Get and check explicit input & GCD filenames.
        if len(options.GCDFILE) > 0:
            self.check(options.GCDFILE, "GCD file")
            self.infiles.append(options.GCDFILE)
            self.noise("Using GCD file %s." % options.GCDFILE)
        else:
            self.noise("No GCD file")

        if len(options.INFILE) > 0:
            self.check(options.INFILE, "input file")
            self.infiles.append(options.INFILE)
            self.noise("Using input file %s." % options.INFILE)
        else:
            if len(morefiles) == 0 and len(self.definput) > 0:
                self.check(self.definput, "default input file")
                self.infiles.append(self.definput)
                self.noise("Using default input file %s." % self.definput)
            else:
                self.error("No input file(s)")
        self.infiles.extend(morefiles)

        # Get and check root output filename.
        if len(options.ROOTFILE) > 0:
            self.rootfile = options.ROOTFILE
            if self.rootfile[-5:] != ".root":
                self.error("ROOT output filename should have .root suffix.")
            self.xmlfile = "%s.xml" % self.rootfile[:-5]
            self.hdf5file = "%s.hdf" % self.rootfile[:-5]
            self.noise("XML summary file is %s." % self.xmlfile)
            self.noise("ROOT output file is %s." % self.rootfile)
            self.noise("HDF5 output file is %s." % self.hdf5file)
        else:
            self.error("No ROOT output file")

        # Get and check i3 output filename.
        if len(options.OUTFILE):
            self.outfile = options.OUTFILE
            self.checksuf(options.OUTFILE, "output file")
            self.noise("Output file is %s." % self.outfile)
        else:
            self.error("No i3(.gz) output file")

        # nevents should be an unsigned int.
        if options.NEVENTS <= 0:
            self.noise("Going to process all events.")
            options.NEVENTS = 0
        else:
            self.noise("Going to process at most %d frames." % options.NEVENTS)
        self.nevents = options.NEVENTS

        if options.PULSES:
            self.noise("Using pulses named %s." % options.PULSES)
            self.pulses = options.PULSES
        for o in self.__special_options__:
            ovalue = options.__dict__.pop(o, None)
            self.__dict__.setdefault(o, ovalue)
