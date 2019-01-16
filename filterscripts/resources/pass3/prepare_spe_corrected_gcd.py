#!/usr/bin/env python

def run(infile, spefile, outfile):
    from I3Tray import I3Tray
    from icecube import icetray, dataclasses, dataio
    from icecube.phys_services import spe_fit_injector

    tray = I3Tray()

    tray.AddModule("I3Reader", "reader", filenamelist=[infile])

    # inject the SPE correction data into the C frame
    tray.AddModule(spe_fit_injector.I3SPEFitInjector, "fixspe", Filename=spefile)

    tray.AddModule("I3Writer", filename=outfile)

    tray.Execute()

    print("Done")

def main():
    import argparse
    parser = argparse.ArgumentParser(description='RAW -> SDST processing')
    parser.add_argument('-i', '--infile', type=str, required=True,
                        help='Input GCD i3 filename')
    parser.add_argument('-s','--spefile', type=str, required=True,
                        help='Input SPE correction filename')
    parser.add_argument('-o', '--outfile', type=str, required=True,
                        help='Output GCD i3 filename')
    args = parser.parse_args()

    run(**vars(args))

if __name__ == "__main__":
    main()