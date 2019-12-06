#!/usr/bin/env python

"""
IceProd module for ``MuonGun`` simulations
"""

from icecube.simprod.util import ReadI3Summary, WriteI3Summary
from icecube.simprod.util import CombineHits, DrivingTime
import os.path
from icecube.simprod.util import simprodtray
from icecube.simprod.util.simprodtray import RunI3Tray
import argparse
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.phys_services
from I3Tray import I3Tray, I3Units
from icecube.simprod.util import BasicCounter
from icecube.simprod.segments import GenerateCosmicRayMuons, PropagateMuons, GenerateNaturalRateMuons
from icecube import clsim
from icecube import polyplopia
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
from icecube.production_histograms.histogram_modules.simulation.mctree import I3MCTreeModule
from icecube.production_histograms.histogram_modules.simulation.mcpe_module import I3MCPEModule


def add_args(parser):
    """
    Args:
        parser (argparse.ArgumentParser): the command-line parser
    """
    simprodtray.add_argument_gcdfile(parser)
    simprodtray.add_argument_outputfile(parser)

    simprodtray.add_argument_nproc(parser)
    simprodtray.add_argument_procnum(parser)
    simprodtray.add_argument_seed(parser)
    simprodtray.add_argument_usegslrng(parser)

    parser.add_argument("--nevents", dest="nevents",
                        default=1, type=int, required=False,
                        help="number of generated events")
    parser.add_argument("--model", dest="model",
                        default="Hoerandel5_atmod12_SIBYLL",
                        type=str, required=False,
                        help="primary cosmic-ray flux parametrization")
    parser.add_argument("--gamma", dest="gamma",
                        default=2., type=float, required=False,
                        help="power law spectral index")
    parser.add_argument("--offset", dest="offset",
                        default=700., type=float, required=False,
                        help="power law offset in GeV")
    parser.add_argument("--emin", dest="emin",
                        default=1e4, type=float, required=False,
                        help="mininum generated energy in GeV")
    parser.add_argument("--emax", dest="emax",
                        default=1e7, type=float, required=False,
                        help="maximum generated energy in GeV")
    parser.add_argument("--length", dest="length",
                        default=1600., type=float, required=False,
                        help="cylinder length in m")
    parser.add_argument("--radius", dest="radius",
                        default=800., type=float, required=False,
                        help="cylinder radius in m")
    parser.add_argument("--x", dest="x",
                        default=0., type=float, required=False,
                        help="cylinder x-position in m")
    parser.add_argument("--y", dest="y",
                        default=0., type=float, required=False,
                        help="cylinder y-position in m")
    parser.add_argument("--z", dest="z",
                        default=0., type=float, required=False,
                        help="cylinder z-position in m")
    parser.add_argument("--length-dc", dest="length_dc",
                        default=500., type=float, required=False,
                        help="inner cylinder length in m")
    parser.add_argument("--radius-dc", dest="radius_dc",
                        default=150., type=float, required=False,
                        help="inner cylinder radius in m")
    parser.add_argument("--x-dc", dest="x_dc",
                        default=46.3, type=float, required=False,
                        help="inner cylinder x-position in m")
    parser.add_argument("--y-dc", dest="y_dc",
                        default=-34.9, type=float, required=False,
                        help="inner cylinder y-position in m")
    parser.add_argument("--z-dc", dest="z_dc",
                        default=-300., type=float, required=False,
                        help="inner cylinder z-position in m")
    parser.add_argument("--deepcore", dest="deepcore",
                        default=False, action="store_true", required=False,
                        help="use inner cylinder")
    parser.add_argument("--no-propagate-muons", dest="propagate_muons",
                        default=True, action="store_false", required=False,
                        help='Run PROPOSAL.')
    parser.add_argument("--no-propagate-photons", dest="propagate_photons",
                        default=True, action="store_false", required=False,
                        help='Run ClSim.')
    parser.add_argument("--summaryfile", dest="summaryfile",
                        default="muongun.json", type=str, required=False,
                        help="Summary filename")
    parser.add_argument("--HistogramFilename", dest="histogramfilename",
                        default=None, type=str, required=False,
                        help='Histogram filename.')
    parser.add_argument("--EnableHistogram", dest="enablehistogram",
                        default=False, action="store_true", required=False,
                        help='Write a SanityChecker histogram file.')
    parser.add_argument("--natural-rate", dest="natural_rate",
                        default=False, action="store_true", required=False,
                        help="Sample natural rate muon bundles")
    parser.add_argument("--GPU", dest="gpu",
                        default=None, type=str, required=False,
                        help="Graphics Processing Unit number (shoud default to environment if None)")
    parser.add_argument("--UseGPUs", dest="usegpus",
                        default=False, action="store_true", required=False,
                        help="Use Graphics Processing Unit")
    parser.add_argument("--UseOnlyDeviceNumber", dest="useonlydevicenumber",
                        default=0, type=int, required=False,
                        help="Use only this device.")
    parser.add_argument("--oversize", dest="oversize",
                        default=5, type=int, required=False,
                        help="over-R: DOM radius oversize scaling factor")
    parser.add_argument("--holeiceparametrization", dest="holeiceparametrization",
                        default=os.path.expandvars("$I3_SRC/ice-models/resources/models/angsens/as.h2-50cm"),
                        type=str, required=False,
                        help="Location of hole ice param files")
    parser.add_argument("--efficiency", dest="efficiency",
                        default=[1.00], type=simprodtray.float_comma_list, required=False,
                        help="overall DOM efficiency correction")
    parser.add_argument("--IceModelLocation", dest="icemodellocation",
                        default=os.path.expandvars("$I3_BUILD/ice-models/resources/models"),
                        type=str, required=False,
                        help="Location of ice model param files")
    parser.add_argument("--IceModel", dest="icemodel",
                        default="spice_3.2", type=str, required=False,
                        help="ice model subdirectory")
    parser.add_argument("--PhotonSeriesName", dest="photonseriesname",
                        default="I3MCPESeriesMap", type=str, required=False,
                        help="Photon Series Name")
    parser.add_argument("--RawPhotonSeriesName", dest="rawphotonseriesname",
                        default=None, type=str, required=False,
                        help="Raw Photon Series Name")
    parser.add_argument("--no-KeepMCTree", dest="keepmctree",
                        default=True, action="store_false", required=False,
                        help='Delete propagated MCTree otherwise')


