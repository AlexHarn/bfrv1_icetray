.. _stochastics:

Stochastics
***********

:Authors:
  S. De Ridder <sam.deridder@ugent.be>, T. Feusels <feusels.tom@gmail.com>

.. toctree::
   :maxdepth: 1

   release_notes
   code_review

Description
===========

This (simple) project is used for the further analysis of muon bundle energy loss reconstruction and is used in cosmic ray analyses. It takes a vector of energy losses along the muon bundle track, reconstructed by millipede, as input. It fits an expected energy loss function to these losses, using the ROOT TMinuit class (for now). This formula is based on the Elbert formula. The fit is evaluated at several slant depths, and thus parameters such Eloss_1500, etc are present in the output.
Furthermore, it selects a certain number of high energy losses, or HE stochastics, which stand out a certain fraction above the energy loss fit. Two selection type curves are implemented, and their parameters can be set through the A_param, B_param and C_param parameters. Certain parameters of these selections are calculated, and all the output is stored as an I3Stochastic object.
The same procedure is applied to the energy loss profile, except some selected extreme energy losses. This information is stored in another I3Stochastic object specified by OutputName_red.

It has been proved that the reconstructed energy loss (evaluated at a certain depth) correlates very well with the number of muons in the bundle. It is thus sensitive to the CR primary composition. The selection of the HE stochastics provides some extra sensitivity to the composition. However, a data-MC discrepancy has been observed in these parameters, so these should be studied further first.  A wiki page adding some plots to this description can be found here: https://wiki.icecube.wisc.edu/index.php/I3Stochastics

At this moment ROOT is needed to run it, but this should be changed in the future.

I3tochastics
============

The I3Stochastics module is the only C++ module in this project, which does all the work. Its parameters are:

::

  -InputParticleVector         [string, DEFAULT="EnergyLosses"] Vector of I3Particles containing the reconstructed energy losses.
  -OutputName                  [string, DEFAULT="Stochastics"] The name of the output I3EnergyLoss object.
  -OutputName_red              [string, DEFAULT="Stochastics"] The name of the output I3EnergyLoss object containing the fit information after fitting the profile without the subtracted, selected extreme energy losses.
  -A_param                     [double, DEFAULT=0] A parameter of the stochastics selection procedure.
  -B_param                     [double, DEFAULT=0] B parameter of the stochastics selection procedure.
  -C_param                     [double, DEFAULT=0] C parameter of the stochastics selection procedure.
  -SelectionType               [string, DEFAULT="Type1"] Stochastics selection types: Type1 : <math> a < dE/dX > + b \cdot Fit^c </math> , Type2 : <math> a +b \cdot Fit^c </math>.
  -FreeParams                  [bitmask, DEFAULT=1] which parameters should be free and which should be ﬁxed in a bitset : [b,a,γµ, kappa, A,E0], set with an int. RECOMMENDED : only one free param, preferable ﬁrst.
  -Minimizer                   [string, DEFAULT="MIGRAD"] Use MIGRAD, SIMPLEX or both (MINIMIZE). The module uses the TMinuit class for fitting.
  -MaxIterations               [int, DEFAULT=1500] Maximum number of iterations for the minimizer.
  -Tolerance                   [double, DEFAULT=0.001] Tolerance for convergence of the minimizer.
  -Verbose                     [bool, DEFAULT=false] Output all details of the TMinuit fitting procedure or not.

PlotStoch example
=================

This I3Module gives an example of how to plot the input energy losses (specified by InputElosses), together with the fits (InputFitResult and InputFitResultRed) for a certain number of events (NEvents). The resulting plots can be saved in a .ps file (OutputFileName) and/or ROOT file (RootOutputFileName).
