#!/usr/bin/env python
import sys, os
from os.path import expandvars

from optparse import OptionParser

from I3Tray import I3Tray
from icecube import dataclasses, dataio, ddddr, icetray
icetray.logging.set_level(icetray.logging.I3LogLevel.LOG_WARN)

# take some test data that is already there
testdata = os.path.join(expandvars("$I3_TESTDATA"), 'icetop')
GCD = os.path.join(expandvars("$I3_TESTDATA"), 'GCD','GeoCalibDetectorStatus_2012.56063_V0.i3.gz')
L2FILE = os.path.join(testdata, 'Level3_IC86.2012_data_Run00120244_Part00_IT73_IT_coinc.i3.tgz')

########### Options ############
ofilename = os.path.join(os.getcwd(), 
                         'plot_events_reco.i3')

usage = "usage: %prog [options] inputfiles"
parser = OptionParser(usage)
parser.add_option("-o", "--outputfile", default=ofilename,
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.bz2} format)")
parser.add_option("-s", "--save-plots", default=False, action="store_true",
                  dest="SAVEPLOTS", 
                  help="Set flag to save plots for each event, default: %default")
parser.add_option("-t", "--seedtrack", default="Laputop",
                  dest="SEEDTRACKNAME", help="Seed Track to use, default: %default")
parser.add_option("-p", "--pulse-map", default="InIcePulses",
        dest="PULSEMAP", help="PulseMap to use, default: %default")
parser.add_option("--show", default=False, action="store_true", dest="SHOW",
                  help="Set flag to suppress plots for each event, default: %default")
parser.add_option("--skip-reco", default=False, action="store_true")
parser.add_option("-g", "--gcd", default=GCD, help='GCD File, default: %default')

(options, args) = parser.parse_args()

if len(args) < 1:
    if os.path.exists(L2FILE):
        filenames = [options.gcd, L2FILE]
    else:
        icetray.logging.log_warn("No input file specified and "+\
                                 "default test file %s not available. Quitting now."%L2FILE)
        sys.exit()
else:
    filenames = [options.gcd]
    filenames.extend(args)

# plotting_________________________

