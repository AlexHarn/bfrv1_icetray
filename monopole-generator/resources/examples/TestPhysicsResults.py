#!/usr/bin/env python
#
#  Monopole Generator
#
#  $Id$
#

import I3Tray
from icecube import icetray, dataclasses, dataio, monopole_generator

# config
nevents = int(1e5);  #default: 1e5
mctreename="I3MCTree"
#betaRange=[0.001]
betaRange=[0.4,0.995]
powerLawIndex=5.
#powerLawIndex=float('nan')
mass=(1e11)*icetray.I3Units.GeV
disk_dist=1000*icetray.I3Units.m
disk_rad=850*icetray.I3Units.m
flux=1e-16

# tray
tray = I3Tray.I3Tray()

# Add the random generator service
tray.AddService("I3GSLRandomServiceFactory","random")

# Make some empty frames
tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)

tray.AddModule("I3MonopoleGenerator","generator")(
    ("TreeName", mctreename),
    ("Disk_dist", disk_dist),
    ("Disk_rad", disk_rad),
    ("Rand_pos", True),
    ("InfoName", "MPInfoDict"),
#   ("Gamma",10),
    ("BetaRange", betaRange),
    ("powerLawIndex", powerLawIndex),
    ("Mass", mass),
# Those parameters are optional. Here's some choices though
#   ("ZenithRange", [0*icetray.I3Units.degree,180*icetray.I3Units.degree]),
#   ("AzimuthRange", [0*icetray.I3Units.degree,360*icetray.I3Units.degree]),

#   ("Rad_on_disk",0*icetray.I3Units.m),
#   ("Azi_on_disk",0*icetray.I3Units.radian),
#   ("Length",1000*icetray.I3Units.m),
)


#tray.AddModule("I3Writer","writer")(
#    ("filename", "mp-gen.i3")
#    )

resultsdict={}
def getResultsFromMCTree(frame):
    if frame.Has(mctreename):
        tree=frame.Get(mctreename)
        particleType=dataclasses.I3Particle.ParticleType.Monopole # Monopole
        prim = dataclasses.I3MCTree.get_most_energetic(tree, particleType)
        if prim == None:
            print tree
            print "WARNING: Couldn't find monopole in tree!"
            return False
    else:
        raise RuntimeError("Fatal Error: No %s in Frame", mctreename)

    if frame.Has("MPInfoDict"):
        mp = frame.Get("MPInfoDict")
    else:
        raise RuntimeError("Something really went wrong")

    global resultsdict
    if not "Counter" in resultsdict:
        #print "Lege keys an"
        resultsdict["Counter"]=1
        resultsdict["zenith"]=[prim.dir.zenith]
        resultsdict["azimuth"]=[prim.dir.azimuth]
        resultsdict["energy"]=[prim.energy]
        resultsdict["fit_status"]=[prim.fit_status]
        resultsdict["length"]=[prim.length]
        resultsdict["location_type"]=[prim.location_type]
        resultsdict["shape"]=[prim.shape]
        resultsdict["speed"]=[prim.speed]
        resultsdict["time"]=[prim.time]
        resultsdict["type"]=[prim.type]
        resultsdict["weight"]=[mp["Weight"]]
        # dont know if checking makes sense for the following
        #resultsdict["radius"]=[prim.pos.r]
        #resultsdict["phi"]=[prim.pos.phi]
        #resultsdict["theta"]=[prim.pos.theta]
        #resultsdict["x"]=[prim.pos.x]
        #resultsdict["y"]=[prim.pos.y]
        #resultsdict["z"]=[prim.pos.z]
    else:
        #print "Benutze fertige keys"
        resultsdict["Counter"]+=1
        resultsdict["zenith"].append(prim.dir.zenith)
        resultsdict["azimuth"].append(prim.dir.azimuth)
        resultsdict["energy"].append(prim.energy)
        resultsdict["fit_status"].append(prim.fit_status)
        resultsdict["location_type"].append(prim.location_type)
        resultsdict["shape"].append(prim.shape)
        resultsdict["speed"].append(prim.speed)
        resultsdict["time"].append(prim.time)
        resultsdict["type"].append(prim.type)
        resultsdict["weight"].append(mp["Weight"])
        #resultsdict["radius"].append(prim.pos.r)
        #resultsdict["phi"].append(prim.pos.phi)
        #resultsdict["theta"].append(prim.pos.theta)
        #resultsdict["x"].append(prim.pos.x)
        #resultsdict["y"].append(prim.pos.y)
        #resultsdict["z"].append(prim.pos.z)

