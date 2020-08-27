# -*- coding: utf-8 -*-


def plot_bracket(x, y, yerr, xerr=None, capsize=3, axes=None, **kwargs):
  """
  Plot brackets to indicate errors.

  Parameters
  ----------
  x,y,yerr,xerr: central value and errors
  markersize: size of the bracket
  capsize: length of the tips of the bracket
  """

  import numpy as np
  import matplotlib as mpl

  if axes is None:
    axes = mpl.pyplot.gca()

  for k in ("mec", "markeredgecolor", "marker", "m"):
    if k in kwargs:
      raise ValueError("keyword %s not allowed" % k)

  col = "k"
  for k in ("c", "color"):
    if k in kwargs:
      col = kwargs[k]
      del kwargs[k]

  x = np.atleast_1d(x)
  y = np.atleast_1d(y)

  if yerr is not None:
    yerr = np.atleast_1d(yerr)
    if yerr.shape[0] == 1:
      yd = y - yerr
      yu = y + yerr
    elif yerr.shape[0] == 2:
      yd = y - yerr[0]
      yu = y + yerr[1]
    else:
      raise ValueError("yerr has unexpected shape")

    dx = 0.01
    dy = 0.02
    t = 2 * dy * capsize
    w = 0.5
    m1 = ((-w - dx, t + dy), (-w - dx, -dy), (w + dx, -dy), (w + dx, t + dy),
          (w - dx, t + dy), (w - dx, dy), (-w + dx, dy), (-w + dx, t + dy))
    m2 = ((-w - dx, -t - dy), (-w - dx, dy), (w + dx, dy), (w + dx, -t - dy),
          (w - dx, -t - dy), (w - dx, -dy), (-w + dx, -dy), (-w + dx, -t - dy))
    axes.plot(x, yd, marker=m1, color=col, mec=col, **kwargs)
    axes.plot(x, yu, marker=m2, color=col, mec=col, **kwargs)

  if xerr is not None:
    xerr = np.atleast_1d(xerr)
    if xerr.shape[0] == 1:
      xd = x - xerr
      xu = x + xerr
    elif xerr.shape[0] == 2:
      xd = x - xerr[0]
      xu = x + xerr[1]
    else:
      raise ValueError("xerr has unexpected shape")

    dx = 0.02
    dy = 0.01
    t = 2 * dx * capsize
    h = 0.5
    m1 = ((t + dx, -h - dy), (-dx, -h - dy), (-dx, h + dy), (t + dx, h + dy),
          (t + dx, h - dy), (dx, h - dy), (dx, -h + dy), (t + dx, -h + dy))
    m2 = ((-t - dx, -h - dy), (dx, -h - dy), (dx, h + dy), (-t - dx, h + dy),
          (-t - dx, h - dy), (-dx, h - dy), (-dx, -h + dy), (-t - dx, -h + dy))
    axes.plot(xd, y, marker=m1, color=col, mec=col, **kwargs)
    axes.plot(xu, y, marker=m2, color=col, mec=col, **kwargs)


def plot_hist(xedges, ws, livetime,axes=None, **kwargs):
  """
  Plots histogram data in ROOT style.

  Parameters
  ----------
  xedge: lower bin boundaries + upper boundary of last bin
  ws: content of the bins
  facecolor: a matplotlib color definition to fill the histogram
  axes: the axes to draw on (defaults to the current axes)
  """

  if axes is None:
    from matplotlib import pyplot as plt
    axes = plt.gca()

  import numpy as np

  m = len(ws)
  n = 2 * m + 2

  xy = np.zeros((2, n))

  xy[0][0] = xedges[0]
  xy[0][-1] = xedges[-1]

  for i in range(m):
    xy[0][1 + 2 * i] = xedges[i]
    xy[1][1 + 2 * i] = ws[i]/livetime
    xy[0][1 + 2 * i + 1] = xedges[i + 1]
    xy[1][1 + 2 * i + 1] = ws[i]/livetime

  if "fc" in kwargs:
    kwargs["facecolor"] = kwargs["fc"]
    del kwargs["fc"]
  if "c" in kwargs:
    kwargs["color"] = kwargs["c"]
    del kwargs["c"]

  if "facecolor" in kwargs:
    if "color" in kwargs:
      kwargs["edgecolor"] = kwargs["color"]
      del kwargs["color"]
    if "label" in kwargs:
      # label hack
      from matplotlib.patches import Rectangle
      r = Rectangle((0, 0), 0, 0, **kwargs)
      axes.add_patch(r)
    return axes.fill_between(xy[0], 0, xy[1], **kwargs)
  else:
    return axes.plot(xy[0], xy[1], **kwargs)


