#
# Module to correct the Corsika TimeScale when not all events are read from a file.
#                                                                           - Roger Moore (3/7/18)
#

# Import python system package
from copy import copy

# Import basic IceCube requirements
from icecube import icetray, dataio

# Setup logging
from icecube.icetray.i3logging import *


class CorsikaTimeScaleFixer(icetray.I3Module):
    """
    Adjusts the corsika timescale when only a fraction of events in a file are read.
    The module loops over the file to count the number of events at initialization and
    then uses this total number of events along with the number of events to read to
    rescale the TimeScale variable so that subsequent use of this file by e.g. polyplopia
    will correctly calculate the event rate.

    This module will also limit the number of events read to the given number and request a suspension
    after the given number is reached. This means it can be used to limit the number of events more
    accurately than the frame limit given to the tray's execute method which limits all frames, not
    just DAQ frames.

    WARNING: This module should only be used when a Corsika file is deliberately truncated by
    not reading all the events e.g. in the case of a very large background event file. If all
    events are read but only a subset pass some selection criteria for output then the Corsika
    'TimeScale' remains correct and should NOT be changed by use of this module!

    :param InputFile: name of the file containing the events being read
    :param ReadEvents: the maximum number of events to read from the input file
    :param MaxExcess: the number of events over the read limit to tolerate before generating a fatal error
    """
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter('InputFile', 'Name of the file being read', '')
        self.AddParameter('ReadEvents', 'The number of events actually being read', 0)
        self.AddParameter('MaxExcess', 'The maximum number of events over the read limit to tolerate', 50)
        # Define the class attributes here to ensure they always exist
        self.read_events = 0
        self.max_excess = 0
        self.scale_factor = 1.0
        self.events_processed = 0

    def Configure(self):
        filename = self.GetParameter('InputFile')
        self.read_events = self.GetParameter('ReadEvents')
        self.max_excess = self.GetParameter('MaxExcess')

        # Check that we have some parameters
        if not filename:
            raise AttributeError('The input file name is required so the events can be counted')
        if self.read_events <= 0:
            raise AttributeError('The number of read events must be specified and be above zero')
        if self.max_excess < 0:
            raise AttributeError('The maximum number of excess events to tolerate must be zero or higher')

        # Open the input file and loop over all the events to count them
        file = dataio.I3File(filename)
        total_events = 0
        log_info("Counting the events in the file this may take a little while...", "CorsikaTimeScaleFixer")
        while file.more():
            file.pop_frame(icetray.I3Frame.DAQ)
            total_events += 1
        file.close()
        log_info("Found %d events in the input file, reading %d" % (total_events, self.read_events))

        # Quick sanity check on the number of events to read
        if self.read_events > total_events:
            raise AttributeError('The number of events to read (%d) is greater than the number in the file (%d)!'
                                 % (self.read_events, total_events))

        # Calculate the scaling factor for the Corsika time scale and store it
        self.scale_factor = float(self.read_events) / float(total_events)
        log_info('Corsika TimeScale being reduced by a factor of %f' % self.scale_factor, "CorsikaTimeScaleFixer")

        # Set the count of events processed
        self.events_processed  = 0

    def DAQ(self, frame):
        """
        Gets the corsika weightmap and changes the TimeScale value by the calculated scale factor.
        This has to happen for every DAQ frame because a copy of the Corsika weightmap is stored
        in each frame eventhough things like the 'TimeScale' are constant over the file.
        :param frame: frame read from the I3 file
        :return: True if everything worked, false otherwise
        """
        # Increase the count of processed events and request suspension if the number processed matches
        # the number to be read
        self.events_processed += 1
        # If we have reached the number of indicated events then request no more events
        if self.events_processed == self.read_events:
            log_info('The number of events has reached the maximum (%d): ' % self.read_events +
                     'suspending processing', "CorsikaTimeScaleFixer")
            self.RequestSuspension()
        # If we are still getting events beyond what we asked for then calculate the number of events
        # we are over the limit and, if this is too large, make the request a demand by raising a fatal
        # error. If it is "within reason" then just refuse to push the frame which will effectively
        # end all processing.
        elif self.events_processed > self.read_events:
            excess = (self.events_processed-self.read_events)
            log_warn("This event is number %d over read limit, frame not pushed" % excess, "CorsikaTimeScaleFixer")
            if excess >= self.max_excess:
                log_fatal('More events than allowed excess (%d) received after requesting suspension' % self.max_excess,
                          "CorsikaTimeScaleFixer")
            return False
        # Check that there is a weight map otherwise generate a warning and exit
        if 'CorsikaWeightMap' not in frame:
            log_warn('No Corsika weightmap found in frame', "CorsikaTimeScaleFixer")
            self.PushFrame(frame)
            return False
        # Get the weight map from the frame, make a copy of it and then update the copy
        # to the new time scale. Then delete the original weight map and add the new, edited
        # one back into the frame. This is convoluted method is required in case some code has
        # cached the object (method and reason from discussion with ClaudioK)
        weights = frame['CorsikaWeightMap']
        new_weights = copy(weights)
        new_weights['TimeScale'] *= self.scale_factor
        del frame['CorsikaWeightMap']
        frame['CorsikaWeightMap'] = new_weights
        self.PushFrame(frame)
        return True

    def Finish(self):
        # Check that the number of events processed matches the number read
        if self.read_events > self.events_processed:
            log_error("The number of events processed (%d) is less than the number promised to be read (%d)." %
                      (self.events_processed, self.read_events) +
                      " Please make sure the time fixer module is loaded before any filters.",
                      "CorsikaTimeScaleFixer")
        elif self.read_events < self.events_processed:
            log_warn("The number of events processed (%d) exceeded the number promised to be read (%d) " %
                     (self.events_processed, self.read_events) +
                     "but was within the allowed tolerance (%d)" % self.max_excess, "CorsikaTimeScaleFixer")
