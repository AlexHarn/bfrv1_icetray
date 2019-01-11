/**
    copyright  (C) 2013
    the IceCube Collaboration
    $Id: FillRatio converter from Frame Info Object to StringDoubleMap  $

    @version $Revision: 19577284585834053.0 $
    @author Chang Hyon Ha (cuh136 @ phys . psu . edu)
*/
#include "fill-ratio/I3FillRatio2StringDoubleMap.h"
#include "fill-ratio/I3FillRatioModule.h"
#include "recclasses/I3FillRatioInfo.h"
#include "dataclasses/I3Map.h"
#include <iostream>
#include <string> 

using namespace std;
I3_MODULE(I3FillRatio2StringDoubleMap);
I3FillRatio2StringDoubleMap::I3FillRatio2StringDoubleMap(const I3Context& ctx) : 
  I3Module(ctx),
  fillratio_output_name_("FRInfo"),
  fillratiobox_("FillRatioBox")
{
  AddOutBox("OutBox");
  AddParameter("FillRatioOutputName","Name of Fill Ratio Object in the frame",fillratio_output_name_);	
  AddParameter("ResultStringDoubleMap","Name of String Double Map Name",fillratiobox_);	
  log_debug("Enter I3FillRatio2StringDoubleMap::I3FillRatio2StringDoubleMap().");
}

I3FillRatio2StringDoubleMap::~I3FillRatio2StringDoubleMap() 
{
}

void I3FillRatio2StringDoubleMap::Configure()
{
  GetParameter("FillRatioOutputName",fillratio_output_name_);	
  GetParameter("ResultStringDoubleMap",fillratiobox_);	
}

void I3FillRatio2StringDoubleMap::Physics(I3FramePtr frame)
{
  log_debug("Entering I3FillRatio2StringDoubleMap Module ... ");

  I3MapStringDoublePtr fillratio_box(new I3MapStringDouble);
  fillratio_box->clear();

  (*fillratio_box)["MeanDistance"]               = -9999;  // initialize the box
  (*fillratio_box)["RMSDistance"]                = -9999;  // initialize the box
  (*fillratio_box)["NChDistance"]                = -9999;  // initialize the box
  (*fillratio_box)["EnergyDistance"]             = -9999;  // initialize the box

  (*fillratio_box)["FillRadius"]                 = -9999;  // initialize the box
  (*fillratio_box)["FillRadiusFromMean"]         = -9999;  // initialize the box
  (*fillratio_box)["FillRadiusFromEnergy"]       = -9999;  // initialize the box
  (*fillratio_box)["FillRadiusFromNCh"]          = -9999;  // initialize the box
  (*fillratio_box)["FillRadiusFromMeanPlusRMS"]  = -9999;  // initialize the box

  (*fillratio_box)["FillRatio"]                  = -9999;  // initialize the box
  (*fillratio_box)["FillRatioFromMean"]          = -9999;  // initialize the box
  (*fillratio_box)["FillRatioFromMeanPlusRMS"]   = -9999;  // initialize the box
  (*fillratio_box)["FillRatioFromNCh"]           = -9999;  // initialize the box
  (*fillratio_box)["FillRatioFromEnergy"]        = -9999;  // initialize the box

  (*fillratio_box)["HitCount"]                   = -9999;  // initialize the box

  I3FillRatioInfoConstPtr fillRatioInfo =  frame->Get<I3FillRatioInfoConstPtr>(fillratio_output_name_);

  if(fillRatioInfo) {

    meanDistance_               =   fillRatioInfo->GetMeanDistance();
    rmsDistance_                =   fillRatioInfo->GetRMSDistance();
    nchDistance_                =   fillRatioInfo->GetNChDistance();
    energyDistance_             =   fillRatioInfo->GetEnergyDistance();

    fillRadius_                 =   fillRatioInfo->GetFillRadiusFromRMS();
    fillRadiusFromMean_         =   fillRatioInfo->GetFillRadiusFromMean();
    fillRadiusFromEnergy_       =   fillRatioInfo->GetFillRadiusFromEnergy();
    fillRadiusFromNCh_          =   fillRatioInfo->GetFillRadiusFromNCh();
    fillRadiusFromMeanPlusRMS_  =   fillRatioInfo->GetFillRadiusFromMeanPlusRMS();

    fillRatio_                  =   fillRatioInfo->GetFillRatioFromRMS();
    fillRatioFromMean_          =   fillRatioInfo->GetFillRatioFromMean();
    fillRatioFromEnergy_        =   fillRatioInfo->GetFillRatioFromEnergy();
    fillRatioFromNCh_           =   fillRatioInfo->GetFillRatioFromNCh();
    fillRatioFromMeanPlusRMS_   =   fillRatioInfo->GetFillRatioFromMeanPlusRMS();

    hitCount_                   =   fillRatioInfo->GetHitCount();

    log_debug("mD=%f :rD=%f :nD=%f :eD=%f",                    meanDistance_,    rmsDistance_,    nchDistance_,    energyDistance_);
    log_debug("rd=%f :rd_m=%f :rd_e=%f :rd_n=%f :rd_mr=%f",    fillRadius_,    fillRadiusFromMean_,    fillRadiusFromEnergy_,    fillRadiusFromNCh_,    fillRadiusFromMeanPlusRMS_);
    log_debug("rt=%f :rt_m=%f :rt_e=%f :rt_n=%f :rt_mr=%f",    fillRatio_,    fillRatioFromMean_,    fillRatioFromEnergy_,    fillRatioFromNCh_,    fillRatioFromMeanPlusRMS_);

  } else {
    log_fatal("%s DOES NOT EXIST",fillratio_output_name_.c_str());
  }


  (*fillratio_box)["MeanDistance"]               = meanDistance_;		 
  (*fillratio_box)["RMSDistance"]                = rmsDistance_;		 
  (*fillratio_box)["NChDistance"]                = nchDistance_;		 
  (*fillratio_box)["EnergyDistance"]             = energyDistance_;		 

  (*fillratio_box)["FillRadius"]                 = fillRadius_;		 
  (*fillratio_box)["FillRadiusFromMean"]         = fillRadiusFromMean_;		 
  (*fillratio_box)["FillRadiusFromEnergy"]       = fillRadiusFromEnergy_;	 
  (*fillratio_box)["FillRadiusFromNCh"]          = fillRadiusFromNCh_;		 
  (*fillratio_box)["FillRadiusFromMeanPlusRMS"]  = fillRadiusFromMeanPlusRMS_;	 
  (*fillratio_box)["FillRatio"]                  = fillRatio_;		 

  (*fillratio_box)["FillRatioFromMean"]          = fillRatioFromMean_;		 
  (*fillratio_box)["FillRatioFromMeanPlusRMS"]   = fillRatioFromMeanPlusRMS_;	 
  (*fillratio_box)["FillRatioFromNCh"]           = fillRatioFromNCh_;		 
  (*fillratio_box)["FillRatioFromEnergy"]        = fillRatioFromEnergy_;	 

  (*fillratio_box)["HitCount"]                   = hitCount_;                    

  frame->Put(fillratiobox_, fillratio_box);
  PushFrame(frame,"OutBox");
  log_debug("Exiting I3FillRatio2StringDoubleMap Module.");
}

