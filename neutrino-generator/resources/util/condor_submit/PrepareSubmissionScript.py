#!/usr/bin/env python
#
# This script generates condor submission scripts to generate
# InEarth propagation tables using NuGen's "InEarth" simulation mode.
# The "InEarth" mode skips final interaction in NuGen.
#
# This propagation tables will be used with "FINALONLY" NuGen dataset
# that skips all InEarth propagation and perform final interaction only.
# 
# HOW TO USE
#
# 1) set variables.
# 2) run this script
#    python PrepareSubmissionScript.py
#    then three .sh files are generated.
# 3) login to submit node (NPX in UW), and set I3_BUILD(source env.sh)
# 4) run inearth.sh 
# 5) run combine_inearth.sh 
#  
# if you need to re-generate propagation tables from generated i3files,
# run run_maketable.sh.
#

# datasetid, you may define any integer 
datasetid = 100100100

# input neutrino flavor. If "NuMu" then both NuMu and NuMuBar are generated.
types = "NuE:NuEBar:NuMu:NuMuBar:NuTau:NuTauBar"
ratios = "1.0:1.0:1.0:1.0:1.0:1.0"

# generation gamma index. 1.0 is recommended to get 
# enough statistics for all energy - zenith range.
gengamma = "1.0"

# set output data dir.
datadir = ('/data/ana/EarthCore/sim/InEarth/prod_test/AllFlavors/E%s' % (gengamma))

# log10 energy range, separated with :
erange = "2:9"
erangetag = erange.replace(":","_")

# number of energy bins for table
nebins = int((9 - 2)/0.05)
#nebins = int((9 - 2)/1.0)

# zenith range in degree, separated with :
zenrange = "90:180"

# number of zenith bins and coszen bins
nzenbins = int((180 - 90))
ncoszenbins = 100

# Target generation index for "FINALONLY" NuGen simulation.
# The "InEarth" simulation is generated with E^-1, but 
# can be used to make propataion tables for "FINALONLY" dataset
# generated with E^-1 or a softer gamma index. 
# You may generate multiple tables for each gamma index (of FINALONLY dataset).
# This example generates three different tables for E^-1, E^-1.5, and E^-2.0.
weightgamma= "1.0:1.5:2.0"

# cross section model
xsecmodel = "csms_differential_v1.0"

# cross section global scale parameter
# do not change it unless you want to simulate modified cross section value
xsecscale = "1.0:1.0:1.0"

# EarthModel file name. 
earthmodel = "PREM_mmc.dat"

# number of generation files.
#nfiles = 100
nfiles = 10

# number of injected neutrinos per file
nevents = 1000000

# procno is used to set starting file number and random seed.
# if you want to add more files, set next file number.
procno = 0 

# do not touch following parameters.
mergefiles = 1


#---- end of setting part --------------------------------

# generate CONDOR submission scripts
f1 = open("run_inearth.sh", 'w')
f1.write("python submit_GenInEarth.py %d %d %s %d -T %s -R %s -g %s -e %s -z %s -G %s -x %s -E %s -n %d " %
(datasetid, nfiles, datadir, procno, types, ratios, gengamma, erange, zenrange, xsecscale, xsecmodel, earthmodel, nevents))

# generate script to make tables only
# you don't need it if you run 'run_inearth.sh" script.
# if you want to re-generated tables(saved in pickles file) from i3file, use run_maketable.sh.
f2 = open("run_maketable.sh", 'w')
f3 = open("run_combine_inearth.sh", 'w')

wgammalist = weightgamma.split(":")
for wgamma in wgammalist :
        #picklesdir = ('%s/%d/Z%d_C%d_E%d' % (datadir,datasetid, nzenbins, ncoszenbins, nebins))
        picklesdir = ('%s/%d/Z%d_E%d' % (datadir,datasetid, nzenbins, nebins))
        curpicklesdir = ('%s/W%s' % (picklesdir, wgamma))
        f2.write("python submit_MakeInEarthPropWeightTable.py %d %d %s %d %s %s %s %s/%d %s %d\n" % 
             (datasetid, nzenbins, zenrange, nebins, erange, gengamma, wgamma, datadir, datasetid, curpicklesdir,  mergefiles))

        # generate script to combine pickles.
        f3.write("python submit_combine_inearth.py %s %d_E%s_W%s_AllFlavors %d_*_AllFlavors.pickles\n" % 
             (curpicklesdir, datasetid, erangetag, wgamma, datasetid))