def plot_boxerrors(xedges, ys, yes, axes=None, **kwargs):
  """
  Plots error boxes for a histogram (recommended way to show
  systematic uncertainties).

  Parameters
  ----------
  xedge: array of floats
    Lower bin boundaries + upper boundary of last bin as returned
    by numpy.histogram.
  ys: array of floats
    Center of the box.
  yes: array of floats
    Distance of the edge of the box from the center. Maybe one-dimensional
    for symmetric boxes or two-dimensional for asymmetric boxes.
  axes: Axes (optional, default: None)
    The axes to draw on (defaults to the current axes).

  Optional keyword arguments are forwarded to the matplotlib.patch.Rectangle
  objects. Useful keywords are: facecolor, edgecolor, alpha, zorder.

  Authors
  -------
  Hans Dembinski <hans.dembinski@kit.edu>
  """

  import numpy as np
  from matplotlib.patches import Rectangle

  if axes is None:
    from matplotlib import pyplot as plt
    axes = plt.gca()

  xedges = np.atleast_1d(xedges)
  ys = np.atleast_1d(ys)
  yes = np.atleast_1d(yes)

  n = len(ys)
  isAsymmetric = len(yes.shape) == 2
  rs = []
  for i in xrange(n):
    x0 = xedges[i]
    y0 = ys[i] - yes[i][0] if isAsymmetric else ys[i] - yes[i]
    xw = xedges[i + 1] - xedges[i]
    yw = yes[i][0] + yes[i][1] if isAsymmetric else 2 * yes[i]
    if yw > 0:
      r = Rectangle((x0, y0), xw, yw, **kwargs)
      rs.append(r)
      axes.add_artist(r)
  return rs


def cornertext(text, loc=2, color=None, frameon=False,
               axes=None, **kwargs):
  """
  Conveniently places text in a corner of a plot.

  Parameters
  ----------
  text: string or tuple of strings
    Text to be placed in the plot. May be a tuple of strings to get
    several lines of text.
  loc: integer or string
    Location of text, same as in legend(...).
  frameon: boolean (optional)
    Whether to draw a border around the text. Default is False.
  axes: Axes (optional, default: None)
    Axes object which houses the text (defaults to the current axes).
  fontproperties: matplotlib.font_manager.FontProperties object
    Change the font style.

  Other keyword arguments are forwarded to the text instance.

  Authors
  -------
  Hans Dembinski <hans.dembinski@kit.edu>
  """

  from matplotlib.offsetbox import AnchoredOffsetbox, VPacker, TextArea
  from matplotlib import rcParams
  from matplotlib.font_manager import FontProperties
  import warnings

  if axes is None:
    from matplotlib import pyplot as plt
    axes = plt.gca()

  locTranslate = {
    'upper right': 1,
    'upper left': 2,
    'lower left': 3,
    'lower right': 4,
    'right': 5,
    'center left': 6,
    'center right': 7,
    'lower center': 8,
    'upper center': 9,
    'center': 10
  }

  if isinstance(loc, str):
    if loc in locTranslate:
      loc = locTranslate[loc]
    else:
      message = 'Unrecognized location "%s". Falling back on "upper left"; valid locations are\n\t%s' \
                % (loc, '\n\t'.join(locTranslate.keys()))
      warnings.warn(message)
      loc = 2

  if "borderpad" in kwargs:
    borderpad = kwargs["borderpad"]
  else:
    borderpad = rcParams["legend.borderpad"]

  if "borderaxespad" in kwargs:
    borderaxespad = kwargs["borderaxespad"]
  else:
    borderaxespad = rcParams["legend.borderaxespad"]

  if "handletextpad" in kwargs:
    handletextpad = kwargs["handletextpad"]
  else:
    handletextpad = rcParams["legend.handletextpad"]

  if "fontproperties" in kwargs:
    fontproperties = kwargs["fontproperties"]
    del kwargs["fontproperties"]
  else:
    if "size" in kwargs:
      size = kwargs["size"]
      del kwargs["size"]
    elif "fontsize" in kwargs:
      size = kwargs["fontsize"]
      del kwargs["fontsize"]
    else:
      size = rcParams["legend.fontsize"]
    fontproperties = FontProperties(size=size)

  texts = [text] if isinstance(text, str) else text

  colors = [color for t in texts] if (isinstance(color, str) or color is None) else color

  tas = []
  for t, c in zip(texts, colors):
    ta = TextArea(t,
                  textprops={"color": c, "fontproperties": fontproperties},
                  multilinebaseline=True,
                  minimumdescent=True,
                  **kwargs)
    tas.append(ta)

  vpack = VPacker(children=tas, pad=0, sep=handletextpad)

  aob = AnchoredOffsetbox(loc, child=vpack,
                          pad=borderpad,
                          borderpad=borderaxespad,
                          frameon=frameon)

  axes.add_artist(aob)
  return aob