def configure_tray(tray, params, stats, logger):
    """
    Configures the I3Tray instance: adds modules, segments, services, etc.

    Args:
        tray (I3Tray): the IceProd tray instance
        params (dict): command-line arguments (and default values)
                            referenced as dict entries; see add_args()
        stats (dict): dictionary that collects run-time stats
        logger (logging.Logger): the logger for this script
    """
    if params['gpu'] is not None and params['usegpus']:
        os.putenv("CUDA_VISIBLE_DEVICES", str(params['gpu']))
        os.putenv("COMPUTE", ":0." + str(params['gpu']))
        os.putenv("GPU_DEVICE_ORDINAL", str(params['gpu']))

    tray.AddModule("I3InfiniteSource", "TheSource",
                   Prefix=params['gcdfile'],
                   Stream=icecube.icetray.I3Frame.DAQ)

    if params['natural_rate']:
        tray.AddSegment(GenerateNaturalRateMuons, "muongun",
                        NumEvents=params['nevents'],
                        mctree_name="I3MCTree_preMuonProp",
                        flux_model="GaisserH4a_atmod12_SIBYLL")
    else:
        # Configure tray segment that actually does stuff.
        tray.AddSegment(GenerateCosmicRayMuons, "muongun",
                        mctree_name="I3MCTree_preMuonProp",
                        num_events=params['nevents'],
                        flux_model=params['model'],
                        gamma_index=params['gamma'],
                        energy_offset=params['offset'],
                        energy_min=params['emin'],
                        energy_max=params['emax'],
                        cylinder_length=params['length'],
                        cylinder_radius=params['radius'],
                        cylinder_x=params['x'],
                        cylinder_y=params['y'],
                        cylinder_z=params['z'],
                        inner_cylinder_length=params['length_dc'],
                        inner_cylinder_radius=params['radius_dc'],
                        inner_cylinder_x=params['x_dc'],
                        inner_cylinder_y=params['y_dc'],
                        inner_cylinder_z=params['z_dc'],
                        use_inner_cylinder=params['deepcore'])

    if params['propagate_muons']:
        tray.AddSegment(PropagateMuons, "propagator",
                        RandomService=tray.context["I3RandomService"],
                        CylinderLength=params['length'],
                        CylinderRadius=params['radius'],
                        InputMCTreeName="I3MCTree_preMuonProp",
                        OutputMCTreeName="I3MCTree")

    tray.AddModule(BasicCounter, "count_events",
                   Streams=[icecube.icetray.I3Frame.DAQ],
                   name="Generated Events",
                   Stats=stats)

    if params['propagate_photons']:
        if not params['propagate_muons']:
            raise BaseException("You have to propagate muons if you want to propagate photons")

        if len(params['efficiency']) == 1:
            efficiency = params['efficiency'][0]
        elif len(params['efficiency']) > 1:
            efficiency = params['efficiency']
        else:
            raise Exception("Configured empty efficiency list")

        try:
            tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                            GCDFile=params['gcdfile'],
                            RandomService=tray.context["I3RandomService"],
                            UseGPUs=params['usegpus'],
                            UseOnlyDeviceNumber=params['useonlydevicenumber'],
                            UseCPUs=not params['usegpus'],
                            IceModelLocation=os.path.join(params['icemodellocation'], params['icemodel']),
                            UnshadowedFraction=efficiency,
                            UseGeant4=False,
                            DOMOversizeFactor=params['oversize'],
                            MCTreeName="I3MCTree",
                            MCPESeriesName=params['photonseriesname'],
                            PhotonSeriesName=params['rawphotonseriesname'],
                            HoleIceParameterization=params['holeiceparametrization'])
            tray.AddModule("MPHitFilter", "hitfilter",
                           HitOMThreshold=1,
                           RemoveBackgroundOnly=False,
                           I3MCPESeriesMapName=params['photonseriesname'])
        except AttributeError as e:
            print(e)
            print("Nevermind...not propagating photons.")

    if not params['keepmctree']:
        logger.info("discarding %s" % (params['photonseriesname']))
        tray.Add("Delete", "clean_mctruth",
                 Keys=["I3MCTree", 'I3MCTree_preSampling'])

    if params['enablehistogram'] and params['histogramfilename']:
        tray.AddModule(ProductionHistogramModule,
                       Histograms=[I3MCTreePrimaryModule, I3MCTreeModule, I3MCPEModule],
                       OutputFilename=params['histogramfilename'])


def main():
    """
    IceProd module for ``MuonGun`` simulations
    """
    # Get Params
    parser = argparse.ArgumentParser(description="MuonGunGenerator script")
    add_args(parser)
    params = vars(parser.parse_args())  # dict()

    # Execute Tray
    summary = RunI3Tray(params, configure_tray, "MuonGunGenerator",
                        summaryfile=params['summaryfile'],
                        summaryin=icecube.dataclasses.I3MapStringDouble(),
                        outputfile=params['outputfile'],
                        seed=params['seed'],
                        nstreams=params['nproc'],
                        streamnum=params['procnum'],
                        usegslrng=params['usegslrng'])


if __name__ == "__main__":
    main()
