/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file VEMCalAnalyzer.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 * 
 * Removed I3Db dependency  --S. Tilav 24/Sep/2019
 */


#include <vemcal/VEMCalAnalyzer.h>
#include <TFile.h>
#include <TClass.h>
#include <TKey.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TMath.h>
#include <TLine.h>
#include <TArrow.h>
#include <TText.h>
#include <TPaveText.h>

#include <TROOT.h>

#include <iostream>
#include <ostream>
#include <fstream>
#include <cstring>

#include <algorithm>


using namespace std;


double hglgDiff(double* x, double* p)
{
    double ret = p[0];
    if(x[0]>p[1]) ret = p[0] + p[3]*log10(1 + p[2]*(x[0] - p[1]));
    return ret;
}


double muonSpec(double* x, double* p)
{
  // Signal of muons which go through top and bottom of the ice
  double sig1 = p[0]*TMath::Landau(x[0], p[1], p[2], true);

  // Signal contribution of edge clipping muons
  // Ratio between edge clippers and muons which go through top and bottom is about 1.85
  // This comes from a pure geometrical simulation (TankToySim) performed during
  // the master thesis of Michael Beimforde (DESY Zeuthen, 2007)
  double sig2 = p[3]*(1.85*p[0]/p[1])/(exp((x[0] - p[1])/p[2]) + 1.0); // (fermi function)
  
  // Electromagnetic background
  double bkg  = p[4]*exp(x[0]*p[5]);
  
  return bkg + sig1 + sig2;
}


VEMCalAnalyzer::VEMCalAnalyzer():
    firstRunID_(-1), lastRunID_(-1), startTime_(0,0), storeHistos_(true)
{
  
}

VEMCalAnalyzer::~VEMCalAnalyzer()
{

}


void VEMCalAnalyzer::OpenFiles(const std::vector<std::string>& fileList)
{
    firstRunID_ = -1;
    lastRunID_  = -1;
    startTime_.SetDaqTime(0,0);
    
    std::vector<std::string>::const_iterator iter;
    for(iter = fileList.begin(); iter != fileList.end(); ++iter)
    {
	// Open file
	TFile* rootfile = new TFile(iter->c_str(), "READ");
	if(rootfile->IsZombie() || !rootfile->IsOpen())
	{
	    log_error("Couldn't open file %s", iter->c_str());
	    delete rootfile;
	    continue;
	}
	log_info("Opening file %s", iter->c_str());
	
	// Extract filename from path
	std::string fileName = *iter;
	fileName.erase(0, fileName.find_last_of("/") + 1);
	
	// Retrieve runID
	int runID;
	if(sscanf(fileName.c_str(),"IceTop_VEMCalData_Run%8d", &runID)==1)
	{
	    if(firstRunID_<0)
	    {
		firstRunID_ = runID;
		lastRunID_  = runID;
	    }
	    else
	    {
	        firstRunID_ = std::min(firstRunID_, runID);
	        lastRunID_  = std::max(lastRunID_, runID);
	    }
	}
	else
	{
	    log_warn("Failed retrieve runID from file %s. Skipping it!", iter->c_str());
	    delete rootfile;
	    continue;
	}
	
	// Loop over all Histogramms in the top level directory of the file
	int muonCounter = 0;
	int hglgCounter = 0;

        TIter next(rootfile->GetListOfKeys()); 
        TKey *key; 
        while ((key=(TKey*)next())) {
	    
	    int str, om;
	    char prefix[9];
	    if(sscanf(key->GetName(),"%8s_%2d_%2d*s", prefix, &str, &om) != 3) continue;
	    
	    OMKey hgKey(str, om);
	    TObject* obj = key->ReadObj();
	    if(std::strncmp(prefix, "muonSpec", 8)==0)
	    {
		if(obj->IsA()->InheritsFrom("TH1F"))
		{
		    AddMuonHist(hgKey, static_cast<TH1F*>(obj));
		    muonCounter++;
		}
		else
		{
		    log_error("  Skipping object %s which does not inherit from TH1F!", obj->GetName());
		}
	    }
	    
	    if(std::strncmp(prefix, "hglgDiff", 8)==0)
	    {
		if(obj->IsA()->InheritsFrom("TH2F"))
		{
		    AddHGLGHist(hgKey, static_cast<TH2F*>(obj));
		    hglgCounter++;
		}
		else
		{
		    log_error("  Skipping object %s which does not inherit from TH2F!", obj->GetName());
		}
	    }
	    
	    delete obj;

	}

	delete rootfile;

	log_info("  Added %d muon histograms.", muonCounter);
	log_info("  Added %d hglg histograms.", hglgCounter);
    }
    
    log_info("First run : %d", firstRunID_);
    log_info("Last  run : %d", lastRunID_);
    log_info("Start time: %s", startTime_.GetUTCString().c_str());
}



void VEMCalAnalyzer::AddMuonHist(const OMKey& hgKey, TH1F* hist)
{
    hist->UseCurrentStyle();
    MuonHistMap::const_iterator iter = muonHistos_.find(hgKey);
    if(iter!=muonHistos_.end())
    {
	iter->second->Add(hist);
    }
    else
    {
	TH1F* clone = static_cast<TH1F*>(hist->Clone());
	clone->SetDirectory(0);
	clone->SetFillColor(17);
	muonHistos_[hgKey] = clone;
    }
}


