/**
 * class: I3InIceDOMObject
 *
 * @version $Id: $
 *
 * @date: $Date: $
 *
 * @author Samuel Flis <samuel.d.flis@gmail.com>
 *
 * (c) 2011,2012 IceCube Collaboration
 */

#include <cfloat>
#include <queue>
#include <boost/foreach.hpp>

#include "phys-services/I3RandomService.h"
#include "dataclasses/I3DOMFunctions.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/geometry/I3Geometry.h"

#include "I3InIceDOM.h"
double const I3InIceDOM::cableCorrectionInIce    = 2325*I3Units::ns;//[ns]
double const I3InIceDOM::cableCorrectionDeepCore = 2250*I3Units::ns;//[ns]

I3InIceDOM::I3InIceDOM(){}
I3InIceDOM::I3InIceDOM(I3RandomServicePtr rng, const OMKey& om): I3DOM(rng, om) {}
I3InIceDOM::I3InIceDOM(boost::shared_ptr<I3RandomService> rng, 
                       const OMKey& om, 
                       double &globlaTime,
                       domlauncherutils::PulseTemplateMap &speTemplateMap):
                       I3DOM(rng, om, globlaTime, speTemplateMap) {}
                         
bool I3InIceDOM::Configure(const I3DOMCalibration& cal,
                           const I3DOMStatus& stat){

  bool success(true);
  // This .93 magic number will scale the discrimintor down to 0.2325PE.
  // Magic number needs to be replaced by a proper
  // handling of the SPEPMTThreshold
  double threshold = SPEPMTThreshold(stat, cal);
  if(std::isnan(threshold)){
    log_error("threshold = %f", threshold);
    success = false;
  }

  success &= I3DOM::Configure(cal, stat, threshold);

  //Hardcoded normal in-ice DOMs as strings < 79
  if(domId_.GetString() < 79){
    cableCorrection_ = I3InIceDOM::cableCorrectionInIce;
  }
  else{ //DeepCore is strings > 78
    cableCorrection_ = I3InIceDOM::cableCorrectionDeepCore;
  }
  return success;
}

void I3InIceDOM::CreateLCLinks(const I3DOMMap& domMap,const I3OMGeoMap &domGeo){

  domNeighbors_.clear();

  std::vector<OMKey> neighbor_keys;
  for(unsigned distance(domStat_.lcSpan); distance > 0; --distance){
    if(domStat_.lcMode == I3DOMStatus::Up ||
       domStat_.lcMode == I3DOMStatus::UpOrDown){
      // copy the original DOM, so the PMT number stays the same
      OMKey neighbor_key(domId_);
      neighbor_key.SetOM(domId_.GetOM() - distance);
      if(domGeo.find(neighbor_key)->second.omtype == I3OMGeo::IceCube)
        neighbor_keys.push_back(neighbor_key);
    }
    if(domStat_.lcMode == I3DOMStatus::Down ||
       domStat_.lcMode == I3DOMStatus::UpOrDown){
      OMKey neighbor_key(domId_);
      neighbor_key.SetOM(domId_.GetOM() + distance);
      if(domGeo.find(neighbor_key)->second.omtype == I3OMGeo::IceCube) 
        neighbor_keys.push_back(neighbor_key);
    }
  }

  BOOST_FOREACH(const OMKey& neighbor_key, neighbor_keys){
    if( domMap.find(neighbor_key) != domMap.end() ){
      I3DOMMap::const_iterator iter = domMap.find(neighbor_key);
      I3InIceDOMPtr obj = boost::dynamic_pointer_cast<I3InIceDOM>(iter->second);
      if(obj)
        domNeighbors_.push_back(obj.get());
      else
        log_fatal("Only InIce neighbors for now.");
    }
  }
}