def uncertainty_ellipse(par, cov, axes=None, **kwargs):
  """
  Draws a 2D uncertainty ellipse.

  Parameters
  ----------
  par: array-like
    The parameter vector.
  cov: array-like
    The covariance matrix.
  axes: Axes (optional, default: None)
    The axes to draw on (defaults to the current axes).
  Other keyword-based arguments may be given, which are forwarded to
  the ellipse object.

  Returns
  -------
  An ellipse patch.

  Authors
  -------
  Hans Dembinski <hans.dembinski@kit.edu>
  """
  import numpy as np
  from math import atan2, pi, sqrt
  from matplotlib.patches import Ellipse

  if axes is None:
    from matplotlib import pyplot as plt
    axes = plt.gca()

  u, s, v = np.linalg.svd(cov)

  coverage_factor = 1.51

  angle = atan2(u[1, 0], u[0, 0]) * 180.0 / pi
  s0 = coverage_factor * sqrt(s[0])
  s1 = coverage_factor * sqrt(s[1])

  ellipse = Ellipse(xy=par, width=2.0 * s0, height=2.0 * s1,
                    angle=angle, **kwargs)
  axes.add_patch(ellipse)

  return ellipse


def ViolinPlot(x, y, bins=10, range=None, meancolor="r", mediancolor="b",
               violincolor="y", axes=None, **kwargs):
  """
  Draws a violin (kernel density estimate) plot with mean and median profiles;
  Adapted from http://pyinsci.blogspot.de/2009/09/violin-plot-with-matplotlib.html.

  Parameters
  ----------
  x: array of type float
    Data dimension to bin in
  y: array of type float
    Data to bin and profile
  bins: int or array of type float or None
    Number of bins or array of bin edges
    If None: Take x as bin centers and y as already binned values
  range (optional):
    The range of x used for binning

  meancolor:
    String representing the colour used for drawing mean,
    to disable drawing, set to ""
  mediancolor:
    String representing the colour used for drawing median,
    to disable drawing, set to ""
  violincolor:
    String representing the colour used for drawing violins,
    to disable drawing, set to ""

  axes: Axes (optional, default: None)
    The axes to draw on (defaults to the current axes)

  Other keyword-based arguments may be given, which are forwarded to
  the individual plot/errorbar/fill_between calls.

  Authors
  -------
  Alexander Schulz <alexander.schulz@kit.edu>
  """

  from matplotlib import pyplot as plt
  from matplotlib.patches import Rectangle
  if axes is None:
    axes = plt.gca()
  else:
    plt.sca(axes)

  from scipy.stats import gaussian_kde
  from pyik.numpyext import bin, centers, mad
  import numpy as np

  # binning in x
  if bins is not None:
    ybins, xedgs = bin(x, y, bins=bins, range=range)
    xcens, xhws = centers(xedgs)
  else:
    xcens = x
    xhws = (x[1:] - x[:-1]) / 2.
    xhws = np.append(xhws, xhws[-1])
    ybins = np.atleast_2d(y)

  l = len(ybins)
  means, stds, meds, mads, ns = np.zeros(l), np.zeros(l), np.zeros(l), \
      np.zeros(l), np.zeros(l)

  for i, ybin in enumerate(ybins):

    means[i] = np.mean(ybin)
    stds[i] = np.std(ybin)
    meds[i] = np.median(ybin)
    mads[i] = mad(ybin)
    ns[i] = len(ybin)

    if len(ybin) > 1:

      k = gaussian_kde(ybin)            # calculates the kernel density
      m = k.dataset.min()               # lower bound of violin
      M = k.dataset.max()               # upper bound of violin

      y = np.arange(m, M, (M - m) / 200.)   # support for violin

      v = k.evaluate(y)                 # violin profile (density curve)
      v = v / v.max() * xhws[i] / 2.          # scaling the violin to the available space

      # drawing the violines
      if violincolor != "":
        plt.fill_betweenx(y, xcens[i] - v, v + xcens[i], facecolor=violincolor, alpha=0.3,
                          edgecolor=violincolor, zorder=0)

      # median lines with "mad" boxes
      if mediancolor != "":
        plt.plot((xcens[i] - xhws[i] / 10., xcens[i] + xhws[i] / 10.), (meds[i], meds[i]), ls="-",
                 color=mediancolor, alpha=0.8, zorder=3, **kwargs)

        r = Rectangle((xcens[i] - xhws[i] / 10., meds[i] - mads[i]), xhws[i] / 5., 2 * mads[i],
                      color=mediancolor, alpha=0.2, zorder=3)
        plt.gca().add_artist(r)

  # Mean profile
  if meancolor != "":
    merrbar = plt.errorbar(xcens, means, stds / np.sqrt(ns), marker="o", ls="o",
                           elinewidth=2, mew=0, color=meancolor, zorder=1, **kwargs)
    # matplotlib is messing up the zorder for me if not explicitly told what to do
    for el in merrbar[2]:
      el.set_zorder(1)

  # to bring all the violins into visible x-range
  plt.xlim(min(xcens - xhws), max(xcens + xhws))

  return plt.gca()


if __name__ == "__main__":
  import doctest
  doctest.testmod()
