import numpy as np
from icecube.shovelart import *
from icecube.dataclasses import I3Particle, I3RecoPulseSeriesMap
from icecube.icetray import I3Units
from icecube.icetray import logging
from icecube.ipdf import muon_pandel_spe1st, muon_pandel_speall, muon_pandel_mpe


class I3DOMLikelihoodArtist( PyArtist ):
    """
    A steamshovel artist to visualize what signal likelihood each DOM contributes to the total event likelihood
    """

    requiredTypes = [ I3Particle, I3RecoPulseSeriesMap ]

    # start with these
    # would be nice to have a solution that is not hard coded
    likelihoods={ "spe1st": muon_pandel_spe1st,
                  "speall": muon_pandel_speall,
                  "mpe": muon_pandel_mpe }

    def __init__( self ):
        PyArtist.__init__(self)
        # lhlist=ArtistKeylist(self.likelihoods.keys()) does not work?
        self.defineSettings( {
            #"Likelihood"       : ArtistKeylist(self.likelihoods.keys()), # does not work
            "Likelihood"       : "spe1st",
            "NoiseColor"       : PyQColor.fromRgb(0,0,0),
            "log10(NoiseLevel)": -8,
            "SignalColorMap"   : I3TimeColorMap(),
            "DOMScale"         : 15,
            "Jitter [ns]"      : 15,
            "Rescale"          : False,
            "Debug"            : True,
        } )
        self.likelihood_name_set = ""
        self.likelihood = None
        self.jitter_set = None
    def description( self ):
        return "Likelihood for each hit DOM"

    def isValidKey( self, frame, key_idx, key ):
        if key_idx == 0:
            return frame.type_name(key) == 'I3Particle'
        if key_idx != 1:
            print("key index is %d ????" % key_idx)
            raise ValueError("unrecognized value for key index")
        # following lines are copied from steamshovel's I3RecoPulseSeries artist
        try:
            return I3RecoPulseSeriesMap.from_frame(frame, key) is not None
        except TypeError:
            return False
        except RuntimeError:
            return False

    def create( self, frame, output ):
        # input
        particlename = self.keys()[0]
        particle = frame[particlename]
        if particle.fit_status != I3Particle.OK:
            logging.log_notice("particle %s did not converge (status is %d=%s)" % (particlename,particle.fit_status,particle.fit_status_string))
            return
        pulsemap = I3RecoPulseSeriesMap.from_frame(frame, self.keys()[1])
        # configurables
        lhname     = self.setting("Likelihood")
        noisecolor = self.setting("NoiseColor")
        noiselevel = self.setting("log10(NoiseLevel)")
        noisefloor = 10**noiselevel
        colormap   = self.setting("SignalColorMap")
        domscale   = self.setting("DOMScale")
        jitter     = self.setting("Jitter [ns]")
        rescale    = self.setting("Rescale")
        debug      = self.setting("Debug")

        if lhname not in self.likelihoods.keys():
            logging.log_error("likelihood name %s is not recognized, use one of: '%s'" % (lhname,"', '".join(self.likelihoods.keys())))
            if self.likelihood_name_set not in self.likelihoods.keys():
                return
            lhname = self.likelihood_name_set

        geo = frame["I3Geometry"]
        if (self.likelihood is None) or (jitter != self.jitter_set) or (self.likelihood_name_set != lhname):
            if debug:
                logging.log_notice("initializing %s likelihood using geometry with %d DOMs and jitter=%d ns" % (lhname,len(geo.omgeo),jitter),unit="ipdf")
            self.likelihood = self.likelihoods[lhname](geo,jitter*I3Units.ns)
            self.jitter_set = jitter
        self.likelihood.set_pulses(pulsemap)

        hit_doms = [ dom for dom, pulseseries in pulsemap if len(pulseseries)>0 ]
        charges = [ np.sum([pulse.charge for pulse in pulseseries]) for pulseseries in pulsemap.values() if len(pulseseries)>0 ]
        lhs = [ self.likelihood.get_likelihood(particle,dom) for dom,pulseseries in pulsemap if len(pulseseries)>0 ]

        if rescale:
            loglhmin=np.log(np.min(lhs)+noisefloor)
            loglhmax=np.log(np.max(lhs))
        else:
            loglhmin=np.log(2*noisefloor)
            loglhmax=np.log(1.) # actually it can get higher, but we force it to be <=1
        if loglhmin<loglhmax:
            norm=1./(loglhmax-loglhmin)
        else:
            # if for some reason loglhmin or max is NAN then we also land here
            logging.log_error("OOPS: loglh min=%g max=%g" % (loglhmin,loglhmax),unit="ipdf")
            logging.log_error("maybe: noiselevel=%d too high? noisefloor=10^noiselevel=%g" % (noiselevel,noisefloor),unit="ipdf")
            return


        for dom,npe,lh in zip(hit_doms,charges,lhs):
            if npe<1.:
                npe=1.
            domsize=domscale*(1.+np.log10(npe))
            s = output.addSphere( domsize, geo.omgeo[dom].position )
            if debug:
                logging.log_notice("%s likelihood=%g npe=%g noisefloor=%g" % (dom,lh,npe,noisefloor),unit="ipdf")
            loglh=np.log(noisefloor+lh)
            if loglh>loglhmax:
                loglh=loglhmax
            if lh>noisefloor:
                f=(loglhmax-loglh)*norm
                logging.log_notice("%s f=%g"%(dom,f),unit="ipdf")
                s.setColor(colormap.value(f))
            else:
                s.setColor(noisecolor)

try:
    from icecube.shovelart import Scenario
    from artists import I3DOMLikelihoodArtist
    Scenario.registerArtist(I3DOMLikelihoodArtist)
    logging.log_debug("successful registation of ipdf artist",unit="ipdf")
except:
    logging.log_warn("registation of ipdf artist failed?",unit="ipdf")
    pass