class EventPlotter(icetray.I3Module):

    def __init__(self, ctx):
        icetray.I3Module.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("Prefixes","List of prefixes of DDDDR reconstructions to plot",
                          ["I3MuonEnergy_"])
        self.AddParameter("SaveFiguresTo","If set, saves figures to given path", None)
        self.AddParameter("Show", "Shows plots at the end of the tray.", False)
        self.AddParameter("XAxis", "Plot energy against Slant or Depth.", "Slant")
        self.AddParameter("ShowChargeDistribution", 
                          "Shows figure with the charges vs. distance of all DOMs taken into account.",
                          False)
        self.AddParameter("ShowEnergyLossDistribution", 
                          "Shows figure with the energy losses vs. distances of all DOMs taken into account.",
                          False)
    
    def Configure(self):
        self.prefixes = self.GetParameter("Prefixes")
        self.save_to  = self.GetParameter("SaveFiguresTo")
        self.show     = self.GetParameter("Show")
        self.xaxis    = self.GetParameter("XAxis")
        self.show_charges = self.GetParameter("ShowChargeDistribution")
        self.show_energy_losses = self.GetParameter("ShowEnergyLossDistribution")

    def Physics(self, frame):
        import matplotlib
        if not self.show:
            matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib as mpl
        import matplotlib.gridspec as gs

        eventid = frame['I3EventHeader'].event_id
        if type(self.prefixes) != list:
            self.prefixes = [self.prefixes]

        usetex = matplotlib.rcParams['text.usetex']
        # differential energy loss plot
        plt.figure()
        ax1 = plt.subplot(111)
        for i, prefix in enumerate(self.prefixes):
            if not frame.Has(prefix + 'Params'):
                continue
            kws = dict()
            colors_ = mpl.rcParams['axes.color_cycle']
            kws.update({"color" : colors_[i%len(colors_)] })
            kws.update({"linewidth" : 2})
            kws.update({"plot_fit" : False})

            self.plot_reconstruction(frame, prefix, xaxis=self.xaxis, **kws)
            
            plot_title = '%s - event %d'%(prefix, eventid)
            plot_title = plot_title.replace('_','\_') if usetex else plot_title
            plt.title(plot_title)
            plt.xlabel("%s [m]"%self.xaxis)
            plt.ylabel("Energy Loss ($\mathrm{GeV / m}$)")

        handles, labels = ax1.get_legend_handles_labels()
        for i, l in enumerate(labels):
            labels[i] = l.replace('_','\_')
        try:
            # I don't know if mpltools is now in a standard matplotlib installation
            from mpltools import layout
            layout.pad_limits()
        except:
            pass

        #plt.legend(handles, labels, prop={'size':14}, loc='best')
        plt.grid()
        
        plt.yscale('log')
        ymin, ymax = plt.gca().get_ylim()
        plt.ylim((.1,ymax))
        plt.tight_layout()

        if self.save_to:
            import os
            if os.path.exists(self.save_to):
                plt.savefig(os.path.join(self.save_to,
                            '%s_%d.png'%(prefix, eventid)), format='png')

        # energy loss distributions
        if self.show_energy_losses:
            plt.figure()
            ax1 = plt.subplot(111)
            for i, prefix in enumerate(self.prefixes):
                sckwargs = dict()
                colors_ = mpl.rcParams['axes.color_cycle']
                sckwargs.update({"color" : colors_[i%len(colors_)] })
                self.scatter_energy_losses(frame, prefix, **sckwargs)
                plot_title = '%s - event %d'%(prefix, eventid)
                plot_title = plot_title.replace('_','\_') if usetex else plot_title
                plt.title(plot_title)
            
            plt.grid()
            plt.legend(loc='best')
            plt.xlabel('Distance to Cascade [m]')
            plt.ylabel('Reconstructed Energy [GeV]')
            plt.yscale('log')
            ymin, ymax = plt.gca().get_ylim()
            plt.ylim((1,ymax))

        # charge distributions
        if self.show_charges:
            plt.figure()
            ax1 = plt.subplot(111)
            for i, prefix in enumerate(self.prefixes):
                sckwargs = dict()
                colors_ = mpl.rcParams['axes.color_cycle']
                sckwargs.update({"color" : colors_[i%len(colors_)] })
                self.scatter_charges(frame, prefix, **sckwargs)
                plot_title = '%s - event %d'%(prefix, eventid)
                plot_title = plot_title.replace('_','\_') if usetex else plot_title
                plt.title(plot_title)
            
            plt.grid()
            plt.legend(loc='lower center')
            plt.xlabel('Distance to Cascade [m]')
            plt.ylabel('Charge [PE]')
            plt.yscale('log')
            ymin, ymax = plt.gca().get_ylim()
            plt.ylim((.1,ymax))

        self.PushFrame(frame)

    def Finish(self):
        if self.show:
            import matplotlib.pyplot as plt
            plt.show()

    def plot_reconstruction(self, frame, reco_prefix,
                            xaxis="Slant", plot_fit=False, **kwargs):

        import numpy as n
        import matplotlib.pyplot as plt
        from matplotlib import rcParams

        # easier than formatting everything to latex compatible
        #rcParams.update({'text.usetex' : False})

        # assume that the frame contains DOMs, binned data, energy loss and a fit
        # plot dEdX for individual DOMs
        pars_name   = reco_prefix + 'Params'
        scatter_kwargs = dict(kwargs)
        scatter_kwargs.update({'s' : 2})
        #scatter_kwargs.update({'color' : '#787878'})
        plt.scatter(frame[reco_prefix+xaxis], frame[reco_prefix+'dEdX'], facecolors='none',
                    marker='.',**scatter_kwargs)
        xbinned     = frame[reco_prefix+xaxis+'binned']
        dEdXbinned  = frame[reco_prefix+'dEdX'+'binned']
        edEdXbinned = frame[reco_prefix+'dEdX_err'+'binned']
        results     = frame[pars_name]

        # plot binned energy loss distribution
        plt.errorbar(xbinned, dEdXbinned, xerr=results.bin_width/2., yerr=edEdXbinned, fmt='+', 
                     capsize=0, markersize=8, label=reco_prefix+'edEdX'+'binned', **kwargs)

        plt.axhline(results.median, linestyle='--', color=kwargs['color'])
        plt.axhline(results.peak_energy, linestyle='-.', color=kwargs['color'])
        # result of the fit for exponential fit
        if plot_fit and xaxis == "Slant":
            x = n.linspace(n.min(xbinned), n.max(xbinned),100)
            y = n.exp(results.N - results.b*x)
            plt.plot(x, y, '-', label=pars_name+' fit', **kwargs)
        elif plot_fit:
            icetray.logging.log_warn("Fit can only be plotted as function of slant depth," + 
                                     " skipping fit.")


    def hist_energy_losses(self, frame, reco_prefix, **kwargs):
        import dashi as d
        d.visual()
        import numpy as n
        elosses = reco_prefix + 'CascadeEnergyLosses'
        if frame.Has(elosses):
            eloss = n.array(frame[elosses])
            h = d.histfactory.hist1d(n.log10(eloss), n.linspace(1,5,17))
            h.line()

    def scatter_energy_losses(self, frame, reco_prefix, **kwargs):
        import matplotlib.pyplot as plt
        elosses = reco_prefix + 'CascadeEnergyLosses'
        distances = reco_prefix + 'CascadeDomDistances'
        if frame.Has(elosses):
            eloss = frame[elosses]
            dist = frame[distances]

            plt.scatter(dist, eloss, s=15, alpha=0.5,label=reco_prefix.replace('_', '\_'), **kwargs)
        
    def scatter_charges(self, frame, reco_prefix, **kwargs):
        import matplotlib.pyplot as plt
        elosses = reco_prefix + 'CascadeDomCharges'
        distances = reco_prefix + 'CascadeDomDistances'
        if frame.Has(elosses):
            eloss = frame[elosses]
            dist = frame[distances]

            plt.scatter(dist, eloss, label=reco_prefix.replace('_', '\_'), **kwargs)

