#This script is thought to submit WimpSim jobs to Condor

import os
#from random import randint, seed
from condor_submit import CondorCreate, DAGSubmit, CondorMakeDirs, DAGMakeFamilyDirs

#---Env-settings------------------------------
CVMFS='eval `/cvmfs/icecube.opensciencegrid.org/py2-v3.0.1/setup.sh`'
ENVSH='/data/user/grenzi/combo/build/env-shell.sh'

#---I/O-settings------------------------------
LOGDIR=''#'/scratch/grenzi'
OUTPUTDIR='/data/user/grenzi/data/EarthWIMP/Data/'

#---WimpSim-settings--------------------------
WASCRIPT='/home/grenzi/wimpsim_launcher/driver_wimpann.py'
WESUNSCRIPT='/home/grenzi/wimpsim_launcher/driver_wimpevent_sun.py'
WEEARTHSCRIPT='/home/grenzi/wimpsim_launcher/driver_wimpevent_earth.py'
WORKPATH='/data/user/grenzi/wimpsim/wimpsim-4.01'
RUN = 10060 
MJDSTART = 57530.87
MJDSTOP = MJDSTART+365.25
SEED = -1
NEV = 1e6
RUNWHOLECHAIN = True #True when running the whole WimpSim chain, False if want to run the parts singulalrly
RUNTEST = True #False for normal usage, True if want to test. In case of True it tests one channel with a low number of events

#I want this (channel,mass,files)
RUN_THESE = [
#channel 11 (tautau)
(11,10,100),
(11,20,100),
(11,35,100),
(11,50,100),
#channel 5 (bb)
(5,35,200),
(5,50,200),
(5,100,200),
(5,250,100),
(5,500,100),
(5,1000,50),
(5,3000,20),
(5,5000,20),
#channel 8 (WW)
(8,100,50),
(8,250,20),
(8,500,20),
(8,1000,20),
(8,3000,20),
(8,5000,20),
#channel 100 (LKP)
(100,250,20),
(100,500,20),
(100,750,20),
(100,1000,20),
(100,1250,20),
(100,1500,20),
(100,3000,20)
]

#-Test options
if RUNTEST:
    print ('!!!Running a test!!!')
    NEV = 1e2
    RUN_THESE = [(11,50,1)]
    
    OUTPUTDIR += 'Test/'

#seed()
#pid = os.getpid()

#-Create necessary directories, if they don't exist------------
CondorMakeDirs()

for opt in RUN_THESE:
    print opt
    
    submit = True #when jobs have to be submitted singularly. If dagman, it sets submit = False
    
    if RUNWHOLECHAIN:
        #-Create "family" directories for DAGMan application-------
        famnum = 1e5
        famnum = DAGMakeFamilyDirs(famnum)
        
        submit = False #see above
    
    #-Launch wimpann through express script--------------------
    wacommand = ((CVMFS+' '+ENVSH+' python  '+WASCRIPT+' -f %d -m %d -c %d -r %s -s %d -w '+WORKPATH+\
                ' -o '+OUTPUTDIR+' -n %.f') % (opt[2], opt[1], opt[0], RUN, SEED, NEV))
    wajob = CondorCreate(wacommand, FAMNUM=famnum, submit=submit)
    
    #-Launch wimpevent-----------------------------------------
    wesjob=[]
    if opt[2]==1:
        #If file is not splitted, i.e. files value in opt is 1, it misses the last suffix
        wescommand = ((CVMFS+' '+ENVSH+' python  '+WESUNSCRIPT+' -m %d -c %d --mjdstart %d --mjdstop %d -r %d -w '+WORKPATH+\
                     ' -o '+OUTPUTDIR) % (opt[1], opt[0], MJDSTART, MJDSTOP, RUN))
        #pid+=1
        wesjob.append(CondorCreate(wescommand, FAMNUM=famnum, submit=submit))
    else:
        for i in range(1,opt[2]+1):
            wescommand = ((CVMFS+' '+ENVSH+' python  '+WESUNSCRIPT+' -f %d -m %d -c %d --mjdstart %d --mjdstop %d -r %d -w '+WORKPATH+\
                         ' -o '+OUTPUTDIR) % (i, opt[1], opt[0], MJDSTART, MJDSTOP, RUN))
            #pid+=1
            wesjob.append(CondorCreate(wescommand, FAMNUM=famnum, submit=submit))
            
    
    weejob=[]
    if opt[2]==1:
        #If file is not splitted, i.e. files value in opt is 1, it doesn't want the last suffix, i.e. option -f has not to be used
        weecommand = ((CVMFS+' '+ENVSH+' python  '+WEEARTHSCRIPT+' -m %d -c %d -r %d -w '+WORKPATH+\
                     ' -o '+OUTPUTDIR) % (opt[1], opt[0], RUN))
        #pid+=1
        weejob.append(CondorCreate(weecommand, FAMNUM=famnum, submit=submit))
    else:
        for i in range(1,opt[2]+1):
            weecommand = ((CVMFS+' '+ENVSH+' python  '+WEEARTHSCRIPT+' -f %d -m %d -c %d -r %d -w '\
                           +WORKPATH+' -o '+OUTPUTDIR) % (i, opt[1], opt[0], RUN))
            #pid+=1
            weejob.append(CondorCreate(weecommand, FAMNUM=famnum, submit=submit))
                      
    wejob = wesjob + weejob        

    if RUNWHOLECHAIN:
        DAGSubmit(famnum, [wajob], wejob) #when jobs are not singularly submitted