tray.AddModule(getResultsFromMCTree,"getResults", Streams=[icetray.I3Frame.DAQ])

#tray.AddModule("Dump","dump")
tray.AddModule("TrashCan", "the can")

tray.Execute(nevents)
tray.Finish()


# ---------------------------------------------------------------------------------------

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import math

if len(betaRange)==1:
    infos="beta=%g, mass=%g, diskDist=%g, distRad=%g" % (betaRange[0], mass, disk_dist, disk_rad)
else:
    infos="betamin=%g,betamax=%g, mass=%g, diskDist=%g, distRad=%g" % (betaRange[0],betaRange[1], mass, disk_dist, disk_rad)


print "\nValidation for %s: " % infos
print "Simulated %d events and found %d events in frame" % (nevents, resultsdict["Counter"])
if nevents  != resultsdict["Counter"]:
    print "ERROR: number of simulated events doesn't match!"

def findDeviations(arr):
    dict={}
    for a in arr:
        if not str(a) in dict:
            dict[str(a)]=1
        else:
            dict[str(a)]+=1
    for key, val in dict.iteritems():
        print "%s\tfound %d times" % (key, val)
    if len(dict) > 1:
        print "ERROR: More than 1 value found for this variable!"

if len(betaRange)==1:
    print "\nTest energy:"
    findDeviations(resultsdict["energy"])

print "\nTest location_type:"
findDeviations(resultsdict["location_type"])

print "\nTest shape:"
findDeviations(resultsdict["shape"])

if len(betaRange)==1:
    print "\nTest speed:"
    findDeviations(resultsdict["speed"])

print "\nTest time:"
findDeviations(resultsdict["time"])

print "\nTest fit_status:"
findDeviations(resultsdict["fit_status"])

print "\nTest type:"
findDeviations(resultsdict["type"])

# ----------------------

def plotRandomDistributedVariables(title,arr, infos="", weights=None):
    print "\nCreate and save a plot for random distributed variable: %s" % title
    mi=np.min(arr)
    ma=np.max(arr)
    borders=[mi, ma]
    binNumber=50
    binning=[ i*float((borders[1]-borders[0]))/binNumber+borders[0] for i in range(binNumber+1)]

    if weights!=None:
        #print nevents
        #print sum(weights)
        discRadius=float(disk_rad*100); # m -> cm
        discArea=math.pi * discRadius**2
        w=flux * discArea * 4 * math.pi
        #print w


        weights = np.array(weights) / sum(weights)
        print sum(weights)
        weights*=w
        print sum(weights)


    hist, bins = np.histogram(arr, bins=binning, weights=weights)
    center=(bins[:-1] + bins[1:]) / 2
    width = 0.7 * (bins[1] - bins[0])
    fig, ax = plt.subplots()
    ax.bar(center, hist, align='center', width=width, color="b")

    #if title!="energy":
    #    # expectation
    #    exp=float(len(arr))/binNumber

    ax.set_title(str(len(arr))+" events, "+infos)
    ax.set_xlabel(title)
    ax.set_ylabel("a.u.")
    if weights!=None:
        ax.set_yscale("log", nonposy='clip')
    fig.savefig(title+".png")

plotRandomDistributedVariables("cos_zenith", np.cos(resultsdict["zenith"]), infos=infos, weights=resultsdict["weight"])
plotRandomDistributedVariables("azimuth", np.array(resultsdict["azimuth"]), infos=infos, weights=resultsdict["weight"])
plotRandomDistributedVariables("speed", np.array(resultsdict["speed"])/0.3, infos=infos)
plotRandomDistributedVariables("speed_weighted", np.array(resultsdict["speed"])/0.3, infos=infos, weights=resultsdict["weight"])
