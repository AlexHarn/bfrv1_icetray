/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalTreeWriter.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */

#include <icetray/I3Frame.h>
#include <dataclasses/I3Time.h>
#include <vemcal/I3VEMCalTreeWriter.h>
#include <icetray/I3Units.h>

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>


I3_MODULE(I3VEMCalTreeWriter);


I3VEMCalTreeWriter::I3VEMCalTreeWriter(const I3Context& context): I3ConditionalModule(context),
outfile_(0), muonTree_(0), hglgTree_(0)
{
    vemCalDataName_ = I3DefaultName<I3VEMCalData>::value();
    AddParameter("VEMCalDataName","Name if the I3VEMCalData object in the frame.", vemCalDataName_);
    
    workDir_ = ".";
    AddParameter("WorkDir","Working directory where the root files will be written", workDir_);
    
    fileBaseName_ = "IceTop_VEMCal";
    AddParameter("FileBaseName","Base name of the file.", fileBaseName_);
}


I3VEMCalTreeWriter::~I3VEMCalTreeWriter()
{
    Clear();
}


void I3VEMCalTreeWriter::Clear()
{
    if(muonTree_) delete muonTree_;
    muonTree_ = NULL;

    if(hglgTree_) delete hglgTree_;
    hglgTree_ = NULL;
      
    if(outfile_) delete outfile_;
    outfile_ = NULL;
    
    currentRunNumber_ = -1;
}


void I3VEMCalTreeWriter::Write()
{
    if(!outfile_) return;
    outfile_->cd();
    
    if(muonTree_) muonTree_->AutoSave();
    if(hglgTree_) hglgTree_->AutoSave();
}


