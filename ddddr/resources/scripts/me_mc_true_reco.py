#!/usr/bin/env python
import sys, os, math
from os.path import expandvars

from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataclasses, dataio, ddddr

# take some test data that is already there
testdata = expandvars("$I3_TESTDATA")
GCD = os.path.join(testdata, 'sim', 'GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
L2FILE = os.path.join(testdata, 'sim', 'Level2_IC86.2011_corsika.010281.001664.00.i3.bz2')

########### Options ############
usage = "usage: %prog [options] inputfiles"
parser = OptionParser(usage)
parser.add_option("-o", "--outputfile", default=None,
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.bz2} format), default: %default")
parser.add_option("-p", "--pulse-map", default="TWOfflinePulsesHLC",
                  dest="PULSEMAP", help="PulseMap to use, default: % default")
parser.add_option("--max-impact", default=100,
                  dest="MAXIMPACT", type="float",
                  help="Max impact factor for DOMs to be considered for energy reconstruction, default: %default")
parser.add_option("--mc-tree-name", default="I3MCTree",
                  dest="MCTREENAME", help="Name of I3MCTREE, default: %default")
parser.add_option("--mmc-track-list", default="MMCTrackList",
                  dest="MMCTRACKLIST", help="Name of the MMCTrackList, default: %default")
parser.add_option("-s", "--slant-bin-size", default=50, type="float",
                  dest="SLANTBINSIZE", help="Bin size for slant depth, default: %default")
parser.add_option("-t", "--seedtrack", default="MPEFit",
                  dest="SEEDTRACKNAME", help="Seed Track to use, default: %default")
parser.add_option("-g", "--gcd-file", default=GCD,
                  dest="GCDFILE")
parser.add_option("--show", default=False, action="store_true", dest="SHOW",
                  help="Set flag to show plots for each event, default: %default")
parser.add_option("--save-plots", default=False, action="store_true",
                  dest="SAVEPLOTS", 
                  help="Set flag to save plots for each event, default: %default")
parser.add_option("--nplots", default=10, dest="NPLOTS", 
                  help="Maximum number of plots generated, only plots first nplots frames, default: %default")

# parse command line options, args are input files
(options, args) = parser.parse_args()

filenamelist = []
if options.GCDFILE:
    filenamelist.append(options.GCDFILE)
filenamelist.extend(args)
if len(filenamelist) == 1:
    if os.path.exists(L2FILE):
        filenamelist.append(L2FILE)
    else:
        icetray.logging.log_warn("No input file specified and "+\
                                 "default test file %s not available. Quitting now."%L2FILE)
        sys.exit()

# plotting_________________________

