import os,sys
from I3Tray import *
import os.path 
from os.path import expandvars

from icecube import (icetray,dataclasses)
from icecube.icetray import I3Int



class CheckFrameIndex(icetray.I3Module):
      """
      Check that frame order is preserved after buffering
      """

      def __init__(self,context):
          icetray.I3Module.__init__(self,context)
          self.AddOutBox("OutBox")
          self.AddParameter("Check","Check existing indices", False)

      def Configure(self):
          self.p_index = 0
          self.q_index = 0
          self.g_index = 0
          self.c_index = 0
          self.d_index = 0
          self.check = self.GetParameter("Check")
          
      def check_frame(self,frame,index,label):
          i3index = I3Int(index)
          if self.check: 
              assert frame[label] == i3index, "Indices do not match!!!"
          else:
              frame.Put(label,i3index)

      def Geometry(self,frame):
          self.check_frame(frame,self.g_index,"Gindex")
          self.PushFrame(frame)
          self.g_index += 1

      def Calibration(self,frame):
          self.check_frame(frame,self.c_index,"Cindex")
          self.PushFrame(frame)
          self.c_index += 1

      def DetectorStatus(self,frame):
          self.check_frame(frame,self.d_index,"Dindex")
          self.PushFrame(frame)
          self.d_index += 1

      def DAQ(self,frame):
          self.check_frame(frame,self.q_index,"Qindex")
          self.PushFrame(frame)
          self.q_index += 1

      def Physics(self,frame):
          self.check_frame(frame,self.p_index,"Pindex")
          self.PushFrame(frame)
          self.p_index += 1

      def Finish(self):
          print("Geometry Frames:", self.g_index)
          print("Calibration Frames:", self.c_index)
          print("Detector Status Frames:", self.d_index)
          print("DAQ Frames:", self.q_index)
          print("Physics Frames:", self.p_index)


