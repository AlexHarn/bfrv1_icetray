from icecube import icetray, dataclasses, dataio, phys_services, shield
from icecube.phys_services import I3Calculator as calc
from icecube.common_variables import direct_hits, hit_multiplicity, hit_statistics, track_characteristics
import numpy

from I3Tray import *

from .level3_Functions import GetBestTrack, CalcChargeCuts

@icetray.traysegment
def CalculateCutValues(tray, name, Pulses, Suffix, If):
    tray.AddModule(GetBestTrack,
        Suffix=Suffix,
        If=If)

    tray.AddModule(CalcChargeCuts,
        Pulses=Pulses,
        Track='BestTrack',
        If=If)

    # Common Variables
    DirectHitsDefs=direct_hits.get_default_definitions()

    DirectHitsDefs.append(direct_hits.I3DirectHitsDefinition("E",-15.,250.))

    tray.AddSegment(hit_multiplicity.I3HitMultiplicityCalculatorSegment, "HitMultiplicityCutValues",
        PulseSeriesMapName=Pulses,
        OutputI3HitMultiplicityValuesName="HitMultiplicityValues",
        BookIt=True,
        If=If)

    tray.AddSegment(hit_statistics.I3HitStatisticsCalculatorSegment, "HitStatisticsCutValues",
        PulseSeriesMapName=Pulses,
        OutputI3HitStatisticsValuesName="HitStatisticsValues",
        BookIt=True,
        COGBookRefFrame=dataclasses.converters.I3PositionConverter.BookRefFrame.Sph,
        If=If)

    tray.AddSegment(direct_hits.I3DirectHitsCalculatorSegment, "DirectHitsBestTrackCutValues",
        DirectHitsDefinitionSeries=DirectHitsDefs,
        PulseSeriesMapName=Pulses,
        ParticleName="BestTrack",
        OutputI3DirectHitsValuesBaseName="BestTrackDirectHits",
        BookIt=True,
        If=If)

    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, "TrackCharacteristsBestTrackCutValues",
        PulseSeriesMapName=Pulses,
        ParticleName="BestTrack",
        OutputI3TrackCharacteristicsValuesName="BestTrackCharacteristics",
        TrackCylinderRadius=150*I3Units.m,
        BookIt=True,
        If=If)
