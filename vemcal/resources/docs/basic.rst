Basic Information About VEMCal Project
======================================

details about the files included in vemcal/private/vemcal, with corresponding libraries in vemcal/public/

pe
        photo-electron
VEM
        vertical equivalent muon


#. I3HGLGPairSelector.cxx
        #. Takes I3RecoPulseSeriesMap and returns frame object OutputPulseMask
        #. Output gives only paired DOMs for high gain and low gain comparisons
#. I3VEMCalData.cxx
        #. Grab HG and LG chip and channel information, also charge in pe
        #. Information specific to DOM for calibration use
        #. HGLGhits and MinBiasHits
#. I3VEMCalExtractor.cxx
        #. Pulls HG and LG information for separate frame object
        #. Also looks for local coincedence
#. I3VEMCalHistWriter.cxx
        #. Makes histograms for HG and LG hits
        #. HG and LG charge histos
        #. Time diff histos, charge diff histos
        #. Makes muon spectrum for HG DOM that was hit
#. I3VEMCalTreeWriter.cxx
        #. Root files for HG and LG trees, also VEM in HG DOM
        #. Includes DOM, string, charge in pe, time in ns



