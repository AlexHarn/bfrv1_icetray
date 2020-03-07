'''
Is there a big in the "ChangeSnowHeight_interpolated" function?
'''

#import csv
import os
import datetime
import unittest

from icecube import icetray,dataio,dataclasses
from icecube.icetray.i3logging import log_fatal
from I3Tray import I3Tray
from icecube.icetop_Level3_scripts.modules import ChangeSnowHeight_interpolated

fname = os.path.expandvars('${I3_BUILD}/icetop_Level3_scripts/resources/data/IT81-MasterwithSnowMeasurements-Mar2019.csv')
fdate = datetime.date(2016, 10, 1)

## Create the object
tray = I3Tray()
icetray.I3Logger.global_logger.set_level(icetray.I3LogLevel.LOG_INFO)

#CSHi = ChangeSnowHeight_interpolated(icetray.I3Module())
#harray = CHSi._read_file(self, fdate, fname)

## Load a random GCD file se we can mess with it
gcd = os.path.expandvars('${I3_TESTDATA}/GCD/GeoCalibDetectorStatus_2016.57531_V0.i3.gz')
tray.Add('I3Reader', 'reader', Filename = gcd)


'''
headers = ['Station', 'Tank', 'Jan-08', 'Jan-09', 'Jan-10', 'Feb-10', 'Dec-10', 'Feb-11', '16-Nov-11', '19-Feb-12', '21-Oct-12', '9-Jan-13', '21-Feb-13', '25-Oct-13', '11-Dec-13', '8-Mar-14', '3-Nov-14', '27-Feb-15', '28-Oct-15', '28-Dec-15', '1-Apr-16', '10-Oct-16', '22-Dec-16', '8-Mar-17', '13-Nov-17', '9-Mar-18', '4-Nov-18', '6-Mar-19']

## Example of a station that's been around forever
st21A = [21, 'A', 0.9, 0.81, 1.219, '-', 1.52, 1.461, 1.695, 1.725, 1.935, 1.925, 1.895, 2.095, 2.135, 2.125, 2.465, 2.477, 2.575, 2.545, 2.595, 2.815, 2.815, 2.785, 3.105, 3.074, 3.277, 3.29]
st21B = [21, 'B', 0.9, 0.79, 1.301, '-', 1.443, 1.386, 1.674, 1.694, 2.024, 1.994, 1.974, 2.204, 2.204, 2.204, 2.434, 2.448, 2.524, 2.534, 2.514, 2.714, 2.714, 2.734, 2.974, 3.007, 3.324, 3.273]

## Example of a station that wasn't deployed until Dec. 2010
st17A = [17, 'A', '-', '-', 0.009, '-', 0.099, 0.059, 0.309, 0.299, 0.409, 0.409, 0.439, 0.739, 0.799, 0.839, 1.229, 1.167, 1.319, 1.389, 1.419, 1.719, 1.699, 1.699, 1.829, 1.802, 2.031, 2.107]
st17B = [17, 'B', '-', '-', 0.001, '-', 0.046, 0.0, 0.177, 0.197, 0.357, 0.387, 0.407, 0.607, 0.667, 0.697, 1.027, 1.084, 1.197, 1.237, 1.227, 1.377, 1.357, 1.377, 1.597, 1.567, 1.872, 1.897]

## Example of a station that wasn't deployed until Feb. 2011:
st1A = [1, 'A', '-', '-', '-', '-', '-', 0.077, 0.287, 0.327, 0.467, 0.437, 0.437, 0.727, 0.757, 0.767, 0.907, 0.967, 1.187, 1.237, 1.227, 1.427, 1.437, 1.467, 1.697, 1.721, 1.949, 1.924]
st1B = [1, 'B', '-', '-', '-', '-', '-', 0.032, 0.207, 0.197, 0.287, 0.297, 0.307, 0.597, 0.607, 0.597, 0.977, 0.977, 1.087, 1.167, 1.167, 1.297, 1.397, 1.407, 1.487, 1.525, 1.703, 1.932]

## Example of a station with a "hole" in April 1, 2016:
st36A = [36, 'A', '-', '-', 0.002, '-', 0.086, 0.201, 0.332, 0.332, 0.372, 0.412, 0.372, 0.612, 0.592, 0.592, 1.142, 1.101, 1.222, 1.242, '-', 1.432, 1.422, 1.392, 1.572, 1.571, 1.736, 1.762]
st36B = [36, 'B', '-', '-', 0.001, '-', 0.238, 0.304, 0.349, 0.379, 0.589, 0.589, 0.559, 0.709, 0.739, 0.759, 1.289, 1.311, 1.429, 1.489, '-', 1.549, 1.539, 1.539, 1.899, 1.87, 1.997, 2.098]
'''

