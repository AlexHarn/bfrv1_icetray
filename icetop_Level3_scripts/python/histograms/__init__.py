from .getLivetime import Livetime
from .PulsesHistograms import IndividualTankPulses

from icecube import dataclasses, toprec, recclasses, phys_services

from icecube.production_histograms.histograms.frame_histograms import PhysicsHistogram

def MCPrimary(name):
    histograms = []
    histograms.append(PhysicsHistogram(0., 1., 90, name+"CosZenith", "cos(frame['%s'].dir.zenith/I3Units.rad)"%name))
    histograms.append(PhysicsHistogram(0., 90., 90, name+"Zenith", "frame['%s'].dir.zenith/I3Units.deg"%name))
    histograms.append(PhysicsHistogram(0., 360., 90, name+"Azimuth", "frame['%s'].dir.azimuth/I3Units.deg"%name))
    histograms.append(PhysicsHistogram(-2700, 2700, 80, name+"X", "frame['%s'].pos.x/I3Units.m"%name))
    histograms.append(PhysicsHistogram(-2700, 2700, 80, name+"Y", "(frame['%s'].pos.y/I3Units.m)"%name))
    histograms.append(PhysicsHistogram(1944., 1952., 80, name+"Z", "(frame['%s'].pos.z/I3Units.m)"%name))
    histograms.append(PhysicsHistogram(5., 9.5, 90, name+"Energy", "(frame['%s'].energy/I3Units.GeV)"%name))
    return histograms

def TopRec(name):
    histograms = []
    histograms.append(PhysicsHistogram(-2, 5, 100, name+"S125", "log10(frame['%sParams'].expected_signal(125))"%name))
    histograms.append(PhysicsHistogram(0., 1., 90, name+"CosZenith", "cos(frame['%s'].dir.zenith/I3Units.rad)"%name))
    histograms.append(PhysicsHistogram(0., 90., 90, name+"Zenith", "frame['%s'].dir.zenith/I3Units.deg"%name))
    histograms.append(PhysicsHistogram(0., 360., 90, name+"Azimuth", "frame['%s'].dir.azimuth/I3Units.deg"%name))
    histograms.append(PhysicsHistogram(-800, 800, 80, name+"X", "frame['%s'].pos.x/I3Units.m"%name))
    histograms.append(PhysicsHistogram(-800, 800, 80, name+"Y", "(frame['%s'].pos.y/I3Units.m)"%name))
    histograms.append(PhysicsHistogram(1944., 1952., 80, name+"Z", "(frame['%s'].pos.z/I3Units.m)"%name))
    # This is the right way, we need to ask Alex to import recclasses and simclasses.
    histograms.append(PhysicsHistogram(1., 5.5, 80, name+"Beta", "frame['%sParams'].value(recclasses.LaputopParameter.Beta)"%name))
    histograms.append(PhysicsHistogram(-1, 5., 100, name+"Chi2_LDF", "log10(frame['%sParams'].chi2_ldf)"%name))
    histograms.append(PhysicsHistogram(-1, 5., 100, name+"Chi2_Time", "log10(frame['%sParams'].chi2_time)"%name))
    # For current trunk (Aug 2016), we need ndf, for V05-00-00 and earlier we need ndf_ldf
    histograms.append(PhysicsHistogram(0,160, 160, name+"Ndf_LDF", "frame['%sParams'].ndf"%name))
    return histograms

def IceTopContainment(name):
    histograms = []
    histograms.append(PhysicsHistogram(0, 3, 100, name, "frame['%s'].value"%name))
    return histograms

def IceTopRecoPulses(name):
    histograms = []
    histograms.append(PhysicsHistogram(-1., 5., 100, name+"TotCharge", "log10(sum([p.charge for f in [frame] if '%s' in frame for k,pulses in dataclasses.I3RecoPulseSeriesMap.from_frame(f, '%s').iteritems() for p in pulses]))"%(name,name)))
    histograms.append(PhysicsHistogram(0., 162., 162, name+"NPulses", "len([p for f in [frame] if '%s' in frame for k,pulses in dataclasses.I3RecoPulseSeriesMap.from_frame(f, '%s').iteritems() for p in pulses])"%(name,name)))
    return histograms

# You cannot give a list to the histogram class. If you want to get individual charges, write your own module.
# For pulses: if it is a mask, you can do .apply(frame), but then we need to define a different "HistogramSuite" for that.

def InIceRecoPulses(name):
    histograms = []
    histograms.append(PhysicsHistogram(0., 7., 100, name+"TotCharge", "log10(sum([p.charge for f in [frame] if '%s' in frame for k,pulses in dataclasses.I3RecoPulseSeriesMap.from_frame(f, '%s').iteritems() for p in pulses]))"%(name,name)))
    histograms.append(PhysicsHistogram(0., 5000., 100, name+"NPulses", "len([p for f in [frame] if '%s' in frame for k,pulses in dataclasses.I3RecoPulseSeriesMap.from_frame(f, '%s').iteritems() for p in pulses])"%(name,name)))
    return histograms

#Millipede parameters, to check the branch where the LaputopTrack is used for the reconstruction.
def Millipede(name):
    from icecube import millipede
    histograms = []
    histograms.append(PhysicsHistogram(0., 7., 100, name+"QTotal", "log10(frame['%s'].qtotal)"%name))
    histograms.append(PhysicsHistogram(0., 7., 100, name+"Predicted_QTotal", "log10(frame['%s'].predicted_qtotal)"%name))
    #histograms.append(PhysicsHistogram(-5., 5., 100, name+"PredQTotalDivQTotal", "log10(frame['%s'].predicted_qtotal/frame['%s'].qtotal)"%(name,name))) #Is sensitive to problems in CalibrationErrata, badDOMs, ..
    histograms.append(PhysicsHistogram(-1, 3., 100, name+"Chi2", "log10(frame['%s'].chi_squared/frame['%s'].chi_squared_dof)"%(name,name)))
    return histograms

def StochasticReco(name):
    from icecube import stochastics
    histograms = []
    histograms.append(PhysicsHistogram(-3., 5., 100, name+"Eloss1500", "log10(frame['%s'].eLoss_1500)"%name))
    histograms.append(PhysicsHistogram(-2, 5., 100, name+"Chi2", "log10(frame['%s'].chi2)"%name))
    return histograms

def D4R(name):
    from icecube import ddddr
    histograms = []
    histograms.append(PhysicsHistogram(-3., 5., 100, name+"Mean", "log10(frame['%s'].mean)"%name))
    histograms.append(PhysicsHistogram(-2, 5., 100, name+"Chi2", "log10(frame['%s'].chi2ndof)"%name))
    histograms.append(PhysicsHistogram(-1, 3., 100, name+"_peak_energy", "log10(frame['%s'].peak_energy)"%name))
    return histograms

def InIceReco(name):
    histograms = []
    histograms.append(PhysicsHistogram(-600, 600, 100, name+"X", "frame['%s'].pos.x/I3Units.m"%name))
    histograms.append(PhysicsHistogram(-600, 600, 100, name+"Y", "frame['%s'].pos.y/I3Units.m"%name))
    histograms.append(PhysicsHistogram(-600, 600, 100, name+"Z", "frame['%s'].pos.z/I3Units.m"%name))
    histograms.append(PhysicsHistogram(0, 100, 100, name+"Rlogl", "frame['%sFitParams'].rlogl"%name))
    return histograms
