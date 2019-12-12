from os import uname
import argparse, os
import numpy as np
print(uname())

desc="This is an example script for using StartingTrackVeto"
parser = argparse.ArgumentParser(description = desc)
parser.add_argument("-i", "--infiles", dest = "infiles",
                    type = str, default = [], nargs = "+",
                    help = "[i]nfiles with frames")
parser.add_argument("-o", "--outfilebase", dest = "outfilebase",
                    type = str, default = "./test",
                    help = "base name for [o]utfiles")
parser.add_argument("-p", "--pulsesname", dest = "pulsesname",
                    type = str, default = "SplitInIcePulses",
                    help = "name of [p]ulse sereies to use")
parser.add_argument("-f", "--fit", dest = "fit",
                    type = str, default = "SPEFit2",
                    help = "name of I3Particle [f]it to feed to STV")
args = parser.parse_args()

infiles     = args.infiles
outfilebase = args.outfilebase
pulsesname  = args.pulsesname
fit         = args.fit

from icecube import dataio, dataclasses, icetray, StartingTrackVeto
from icecube import photonics_service, phys_services
from I3Tray import *

###--------Loading PhotoSoline Services--------###

def return_photonics_service(service_type="inf_muon"):
    '''
    Checks various locations for Spline Tables then loads 
    and returns a I3PhotoSplineService.

    Inputs:
    -------
      - service_type: type of photonics service to load.
          choices: "inf_muon", "seg_muon", "cscd"

    Outputs:
    --------
      - I3PhotoSplineService of requested type for use
          in the StartinTrackVeto calculations
    '''
    #Find location of spline tables
    table_base=""
    if os.path.isfile(os.path.expandvars("$I3_DATA/photon-tables/splines/ems_mie_z20_a10.%s.fits") % "abs"):
       table_base = os.path.expandvars("$I3_DATA/photon-tables/splines/ems_mie_z20_a10.%s.fits")
    elif os.path.isfile("splines/ems_mie_z20_a10.%s.fits" % "abs"):
       table_base = os.path.expandvars("splines/ems_mie_z20_a10.%s.fits")
    elif os.path.isfile("/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.%s.fits" % "abs"):
        table_base = os.path.expandvars("/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.%s.fits")
    elif os.path.isfile("/home/icecube/i3/data/generalized_starting_events/splines/ems_mie_z20_a10.%s.fits" % "abs"):
        table_base = os.path.expandvars("/home/icecube/i3/data/generalized_starting_events/splines/ems_mie_z20_a10.%s.fits")
    else:
        print("You don't have splines anywhere I can find. This will eventually raise an error, for now it semi-silently dies")

    #Load photospline service from location found above
    if service_type=="cscd":
        cascade_service = photonics_service.I3PhotoSplineService(table_base % "abs", 
                                                                 table_base % "prob", 
                                                                 0,
                                                                 maxRadius = 600.0)
        return cascade_service
    elif service_type=="seg_muon":
        seg_muon_service = photonics_service.I3PhotoSplineService(
                           amplitudetable = os.path.join( os.path.expandvars("$I3_DATA/photon-tables/splines/") ,"ZeroLengthMieMuons_250_z20_a10.abs.fits"),  ## Amplitude tables
                           timingtable = os.path.join( os.path.expandvars("$I3_DATA/photon-tables/splines/") ,"ZeroLengthMieMuons_250_z20_a10.prob.fits"),    ## Timing tables
                           timingSigma  = 0.0,
                           maxRadius    = 600.0)
        return seg_muon_service
    elif service_type=="inf_muon":
        inf_muon_service = photonics_service.I3PhotoSplineService(
                           amplitudetable = os.path.join( os.path.expandvars("$I3_DATA/photon-tables/splines/") ,"InfBareMu_mie_abs_z20a10.fits"),  ## Amplitude tables
                           timingtable = os.path.join( os.path.expandvars("$I3_DATA/photon-tables/splines/") ,"InfBareMu_mie_prob_z20a10.fits"),    ## Timing tables
                           timingSigma  = 0.0,
                           maxRadius    = 600.0) 
        return inf_muon_service
    else:
        print("You didn't give me a spline service type I recognize. This will eventually raise an error, for now it semi-silently dies")


#Loading an infinite muon service and segmented muon service for methods below
inf_muon_service = return_photonics_service(service_type = "inf_muon")
seg_muon_service = return_photonics_service(service_type = "seg_muon")

###--------End of PhotoSpline Service Loading--------###

###--------Icetray Utility Methods--------###
def pulli3omgeo(frame):
    #Pull I3omgeo from geometry frame
    global i3omgeo
    i3omgeo = frame["I3Geometry"].omgeo

