Basic Information About VEMCalAnalyzer.cxx
==========================================

Here High Gain and Low Gain DOMs are compared for analysis.  These sets of hglg DOMs give and opportunity to determine what the conversion from PE to VEM is in each set of DOMs.   Using the two DOMs reduces a chance for error.

Listed below is each component of the VEMCalAnalyzer.cxx script, with a short explanation

hglgDiff(x_axis,profile_hist_params)

muonSpec(x_axis,profile_hist_params)
        Signal including background, contained signals, corner clippers

VEMCalAnalyzer::VEMCalAnalyzer(string,oudir_for_rootfiles)
        Get ahold of the OM database

VEMCalAnalyzer::OpenFiles(filelist)
        Look at histograms - deletes rootfiles
        Set start time for Run

VEMCalAnalyzer::AddMuonHist(omkey_hg,roothist_1D)
        Muon histograms from HG DOMs only

VEMCalAnalyzer::AddHGLGHist(omkey_hg,roothist_2D)

VEMCalAnalyzer::Analyze(outdir_for_rootfiles)
        FitMuonSpectrum
        FitHGLGCorrelation
        Does muon calibration for the amount of pe that constitutes a VEM in a particular DOM

VEMCalAnalyzer::Smooth(roothist_1D,bin_range)
        bin_range = 3
        Removes negative values, and smooths histogram

VEMCalAnalyzer::ExpStartValues(roothist_1D,x1,x2,norm,slope,offset)
        Calc start for histograms

VEMCalAnalyzer::FindPeakBin(roothist_1D,xmin,xmax,minimun_value,nsigma)
        Self explanatory

VEMCalAnalyzer::FitMuonSpectrum(roothist_1D,peak,width,,ratio_signal_bkg,Chi2)
        Smooth, FindPeakBin
        Except values of <25pe, fit with gaussian
        Find peak position, width and amplitude
        ExpStartValues
        Add fit functions to histograms
        1 VEM = 95% of peak value position
        GetChi2

VEMCalAnalyzer::ProfileX(roothist_2D)
        Take 2D histogram ad make a 1D weighted histogam

VEMCalAnalyzer::HGLGStartValues(roothist_1D,x1,x2,x0,y0,p2,p3)
        Calc mean start time from profile of 2D root histogram

VEMCalAnalyzer::FitHGLGCorrelation(roothist_2D,lgCorr,hgCorr,Chi2)
        Take HGLG histogram
        Find difference weighted with errors
        Look for best fit with chi2 values

VEMCalAnalyzer::Summarize(rootfile,VEMCalResultMap)
        All chi2, VEM, crossover printed to frame
        Check goodness fits

VEMCalAnalyzer::FillVEMHistograms(VEMCalResultMap,omkey,hg_minVEM,omkey,hg_maxVEM,omkey,hg_minVEM,omkey,hg_maxVEM)
        Histograms for pe to VEM conversion

VEMCalAnalyzer::FillCrossOverHistogram(VEMCalResultMap,omkey,min_crossover,omkey,max_crossover)
        Cross over for HG and LG DOMs

VEMCalAnalyzer::FillRChi2Histograms(VEMCalResultMap,omkey,min_chi2_muon,omkey,max_chi2_muon,omkey,min_chi2_hglg,omkey,max_chi2_hglg)

VEMCalAnalyzer::WriteXML(VEMCalResultMap,true_if_good)