void VEMCalAnalyzer::AddHGLGHist(const OMKey& hgKey, TH2F* hist)
{
    hist->UseCurrentStyle();
    HGLGHistMap::const_iterator iter = hglgHistos_.find(hgKey);
    if(iter!=hglgHistos_.end())
    {
	iter->second->Add(hist);
    }
    else
    {
	TH2F* clone = static_cast<TH2F*>(hist->Clone());
	clone->SetDirectory(0);
	hglgHistos_[hgKey] = clone;
    }
}


void VEMCalAnalyzer::Analyze(const std::string& outDir)
{
    printf("Calibrating %zu tanks ...\n", muonHistos_.size());
    
    TFile* rootFile = NULL;
    if(storeHistos_ && firstRunID_ > 0)
    {
    std::ostringstream rootFileName;
    rootFileName << "VEM_calibration_" << startTime_.GetUTCString("%Y-%m-%d_%H%M%S") << ".root";
	rootFile = new TFile(rootFileName.str().c_str(), "RECREATE");
    }
    
    bool isGood = true;
    VEMCalResultMap results;
    MuonHistMap::iterator muon_iter;
    HGLGHistMap::iterator hglg_iter;
    for(muon_iter=muonHistos_.begin(); muon_iter!=muonHistos_.end(); ++muon_iter)
    {
	const OMKey& hgKey = muon_iter->first;
	
	VEMCalResult hgResult;
	hgResult.gainType = 0;
	hgResult.muonFitStatus  = FitMuonSpectrum(muon_iter->second,
						  hgResult.pePerVEM,
						  hgResult.muonWidth,
						  hgResult.sbRatio,
						  hgResult.muonFitRchi2);
	
	// Write fitted histogram to rootfile
	muon_iter->second->Write();
	
	// Find corresponding HGLG difference plot
	hglg_iter = hglgHistos_.find(hgKey);
	if(hglg_iter!=hglgHistos_.end())
	{
	    // Retrieve LG key from histogram name
	    int lg_om;                             
	    if(sscanf(hglg_iter->second->GetName(),"hglgDiff_%*2d_%*2d_%2d", &lg_om) == 1)
	    {
		// Create LG key
	        OMKey lgKey(hgKey.GetString(), lg_om);
		
		// Fit HGLG difference plot
		double lgCorr = NAN;
		hgResult.hglgFitStatus = FitHGLGCorrelation(hglg_iter->second,
							    lgCorr,
							    hgResult.hglgCrossOver,
							    hgResult.hglgFitRchi2);
		// Write fitted histogram to rootfile
		hglg_iter->second->Write();
		
		VEMCalResult lgResult;
		lgResult.gainType  = 1;
		lgResult.pePerVEM  = hgResult.pePerVEM/lgCorr;
		lgResult.muonWidth = hgResult.muonWidth;
		lgResult.sbRatio   = hgResult.sbRatio;
		results[lgKey] = lgResult;
	    }
	    else
	    {
	        log_error("Failed to retrieve LG key from histogram name \"%s\"!", hglg_iter->second->GetName());
		isGood = false;
	    }
	}
	
	results[hgKey] = hgResult;
    }
    
    if(!Summarize(rootFile, results)) 
      isGood = false;    

rootFile->Close();


    // Close root file
    //if(rootFile) delete rootFile;
    
    if(isGood) printf("Muon calibration succeeded!\n");
    else       printf("Muon calibration failed!\n");
    
    // Write calibration results to xml file
    WriteXML(results, isGood);

}


TH1F* VEMCalAnalyzer::Smooth(TH1F* hist, int binRange)
{
    // Smooth histogram by calculating the sliding average
  
    stringstream savName;
    savName << hist->GetName() << "_smoothed";
    
    TH1F* savHist = new TH1F(savName.str().c_str(),
			     hist->GetTitle(),
			     hist->GetNbinsX(),
			     hist->GetXaxis()->GetXmin(),
			     hist->GetXaxis()->GetXmax());
    
    for(int bin=hist->GetNbinsX()-binRange; bin>=1+binRange; bin--)
    {
        double sav_val = 0;
	int num = 0;
	for(int i=bin-binRange; i<=bin+binRange; i++)
	{
	    double y = hist->GetBinContent(i);
	    if(y<=0) continue;
	    
	    sav_val += y;
	    num++;
	}
	
	if(num>0)
	{
	    sav_val = sav_val/num;
	      
	    savHist->SetBinContent(bin, sav_val);
	    savHist->SetBinError(bin, sqrt(sav_val/num));
	}
    }
    
    return savHist;
}


void VEMCalAnalyzer::ExpStartValues(TH1F* hist, double x1, double x2, double& norm, double& slope, double offset)
{
    int num = 0;
    double mean_x  = 0;
    double mean_y  = 0;
    double mean_xy = 0;
    double mean_xx = 0; 
    
    int start_bin = hist->GetXaxis()->FindBin(x1);
    int stop_bin  = hist->GetXaxis()->FindBin(x2);
    for(int i=start_bin; i<=stop_bin; i++)
    {
	double x = hist->GetBinCenter(i);
	double y = hist->GetBinContent(i) - offset;
	
	if(y>0)
	{
	    y = log(y);
	    mean_x  += x;
	    mean_y  += y;
	    mean_xy += x*y;
	    mean_xx += x*x;
	    num++;
	}
    }
    
    mean_x  /= num;
    mean_y  /= num;
    mean_xy /= num;
    mean_xx /= num;
    
    slope = (mean_y*mean_x - mean_xy)/(mean_x*mean_x - mean_xx);
    norm  = exp(mean_y - slope*mean_x);
}


