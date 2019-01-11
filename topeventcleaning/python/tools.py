from icecube import icetray, dataclasses
from icecube.icetray import I3Units



def tank_geometry(geometry, om_key):
    """
    Returns the IceTop tank geometry object corresponding to a given DOM.
    """
    if om_key.om == 61 or om_key.om == 62:
        return geometry.stationgeo[om_key.string][0]
    elif om_key.om == 63 or om_key.om == 64:
        return geometry.stationgeo[om_key.string][1]
    return None


def to_shower_cs(fit):
    """
    Rotate to shower CS takes a fit (assumes fit.dir is set) and returns a rotation matrix.
    Requires numpy.
    """
    import numpy
    from math import cos, sin
    # counter-clockwise (pi + phi) rotation
    d_phi = numpy.matrix([ [ -cos(fit.dir.phi), -sin(fit.dir.phi), 0],
                           [  sin(fit.dir.phi), -cos(fit.dir.phi), 0],
                           [  0,                 0,                1] ])
    # clock-wise (pi - theta) rotation
    d_theta = numpy.matrix([ [  -cos(fit.dir.theta), 0, -sin(fit.dir.theta)],
                             [  0,                  1,  0,                ],
                             [  sin(fit.dir.theta), 0,  -cos(fit.dir.theta)] ])
    return d_theta*d_phi


def classify_from_seed(pulses, reco, geometry, min_time=-200*I3Units.ns, max_time=800*I3Units.ns, afterpulse_time=6500*I3Units.ns):
    """
    Classify pulses according to their agreement in time with a shower axis reconstruction.
    The classification is done using a constant cut on the time difference of the pulse time
    and the arrival time of the plane front at the position of the DOM.
    Requires numpy.

    Arguments:
     - pulses (a list of I3RecoPulse)
     - reco (an I3Particle with position and direction set)
     - geometry (an I3Geometry instance)
     - min_time (float, optional)
     - max-time (float, optional)
     - afterpulse_time (float, optional)

    Returns:
      A dictionary with keys ('ok', 'rejected', 'after-pulse').
      The values of the dictionary are lists of (OMKey, launch number) pairs.
      The lists are disjoint.
    """
    import numpy
    stations = []
    keys = []
    for k in pulses.keys():
        for launch, p in enumerate(pulses[k]):
            keys.append((k, launch))
            stations.append((0.,
                             launch,
                             tank_geometry(geometry, k).position.x,
                             tank_geometry(geometry, k).position.y,
                             tank_geometry(geometry, k).position.z,
                             p.charge, p.time, p.width, p.flags))

    stations = numpy.array(stations)

    if len(stations) == 0:
        return {'ok': [], 'after-pulses': [], 'rejected': []}

    # Rotate position to shower coordinates according to reco
    M = to_shower_cs(reco)
    try:
        from icecube.icetray.i3logging import log_trace
        log_trace("stations:"% stations, 'topeventcleaning')
    except:
        pass
    stations[:,2:5] -= numpy.array([reco.pos.x, reco.pos.y, reco.pos.z])
    stations[:,2:5] = numpy.array(M*stations[:,2:5].transpose()).transpose()

    # subtract travelling time of plane front from station time
    stations[:,6] -= reco.time - stations[:,4]/dataclasses.I3Constants.c

    # afterpulses occur some time after the pulse (at least afterpulse_time) and are never the first launch in the RecoPulse list.
    ap = [(keys[i][0], keys[i][1]) for i,p in enumerate(stations.tolist()) if p[6] > afterpulse_time and p[1] > 0.0]
    # OK pulses are within a time interval from the plane front and they are not afterpulses
    ok = [(keys[i][0], keys[i][1]) for i,p in enumerate(stations.tolist()) if not (keys[i][0], keys[i][1]) in ap and p[6] > min_time and p[6] < max_time]
    # The rest are rejected
    rej = [(keys[i][0], keys[i][1]) for i,p in enumerate(stations.tolist()) if not (keys[i][0], keys[i][1]) in ap and not (keys[i][0], keys[i][1]) in ok]

    return { 'ok':ok, 'after-pulses':ap, 'rejected':rej }


