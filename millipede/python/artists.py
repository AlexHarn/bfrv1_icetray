from icecube.shovelart import *
from icecube import icetray, dataclasses, millipede
import numpy
from scipy.stats import chi2
from scipy.special import gammaln
from matplotlib.colors import colorConverter as mplcol
from icecube.steamshovel.artists.mplart import MPLArtist

def get_photonics(tablePrefix):
    if hasattr(get_photonics, 'instance') and hasattr(get_photonics, 'prefix'):
        if get_photonics.prefix == tablePrefix:
            return get_photonics.instance
    from icecube import photonics_service
    from os.path import expandvars
    get_photonics.prefix = tablePrefix
    tablePath = expandvars(tablePrefix)
    tablePath += ".%s.fits"
    get_photonics.instance = \
           photonics_service.I3PhotoSplineService(tablePath % 'abs',
                                                  tablePath % 'prob', 0.0)
    return get_photonics.instance

class Millipede( MPLArtist ):
   
    def __init__(self):
        MPLArtist.__init__(self)
        self.defineSettings( {'OMKeys':OMKeySet(),
                              'PhotonsPerBin':5,
                              'BinSigma':0,
                              'legend':True,
                              'time range':1500,
                              'differential':False,
                              'photonTablePrefix': '$I3_DATA/photon-tables/splines/ems_mie_z20_a10'
                              } )
        self.context = icetray.I3Context()
        self.millipede = millipede.PyPyMillipede(self.context)
    
    numRequiredKeys = 2

    def isValidKey( self, frame, key_idx, key ):
        if key_idx == 0:
            try:
                return dataclasses.I3RecoPulseSeriesMap.from_frame(frame, key) is not None
            except TypeError:
                return False
            except RuntimeError:
                return False
        elif key_idx == 1:
            try:
                return isinstance(frame[key], dataclasses.I3Particle) or isinstance(frame[key], dataclasses.ListI3Particle)
            except RuntimeError:
                return False
    
    def create_plot( self, frame, fig ):
        keys = self.setting('OMKeys')
        
        if len(keys) == 0:
            return
        
        dkey = self.keys()[0]
        
        if not dkey+'TimeRange' in frame:
            header = frame['I3EventHeader']
            frame[dkey+'TimeRange'] = dataclasses.I3TimeWindow(-200, header.end_time-header.start_time+6400)
        
        self.millipede.SetParameter('Pulses', dkey)
        if self.setting("BinSigma") > 0:
            self.millipede.SetParameter('BinSigma', self.setting("BinSigma"))
        else:
            self.millipede.SetParameter('PhotonsPerBin', self.setting("PhotonsPerBin"))
        
        photonTablePrefix = self.setting('photonTablePrefix')
        self.millipede.SetParameter('CascadePhotonicsService',
                                    get_photonics(photonTablePrefix))
        
        self.millipede.DatamapFromFrame(frame)
        
        axes = fig.add_subplot(1,1,1)
        # protect against missed clicks
        keys = [k for k in keys if k in self.millipede.domCache]
        
        source = frame[self.keys()[1]]
        if isinstance(source, dataclasses.I3Particle):
            sources = dataclasses.ListI3Particle([source])
        else:
            sources = dataclasses.ListI3Particle([p for p in source if p.is_cascade])
            if not all([p.is_cascade for p in source]):
                warnings.warn("Removed {} non-cascade particles from hypothesis".format(len(source)-sum([p.is_cascade for p in source])))
        response = self.millipede.GetResponseMatrix(sources)
        expectations = numpy.inner(response.to_I3Matrix(), [p.energy for p in sources])
        slices = dict()
        i = 0
        for k, dc in self.millipede.domCache.items():
            valid = sum(dc.valid)
            slices[k] = slice(i, i+valid)
            i += valid
        
        start, end = float('inf'), float('-inf')
        colors = axes._get_lines.color_cycle
        for k in keys:
            dc = self.millipede.domCache[k]
            left = dc.time_bin_edges[:-1]
            right = dc.time_bin_edges[1:]
            widths = right-left
            color = colors.next()

            ex = numpy.zeros(dc.charges.size)
            ex[numpy.where(dc.valid)[0]] = expectations[slices[k]]
            if self.setting('differential'):
                ex /= widths
            x = numpy.zeros( 2*len(dc.time_bin_edges), numpy.float )
            y = numpy.zeros( 2*len(dc.time_bin_edges), numpy.float )

            x[0::2], x[1::2] = dc.time_bin_edges, dc.time_bin_edges
            y[1:-1:2], y[2::2] = ex, ex
            # axes.fill(x, y, color=color, alpha=0.5)
            axes.plot(x, y, color=color, linewidth=2)
            
            # construct a 68% credible interval for the mean of a Poisson
            # distribution, given a single sample
            cl = 0.68
            a = (1-cl)/2.
            lo = chi2.ppf(a, 2*dc.charges)/2.
            lo[numpy.isnan(lo)] = 0
            hi = chi2.ppf(1-a, 2*(dc.charges+1))/2.
            yerr = dc.charges-lo, hi-dc.charges
            y = dc.charges
            if self.setting('differential'):
                yerr = (yerr[0]/widths, yerr[1]/widths)
                y = y/widths
            if self.setting('differential'):
                ex *= widths
            mask = numpy.where(numpy.asarray(dc.valid)&(ex>0))[0]
            q = dc.charges[mask]
            lam = ex[mask]
            logl = q*numpy.log(lam) - lam - gammaln(q+1)
            label='OM(%d,%d): %.1f/%1.f PE $\ln L = %.0f$' % (k.string, k.om, dc.charges.sum(), lam.sum(), logl.sum())
            axes.errorbar((left+right)/2, y, xerr=(right-left)/2., yerr=yerr, fmt=None, ecolor=color, label=label)
            
            if( 'I3Geometry' in frame ):
                self.addOverlayLine( frame['I3Geometry'].omgeo[k].position, mplcol.to_rgb(color) )
            if len(dc.time_bin_edges) > 2:
                if left[1]-50 < start:
                    start = left[1]-50
                if right[-2] > end:
                    end = right[-2]
        axes.set_xlim(start, start+self.setting('time range'))
        axes.set_title('%s: %.1e GeV' % (self.keys()[1], sources[0].energy))
        if self.setting('differential'):
            axes.set_ylabel('Collected charge [PE/ns]')
        else:
            axes.set_ylabel('Collected charge [PE]')
        axes.set_xlabel('Time [ns]')
        
        if( self.setting('legend') ):
            axes.legend(prop={'size':'small'}, loc='best')

Scenario.registerArtist(Millipede)
