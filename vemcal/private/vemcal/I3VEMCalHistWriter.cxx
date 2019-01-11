/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalHistWriter.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */

#include <icetray/I3Frame.h>
#include <icetray/I3Units.h>
#include <dataclasses/I3Time.h>
#include <vemcal/I3VEMCalHistWriter.h>
#include <dataclasses/TankKey.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TSystem.h>

#include <sstream>


I3_MODULE(I3VEMCalHistWriter);


I3VEMCalHistWriter::I3VEMCalHistWriter(const I3Context& context): I3ConditionalModule(context),
outfile_(0)
{
    vemCalDataName_ = I3DefaultName<I3VEMCalData>::value();
    AddParameter("VEMCalDataName","Name of the I3VEMCalData object in the frame.", vemCalDataName_);
    
    workDir_ = ".";
    AddParameter("WorkDir","Output directory", workDir_);
    
    fileBaseName_ = "IceTop_VEMCalData";
    AddParameter("FileBaseName","Base name (prefix) of the root files.", fileBaseName_);
    
    //
    // Histogram binning
    //
    muonBinning_.clear();
    muonBinning_.push_back(200); // number of bins
    muonBinning_.push_back(0);   // minimum value [pe]
    muonBinning_.push_back(600); // maximum value [pe]
    AddParameter("MuonBinning","Binning of muon spectra (bin width must be a multiple of 0.1 pe)", muonBinning_);
    
    hglgBinning_.clear();
    hglgBinning_.push_back(400);   // number of bins
    hglgBinning_.push_back(0);     // minimum value [pe]
    hglgBinning_.push_back(20000); // maximum value [pe]
    AddParameter("HGLGBinning","Binning of HG-LG scatter plots (bin width must be a multiple of 1 pe)", hglgBinning_);   

    maxTimeDiff_ = 20.0;
    AddParameter("MaxHGLGTimeDifference", "Maximum time difference between HG and LG in [ns]", maxTimeDiff_);
}


I3VEMCalHistWriter::~I3VEMCalHistWriter()
{
    Clear();
}


void I3VEMCalHistWriter::Clear()
{
    // Clear muon histos
    MuonHistMap::iterator muon_iter;
    for(muon_iter = muonHistos_.begin(); muon_iter != muonHistos_.end(); muon_iter++)
    {
	delete muon_iter->second;
    }
    muonHistos_.clear();

    // Clear HGLG histos
    HGLGHistMap::iterator hglg_iter;
    for(hglg_iter = hglgHistos_.begin(); hglg_iter != hglgHistos_.end(); hglg_iter++)
    {
	delete hglg_iter->second;
    }
    hglgHistos_.clear();

    // Delete rootfile
    if(outfile_) delete outfile_;
    outfile_ = NULL;
    
    // Reset run number
    currentRunNumber_ = -1;
}


void I3VEMCalHistWriter::Write()
{
    if(!outfile_) return;
    outfile_->cd();
    
    // Write muon histos
    MuonHistMap::iterator muon_iter;
    for(muon_iter = muonHistos_.begin(); muon_iter != muonHistos_.end(); muon_iter++)
    {
	muon_iter->second->Write();
    }
    
    // Write HGLG histos
    HGLGHistMap::iterator hglg_iter;
    for(hglg_iter = hglgHistos_.begin(); hglg_iter != hglgHistos_.end(); hglg_iter++)
    {
	hglg_iter->second->Write();
    }
}