int VEMCalAnalyzer::FindPeakBin(TH1F* hist, double xmin, double xmax, double minval, int nsigma)
{
    int min_bin = hist->GetXaxis()->FindBin(xmin);
    int max_bin = hist->GetXaxis()->FindBin(xmax);
    
    int peak_bin   = 0;
    double max_val = 0;
    double max_err = 0;
    for(int bin=max_bin; bin>=min_bin; bin--)
    {
        double y_val = hist->GetBinContent(bin);
	double y_err = hist->GetBinError(bin);
	if(y_val <= minval) continue;
	
	double delta_val = max_val - y_val;
	double delta_err = sqrt(max_err*max_err + y_err*y_err);
	if(delta_val > nsigma*delta_err) break;
	
	if(y_val > max_val)
	{
	    max_val  = y_val;
	    max_err  = y_err;
	    peak_bin = bin;
	}
    }
    
    return peak_bin;
}



int VEMCalAnalyzer::FitMuonSpectrum(TH1F* hist, double& peak, double& width, double& sbRatio, double& rChi2)
{
    // Determine maximum x value for prefits (97% of total sum) 
    double integral = 0;
    int max_bin = hist->GetNbinsX();
    for(int i=1; i<=max_bin; i++) integral += hist->GetBinContent(i);
    
    double sumBins=integral;
    while(sumBins > 0.97*integral)
    {
        sumBins -= hist->GetBinContent(max_bin);
	max_bin--;
    }
    double xmax = hist->GetXaxis()->GetBinLowEdge(max_bin);
    
    
    // Smooth muon spectrum
    TH1F* smoothHist = Smooth(hist,3);
    
    // Find first guess muon peak position and amplitude
    int peak_bin = FindPeakBin(smoothHist, 0, xmax, 0, 3);
    double peak_amp = smoothHist->GetBinContent(peak_bin);
    double peak_pos = smoothHist->GetBinCenter(peak_bin);
    
    
    // Make a Gaussian pre-fit of the peak
    TF1* gaus = new TF1("gaus","gaus", 0.8*peak_pos, 1.4*peak_pos);
    hist->Fit(gaus,"QNR","GOFF");
    peak_amp = gaus->GetParameter(0);
    peak_pos = gaus->GetParameter(1);
    double peak_width = gaus->GetParameter(2);
    delete gaus;
    
    // Determine fit range (do not allow values below 25 pe)
    int thr_bin = FindPeakBin(hist, 25, 0.5*(25 + peak_pos), hist->GetBinContent(peak_bin), 3);
    double xmin = smoothHist->GetBinLowEdge(thr_bin+5);
    xmax = 2.2*peak_pos;
    
    smoothHist->SetLineColor(2);
    smoothHist->SetOption("SAME");
    hist->GetListOfFunctions()->Add(smoothHist);
    
    
    // Determine start values for final fit
    peak_pos   = 1.12*peak_pos;
    peak_width = 0.5*peak_width;
    peak_amp   = 0.5*peak_amp/TMath::Landau(peak_pos, peak_pos, peak_width, true);
    
    TF1* fitFun = new TF1("total", muonSpec, xmin, xmax, 6);
    fitFun->SetNpx(500);
    fitFun->SetParameter(0, peak_amp);
    fitFun->SetParameter(1, peak_pos);
    fitFun->SetParameter(2, peak_width);
    fitFun->FixParameter(3, 1); // Switch on step function
    fitFun->SetParameter(4, 0);
    double offset = fitFun->Eval(xmin);
    double bkg_norm, bkg_slope;
    ExpStartValues(hist, xmin, 0.5*(xmin+peak_pos), bkg_norm, bkg_slope, offset);
    fitFun->SetParameter(4, bkg_norm);
    fitFun->SetParameter(5, bkg_slope);
    
    /*
    // Add function with start values
    TF1* start = (TF1*)fitFun->Clone("start");
    fitFun->Copy(*start);
    start->SetName("start");
    start->SetLineColor(4);
    hist->GetListOfFunctions()->Add(start);
    start->SetParent(hist);
    start->SetBit(TFormula::kNotGlobal);
    */

    // Do final fit
    int fitStatus = hist->Fit(fitFun,"QNR","GOFF");
    
    
    // Add signal and background functions to histogram
    TF1* bkg = (TF1*)fitFun->Clone("bkg");
    fitFun->Copy(*bkg);
    bkg->SetName("bkg");
    bkg->SetParameter(0, 0);
    bkg->SetParameter(3, 0);
    bkg->SetLineColor(2);
    hist->GetListOfFunctions()->Add(bkg);
    bkg->SetParent(hist);
    bkg->SetBit(TFormula::kNotGlobal);
    
    
    TF1* sig = (TF1*)fitFun->Clone("sig");
    fitFun->Copy(*sig);
    sig->SetName("sig");
    sig->SetParameter(4, 0);
    sig->SetLineColor(3);
    hist->GetListOfFunctions()->Add(sig);
    sig->SetParent(hist);
    sig->SetBit(TFormula::kNotGlobal);
    
    
    hist->GetListOfFunctions()->Add(fitFun);
    fitFun->SetParent(hist);
    fitFun->SetBit(TFormula::kNotGlobal);
    
    
    if(fitStatus==0)
    {
        // Determine maximum
        double x = sig->GetXmax();
	double deltaX = hist->GetBinWidth(1);
	while(sig->Eval(x-deltaX)>=sig->Eval(x))
	{
	    x-=deltaX;
	}
	double sig_peak = sig->GetMaximumX(x, x+5*deltaX);
    
	peak    = 0.95*sig_peak; // 1 VEM is defined as 95 % of the signal peak position 
	width   = fitFun->GetParameter(2);
	sbRatio = sig->Integral(0.3*sig_peak, 2.0*sig_peak)/bkg->Integral(0.3*sig_peak, 2.0*sig_peak);
	
	TArrow* arrow1 = new TArrow(sig_peak, 0.7*sig->Eval(sig_peak), sig_peak, 0.95*sig->Eval(sig_peak), 0.03, "|>");
	arrow1->SetAngle(30);
	arrow1->SetLineColor(3);
	arrow1->SetFillColor(3);
	arrow1->SetLineWidth(2);
	hist->GetListOfFunctions()->Add(arrow1);
	
	TArrow* arrow2 = new TArrow(peak, 1.05*sig->Eval(peak), peak, 1.3*sig->Eval(peak), 0.03, "<|");
	arrow2->SetAngle(30);
	arrow2->SetLineColor(1);
	arrow2->SetFillColor(1);
	arrow2->SetLineWidth(2);
	hist->GetListOfFunctions()->Add(arrow2);
	
	char info_text[100];
	sprintf(info_text,"1 VEM = %.1f pe", peak);
	TText* text = new TText(peak, 1.35*sig->Eval(peak), info_text);
	text->SetName("vem_label");
	text->SetTextAlign(21);
	text->SetTextSize(0.03);
	hist->GetListOfFunctions()->Add(text);
    }
    else
    {
        peak    = NAN;
	width   = NAN;
	sbRatio = NAN;
    }
    
    rChi2   = fitFun->GetChisquare()/fitFun->GetNDF();
    return fitStatus;
}


