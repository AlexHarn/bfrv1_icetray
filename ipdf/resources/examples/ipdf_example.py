
#example ipdf python interface

import os.path
from icecube import icetray,dataclasses,dataio,ipdf

#get geometry from test data
i3file = dataio.I3File(
    os.path.join(os.environ["I3_TESTDATA"],"GCD",
                 "GeoCalibDetectorStatus_2012.56063_V0.i3.gz"))
while True:
    frame = i3file.pop_frame()
    if frame.Stop == icetray.I3Frame.Geometry:
        geometry = frame['I3Geometry']
        break
pdf = ipdf.muon_pandel_spe1st(geometry)

#get data from L3 file
i3file = dataio.I3File(
    os.path.join(os.environ["I3_TESTDATA"],"sim",
                 "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"))

frame=True
while i3file.more():
    frame = i3file.pop_physics()
    pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, 'InIcePulses')
    pdf.set_pulses(pulsemap)
    particle = frame['SPEFit2_HV']
    for omkey,pulseseries in pulsemap:
        liklihood = pdf.get_likelihood(particle,omkey)
        print ("{!r:14} {:10.4e}".format(omkey, liklihood))

