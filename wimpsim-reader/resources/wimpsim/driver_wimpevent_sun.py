#!/usr/bin/env python
 
###
# A driver script to correctly compose the operations needed to process files. Conceived to run union with ClusterSubmitManager submit-scripts.
###
# For calling wimpsim-3.03/bin/wimpevent wit a sun file :
# --------------------------------------
# if ran manually:
# option 1: trust the script
#   call it specifying all options in the parser
# option 2: do not trust or modify
#   fix all member-variables of the classes wimpevent_sun_params,
#   fix all pathes in filelocation
#   take care with file operations at the bottom of the script

import sys, os, numpy, shutil
from os.path import expandvars
from optparse import OptionParser
from datetime import datetime

class wimpevent_sun_params:
  infile_sun = ''
  detector_lat = -90.
  detector_long = 0.
  simple_accu = 2
  start_time = ''
  stop_time = ''
  cc_nc_both = 3
  target_material = 1
  outfile_sun = ''
  equatorial_pos = 0
  summery_dir = 'none'
  seed = -1

class filelocation:
  workdir = ''
  indir = ''
  tmpdir = ''
  scratchdir = ''
  outdir = ''
  
parser = OptionParser()
usage = """%prog [options]"""
parser.set_usage(usage)
parser.add_option("-f", "--fileindex", action="store", type="int", default=0, dest="FILEINDEX", help="process this FILEINDEX")
parser.add_option("-m", "--mass", action="store", type="int", default=float(numpy.nan), dest="MASS", help="MASS of the WIMP")
parser.add_option("-c", "--channel", action="store", type="int", default=float(numpy.nan), dest="CHANNEL", help="CHANNEL of the WIMP")
parser.add_option("--mjdstart", action="store", type="float", default=float(numpy.nan), dest="MJDSTART", help="START mjd of the simulation")
parser.add_option("--mjdstop", action="store", type="float", default=float(numpy.nan), dest="MJDSTOP", help="STOP mjd of the simulation")
parser.add_option("-s", "--seed", action="store", type="int", default=0, dest="SEED", help="SEED for everything")
parser.add_option("-r", "--run", action="store", type="int", default=float(numpy.nan), dest="RUN", help="RUN to process files")
parser.add_option("-w", "--workpath", action="store", default='', dest="WORKPATH", help="Path to WimpSim")
parser.add_option("-o", "--output", action="store", default='', dest="OUTPUTDIR", help="Output files directory")
(options,args) = parser.parse_args()

### general purpose preparation
if options.FILEINDEX==0:
    infile_name = 'wa-m%d-ch%d-sun-%06d.dat' % (options.MASS, options.CHANNEL, options.RUN)
    outfile_name = 'we-m%d-ch%d-sun-%06d.dat' % (options.MASS, options.CHANNEL, options.RUN)
    final_name = 'we-m%d-ch%d-sun.%06d.dat.gz' % (options.MASS, options.CHANNEL, options.RUN)
else:
    infile_name = 'wa-m%d-ch%d-sun-%06d-%d.dat' % (options.MASS, options.CHANNEL, options.RUN, options.FILEINDEX)
    outfile_name = 'we-m%d-ch%d-sun-%06d-%d.dat' % (options.MASS, options.CHANNEL, options.RUN, options.FILEINDEX)
    final_name = 'we-m%d-ch%d-sun.%06d.%06d.dat.gz' % (options.MASS, options.CHANNEL, options.RUN, options.FILEINDEX)

### filelocation preparation
pathes = filelocation()
pathes.workdir = options.WORKPATH
pathes.indir = ''
pathes.tmpdir = '/data/user/grenzi/data/EarthWIMP/Data/tmp'
pathes.scratchdir = os.getenv('_CONDOR_SCRATCH_DIR')
pathes.outdir = (options.OUTPUTDIR+'/Sun') #'/data/user/grenzi/data/EarthWimpData/Sun'

### check parameter correctness                                                                                                                               
print(pathes.scratchdir)
if pathes.scratchdir == None:
  exit('_CONDOR_SCRATCH_DIR not set!')

### wimpann_parameter preparation
params = wimpevent_sun_params()
params.infile_sun = os.path.join(pathes.scratchdir, infile_name)
params.start_time = 'MJD '+str(options.MJDSTART)
params.stop_time = 'MJD '+str(options.MJDSTOP)
params.outfile_sun = os.path.join(pathes.scratchdir, outfile_name)
params.seed = options.SEED

#check access
if os.access(os.path.join(pathes.tmpdir, infile_name),os.R_OK) == False:
  exit('cannot access input file %s for reading!' % os.path.join(pathes.tmpdir, infile_name))
if os.access(pathes.scratchdir,os.W_OK) == False:
  exit('cannot access scratch directory %s for writing!' % pathes.scratchdir)
if os.access(pathes.tmpdir,os.W_OK) == False:
  exit('cannot access out directory %s for writing!' % pathes.outdir)

# Start collecting information about the node I'm running on.
print ("This is" +str(os.uname())+': '+str(datetime.now())) 
print ("Execution begins for WIMPEVENT: Channel %d Mass %d File %d" % (options.CHANNEL, options.MASS, options.FILEINDEX))

print ('Fetching '+infile_name)
shutil.copy(os.path.join(pathes.tmpdir, infile_name), pathes.scratchdir)
print ("Processing events")

cmdstring = '"%s\n%f\n%f\n%d\n%s\n%s\n%d\n%d\n%s\n%d\n%s\n%d"' % (params.infile_sun, params.detector_lat, params.detector_long, params.simple_accu, params.start_time, params.stop_time, params.cc_nc_both, params.target_material, params.outfile_sun, params.equatorial_pos, params.summery_dir, params.seed)

os.system('echo -e '+cmdstring+' | '+pathes.workdir+'/bin/wimpevent')

print("packing outfile")
os.system('gzip %s' % (params.outfile_sun))
print ("Moving final outfiles")
shutil.move(params.outfile_sun+'.gz', os.path.join(pathes.outdir, final_name))
print ("Delete files")
os.remove(params.infile_sun)
os.remove(os.path.join(pathes.tmpdir, infile_name))
print ("Finished")