void I3VEMCalHistWriter::Configure()
{
    Clear();
    
    log_info("Configuring I3VEMCalHistWriter:");
    
    GetParameter("VEMCalDataName", vemCalDataName_);
    
    GetParameter("FileBaseName", fileBaseName_);
    log_info("   File base name               : %s", fileBaseName_.c_str());
    

    //
    // Histogram binning
    //
    GetParameter("MuonBinning", muonBinning_);
    if(muonBinning_.size() != 3)
    {
	log_fatal("Invalid number of entries in parameter \"MuonBinning\"!");
    }
    
    int muonNumBins = static_cast<int>(muonBinning_.at(0));
    double muonXmin = muonBinning_.at(1);
    double muonXmax = muonBinning_.at(2);
    
    double muonBinWidth = 0.1*std::floor(10.0*(muonXmax - muonXmin)/muonNumBins + 0.5);
    muonXmin = 0.1*std::floor(10.0*muonXmin + 0.5) + 0.05;
    muonXmax = muonXmin + muonNumBins*muonBinWidth;
    
    muonBinning_[0] = muonNumBins;
    muonBinning_[1] = muonXmin;
    muonBinning_[2] = muonXmax;
    
    log_info("   Muon binning: num bins = %.0f, xmin = %.2f pe, xmax = %.2f pe  (bin width = %.2f pe)",
	     muonBinning_.at(0), muonBinning_.at(1), muonBinning_.at(2), muonBinWidth);   
    
    
    GetParameter("HGLGBinning", hglgBinning_);
    if(hglgBinning_.size() != 3)
    {
	log_fatal("Invalid number of entries in parameter \"HGLGBinning\"!");
    }
    
    int hglgNumBins = static_cast<int>(hglgBinning_.at(0));
    double hglgXmin = hglgBinning_.at(1);
    double hglgXmax = hglgBinning_.at(2);
    
    double hglgBinWidth = std::floor((hglgXmax - hglgXmin)/hglgNumBins + 0.5);
    hglgXmin = std::floor(hglgXmin + 0.5) + 0.5;
    hglgXmax = hglgXmin + hglgNumBins*hglgBinWidth;
    
    hglgBinning_[0] = hglgNumBins;
    hglgBinning_[1] = hglgXmin;
    hglgBinning_[2] = hglgXmax;
    
    log_info("   HGLG binning: num bins = %.0f, xmin = %.2f pe, xmax = %.2f pe  (bin width = %.2f pe)",
	     hglgBinning_.at(0), hglgBinning_.at(1), hglgBinning_.at(2), hglgBinWidth);
    
    
    GetParameter("MaxHGLGTimeDifference", maxTimeDiff_);
    maxTimeDiff_*=I3Units::ns;
    log_info("   Maximum HG-LG time difference: %.2f ns", maxTimeDiff_/I3Units::ns);
    
    
    GetParameter("WorkDir", workDir_);
    
      // Check the working directory and remove unnecessary "/" characters
    if(workDir_.empty()) workDir_ = ".";
    while(workDir_.find_last_of("/") == workDir_.length()-1) workDir_.erase(workDir_.length()-1); 
    workDir_ += "/";
    
    // Create the working directory, if it doesn't exist
    gSystem->mkdir(workDir_.c_str(),true);
    if(gSystem->OpenDirectory(workDir_.c_str())) log_info("   Working directory            : %s", workDir_.c_str());
    else                                         log_error("Failed to open working directory %s !", workDir_.c_str());
}


void I3VEMCalHistWriter::Physics(I3FramePtr frame)
{
    // Get the I3VEMCalData container from the frame
    I3VEMCalDataConstPtr vemCalData = frame->Get<I3VEMCalDataConstPtr>(vemCalDataName_);
    if(!vemCalData)
    {
	PushFrame(frame);
	return;
    }
    
    int runNumber = static_cast<int>(vemCalData->runID);
    
    if(currentRunNumber_ != runNumber)
    {
	// Write the data of the previous run
	Write();
	Clear();
	
	currentRunNumber_ = runNumber;
	
	// Create filename and open root file
	std::stringstream fileName;
	fileName.fill('0');
	fileName << workDir_ << fileBaseName_ << "_" 
		 << "Run" << std::setw(8) << currentRunNumber_ << ".root";
	
	outfile_ = new TFile(fileName.str().c_str(), "RECREATE");
	if(outfile_->IsOpen()) log_info("Opened file %s", fileName.str().c_str());
	else                   log_error("Failed to create file %s", fileName.str().c_str());
    }
    
    // Fill MinBias hits into histograms
    std::vector<I3VEMCalData::MinBiasHit>::const_iterator mbhit_iter;
    for(mbhit_iter = vemCalData->minBiasHits.begin(); mbhit_iter != vemCalData->minBiasHits.end(); ++mbhit_iter)
    {
	FillMuonHistos(*mbhit_iter);
    }
    
    // Fill HGLG correlation hits into histograms
    std::vector<I3VEMCalData::HGLGhit>::const_iterator hglghit_iter;
    for(hglghit_iter = vemCalData->hglgHits.begin(); hglghit_iter != vemCalData->hglgHits.end(); ++hglghit_iter)
    {
	FillHGLGHistos(*hglghit_iter);
    }
    
    PushFrame(frame);
}


