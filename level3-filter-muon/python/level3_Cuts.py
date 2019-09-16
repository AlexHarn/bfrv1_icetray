from icecube import icetray, dataclasses, dataio, phys_services, hdfwriter
from icecube.phys_services import I3Calculator as calc
import numpy

from I3Tray import *

from .level3_Functions import GetBestTrack, CalcChargeCuts

# MuonFilter_12 zenith-charge cut for downgoing events
def PoleCut(zenith, qtot):
    if qtot<=0.:
        return False
    logQtot=numpy.log10(qtot)
    cosZenith=numpy.cos(zenith)
    # muon filter 12 regions cut
    if zenith <= numpy.radians(60):
        PoleCut=(logQtot>=(0.6*(cosZenith-0.5)+2.6))
        return PoleCut
    elif zenith > numpy.radians(60) and zenith < numpy.radians(78.5):
        PoleCut=(logQtot>=(3.9*(cosZenith-0.5)+2.6))
        return PoleCut
    # no cut on the 3rd downgoing region right now
    elif zenith >= numpy.radians(78.5) and zenith < numpy.radians(85):
        return True
    elif numpy.isnan(zenith):
        return False
    else:
        sys.exit("Ahhh!  What do I do with a zenith of: %5.3f??" % (zenith))

# Function for level3 zenith-dependent qtot cut
def QtotZenithCut(Zenith,Qtot):
    cZ = numpy.cos(Zenith)
    logQtot = numpy.log10(Qtot)
    C={"p0":0.155847026166,
       "p1":13.9060109867,
       "p2":-28.1169390193,
       "p3":26.4767509931,
       "p4":-9.34974616578}

    return (logQtot>=(C["p0"]+C["p1"]*cZ+C["p2"]*cZ**2+C["p3"]*cZ**3+C["p4"]*cZ**4))

# Function to keep only frames that pass the level3 cuts
def DoLevel3Cuts(frame, Track, Pulses, Suffix):
    if frame.Stop==icetray.I3Frame.Physics:
        Zenith=frame[Track].dir.zenith
        LineFitZenith=frame["LineFit_"+Suffix].dir.zenith
        NDir=frame["BestTrackDirectHitsE"].n_dir_doms
        LDir=frame["BestTrackDirectHitsE"].dir_track_length
        # if all llh fits failed, don"t let the event
        # satisfy the cut via rlogl
        # it can still satisfy via direct hits...
        BestTrackName=frame["BestTrackName"].value
        if "Line" in BestTrackName:
            Rlogl=1000
        else:
            Rlogl=frame["%sFitParams" % (BestTrackName)].rlogl
        NCh=frame["HitMultiplicityValues"].n_hit_doms
        Qtot=frame[Pulses+"_Qtot"].value
        QtotWithDC=frame[Pulses+"_QtotWithDC"].value

        if NCh>5:
            Plogl=Rlogl*(NCh-5)/(NCh-3)
        else:
            Plogl=1000
            
        DirectEllipse=(LDir/180)**2+(NDir//10)**2
        Level3Cut=((DirectEllipse>2 and NDir>6) or (Plogl<7.5) or (Rlogl<9.0))

        if (Qtot>10**4):
            return True
        if Zenith<numpy.radians(85):
            PoleCutBool=PoleCut(Zenith,QtotWithDC)
            if (Level3Cut and PoleCutBool):
                if QtotZenithCut(Zenith, Qtot):
                    return True
                else:
                    return False
            else:
                return False
        elif Zenith>=numpy.radians(85):
            if (Level3Cut):
                return True
            else:
                return False
        else:
            sys.exit("DoLevel3Cuts: What kind of Zenith is %5.4f!?" % (Zenith))

@icetray.traysegment
def DoPrecuts(tray, Name, Pulses, Suffix, If):
    def MuonL3Precut(frame, Pulses, Track):
        if frame.Stop==icetray.I3Frame.Physics:
            # First, get cut variables from frame
            AvgDistQ=frame[Track+"_AvgDistQ"].value
            Qtot=frame[Pulses+"_Qtot"].value
            QtotWithDC=frame[Pulses+"_QtotWithDC"].value
            Track=frame[Track]
            # Now, make cuts
            PassPreCut=False
            if AvgDistQ < 90 or Qtot > 100:
                PassPreCut=True

            # Upgoing - standard precut
            if Track.dir.zenith>=numpy.radians(85) and PassPreCut:
                return True
            # Downgoing - standard precut and pole filter cut (with DC)
            elif Track.dir.zenith<numpy.radians(85):
                PoleCutBool=PoleCut(Track.dir.zenith,QtotWithDC)
                if PoleCutBool and PassPreCut:
                    return True
                else:
                    return False
            else:
                return False

    tray.AddModule(GetBestTrack,
            Suffix=Suffix,
            If=If)

    tray.AddModule(CalcChargeCuts,
            Pulses=Pulses,
            Track="BestTrack",
            If=If)

    # Make precuts, drop orphan Q-frames
    tray.AddModule(MuonL3Precut,
            Pulses=Pulses,
            Track="BestTrack",
            If=If)

    tray.AddModule("I3OrphanQDropper")

