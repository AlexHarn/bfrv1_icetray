
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip> 
#include <fstream> 

#include <icetray/OMKey.h>
#include <dataclasses/I3Time.h>


#include <TFile.h>
#include <TNtupleD.h>
#include <TGraph.h>
#include <TProfile2D.h>



struct VEMCalValues_t 
{
  double pePerVEM;
  double muWidth;
  double sbRatio;
  double corrFactor;
  double hglgCrossOver;

  VEMCalValues_t():
    pePerVEM(NAN),
    muWidth(NAN),
    sbRatio(NAN),
    corrFactor(NAN),
    hglgCrossOver(NAN)
  {};
};

typedef std::map<OMKey, VEMCalValues_t> VEMCalValuesMap_t;

void ParseVEMCalFile(const std::string& filename, VEMCalValuesMap_t& vemValuesMap, I3Time& startTime, int& firstRun, int& lastRun)
{
    // Open file	 
    ifstream ifs(filename.c_str(), ifstream::in);
    if(!ifs.good())
    {
	log_error("Couldn't open file \"%s\"!", filename.c_str());
	return;
    }
    
    // Find start
    int domCount=0;
    std::string line;
    do getline(ifs,line);
    while(line.find("<VEMCalibOm>")==std::string::npos && !ifs.eof());
    
    // Loop over entries
    while(line.find("</VEMCalibOm>")==std::string::npos && !ifs.eof())
    {
	getline(ifs,line);
	
	if(line.find("<Date>")!=std::string::npos)
	{
	    int year, month, day, hour, minute, second;
	    if(sscanf(line.c_str()," <Date> %d-%d-%d %d:%d:%d </Date>", &year, &month, &day, &hour, &minute, &second)==6)
	    {
		startTime.SetUTCCalDate(year,month,day,hour,minute,second);
		log_debug("VEM Calibration date: %d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", year, month, day, hour, minute, second); 
	    }
	}

	if(line.find("<FirstRun>")!=std::string::npos)
	{
	    if(sscanf(line.c_str()," <FirstRun> %d </FirstRun>", &firstRun)!=1) firstRun = -1; 
	}

	if(line.find("<LastRun>")!=std::string::npos)
	{
	    if(sscanf(line.c_str()," <LastRun> %d </LastRun>", &lastRun)!=1) lastRun = -1; 
	}
	
	if(line.find("<DOM>")!=std::string::npos)
	{
	    domCount++;
	    
	    int str = -1;
	    int om  = -1;
	    VEMCalValues_t vemValues;
	    
	    while(line.find("</DOM>")==std::string::npos && !ifs.eof())
	    {
		getline(ifs,line);
		
		if(line.find("<StringId>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <StringId> %d </StringId>", &str)!=1) str = -1;
		}

		if(line.find("<TubeId>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <TubeId> %d </TubeId>", &om)!=1) om = -1;
		}

		if(line.find("<pePerVEM>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <pePerVEM> %lf </pePerVEM>", &vemValues.pePerVEM)!=1) vemValues.pePerVEM = NAN;
		}

		if(line.find("<muPeakWidth>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <muPeakWidth> %lf </muPeakWidth>", &vemValues.muWidth)!=1) vemValues.muWidth = NAN;
		}
		
		if(line.find("<sigBkgRatio>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <sigBkgRatio> %lf </sigBkgRatio>", &vemValues.sbRatio)!=1) vemValues.sbRatio = NAN;
		}
		
		if(line.find("<corrFactor>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <corrFactor> %lf </corrFactor>", &vemValues.corrFactor)!=1) vemValues.corrFactor = NAN;
		}

		if(line.find("<hglgCrossOver>")!=std::string::npos)
		{
		    if(sscanf(line.c_str()," <hglgCrossOver> %lf </hglgCrossOver>", &vemValues.hglgCrossOver)!=1) vemValues.hglgCrossOver = NAN;
		}
	    }
	    
	    OMKey omKey(str, om);
	    vemValuesMap[omKey] = vemValues;
	}
    }
    
    ifs.close();
}


double GetMoniValue(const OMKey& omKey, TProfile2D* prof)
{
    if(!prof) return NAN;
    if(prof->GetEntries()<=0) return NAN;
  
    int xbin = prof->GetXaxis()->FindBin(omKey.GetOM());
    int ybin = prof->GetYaxis()->FindBin(omKey.GetString());
    
    return prof->GetBinContent(xbin, ybin); 
}


int GetRunID(TFile* moniFile)
{
    TNtupleD* runInfo = static_cast<TNtupleD*>(moniFile->Get("MoniRunInfo"));
    if(!runInfo) return -1;
    
    if(runInfo->GetEntries()<1) return -1;
    
    double minID  = runInfo->GetMinimum("Run");
    if(!finite(minID) || minID <= 0) return -1;
    
    double maxID  = runInfo->GetMaximum("Run");
    if(!finite(maxID) || maxID <= 0) return -1;
    
    if(minID!=maxID) return -1;
    
    return static_cast<int>(minID);
}


