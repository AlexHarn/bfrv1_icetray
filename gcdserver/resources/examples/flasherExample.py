#!/usr/bin/env python

from icecube.gcdserver.I3FlasherSubrunMapBuilder import buildI3FlasherSubrunMap
from icecube.gcdserver.I3Live import getLiveFlasherData
from icecube import icetray, dataclasses


def main():
    # Note: Here we get the flasher subrun map directly from I3Live.
    # Normally, it is found in the D-frame with key 'I3FlasherSubrunMap'
    flasherSubrunMap = buildI3FlasherSubrunMap(
                        getLiveFlasherData(129335, "live.icecube.wisc.edu"))
    # flasherSubrunMap is a map of subrun number to a map of flashing DOMs
    # Print the entrire I3FlasherSubrunMap object to see all the data
    #print flasherSubrunMap
    # Print the subruns for which we have data
    print "Subruns available: %s" % [s for s in list(flasherSubrunMap.keys())]
    # Examine subrun 1:
    subrun = 1
    subrunData = flasherSubrunMap[subrun]
    # Print the OMKey for DOMs that were flashing during this subrun
    print ("DOMs available in subrun %s: %s" %
                           (subrun, [k for k in list(subrunData.keys())]))
    # Examine OMKey(86, 41)
    omKey = icetray.OMKey(86, 41)
    domData = subrunData[omKey]
    print "Data for %s: %s" % (omKey, domData)
    # Access the data for this DOM
    print "Printing data for %s:" % omKey
    print "  Brightness: %s" % domData.brightness
    print "  Window': %s" % domData.window
    print "  Rate: %s" % domData.rate
    print "  Mask: %s" % domData.mask
    print "  Delay: %s" % domData.delay


if __name__ == "__main__":
    main()