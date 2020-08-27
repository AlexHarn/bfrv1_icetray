#!/usr/bin/env python

import argparse
from icecube import icetray,dataio,dataclasses

def replace_noise_rate(ref, oldcal,geo,thresh):

    newcal = dataclasses.I3Calibration()
    newcal.dom_cal = oldcal.dom_cal.copy()
    newcal.end_time = oldcal.end_time
    newcal.start_time = oldcal.start_time
    newcal.vem_cal = oldcal.vem_cal
    
    for k,c in oldcal.dom_cal.items():        
        if c.dom_noise_rate < thresh:
            if k not in geo:
                continue
            omtype = geo[k].omtype
            if omtype != dataclasses.I3OMGeo.IceCube:
                continue
            if k not in ref.dom_cal:
                continue
            ref_rate = ref.dom_cal[k].dom_noise_rate
            if ref_rate <thresh:
                continue
            print("Replacing noise rate for {:14s} : {:6.2f} Hz -> {:6.2f} Hz"
                  .format(str(k),c.dom_noise_rate/icetray.I3Units.hertz,
                          ref_rate/icetray.I3Units.hertz))
            newcal.dom_cal[k].dom_noise_rate = ref_rate
    return newcal

def main():

    parser = argparse.ArgumentParser(
        description='Creates a GCD file which is an exact copy of INFILE except the noise rates '
        'for DOMs with obviously low noise are replaced with the value from REFFILE')
    parser.add_argument('-t','--threshold',type=float, default=300*icetray.I3Units.hertz,
                        help='Replace the noise rate if it is under this value [ Default = 300 Hz ]')    
    parser.add_argument('-r','--reffile',type=str,required=True,
                        help='file to use as a reference for noise rate')
    parser.add_argument('-i','--infile',type=str,required=True,
                        help='file to copy everything that isn\'t noise')
    parser.add_argument('-o','--outfile',type=str,required=True,
                        help='file to write')
    args = parser.parse_args()

    print (args.threshold)

    reffile = dataio.I3File(args.reffile,'r')
    while reffile.more():
        frame = reffile.pop_frame()
        if frame.Stop==icetray.I3Frame.Calibration:
            refcal = frame["I3Calibration"]
            break
    reffile.close()

    infile = dataio.I3File(args.infile,'r')
    outfile = dataio.I3File(args.outfile,'w')

    while infile.more():
        frame = infile.pop_frame()
        if frame.Stop==icetray.I3Frame.Geometry:
            geo = frame["I3Geometry"].omgeo            
    
        if frame.Stop==icetray.I3Frame.Calibration:
            incal = frame["I3Calibration"]
            new_dom_cal = replace_noise_rate(refcal, incal,geo,args.threshold)            
            del  frame["I3Calibration"]
            frame["I3Calibration"]=new_dom_cal
        
        outfile.push(frame)
        
    infile.close()

if __name__ == "__main__":
    main()



