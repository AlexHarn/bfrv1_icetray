/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalTreeWriter.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_I3VEMCALTREEWRITER_H_
#define _VEMCAL_I3VEMCALTREEWRITER_H_


#include <icetray/I3ConditionalModule.h>
#include <vemcal/I3VEMCalData.h>
#include <string>

class I3Time;
class TFile;
class TTree;


/**
 * @brief This class collects the I3VEMCalData containers from the frame and
 * stores their contents into individual ROOT files for each run. The rootfiles will
 * contain two trees: The "muonTree" stores the information of the minimum bis hits,
 * whereas the "hglgTree" contains the HG-LG correlation data.  
 */

class I3VEMCalTreeWriter : public I3ConditionalModule 
{
public:
  I3VEMCalTreeWriter(const I3Context& context);

  ~I3VEMCalTreeWriter();

  /// Re-implementation of the Configure method of the I3Module
  void Configure();
  
  /// Re-implementation of the Physics method of the I3Module
  void Physics(I3FramePtr frame);
  
  /// Re-implementation of the Finish method of the I3Module
  void Finish() {Write(); Clear();};
  
private:
  
  /// Deletes all trees and the ROOT file and resets the run number
  void Clear();
  
  /// Writes the trees to the ROOT file
  void Write();
  
  /// Fills the minimum bias hits to the "muonTree"
  void FillVEMTree(const I3Time& startTime,
		   const I3VEMCalData::MinBiasHit& minBiasHit);
  
  /// Fills the HG-LG correlation hits to the "hglgTree"
  void FillHGLGTree(const I3VEMCalData::HGLGhit& hglgHit);
  
  
  /// Name of the I3VEMCalData container in the frame 
  std::string vemCalDataName_;

  /**
   * Base directory where the ROOT files will be stored in separate
   * folders for each day
   */
  std::string workDir_;
  
  /// Base name of the ROOT file e.g. "IceTop_VEMCal"
  std::string fileBaseName_;
  
  /// Run number of the current run
  int currentRunNumber_;
  
  /// Pointer to the ROOT file
  TFile* outfile_;
  
  /// Pointer to the muon tree
  TTree* muonTree_;

  /// Pointer to the HGLG tree
  TTree* hglgTree_;
  
  SET_LOGGER("I3VEMCalTreeWriter");
};



#endif
