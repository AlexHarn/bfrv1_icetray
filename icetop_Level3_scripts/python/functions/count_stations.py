def count_stations(pulses):
    """
    Simple function to count pairs of pulses so we can count HLC stations after cleaning.
    """
    import numpy
    from icecube.icetray import I3Units
    time_window = 500*I3Units.ns
    tank_a = [k for k in pulses.keys() if k.om < 63]
    tank_b = [k for k in pulses.keys() if k.om > 62]
    count = 0
    for k_a in tank_a:
        for launch_a in pulses[k_a]:
            for k_b in tank_b:
                if k_b.string != k_a.string:
                    continue
                for launch_b in pulses[k_b]:
                    if numpy.abs(launch_a.time - launch_b.time) < time_window:
                        count += 1
                        break
    return count

def count_standard_stations(pulses):
    """                                                                                                                                                                                                     
    Simple function to count pairs of pulses so we can count HLC stations after cleaning.                                                                                                                   
    """
    import numpy
    from icecube.icetray import I3Units
    infill_stations=[79,80,81]
    time_window = 500*I3Units.ns
    tank_a = [k for k in pulses.keys() if k.om < 63]
    tank_b = [k for k in pulses.keys() if k.om > 62]
    count = 0
    for k_a in tank_a:
        if k_a.string not in infill_stations:
            for launch_a in pulses[k_a]:
                for k_b in tank_b:
                    if k_b.string != k_a.string:
                        continue
                    for launch_b in pulses[k_b]:
                        if numpy.abs(launch_a.time - launch_b.time) < time_window:
                            count += 1
                            break
    return count

def count_infill_stations(pulses):
    """                                                                                                                                                                                                     
    Simple function to count pairs of pulses so we can count HLC stations after cleaning.                                                                                                                   
    """
    import numpy
    from icecube.icetray import I3Units
    infill_stations=[26,27,36,37,46,79,80,81]
    time_window = 500*I3Units.ns
    tank_a = [k for k in pulses.keys() if k.om < 63]
    tank_b = [k for k in pulses.keys() if k.om > 62]
    count = 0
    for k_a in tank_a:
        if k_a.string in infill_stations:
            for launch_a in pulses[k_a]:
                for k_b in tank_b:
                    if k_b.string != k_a.string:
                        continue
                    for launch_b in pulses[k_b]:
                        if numpy.abs(launch_a.time - launch_b.time) < time_window:
                            count += 1
                            break
    return count