TH1F* VEMCalAnalyzer::ProfileX(TH2F* hist)
{
    std::stringstream profName;
    profName << hist->GetName() << "_prfx";
    
    TH1F* prof = new TH1F(profName.str().c_str(),
			  hist->GetTitle(),
			  hist->GetNbinsX(),
			  hist->GetXaxis()->GetXmin(),
			  hist->GetXaxis()->GetXmax()); 
    prof->GetXaxis()->SetTitle(hist->GetXaxis()->GetTitle());
    prof->GetYaxis()->SetTitle(hist->GetYaxis()->GetTitle());
    
    for(int i=1; i<hist->GetNbinsX(); i++)
    {
        double sumW   = 0;
	double sum    = 0;
	double sumSqr = 0;
	for(int k=1; k<hist->GetNbinsY(); k++)
	{
	  double y = hist->GetYaxis()->GetBinCenter(k);
	  double weight = hist->GetBinContent(i,k); 
	  if(weight<=0) continue;
	  
	  sum    += y*weight;
	  sumSqr += y*y*weight;
	  sumW   += weight;
	}
	
	if(sumW<10) continue;
	
	prof->SetBinContent(i, sum/sumW);
	prof->SetBinError(i, sqrt(sumSqr/sumW - (sum/sumW)*(sum/sumW))/sqrt(sumW));
    }
    
    prof->SetOption("E");

    return prof;
}


void VEMCalAnalyzer::HGLGStartValues(TH1F* hist, double x1, double x2, double x0, double y0, double& p2, double& p3)
{
  int bin1 = hist->GetXaxis()->FindBin(x1);
  int bin2 = hist->GetXaxis()->FindBin(x2);
  
  double meanX   = 0;
  double meanY   = 0;
  double meanXX  = 0;
  double meanXY  = 0;
  int    nPoints = 0;
  for(int i=bin1; i<=bin2; i++)
  {
    if(hist->GetBinContent(i)==0) continue;

    double x = log10(hist->GetBinCenter(i) - x0);
    double y = hist->GetBinContent(i) - y0;
    
    meanX  += x;
    meanY  += y;
    meanXX += x*x;
    meanXY += x*y;
    nPoints++;
  }
  
  meanX/=nPoints;
  meanY/=nPoints;
  meanXX/=nPoints;
  meanXY/=nPoints;
   
  double slope = (meanXY - meanX*meanY)/(meanXX - meanX*meanX);
  double intercept = meanY - slope*meanX;
  
  p2 = pow(10.0, intercept/slope);
  p3 = slope;
}


