#!/usr/bin/env python

import os, math
import sys,getopt
from os.path import expandvars

from I3Tray import *
from icecube import icetray, dataclasses, dataio, toprec, recclasses
from icecube.icetray import I3Module
from icecube.dataclasses import I3EventHeader, I3Particle
from icecube.recclasses import I3LaputopParams

load("libgulliver")
load("liblilliput")

## -------------------------------------
## This is a test-script for testing code, NOT a collection of standardized parameters!
## It contains options NOT USED in "standard" scripts, just to make sure that they work.
## DO NOT USE FOR ANALYSIS!

## This one is similar to its neighbor, run_laputop.py,
## except that is uses MIGRAD instead of SIMPLEX, so has to harvest interesting numbers for errors
## and make sure that they end up where they're supposed to

### Input and Output
infile = expandvars("$I3_TESTDATA") + "/icetop/Level2a_IC79_data_Run00116080_NSta10_17events.i3.gz"
outfile = "laputop_testscript_output2.i3.gz"

tray = I3Tray()

########## SERVICES FOR GULLIVER ##########

datareadoutName = "CleanedHLCTankPulses"
excludedName = "ClusterCleaningExcludedStations"
excludedTanksName = "ClusterCleaningExcludedTanks"   # <-- computed on the fly!

## The "simple lambda" snowservice
tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleSnow21")(
    ("Lambda", 2.1)
    )

## This one is the standard one.
tray.AddService("I3GulliverMinuitFactory","Minuit")(
    ("MinuitPrintLevel",-2),  
    ("FlatnessCheck",True),  
    ("Algorithm","MIGRAD"),  
    ("MaxIterations",1000),
    ("MinuitStrategy",2),
    ("Tolerance",0.01),    
    )

tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
    ("InCore", "ShowerCOG"),
    ("InPlane", "ShowerPlane"),
    #("SnowCorrectionFactor", 1.5),   # Now obsolete
    ("Beta",2.9),                    # first guess for Beta
    ("InputPulses",datareadoutName)  # this'll let it first-guess at S125 automatically
)

fixcore = False   #always

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam2")(
    ("FixCore", fixcore),        
    ("FixTrackDir", True),
    ("IsBeta", True),
    ("MinBeta", 2.9),   ## From toprec... 2nd iteration (DLP, using beta)
    ("MaxBeta", 3.1),
    ("CoreXYLimits", 1000.0)
    )

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam3")(
    ("FixCore", fixcore),        
    ("FixTrackDir", False),      # FREE THE DIRECTION!
    ("IsBeta", True),
    ("MinBeta", 2.0),   ## From toprec... 3rd iteration (DLP, using beta)
    ("MaxBeta", 4.0),
    ("LimitCoreBoxSize", 15.0),
    ## Use these smaller stepsizes instead of the defaults:
    ("VertexStepsize",5.0),      # default is 20
    ("SStepsize", 0.045),        # default is 1
    ("BetaStepsize",0.15)        # default is 0.6    
    )

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam4")(
    ("FixCore", fixcore),        
    ("FixTrackDir", True),
    ("IsBeta", True),
    ("MinBeta", 1.5),   ## From toprec... 4th iteration (DLP, using beta)
    ("MaxBeta", 5.0),
    ("LimitCoreBoxSize", 15.0),
    ("MaxLogS125", 8.0),
    ## Use these smaller stepsizes instead of the defaults:
    ("VertexStepsize", 4.0),     # default is 20
    ("SStepsize", 0.045),        # default is 1
    ("BetaStepsize",0.15)        # default is 0.6 
    )

tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike2")(
    ("datareadout", datareadoutName),
    ("badtanks", excludedTanksName),
    ("dynamiccoretreatment",11.0),     # do the 11-meter core cut
    ("curvature",""),      # NO timing likelihood
    ("SnowServiceName","SimpleSnow21")
    )


#------------------- LET'S RUN THE MODULES!  ------------------
#**************************************************
#                  Read file
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileName", infile)
    )

