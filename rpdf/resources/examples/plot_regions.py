#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Plot the different Pandel regions.

"""
from math import sqrt

from icecube import rpdf

import matplotlib
matplotlib.use("agg")

import matplotlib.pyplot as plt

ice = rpdf.H2
sigma = 15.

color = 'r'

# dmin = -0.1*ice.scattering_length
# dmax = 0.5*ice.scattering_length
# tmin = -8.*sigma
# tmax = 33.*sigma

t12 = 30.*sigma
t15 = -5.*sigma
t34 = rpdf.H2.rho * sigma**2
tAB = sigma*(ice.rho*sigma - 1.35)
tBC = sigma*(ice.rho*sigma + sqrt(200.))
d1 = ice.scattering_length
d2 = 5.*ice.scattering_length
dBC = 0.05*ice.scattering_length


def plot(ax, tmin, tmax, dmin, dmax, tlabels, dlabels):
    ax.plot([tmin, tmax], [0., 0.], c=color)
    ax.plot([t12, t15], [d2, d2], c=color)
    ax.plot([t12, tmax], [d1, d1], c=color)
    ax.plot([tmin, t15], [d1, d1], c=color)
    ax.plot([t12, t12], [0., d2], c=color)
    ax.plot([t15, t15], [0., d2], c=color)
    ax.plot([t15, t15], [0., d2], c=color)
    ax.plot([t34, t34], [d2, dmax], c=color)

    ax.plot([tAB, tAB], [0., d2], ls="--", c=color)
    ax.plot([tAB, tBC], [dBC, dBC], ls="--", c=color)
    ax.plot([tBC, tBC], [dBC, d2], ls="--", c=color)

    ax.text(
        (tmax + tmin)/2., (dmin + 0.)/2., '0',
        horizontalalignment="center",
        verticalalignment="center")

    ax.text(
        (t15 + tAB)/2., (0. + min(d2, dmax))/2., "1A",
        horizontalalignment="center",
        verticalalignment="center")

    ax.text(
        (tAB + tBC)/2., (dBC + min(d2, dmax))/2., "1B",
        horizontalalignment='center',
        verticalalignment='center')

    ax.text(
        (tBC + min(t12, tmax))/2., (0. + min(d2, dmax))/2., "1C",
        horizontalalignment="center",
        verticalalignment="center")

    if (dmax - dmin) < 25.:
        ax.text(
            (tAB + tBC)/2., (0. + min(dBC, dmax))/2., "1C",
            horizontalalignment="center",
            verticalalignment="center")

    ax.text(
        (t12 + tmax)/2., (0. + min(d1, dmax))/2., '2',
        horizontalalignment="center",
        verticalalignment="center")

    ax.text(
        (t12 + tmax)/2., (d1 + dmax)/2., '3',
        horizontalalignment="center",
        verticalalignment="center")

    if t15 > tmin:
        ax.text(
            (t15 + tmin)/2., (d1 + dmax)/2., '4',
            horizontalalignment="center",
            verticalalignment="center")

        ax.text(
            (t15 + tmin)/2, (0.+min(d1, dmax))/2., '5',
            horizontalalignment="center",
            verticalalignment="center")

    ax.axis(xmin=tmin, xmax=tmax, ymin=dmin, ymax=dmax)
    ax.set_ylabel("Residual Time ($t_{res}$) [ns]")
    ax.set_xlabel("Effective Distance ($d_{eff}$) [m]")

    axp = ax.twinx().twiny()
    axp.set_xlim(tmin, tmax)
    axp.set_ylim(dmin, dmax)

    tvals, tlabels = zip(*tlabels)
    axp.set_xticks(tvals)
    axp.set_xticklabels(tlabels)
    dvals, dlabels = zip(*dlabels)
    axp.set_yticks(dvals)
    axp.set_yticklabels(dlabels)


fig = plt.figure(figsize=(6.4, 9.6))

gridspec = plt.GridSpec(ncols=1, nrows=2, hspace=0.5)

ax1 = fig.add_subplot(gridspec[0, 0])

plot(
    ax1,
    tmin=-20.*sigma,
    tmax=40.*sigma,
    dmin=-1.*ice.scattering_length,
    dmax=8.*ice.scattering_length,
    tlabels=[
        (t12, r"$30\sigma$"),
        (t15, r"$-5\sigma$"),
        (t34, r"$\rho\sigma^{2}$")
        ],
    dlabels=[(0., 0.), (d1, r"$\lambda$"), (d2, r"$5\lambda$")])

ax2 = fig.add_subplot(gridspec[1, 0])

plot(
    ax2,
    tmin=-8.*sigma,
    tmax=20.*sigma,
    dmin=-0.05*ice.scattering_length,
    dmax=0.2*ice.scattering_length,
    tlabels=[
        (tAB, r"$\sigma(\sigma\rho - 1.35)$"),
        (tBC, r"$\sigma(\sigma\rho + \sqrt{200})$")
        ],
    dlabels=[(0., 0.), (dBC, r"$\frac{\lambda}{20}$")])

ax1.set_title(r"Regions of Pandel PDF for $\sigma = {}\,$ns".format(sigma))

gridspec.tight_layout(fig)

fig.savefig("pandel_regions.png")
