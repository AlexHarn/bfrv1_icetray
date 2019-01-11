from icecube.icetop_Level3_scripts import icetop_globals

def AssembleExcludedList(frame, out='IceTopExcludedTanks',
                         ExcludedStations=[],
                         ExcludedOMs=[icetop_globals.icetop_bad_doms],
                         ExcludedTanks=[icetop_globals.icetop_bad_tanks,
                                        #'TankPulseMergerExcludedSLCTanks',
                                        icetop_globals.icetop_tank_pulse_merger_excluded_tanks,
                                        icetop_globals.icetop_cluster_cleaning_excluded_tanks]):
    from icecube import dataclasses
    exclude = dataclasses.TankKey.I3VectorTankKey()
    for tag in ExcludedTanks:
        if tag in frame:
            tanks = frame[tag]
            for key in tanks:
                exclude.append(key)
    for tag in ExcludedStations:
        if tag in frame:
            stations = frame[tag]
            for string in stations:
                exclude.append(dataclasses.TankKey(string, dataclasses.TankKey.TankA))
                exclude.append(dataclasses.TankKey(string, dataclasses.TankKey.TankB))
    for tag in ExcludedOMs:
        if tag in frame:
            oms = frame[tag]
            for om in oms:
                if om.om <= 60:
                    continue
                exclude.append(dataclasses.TankKey(om))
    if out in frame:
        frame.Delete(out)
    frame.Put(out, exclude)
