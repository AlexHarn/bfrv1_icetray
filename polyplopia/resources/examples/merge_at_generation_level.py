"""
This example script show how to inject background events (either from MuonGun or
a CORSIKA file at generation-level (i.e. before lepton propagation).

It is assumed that the input file cointains primary MCTrees
(CORSIKA-5comp, NuGen, GENIE, etc.) and the background file contains CR-showers
pregenerated by CORSIKA with a natural spectrum (e.g. Hoerandel Polygonato).
"""

import icecube
from icecube import icetray, dataio, phys_services, polyplopia, MuonGun, dataclasses
from icecube.simprod import segments, util
from optparse import OptionParser

from I3Tray import *

usage = "usage: %prog [options]"
parser = OptionParser(usage)
parser.add_option("-o", "--outfile", default="merged.i3", dest="OUTFILE", help="Write output to OUTFILE (.i3{.gz} format)")
parser.add_option("-g", "--gcdfile", dest="GCDFILE", help="Input Geometry Calibration Detector status (.i3{.gz} format)")
parser.add_option("--seed",type="int", default=12344, dest="SEED", help="Initial seed for the random number generator")
parser.add_option("--inputfile", dest="INPUTFILE", 
                   help="Pre-generated input events (.i3{.gz} format)")
parser.add_option("--backgroundfile",dest="BACKGROUNDFILE", 
                   help="Pre-generated background CORSIKA events (.i3{.gz} format)")
parser.add_option("--nevents", type="int", dest="NEVENTS", default=0, help="Stop after given number of events")
parser.add_option("--propagate-photons",dest="RUNCLSIM", action="store_true",
                   default=False, help="Propagate photons for combined events")
parser.add_option("--use-gpu", dest="GPU", action="store_true", 
                   default=False, help="Use GPU for photon propagation")

(options,args) = parser.parse_args()
if len(args) != 0:
        crap = "Got undefined options:"
        for a in args:
                crap += a
                crap += " "
        parser.error(crap)


tray = I3Tray()

gcdfile = options.GCDFILE
inputfile = options.INPUTFILE


randomService = phys_services.I3SPRNGRandomService(options.SEED,nstreams=2,streamnum=0)
randomServiceForPropagators = phys_services.I3SPRNGRandomService(options.SEED,nstreams=2,streamnum=1)

tray.context["I3RandomService"] = randomService
tray.context["I3RandomServiceForPropagators"] = randomServiceForPropagators
tray.context['I3SummaryService'] = dataclasses.I3MapStringDouble()

if options.BACKGROUNDFILE:
   # Read pre-generated CORSIKA file with CR showers. Assumed that events inf i3
   # files follow natural spectrum (e.g. Hoerandel)
   background = polyplopia.CoincidentI3ReaderService(options.BACKGROUNDFILE)

else: # No background file
   # Default: use Hoerandel as a template for generating muons.
   model = icecube.MuonGun.load_model("Hoerandel5_atmod12_SIBYLL")

   # Generate only single muons, no bundles.
   model.flux.max_multiplicity = 1

   gamma_index=2.
   energy_offset=700.
   energy_min=1e4
   energy_max=1e7
   cylinder_length=1600.
   cylinder_radius=800.
   cylinder_x=0.
   cylinder_y=0.
   cylinder_z=0.
   inner_cylinder_length=500.
   inner_cylinder_radius=150.
   inner_cylinder_x=46.3
   inner_cylinder_y=-34.9
   inner_cylinder_z=-300.


   # Default: cylinder aligned with z-axis at detector center as injection
   # surface.
   outsurface_center = icecube.dataclasses.I3Position(
        cylinder_x*icecube.icetray.I3Units.m,
        cylinder_y*icecube.icetray.I3Units.m,
        cylinder_z*icecube.icetray.I3Units.m)

   outsurface = icecube.MuonGun.Cylinder(
        length=cylinder_length*icecube.icetray.I3Units.m,
        radius=cylinder_radius*icecube.icetray.I3Units.m,
        center=outsurface_center)

   # Draw energies from a power law with offset.
   spectrum = icecube.MuonGun.OffsetPowerLaw(
        gamma=gamma_index,
        offset=energy_offset*icecube.icetray.I3Units.GeV,
        min=energy_min*icecube.icetray.I3Units.GeV,
        max=energy_max*icecube.icetray.I3Units.GeV)

   generator = icecube.MuonGun.StaticSurfaceInjector(
            outsurface, model.flux, spectrum, model.radius)

   background = polyplopia.MuonGunBackgroundService()
   background.set_generator(generator)
   background.set_rng(randomService)
   background.set_rate(5.0*I3Units.kilohertz)
   background.set_mctree_name("I3MCTree")


# Configure tray
tray.AddModule("I3Reader","reader",filenamelist=[gcdfile,inputfile])
tray.AddModule("Delete","del", keys=["I3MCTree","MMCTrackList"])
tray.AddModule("Rename","rename", keys=["I3MCTree_preMuonProp","I3MCTree"])
tray.AddModule("PoissonMerger","merge", CoincidentEventService = background)
tray.AddModule("Rename",keys=["I3MCTree","I3MCTree_preMuonProp"])

# Pregenerated events are assumed not to be propagated
tray.AddSegment(segments.PropagateMuons, "PropagateMuons",
    RandomService = randomServiceForPropagators)

if options.RUNCLSIM:
    # Do the rest of the simulaton
    tray.AddSegment(segments.PropagatePhotons, "normalpes",
            RandomService = "I3RandomService",
            HybridMode = False,
            IgnoreMuons = False,
            IgnoreCascades = False,
            UseGPUs = options.GPU,
            UseAllCPUCores = False,
            InputMCTree = "I3MCTree",
            UseGeant4 = False)

    tray.AddModule("MPHitFilter","hitfilter", 
        HitOMThreshold=1,
    )

tray.AddModule("Dump")


tray.AddModule("I3Writer","write",Filename=options.OUTFILE)

if options.NEVENTS:
  tray.Execute(options.NEVENTS)
else:
  tray.Execute()


summary = tray.context['I3SummaryService']
util.WriteI3Summary(summary, "muongunbg.xml")

