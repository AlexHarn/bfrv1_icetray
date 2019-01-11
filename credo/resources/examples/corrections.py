"""
 id     : $Id: corrections.py 137801 2015-09-21 17:47:10Z jtatar $
 version: $Revision: 137801 $
 date   : $Date: 2015-09-21 13:47:10 -0400 (Mon, 21 Sep 2015) $
 author : emiddell
 
 this script use credo's  python bindings to plot the dom-wise
 correction factors that are applied
"""
try:
    import numpy as n
    import pylab as p
except ImportError:
    print("You need to have numpy and matplotlib installed to run this")

from icecube import icetray, credo

p.figure(figsize=(9,4.5))

amps = n.logspace(-3,5,101)
p.subplot(121)
corr = n.vectorize(lambda amp: credo.GetPhotonicsCorrectionFactor(credo.ICECUBE_DOM, amp, 10., False))
p.semilogx(amps, corr(amps), "g-", label="I3 DOM atwd+fadc")
p.legend(loc="lower left")
p.ylim(-0.1,1.1)
p.xlabel("NPE (photonics")
p.ylabel("correction factor")

dists = n.linspace(0,500,101)
p.subplot(122)
corr = n.vectorize(lambda dist: credo.GetPhotonicsCorrectionFactor(credo.ICECUBE_DOM, 10., dist, False))
p.plot(dists, corr(dists), "r-", label="I3 DOM atwd+fadc")
corr = n.vectorize(lambda dist: credo.GetPhotonicsCorrectionFactor(credo.ICECUBE_DOM, 10., dist, True))
p.plot(dists, corr(dists), "g-", label="I3 DOM atwd only")
p.legend()
p.xlabel("distance cascade DOM / m")
p.ylabel("correction factor")
p.ylim(-.1,1.1)
p.show()
