
#include "CascadeVariables/I3VetoModule.h"
#include "recclasses/I3Veto.h"
// icetray/dataclasses stuff
#include "icetray/I3Frame.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Double.h"
#include <boost/assign/std/vector.hpp> // for 'operator+=()'
#include <boost/assert.hpp>
using namespace std;
using namespace boost::assign; // bring 'operator+=()' into scope

I3_MODULE(I3VetoModule);


/*****************************************************************************/


I3VetoModule::I3VetoModule(const I3Context& ctx) :
    I3ConditionalModule(ctx),
    hitmapName_("InIceRawData"),
    outputName_("veto"),
    useAMANDA_(false),
    detectorGeometry_(40), 
    writeFullOutput_(true) {

    AddParameter("HitmapName", 
                 "name of the map containing the detector response",
                 hitmapName_);
    AddParameter("OutputName", 
                 "name of resulting I3Veto object",
                 outputName_);
    AddParameter("UseAMANDA", 
                 "toggle if AMANDA OMs are considered",
                 useAMANDA_);
    AddParameter("DetectorGeometry", 
                 "Detector Geometry: 40, 59, 79, or 86 (for ICXX)",
                 detectorGeometry_);
    AddParameter("FullOutput",
                 "If False, only maxDomChargeContainment and earliestHitCotainment are written to the frame",
                 writeFullOutput_); 
    
    // wiring 
    AddOutBox("OutBox");
}

/*****************************************************************************/

I3VetoModule::~I3VetoModule() {};

/*****************************************************************************/

void I3VetoModule::Configure() {
    GetParameter("HitmapName", hitmapName_);
    GetParameter("OutputName", outputName_);
    GetParameter("UseAMANDA", useAMANDA_);
    GetParameter("DetectorGeometry", detectorGeometry_);
    GetParameter("FullOutput", writeFullOutput_);
    
    layerStrings_ += vector<short>(),vector<short>(),vector<short>();
    
    switch( detectorGeometry_ ) {
    
    case 40:  
      layerStrings_[0] += 63;
      layerStrings_[1] += 39,49,58,66,65,64,71,70,69,61,62,54,55,56,57,48;
      layerStrings_[2] += 21,30,40,50,59,67,74,73,72,78,77,76,75,68,60,52,53,44,45,46,47,38,29;
      break;
    
      
    case 59:   
      layerStrings_[0] += 63,56,57,47,48,37,38,28,19;
      layerStrings_[1] += 69,70,71,64,65,66,58,49,39,29,20,12,11,18,27,83,46,55,54,62,61;
      layerStrings_[2] += 2,3,4,5,6,13,21,30,40,50,59,67,74,73,72,78,77,76,75,68,60,52,53,44,45,36,26,17,10;   
      break;
      
    
    case 79:
      layerStrings_.resize(6);
      layerStrings_[0] += 36;
      layerStrings_[1] += 81,82,83,84,85,86;            // DeepCore
      layerStrings_[2] += 26,27,35,37,44,45,46,47,54;
      layerStrings_[3] += 17,18,19,25,28,34,38,43,48,53,55,56,57,62,63;
      layerStrings_[4] +=  9,10,11,12,16,20,24,29,33,39,42,49,52,58,61,64,65,66,69,70,71;
      layerStrings_[5] +=  2, 3, 4, 5, 6, 8,13,15,21,23,30,32,40,41,50,51,59,60,67,68,72,73,74,75,76,77,78;
      break;
      
      
    case 86:
      layerStrings_.resize(6);
      layerStrings_[0] += 36;
      layerStrings_[1] += 79,80,81,82,83,84,85,86;      // DeepCore
      layerStrings_[2] += 25,26,27,37,47,46,45,54,44,34,35;
      layerStrings_[3] += 16,17,18,19,28,38,48,57,56,55,63,62,53,43,33,24;
      layerStrings_[4] +=  8, 9,10,11,12,20,29,39,49,58,66,65,64,71,70,69,61,52,42,32,23,15;
      layerStrings_[5] +=  1, 2, 3, 4, 5, 6,13,21,30,40,50,59,67,74,73,72,78,77,76,75,68,60,51,41,31,22,14, 7;
      break;
      

    default: 
      log_fatal("DetectorGeometry %i is not implemented (only 40 (IC40), 59 (IC59), 79 (IC79), and 86 (IC86) are available up to now)! What crazy geometry do you want to use?", 
		detectorGeometry_);
         
    }
    
}

    
/*****************************************************************************/

