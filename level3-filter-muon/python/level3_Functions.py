from icecube import icetray, dataclasses, dataio
from icecube.phys_services import I3Calculator as calc
import numpy

from I3Tray import *

# Get (and use) track information for the highest-level
# reconstruction that suceeded
def GetBestTrack(frame, Suffix):
    tracks=["MPEFit", "SPEFit2", "SPEFitSingle", "LineFit"]
    if Suffix!="":
        Suffix="_"+Suffix
    tracks=[track+Suffix for track in tracks]
    if frame.Has("BestTrack"):
        frame.Delete("BestTrack")
    if frame.Has("BestTrackName"):
        frame.Delete("BestTrackName")
    HasGoodTrack=False
    for track in tracks:
        if frame[track].fit_status==dataclasses.I3Particle.OK:
            HasGoodTrack=True
            frame["BestTrack"]=frame[track]
            best_track=track
            frame.Put("BestTrackName",dataclasses.I3String(best_track))
            return True
    if not HasGoodTrack:
        return False

# Function to calculate qtot with and without DeepCore,
# and to calculate AvgDistQ without DeepCore
def CalcChargeCuts(frame, Pulses, Track):
    if frame.Stop==icetray.I3Frame.Physics:
        # First, calculate AvgDistQ and Qtot
        if len(frame[Pulses].apply(frame))==0:
            AvgDistQ=0.
            Qtot=0.
            Qtot_DC=0.
        else:
            pulses=frame[Pulses].apply(frame).items()
            track=frame[Track]
            geo=frame.Get("I3Geometry")
            omgeo=geo.omgeo
            AvgDistQ=0.
            Qtot=0.
            Qtot_DC=0.
            DeepCoreStrings=[79,80,81,82,83,84,85,86]
            for item in pulses:
                if item[0].string not in DeepCoreStrings:
                    DomPosition=omgeo[item[0]].position
                    Dist=calc.closest_approach_distance(track,DomPosition)
                    # Calc Qtot and Qdom without DeepCore
                    Qdom=0.
                    for p1 in item[1]:
                        Qdom+=p1.charge
                    Qtot+=Qdom
                    AvgDistQ+=Dist*Qdom
                else:
                    # Calc Qtot with DeepCore too
                    Qdom=0.
                    for p1 in item[1]:
                        Qdom+=p1.charge
                    Qtot_DC+=Qdom
            if Qtot==0:
                AvgDistQ=numpy.nan
            else:
                AvgDistQ/=Qtot
        if frame.Has(Track+"_AvgDistQ"):
            frame.Delete(Track+"_AvgDistQ")
        frame.Put(Track+"_AvgDistQ",dataclasses.I3Double(AvgDistQ))
        if frame.Has(Pulses+"_Qtot"):
            frame.Delete(Pulses+"_Qtot")
        frame.Put(Pulses+"_Qtot",dataclasses.I3Double(Qtot))
        if frame.Has(Pulses+"_QtotWithDC"):
            frame.Delete(Pulses+"_QtotWithDC")
        frame.Put(Pulses+"_QtotWithDC",dataclasses.I3Double(Qtot+Qtot_DC))

@icetray.traysegment
def CleanInputStreams(tray, Name):
    def muon_stream(frame):
        if frame.Has("FilterMask"):
            for filter in ["MuonFilter_12", "MuonFilter_13"]:
                if frame["FilterMask"].has_key(filter) and bool(frame["FilterMask"][filter]):
                    return True
        else:
            return False
        if frame.Has("QFilterMask"):
            for filter in ["EHEFilter_12", "EHEFilter_13"]:
                if frame["QFilterMask"].has_key(filter) and bool(frame["QFilterMask"][filter]):
                    return True
        return False

    def OnlyInIce(frame):
        if frame["I3EventHeader"].sub_event_stream=="InIceSplit":
            return True
        else:
            return False

    # get only in_ice events that pass the MuonFilter, EHEFilter
    tray.AddModule(muon_stream)

    tray.AddModule(OnlyInIce)

    tray.AddModule("I3OrphanQDropper")
