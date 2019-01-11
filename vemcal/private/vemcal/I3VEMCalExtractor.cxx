/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalExtractor.cxx
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#include <vemcal/I3VEMCalExtractor.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Units.h>
#include <icetray/OMKey.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <tpx/I3TopPulseInfo.h>


I3_MODULE(I3VEMCalExtractor);


using namespace std;


I3VEMCalExtractor::I3VEMCalExtractor(const I3Context& context): I3ConditionalModule(context)
{
    minbiasPulsesName_ = "IceTopMinBiasPulses";
    AddParameter("IceTopMinBiasPulsesName",
                 "Name of IceTop MinBias pulses in frame.",
                 minbiasPulsesName_);

    minbiasPulseInfoName_ = "IceTopMinBiasPulseInfo";
    AddParameter("IceTopMinBiasPulseInfoName",
		 "Name of the IceTop pulse info series connected with IceTopMinBiasPulses.",
		 minbiasPulseInfoName_);
    
    icetopPulsesName_ = "IceTopPulses";
    AddParameter("IceTopPulsesName",
                 "Name of normal IceTop physics pulses in frame.",
                 icetopPulsesName_);

    icetopPulseInfoName_ = "IceTopPulseInfo";
    AddParameter("IceTopPulseInfoName",
		 "Name of the IceTop pulse info series connected with IceTopPulses.",
		 icetopPulseInfoName_);

    showerVetoName_ = "";
    AddParameter("ShowerVeto",
                 "Name of IceTop raw data in frame. If specified it will be used as an air shower veto",
                 showerVetoName_);
    
    vemCalDataName_ = I3DefaultName<I3VEMCalData>::value();
    AddParameter("VEMCalDataName",
                 "Name of the VEMCal data container "
                 "in which the data is written to the frame ",
                 vemCalDataName_);
}


I3VEMCalExtractor::~I3VEMCalExtractor()
{
}


void I3VEMCalExtractor::Configure()
{
    log_info("Configuring I3VEMCalExtractor:");
    
    GetParameter("IceTopMinBiasPulsesName", minbiasPulsesName_);
    log_info("+ Name of IceTop minbias pulses: %s", minbiasPulsesName_.c_str());
    
    GetParameter("IceTopMinBiasPulseInfoName", minbiasPulseInfoName_);
    log_info("+ Name of IceTop minbias pulse info: %s", minbiasPulseInfoName_.c_str());

    GetParameter("IceTopPulsesName", icetopPulsesName_);
    log_info("+ Name of IceTop physics pulses: %s", icetopPulsesName_.c_str());

    GetParameter("IceTopPulseInfoName", icetopPulseInfoName_);
    log_info("+ Name of IceTop pulse info: %s", icetopPulseInfoName_.c_str());

    GetParameter("ShowerVeto", showerVetoName_);
    log_info("+ Air shower veto              : %s", (showerVetoName_.empty()?"DISABLED":showerVetoName_.c_str()));
    
    GetParameter("VEMCalDataName", vemCalDataName_);
}


void I3VEMCalExtractor::DAQ(I3FramePtr frame)
{
    I3EventHeaderConstPtr header = frame->Get<I3EventHeaderConstPtr>();
    if(header)
    {
        I3VEMCalDataPtr vemCalData(new I3VEMCalData());
        vemCalData->runID = header->GetRunID();
        
        FillVEMData(*frame, *vemCalData);
        FillHGLGData(*frame, *vemCalData);
	
        // Only put I3VEMCalData to the frame if it is not empty
        if(!vemCalData->minBiasHits.empty() ||
	   !vemCalData->hglgHits.empty())
	  frame->Put(vemCalDataName_, vemCalData);
    }
    else log_error("Missing I3EventHeader in frame. Doing nothing!");
    
    PushFrame(frame);
}


