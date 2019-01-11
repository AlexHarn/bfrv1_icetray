
Cross section
--------------

To set cross section, set name of cross section to I3NuGInteractionInfo. 
::

 tray.AddService("I3NuGInteractionInfoFactory", "interaction",
                  RandomService = random,
                  SteeringName = "steering",
                  CrossSectionModel = "csms"
               )

The available cross sections are 

* csms (recommended) 
  A.Cooper-Sarker, P.Mertsch, S.Sarkar, ''The high energy neutrino cross-section in the Standard Model and its uncertainty'' ([https://arxiv.org/abs/1106.3723 pdf])
  
* cteq5 (old cross section)
* css (not recommended)

Also, spline differential cross section can be used with I3NuGInteractionInfoDifferential class.
::

 tray.AddService("I3NuGInteractionInfoDifferentialFactory", "interaction",
                  RandomService = random,
                  SteeringName = "steering",
                  CrossSectionModel = "carlos"
               )

The available cross section is

* carlos's cross section
  C.A.Arguelles, F.Halzen, L.Wille, ''The High-Energy Behavior of Photon, Neutrino and Proton Cross Section'' ([https://arxiv.org/abs/1504.06639 pdf])


File format for the total cross section
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

I3NuGInteractionInfo
====================

Cross-section tables used with I3NuGInteractionInfo must be written in [mb]. A conversion factor from [mb] to [cm2] is applied in InteractionBase::GetXsecCGS().
The first line of total cross section file has 4 column. Here is the example of CSMS table:

111 1 1 12
2.00757e-11
2.78656e-11
3.80786e-11
5.13563e-11
.
.
.

The first number of the first line is number of bins in the table.
The second number is always 1, which represents 1-dim table.
The third number is lower energy limit in log10. The last number is upper energy limit in log10.

In this example, this table contains 111 bins from 10TeV to 10^12 TeV, and the first line of the 
table "2.00757e-11" [mb] represents cross section of 10GeV. 

This table is read by the CrosssectionTableReader class. The CrosssectionTableReader::EvaluateCrosssection() linearly interpolate between the binned values.

I3NuGInteractionDifferentialInfo
=================================

This class uses differential cross section written in binary files (suffix is .fits), developed for LeptonInjector. 
To generate .fits tables, use NuXSSplMkr (https://github.com/arguelles/NuXSSplMkr).

Adding a new cross section model
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
If you want to add a new model of cross section, first you have to prepare cross section tables that supports I3NuGInteractionInfo or I3NuGInteractionDifferentialInfo. 

Then, name the cross section model. For now, we use "myxsec" for example.

Next, put all generated tables in one directory at **neutrino-generator/cross_section_data/myxsec**, and generate **myxsec.list** file under **neutrino-generator/resources/cross_section_data/** .

The format of myxsec.list must be tablename and the inputfile. For all tablenames end with _xsec, set total cross section files(or .fits files). For tablenames end with _final, set final interaction tables or differential cross section fits files. The example shows an example of name of files and must be replaced to yours.

As long as the specified tables exist, you may mix several cross sections in one cross section model. In this example, we use tau decay table calculated by tauola with "myxsec" model.

    #
    # you may write comments here, start with #.
    #
    CC_NuBar_n_xsec    myxsec/sigma-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_NuBar_p_xsec    myxsec/sigma-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_NuBar_iso_xsec  myxsec/sigma-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_n_xsec    myxsec/sigma-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_p_xsec    myxsec/sigma-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_iso_xsec  myxsec/sigma-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_n_xsec       myxsec/sigma-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_p_xsec       myxsec/sigma-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_iso_xsec     myxsec/sigma-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_n_xsec       myxsec/sigma-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_p_xsec       myxsec/sigma-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_iso_xsec     myxsec/sigma-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    CC_NuBar_n_final   myxsec/dsdxdy-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_NuBar_p_final   myxsec/dsdxdy-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_NuBar_iso_final myxsec/dsdxdy-numubar-N-cc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_n_final   myxsec/dsdxdy-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_p_final   myxsec/dsdxdy-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_NuBar_iso_final myxsec/dsdxdy-numubar-N-nc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_n_final      myxsec/dsdxdy-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_p_final      myxsec/dsdxdy-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    CC_Nu_iso_final    myxsec/dsdxdy-numu-N-cc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_n_final      myxsec/dsdxdy-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_p_final      myxsec/dsdxdy-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    NC_Nu_iso_final    myxsec/dsdxdy-numu-N-nc-HERAPDF15NLO_EIG_central.fits
    Tau_Decay          decay/tau_decay_tauola.dat

 


Note about Iso-scalar target
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you use I3NuGInteractionInfo, the total cross section of neutrino doesn't change whatever the target material is.

This is not correct, because ice (:math:`H_{2}O`) has more protons than neutron, while rock (:math:`SiO_{2}`) has same amount of protons and neutrons.</font>

I3NuGInteractionInfo service uses cross section table that is based on ''iso-scalar parton model'', a hypothetical particle that composed from 

.. math::
 \frac{1}{2} ~ proton + \frac{1}{2} ~ neutron

This approximation works well for :math:`SiO_{2}` but not for :math:`H_{2}O` for low energy particle.
The difference of cross section will be around 4% at low energy, and will be negligible at high energy regime.

See following documents for details.

* A. Heijboer, ''Track reconstruction and point source searches with ANTARES'', PhD thesis, 2004, p39. ([http://cdsweb.cern.ch/record/798707 pdf])
* T. Montaruli and I. Sokalski "Influence of neutrino interaction and muon propagation media on neutrino-induced muln rates in deep underwater detectors'', internal report of ANTARE'', 2003, p5. ([http://www.icecube.wisc.edu/~hoshina/docs/sea.pdf pdf])

For full-support of different cross sections between ice and rock, we need code update of NuGen.




