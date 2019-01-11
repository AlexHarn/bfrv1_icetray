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

## Set logging
## Uncomment me for gooey details!
#icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
#icetray.set_log_level(icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('Laputop', icetray.I3LogLevel.LOG_DEBUG)

### Input and Output
infile = expandvars("$I3_TESTDATA") + "/icetop/Level2a_IC79_data_Run00116080_NSta10_17events.i3.gz"
outfile = "laputop_testscript_output.i3.gz"

tray = I3Tray()

########## SERVICES FOR GULLIVER ##########

datareadoutName = "CleanedHLCTankPulses"
excludedName = "ClusterCleaningExcludedStations"
excludedTanksName = "ClusterCleaningExcludedTanks"  #<-- created on the fly!

## The "simple lambda" snowservice
tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleSnow21")(
    ("Lambda", 2.1)
    )

## This one is the standard one.
tray.AddService("I3GulliverMinuitFactory","Minuit")(
    ("MinuitPrintLevel",-2),  
    ("FlatnessCheck",True),  
    ("Algorithm","SIMPLEX"),  
    ("MaxIterations",1000),
    ("MinuitStrategy",2),
    ("Tolerance",0.01),    
    )

tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
    ("InCore", "ShowerCOG"),
    ("InPlane", "ShowerPlane"),
    #("SnowCorrectionFactor", 1.5),   # Now obsolete
    ("Beta",2.6),                    # first guess for Beta
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
        eventheader = frame['I3EventHeader']
        track = frame['Laputop']
        params = frame['LaputopParams']
        #print eventheader.run_id
        #print eventheader.event_id
        #print track.pos.x
        #print track.pos.y
        #print track.pos.z
        #print track.dir.zenith
        #print track.dir.azimuth
        #print track.time
        #print params.s125
        #print params.beta
        #print params.ndf
        #print params.nmini
        #print params.llh
        #print params.chi2_ldf
        #print params.chi2_time
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
        self.runnumber = [ 116080, 116080, 116080, 116080, 116080, 116080, 116080,
			  116080, 116080, 116080, 116080, 116080, 116080, 116080,
			  116080, 116080, 116080 ]

        self.eventnumber = [ 355, 2433, 3916, 4682, 5052, 6462, 6900, 6989, 8003, 8820, 
			    9197, 9333, 11550, 11567, 11933, 11999, 12206 ]

        self.x = [ 9.11610316, -8.34095910401, -478.869275716, -21.5824324506, -153.212254361, 
                   414.533766779, 113.216200029, 429.377612485, 61.0216242353, 6.33981386544, 
                   -417.683754552, -79.7613240393, -145.229169511, -105.542348643, -349.135269414, 
                   27.8981277172, 215.086198092 ]
        
        
        self.y = [ -354.507611321, -53.215149742, -68.6473525442, -284.763406204, 208.211139034, 
                    -166.387209737, -339.317574059, -146.925188677, -399.475975698, -188.728924362, 
                    76.4351531101, 382.820121951, 121.605138152, 400.81562063, -343.858862757, 
                    -91.7044233612, -268.102426409]
        
        self.z = [ 1945.04588614, 1946.19168882, 1945.1032977, 1944.92586643, 1945.92947767, 1946.86988832, 
                   1945.22500472, 1946.85082157, 1945.19851042, 1944.69547197, 1945.27661291, 1946.68763615, 
                   1945.55454758, 1946.65226975, 1945.08727868, 1945.82888424, 1945.20279372]
        
        self.zen = [ 0.777142001474, 0.669230265248, 0.422297911091, 0.501670686132, 0.754144189049, 
                     0.795803717295, 0.385096627003, 0.40152149311, 0.358294710068, 0.585478638218, 
                     0.417442254743, 0.878706056293, 0.368829333914, 0.425234704767, 0.629578318204, 
                     0.534565574731, 0.689677408254 ]
        
        self.az = [ 3.11800732818, 1.92773798117, 5.44669258973, 5.4285474158, 2.68658566913, 
                    2.16140252267, 3.84788925546, 4.08531348411, 4.04763069064, 1.51438314022, 
                    1.68152832978, 0.0931070813664, 3.8420552506, 4.93880432701, 2.90837432297, 
                    0.718207697073, 4.05377497807 ]

        self.time = [ 27092.4671776, 10096.0808103, 10357.6813885, 10686.9948159, 10248.743208, 
                      10304.6061609, 10044.3588728, 10167.4140575, 9990.80503686, 10106.9363527, 19480.3986828, 10563.2503222,
                      10140.054843, 10070.662245, 10008.142569, 10105.5588441, 10208.0337974 ]

        self.s125 = [ 0.795419603389, 1.16015719144, 3.34816510433, 2.80425285326, 0.966855843083, 0.478348741813,
                      1.19294658431, 1.9558244078, 1.38126096846, 0.253913652437, 2.65809645262, 1.10260053561,
                      1.01366014212, 0.648260602353, 1.49507427128, 0.740623511384, 0.507334784144 ]

        self.beta = [ 2.69099882585, 2.71980549605, 2.3322305554, 2.98000577635, 2.68077782669, 2.52851765555, 
                      3.40354311154, 2.93316991122, 3.68541147579, 3.22465765317, 2.58823963744, 3.03022865604, 
                      2.89614586163, 3.30685194552, 2.34934022527, 2.77700532947, 3.00180736272 ]

        self.ndf = [ 36, 40, 40, 60, 36, 16, 28, 36, 24,
                     16, 48, 36, 28, 16, 28, 32, 24 ]

        self.nmini = [ 82, 92, 63, 45, 95, 44, 46, 58, 40, 
                       42, 60, 35, 44, 40, 236, 41, 40 ]

        self.llh = [ -33.1697427845, -63.4817996664, -65.5744629819, -79.0305065826, -46.7616335053, -30.5505858765,
                      -41.8658968713, -38.9558917961, -30.1614234387, -10.0110688952, -78.4019471972, -48.7074923963, 
                      -48.5546699865, -22.0557877596, -70.148860726, -39.2118190567, -19.0407067201 ]

        self.chi2 = [ 1.08782029392 , 2.45038495089, 1.85962788273, 1.12120059288, 1.09605788248, 1.06501899153, 
                      0.897989567145, 0.826033366012, 0.309130686632, 0.570722245064, 1.01942269646, 1.87065284316,
                      2.55653375513, 1.38920892779, 4.88285998341, 0.984413701337, 0.559384260804 ]

        self.chi2_t = [ 0.201617804539, 0.519080851621, 0.562369521164, 0.347574747666, 1.07433292787, 3.65559129068,
                        2.39655622322, 0.713441115688, 2.94964605392, 0.545259890676, 2.0447504143, 0.771223741709, 
                        0.455828586229, 1.57454578956, 0.361413907488, 1.93876284795, 0.301501419585 ]


    def Physics(self, frame): 
        header = frame['I3EventHeader']
        outParticle = frame['Laputop']
        outParams = frame['LaputopParams']

        # Check the numbers!
        print("Checking event "+str(self.count_))
        ensure_equal(header.run_id, self.runnumber[self.count_], "Run numbers do not match.")
        ensure_equal(header.event_id, self.eventnumber[self.count_], "Event numbers do not match.")
        ensure_distance(outParticle.pos.x, self.x[self.count_], 0.00001, "X does not match.")
        ensure_distance(outParticle.pos.y, self.y[self.count_], 0.00001, "Y does not match.")
        ensure_distance(outParticle.pos.z, self.z[self.count_], 0.00001, "Z does not match.")
        ensure_distance(outParticle.dir.zenith, self.zen[self.count_], 0.00001, "Zenith does not match.")
        ensure_distance(outParticle.dir.azimuth, self.az[self.count_], 0.00001, "Azimuth does not match.")
        ensure_distance(outParticle.time, self.time[self.count_], 0.001, "Time does not match.")

        # things in the params
        ensure_distance(pow(10,outParams.value(recclasses.LaputopParameter.Log10_S125)), self.s125[self.count_], 0.000001, "S125 does not match.")
        ensure_distance(outParams.value(recclasses.LaputopParameter.Beta), self.beta[self.count_], 0.00001, "Beta does not match.")
        ensure_equal(outParams.ndf, self.ndf[self.count_], "NDF's do not match.")
        ensure_equal(outParams.n_mini, self.nmini[self.count_], "Nminimizations do not match.")
        ensure_distance(outParams.log_likelihood, self.llh[self.count_], 0.00001, "LLH does not match.")
        ensure_distance(outParams.chi2_ldf, self.chi2[self.count_], 0.00001, "Chi2_ldf does not match.")
        ensure_distance(outParams.chi2_time, self.chi2_t[self.count_], 0.00001, "Chi2_time does not match.")
        
        # Increment the counter
        self.count_ = self.count_+1

        self.PushFrame(frame)                     # push the frame 
        return

## Now run the test!
tray.AddModule(QuickTester,"testme")





 
   
# Execute the Tray
tray.Execute()

