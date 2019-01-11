.. _cmc:

cmc - Cascade Monte Carlo
=========================

The cmc (Cascade Monte Carlo) module simulates the development of
electro-magnetic showers in the ice.

This module should be used after the neutrino-generator in case of
electron neutrion samples. If you want to treat cascades from muon
interactions, put into the MC tree by mmc, you need to run this module
after PROPOSAL.  But if you want to look at hadronic cascades and use
this module for generating muons, you have to run PROPOSAL after it.  

The main documentation can be found here: http://wiki.icecube.wisc.edu/index.php/Cascade_Simulation

.. toctree::
   :maxdepth: 3
   
   release_notes