void I3InIceDOM::MakeCoarseChargeStamp(const dlud::DiscCross& discrx,
                                       int chip,
                                       I3DOMLaunch &domLaunch){
   //One extra trailing bin must be digitized in case the last bin checked
   //(nFADCCoarseChargeStampBins-1) is the highest and we need to report 
   //the sample after it. 
   double analogReadOut[nFADCCoarseChargeStampBins+1];
   std::vector<int>& coarseChargeStamp = domLaunch.GetRawChargeStamp();
   int max = 0;
   uint index = 0;

   // If Launch is HLC we already have a raw FADC waveform and we only need to find the largest
   //bin and its neighbors in the 16 first bins.
   if(discrx.type == dlud::HLC || discrx.type == dlud::CPU_REQUESTED){
      std::vector<int> &rawWaveForm = domLaunch.GetRawFADC();
      for(uint i = 0; i < nFADCCoarseChargeStampBins+1; i++){
         analogReadOut[i] = rawWaveForm[i];
         //only the bins [1,nFADCCoarseChargeStampBins] are checked for being the maximum
         if(i>0 && i<nFADCCoarseChargeStampBins && analogReadOut[i]>max){
            max = rawWaveForm[i];
            index = i;
         }
      }
   }
    else if(discrx.type == dlud::SLC){//If launch is SLC we need to compute the 17 bin
                                //long waveform to decide which bin in the largest.
        double gain = domCal_.fadcGain;
        double transitDeltaTime = transitTime_ - domCal_.fadcDeltaT;
        double binLength = clockCycle;
        double norm = speMean_ * domCal_.frontEndImpedance;

        if(std::isnan(speMean_)){
            log_warn("SPEMean is NAN for DOM %s:",domId_.str().c_str());
            return;
        }
        if(std::isnan(gain)){
            log_warn("ATWD gain is NAN for DOM %s:",domId_.str().c_str());
            return;
        }
        //making sure the array is reset to zero
        for(uint i = 0; i < nFADCCoarseChargeStampBins+1; i++) analogReadOut[i] = 0;
    

      for(uint i = 0; i < nFADCCoarseChargeStampBins+1; i++){
        double binTime = discrx.time + (i+1)*binLength;
        double t = binTime - transitDeltaTime;
        analogReadOut[i] += norm * waveform_.WaveFormAmplitude(t,*fadcSPETemplatePtr_);         
      }

      //digitizing waveform
      for(uint i = 0; i < nFADCCoarseChargeStampBins+1; i++){

         // convert from GV to counts
         analogReadOut[i] /= gain;
         // Adding electronic noise. The parameters are taken from the old DOMSimulator.
         analogReadOut[i] += rng_->Gaus(FADCNoiseMean,FADCNoiseVariance);
         analogReadOut[i] += domCal_.fadcBeaconBaseline;
         analogReadOut[i] = int(analogReadOut[i]);//floor the number of counts

         //10 bits, 2^10-1
         if(analogReadOut[i] > digitizerDynamicRange) analogReadOut[i] = digitizerDynamicRange;
         if(analogReadOut[i] < 0) analogReadOut[i] = 0;
         //only the bins [1,nFADCCoarseChargeStampBins] are checked for being the maximum
         if(i>0 && i<nFADCCoarseChargeStampBins && max<analogReadOut[i]){
            max = int(analogReadOut[i]);
            index = i;
      }
    }

  }
  assert(index>0 && index<nFADCCoarseChargeStampBins);

  coarseChargeStamp.push_back(int(analogReadOut[index - 1]));
  coarseChargeStamp.push_back(int(analogReadOut[index]));
  coarseChargeStamp.push_back(int(analogReadOut[index + 1]));

  domLaunch.SetChargeStampHighestSample(index);

  //Extra trickiness: Charge stamps are stored in payloads as 3 9-bit integers
  //plus one extra 'exponent' bit.
  //For stamps containing only values which fit in 9 bits this is lossless, but
  //if any sample needs the high bit set, the lowest bits must be sacrificed
  bool dropLowBit=false;
  for(unsigned int i = 0; i < coarseChargeStamp.size(); i++){
    if((coarseChargeStamp[i])>>(digitizerBits-1))
      dropLowBit = true;
  }
  if(dropLowBit){
    for(unsigned int i = 0; i < coarseChargeStamp.size(); i++)
      coarseChargeStamp[i] &= ~1u;
  }
}

