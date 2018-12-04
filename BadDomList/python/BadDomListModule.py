
import os
try:
    from urllib import urlencode
    from urllib2 import urlopen, Request
except ImportError:
    from urllib.parse import urlencode
    from urllib.request import urlopen, Request
import json
from icecube import icetray, dataclasses
from icecube.icetray import OMKey

class BadDomListModule(icetray.I3Module):
    """
    This module adds a list of bad doms to the D frame.

    It has several configuration parameters with which you can adjust which
    doms will put into the list etc.

    An example of how to use this module is in resources/test/BadDomListModule.py.
    """

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

        self.AddParameter('RunId',
                          'The run id if you are using experimental data',
                          -1)

        self.AddParameter('Simulation',
                          'If you need a bad dom list for simulation',
                          False)

        self.AddParameter('ListName',
                          'Name of the bad dom list in frame')

        self.AddParameter('CustomBadKeys',
                          'User defined list of bad keys (OMKey) that should be added to the bad dom list. Note that custom bad keys are ALWAYS added',
                          [])

        self.AddParameter('TemporaryBadDomsOnly',
                          'Add only temporary bad doms. Does not work with Simumation = True',
                          False)

        self.AddParameter('IgnoreNewDOMs',
                          'Ignore going through all the new modules in GCD files for Upgrade/Gen2 simulations'
                          '(list of ignored OM types defined in NewDOMTypes)',
                          False)
        self.AddParameter('NewDOMTypes', 
                          'List of new OMTypes to ignore if IgnoreNewDOMs is True',
                          [dataclasses.I3OMGeo.mDOM, 
                           dataclasses.I3OMGeo.DEgg, 
                           dataclasses.I3OMGeo.PDOM, 
                           dataclasses.I3OMGeo.UnknownType] 
                          )
        self.AddParameter('DisabledKeysOnly',
                          'Add only disabled doms.',
                          False)

        self.AddParameter('AddGoodSlcOnlyKeys',
                          'Add SLC only keys to the bad dom list.',
                          True)

        self.AddParameter('I3LiveUrlRunDoms',
                          'Url from where the dom information should be requested. Use %s for where the run id should be added. Only for Simulation = False (default)',
                          "https://live.icecube.wisc.edu/run_doms/%s/")

        self.AddParameter('I3LiveUrlSnapshotExport',
                          'Url from where the snapshort information for the run should be queried. Only for Simulation = False (default)',
                          'https://live.icecube.wisc.edu/snapshot-export/')

        self.AddParameter('I3LiveAuth',
                          'Authentication for I3Live. Only for Simulation = False (default)',
                          {'user': 'icecube', 'pass': 'skua'})

        self.AddOutBox("OutBox")

    def Configure(self):
        self.runId = self.GetParameter('RunId')
        self.simulation = self.GetParameter('Simulation')
        self.listName = self.GetParameter('ListName')
        self.customKeys = self.GetParameter('CustomBadKeys')
        self.temporaryBadDomsOnly = self.GetParameter('TemporaryBadDomsOnly')
        self.disabledKeysOnly = self.GetParameter('DisabledKeysOnly')
        self.addGoodSlcOnlyKeys = self.GetParameter('AddGoodSlcOnlyKeys')
        self.i3liveUrlRunDoms = self.GetParameter('I3LiveUrlRunDoms')
        self.i3liveUrlSnapshotExport = self.GetParameter('I3LiveUrlSnapshotExport')
        self.i3liveAuth = self.GetParameter('I3LiveAuth')
        self.ignoreNewDOMs = self.GetParameter('IgnoreNewDOMs')
        self.newDOMTypes = self.GetParameter("NewDOMTypes")
        if self.ignoreNewDOMs: 
            icetray.logging.log_info("In: "+ self.name+ ", the following OMTypes will be ignored: " + 
                        ", ".join([str(onetype) for onetype in self.newDOMTypes]) )
        self.frame = None
        self.runInfo = None
        self.snapshot = None

    def DetectorStatus(self, frame):
        """
        Adds the bad dom list with name `self.listName` to the D frame.
        Which sources are used for the list is specified by the options (see method `Configure` or `__init__`).

        Args:
            frame (icecube.icetray.I3Frame): The detector status frame
        """

        # Make frame accessible for all methods
        self.frame = frame

        badDomList = []

        # Add requested sources
        if self.temporaryBadDomsOnly:
            badDomList = badDomList + self._GetTemporarilyBadDoms()
        elif self.disabledKeysOnly:
            badDomList = badDomList + self._GetUnconfiguredDoms()
        else:
            badDomList = badDomList + self._GetUnconfiguredDoms() \
                                    + self._GetNoHVDoms() \
                                    + self._GetDroppedDoms() \
                                    + self._GetTemporarilyBadDoms()

        # Add good good SLC keys ("dark noise")
        if self.addGoodSlcOnlyKeys:
            badDomList = badDomList + self._GetDarkNoiseModeDoms()

        # Custom keys are always added
        badDomList = badDomList + self.customKeys

        # Sort the list and remove duplicates
        badDomList = set(sorted(badDomList))

        # Convert list
        i3bdl = dataclasses.I3VectorOMKey()
        for om in badDomList:
            i3bdl.append(om)

        # Write list to frame
        frame[self.listName] = i3bdl

        self.PushFrame(frame)

    def _GetUnconfiguredDoms(self):
        """
        Returns the list of unconfigured doms.

        If the simulation flag is set to `True`, it checks the dom status in the frame.
        If a dom is not in the status or has no HV, it is considered as unconfigured dom.

        If you provided a run id and the simulation flag is `False`, it gets the information from I3Live.

        Returns:
            list: A list if `OMKeys`.
        """

        unconfDoms = []
        icetray.logging.log_info("Getting unconfigured OMs")
        # If simulation is True, we need to get the information from the GCD file
        if self.simulation:
            for dom in self.frame['I3Geometry'].omgeo.keys():
                if self.ignoreNewDOMs  and \
                   self.frame['I3Geometry'].omgeo[dom].omtype in self.newDOMTypes : 
                    continue
                if dom not in self.frame['I3DetectorStatus'].dom_status.keys() or \
                   self.frame['I3DetectorStatus'].dom_status[dom].pmt_hv == 0:
                    unconfDoms.append(dom)
        else:
            self._GetDomInfoFromI3Live()
            
            for key, om in self.runInfo['unconfigured_doms'].items():
                unconfDoms.append(OMKey(om['string'], om['position']))
        icetray.logging.log_info("Finished getting unconfigured OMs")
        return unconfDoms

    def _GetNoHVDoms(self):
        """
        Returns the list of doms with no HV.

        If the simulation flag is set to `True`, it checks the dom status in the frame.

        If you provided a run id and the simulation flag is `False`, it gets the information from I3Live.

        Returns:
            list: A list if `OMKeys`.
        """

        noHVDoms = []

        # If simulation is True, we need to get the information from the GCD file
        if self.simulation:
            for dom in self.frame['I3Geometry'].omgeo.keys():
                if self.ignoreNewDOMs and \
                   self.frame['I3Geometry'].omgeo[dom].omtype in self.newDOMTypes : 
                    continue
                if self.frame['I3DetectorStatus'].dom_status[dom].pmt_hv == 0:
                    noHVDoms.append(dom)
        else:
            self._GetDomInfoFromI3Live()
            
            if 'No HV' in self.runInfo['problem_doms'].keys():
                for key, om in self.runInfo['problem_doms']['No HV'].items():
                    noHVDoms.append(OMKey(om['string'], om['position']))

        return noHVDoms

    def _GetDroppedDoms(self):
        """
        Returns a list of dropped doms. Only experimental data supported (run id required)

        Returns:
            list: A list of `OMKeys`.
        """

        # Simulation is not supported
        if self.simulation:
            icetray.logging.log_fatal("Dropped dom information is not supported for simulation.")

        self._GetDomInfoFromI3Live()

        droppedDoms = []

        # Dropped doms are categorized by souce, e.g. `user_alert` and `moni_file`
        for key, src in self.runInfo['dropped_doms'].items():
            for om in src:
                # Only use this dropped dom if it dropped before the good stop time
                # string compare works since both date times are in the correct format for it
                if om['drop_time'] < self.snapshot['runs'][0]['good_tstop']:
                    droppedDoms.append(OMKey(om['dom_string'], om['dom_position']))

        return droppedDoms

    def _GetTemporarilyBadDoms(self):
        """
        Returns a list of temporarily bad doms. Only experimental data supported (run id required)

        Returns:
            list: A list of `OMKeys`.
        """

        # Simulation is not supported
        if self.simulation:
            icetray.logging.log_fatal("Temporarily bad dom information is not supported for simulation.")

        self._GetDomInfoFromI3Live()

        tmpBadDoms = []
       
        if 'Temporarily bad' in self.runInfo['problem_doms'].keys(): 
            for key, om in self.runInfo['problem_doms']['Temporarily bad'].items():
                tmpBadDoms.append(OMKey(om['string'], om['position']))

        return tmpBadDoms

    def _GetDarkNoiseModeDoms(self):
        """
        Returns a list of good dark noise doms. That means, doms that have HV, are configured, and are in dark noise mode.

        If the simulation flag is set to `True`, it checks the dom status in the frame.

        If you provided a run id and the simulation flag is `False`, it gets the information from I3Live.

        Note: Currently is the only source the frame. Will be updated as soon I3Live provides this information.

        Returns:
            list: A list of `OMKeys`.
        """

        goodDarkNoiseDoms = []

        # If simulation is True, we need to get the information from the GCD file
        if self.simulation:
            for dom in self.frame['I3Geometry'].omgeo.keys():
                if self.ignoreNewDOMs and \
                   self.frame['I3Geometry'].omgeo[dom].omtype in self.newDOMTypes : 
                    continue
                if dom in self.frame['I3DetectorStatus'].dom_status.keys() and \
                   self.frame['I3DetectorStatus'].dom_status[dom].lc_mode == self.frame['I3DetectorStatus'].dom_status[dom].LCMode.SoftLC and \
                   self.frame['I3DetectorStatus'].dom_status[dom].pmt_hv > 0:
                    goodDarkNoiseDoms.append(dom)
        else:
            self._GetDomInfoFromI3Live()
            
            if 'No LC' in self.runInfo['problem_doms'].keys():
                for key, om in self.runInfo['problem_doms']['No LC'].items():
                    position = "%s-%s" % (om['string'], om['position'])

                    if position in self.runInfo['configured_doms'].keys() and \
                       position not in self.runInfo['problem_doms']['No HV'].keys():
                        goodDarkNoiseDoms.append(OMKey(om['string'], om['position']))

        return goodDarkNoiseDoms


    def _GetDomInfoFromI3Live(self, force = False):
        """
        Requests dom information from I3Live via web interface.
        It stores the result in `self.runInfo` and uses `self.runId`.

        It only gets the information from I3Live if this module doesn't hold any or
        if the `force` argument is set to `True`.

        Args:
            force (bool): Force to reload information. Default is `False`.
        """

        # Only load information if necessary
        if self.runInfo is not None and self.snapshot is not None and not force:
            return
   
        if self.runId < 0 or self.simulation:
            icetray.logging.log_fatal("Tries to get dom information from I3Live without having " +
                                      "a run number (runId = %s) or the simulation flag is set " % self.runId +
                                      "(simulation = %s)" % self.simulation)
 
        url = self.i3liveUrlRunDoms % self.runId
    
        # Create and send request
        request = Request(url,
                          urlencode({'user': self.i3liveAuth['user'],
                                     'pass': self.i3liveAuth['pass']}
                          ).encode('ascii')
        )

        response = urlopen(request)
        responseText = response.read() 
        self.runInfo = json.loads(responseText.decode('ascii'))

        # Query information for good stop time
        request = Request(self.i3liveUrlSnapshotExport,
                          urlencode({'user': self.i3liveAuth['user'],
                                     'pass': self.i3liveAuth['pass'],
                                     'start_run': self.runId,
                                     'end_run': self.runId}
                          ).encode('ascii')
        )

        response = urlopen(request)
        responseText = response.read()
        self.snapshot = json.loads(responseText.decode('ascii'))

        if len(self.snapshot['runs']) != 1:
            icetray.logging.log_fatal("Expected to get exactly one entry for run %s from the snapshot export " % self.runId +
                                      "but got %s." % len(self.snapshot['runs']))


