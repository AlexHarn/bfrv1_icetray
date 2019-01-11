from icecube import icetray,dataio,dataclasses,simclasses
from icecube.icetray.i3logging import log_fatal

##########################################
# Move MCPrimary from P to Q frame   #
#########################################
class MoveMCPrimary(icetray.I3PacketModule):
    def __init__(self, ctx):
        icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
        self.AddOutBox("OutBox")
       
    def Configure(self):
        pass

    def FramePacket(self, frames):
        if len(frames) <= 1:     # only q frame
            for frame in frames:
                self.PushFrame(frame)
            return

        qframe = frames[0]
        pframes = frames[1:]
        prim = 0
        prim_info = 0
       
        for frame in pframes:
            if 'MCPrimary' in frame:
                if prim != 0:
                    log_fatal("Sorry, but this module is pretty dumb. It cannot treat cases where the MCPrimary appears in more than one P frame.")
                prim = frame['MCPrimary']
                frame.Delete('MCPrimary')
            if 'MCPrimaryInfo' in frame:
                prim_info = frame['MCPrimaryInfo']
                frame.Delete('MCPrimaryInfo')
        
        if prim !=0:
            qframe.Put('MCPrimary', prim)
        if prim_info !=0:
            qframe.Put('MCPrimaryInfo',prim_info)

        self.PushFrame(qframe,"OutBox")
        for frame in pframes:
            self.PushFrame(frame,"OutBox")
