import os, urllib, hashlib

from icecube import icetray, dataio

class I3GCDSanityChecker(icetray.I3Module):
    def __init__(self, context):
        super().__init__(context)

        default_url = 'http://simprod.icecube.wisc.edu/downloads/GCD/'
        self.AddParameter('GCDFilename', 'Name of GCD file to use.', None)
        self.AddParameter('LocalPath', 'Location of GCD file.', None)
        self.AddParameter('URL', 'URL to pull GCD files from', default_url)

        self.__validated = False
        self.__stops_called = {'Geometry': 0,
                               'Calbration': 0,
                               'DetectorStatus': 0}
        
    def Configure(self):
        self.gcd_filename = self.GetParameter('GCDFilename')
        self.local_path = self.GetParameter('LocalPath')
        self.url = self.GetParameter('URL')        
        
        fn = os.path.join(self.local_path, self.filename)
        if not os.path.exists(fn):
            url = self.url + self.gcd_filename
            response = urllib.urlopen(url)
            with open(fn, 'wb') as f:
                f.write(content.read())
                
        with dataio.I3File(fn) as f:
            self.geometry_frame = None
            self.calibration_frame = None
            self.detector_status_frame = None

            while f.more():
                frame = f.pop_frame()
                if 'I3Geometry' in frame:
                    self.geometry_frame = frame['I3Geometry']
                if 'I3Calibration' in frame:
                    self.calibration_frame = frame['I3Calibration']
                if 'I3DetectorStatus' in frame:
                    self.detector_status_frame = frame['I3DetectorStatus']

        # We'll need to get the checksums from some location
        try:
            base_fn = self.gcd_filename.split('.')[0]
            url = self.url + base_fn + '.checksums'
            response = urllib.urlopen(url)
            self.checksums = pickle.loads(response.read())
        except:
            print("Something went wrong checking/loading checksums")
            
    def Geometry(self, frame):
        frame['I3Geometry'] = self.geometry_frame
        self.__stops_called['Geometry'] += 1
        self.PushFrame(frame)
        
    def Calibration(self, frame):
        frame['I3Calibration'] = self.calibration_frame
        self.__stops_called['Calibration'] += 1
        self.PushFrame(frame)
    
    def DetectorStatus(self, frame):
        frame['I3DetectorStatus'] = self.detector_status_frame
        self.__stops_called['DetectorStatus'] += 1
        self.PushFrame(frame)

    def DAQ(self, frame):
        '''
        Only check the G,C, and D frames at the beginning of the first Q frame.
        '''
        if self.__validated:
            self.PushFrame(frame)
            return

        # At this point we should have access to all frames
        for frame_key in ['I3Geometray', 'I3Calibration', 'I3DetectorStatus']:
            m = hashlib.sha256()
            m.update(frame[frame_key])
            if m.hexdigest() != self.checksums[frame_key]:
                print('The %s frame is different' % frame_key)
            else:
                print('%s checks out.' % frame_key)

        self.PushFrame(frame)
                
    def Finish(self, frame):
        for stop, ncalls in self.__stops_called.items():
            if ncalls > 1 :
                print("WARNING: %s called %d times." % (stop, ncalls))
