from icecube.finiteReco import I3FiniteCuts
from icecube.finiteReco import I3StartStopParams
from icecube import tableio

class I3FiniteCutsConverter(tableio.I3Converter):
	booked = I3FiniteCuts
	def CreateDescription(self,fincuts):
		desc = tableio.I3TableRowDescription()
		desc.add_field('length',	  tableio.types.Float64,'m', 'The estimated length of the track')
		desc.add_field('detector_length', tableio.types.Float64,'m', 'The estimated length of the detector the event has passed')
		desc.add_field('s_det',		  tableio.types.Float64, '', 'This parameter is similar to a smoothness. It depends on the distribution of the Cherenkov light emission points for the given hits along the track.')
		desc.add_field('start_fraction',  tableio.types.Float64, '', 'Similar to end_fraction. Distance between the interaction vertex and the border of the detector. From the border on Cherenkov light can reach a DOM without scattering.')
		desc.add_field('end_fraction',    tableio.types.Float64, '', 'Distance between the stop point of the track and the border of the detector. As border of the detector the last Cherenkov emission point on the track is used. Light emitted up to this point can reach a DOM without scattering.')
		desc.add_field('finite_cut',	  tableio.types.Float64, '', 'Sum of the signed distances between the middle of an assumed infinite track and the Cherenkov emission points on the track corresponding to the given hits. The sum is normalized by the number of hits.')
		return desc
	def FillRows(self,fincuts,rows):
		rows['length']		       = fincuts.Length
		rows['detector_length']        = fincuts.DetectorLength
		rows['s_det']		       = fincuts.Sdet
		rows['start_fraction']	       = fincuts.startFraction
		rows['end_fraction']	       = fincuts.endFraction
		rows['finite_cut']	       = fincuts.finiteCut
		return 1 # return number of rows added (only one in this case)
		
class I3StartStopParamsConverter(tableio.I3Converter):
	booked = I3StartStopParams
	def CreateDescription(self,ssparams):
		desc = tableio.I3TableRowDescription()
		desc.add_field('llh_inf_track',	      tableio.types.Float64,'', 'log likelihood for an infinite track')
		desc.add_field('llh_starting_track',  tableio.types.Float64,'', 'log likelihood for a starting track')
		desc.add_field('llh_stopping_track',  tableio.types.Float64,'', 'log likelihood for a stopping track')
		return desc
	def FillRows(self,ssparams,rows):
		rows['llh_inf_track']		  = ssparams.LLHInfTrack
		rows['llh_starting_track']        = ssparams.LLHStartingTrack
		rows['llh_stopping_track']	  = ssparams.LLHStoppingTrack
		return 1 # return number of rows added (only one in this case)
		
tableio.I3ConverterRegistry.register(I3FiniteCutsConverter)
tableio.I3ConverterRegistry.register(I3StartStopParamsConverter)