int VEMCalAnalyzer::FitHGLGCorrelation(TH2F* hist, double& lgCorr, double& hgCross, double& rChi2)
{
  TH1F* prof = ProfileX(hist);
  
  // Determine minimim x bin and value
  TH1D* projX = hist->ProjectionX();
  int minBinX = projX->GetMaximumBin();
  double minX = projX->GetBinLowEdge(minBinX);
  delete projX;
  
  // Determine maximum x value
  int b=prof->GetNbinsX();
  int count = 0;
  while(b>0 && count<3)
  {
      if(prof->GetBinError(b)>0) count++;
      b--;
  }
  double maxX = prof->GetBinLowEdge(b+1);

  //
  int startBin = prof->GetXaxis()->FindBin(700);
  int stopBin  = prof->GetXaxis()->FindBin(5000);
  
  TF1* bestFit = NULL;
  int  bestBin = 0;
  double bestRChi2 = 0;
  for(int i=startBin; i<=stopBin; i++)
  {
      // Calculate average diff value
      double sumWeights = 0;
      double diff_start = 0;
      for(int k=minBinX; k<=i; k++)
      {
	  double y  = prof->GetBinContent(k);
	  double ey = prof->GetBinError(k);
	  if(ey==0) continue;
	
	  double weight = 1.0/(ey*ey);
	  
	  diff_start += weight*y;
	  sumWeights += weight;
      }
      diff_start/=sumWeights; 
      
      double co_start = prof->GetXaxis()->GetBinCenter(i);
      
      double p2, p3;
      HGLGStartValues(prof, 8000, maxX, co_start, diff_start, p2, p3);

      std::stringstream funName;
      funName << "fit" << i;
      
      TF1* fitFun = new TF1(funName.str().c_str(), hglgDiff, minX, maxX, 4);
      fitFun->FixParameter(0, diff_start);
      fitFun->FixParameter(1, co_start);
      fitFun->SetParameter(2, p2);
      fitFun->SetParameter(3, p3);
      
      if(prof->Fit(fitFun,"QN","GOFF", co_start + prof->GetBinWidth(i), maxX)==0)
      {
	  double rChi2 = fitFun->GetChisquare()/fitFun->GetNDF();
	  if(bestBin==0 || rChi2 < bestRChi2)
	  {
	      bestRChi2 = rChi2;
	      bestBin = i;
	  
	      if(bestFit) delete bestFit;
	      bestFit = fitFun;
	  }
	  else
	  {
	      delete fitFun;
	  }
      }
  }

  //cout << bestBin << ": " << bestRChi2 << endl;
  
  prof->SetLineColor(2);
  prof->SetOption("SAME");
  hist->GetListOfFunctions()->Add(prof);
  
  if(bestFit)
  {
      bestFit->SetName("bestFit");
      bestFit->SetLineColor(3);
      bestFit->SetRange(minX, maxX);
      hist->GetListOfFunctions()->Add(bestFit);
      bestFit->SetParent(hist);
      bestFit->SetBit(TFormula::kNotGlobal);
      
      rChi2   = bestRChi2;
      lgCorr  = 1.0 - bestFit->GetParameter(0);
      hgCross = lgCorr*bestFit->GetParameter(1);
      return 0;
  }
  else
  {
      lgCorr  = NAN;
      hgCross = NAN;
      return 4;
  }
}