I3Time GetRunStartTime(TFile* moniFile, const I3Time& vemCalTime)
{
    I3Time runStartTime(0,0);
  
    TTree* tree = static_cast<TTree*>(moniFile->Get("RunStartTimes"));
    if(!tree) return runStartTime;
    
    // If tree is empty return 0 
    if(tree->GetEntries()<1) return runStartTime;
    
    // Set Branch Address
    ULong64_t time;
    tree->SetBranchAddress("UT",&time);
    
    ULong64_t minTime = 0;
    for(int i=0; i<tree->GetEntries(); i++)
    {
        tree->GetEntry(i);
	if(i==0) minTime = time;
	else     minTime = std::min(minTime, time);
    }
    
    int64_t minDaqTime = static_cast<int64_t>(minTime);
    if(minDaqTime < vemCalTime.GetUTCDaqTime())
    {
        runStartTime.SetDaqTime(vemCalTime.GetUTCYear()+1, minDaqTime);
    }
    else
    {
        runStartTime.SetDaqTime(vemCalTime.GetUTCYear(), minDaqTime);
    }
    
    return runStartTime;
}


double GetRunDuration(TFile* moniFile, const I3Time& vemCalTime)
{
    TNtupleD* runInfo = static_cast<TNtupleD*>(moniFile->Get("MoniRunInfo"));
    if(!runInfo) return NAN;
    
    // If RunInfo is empty return NAN;
    if(runInfo->GetEntries()<1) return NAN;
    
    double startDaqTime = runInfo->GetMinimum("MoniStartTime");
    if(!finite(startDaqTime) || startDaqTime<0) return NAN;
    
    double endDaqTime = runInfo->GetMaximum("MoniEndTime");
    if(!finite(endDaqTime) || endDaqTime<0) return NAN;
    
    // Determine start time
    I3Time startTime;
    if(startDaqTime < vemCalTime.GetUTCDaqTime())
    {
        startTime.SetDaqTime(vemCalTime.GetUTCYear()+1, static_cast<int64_t>(startDaqTime));
    }
    else
    {
        startTime.SetDaqTime(vemCalTime.GetUTCYear(), static_cast<int64_t>(startDaqTime));
    }
    
    // Determine end time
    I3Time endTime;
    if(endDaqTime < startTime.GetUTCDaqTime())
    {
        endTime.SetDaqTime(startTime.GetUTCYear()+1, static_cast<int64_t>(endDaqTime));
    }
    else
    {
        endTime.SetDaqTime(startTime.GetUTCYear(), static_cast<int64_t>(endDaqTime));
    }
    
    // Return approximate run duration in seconds
    return (endTime - startTime)*1e-9;
}


