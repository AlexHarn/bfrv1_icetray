 ###################################################################################
 # TraySegment: FillRatio
 # @Version $Id: $
 # @date: $Date: $
 # @author Michael Larson <mjlarson@nbi.ku.dk>
 # (c) 2016 IceCube Collaboration
 ###################################################################################
import icecube
from icecube import icetray

@icetray.traysegment
def FillRatioModule(tray, name = "FillRatioSegment",
		    VertexName = "CFirst",
		    ResultName = "FillRatioInfo",
		    MapName    = "FillRatioMap",
		    RecoPulseName = "RecoPulses",
		    SphericalRadiusRMS = 3.0,
		    SphericalRadiusMean = 2.0,
		    SphericalRadiusMeanPlusRMS = 1.3,
		    SphericalRadiusNCh = 1.3,
		    AmplitudeWeightingPower = 0.0,
		    If = lambda f: True, 
		    ):
	"""
	Algorithm to look for the fraction of hit DOMs to unhit DOMs within some distance
	to a given vertex.
	
	:param VertexName: Name of the previous vertex reconstruction
	:param ResultName: The name that the resulting I3FillRatioInfo object will take in the frame"
	:param MapName:    The name that the I3MapStringDouble object will take in the frame"
	:param RecoPulseName: The name of the recopulses to be used
	:param SphericalRadiusRMS: The radius (in units of the RMS) of the sphere to be used to calculate the fill ratio.
	:param SphericalRadiusMean: The radius (in units of the mean) of the sphere used to calculate the fill ratio.
	:param SphericalRadiusMeanPlusRMS: The radius (in units of the mean plus rms) of the sphere used to calculate the fill ratio.
	:param SphericalRadiusNCh: The radius (in units of the SPE Radius) of the sphere used to define the fill-ratio
	:param AmplitudeWeightingPower: The means and RMSs can be weighted by the charge of the hit. \
	       This parameter sets the power for the exponential wieghting.
	:param If: the usual python function, makes the segment run conditionally frame by frame. 
	"""
        

	tray.AddModule("I3FillRatioModule", "fill-ratio_" + name ,
		       VertexName = VertexName,
		       RecoPulseName = RecoPulseName, 
		       ResultName = ResultName,
		       SphericalRadiusRMS = SphericalRadiusRMS,
		       SphericalRadiusMean = SphericalRadiusMean,
		       AmplitudeWeightingPower = AmplitudeWeightingPower,
		       )

	tray.AddModule("I3FillRatio2StringDoubleMap", "FR_to_Map_" + name,
		       FillRatioOutputName = ResultName,
		       ResultStringDoubleMap = MapName)

	return