void I3VEMCalExtractor::FillVEMData(const I3Frame& frame, I3VEMCalData& vemCalData)
{
    // Get IceTop MinBias pulses from the frame
    I3RecoPulseSeriesMapConstPtr minBiasPulses =
	frame.Get<I3RecoPulseSeriesMapConstPtr>(minbiasPulsesName_);
    if(!minBiasPulses) return;

    // GetIceTop MinBias pulse info from the frame
    I3TopPulseInfoSeriesMapConstPtr minBiasPulseInfo = 
      frame.Get<I3TopPulseInfoSeriesMapConstPtr>(minbiasPulseInfoName_);
    if(!minBiasPulseInfo)
      log_info("There are min bias pulses in the frame, but no min bias pulse info!");
    
    // Check if minbias hit is coincident with an air shower
    if(HasLCHits(frame))
    {
        log_trace("Air shower veto --> Skipping minbias hits!"); 
	return;
    }
    
    // Get the detector status from the frame
    I3DetectorStatusConstPtr status = frame.Get<I3DetectorStatusConstPtr>();
    if(!status)
    {
        log_error("Missing I3DetectorStatus in frame!");
        return;
    }
    const std::map<OMKey, I3DOMStatus>& dstm = status->domStatus;
    const I3OMGeoMap& omgeo = frame.Get<I3Geometry>().omgeo;
    
    I3RecoPulseSeriesMap::const_iterator rpsm_iter;
    for(rpsm_iter = minBiasPulses->begin(); rpsm_iter != minBiasPulses->end(); ++rpsm_iter)
    {
        const OMKey& omKey = rpsm_iter->first;
	
	// Get the DOM status for this DOM
	std::map<OMKey, I3DOMStatus>::const_iterator dstm_iter = dstm.find(omKey);
        if(dstm_iter == dstm.end())
        {
            log_error_stream("Missing DOMStatus for " << omKey << ". Skipping it!");
            continue;
        }
        
        // If DOM gain is NOT HIGH try next DOM
	
        if(omgeo.find(omKey)->second.omtype==I3OMGeo::IceTop && dstm_iter->second.domGainType!=I3DOMStatus::High)
	{
	    log_warn_stream(omKey << " is NOT a HIGH GAIN DOM but has the minimum bias hits enabled. Skipping it!");
	    continue;
	}

	// Check if reco pulse series is empty (you never now ...)
	if(rpsm_iter->second.empty())
	{
	    log_warn_stream("Empty I3RecoPulseSeries in module " << omKey << ". Skipping it!");
	    continue;
	}

	// Convert the charge to "deci-photoelectrons" (only use the first pulse. Usually there is only one)
	double charge_dpe = 10.0*rpsm_iter->second.at(0).GetCharge();
	
	// Check if charge is finite, greater zero and within dynamic range of UINT16
	if(!(charge_dpe > 0) || charge_dpe > INT16_MAX) continue;
	
	I3VEMCalData::MinBiasHit minBiasHit;
        minBiasHit.str = static_cast<int8_t>(omKey.GetString());
        minBiasHit.om  = static_cast<int8_t>(omKey.GetOM());

	// Check for pulse info
	if (minBiasPulseInfo) {
	  I3TopPulseInfoSeriesMap::const_iterator piIter = minBiasPulseInfo->find(omKey);
	  if (piIter != minBiasPulseInfo->end() && piIter->second.size() == rpsm_iter->second.size()) {
	    minBiasHit.chip = piIter->second.at(0).sourceID;
	    minBiasHit.channel = piIter->second.at(0).channel;
	  } else {
	    log_error_stream("MinBias pulse info in frame, but no pulse info for pulse in DOM " << omKey);
	  }
	}
		
	// Round to one decimal point (deci photoelectrons) and convert to 16 bit integer 
	// (addition of 0.5 effectively rounds)
        minBiasHit.charge_dpe = static_cast<int16_t>(std::floor(charge_dpe + 0.5));
	
        vemCalData.minBiasHits.push_back(minBiasHit);
    }
}