void ReadMoniFiles(TFile* outfile,
		   std::vector<std::string>& monifiles,
		   const VEMCalValuesMap_t& vemValuesMap,
		   const I3Time& vemCalTime,
		   int firstRun,
		   int lastRun)
{
    if(!outfile) return;
    outfile->cd();
    
    std::cout << "Generating VEMCal history file ..." << std::endl;
    
    // Sort moni-files in ascending order (runIDs) 
    std::sort(monifiles.begin(), monifiles.end());
    
    // Create NTuple containing the monitoring values
    TNtupleD* ntuple = new TNtupleD("tree", "VEMCal Monitoring Data",
				    "RunID:StartYear:StartSec:RunDuration:Str:Om:"
				    "IsFirstRun:PEperVEM:MuPeakWidth:MuEmRatio:HGLGCrossOver:"
				    "SPERate:MPERate:PMTVoltage:DOMTemperature"); 
    
    // Loop over moni files
    int prevRunID = -1;
    TFile* moniFile = NULL;
    for(unsigned int i=0; i< monifiles.size(); i++)
    {
        if(moniFile) delete moniFile;
	moniFile = new TFile(monifiles.at(i).c_str(), "READ");
	
	// Get run ID and check range
	int runID = GetRunID(moniFile);
	if(runID < firstRun || runID > lastRun) continue;
	
	// Check run order
	if(prevRunID > 0 && runID <= prevRunID)
	{
	    log_error("Wrong run order or reading the same run twice!");
	    return;
	}
	prevRunID = runID;
	
	// Get run start time
	I3Time runStartTime = GetRunStartTime(moniFile, vemCalTime);
	if(runStartTime.GetUTCYear()==0) continue;
	
	// If runID corresponds to firstRun use VEMCal start time instead
	if(runID == firstRun) runStartTime = vemCalTime;
	
	// Get run duration in seconds
	double runDuration = GetRunDuration(moniFile, vemCalTime);
	if(!finite(runDuration))
	{
	    log_warn("Failed to determine duration of run %d. Skipping it", runID);
	    continue;
	}
	
	// Only use runs of at least 10 minutes duration
	if(runDuration < 600.0) continue;
	
	if(runDuration > 90000.0)
	{
	    log_warn("Run %d seems to be longer that 25 h. This is odd --> Skipping it", runID);
	    continue;
	}

	// Get pointers to monitoring profiles
	TProfile2D* speProf = static_cast<TProfile2D*>(moniFile->Get("SPE"));
	TProfile2D* mpeProf = static_cast<TProfile2D*>(moniFile->Get("MPE"));
	TProfile2D* hvProf  = static_cast<TProfile2D*>(moniFile->Get("HV_read"));
	TProfile2D* tmpProf = static_cast<TProfile2D*>(moniFile->Get("Temperature"));
	
	// Check if first run exist. If not fill just the VEMCal values
	VEMCalValuesMap_t::const_iterator vem_iter;
	if(i==0 && runID!=firstRun)
	{  
	    for(vem_iter = vemValuesMap.begin(); vem_iter != vemValuesMap.end(); ++vem_iter)
	    {
	        const OMKey& omKey = vem_iter->first;
		const VEMCalValues_t& vemValues = vem_iter->second;
		
		ntuple->Fill(firstRun, vemCalTime.GetUTCYear(), vemCalTime.GetUTCSec(), NAN, omKey.GetString(), omKey.GetOM(),
			     1.0, vemValues.pePerVEM, vemValues.muWidth, vemValues.sbRatio, vemValues.hglgCrossOver, NAN, NAN, NAN, NAN);
	    }
	}
	
	// Fill the monitoring and VEMCal values
	for(vem_iter = vemValuesMap.begin(); vem_iter != vemValuesMap.end(); ++vem_iter)
	{
	    const OMKey& omKey = vem_iter->first;
	    const VEMCalValues_t& vemValues = vem_iter->second;
	  
	    double spe = GetMoniValue(omKey, speProf);
	    double mpe = GetMoniValue(omKey, mpeProf);
	    double hv  = 0.5*GetMoniValue(omKey, hvProf); // Voltage is half of the DAC value
	    double tmp = GetMoniValue(omKey, tmpProf);
	    
	    // Check if this is the first occurence of a new VEMCal
	    double isFirstRun = 0.0;
	    if(runID == firstRun) isFirstRun = 1.0;
	    
	    ntuple->Fill(runID, runStartTime.GetUTCYear(), runStartTime.GetUTCSec(), runDuration, omKey.GetString(), omKey.GetOM(),
			 isFirstRun, vemValues.pePerVEM, vemValues.muWidth, vemValues.sbRatio, vemValues.hglgCrossOver, spe, mpe, hv, tmp);
	    
	}
    }
    
    // Delete the last opened moni file
    if(moniFile) delete moniFile;

    outfile->cd();
    ntuple->Write();
}



int main(int argc, char *argv[])
{
    std::vector<std::string> vemcal_files;
    std::vector<std::string> moni_files;
  
    for(int i=1; i<argc; i++)
    {
        std::string arg(argv[i]);
	if(arg.compare(arg.size()-4,arg.size(),".xml")==0) vemcal_files.push_back(argv[i]);
	if(arg.compare(arg.size()-5,arg.size(),".root")==0) moni_files.push_back(argv[i]);
    }
    
    if(vemcal_files.empty())
    {
        std::cout << "Please specify VEMCal xml-file as command line argument!" << std::endl;
	return 0;
    }
    
    if(vemcal_files.size()>1)
    {
        std::cout << "Can only handle on VEMCal file at a time!" << std::endl;
	return 0;
    }
    
    if(moni_files.empty())
    {
        std::cout << "Please specify monitoring root-files as command line argument!" << std::endl;
	return 0;
    }
    
    
    // Extract filename from path
    std::string vemFile = vemcal_files.at(0);
    vemFile.erase(0, vemFile.find_last_of("/") + 1);
    
    int year, month, day, hour, minute, second;
    sscanf(vemFile.c_str(),"VEM_calibration_%4d-%2d-%2d_%2d%2d%2d.xml", &year, &month, &day, &hour, &minute, &second);
    
    char outfileName[200];
    sprintf(outfileName, "VEM_history_%4d-%02d-%02d_%02d%02d%02d.root",  year, month, day, hour, minute, second);
    TFile* outfile = new TFile(outfileName, "RECREATE");
    
    int firstRun = -1;
    int lastRun = -1;
    I3Time startTime;
    VEMCalValuesMap_t vemValuesMap;
    ParseVEMCalFile(vemcal_files[0], vemValuesMap, startTime, firstRun, lastRun);
    
    ReadMoniFiles(outfile, moni_files, vemValuesMap, startTime, firstRun, lastRun);
    
    delete outfile;
      
    return 0;
}