void I3VEMCalHistWriter::FillMuonHistos(const I3VEMCalData::MinBiasHit& minBiasHit)
{
    OMKey hgKey(minBiasHit.str, minBiasHit.om);
    MuonHistMap::const_iterator iter = muonHistos_.find(hgKey);
    
    TH1F* hist = NULL;
    if(iter == muonHistos_.end())
    {
	//
        // Create new histogram
	//
	std::stringstream histName;
	histName.fill('0');
	histName << "muonSpec_"
		 << std::setw(2) << hgKey.GetString() << "_"
		 << std::setw(2) << hgKey.GetOM();
	
	std::stringstream histTitle;
	histTitle << "Muon Spectrum of DOM(" << hgKey.GetString() << ", " << hgKey.GetOM() << ")";
	
	hist = new TH1F(histName.str().c_str(),
			histTitle.str().c_str(), 
			static_cast<int>(muonBinning_.at(0)),
			muonBinning_.at(1),
			muonBinning_.at(2));
	
	hist->SetDirectory(0);
	hist->GetXaxis()->SetTitle("Charge [pe]");
	hist->GetYaxis()->SetTitle("Counts per bin");
	hist->SetFillColor(17);
	
	muonHistos_[hgKey] = hist;
    }
    else
    {
	hist = iter->second;
    }
    
    hist->Fill(0.1*static_cast<double>(minBiasHit.charge_dpe));
}


void I3VEMCalHistWriter::FillHGLGHistos(const I3VEMCalData::HGLGhit& hglgHit)
{
    // Only accept hglgHit with |deltaT| < maxTimeDiff_  
    double deltat = static_cast<double>(hglgHit.deltat_2ns)*2.0*I3Units::ns;
    if(fabs(deltat) > maxTimeDiff_)
    {
      log_warn("Tank %d%s: HG-LG time difference of %.1f ns exceeds %.1f ns. Skipping this hit!",
	       hglgHit.str, (hglgHit.hg_om<63?"A":"B"), deltat/I3Units::ns, maxTimeDiff_/I3Units::ns);
      return;
    }

    OMKey hgKey(hglgHit.str, hglgHit.hg_om);
    HGLGHistMap::const_iterator iter = hglgHistos_.find(hgKey);
    
    TH2F* hist = NULL;
    if(iter == hglgHistos_.end())
    {
	//
        // Create new histogram
	//
      
        std::stringstream histName;
	histName.fill('0');
	histName << "hglgDiff_"
		 << std::setw(2) << hgKey.GetString() << "_"
		 << std::setw(2) << hgKey.GetOM() << "_"
		 << std::setw(2) << static_cast<int>(hglgHit.lg_om);
	
	TankKey tankKey(hgKey);
	std::stringstream histTitle;
	histTitle << "Relative charge difference in Tank " << tankKey;
	
	hist = new TH2F(histName.str().c_str(),
			histTitle.str().c_str(),
			static_cast<int>(hglgBinning_.at(0)),
			hglgBinning_.at(1),
			hglgBinning_.at(2),
			400,
			-2,
			2);
	
	hist->SetDirectory(0);
	hist->GetXaxis()->SetTitle("LG charge [pe]");
	hist->GetYaxis()->SetTitle("(LG - HG)/LG");
	
	hglgHistos_[hgKey] = hist;
    }
    else
    {
	hist = iter->second;
    }
    
    double lgCharge = static_cast<double>(hglgHit.lg_charge_pe);
    double hgCharge = static_cast<double>(hglgHit.hg_charge_pe);
    
    hist->Fill(lgCharge, (lgCharge - hgCharge)/lgCharge); 
}
