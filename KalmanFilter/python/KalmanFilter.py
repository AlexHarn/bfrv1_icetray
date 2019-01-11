#
#   Kalman filter module
# 
#   KalmanFilter.py
#   KalmanAlgorithm.py
#
#   Created on: 24-09-2012
#   Author: Simon Zierke
#   E-Mail: zierke@physik.rwth-aachen.de
#

from icecube import icetray, dataclasses

import numpy as np
import math

from icecube.KalmanFilter.KalmanAlgorithm import *

####################################################################################
##########################            hitfilter           ##########################
####################################################################################
# Wrapper for timewise sorted Hits(DOMLaunches oder RecoPulses) - [(OMkey, Time, Hit)]

class SlowMPHit:
  def __init__(self, OMKey, time, info):
     self.omkey = OMKey
     self.time = time
     self.info = info


def hitfilter(frame, inputMapName, ignoreDC=False):
    hitlist = []
    
    if type(frame[inputMapName]) == dataclasses.I3RecoPulseSeriesMapMask:
        inputMap = frame[inputMapName].apply(frame)
    else:
        inputMap = frame[inputMapName]

    
    for omkey, recoPulseVector in inputMap:
        if ignoreDC and omkey.string > 78:
            continue
        else:
            hitlist.extend([SlowMPHit(omkey, pulse.time, pulse) for pulse in recoPulseVector])
    
    hitlist.sort(key=lambda h: h.time)
    
    return hitlist
    


####################################################################################
##########################         LineFitCallKalman      ##########################
####################################################################################

def LineFitCallKalman(X, T):
    X = np.array(X)
    T = np.array(T)
    
    particle = dataclasses.I3Particle()
    particle.shape = dataclasses.I3Particle.InfiniteTrack
    
    if np.var(T) != 0.:
        v = (np.mean(X * T[:, np.newaxis], axis=0) - np.mean(X, axis=0) * np.mean(T)) / np.var(T)
        x = np.mean(X, axis=0) - v * np.mean(T)
        
        particle.pos = dataclasses.I3Position(x[0], x[1], x[2])
        particle.dir = dataclasses.I3Direction(v[0], v[1], v[2])
        particle.speed = np.linalg.norm(v)
        particle.time = 0
        particle.fit_status = dataclasses.I3Particle.FitStatus.OK
    
    else:
        particle.fit_status = dataclasses.I3Particle.FitStatus.InsufficientHits
    
    return particle


####################################################################################
##########################          Chi2CallKalman        ##########################
####################################################################################

def Chi2CallKalman(X,T,Particle):
    if len(T) > 2:
        chi2 = 0.
        v = np.array([Particle.dir.x, Particle.dir.y, Particle.dir.z]) * Particle.speed
        chi2 = np.sum(np.linalg.norm(X - (Particle.pos + v * np.array(T)[:, np.newaxis]) ) ** 2)
        return chi2/(3.*len(T)-6.)
    
    else:
       return float('nan')
    
####################################################################################
##########################          MyKalmanSeed          ##########################
####################################################################################

class MyKalmanSeed(icetray.I3ConditionalModule):
    def __init__(self,context):
        icetray.I3ConditionalModule.__init__(self,context)
        self.AddOutBox("OutBox")
        self.AddParameter("InputMapName","Map/Mask as input", "SLOPPulseMaskMPClean")
        self.AddParameter("OutputTrack","Name for the output track", "MyKalman")
        self.AddParameter("IgnoreDC", "Whether to include DeepCore strings or not.", False)
    
    def Configure(self):
        self.InputMapName = self.GetParameter("InputMapName")
        self.OutputTrack = self.GetParameter("OutputTrack")
        self.IgnoreDC = self.GetParameter("IgnoreDC")
    
    def DAQ(self,frame):
        self.PushFrame(frame)
    
    def Physics(self,frame):
        if  frame.Has('I3Geometry') and frame.Has(self.InputMapName):
            geo = frame['I3Geometry']
            hitlist = hitfilter(frame, self.InputMapName, self.IgnoreDC)
            
            if not frame.Has(self.OutputTrack) and len(hitlist) > 1:
                x, y, z, t = [], [], [], []
                for hit in hitlist:
                    pos = geo.omgeo[hit.omkey].position
                    x += [np.array(pos)]
                    t += [hit.time]
                particle = LineFitCallKalman(x,t)
                if particle.fit_status == dataclasses.I3Particle.FitStatus.OK:
                    frame[self.OutputTrack] = particle
        self.PushFrame(frame)

####################################################################################
##########################            MyKalman            ##########################
####################################################################################