void I3VEMCalTreeWriter::Configure()
{
    Clear();
    
    log_info("Configuring I3VEMCalTreeWriter:");
    
    GetParameter("VEMCalDataName", vemCalDataName_);
    
    GetParameter("FileBaseName", fileBaseName_);
    log_info("   File base name               : %s", fileBaseName_.c_str());
    
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


void I3VEMCalTreeWriter::Physics(I3FramePtr frame)
{
    // Get the I3VEMCalData container from the frame
    I3VEMCalDataConstPtr vemCalData = frame->Get<I3VEMCalDataConstPtr>(vemCalDataName_);
    if(!vemCalData)
    {
	PushFrame(frame);
	return;
    }
    
    // Get the driving time (start time of the event) from the frame
    I3TimeConstPtr startTime = frame->Get<I3TimeConstPtr>("DrivingTime");
    if(!startTime)
    {
	log_error("Missing DrivingTime. Skipping this frame!");
	PushFrame(frame);
	return;
    }
    
    int runNumber = static_cast<int>(vemCalData->runID);
    
    if(currentRunNumber_ != runNumber)
    {
	Write();
	Clear();
	
	currentRunNumber_ = runNumber;
	
	std::stringstream path;
	path.fill('0');
	path << workDir_ 
	     << std::setw(2) << startTime->GetUTCMonth()
	     << std::setw(2) << startTime->GetUTCDayOfMonth()
	     << "/";
	
	// Create sub-directory for specific date
	// if it doesn't exist yet
	gSystem->mkdir(path.str().c_str(),true);
	if(!gSystem->OpenDirectory(path.str().c_str()))
	{
	    log_error("Failed to open directory %s.", path.str().c_str());
	    PushFrame(frame);
	    return;
	}
	
	std::stringstream fileName;
	fileName.fill('0');
	fileName << path.str() << fileBaseName_ << "_" 
		 << "Run" << std::setw(8) << currentRunNumber_ << "_"
		 << startTime->GetUTCString("%Y-%m-%d_%H%M%S") << ".root";
	
	outfile_ = new TFile(fileName.str().c_str(), "RECREATE");
	if(outfile_->IsOpen()) log_info("Opened file %s", fileName.str().c_str());
	else                   log_error("Failed to create file %s", fileName.str().c_str());
    }
    
    // Fill MinBias hits into root tree
    std::vector<I3VEMCalData::MinBiasHit>::const_iterator mbhit_iter;
    for(mbhit_iter = vemCalData->minBiasHits.begin(); mbhit_iter != vemCalData->minBiasHits.end(); ++mbhit_iter)
    {
	FillVEMTree(*startTime, *mbhit_iter);
    }
    
    // Fill HGLG correlation hits into root tree
    std::vector<I3VEMCalData::HGLGhit>::const_iterator hglghit_iter;
    for(hglghit_iter = vemCalData->hglgHits.begin(); hglghit_iter != vemCalData->hglgHits.end(); ++hglghit_iter)
    {
	FillHGLGTree(*hglghit_iter);
    }
    
    PushFrame(frame);
}


void I3VEMCalTreeWriter::FillVEMTree(const I3Time& startTime, const I3VEMCalData::MinBiasHit& minBiasHit)
{
    if(!outfile_) return;
    
    UChar_t str        = static_cast<UChar_t>(minBiasHit.str);
    UChar_t om         = static_cast<UChar_t>(minBiasHit.om);
    Float_t charge_pe  = static_cast<Float_t>(minBiasHit.charge_dpe/10.0);
    Double_t time_ms   = static_cast<Double_t>(1e-7*startTime.GetUTCDaqTime());
    
    if(!muonTree_)
    {
	outfile_->cd();
	muonTree_ = new TTree("muonTree","Minimum bias hits for the muon spectrum");
	muonTree_->Branch("str",        &str,        "str/b");
	muonTree_->Branch("om",         &om,         "om/b");
	muonTree_->Branch("charge_pe",  &charge_pe,  "charge_pe/F");
	muonTree_->Branch("time_ms",    &time_ms,    "time_ms/D");
	
	// Automatically save tree after every 1MB of data
	muonTree_->SetAutoSave(1000000);
    }
    else
    {
	muonTree_->SetBranchAddress("str",        &str);
	muonTree_->SetBranchAddress("om",         &om);
	muonTree_->SetBranchAddress("charge_pe",  &charge_pe);
	muonTree_->SetBranchAddress("time_ms",    &time_ms);
    }
    
    // Fill the tree
    muonTree_->Fill();
};


void I3VEMCalTreeWriter::FillHGLGTree(const I3VEMCalData::HGLGhit& hglgHit)
{
    if(!outfile_) return;
    
    UChar_t str          = static_cast<UChar_t>(hglgHit.str);
    UChar_t hg_om        = static_cast<UChar_t>(hglgHit.hg_om);
    Float_t hg_charge_pe = static_cast<Float_t>(hglgHit.hg_charge_pe);
    UChar_t lg_om        = static_cast<UChar_t>(hglgHit.lg_om);
    Float_t lg_charge_pe = static_cast<Float_t>(hglgHit.lg_charge_pe);
    
    if(!hglgTree_)
    {
	outfile_->cd();
	hglgTree_ = new TTree("hglgTree","High gain - low gain correlation data");
	hglgTree_->Branch("str",          &str,          "str/b");
	hglgTree_->Branch("hg_om",        &hg_om,        "om/b");
	hglgTree_->Branch("hg_charge_pe", &hg_charge_pe, "charge/F");
	hglgTree_->Branch("lg_om",        &lg_om,        "om/b");
	hglgTree_->Branch("lg_charge_pe", &lg_charge_pe, "charge/F");
	
	// Automatically save tree after every 1MB of data
	hglgTree_->SetAutoSave(1000000);
    }
    else
    {
	hglgTree_->SetBranchAddress("str",          &str);
	hglgTree_->SetBranchAddress("hg_om",        &hg_om);
	hglgTree_->SetBranchAddress("hg_charge_pe", &hg_charge_pe);
	hglgTree_->SetBranchAddress("lg_om",        &lg_om);
	hglgTree_->SetBranchAddress("lg_charge_pe", &lg_charge_pe);
    }
    
    // Fill the tree
    hglgTree_->Fill();
};