bool VEMCalAnalyzer::Summarize(TFile* rootfile, const VEMCalResultMap& results)
{
    if(!rootfile) return false;
    
    // Determine the histogram ranges
    std::pair<OMKey, double> min_vem_hg = make_pair(OMKey(0,0), 0);
    std::pair<OMKey, double> max_vem_hg = make_pair(OMKey(0,0), 0);
    
    std::pair<OMKey, double> min_vem_lg = make_pair(OMKey(0,0), 0);
    std::pair<OMKey, double> max_vem_lg = make_pair(OMKey(0,0), 0);
    
    std::pair<OMKey, double> min_crover = make_pair(OMKey(0,0), 0);
    std::pair<OMKey, double> max_crover = make_pair(OMKey(0,0), 0);

    std::pair<OMKey, double> min_rchi2_muon = make_pair(OMKey(0,0), 0);
    std::pair<OMKey, double> max_rchi2_muon = make_pair(OMKey(0,0), 0);
    
    std::pair<OMKey, double> min_rchi2_hglg = make_pair(OMKey(0,0), 0);
    std::pair<OMKey, double> max_rchi2_hglg = make_pair(OMKey(0,0), 0);
    
    VEMCalResultMap::const_iterator iter;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
        const OMKey& omKey = iter->first;
        const VEMCalResult& result = iter->second;
	
	switch(result.gainType)
	{
	    case 0: // HG
	    {
	        Minimum(min_vem_hg, omKey, result.pePerVEM);
		Maximum(max_vem_hg, omKey, result.pePerVEM);
		
		Minimum(min_crover, omKey, result.hglgCrossOver);
		Maximum(max_crover, omKey, result.hglgCrossOver);
		
		Minimum(min_rchi2_muon, omKey, result.muonFitRchi2);
		Maximum(max_rchi2_muon, omKey, result.muonFitRchi2);
		
		Minimum(min_rchi2_hglg, omKey, result.hglgFitRchi2);
		Maximum(max_rchi2_hglg, omKey, result.hglgFitRchi2);
		break;
	    }
	    case 1: // LG
	    {
	        Minimum(min_vem_lg, omKey, result.pePerVEM);
		Maximum(max_vem_lg, omKey, result.pePerVEM);
		break;
	    }
	    default:
	    {
	        log_error("Undefined gain type of %s", iter->first.str().c_str());
	        break;
	    }
	}
    }
    
    // Create and fill summary histograms
    TH1F* sum_vem    = FillVEMHistograms(results, min_vem_hg, max_vem_hg, min_vem_lg, max_vem_lg);
    TH1F* sum_vem_lg = static_cast<TH1F*>(sum_vem->GetListOfFunctions()->FindObject("summary_vem_lg"));
    if(!sum_vem_lg) 
    {
      delete sum_vem_lg;
      log_fatal("Couldn't find histogram \"summary_vem_lg\"!");
    }

    TH1F* sum_crover = FillCrossOverHistogram(results, min_crover, max_crover);
    TH1F* sum_rchi2  = FillRChi2Histograms(results, min_rchi2_muon, max_rchi2_muon, min_rchi2_hglg, max_rchi2_hglg);
    TH1F* sum_rchi2_hglg = static_cast<TH1F*>(sum_rchi2->GetListOfFunctions()->FindObject("summary_rchi2_hglg"));
    if(!sum_rchi2_hglg) 
    {
       delete sum_rchi2_hglg;
       log_fatal("Couldn't find histogram \"summary_rchi2_hglg\"!");
    }

    sum_vem->Write("summary_vem");
    sum_crover->Write("summary_crover");
    sum_rchi2->Write("summary_rchi2");
    
    
    // Check goodness of fits
    
    const double min_rchi2 = 0.3;
    const double max_rchi2 = 4.0;
    
    double mpv_rchi2_muon = sum_rchi2->GetBinCenter(sum_rchi2->GetMaximumBin());
    double mpv_rchi2_hglg = sum_rchi2_hglg->GetBinCenter(sum_rchi2_hglg->GetMaximumBin());
    
    bool isGood = true;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
        const OMKey& omKey = iter->first;
        const VEMCalResult& result = iter->second;
	
	// Only HG DOMs store fit results
	if(result.gainType!=0) continue;
	
	if(result.muonFitStatus>=0)
	{
	    // Divide rchi2 by most probable value 
	    double rchi2_muon = result.muonFitRchi2/mpv_rchi2_muon;
	    if(result.muonFitStatus!=0 || rchi2_muon<min_rchi2 || rchi2_muon>max_rchi2)
	    {
	        printf("Bad muon fit in module %02d-%02d: status=%d, chi2/ndf=%.2f\n",
		       omKey.GetString(), omKey.GetOM(), result.muonFitStatus, result.muonFitRchi2);
		isGood = false;
	    }
	}
	
	if(result.hglgFitStatus>=0)
	{
	    // Divide rchi2 by most probable value 
	    double rchi2_hglg = result.hglgFitRchi2/mpv_rchi2_hglg;
	    if(result.hglgFitStatus>0 || rchi2_hglg<min_rchi2 || rchi2_hglg>max_rchi2)
	    {
	        printf("Bad hglg fit in module %02d-%02d: status=%d, chi2/ndf=%.2f\n",
		       omKey.GetString(), omKey.GetOM(), result.hglgFitStatus, result.hglgFitRchi2);
		isGood = false;
	    }
	}
    }
    
    
    // Print summary  
    printf("----------------------------------------------------\n");
    printf("                 VEMCAL SUMMARY\n");
    printf("----------------------------------------------------\n");
    printf("Minimum HG VEM    : %6.1f pe --> (%02d-%02d)\n", min_vem_hg.second, min_vem_hg.first.GetString(), min_vem_hg.first.GetOM());
    printf("Maximum HG VEM    : %6.1f pe --> (%02d-%02d)\n", max_vem_hg.second, max_vem_hg.first.GetString(), max_vem_hg.first.GetOM());
    printf("Mean HG VEM       : %6.1f pe\n\n", sum_vem->GetMean());
    
    printf("Minimum LG VEM    : %6.1f pe --> (%02d-%02d)\n", min_vem_lg.second, min_vem_lg.first.GetString(), min_vem_lg.first.GetOM());
    printf("Maximum LG VEM    : %6.1f pe --> (%02d-%02d)\n", max_vem_lg.second, max_vem_lg.first.GetString(), max_vem_lg.first.GetOM());
    printf("Mean LG VEM       : %6.1f pe\n\n", sum_vem_lg->GetMean());

    printf("Minimum cross-over: %6.1f pe --> (%02d-%02d)\n", min_crover.second, min_crover.first.GetString(), min_crover.first.GetOM());
    printf("Maximum cross-over: %6.1f pe --> (%02d-%02d)\n", max_crover.second, max_crover.first.GetString(), max_crover.first.GetOM());
    printf("Mean cross-over   : %6.1f pe\n", sum_crover->GetMean());
    printf("----------------------------------------------------\n");

    delete sum_vem;
    delete sum_crover;
    delete sum_rchi2;

    return isGood;
}


