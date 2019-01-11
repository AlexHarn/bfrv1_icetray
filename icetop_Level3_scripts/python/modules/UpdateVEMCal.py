import glob, re
from datetime import datetime
from xml.etree import ElementTree

from icecube import dataclasses, icetray

class UpdateVEMCal(icetray.I3Module):
    '''
    This module locates the VEMCal file for a certain date (which is the big bunch of ugly work down here),
    And replaces the VEMCal values in the calibration frame.
    It puts the (wrong) values present in the L2 GCD in separate frame objects, since we need them for the correction. 
    '''
    
    def __init__(self, ctx):
        super(UpdateVEMCal, self).__init__(ctx)
        self.AddParameter('Runnumber', 'Number of the run to be processed')
        self.AddParameter('Day', 'Day for VEMcal determination')
        self.AddParameter('Month', 'Month for VEMcal determination')
        self.AddParameter('Year', 'Year for VEMcal determination')
        self.AddOutBox('OutBox')
    
    def Configure(self):
        self.runnumber = self.GetParameter('Runnumber')
        self.day       = self.GetParameter('Day')
        self.month     = self.GetParameter('Month')
        self.year      = self.GetParameter('Year')
        
        self.vemcal_l3  = dataclasses.I3VEMCalibrationMap()
        self.vemcal_files=glob.glob('/data/exp/IceCube/201?/calibration/VEMCal/VEM_calibration_*.xml')
        self.vemcal_files.sort()
        
    def Calibration(self, frame):

        # Store the pePerVEM and corrFactor used for L2 calibration
        # These will be needed for updating the calibration of existing pulses
        cal_l2 = frame['I3Calibration']
        pe_per_vem_l2       = dataclasses.I3MapKeyDouble()
        mu_peak_width_l2    = dataclasses.I3MapKeyDouble()
        hglg_cross_over_l2  = dataclasses.I3MapKeyDouble()
        corr_factor_l2      = dataclasses.I3MapKeyDouble()
        for key, vemcal in cal_l2.vem_cal:
            pe_per_vem_l2[key]    = vemcal.pe_per_vem
            mu_peak_width_l2[key]    = vemcal.mu_peak_width
            hglg_cross_over_l2[key]  = vemcal.hglg_cross_over
            corr_factor_l2[key]   = vemcal.corr_factor
        frame.Put('IceTopLevel2PEperVEM', pe_per_vem_l2)
        frame.Put('IceTopLevel2MuPeakWidth', mu_peak_width_l2)
        frame.Put('IceTopLevel2HGLGCrossOver', hglg_cross_over_l2)
        frame.Put('IceTopLevel2CorrFactor', corr_factor_l2)
        
        cal = dataclasses.I3Calibration(cal_l2)
        self._find_VEMCal(cal.vem_cal.keys())
        # Replace:
        for key in cal.vem_cal.keys():
            cal.vem_cal[key] = self.vemcal_l3[key]
        frame.Delete('I3Calibration')
        frame.Put('I3Calibration', cal)
        self.PushFrame(frame)

    def _find_VEMCal(self, l2_domkeys):
        # First find the correct vemcal file for that date. 
        # But it can be that some doms do not have appropriate vemcal. For those, you need to look into the previous files until you find them.
        vc = self._find_VEMCal_file()        
        good_vemcal_index=self.vemcal_files.index(vc)
        i=0
        # During first VEMCal checks, we saw that the values used for VEMCal during the below period were bad.
        # (Shown in presentation during CR call https://wiki.icecube.wisc.edu/index.php/CR_phone_call_2016-12-19)
        # Therefore, we will exclude those files and use the values of the files before that. 
        VEMCal_files_not_to_use=["VEM_calibration_2011-12-15.xml","VEM_calibration_2012-01-01.xml","VEM_calibration_2012-01-15.xml","VEM_calibration_2012-02-01.xml"]
        while (len(l2_domkeys)!=len(self.vemcal_l3.keys()) and good_vemcal_index-i>=0):
            vc_file=self.vemcal_files[good_vemcal_index-i]
            if vc_file.split("/")[-1] in VEMCal_files_not_to_use:
                i=i+1 
                icetray.logging.log_info("Trying to use file: %s but we do not trust the VEMCals in this file so will omit it." %vc_file)
                continue
            if i >0:
                for key in l2_domkeys:
                    if key not in self.vemcal_l3.keys():
                        icetray.logging.log_info("OMKey %i,%i not found in previous VEMCal file (or we do not trust the file), looking into previous file now: %s " %(key.string, key.om, vc_file))
            tree=ElementTree.parse(vc_file)
            root=tree.getroot()
            for dom in root.findall('DOM'):
                string                 = int(dom.find('StringId').text)
                om                     = int(dom.find('TubeId').text)
                key                    = icetray.OMKey(string, om)
                if key not in self.vemcal_l3.keys():
                    vemcal                 = dataclasses.I3VEMCalibration()
                    vemcal.pe_per_vem      = float(dom.find('pePerVEM').text)
                    vemcal.mu_peak_width   = float(dom.find('muPeakWidth').text)
                    vemcal.hglg_cross_over = float(dom.find('hglgCrossOver').text)
                    vemcal.corr_factor     = float(dom.find('corrFactor').text)
                    self.vemcal_l3[key]    = vemcal
                
            i=i+1

    def _find_VEMCal_file(self):
        # Find appropriate VEMCal XML file.                                                                                                                                                                 
        # On the switching dates, it could be that we need to look for the week before.
        # If the date is found, but the run is for some reason not in the VEMCal (XML) file, we will use the file of that week, but a warning is given.
        # If the date is not found, this script is stopped.
        date=datetime(self.year,self.month,self.day) 
        # Are time ordered.
        vemcal_file=self.vemcal_files[0]
        vemcal_date=re.search("VEM_calibration_([0-9-]+)",vemcal_file).group(1).split("-")
        vemcal_time=datetime(int(vemcal_date[0]),int(vemcal_date[1]),int(vemcal_date[2]))
        found=False
        foundWeek=False
        for i in range(1, len(self.vemcal_files)):
            next_vemcal_file=self.vemcal_files[i]
            next_vemcal_date=re.search("VEM_calibration_([0-9-]+)",next_vemcal_file).group(1).split("-")
            next_vemcal_time=datetime(int(next_vemcal_date[0]),int(next_vemcal_date[1]),int(next_vemcal_date[2]))
            if (date>=vemcal_time and date<next_vemcal_time):
                foundWeek=True
                if self._validate(vemcal_file):
                    found=True
                else:
                    # Could be that the run is still included in the week before.                                                                                                       
                    vemcal_file_before=self.vemcal_files[i-2]
                    if self._validate(vemcal_file_before):                                    
                        vemcal_file=vemcal_file_before
                        found=True
                break
            else:
                vemcal_time=next_vemcal_time
                vemcal_file=next_vemcal_file
                
        if foundWeek:
            if found:
                icetray.logging.log_info('VEMCal file for runnumber to process ({0}) found.'.format(self.runnumber))
            else:
                icetray.logging.log_warn('VEMCal file for runnumber to process ({0}) NOT found. Using VEMCal file of the appropriate week though.'.format(self.runnumber))
            icetray.logging.log_info('Using VEMCAL: {0}'.format(vemcal_file))
        else:
            icetray.logging.log_fatal('No VEMCal directory for this date seems to exist.')
    
        return vemcal_file
       
    # Validate that we have the right VEMCal file                                                                                                                                                           
    def _validate(self,vc_file):
        tree = ElementTree.parse(vc_file)
        root = tree.getroot()
        inFile=False
        first_run = int(root.find('FirstRun').text)
        last_run  = int(root.find('LastRun').text)
        if self.runnumber < first_run or self.runnumber > last_run:
            icetray.logging.log_info('Runnumber to process ({0}) out of valid range from {1} to {2}. Possibly try again.'.format(self.runnumber, first_run, last_run))
        else:
            inFile=True
            
        return inFile