void I3VetoModule::Physics(I3FramePtr frame) {
    I3RecoPulseSeriesMapConstPtr hitmap = frame->Get<I3RecoPulseSeriesMapConstPtr>(hitmapName_);
    I3GeometryConstPtr geometry = frame->Get<I3GeometryConstPtr>();

    if (!hitmap) {
        log_warn("%s: no I3RecoPulseSeriesMap with name %s in frame! skipping this event", GetName().c_str(),
                 hitmapName_.c_str());
        PushFrame(frame); 
        return;
    }
    
    // do the calculations
    I3VetoDetail::VetoParams resultCollection = CalcFromPulseMap(hitmap, geometry, layerStrings_, useAMANDA_);
    
    // fill results (full or reduced) into frame object
    if( writeFullOutput_ ) {
        log_info("Writing full I3Veto output...");
        I3VetoPtr veto = I3VetoPtr(new I3Veto());
        
        veto->nUnhitTopLayers =         resultCollection.nUnhitTopLayers;
        veto->nLayer =                  resultCollection.nLayer;
        veto->earliestLayer =           resultCollection.earliestLayer;
        veto->earliestOM =              resultCollection.earliestOM;
        veto->earliestContainment =     resultCollection.earliestContainment;
        veto->latestLayer =             resultCollection.latestLayer;
        veto->latestOM =                resultCollection.latestOM;
        veto->latestContainment =       resultCollection.latestContainment;
        veto->mostOuterLayer =          resultCollection.mostOuterLayer;
        veto->depthHighestHit =         resultCollection.depthHighestHit;
        veto->depthFirstHit =           resultCollection.depthFirstHit;
        veto->maxDomChargeLayer =       resultCollection.maxDomChargeLayer;
        veto->maxDomChargeString =      resultCollection.maxDomChargeString;
        veto->maxDomChargeOM =          resultCollection.maxDomChargeOM;
        veto->nDomsBeforeMaxDOM =       resultCollection.nDomsBeforeMaxDOM;
        veto->maxDomChargeLayer_xy =    resultCollection.maxDomChargeLayer_xy;
        veto->maxDomChargeLayer_z =     resultCollection.maxDomChargeLayer_z;
        veto->maxDomChargeContainment = resultCollection.maxDomChargeContainment;
        
        frame->Put(outputName_, veto);
    }
    else {
        //log_fatal("Reduced Output not implemented yet!");
        
        log_info("Writing reduced I3VetoShort output...");
        I3VetoShortPtr shortVeto = I3VetoShortPtr(new I3VetoShort());
        
        shortVeto->earliestContainment =     resultCollection.earliestContainment;
        shortVeto->maxDomChargeContainment = resultCollection.maxDomChargeContainment;
        
        frame->Put(outputName_, shortVeto);
        
    }


//    veto->CalcFromPulseMap(hitmap, geometry, layerStrings_, useAMANDA_);
    
    
    
    PushFrame(frame); 
}

/*****************************************************************************/