class EventPlotter(icetray.I3ConditionalModule):

    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("Prefixes","List of prefixes of DDDDR reconstructions to plot",
                          ["I3MuonEnergy_"])
        self.AddParameter("SaveFiguresTo","If set, saves figures to given path", None)
        self.AddParameter("Show", "Shows plots at the end of the tray.", False)
        self.AddParameter("XAxis", "Plot energy against Slant or Depth.", "Slant")
        self.AddParameter("NPlots", "Number of plots.", 10)
    
    def Configure(self):
        self.prefixes = self.GetParameter("Prefixes")
        self.save_to  = self.GetParameter("SaveFiguresTo")
        self.show     = self.GetParameter("Show")
        self.xaxis    = self.GetParameter("XAxis")
        self.n_plots  = self.GetParameter("NPlots")
        self.curr_plots = 0

    def Physics(self, frame):
        import matplotlib
        if not self.show:
            matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib as mpl
        import matplotlib.gridspec as gs

        if type(self.prefixes) != list:
            self.prefixes = [self.prefixes]


        if self.curr_plots < self.n_plots:
            plt.figure()
            self.curr_plots += 1
        else:
            self.PushFrame(frame)
            return

        ax1 = plt.subplot(111)
        for i, prefix in enumerate(self.prefixes):

            kws = dict()
            colors_ = mpl.rcParams['axes.color_cycle']
            kws.update({"color" : colors_[i%len(colors_)] })
            kws.update({"linewidth" : 2})

            self.plot_reconstruction(frame, prefix, xaxis=self.xaxis, **kws)
            
            eventid = frame['I3EventHeader'].event_id
            plot_title = '%s - event %d'%(prefix, eventid)
            plt.title(plot_title.replace('_','\_'))
            plt.xlabel("%s [m]"%self.xaxis.replace('Depth','z'))
            plt.ylabel("Energy Loss ($\mathrm{GeV / m}$)")

        if self.save_to:
            import os
            if os.path.exists(self.save_to):
                plt.savefig(os.path.join(self.save_to),
                            '%s_%d.png'%(prefix, eventid))
        handles, labels = ax1.get_legend_handles_labels()
        for i, l in enumerate(labels):
            labels[i] = l.replace('_','\_')
        try:
            # I don't know if mpltools is now in a standard matplotlib installation
            from mpltools import layout
            layout.pad_limits()
        except:
            pass
        plt.legend(handles, labels, prop={'size':10}, loc='best')
        plt.tight_layout()
        plt.grid()
        
        self.PushFrame(frame)

    def Finish(self):
        if self.show:
            import matplotlib.pyplot as plt
            plt.show()

    def plot_reconstruction(self, frame, reco_prefix,
                            xaxis="Slant", plot_fit=True, **kwargs):

        import numpy as n
        import matplotlib.pyplot as plt
        from matplotlib import rcParams

        # easier than formatting everything to latex compatible
        #rcParams.update({'text.usetex' : False})

        # assume that the frame contains DOMs, binned data, energy loss and a fit
        # plot dEdX for individual DOMs
        pars_name   = reco_prefix + 'Params'
        scatter_kwargs = dict(kwargs)
        scatter_kwargs.update({'color' : '#787878'})
        if frame.Has(reco_prefix+xaxis):
            plt.scatter(frame[reco_prefix+xaxis], frame[reco_prefix+'dEdX'], facecolors='none',
                        marker='.',**scatter_kwargs)
        xbinned     = frame[reco_prefix+xaxis+'binned']
        dEdXbinned  = frame[reco_prefix+'dEdX'+'binned']
        results     = frame[pars_name]

        if reco_prefix.find('True') > -1:
            plt.plot(xbinned, dEdXbinned, label=reco_prefix + ' true eloss', **kwargs)
        else:
            edEdXbinned = frame[reco_prefix+'dEdX_err'+'binned']
            # plot binned energy loss distribution
            plt.errorbar(xbinned, dEdXbinned, xerr=results.bin_width/2., yerr=edEdXbinned, fmt='+', 
                         capsize=0, markersize=8, label=reco_prefix+'edEdX'+'binned', **kwargs)

        # result of the fit for exponential fit
        if plot_fit and xaxis == "Slant":
            x = n.linspace(n.min(xbinned), n.max(xbinned),100)
            y = n.exp(results.N - results.b*x)
            plt.plot(x, y, '-', label=pars_name+' fit', **kwargs)
        elif plot_fit:
            icetray.logging.log_warn("Fit can only be plotted as function of slant depth," + 
                                     " skipping fit.")

# only look at EHE events
def filterFrame(frame):
    if frame.Has('FilterMask'):
        if frame['FilterMask']['EHEFilter_11'].prescale_passed:
            return True
    return False

# define tray and modules
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=filenamelist)

#tray.AddModule(filterFrame, 'filter_frame',
#               Streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

tray.AddModule('I3TrueMuonEnergy', 'me_true',
               BinWidth = options.SLANTBINSIZE,
               I3MCTree   = options.MCTREENAME,
               MMCTrackList      = options.MMCTRACKLIST,
               SaveEnergyLosses = True,
              )

tray.AddModule('I3MuonEnergy', 'me_reco',
               BinWidth          = options.SLANTBINSIZE,
               InputPulses        = options.PULSEMAP,
               MaxImpact            = options.MAXIMPACT,
               UseMonteCarloTrack   = True,
               I3MCTree= options.MCTREENAME,
               MMCTrackList              = options.MMCTRACKLIST,
               SaveDomResults = True,
              )

if options.SHOW:
    prefixes = ['I3MuonEnergyMC', 'I3TrueMuonEnergy']
    if options.SAVEPLOTS:
        savefigsto = os.path.join(os.getenv('I3_SRC'), 'ddddr', 'resources', 'scripts')
    else:
        savefigsto = False

    tray.AddModule(EventPlotter, 'eventplotter',
                   Prefixes      = prefixes,
                   SaveFiguresTo = savefigsto,
                   Show          = True,
                   XAxis         = 'Slant',
                   #If            = lambda frame : frame['FilterMask']['EHEFilter_11'].prescale_passed,
                  )
else:
    icetray.logging.log_warn("Use -s/--save-plots and/or --show to create plots. " +
                             "Only doing reconstruction now.")


if options.OUTFILE is not None:
    tray.AddModule('I3Writer', 'write',
                   FileName = options.OUTFILE,
                   Streams  = [icetray.I3Frame.Geometry, icetray.I3Frame.DAQ, icetray.I3Frame.Physics])


tray.Execute()