# tray_____________________________
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FileNameList=filenames)

# only look at EHE events
def filterFrame(frame):
    if frame.Has('FilterMask'):
        if frame['FilterMask']['EHEFilter_11'].prescale_passed:
            return True
    return False

#tray.AddModule(filterFrame, 'filter_frame',
#               Streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

prefixes = []
for impact in [150, 100, 75]:
    if not options.skip_reco:
        tray.AddModule('I3MuonEnergy', 'me'+str(impact),
                       InputPulses        = options.PULSEMAP,
                       MaxImpact            = impact,
                       Seed = options.SEEDTRACKNAME,
                       Prefix          = 'I3MuonEnergy_MaxImpact-%d_'%impact,
                       SaveDomResults = True,
                       Method = 1,
                      )
    prefixes.append('I3MuonEnergy_MaxImpact-%d_'%impact)

# only create plots if asked for since the script will potentially generate 
# a huge number of figures depending on the input file
if options.SAVEPLOTS or options.SHOW:
    if options.SAVEPLOTS:
        savefigsto = os.path.join(os.getenv('I3_SRC'), 'ddddr', 'resources', 'scripts')
    else:
        savefigsto = False

    tray.AddModule(EventPlotter, 'eventplotter',
                   Prefixes      = prefixes,
                   SaveFiguresTo = savefigsto,
                   Show          = options.SHOW,
                   XAxis         = 'Slant'
                  )
else:
    icetray.logging.log_warn("Use -s/--save-plots and/or --show to create plots. " +
                             "Only doing reconstruction now.")

if options.OUTFILE is not None:
    tray.AddModule('I3Writer', 'write',
                   FileName = options.OUTFILE,
                   Streams  = [icetray.I3Frame.Geometry, icetray.I3Frame.DAQ, icetray.I3Frame.Physics]
                  )


tray.Execute(30)