void I3VEMCalExtractor::FillHGLGData(const I3Frame& frame, I3VEMCalData& vemCalData)
{
    // Get IceTop physics pulses from the frame
    I3RecoPulseSeriesMapConstPtr rpsm =
	frame.Get<I3RecoPulseSeriesMapConstPtr>(icetopPulsesName_);
    if(!rpsm) return;
    
    // GetIceTop MinBias pulse info from the frame
    I3TopPulseInfoSeriesMapConstPtr pulseInfo = 
      frame.Get<I3TopPulseInfoSeriesMapConstPtr>(icetopPulseInfoName_);
    if(!pulseInfo)
      log_info("There are pulses in the frame, but no pulse info!");
    
    // Get the detector status from the frame
    I3DetectorStatusConstPtr status = frame.Get<I3DetectorStatusConstPtr>();
    if(!status)
    {
        log_error("Missing I3DetectorStatus in frame!");
        return;
    }
    const std::map<OMKey, I3DOMStatus>& dstm = status->domStatus;

    const I3OMGeoMap& omgeo=frame.Get<I3Geometry>().omgeo;
    
    // Loop over all (HG) DOMs
    I3RecoPulseSeriesMap::const_iterator hg_rpsm_iter;
    for(hg_rpsm_iter = rpsm->begin(); hg_rpsm_iter != rpsm->end(); ++hg_rpsm_iter)
    {
        // Check if DOM is really an IceTop DOM
        const OMKey& hgKey = hg_rpsm_iter->first;
        if(omgeo.find(hgKey)->second.omtype!=I3OMGeo::IceTop)
	{
	    log_warn_stream(hgKey << " is not an IceTop DOM. Skipping it!");
	    continue;
	}
	
	// Get the DOM status for this DOM
	std::map<OMKey, I3DOMStatus>::const_iterator dstm_iter = dstm.find(hgKey);
        if(dstm_iter == dstm.end())
        {
            log_error_stream("Missing DOMStatus for module " << hgKey << ". Skipping it!");
            continue;
        }
        
        // If DOM gain is NOT HIGH try next DOM
        if(dstm_iter->second.domGainType!=I3DOMStatus::High) continue;
        
	// Check if HG reco pulse series is empty (you never now ...)
	if(hg_rpsm_iter->second.empty())
	{
	    log_warn_stream("Empty I3RecoPulseSeries in module " << hgKey << ". Skipping it!");
	    continue;
	}

        // Create LG OMKey
        OMKey lgKey(hgKey);
        if(hgKey.GetOM() == 61 || hgKey.GetOM() == 63) lgKey.SetOM(hgKey.GetOM() + 1);
        else                                           lgKey.SetOM(hgKey.GetOM() - 1);
        
        // Look if we also have a LG pulse.
        // If not continue and try next HG DOM
        I3RecoPulseSeriesMap::const_iterator lg_rpsm_iter = rpsm->find(lgKey);
        if(lg_rpsm_iter == rpsm->end()) continue;
        
        // Get the DOM status for this (LG) DOM
        dstm_iter = dstm.find(lgKey);
        if(dstm_iter == dstm.end())
        {
            log_error_stream("Missing DOMStatus for module " << lgKey << ". Skipping it!");
            continue;
        }
        
        // If DOM gain is NOT LOW try next DOM
        if(dstm_iter->second.domGainType != I3DOMStatus::Low)
        {
            log_error_stream(lgKey << " is NOT a LOW gain DOM. Skipping it!");
            continue;
        }
	
	// Check if LG pulse series is not empty
        if(lg_rpsm_iter->second.empty())
	{
	    log_warn_stream("Empty I3RecoPulseSeries in module " << lgKey << ". Skipping it!");
	    continue;
	}
	    
	// Use only the first pulses to keep things simple
	// (in most cases there should be only one pulse per pulse series)
        const I3RecoPulse& hgPulse = hg_rpsm_iter->second.at(0); 
        const I3RecoPulse& lgPulse = lg_rpsm_iter->second.at(0); 
        
        // Check if data is valid
	if(!(hgPulse.GetCharge() > 0) || hgPulse.GetCharge() > UINT16_MAX) continue;
	if(!(lgPulse.GetCharge() > 0) || lgPulse.GetCharge() > UINT16_MAX) continue;
	if(!finite(hgPulse.GetTime()) || !(finite(lgPulse.GetTime())))     continue;
        
	I3VEMCalData::HGLGhit hglgHit;
        hglgHit.str   = static_cast<int8_t>(hgKey.GetString());
        hglgHit.hg_om = static_cast<int8_t>(hgKey.GetOM());
	hglgHit.lg_om = static_cast<int8_t>(lgKey.GetOM());
	
	// Round charges to pe and convert to 16 bit unsigned integer (addition of 0.5 effectively rounds)
        hglgHit.hg_charge_pe = static_cast<uint16_t>(std::floor(hgPulse.GetCharge() + 0.5));
	hglgHit.lg_charge_pe = static_cast<uint16_t>(std::floor(lgPulse.GetCharge() + 0.5));
        
	// Calculate time difference in 2ns steps (addition of 0.5 effectively rounds)
	double deltat_2ns = std::floor((hgPulse.GetTime() - lgPulse.GetTime())/(2.0*I3Units::ns) + 0.5);
	
	// If deltat exceeds dynamic range of INT8 set it to min/max value
	deltat_2ns = std::max(deltat_2ns, static_cast<double>(INT8_MIN));
	deltat_2ns = std::min(deltat_2ns, static_cast<double>(INT8_MAX));
	
	hglgHit.deltat_2ns = static_cast<int8_t>(deltat_2ns);

	// Look for pulse info
	if (pulseInfo) {
	  I3TopPulseInfoSeriesMap::const_iterator hgPIIter = pulseInfo->find(hgKey);
	  if (hgPIIter != pulseInfo->end() && hgPIIter->second.size() == hg_rpsm_iter->second.size()) {
	    hglgHit.hg_chip = hgPIIter->second.at(0).sourceID;
	    hglgHit.hg_channel = hgPIIter->second.at(0).channel;
	  } else {
	    log_error_stream("Pulse info in frame, but no pulse info for pulse in DOM " << hgKey);
	  }
	  I3TopPulseInfoSeriesMap::const_iterator lgPIIter = pulseInfo->find(lgKey);
	  if (lgPIIter != pulseInfo->end() && lgPIIter->second.size() == lg_rpsm_iter->second.size()) {
	    hglgHit.lg_chip = lgPIIter->second.at(0).sourceID;
	    hglgHit.lg_channel = lgPIIter->second.at(0).channel;
	  } else {
	    log_error_stream("Pulse info in frame, but no pulse info for pulse in DOM " << lgKey);
	  }
	}
    
        vemCalData.hglgHits.push_back(hglgHit);
    }
}


bool I3VEMCalExtractor::HasLCHits(const I3Frame& frame)
{
    if(showerVetoName_.empty()) return false;
  
    I3DOMLaunchSeriesMapConstPtr dlsm = frame.Get<I3DOMLaunchSeriesMapConstPtr>(showerVetoName_);
    if(!dlsm) return false;
    
    I3DOMLaunchSeriesMap::const_iterator dlsm_iter;
    for(dlsm_iter=dlsm->begin(); dlsm_iter!=dlsm->end(); ++dlsm_iter)
    {
        I3DOMLaunchSeries::const_iterator dls_iter;
	for(dls_iter=dlsm_iter->second.begin(); dls_iter!=dlsm_iter->second.end(); ++dls_iter)
	{
	    if(dls_iter->GetLCBit()) return true;
	}
    }
    
    return false;
}
