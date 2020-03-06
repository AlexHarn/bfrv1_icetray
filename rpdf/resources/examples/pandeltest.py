#!/usr/bin/env python
# -*- coding: utf-8 -*-

from icecube import icetray, phys_services, rpdf
import matplotlib
import numpy

matplotlib.use("agg")
import matplotlib.pyplot

rng = phys_services.I3GSLRandomService(42)
nsamples = 2000
ice = rpdf.H0
effd = 150.*icetray.I3Units.m

samples = [rpdf.pandel_sample(effd, ice, rng) for i in range(nsamples)]

fig = matplotlib.pyplot.figure()
gridspec = matplotlib.pyplot.GridSpec(ncols=1, nrows=1)
ax = fig.add_subplot(gridspec[0, 0])

hist = ax.hist(samples, bins=100)
bincenters = (hist[1][1:] + hist[1][:-1])/2.
pdf = [rpdf.pandel_pdf(b, effd, ice) for b in bincenters]
ax.plot(bincenters, numpy.asarray(pdf)*nsamples/numpy.sum(pdf))

cpdf = rpdf.FastConvolutedPandel(1.5*icetray.I3Units.ns, ice)
pdf = [cpdf.pdf(b, effd) for b in bincenters]
ax.plot(bincenters, numpy.asarray(pdf)*nsamples/numpy.sum(pdf))

gridspec.tight_layout(fig)
fig.savefig("pandeltest.png")
