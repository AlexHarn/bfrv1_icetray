/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalHistWriter.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_I3VEMCALHISTWRITER_H_
#define _VEMCAL_I3VEMCALHISTWRITER_H_


#include <icetray/I3ConditionalModule.h>
#include <icetray/OMKey.h>
#include <vemcal/I3VEMCalData.h>
#include <string>

class I3Time;
class TFile;
class TH1F;
class TH2F;

typedef std::map<OMKey, TH1F*> MuonHistMap; 
typedef std::map<OMKey, TH2F*> HGLGHistMap; 


/**
 * @brief This module reads the information from the I3VEMCalData containers and 
 * generates histohrams which are stored in one rootfile per run.  
 */

class I3VEMCalHistWriter : public I3ConditionalModule 
{
public:
  I3VEMCalHistWriter(const I3Context& context);

  ~I3VEMCalHistWriter();

  /// Re-implementation of the Configure method of the I3Module
  void Configure();
  
  /// Re-implementation of the Physics method of the I3Module
  void Physics(I3FramePtr frame);
  
  /// Re-implementation of the Finish method of the I3Module
  void Finish() {Write(); Clear();};
  
private:
  
  /// Deletes all histograms, the root file and resets the run number
  void Clear();
  
  /// Writes the histograms to the root file
  void Write();
  
  /// Fills the minimum bias hits into 1D-histograms
  void FillMuonHistos(const I3VEMCalData::MinBiasHit& minBiasHit);
  
  /// Fills the HG-LG correlation hits into 2D-histograms
  void FillHGLGHistos(const I3VEMCalData::HGLGhit& hglgHit);
  
  
  /// Name of the I3VEMCalData container in the frame 
  std::string vemCalDataName_;
  
  /// Base directory where the ROOT files will be stored in separate folders for each day
  std::string workDir_;
  
  /// Base name of the ROOT file e.g. "IceTop_VEMCalData"
  std::string fileBaseName_;
  
  /// Parameters for histogram binning
  std::vector<double> muonBinning_;
  std::vector<double> hglgBinning_;
  
  /// Upper limit for the time difference between HG and LG hits in the same tank
  double maxTimeDiff_;
  
  /// Parameters to configure database access
  std::string dbHost_;
  std::string dbUserName_;
  std::string dbPassword_;
  std::string dbDatabase_;
  
  /// Run number of the current run
  int currentRunNumber_;
  
  /// Pointer to the ROOT file
  TFile* outfile_;
  
  MuonHistMap muonHistos_; 
  HGLGHistMap hglgHistos_;
  
  SET_LOGGER("I3VEMCalHistWriter");
};



#endif