class MyKalman(icetray.I3ConditionalModule):
    def __init__(self,context):
        icetray.I3ConditionalModule.__init__(self,context)
        
        self.AddOutBox("OutBox")
        
        self.AddParameter("InputMapName","Map/Mask as input", "SLOPPulseMaskMPClean")
        self.AddParameter("InputTrack","Name of the track for the seed", "")
        self.AddParameter("OutputTrack","Name for the output track", "MyKalman")
        self.AddParameter("CutRadius","Radius of the sphere around the track", 200)
        self.AddParameter("Iterations","Number of global iterations", 3)
        
        self.AddParameter("NoiseQ","Process noise", 1e-11)
        self.AddParameter("NoiseR","Observation noise (RMS^2)", 3600)
        
        self.AddParameter("IterationMethod","Number of method used for the calculation of the resulting track. 1-Last iteration, 2-LineFit on Hits, 3-LineFit on iterations", 2)
        self.AddParameter("UseAcceleration","Use an accelerated movement", False)
        self.AddParameter("AdditionalInformation","Store additional information", False)
   
        self.AddParameter("IgnoreDC", "Whether to include DeepCore strings or not.", False)
     
    def Configure(self):
        self.inputMapName = self.GetParameter("InputMapName")
        self.inputTrack = self.GetParameter("InputTrack")
        self.outputTrack = self.GetParameter("OutputTrack")
        self.cutRadius = self.GetParameter("CutRadius")
        self.iterations = self.GetParameter("Iterations")
        
        self.noiseQ = self.GetParameter("NoiseQ")
        self.noiseR = self.GetParameter("NoiseR")
        self.iterationMethod = self.GetParameter("IterationMethod")
        
        self.useAcceleration = self.GetParameter("UseAcceleration")
        self.additionalInformation = self.GetParameter("AdditionalInformation")
        
        self.IgnoreDC = self.GetParameter("IgnoreDC")

    def Geometry(self, frame):
        self.geo = frame['I3Geometry']
        self.PushFrame(frame)
    
    def DAQ(self,frame):
        self.PushFrame(frame)
    
    def Physics(self,frame):
        if not frame.Has(self.inputMapName):
            self.PushFrame(frame)
            return
            
        if type(frame[self.inputMapName]) == dataclasses.I3RecoPulseSeriesMapMask or \
            type(frame[self.inputMapName]) == dataclasses.I3RecoPulseSeriesMap:
            workingOnPulses = True
        else:
            workingOnPulses = False
        
        
        if frame.Has(self.inputTrack):
            seed = frame[self.inputTrack]
            if type(frame[self.inputTrack]) == dataclasses.I3MCTree:
                seed = frame[self.inputTrack][0]
            
            initialPos = [seed.pos.x, seed.pos.y, seed.pos.z]
            initialDir = [seed.speed * seed.dir.x, seed.speed * seed.dir.y, seed.speed * seed.dir.z]
            initialState = initialPos + initialDir
        else:
            print("Kalman - Error: InputTrack not found")
            self.PushFrame(frame)
            return
        
        
        ## timewise sorted Pulses/DomLaunches - [(OMkey, Time, Pulse/DomLaunch)]
        hitlist = hitfilter(frame,self.inputMapName,self.IgnoreDC)
        
        if self.useAcceleration:
            stateDim = 9
        else:
            stateDim = 6
        
        ## Initialize Kalman filter
        kalman = Kalman(stateDim, 3, initialState, self.noiseQ, self.noiseR)
        
        for i in range(self.iterations):
            
            ## create variables for results
            prediction = dataclasses.I3VectorI3Particle()
            if self.useAcceleration: 
                acceleration = dataclasses.I3VectorI3Particle()
            
            varianceVector = dataclasses.I3VectorDouble()
            if workingOnPulses:
                recoPulseSeriesMapMask = dataclasses.I3RecoPulseSeriesMapMask( frame, self.inputMapName)
                recoPulseSeriesMapMask.set_none()
            else:
                domLaunchSeriesMap = dataclasses.I3DOMLaunchSeriesMap()
            
            
            ## write seed to predictionlist
            seed = dataclasses.I3Particle()
            seed.pos = dataclasses.I3Position(initialState[0], initialState[1], initialState[2])
            seed.dir = dataclasses.I3Direction(initialState[3], initialState[4], initialState[5])
            seed.speed = np.linalg.norm(initialState[3:6])
            seed.time = 0
            prediction.append(seed)
            
            ## Write acceleration seed
            if self.useAcceleration:
                accel = dataclasses.I3Particle()
                accel.pos = dataclasses.I3Position(0, 0, 0)
                acceleration.append(accel)
            
            
            ## clean variables
            hitList = []
            predList = []
            time = []
            chi2 = 0
            lastTime = 0
            
            p = kalman.Predict()
            
            ## loop over all hits in given map
            for hit in hitlist:
                
                ## extrapolate Kalman-Prediction
                
                pos = p[0:3] + p[3:6] * (hit.time - lastTime)
                
                om = np.array(self.geo.omgeo[hit.omkey].position)
                
                ## Distance to next hit
                d = np.linalg.norm(om - pos)
                
                if d <= self.cutRadius:
                    dt = hit.time - lastTime
                    lastTime = hit.time
                    
                    ## Update Kalman filter with new hit
                    kalman.Update(om, dt)
                    
                    p = kalman.Predict()
                    
                    ## Uncomment to activate debug mode
                    #kalman.Debug()
                    
                    ## Add Kalman-step to predictionlist
                    pred = dataclasses.I3Particle()
                    pred.pos = dataclasses.I3Position(p[0], p[1], p[2])
                    pred.dir = dataclasses.I3Direction(p[3], p[4], p[5])
                    pred.speed = np.linalg.norm(p[3:6])
                    pred.time = hit.time
                    prediction.append(pred)
                    
                    if self.useAcceleration:
                        accel = dataclasses.I3Particle()
                        accel.pos = dataclasses.I3Position(p[6], p[7], p[8])
                        acceleration.append(accel)
                    
                    varianceVector.append(kalman.GetVariance()[0])
                    chi2 += d**2
                    
                    ## write PulseMapMask/DOMLaunchMap
                    if workingOnPulses:
                        recoPulseSeriesMapMask.set(hit.omkey, hit.info, True)
                    else:
                        if hit.omkey in domLaunchSeriesMap:
                            domLaunchSeriesMap[hit.omkey].append(hit.info)
                        else:
                            domLaunchSeries = dataclasses.I3DOMLaunchSeries()
                            domLaunchSeries.append(hit.info)
                            domLaunchSeriesMap[hit.omkey] = domLaunchSeries
                    
                    hitList += [om]
                    predList += [p[0:3]]
                    
                    time += [hit.time]
                    
            
            ## new initialization for the next global iteration
            if len(hitList) > 0:
                ## use last iteration as LineFit
                if self.iterationMethod == 1: 
                    x0 = p[0] - p[3] * hit.time
                    y0 = p[1] - p[4] * hit.time
                    z0 = p[2] - p[5] * hit.time
                    
                    particle = dataclasses.I3Particle()
                    particle.pos = dataclasses.I3Position(x0, y0, z0)
                    particle.dir = dataclasses.I3Direction(p[3], p[4], p[5])
                    particle.speed = np.linalg.norm([p[3:6]])
                    particle.time = 0
                
                elif self.iterationMethod == 2:
                    ## calculate the LineFit on selected pulses
                    particle = LineFitCallKalman(hitList, time)
                
                elif self.iterationMethod == 3:
                    ## calculate the LineFit on iteration steps
                    particle = LineFitCallKalman(predList, time)
                else:
                    raise NotImplementedError("No IterationMethod with number %s" % str(self.IterationMethod))
                
                ## Last Iteration ?
                if i < self.iterations - 1:
                    initialPos = [particle.pos.x, particle.pos.y, particle.pos.z]
                    initialDir = [particle.speed * particle.dir.x, particle.speed * particle.dir.y, particle.speed * particle.dir.z]
                    initialState = initialPos + initialDir
                    initialP = kalman.GetVarianceVector()
                    kalman = Kalman(stateDim, 3, initialState, self.noiseQ, self.noiseR, initialP)
                    
                else:
                    ## Write everything to the frame
                    
                    frame["%s_LineFit"%(self.outputTrack) ] = particle
                    frame["%s_NHits"%(self.outputTrack)] = dataclasses.I3Double(len(hitList))
                    frame["%s_P"%(self.outputTrack)] = varianceVector
                    frame["%s_Chi2Pred"%(self.outputTrack)] = dataclasses.I3Double(Chi2CallKalman(predList, time, particle))
                    
                    if workingOnPulses:
                        frame["%s_Map"%(self.outputTrack)] = recoPulseSeriesMapMask
                    else:
                        frame["%s_Map"%(self.outputTrack)] = domLaunchSeriesMap
                    
                    if self.additionalInformation:
                        frame["%s_Pred"%(self.outputTrack)] = prediction
                        if self.useAcceleration: frame["%s_Accel"%(self.outputTrack)] = acceleration
        
        self.PushFrame(frame)
        
    def Finish(self):
        return True


