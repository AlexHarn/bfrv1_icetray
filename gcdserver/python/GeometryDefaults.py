
"""
file: GeometryDefaults.py
Contains magic numbers for geometry objects.  These numbers were
never measured/verified for any particular object, but they are
assumed to be reasonable default values in the case more accurate
numbers are not contained in the database.
"""

from I3Tray import I3Units

# Area of R7081/R7081-100 PMT
R7081PMTArea = 0.044400 * I3Units.m2

# Default IceTop dimensions
IceTopTankRadius = 0.93 * I3Units.m
IceTopTankHeight = 1.30 * I3Units.m
IceTopTankFillHeight = 0.90 * I3Units.m
IceTopTankOrientation = 0.
