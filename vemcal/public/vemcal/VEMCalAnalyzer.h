/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file VEMCalAnalyzer.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_VEMCALANALYZER_H_
#define _VEMCAL_VEMCALANALYZER_H_



#include <icetray/OMKey.h>
#include <dataclasses/I3Time.h>
#include <I3Db/I3OmDb/I3OmDb.h>

#include <vector>
#include <string>
#include <utility>

class TH1F;
class TH2F;
class TFile;

typedef std::map<OMKey, TH1F*> MuonHistMap; 
typedef std::map<OMKey, TH2F*> HGLGHistMap; 


struct VEMCalResult
{
    int gainType;  // 0: HG, 1: LG, -1: Undefined
    double pePerVEM;
    double muonWidth;
    double sbRatio;
    double hglgCrossOver;
    
    int muonFitStatus;
    double muonFitRchi2;

    int hglgFitStatus;
    double hglgFitRchi2;
    
    VEMCalResult():
      gainType(-1),
      pePerVEM(NAN),
      muonWidth(NAN),
      sbRatio(NAN),
      hglgCrossOver(-1),
      muonFitStatus(-1),
      muonFitRchi2(-1),
      hglgFitStatus(-1),
      hglgFitRchi2(-1)
    {};
};

typedef std::map<OMKey, VEMCalResult> VEMCalResultMap;


class VEMCalAnalyzer
{
public:
    VEMCalAnalyzer(const std::string& host  = "dbs2.icecube.wisc.edu",
		   const std::string& user  = "www",
		   const std::string& pword = "",
		   const std::string& dbase = "I3OmDb");
    
    ~VEMCalAnalyzer();
    
    
    void OpenFiles(const std::vector<std::string>& fileList);
    
    void Analyze(const std::string& outDir=".");
    
private:
    
    void AddMuonHist(const OMKey& hgKey, TH1F* hist);
    
    void AddHGLGHist(const OMKey& hgKey, TH2F* hist);
    
    int FitMuonSpectrum(TH1F* hist, double& peak, double& width, double& sbRatio, double& rChi2);
	
    int FitHGLGCorrelation(TH2F* hist, double& lgCorr, double& hgCross, double& rChi2);
    
    inline void Minimum(std::pair<OMKey, double>& min, const OMKey& omKey, double value);
    inline void Maximum(std::pair<OMKey, double>& max, const OMKey& omKey, double value);
      
    bool Summarize(TFile* rootfile, const VEMCalResultMap& results);
    void WriteXML(const VEMCalResultMap& results, bool isGood=true);
    
    TH1F* Smooth(TH1F* hist, int binRange=4);
    void ExpStartValues(TH1F* hist, double x1, double x2, double& norm, double& slope, double offset=0);
    int FindPeakBin(TH1F* hist, double xmin, double xmax, double minval=0, int nsigma=3);
    
    TH1F* ProfileX(TH2F* hist);
    void HGLGStartValues(TH1F* hist, double x1, double x2, double x0, double y0, double& p2, double& p3);

    TH1F* FillVEMHistograms(const VEMCalResultMap& results,
			    const std::pair<OMKey, double>& min_vem_hg,
			    const std::pair<OMKey, double>& max_vem_hg,
			    const std::pair<OMKey, double>& min_vem_lg,
			    const std::pair<OMKey, double>& max_vem_lg);

    TH1F* FillCrossOverHistogram(const VEMCalResultMap& results,
				 const std::pair<OMKey, double>& min_crover,
				 const std::pair<OMKey, double>& max_crover);
      
    TH1F* FillRChi2Histograms(const VEMCalResultMap& results,
			      const std::pair<OMKey, double>& min_rchi2_muon,
			      const std::pair<OMKey, double>& max_rchi2_muon,
			      const std::pair<OMKey, double>& min_rchi2_hglg,
			      const std::pair<OMKey, double>& max_rchi2_hglg);

    
    int firstRunID_;
    int lastRunID_;
    I3Time startTime_;
    bool storeHistos_;
    
    MuonHistMap muonHistos_; 
    
    HGLGHistMap hglgHistos_;

    I3OmDb* omDb_;
};


inline void VEMCalAnalyzer::Minimum(std::pair<OMKey, double>& min, const OMKey& omKey, double value)
{
    if(value<0) return; 
    
    if(min.first.GetOM()==0 || value<min.second)
    {
        min.second = value;
	min.first  = omKey;
    }
};

inline void VEMCalAnalyzer::Maximum(std::pair<OMKey, double>& max, const OMKey& omKey, double value)
{
    if(value<0) return; 
  
    if(max.first.GetOM()==0 || value>max.second)
    {
        max.second = value;
	max.first  = omKey;
    }
};


#endif
