print(" ===== Welcome to SnowSuite =====")
print("")
print("                /\ ")
print("           __   \/   __")
print("           \_\_\/\/_/_/ ")
print("             _\_\/_/_ ")
print("            __/_/\_\__ ")
print("           /_/ /\/\ \_\ ")
print("                /\ ")
print("                \/ ")
print("")
print(" ======== Process Weight ========")

# just load in argparse so the help function can be quickly accessed
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-i", "--infiles", type=str, required=True,
                    dest="infiles", nargs="+",
                    help="Files to be processed.")

parser.add_argument("-c", "--LICfile", type=str, required=True,
                    dest="LICfile",
                    help="Path to LIC file from LeptonInjector event generation.")

parser.add_argument("-o", "--outfile", type=str, required=True,
                    dest="outfile",
                    help="Path to (i3) output file.")

parser.add_argument("--interaction", type=str, required=True,
                    dest="interaction", choices=["CC", "NC", "GR"],
                    help="Interaction type used in LeptonInjector.")

# set default paths to differential cross sections
default_data_location = "/cvmfs/icecube.opensciencegrid.org/data/neutrino-generator/cross_section_data/csms_differential_v1.0/"
parser.add_argument("--dsdxdy_nu_CC", type=str,
                    dest="dsdxdy_nu_CC", default=default_data_location+"dsdxdy_nu_CC_iso.fits",
                    help="dsdxdy_nu_CC fits file to use.")

parser.add_argument("--dsdxdy_nubar_CC", type=str,
                    dest="dsdxdy_nubar_CC", default=default_data_location+"dsdxdy_nubar_CC_iso.fits",
                    help="dsdxdy_nubar_CC fits file to use.")

parser.add_argument("--dsdxdy_nu_NC", type=str,
                    dest="dsdxdy_nu_NC", default=default_data_location+"dsdxdy_nu_NC_iso.fits",
                    help="dsdxdy_nu_NC fits file to use.")

parser.add_argument("--dsdxdy_nubar_NC", type=str,
                    dest="dsdxdy_nubar_NC", default=default_data_location+"dsdxdy_nubar_NC_iso.fits",
                    help="dsdxdy_dsdxdy_nubar_NCnu_CC fits file to use.")

parser.add_argument("--log-level", dest="log_level",
                    type=str, default="WARN",
                    help="Sets the icetray logging level (ERROR, WARN, INFO, DEBUG, TRACE)")

args = parser.parse_args()

# import time to measure runtime
import time
start_time = time.time()

#import icecube packages
print("=====> Importing necessary packages...")
import os
#import warnings

from I3Tray import I3Tray
from icecube import icetray, dataio, dataclasses
from icecube import LeptonInjector
import LeptonWeighter as LW

import pickle
import numpy as np
print("=====> Importing done")

# set argparser arguments
infiles         = args.infiles
outfile         = args.outfile
LIConfiguration = args.LICfile
interaction     = args.interaction.lower()
log_level       = args.log_level
dsdxdy_nu_CC    = args.dsdxdy_nu_CC
dsdxdy_nubar_CC = args.dsdxdy_nubar_CC
dsdxdy_nu_NC    = args.dsdxdy_nu_NC
dsdxdy_nubar_NC = args.dsdxdy_nubar_NC

NFiles = len(infiles)
print("Running on {} files:".format(NFiles))
print("\n".join(infiles))

# set icetray logging level
log_levels = {"error" : icetray.I3LogLevel.LOG_ERROR,
              "warn" : icetray.I3LogLevel.LOG_WARN,
              "info" : icetray.I3LogLevel.LOG_INFO,
              "debug" : icetray.I3LogLevel.LOG_DEBUG,
              "trace" : icetray.I3LogLevel.LOG_TRACE}
if log_level.lower() in log_levels.keys():
    icetray.set_log_level(log_levels[log_level.lower()])

#
# setup LeptonWeighter to do the magic
#

# read LIC and make a generator
simulation_generation   = LW.MakeGeneratorsFromLICFile(LIConfiguration)

# setup weighter for different interactions
if interaction in ["cc", "nc"]:
    # for CC/NC interaction reweighting
    xs           = LW.CrossSectionFromSpline(dsdxdy_nu_CC, dsdxdy_nubar_CC, dsdxdy_nu_NC, dsdxdy_nubar_NC)
    #weight_event = LW.Weighter(xs, simulation_generation)
    
    # flux parameters for reweighting
    flux_params = {'constant': 10**-18, 'index':-2.0, 'scale':10**5}
    flux = LW.PowerLawFlux(flux_params['constant'] , flux_params['index'] , flux_params['scale'] )  
    weight_event = LW.Weighter(flux, xs, simulation_generation)
elif interaction in ["gr"]:
    # cross section follows the soon-to-be-deprecated GRCS funtion in LW
    xs           = LW.GlashowResonanceCrossSection()
    weight_event = LW.Weighter(xs, simulation_generation)

def LW_calculate_weight(frame):
    LWevent = LW.Event()
    EventProperties                 = frame['EventProperties']
    
    #LeptonInjectorProperties        = frame['LeptonInjectorProperties']
    LWevent.primary_type            = LW.ParticleType(EventProperties.initialType)
    LWevent.final_state_particle_0  = LW.ParticleType(EventProperties.finalType1)
    LWevent.final_state_particle_1  = LW.ParticleType(EventProperties.finalType2)
    LWevent.zenith                  = EventProperties.zenith
    LWevent.azimuth                 = EventProperties.azimuth
    LWevent.energy                  = EventProperties.totalEnergy
    LWevent.interaction_x           = EventProperties.finalStateX
    LWevent.interaction_y           = EventProperties.finalStateY
    
    if type(EventProperties)==LeptonInjector.VolumeEventProperties:
        LWevent.total_column_depth  = EventProperties.totalColumnDepth
        LWevent.radius              = EventProperties.radius
        print(LWevent.total_column_depth)
    else:
        LWevent.total_column_depth  = EventProperties.totalColumnDepth
        LWevent.radius              = EventProperties.impactParameter
    
    # get MCPrimary
    if "MCPrimary1" in frame:
        MCPrimary                   = frame["MCPrimary1"]
    else:
        MCPrimary                   = frame["I3MCTree"].get_primaries()[0]
        
    LWevent.x                       = MCPrimary.pos.x
    LWevent.y                       = MCPrimary.pos.y
    LWevent.z                       = MCPrimary.pos.z
    
    # calculate OneWeight and simple Powerlaw weight
    weight_OneWeight                = weight_event.get_oneweight(LWevent)
    weight_Powerlaw                 = weight_event(LWevent)
    
    # write weight to frame
    LeptonWeigtherDict = dataclasses.I3MapStringDouble()
    LeptonWeigtherDict["FluxLessWeight"] = weight_OneWeight
    LeptonWeigtherDict["PowerlawWeight"]  = weight_Powerlaw
    frame["LeptonWeigtherDict"] = LeptonWeigtherDict
    
    # delete LWevent to be sure its gone
    del(LWevent)

#
# initialize and run I3Tray
#

print("Constructing I3Tray")

tray = I3Tray()

tray.Add(dataio.I3Reader, FilenameList=infiles)

tray.AddModule(LW_calculate_weight, "LeptonWeighter", Streams=[icetray.I3Frame.DAQ])

tray.AddModule("I3Writer", 'i3writer', Filename=outfile)

print("Executing Tray")

tray.Execute()
tray.Finish()

#
# say something about the runtime
#

end_time = time.time()
print("Done!")
print("That took "+str(end_time - start_time)+" seconds.")
