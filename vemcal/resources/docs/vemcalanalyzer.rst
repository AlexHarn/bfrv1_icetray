Basic Information About VEMCalAnalyzer.cxx
==========================================

Produces Muon Spectrum for HG DOMs, fits the spectrum, produces the VEM value for HG DOMs
Compares HG and LG DOMs in same tank, calibrates LG DOM wrt HG using the linear overlap region.

Listed below is each component of the VEMCalAnalyzer.cxx script, with a short explanation

hglgDiff(x_axis,profile_hist_params)
        HG/LG charge ratio function

muonSpec(x_axis,profile_hist_params)
        Muon spectrum function

VEMCalAnalyzer::VEMCalAnalyzer()

VEMCalAnalyzer::OpenFiles(filelist)

VEMCalAnalyzer::AddMuonHist(omkey_hg,roothist_1D)
        VEM histograms for plotting

VEMCalAnalyzer::AddHGLGHist(omkey_hg,roothist_2D)
        HGLG correlation for plotting

VEMCalAnalyzer::Analyze(outdir_for_rootfiles)
        FitMuonSpectrum
        FitHGLGCorrelation

VEMCalAnalyzer::Smooth(roothist_1D,bin_range)
        Removes negative values, and smooths histogram

VEMCalAnalyzer::ExpStartValues(roothist_1D,x1,x2,norm,slope,offset)
        Calc start for histograms

VEMCalAnalyzer::FindPeakBin(roothist_1D,xmin,xmax,minimun_value,nsigma)

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

VEMCalAnalyzer::FillCrossOverHistogram(VEMCalResultMap,omkey,min_crossover,omkey,max_crossover)

VEMCalAnalyzer::FillRChi2Histograms(VEMCalResultMap,omkey,min_chi2_muon,omkey,max_chi2_muon,omkey,min_chi2_hglg,omkey,max_chi2_hglg)

VEMCalAnalyzer::WriteXML(VEMCalResultMap,true_if_good)



