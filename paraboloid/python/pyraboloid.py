#!/usr/bin/env python

from icecube.icetray import I3ConditionalModule,I3Units,logging
from icecube.gulliver import I3Gulliver
from icecube.gulliver import I3SeedService, I3SeedServiceBase
from icecube.gulliver import I3EventLogLikelihood, I3EventLogLikelihoodBase
from icecube.gulliver import I3Minimizer, I3MinimizerBase
from icecube.lilliput import I3SimpleParametrization
from icecube.dataclasses import I3Direction,I3Position,I3Particle
from icecube.paraboloid import I3ParaboloidFitParams
from .config_util import get_service_by_name_or_by_object
from .gridpoint import gridpoint
from .fit_paraboloid import fit_paraboloid

from numpy import cos,sin
import numpy as np

class pyraboloid(I3ConditionalModule):
    """
    Pyraboloid is a python reimplementation of the "paraboloid" angular error
    estimation algorithm.  The hope is that this will allow us to retire the
    old and messy C++ implementation (though fortunately it does not depend on
    ROOT anymore).
    The algorithm is described in Till Neunhoefer's thesis and in a NIM paper.
    A summary is given in the RST documentation of this project.  Once we have
    established that this works, we can hopefully make the code a slightly more
    generic to make it usable also for other types than 'infinite tracks'.
    Specifically, there is a request to make this work with starting tracks
    (obtained with multinest/hybridreco).
    Also: it would be nice to fit higher order terms as well. For events in which
    those terms are significant, fitting a pure paraboloid (to a LLH landscape that
    does not really look like a paraboloid) *may* result in wrongly fitted 2nd order 
    terms and hence a wrong angular uncertainty estimate. I have no idea how urgent
    this problem is. Nice student project. :-)
    """
    def __init__(self,ctx):
        super(pyraboloid,self).__init__(ctx)
        self.logl_service     = None
        self.seed_service     = None
        self.mini_service     = None
        self.xyz_step         = 10.*I3Units.m
        self.nrings           = 2
        self.npoints_per_ring = 8
        self.grid             = []
        self.AddParameter("LikelihoodService","Event likelihood service (should be the same as the one you used to get the seed fit).",None)
        self.AddParameter("SeedService","Seed service (delivers the input track for which you want to know the angular uncertainty).",None)
        self.AddParameter("GridPointSeedService","Optional point seed service (optional), used to tweak (e.g. vertex time) seeds on non-central grid points.",None)
        self.AddParameter("MinimizerService","Minimizer service (optional): used to optimize xyz for each direction grid point.",None)
        # TODO: split out grid making stuff into separate class
        self.AddParameter("VertexStepSize","Step size for vertex refit on each grid point (minimizer).",10.*I3Units.m)
        self.AddParameter("NRings","Number of rings around seed direction",2)
        self.AddParameter("NPointsPerRing","Number of grid points per ring",8)
        self.AddParameter("Radius","Radius (half opening angle) of the outer ring",1.*I3Units.degree)
    def Configure(self):
        self.logl_service     = get_service_by_name_or_by_object(self,"LikelihoodService",I3EventLogLikelihood,I3EventLogLikelihoodBase,False)
        self.seed_service     = get_service_by_name_or_by_object(self,"SeedService",I3SeedService,I3SeedServiceBase,False)
        self.mini_service     = get_service_by_name_or_by_object(self,"MinimizerService",I3Minimizer,I3MinimizerBase,True)
        self.xyz_step         = float(self.GetParameter("VertexStepSize"))
        self.nrings           = int(self.GetParameter("NRings"))
        self.npoints_per_ring = int(self.GetParameter("NPointsPerRing"))
        self.radius           = float(self.GetParameter("Radius"))
        if self.mini_service:
            self.para_service = I3SimpleParametrization(self.name+"_xyz_par")
            self.para_service.SetStep(I3SimpleParametrization.PAR_X,self.xyz_step,False)
            self.para_service.SetStep(I3SimpleParametrization.PAR_Y,self.xyz_step,False)
            self.para_service.SetStep(I3SimpleParametrization.PAR_Z,self.xyz_step,True)
            self.gulliver     = I3Gulliver(self.name,self.logl_service,self.para_service,self.mini_service)
            logging.log_debug("doing marginalization on grid points",unit=self.name)
        else:
            self.gulliver     = None
            logging.log_debug("omitting marginalization on grid points",unit=self.name)
    def Geometry(self,frame):
        g=frame["I3Geometry"]
        if self.gulliver:
            self.gulliver.SetGeometry(g)
        else:
            self.logl_service.SetGeometry(g)
        self.PushFrame(frame)
    def make_grid(self):
        # define cartesian frame w.r.t. seed direction
        dir0   = self.seed.particle.dir
        dirZEN = I3Direction(dir0.zenith+90.*I3Units.degree,dir0.azimuth)
        dirAZI = dir0.cross(dirZEN)
        # central grid point
        self.grid = [gridpoint(self.seed_service.GetCopy(self.seed),(0.,0.))]
        # self.xygrid = [(0,0)]
        # phi is the angular coordinate on the rings around the seed direction
        dphi=360*I3Units.degree/self.npoints_per_ring
        for i in range(1,self.nrings+1):
            r=i*self.radius/self.nrings
            cosr=cos(r)
            sinr=sin(r)
            for j in range(self.npoints_per_ring):
                sinphi = sin(j*dphi)
                cosphi = cos(j*dphi)
                perp   = cosphi*dirZEN+sinphi*dirAZI # perp is an I3Position object
                gdir   = cosr*dir0+sinr*perp
                seedcp = self.seed_service.GetCopy(self.seed)
                seedcp.particle.dir = I3Direction(*gdir)
                gridproj = (sinr*cosphi, sinr*sinphi)
                self.grid.append(gridpoint(seedcp,gridproj))
    def Physics(self,frame):
        nseeds = self.seed_service.SetEvent(frame)
        if nseeds<1:
            return
        self.seed = self.seed_service.GetSeed(0)
        self.make_grid()
        self.logl_service.SetEvent(frame)
        self.neglogl_values = []
        if self.gulliver:
            self.gulliver.SetEvent(frame)
        else:
            self.logl_service.SetEvent(frame)
        for g in self.grid:
            if self.gulliver:
                fitparams = self.gulliver.Fit(g.hypothesis)
                g.value = fitparams.logl
            else:
                g.value = -self.logl_service.GetLogLikelihood(g.hypothesis)
        logging.log_debug("\n".join("%s %f" % (g.hypothesis.particle.dir,g.value) for g in self.grid),unit=self.name)
        axyt   = np.transpose([g.gridproj for g in self.grid])
        alogl  = np.array([g.value for g in self.grid])
        logging.log_debug("axyt=%s" % axyt)
        logging.log_debug("alogl=%s" % alogl)
        fitpar = fit_paraboloid(axyt,alogl,self.seed.particle.dir,self.name)
        frame.Put(self.name+"FitParams",fitpar)
        self.PushFrame(frame)
