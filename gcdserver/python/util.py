
from icecube import dataclasses
from icecube.icetray import OMKey, I3Units

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import math


def getOMKey(o):
    """
    Retrieve an OMKey instance from geometry object 'o'.  Throw
    KeyException if geometry keys are not available.
    @o: Geometry object
    @return: OMKey
    """
    try:
        # First check if we have a PMT ID
        return OMKey(int(o.data[G.Keys.STRING_ID]),
                     int(o.data[G.Keys.POSITION_ID]),
                     int(o.data[G.Keys.PMT_ID]))
    except KeyError:
        pass
    # No PMT ID.  Just use string/position
    return OMKey(int(o.data[G.Keys.STRING_ID]),
                 int(o.data[G.Keys.POSITION_ID]))


def getPosition(o):
    """
    Construct an I3Position object representing the position data in 'o'
    @o: Geometry object
    @return: I3Position
    """
    return dataclasses.I3Position(float(o.data[G.Keys.GEOMETRY_X] * I3Units.m),
                                  float(o.data[G.Keys.GEOMETRY_Y] * I3Units.m),
                                  float(o.data[G.Keys.GEOMETRY_Z] * I3Units.m))


def getOrientationDirection(o):
    """
    Construct an I3Direction object representing the z-axis of 'o'
    @o: Geometry object
    @return: I3Direction
    """
    if all(key in o.data for key in (G.Keys.ORIENTATION_NX,
                                     G.Keys.ORIENTATION_NY,
                                     G.Keys.ORIENTATION_NZ)):
        return dataclasses.I3Direction(float(o.data[G.Keys.ORIENTATION_NX]),
                                       float(o.data[G.Keys.ORIENTATION_NY]),
                                       float(o.data[G.Keys.ORIENTATION_NZ]))
    if o.data[G.Keys.ORIENTATION] == G.Orientation.UP:
        return dataclasses.I3Direction(0., 0., 1.)
    if o.data[G.Keys.ORIENTATION] == G.Orientation.DOWN:
        return dataclasses.I3Direction(0., 0., -1.)
    raise Exception("Object %s: unknown Z orientation" % o.name)


def getOrientation(o):
    """
    Construct an I3Orientation object representing
    the orientation data in 'o'
    @o: Geometry object
    @return: I3Orientation
    """
    z_up = getOrientationDirection(o)
    # Default to old behavior: azimuth=0
    azimuth = 0.
    if G.Keys.ORIENTATION_AZIMUTH in o.data:
        azimuth = float(o.data[G.Keys.ORIENTATION_AZIMUTH])
    x = math.cos(azimuth)
    y = math.sin(azimuth)
    z = -1. * (x * z_up.x + y * z_up.y) / z_up.z
    x_up = dataclasses.I3Direction(x, y, z)
    return dataclasses.I3Orientation(z_up, x_up)


def getDOMCalFit(o):
    """
    Return a LinearFit object from a DOMCal linear fit
    """
    fit = dataclasses.LinearFit()
    fit.slope = o.data[C.Keys.SLOPE]
    fit.intercept = o.data[C.Keys.INTERCEPT]
    return fit


def setI3Time(i3Time, timeStr, frac):
    """
    Set an I3Time object using a time string of the format
    "%Y-%m-%d %H:%M:%S" and fractional second, in DAQ units (integer
    number of 1e-10 seconds).  Use instead of e.g. datetime.strptime
    because the time string has leap second information (second=60)
    that set_utc_cal_date can use that would be lost by
    e.g. datetime.strptime.
    """
    i3Time.set_utc_cal_date(int(timeStr.split(' ')[0].split('-')[0]),
                            int(timeStr.split(' ')[0].split('-')[1]),
                            int(timeStr.split(' ')[0].split('-')[2]),
                            int(timeStr.split(' ')[1].split(':')[0]),
                            int(timeStr.split(' ')[1].split(':')[1]),
                            int(timeStr.split(' ')[1].split(':')[2]),
                            1e-1 * frac)


def setStartStopTime(target, source):
    """
    Copy the start/end times from source to target
    """
    target.start_time = source.start_time
    target.end_time = source.end_time