##------ TEST #1: a date that lands exactly on a measurement date (and one station has a "hole" there) ----
tray.Add(ChangeSnowHeight_interpolated, 'updateSnow-2016-Apr1',
    Filename = os.path.expandvars('${I3_BUILD}/icetop_Level3_scripts/resources/data/IT81-MasterwithSnowMeasurements-Mar2019.csv'),
    Day      = 1,
    Month    = 4,
    Year     = 2016
)


## Now test the output:
class TestTheChange(icetray.I3Module,unittest.TestCase):
  def __init__(self, ctx):
    icetray.I3Module.__init__(self,ctx)
    self.AddParameter('whichtest','Which test?', 0)
    self.AddOutBox("OutBox")

  def Configure(self):
    self.which = self.GetParameter('whichtest')
    
  def Geometry(self, frame):
    if 'I3Geometry' in frame:
        geom = frame['I3Geometry']
        stageo = geom.stationgeo
        hA = {}
        hB = {}
        ## Let's pick a station:
        for e in [1, 17, 21, 36]:
          st = stageo[e]
          hA[e] = st[0].snowheight
          hB[e] = st[1].snowheight
          print("St ", e, " heightA = ", hA[e], ", heightB = ", hB[e])
        ## Test 'em:
        if (self.which <= 0):
          log_fatal("You have to enter a test number (1,2,etc)")
        elif (self.which == 1):  ## APRIL 1, 2016
          self.assertTrue(hA[1] == 1.227 and hB[1] == 1.167)
          self.assertTrue(hA[17] == 1.419 and hB[17] == 1.227)
          self.assertTrue(hA[21] == 2.595 and hB[21] == 2.514)
          self.assertAlmostEqual(hA[36], 1.304891986)   #<--- where the "hole" is, so interpolated!
          self.assertAlmostEqual(hB[36], 1.508860627)   #<--- where the "hole" is, so interpolated!
        elif (self.which == 2):  ## SEPT 15, 2009
          self.assertTrue(hA[1] == 0.0 and hB[1] == 0.0)
          self.assertAlmostEqual(hA[17], 0.0063369863)
          self.assertAlmostEqual(hB[17], 0.0007041096)
          self.assertAlmostEqual(hA[21], 1.0979808219)
          self.assertAlmostEqual(hB[21], 1.1498)
          self.assertAlmostEqual(hA[36], 0.0014082192)
          self.assertAlmostEqual(hB[36], 0.0007041096)
        else:
          log_fatal("Test number %d is not programmed up yet."%self.which)

    else:
        log_fatal('No geometry found')
    self.PushFrame(frame,"OutBox")

tray.Add(TestTheChange,"test1")(("whichtest", 1))


## TEST #2: interpolate somewhere early, before all the tanks existed (but still within bounds)
tray.Add(ChangeSnowHeight_interpolated, 'updateSnow-2009-Sept15',
    Filename = os.path.expandvars('${I3_BUILD}/icetop_Level3_scripts/resources/data/IT81-MasterwithSnowMeasurements-Mar2019.csv'),
    Day      = 15,
    Month    = 9,
    Year     = 2009
)
tray.Add(TestTheChange,"test2")(("whichtest", 2))


tray.Execute()