def pullbadDOMList(frame):
    #Pull bad DOM lists from detector status frame
    global BadOMs
    print(frame["BadDomsList"], frame["BadDomsListSLC"])
    BadOMs = frame["BadDomsList"]
    BadOMs.extend(frame["BadDomsListSLC"])

def rmdictvalue(frame, keys):
    #Delete key from frame if key already exists in frame
    for key in keys:
        if frame.Has(key):
            del frame[key]

###--------End of I3 Utilities--------###

###--------STV Functions--------###
def make_n_segment_vector(frame, fit, n=1):
    '''
    Splits fit up into segments which is required for 
    StartingTrackVeto. If n=1, one infinite track segment
    is saved to the frame. If n>1, finite track segements
    are saved to the frame.

    Inputs:
    -------
      -frame: I3Frame
      -fit: I3Particle fit to be segemented and used 
         in StartingTackVeto
      -n: number of segments to split fit up into
         (must be odd!)   

    Outputs:
    --------
      -Save segments to frame as an I3Vector of I3Particles
    '''
    #Check that n is odd and fit exists in frame
    if n%2 == 0:
        print("n =",n,"is even! change this!")
        sys.exit(910)
    try:
        basep = frame[fit]
    except:
        print("I can't find the fit you want to segment")
        return False

    #Shift to closest approach position (cap) to the origin (0,0,0)
    origin_cap = phys_services.I3Calculator.closest_approach_position(basep, dataclasses.I3Position(0,0,0))
    basep_shift_d   = np.sign(origin_cap.z - basep.pos.z) *\
                      np.sign(basep.dir.z) *\
                      (origin_cap - basep.pos).magnitude
    basep_shift_pos = basep.pos + (basep.dir*basep_shift_d)
    basep_shift_t   = basep_shift_d/basep.speed

    basep.pos  = basep_shift_pos
    basep.time = basep.time + basep_shift_t
    
    #Create segments
    segments = []
    segment_length = 1950./n
    for idx in range(n):
        dshift = segment_length * (idx - ((n-1)/2.))
        particle = dataclasses.I3Particle()
        particle.time   = basep.time + (dshift / basep.speed)
        particle.pos    = basep.pos + (basep.dir * dshift)
        particle.dir    = basep.dir
        particle.energy = 0.01

        if n==1:
            particle.shape  = particle.shape.InfiniteTrack
            particle.length = 0
        else:
            particle.shape  = particle.shape.ContainedTrack
            particle.length = segment_length

        segments.append(particle)

    #Save segments in frame
    segments_str = fit + '_' + str(n) + '_segments'
    rmdictvalue(frame, [segments_str])
    frame[segments_str] = dataclasses.I3VectorI3Particle(segments)
    print("Put", segments_str, "in the frame")
    del segments

def runSTV(tray, name, n=1, fits_to_try = [], phot_serv = None, dtype = "cherdat", cad_d = 125, pulsesname = ""):
    for fn in fits_to_try:
        tray.Add(make_n_segment_vector, "make_n_segment_vector_"+fn+"_"+str(n),
                 fit = fn,
                 n   = n)
        tray.Add("StartingTrackVeto", "STV_"+fn+"_"+str(n),
                 Pulses                    = pulsesname,
                 Photonics_Service         = phot_serv,
                 Miss_Prob_Thresh          = 1,
                 Fit                       = fn,
                 Particle_Segments         = fn + '_' + str(n) + '_segments',
                 Distance_Along_Track_Type = dtype,
                 Supress_Stochastics       = True,
                 Min_CAD_Dist              = cad_d)


###--------End of STV Methods--------###

###--------IceTray--------###
tray = I3Tray()
tray.Add("I3Reader", "reader",
         FilenameList = infiles)

tray.Add(pulli3omgeo, "soitonlyhappensonce",
         Streams = [icetray.I3Frame.Geometry])
tray.Add(pullbadDOMList, "pullbadDOMList",
         Streams = [icetray.I3Frame.DetectorStatus])

tray.AddSegment(runSTV, "runSTV_nseg_1",
                fits_to_try = [fit],
                phot_serv   = inf_muon_service,
                pulsesname  = pulsesname)
'''
Option to run STV with a segmented track        
tray.AddSegment(runSTV, "runSTV_nseg_1",
                fits_to_try = [fit],
                n           = 131,
                phot_serv   = seg_muon_service,
                pulsesname  = pulsesname)
'''
tray.Add("I3Writer", "writer",
         Filename          = outfilebase + ".i3.bz2",
         Streams           = [icetray.I3Frame.DAQ,
                              icetray.I3Frame.Physics],
         DropOrphanStreams = [icetray.I3Frame.Geometry,
                              icetray.I3Frame.Calibration,
                              icetray.I3Frame.DetectorStatus,
                              icetray.I3Frame.DAQ])

tray.Execute()
###--------End of IceTray--------###