TH1F* VEMCalAnalyzer::FillVEMHistograms(const VEMCalResultMap& results,
					const std::pair<OMKey, double>& min_vem_hg,
					const std::pair<OMKey, double>& max_vem_hg,
					const std::pair<OMKey, double>& min_vem_lg,
					const std::pair<OMKey, double>& max_vem_lg)
{
    const int margin = 5; // bins
    const double binWidth = 5.0; // pe
    
    double min_vem = std::min(min_vem_hg.second, min_vem_lg.second);
    double max_vem = std::max(max_vem_hg.second, max_vem_lg.second);
    
    double xmin = (std::floor(min_vem/binWidth) - margin)*binWidth;
    int nbins   = static_cast<int>(std::ceil((max_vem - xmin)/binWidth)) + margin;
    double xmax = xmin + nbins*binWidth;
    
    TH1F* sum_vem_hg = new TH1F("summary_vem_hg","Summary VEM", nbins, xmin, xmax);
    sum_vem_hg->GetXaxis()->SetTitle("Muon peak position [pe]");
    sum_vem_hg->GetYaxis()->SetTitle("Counts per bin");
    sum_vem_hg->SetLineColor(2);
    
    TH1F* sum_vem_lg = new TH1F("summary_vem_lg","Summary LG VEM", nbins, xmin, xmax);
    sum_vem_lg->GetXaxis()->SetTitle("Muon peak position [pe]");
    sum_vem_lg->GetYaxis()->SetTitle("Counts per bin");
    sum_vem_lg->SetLineColor(4);
    
    // Add lg histogram to hg histogram
    sum_vem_lg->SetOption("SAME");
    sum_vem_hg->GetListOfFunctions()->Add(sum_vem_lg);
    
    // Fill histograms
    VEMCalResultMap::const_iterator iter;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
        const VEMCalResult& result = iter->second;
	
	if(result.gainType==0)  // HG
	{
	    sum_vem_hg->Fill(result.pePerVEM);
	}
	else if(result.gainType==1) // LG
	{
	    sum_vem_lg->Fill(result.pePerVEM);
	}
	else
	{
	    log_fatal("Invalid gain type!");
	} 
    }
    
    TPaveText* vem_info = new TPaveText(0.6,0.4,0.96,0.6,"NDC");
    vem_info->SetName("min_max_values");
    vem_info->SetFillColor(0);
    vem_info->SetBorderSize(1);
    vem_info->SetShadowColor(0);
    vem_info->SetTextSize(0.03);
    vem_info->SetTextAlign(12);
    
    char line[100];
    sprintf(line, "Min. HG: %5.1f pe  (%d-%d)", min_vem_hg.second, min_vem_hg.first.GetString(), min_vem_hg.first.GetOM());
    vem_info->AddText(line);
    sprintf(line, "Max. HG: %5.1f pe  (%d-%d)", max_vem_hg.second, max_vem_hg.first.GetString(), max_vem_hg.first.GetOM());
    vem_info->AddText(line);
    sprintf(line, "Min. LG: %5.1f pe  (%d-%d)", min_vem_lg.second, min_vem_lg.first.GetString(), min_vem_lg.first.GetOM());
    vem_info->AddText(line);
    sprintf(line, "Max. LG: %5.1f pe  (%d-%d)", max_vem_lg.second, max_vem_lg.first.GetString(), max_vem_lg.first.GetOM());
    vem_info->AddText(line);
    
    sum_vem_hg->GetListOfFunctions()->Add(vem_info);
    
    return sum_vem_hg;
}


TH1F* VEMCalAnalyzer::FillCrossOverHistogram(const VEMCalResultMap& results,
					     const std::pair<OMKey, double>& min_crover,
					     const std::pair<OMKey, double>& max_crover)
{
    const int margin = 5; // bins
    const double binWidth = 50.0; // pe
    
    double xmin = (std::floor(min_crover.second/binWidth) - margin)*binWidth;
    int nbins   = static_cast<int>(std::ceil((max_crover.second - xmin)/binWidth)) + margin;
    double xmax = xmin + nbins*binWidth;
    
    TH1F* sum_crover = new TH1F("summary_crossover","Summary HG-LG cross-over", nbins, xmin, xmax);
    sum_crover->GetXaxis()->SetTitle("HG charge [pe]");
    sum_crover->GetYaxis()->SetTitle("Counts per bin");
    
    // Fill histograms
    VEMCalResultMap::const_iterator iter;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
        const VEMCalResult& result = iter->second;
	
	if(result.gainType==0 && result.hglgCrossOver>0)
	{
	    sum_crover->Fill(result.hglgCrossOver);
	}
    }
    
    TPaveText* info = new TPaveText(0.58,0.67,0.97,0.77,"NDC");
    info->SetName("min_max_values");
    info->SetFillColor(0);
    info->SetBorderSize(1);
    info->SetShadowColor(0);
    info->SetTextSize(0.03);
    info->SetTextAlign(12);
    
    char line[100];
    sprintf(line, "Min: %6.1f pe  (%d-%d)", min_crover.second, min_crover.first.GetString(), min_crover.first.GetOM());
    info->AddText(line);
    sprintf(line, "Max: %6.1f pe  (%d-%d)", max_crover.second, max_crover.first.GetString(), max_crover.first.GetOM());
    info->AddText(line);
    
    sum_crover->GetListOfFunctions()->Add(info);
    
    
    return sum_crover;
}


