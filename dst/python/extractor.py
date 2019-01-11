from icecube import icetray, dataclasses
import time
import logging

def filterStream(f, StreamName='TDST'):
    if "I3EventHeader" in f and f["I3EventHeader"].sub_event_stream == StreamName:
          return False
    return True

@icetray.traysegment
def ExtractDST13(tray, name, 
	dst_output_filename  = "I3DST13.root",
	dstname      = "I3DST13",
	extract_to_frame = True,
	If = lambda f: True):
	
	"""
	Record in compact form limited information from reconstructions, triggers and cut
	parameters for every triggered event.
	"""
	from icecube import dst
	from icecube import phys_services
	from icecube.tableio import I3TableWriter
	from icecube.rootwriter import I3ROOTTableService
	from icecube.hdfwriter import I3HDFTableService
	from . import TDSTConverter
	
	# Open output file
	if dst_output_filename.endswith('.root'):
	# Open output file
	   table_service = I3ROOTTableService(filename= dst_output_filename,  
                                   master= "dst", #Default name: "MasterTree".
                                   #mode=RECREATE,     
                                   )
	elif dst_output_filename.endswith('.hdf5'):
	   table_service = I3HDFTableService(dst_output_filename, 6)

	if "I3RandomService" not in tray.context:
	   dstRng = phys_services.I3GSLRandomService(int(time.time()))
	   tray.context["I3RandomService"]=dstRng


	tray.AddModule('I3DSTExtractor13', 'UnpackDST',
                SubEventStreamName = 'TDST13',
                FileName        = dst_output_filename,
                DSTName         = dstname,
                DSTHeaderName   = dstname+"Header",
                EventHeaderName = 'I3EventHeader',
                ExtractToFrame  = extract_to_frame,
                TriggerName     = 'I3TriggerHierarchy',
               )

	tray.AddModule(I3TableWriter, "writer",
               TableService = table_service,
               SubEventStreams= ['TDST13'],           
               Keys = [ "CutDST", "TDSTTriggers"],  
               )

	
	
@icetray.traysegment
def ExtractDST(tray, name, 
	dst_output_filename  = "I3DST.root",
	dstname      = "I3DST",
	simulation = False,
	extract_to_frame = False,
	remove_filter_stream = True,
	cut_data = False,
	If = lambda f: True):
	
	"""
	Record in compact form limited information from reconstructions, triggers and cut
	parameters for every triggered event.
	"""
	from icecube import dst
	from icecube import phys_services
	from icecube.tableio import I3TableWriter
	from icecube.rootwriter import I3ROOTTableService
	from icecube.hdfwriter import I3HDFTableService
	from . import TDSTConverter        

	


	if dst_output_filename.endswith('.root'):
	# Open output file
	   table_service = I3ROOTTableService(filename= dst_output_filename,  
                                   master= "dst", #Default name: "MasterTree".
                                   #mode=RECREATE,     
                                   )
	elif dst_output_filename.endswith('.hdf5'):
	   table_service = I3HDFTableService(dst_output_filename, 6)

	if "I3RandomService" not in tray.context:
	   dstRng = phys_services.I3GSLRandomService(int(time.time()))
	   tray.context["I3RandomService"]=dstRng

	if simulation:
	   from icecube import filter_tools
	   tray.AddModule("KeepFromSubstream","dst_stream",
	      StreamName = "InIceSplit",
	      KeepKeys = ["I3DST"],
	      KeepStream=True,
	      )


	tray.AddModule('I3DSTExtractor16', 'UnpackDST',
                SubEventStreamName = 'TDST',
                FileName        = dst_output_filename,
                DSTName         = dstname,
                DSTHeaderName   = "I3DSTHeader",
                EventHeaderName = 'I3EventHeader',
                ExtractToFrame  = extract_to_frame,
                TriggerName     = 'I3TriggerHierarchy',
                Cut             = cut_data,
               )
	
	tray.AddModule(I3TableWriter, "writer",
               TableService = table_service,
               SubEventStreams= ['TDST'],           
               Keys = [ "CutDST", "TDSTTriggers"],  
               )

	if remove_filter_stream:
	   tray.AddModule(filterStream,name+'_stream_filter', StreamName='TDST')
	
	