#**************************************************
#    Adapt (old) BadStation list -> (new) BadTank list
#    We need this because this is an old testscript, from
#    before "BadStations" went obsolete
#**************************************************
class createBadTankList(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter('InputBadStationsName','Old BadStations name',0)
        self.AddParameter('OutputBadTanksName','New BadTanks name',0)
        self.AddOutBox("OutBox")
        
    def Configure(self):
        self.oldName = self.GetParameter('InputBadStationsName')
        self.newName = self.GetParameter('OutputBadTanksName')

    def Geometry(self, frame):
        if 'I3Geometry' in frame:
            geo = frame['I3Geometry']
            self.omg = geo.omgeo
        else:
            print 'No geometry found'
        self.PushFrame(frame,"OutBox")
    
    def Physics(self, frame):
        if self.oldName in frame:
            oldlist = frame[self.oldName]
            geo = frame["I3Geometry"]
            newlist = dataclasses.TankKey.I3VectorTankKey()   # startin' off empty!

            for station in oldlist:
                ## put two entries (one for each tank) in the BadTanks list
                newlist.append(dataclasses.TankKey(station, dataclasses.TankKey.TankA))
                newlist.append(dataclasses.TankKey(station, dataclasses.TankKey.TankB))

        frame[self.newName] = newlist
        self.PushFrame(frame,"OutBox")


tray.AddModule(createBadTankList,"convert_badlist")(
    ("InputBadStationsName", excludedName),
    ("OutputBadTanksName", excludedTanksName)
)


#**************************************************
#                  The Laputop Fitter
#**************************************************
tray.AddModule("I3LaputopFitter","Laputop")(
    ("SeedService","ToprecSeed"),
    ("NSteps",3),                    # <--- tells it how many services to look for and perform
    ("Parametrization1","ToprecParam2"),   # the three parametrizations
    ("Parametrization2","ToprecParam3"),
    ("Parametrization3","ToprecParam4"),
    ("StoragePolicy","OnlyBestFit"),
    ("Minimizer","Minuit"),
    ("LogLikelihoodService","ToprecLike2"),     # the three likelihoods
    ("LDFFunctions",["dlp","dlp","dlp"]),
    ("CurvFunctions",["","gausspar","gausspar"])   # VERY IMPORTANT : use time Llh for step 3, but fix direction!
    )


#tray.AddModule("Dump","dump")()

## Don't need to write an output here.
## Comment me back in, if you want to debug something:
#tray.AddModule("I3Writer","EventWriter")(
#    ("DropOrphanStreams", [icetray.I3Frame.DAQ]),
#    ("Filename", outfile),
#    )
 

## ----------------- THE DIAGNOSTIC PART OF THE SCRIPT! ------------

## This dumps variables to the screen.
## Uncomment this if you're booking something new and want to update this test
## to "harvest" the numbers from these 17 events.
'''
class QuickSpitter(icetray.I3Module): 
    def __init__(self, context):                       ## Constructor 
        icetray.I3Module.__init__(self, context) 
        self.AddOutBox('OutBox')

    def Configure(self):
        # ain't got nothin'
        #        self.outputname = self.GetParameter('OutputName') 
        self.fakenumber = 6

    def Physics(self, frame): 
        params = frame['LaputopParams']
        print params.xc_err
        #print params.yc_err
        #print params.log10_s125_err
        #print params.beta_err
        #print params.nx_err
        #print params.ny_err
        #print params.tc_err
        self.PushFrame(frame)                     # push the frame 
        return

tray.AddModule(QuickSpitter,"prepareme")
'''

## ----------------- TEST THE OUTPUT OF THE FIT ----------------------
def ensure_equal(a, b, message="Test FAILED"):
    if a != b:
        raise Exception(message + " " + str(a) + " " + str(b))

def ensure_distance(a, b, eps, message="Test FAILED"):
    if math.fabs(a-b) > eps:
        raise Exception(message + " " + str(a) + " " + str(b))


class QuickTester(icetray.I3Module): 
    def __init__(self, context):                       ## Constructor 
        icetray.I3Module.__init__(self, context) 
        self.AddOutBox('OutBox')

    def Configure(self):
        self.count_ = 0

        ## Here's what the numbers "should" be:
        ## Today, we'll test just the errors that get put into the Params

        ## Note: the 16th event failed to converge, and so it fills these fields with NAN

        ## Also note: these numbers were harvested on Kath's Mac (OS Mountain Lion), and 
        ## the numbers are slightly different when run on other OS's, for instance
        ##                                   (RHEL 6)        (Mac)
        ## Exception: Xerr does not match. 1.19008589277 1.19007124466
        ## So we'll set the tolerances a little lower here than in the other script...


        self.xerr = [ 1.28192190901, 0.498747361329, 1.07518929681, 0.757236160344, 1.19007124466, 
                      2.38101274005, 0.47567115733, 1.13782390503, 0.598806109777, 0.862575623633, 0.803942643575, 0.744888138514,
                      0.610335040907, 0.873299117983, 1.19631598055, float('NaN'), 1.04136184892 ]
        
        self.yerr = [ 0.992496552467, 0.846835409726, 1.1656065783, 0.883602980451, 0.482572249571, 1.12890415998,
                      0.713804457353, 0.908067636868, 0.542553916371, 0.736788964837, 1.07078529457, 0.847458319481, 
                      0.734918117479, 1.006814414, 1.02903538215, float('NaN'), 0.961149892443 ]
        

        self.logs125err = [ 0.00804464992663, 0.00646284346195, 0.0136996737091, 0.00713963582915, 0.0479100800857,
                            0.0160114913817, 0.00828457708939, 0.00880882594148, 0.00904436990671, 0.016327767823, 0.00549296443241,
                            0.00783679171425, 0.00821045690823, 0.0126904247156, 0.0140017306584, float('NaN'), 0.00987498234322 ]
        
        self.betaerr = [ 0.0291600801284, 0.0230591596164, 0.0296033708923, 0.031162196042, 0.0686821421402, 
                         0.0475445668459, 0.0356604367506, 0.0330244027195, 0.0362615606848, 0.060097726176, 0.0215481065378, 
                         0.0303819368169, 0.0345167677604, 0.0650051140465, 0.0411471201812, float('NaN'), 0.0597359348343 ]

    def Physics(self, frame): 
        outParams = frame['LaputopParams']

        # Check the numbers!
        print("Checking event "+str(self.count_))

        # all errors are stored in the params:
        ensure_distance(outParams.error(recclasses.LaputopParameter.Xc), self.xerr[self.count_], 0.0001, "Xerr does not match.")
        ensure_distance(outParams.error(recclasses.LaputopParameter.Yc), self.yerr[self.count_], 0.0001, "Yerr does not match.")
        ensure_distance(outParams.error(recclasses.LaputopParameter.Log10_S125), self.logs125err[self.count_], 0.00001, "logS125err does not match.")
        ensure_distance(outParams.error(recclasses.LaputopParameter.Beta), self.betaerr[self.count_], 0.0001, "Betaerr does not match.")


        # Increment the counter
        self.count_ = self.count_+1

        self.PushFrame(frame)                     # push the frame 
        return

## Now run the test!
tray.AddModule(QuickTester,"testme")





 
   
# Execute the Tray
tray.Execute()