TH1F* VEMCalAnalyzer::FillRChi2Histograms(const VEMCalResultMap& results,
					  const std::pair<OMKey, double>& min_rchi2_muon,
					  const std::pair<OMKey, double>& max_rchi2_muon,
					  const std::pair<OMKey, double>& min_rchi2_hglg,
					  const std::pair<OMKey, double>& max_rchi2_hglg)
{
    const int margin = 5; // bins
    const double binWidth = 0.1; 
    
    double min_rchi2 = std::min(min_rchi2_muon.second, min_rchi2_hglg.second);
    double max_rchi2 = std::max(max_rchi2_muon.second, max_rchi2_hglg.second);
    
    double xmin = (std::floor(min_rchi2/binWidth) - margin)*binWidth;
    int nbins   = static_cast<int>(std::ceil((max_rchi2 - xmin)/binWidth)) + margin;
    double xmax = xmin + nbins*binWidth;
    
    TH1F* sum_rchi2_muon = new TH1F("summary_rchi2","Summary #chi^{2}/dof", nbins, xmin, xmax);
    sum_rchi2_muon->GetXaxis()->SetTitle("#chi^{2}/dof");
    sum_rchi2_muon->GetYaxis()->SetTitle("Counts per bin");
    sum_rchi2_muon->SetLineColor(2);
    
    TH1F* sum_rchi2_hglg = new TH1F("summary_rchi2_hglg","Summary HG-LG #chi^{2}/dof", nbins, xmin, xmax);
    sum_rchi2_hglg->GetXaxis()->SetTitle("#chi^{2}/dof");
    sum_rchi2_hglg->GetYaxis()->SetTitle("Counts per bin");
    sum_rchi2_hglg->SetLineColor(4);
    
    // Add hglg histogram to muon histogram
    sum_rchi2_hglg->SetOption("SAME");
    sum_rchi2_muon->GetListOfFunctions()->Add(sum_rchi2_hglg);
    
    // Fill histograms
    VEMCalResultMap::const_iterator iter;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
        const VEMCalResult& result = iter->second;
	if(result.gainType!=0) continue; 
	
	if(result.muonFitRchi2>0) sum_rchi2_muon->Fill(result.muonFitRchi2);
	if(result.hglgFitRchi2>0) sum_rchi2_hglg->Fill(result.hglgFitRchi2);
    }
    
    TPaveText* chi2_info = new TPaveText(0.6,0.4,0.96,0.6,"NDC");
    chi2_info->SetName("min_max_values");
    chi2_info->SetFillColor(0);
    chi2_info->SetBorderSize(1);
    chi2_info->SetShadowColor(0);
    chi2_info->SetTextSize(0.03);
    chi2_info->SetTextAlign(12);
    
    char line[100];
    sprintf(line, "Min. Muon:  %4.1f  (%d-%d)", min_rchi2_muon.second, min_rchi2_muon.first.GetString(), min_rchi2_muon.first.GetOM());
    chi2_info->AddText(line);
    sprintf(line, "Max. Muon:  %4.1f  (%d-%d)", max_rchi2_muon.second, max_rchi2_muon.first.GetString(), max_rchi2_muon.first.GetOM());
    chi2_info->AddText(line);
    sprintf(line, "Min. HG-LG: %4.1f  (%d-%d)", min_rchi2_hglg.second, min_rchi2_hglg.first.GetString(), min_rchi2_hglg.first.GetOM());
    chi2_info->AddText(line);
    sprintf(line, "Max. HG-LG: %4.1f  (%d-%d)", max_rchi2_hglg.second, max_rchi2_hglg.first.GetString(), max_rchi2_hglg.first.GetOM());
    chi2_info->AddText(line);
    
    sum_rchi2_muon->GetListOfFunctions()->Add(chi2_info);
    
    return sum_rchi2_muon;
}



void VEMCalAnalyzer::WriteXML(const VEMCalResultMap& results, bool isGood)
{
    // Create file name
    std::ostringstream xmlFileName;
    xmlFileName << "VEM_calibration_" << startTime_.GetUTCString("%Y-%m-%d_%H%M%S");
    if(!isGood) xmlFileName << "_failed";
    xmlFileName << ".xml";
    
    // Open file
    std::ofstream xmlFile(xmlFileName.str().c_str());
    
    log_info("Creating file %s", xmlFileName.str().c_str());

    // Write header information 
    xmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << endl;
    xmlFile << "<!-- VEM calibration from Run" << firstRunID_ << " to Run" << lastRunID_ << "  -->" << endl;
    xmlFile << "<!-- Valid from date: " << startTime_.GetUTCString("%Y-%m-%d %H:%M:%S") << "  -->" << endl << endl;
    
    xmlFile << "<VEMCalibOm>" << endl;
    xmlFile << "  <Date> " << startTime_.GetUTCString("%Y-%m-%d %H:%M:%S") << " </Date>" << endl;
    xmlFile << "  <FirstRun> " << firstRunID_ << " </FirstRun>" << endl;
    xmlFile << "  <LastRun> " << lastRunID_ << " </LastRun>" << endl;
    
    // Write the calibration results
    VEMCalResultMap::const_iterator iter;
    for(iter=results.begin(); iter!=results.end(); iter++)
    {
	const OMKey& omKey = iter->first;
	const VEMCalResult& result = iter->second;
	
	// Set default hglgCrossOver if LG DOM doesn't exist
	double crossOver = result.hglgCrossOver;
	if(result.gainType==0 && crossOver<0) crossOver = 2500;
	
	xmlFile << "  <DOM>" << endl;
	xmlFile << "    <!-- This is DOM " << omKey.GetString() << "-" << omKey.GetOM() << "  -->" << endl;
	xmlFile << "    <StringId> " << omKey.GetString() << " </StringId>" << endl; 
	xmlFile << "    <TubeId> " << omKey.GetOM() << " </TubeId>" << endl; 
	xmlFile << "    <pePerVEM> " << result.pePerVEM << " </pePerVEM>" << endl; 
	xmlFile << "    <muPeakWidth> " << result.muonWidth << " </muPeakWidth>" << endl; 
	xmlFile << "    <sigBkgRatio> " << result.sbRatio << " </sigBkgRatio>" << endl; 
	xmlFile << "    <corrFactor> 1 </corrFactor>" << endl;  // This is always 1 for the new calibration method! 
	xmlFile << "    <hglgCrossOver> " << crossOver << " </hglgCrossOver>" << endl; 
	if(result.gainType==0)
	{
	    xmlFile << "    <muonFitStatus> " << result.muonFitStatus << " </muonFitStatus>" << endl;
	    xmlFile << "    <muonFitRchi2> " << result.muonFitRchi2 << " </muonFitRchi2>" << endl;
	    xmlFile << "    <hglgFitStatus> " << result.hglgFitStatus << " </hglgFitStatus>" << endl;
	    xmlFile << "    <hglgFitRchi2> " << result.hglgFitRchi2 << " </hglgFitRchi2>" << endl;
	}
	xmlFile << "  </DOM>" << endl;
    }
    
    xmlFile << "</VEMCalibOm>" << endl;

    xmlFile.close();
}
