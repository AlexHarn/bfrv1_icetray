"""A demonstration of a converter written in pure Python"""

from icecube import tableio,dataclasses
from icecube import dst

class TDSTConverter(tableio.I3Converter):
   booked = dst.TDST
   def CreateDescription(self,part):
        desc = tableio.I3TableRowDescription()
        desc.add_field("LocalMST", tableio.types.Float64, "s", "Local siderial time (GMST)")
        desc.add_field("LocalAntiS", tableio.types.Float64, "s", "Local anti-siderial time (GMAST)")
        desc.add_field("LocalExtS", tableio.types.Float64, "s", "Local extend-siderial time (GMEST)")
        desc.add_field("ModJulDay", tableio.types.Float64, "d", "Modified Julian Day")
        desc.add_field("LLHAzimuthDeg", tableio.types.Float32, "deg", "Log-likelyhood azimuth angle")
        desc.add_field("LLHZenithDeg", tableio.types.Float32, "deg", "Log-likelyhood zenith angle")
        desc.add_field("LFAzimuthDeg", tableio.types.Float32, "deg", "LineFit azimuth angle")
        desc.add_field("LFZenithDeg", tableio.types.Float32, "deg", "LineFit zenith angle")
        desc.add_field("LinLLHOpeningAngleDeg", tableio.types.Float32, "deg", "Opening angle between LLHReco and LineFit")
        desc.add_field("RADeg", tableio.types.Float32, "deg", "Right Ascension")
        desc.add_field("DecDeg", tableio.types.Float32, "deg", "Declination")
        desc.add_field("RAAntiS", tableio.types.Float32, "deg", "Antisiderial RA")
        desc.add_field("DecAntiS", tableio.types.Float32, "deg", "Antisiderial Dec")
        desc.add_field("RAExtS", tableio.types.Float32, "deg", "Extended-siderial RA")
        desc.add_field("RASolar", tableio.types.Float32, "deg", "RA solar time")
        desc.add_field("DecSolar", tableio.types.Float32, "deg", "Dec solar time")
        desc.add_field("RASun", tableio.types.Float32, "deg", "RA of Sun")
        desc.add_field("DecSun", tableio.types.Float32, "deg", "Dec of Sun")
        desc.add_field("RAMoon", tableio.types.Float32, "deg", "RA of Moon")
        desc.add_field("DecMoon", tableio.types.Float32, "deg", "Dec of Moon")
        desc.add_field("LogMuE", tableio.types.Float32, "log(E/GeV)", "Log of MuE energy reco")
        desc.add_field("RLogL", tableio.types.Float32, "", "RLogL quality of reconstruction")
        desc.add_field("CoG_X", tableio.types.Float32, "m", "CoG_X of LLHReco")
        desc.add_field("CoG_Y", tableio.types.Float32, "m", "CoG_Y of LLHReco")
        desc.add_field("CoG_Z", tableio.types.Float32, "m", "CoG_Z of LLHReco")
        desc.add_field("LDir", tableio.types.UInt32, "", "LDir-C")
        desc.add_field("RunId", tableio.types.UInt32, "", "Run ID")
        desc.add_field("NDirHits", tableio.types.UInt32, "", "Number of direct hits")
        desc.add_field("NChannels", tableio.types.UInt16, "", "Number of hit channels")
        desc.add_field("NStrings", tableio.types.UInt16, "", "Number of strings")
        desc.add_field("SubRunId", tableio.types.UInt16, "", "SubRunId")
        desc.add_field("IsGoodLineFit", tableio.types.Bool, "", "Is Good LineFit")
        desc.add_field("IsGoodLLH", tableio.types.Bool, "", "Is Good LLH fit")
        return desc

   def Convert(self,dst,row,frame):
        row['LocalMST'] = dst.localMST
        row['LocalAntiS'] = dst.localAntiS
        row['LocalExtS'] = dst.localExtS
        row['ModJulDay'] = dst.mjdTime
        row['LLHAzimuthDeg'] = dst.llhAzimuth
        row['LLHZenithDeg'] = dst.llhZenith
        row['LFAzimuthDeg'] = dst.lfAzimuth
        row['LFZenithDeg'] = dst.lfZenith
        row['LinLLHOpeningAngleDeg'] = dst.linllhOpeningAngle
        row['RADeg'] = dst.RA
        row['DecDeg'] = dst.Dec
        row['RAAntiS'] = dst.RAAntiS
        row['DecAntiS'] = dst.DecAntiS
        row['RAExtS'] = dst.RAExtS
        row['RASolar'] = dst.RASolar
        row['DecSolar'] = dst.DecSolar
        row['RASun'] = dst.RASun
        row['DecSun'] = dst.DecSun
        row['RAMoon'] = dst.RAMoon
        row['DecMoon'] = dst.DecMoon
        row['LogMuE'] = dst.logMuE
        row['RLogL'] = dst.rlogl
        row['CoG_X'] = dst.cogx
        row['CoG_Y'] = dst.cogy
        row['CoG_Z'] = dst.cogz
        row['LDir'] = dst.ldir
        row['RunId'] = dst.runId
        row['NDirHits'] = dst.ndir
        row['NChannels'] = dst.nchan
        row['NStrings'] = dst.nstring
        row['SubRunId'] = dst.subrunId
        row['IsGoodLineFit'] = dst.isGoodLineFit
        row['IsGoodLLH'] = dst.isGoodLLH
        return 1
		
tableio.I3ConverterRegistry.register(TDSTConverter)