I3VetoDetail::VetoParams I3VetoModule::CalcFromPulseMap(I3RecoPulseSeriesMapConstPtr pulsemap, 
			I3GeometryConstPtr geometry,
			const std::vector<std::vector<short> >& layers,
			bool useAMANDA) {		
    
    // get some iterators
    I3RecoPulseSeriesMap::const_iterator i_pmap;
    I3RecoPulseSeries::const_iterator i_pls;
    std::map<OMKey, I3OMGeo>::const_iterator i_geo;

    // comparison variables for reconstruction
    unsigned short smallestDOMNumber = 1; 
    double earliestTime = +HUGE_VAL;
    double latestTime = -HUGE_VAL;
    double maxCharge = -1;
    std::set<short> layerset;
    // separate treatment of horizontal and vertical layers
    std::set<short> layerset_xy;
    std::set<short> layerset_z;

    // manage results from DOMs that have already been checked
    std::vector<I3VetoDetail::DOMInfo> domInformation;
    
    // container for results
    I3VetoDetail::VetoParams theResults;

    for (i_pmap = pulsemap->begin(); i_pmap != pulsemap->end(); ++i_pmap) {
        const OMKey& omkey = i_pmap->first;
        const I3RecoPulseSeries& pulses = i_pmap->second;
	
	    if (!useAMANDA && (omkey.GetString() < 0))
            continue;
       
        if ( (i_geo = geometry->omgeo.find(omkey)) == geometry->omgeo.end() ) {
            log_warn("skipping DOM (%02d,%02d) which is in the I3RecoPulseSeriesMap but not in the geometry!",
                     omkey.GetString(), omkey.GetOM());
            continue;
        }

        const double om_z = i_geo->second.position.GetZ();

        // check z-position of this DOM
        if (omkey.GetOM() < smallestDOMNumber)
            smallestDOMNumber = omkey.GetOM();

        if (om_z > theResults.depthHighestHit)
            theResults.depthHighestHit = om_z;
	
        // determine the xy layer of this DOM
        short layer_xy = -1;
        for (size_t i = 0; i < layers.size(); ++i) {
            if ( find(layers[i].begin(), layers[i].end(), omkey.GetString()) != layers[i].end() ) {
                layer_xy = i;
                break;
            }
        }
        if ( layer_xy != -1 )
            layerset_xy.insert(layer_xy);
        else log_warn("couldn't attribute OM(%d,%d) to any xy layer)", omkey.GetString(), omkey.GetOM());
        
        // determine z layer of this DOM
        short layer_z = -1;
        for (size_t i = 0; i < layers.size(); ++i) {
            if ( ( omkey.GetOM() >= ( 5*(layers.size() - i - 1 ) ) ) &&            // z layers at the top of the detector consist of 5 DOMs each,
                                                                              // so that their thickness is comparable to the xy layer thickness
                    ( omkey.GetOM() <= ( 60 - layers.size() + i + 1 ) ) ) {   // z layers at the bottom have only one DOM - don't need that much shielding from below
                layer_z = i;
                break;
            }
        }
        if ( layer_z != -1 )
            layerset_z.insert(layer_z);
        else log_warn("couldn't attribute OM(%d,%d) to any z layer)", omkey.GetString(), omkey.GetOM());

        short layer = -1;
        if (layer_xy >= layer_z) layer = layer_xy;
        else layer = layer_z;

        if (layer != -1)
            layerset.insert(layer);
        if (layer > theResults.mostOuterLayer)
            theResults.mostOuterLayer = layer;
        if (layer == -1)
            log_warn("couldn't attribute OM(%d,%d) to any layer at all)", omkey.GetString(), omkey.GetOM());


        // loop over this DOM's pulsemap
        // search for the event-wise timely first and last DOM as 
        // well as for for the DOM with the highest charge. 
        double charge = 0;
        for (i_pls = pulses.begin(); i_pls != pulses.end(); ++i_pls) {
            charge += i_pls->GetCharge();
            if (i_pls->GetTime() < earliestTime) {
                earliestTime = i_pls->GetTime();
                theResults.earliestLayer = layer_xy;
                theResults.earliestOM = omkey.GetOM();
                theResults.depthFirstHit = om_z;
            }
            if (i_pls->GetTime() > latestTime) {
                latestTime = i_pls->GetTime();
                theResults.latestLayer = layer_xy;
                theResults.latestOM = omkey.GetOM();
            }
        }

        //get Layer with highest charge Dom and the number of the highest charge Dom
        if ( (charge > maxCharge) && ( charge != 0 ) ) {
            maxCharge = charge;
            theResults.maxDomChargeString = omkey.GetString();
            theResults.maxDomChargeOM = omkey.GetOM();
            theResults.maxDomChargeLayer = layer;
            theResults.maxDomChargeLayer_xy = layer_xy;                        
            theResults.maxDomChargeLayer_z = layer_z;
        }
        
        // remember DOM-wise information; skip DOMs without pulses
        if (pulses.size() > 0) {
            I3VetoDetail::DOMInfo info;
            info.key = omkey;
            info.charge = charge;
            info.firstPulseTime = pulses.begin()->GetTime();
            domInformation.push_back(info);
        }
    }
    
    // calculate containment variables
    theResults.maxDomChargeContainment = 100*theResults.maxDomChargeLayer_xy + theResults.maxDomChargeOM;
    theResults.earliestContainment = 100*theResults.earliestLayer + theResults.earliestOM;
    theResults.latestContainment = 100*theResults.latestLayer + theResults.latestOM;

    // calculate number of hit and unhit (top-) layers
    theResults.nLayer = layerset.size();
    theResults.nUnhitTopLayers = smallestDOMNumber - 1;

    log_info("OM(%d,%d) with max charge is in layer %d", theResults.maxDomChargeString, 
            theResults.maxDomChargeOM, theResults.maxDomChargeLayer ); 
    
    // calculate number of DOMs hit before the DOM with the highest charge

    std::sort(domInformation.begin(), domInformation.end());
    std::vector<I3VetoDetail::DOMInfo>::const_iterator i_info;
    
    theResults.nDomsBeforeMaxDOM = 0;
    for(i_info=domInformation.begin(); i_info != domInformation.end(); ++i_info) {
        const OMKey& key = i_info->key;
        if ( (key.GetOM() == (unsigned)theResults.maxDomChargeOM) && (key.GetString() == theResults.maxDomChargeString) ) 
            break;
        else
            ++theResults.nDomsBeforeMaxDOM;
    }
	
	return theResults;		
}			
