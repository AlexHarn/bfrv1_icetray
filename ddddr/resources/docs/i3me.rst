.. _i3me:

I3MuonEnergy module
-------------------

The I3MuonEnergy module performs the DDDDR algorithm on a given input track and with a given pulse map. 
The energy loss is calculated individually for each DOM within a perpendicular distance up to  maximum distance MaxImpact. The energy losses are then averaged over bins along the track 
length. The resulting binned energy loss distribution is then fitted with a model for the energy loss 
of a muon bundle, see :ref:`fitting`.

The results are stored in an I3MuonEnergyParams object. Optionally, the energy losses can be stored in the 
frame as well. In that case, vectors containing the slant depth and energy loss of the individual DOMs as well as vectors containing the bin centers of the slant depth and the vertical depth, energy loss and error on the energy loss of the bins along the track are stored.

Input Parameters
^^^^^^^^^^^^^^^^
This is an overview of the input parameters.

BadDomListName
    Optional for non-default baddomlist.

BinWidth
    The bin width in slant depth for the binned energy loss distribution that is used as basis for the further calculations.

FixB and FixGamma
    Fix the parameters B or Gamma in Tom Feusel's function.

I3MCTree and MMCTrackList
    Only used when a MC track is used as seed for the reconstruction. Can be used to supply non-default frame objects for I3MCTree and mmc track list.

IceModelFileName
    The path to the table containing the depth dependent data derived light attenuation that is used as parameter in the energy loss estimation. The table is a text file of zmin, zmax and light attenuation parameter.

InputPulses
    A pulse map or pulse mask that will be used to calculate the energy losses per bin.

LevelDist
    For distances smaller than LevelDist, the light yield at the DOMs is assumed to scale linearly with the distance to the track. See `the wiki <https://wiki.icecube.wisc.edu/index.php/IC79_Atmospheric_Muons/DDDDR#Application>`_ for more information.

MaxImpact
    Maximum perpendicular distance to the track up to which DOMs are considered for the energy loss distribution. 

Method
    If the energy profile is fitted, Method determins the fit function. 0 for exponential fit and 1 for Tom Feusel's muon bundle energy loss function.

Prefix
    The prefix for objects stored in the frame. By default, the algorithm stores two frame objects, PrefixParams and PrefixCascadeParams in the frame.

UseMonteCarloTrack
    Use the direction of the primary particle as seed track. In case this option is set to true, the parameters I3MCTree and MMCTrackList must be set as well. If the seed parameter is also set, a warning will be given and the Monte Carlo track is used.

PurityAdjust
    Scales the attenuation length with a linear function of the depth, positive values decrease the attenuation length. Default is 0. (leaves attenuation length unchanged).

SaveDomResults
    Save the energy loss measured by each DOM and the binned distribution as vectors in the frame.

Seed
    A track around which the energy loss will be calculated. The track can be any I3Particle resulting from a track reconstruction. Alternatively, the direction of the primary particle that created the muon bundle can be used in the case of simulated data. See :ref:`truemc` for more information.

.. _i3meparams:

I3MuonEnergyParams
^^^^^^^^^^^^^^^^^^

N, N_err
    Normalization constant of the fit to the energy loss distribution

b, b_err
    Slope of the fit to the energy loss distribution

gamma, gamma_err
    Exponent of the TomF function for the muon bundle energy loss.

nDoms
    Number of DOMs used to determine energy distribution

chi2, chi2ndof
    Chi square (per degree of freedom)

peak_energy
    Peak energy of the energy loss distribution

peak_sigma
    Uncertainty of the peak energy, calculated from the distribution of energy losses in the bin with the maximum energy loss

mean
    Mean of the energy loss distribution

median
    Median of the energy loss distribution

bin_width 
    Bin width of the energy loss distribution
    
.. _truemc:

Reconstruction of Monte Carlo data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to compare the reconstructed energy losses to the true energy losses using simulated data, the seed 
track can also be derived from simulated data. To approximate the track of the whole muon bundle, the direction
of the primary particle that created it is used as seed track. In that case, the bins for the energy loss 
distribution will be calculated the same way in both :ref:`i3trueme` and :ref:`i3me` and the reconstructed 
energy loss can be compared to the true energy loss bin by bin.
