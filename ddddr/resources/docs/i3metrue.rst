.. _i3trueme:

I3TrueMuonEnergy module
-----------------------

In standard IceCube simulations, detailed muon propagation and their energy losses are saved for a cylindrical simulation volume around the detector.
Stochastic energy losses up to an energy E_cut, defined during simulation, are considered continuous and not saved explicitly.
The I3TrueMuonEnergy module divides the path of the muon bundle from its entrance into the simulation volume until either its end or its exit of the simulation volume into equally sized slant depth bins of a user-defined bin width.
The stochastic energy losses are summed up for each bin, while the continuous energy losses for each muon are estimated by the difference
between final muon energy and the sum of stochastic losses for this muon.

The user can choose to either fit the total muon bundle energy as a function of the slant depth or the differential 
energy loss per bin. In the first case, the total muon energy as well as the bin centers of the slant depth are stored 
as vectors in the frame. If the energy loss is fit, the differential energy loss per bin is saved as well.

The fit results are stored as :ref:`i3meparams` object.

Input Parameters
^^^^^^^^^^^^^^^^

BinWidth
    Size of the slant depth bins into which the track is divided. 

SlantBinNo
    Max. number of slant depth bins. If the actual track length of the muon bundle is shorter than
    BinWidth*SlantBinNo, the number of bins will be calculated from the bin width.

EnergyLoss
    If set to true, the differential energy loss will be fit and saved in the frame.
