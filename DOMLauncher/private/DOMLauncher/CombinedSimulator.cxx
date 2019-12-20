#include "DOMLauncher/CombinedSimulator.h"

#include <cmath>
#include <cassert>
#include <numeric>
#include <limits>
#include <algorithm>
#include <stack>

#include <boost/make_shared.hpp>
#include <boost/math/constants/constants.hpp>

#include <icetray/I3Units.h>
#include <simclasses/I3MCPE.h>
#include <simclasses/I3MCPulse.h>
#include <sim-services/MCPEMCPulseTools.hpp>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/calibration/I3Calibration.h>

#include "discreteDistribution.h"
#include <iostream>
#include <fstream>
#include <cmath>

#include <boost/foreach.hpp>

#include <simclasses/I3ParticleIDMap.hpp>

#include "DOMLauncher/DOMLauncher.h"
#include "DOMLauncher/I3DOM.h"
#include "DOMLauncher/I3InIceDOM.h"
#include "DOMLauncher/I3IceTopDOM.h"


using namespace domlauncherutils;
I3_MODULE(CombinedSimulator);

CombinedSimulator::CombinedSimulator(const I3Context& context):
	I3ConditionalModule(context),
	inputHitsName_("I3MCPESeriesMap"),
	useSPEDistribution_(true),
	usePMTJitter_(true),
	prePulseProbability_(0.003), //http://icecube.wisc.edu/~chwendt/talks/PMTChargeSpectrumVsTime-Ghent2007.pdf
	latePulseProbability_(0.035),
	afterPulseProbability_(0.0593), //http://icecube.wisc.edu/~chwendt/afterpulse-notes/
	applySaturation_(true),
	mergeHits_(true),
	lowMem_(false),
    domMapInitialized_(false),
randomServiceName_("I3RandomService")
{
	//For use if module description strings become supported in icetray
	/*SetDescription("A module which simulates the behaviour of a PMT\n"
	               "\n"
	               "This module converts 'raw' MC PEs (representing integer numbers of \n"
	               "photoelectrons ejected from the photocathode of the PMT) into 'processed' \n"
	               "MC Hits whose weights are proportional to the amount of current produced \n"
	               "by the PMT at the anode and with times randomly shifted according to \n"
	               "appropriate probability distributions. The hits produced by this module \n"
	               "are sorted in time order. \n"
	               "\n"
	               "The weights of the processed hits are still in units of photoelectrons, \n"
	               "and so must be multiplied by the PMT gain when convolved with digitizer \n"
	               "pulse templates to produce electronics readouts. Similarly, the times of \n"
	               "the hits do not include the 'transit time' of the DOM (which includes \n"
	               "both the PMT transit time and delays in the mainboard) so this time must \n"
	               "be added when computing digitizer outputs. \n"
	               "\n"
	               "Note: This module is intended for use with the I3PhotonicsHitMaker and \n"
	               "      DOMLauncher modules; it is not compatible with I3HitMaker and \n"
	               "      I3DOMsimulator. ");*/

	AddParameter("Input",
	             "The name of the I3MCPESeriesMap to process.",
	             inputHitsName_);

    domLaunchMapName_ = "I3DOMLaunchSeriesMap";
    AddParameter("Output","The output name of the DOMLaunchSeries.",
                domLaunchMapName_);

	AddParameter("UseSPEDistribution",
	             "Whether hit charges should be drawn from the proper charge distribution or left ideal",
	             useSPEDistribution_);
	AddParameter("UsePMTJitter",
	             "Whether the times of pulses should be randomly perturbed",usePMTJitter_);
	AddParameter("PrePulseProbability",
	             "The probability that a pulse arrives early",prePulseProbability_);
	AddParameter("LatePulseProbability",
	             "The probability that a pulse arrives late",latePulseProbability_);
	AddParameter("AfterPulseProbability",
	             "The probability that a pulse produces an accompanying afterpulse",
	             afterPulseProbability_);
	AddParameter("ApplySaturation",
	             "Whether the weights of the hits should be modified to account for saturation",
	             applySaturation_);
	AddParameter("MergeHits", "Whether hits very near in time should be merged in the output",
	             mergeHits_);
	AddParameter("LowMem", "If true, attempt to reduce peak memory use by repeatedly merging hits "
	             "as they are generated. Use of this option requires that MergeHits be true.\n"
	             "WARNING: Use of this option may slightly reduce precision and drastically "
	             "increase running time. It is potentially useful for very bright events, and "
	             "probably harmful for very long events.\n"
	             "Please only use this if you know what you are doing, which probably requires "
	             "benchmarking both runtime and peak memory use.",
	             lowMem_);
	AddParameter("RandomServiceName",
		     "Name of the random service in the context.",
		     randomServiceName_);

    snMode_ = false;
    AddParameter("SuperNovaMode","Outputs discriminators in a I3MapKeyVectorDouble",
                snMode_);

    multiFrameEvents_ = false;
    AddParameter("MultiFrameEvents","Does not reset the DOMs on each frame",
                multiFrameEvents_);

    tabulatePTs_ = true;
    AddParameter("UseTabulatedPT","Pulse templates used in simulation are interpolated for better "
                "speed performance with the cost of worse precision and larger memory consumption.",
                tabulatePTs_);

    mergePulses_ = true;
    AddParameter("MergePulses","If pulses in the past should be merged. Speeds up considerably "
                "if the number of pulses per DOM to process is large.",
                mergePulses_);

    droopedPulseTemplates_ = true;
    AddParameter("DroopedPulses","If drooped SPE pulse templates should be used.",
                droopedPulseTemplates_);

    beaconLaunches_ = true;
    AddParameter("BeaconLaunches","If beacon launches should be simulated.",
                beaconLaunches_);
    
    
    beaconLaunchRate_ = 0.6*I3Units::hertz;
    AddParameter("BeaconLaunchRate","The rate of beacon launches.",
                beaconLaunchRate_);

	AddOutBox("OutBox");
}

void CombinedSimulator::Configure(){
	GetParameter("Input",inputHitsName_);
    GetParameter("Output",domLaunchMapName_);
	GetParameter("UseSPEDistribution",useSPEDistribution_);
	GetParameter("UsePMTJitter",usePMTJitter_);
	GetParameter("PrePulseProbability",prePulseProbability_);
	GetParameter("LatePulseProbability",latePulseProbability_);
	GetParameter("AfterPulseProbability",afterPulseProbability_);
	GetParameter("ApplySaturation",applySaturation_);
	GetParameter("MergeHits",mergeHits_);
	GetParameter("LowMem",lowMem_);
	GetParameter("RandomServiceName",randomServiceName_);
    GetParameter("SuperNovaMode", snMode_);
    GetParameter("UseTabulatedPT", tabulatePTs_);
    GetParameter("MergePulses", mergePulses_);
    GetParameter("DroopedPulses",droopedPulseTemplates_);
    GetParameter("MultiFrameEvents",multiFrameEvents_);
    GetParameter("BeaconLaunches",beaconLaunches_);
    GetParameter("BeaconLaunchRate",beaconLaunchRate_);
	
	randomService_ = context_.Get<I3RandomServicePtr>(randomServiceName_);
    I3DOM::beaconLaunchRate = beaconLaunchRate_; // TODO: Why?!
    
	if(!randomService_)
		log_fatal("No random service available");
	if(prePulseProbability_<0.0 || prePulseProbability_>1.0)
		log_fatal_stream("Invalid probability of prepulses: " << prePulseProbability_);
	if(latePulseProbability_<0.0 || latePulseProbability_>1.0)
		log_fatal_stream("Invalid probability of late pulses: " << latePulseProbability_);
	if(afterPulseProbability_<0.0 || afterPulseProbability_>1.0)
		log_fatal_stream("Invalid probability of afterpulses: " << afterPulseProbability_);
    if(afterPulseProbability_==1.0)
        log_fatal("An afterpulses probability of 1 will lead to generation of an infinite number of afterpulses");
	if((prePulseProbability_+latePulseProbability_)>1.0)
		log_fatal_stream("Combined probablity of prepulses (" << prePulseProbability_
		                 << ") and late pulses (" << latePulseProbability_ << ") is greater than 1");
	if(lowMem_){
		if(!mergeHits_)
			log_fatal("Specifying 'LowMem' for PMTResponseSimulator will not work without also specifying 'MergeHits'");
		log_warn("Enabling 'low memory' mode, which will attempt to reduce memory use at the cost of longer run time");
	}

	// This generic charge distribution was taken as the average charge distribution from in-ice data. See https://wiki.icecube.wisc.edu/index.php/SPE_templates.
	genericChargeDistribution_ = boost::make_shared<I3SumGenerator>(randomService_,
	    SPEChargeDistribution(
	        6.9, //exp1_amp
	        0.027, //exp1_width
	        0.5487936979998159, //exp2_amp
	        0.38303036483159947, //exp2_width
	        0.753056240984677, //gaus_amp
	        1.0037645548431489, //gaus_mean
	        0.3199834176880081, //gaus_width
	        1.25, // compensation factor
	        1. // SLC mean
	    ),
	    0, //minimum value
	    3, //maximum value
	    1000, //number of bins
	    12 //when to switch to gaussian sampling
	);
}

void CombinedSimulator::DetectorStatus(I3FramePtr frame){
    const I3DetectorStatus& detectorStatus = frame->Get<I3DetectorStatus>();
    domStatus_ = detectorStatus.domStatus;
    domMapInitialized_ = false;
    PushFrame(frame, "OutBox");
}

void CombinedSimulator::Geometry(I3FramePtr frame){
    const I3Geometry& geometry = frame->Get<I3Geometry>();
    domGeo_ = geometry.omgeo;
    domMapInitialized_ = false;
    PushFrame(frame, "OutBox");
}

void CombinedSimulator::Calibration(I3FramePtr frame){
    const I3Calibration& calibration = frame->Get<I3Calibration>();
    domCal_ = calibration.domCal;
    domMapInitialized_ = false;
    PushFrame(frame, "OutBox");
}

void CombinedSimulator::DAQ(I3FramePtr frame)
{
	if(!frame->Has(inputHitsName_)){
		log_info_stream("Input '" << inputHitsName_ << "' not found; ignoring frame");
		PushFrame(frame);
		return;
	}
	boost::shared_ptr<const I3Map<OMKey,std::vector<I3MCPE>>> inputHits
	  = frame->Get<boost::shared_ptr<const I3Map<OMKey,std::vector<I3MCPE>>>>(inputHitsName_);
	if(!inputHits){
		log_error_stream("Input '" << inputHitsName_ << "' does not exist or is not an I3Map<OMKey,std::vector<I3MCPE>>");
		PushFrame(frame);
		return;
	}
	//Check for parent information stored in a side table. This need not exist.
	boost::shared_ptr<const I3ParticleIDMap> inputPedigree
	  = frame->Get<boost::shared_ptr<const I3ParticleIDMap>>(inputHitsName_+"ParticleIDMap");
	//Placeholder object in case we don't have a real one. Permanently empty.
	const ParticlePulseIndexMap dummyPedigree;

	boost::shared_ptr<const I3DetectorStatus> detStatus
	  = frame->Get<boost::shared_ptr<const I3DetectorStatus>>();
	if(!detStatus)
		log_fatal("Could not find detector status data");
	boost::shared_ptr<const I3Calibration> calibration
	  = frame->Get<boost::shared_ptr<const I3Calibration>>();
	if(!calibration)
		log_fatal("Could not find calibration data");
	if(calibration!=lastCalibration_){
		chargeDistributions_.clear();
		lastCalibration_=calibration;
	}

    // (Re-)Initializing the DOMMap if a new Geometry, Calibration,
    // or a Detector frame has occured.
	if(!domMapInitialized_) InitilizeDOMMap();

    domlauncherutils::DCStream dcStream;
    // Simulating the discriminator and so determing the discriminator crossings for the entire
    // event or more generally for an entire frame for each DOM.
    double frameStart = DBL_MAX, frameEnd = -DBL_MAX;

	//iterate over DOMs
	for(const auto& domPair : *inputHits){
        OMKey dom=domPair.first;
		std::map<OMKey,I3DOMStatus>::const_iterator omStatus=detStatus->domStatus.find(dom);
		if(omStatus==detStatus->domStatus.end()){
			log_debug_stream("No detector status record for " << dom);
			continue;
		}

		const double pmtVoltage=omStatus->second.pmtHV;
		if(pmtVoltage==0.0){
			log_debug_stream("Ignoring hits in DOM with voltage set to zero (" << dom << ')');
			continue;
		}

		std::map<OMKey, I3DOMCalibration>::const_iterator omCalibration=calibration->domCal.find(dom);
		if(omCalibration==calibration->domCal.end()){
			log_warn_stream("No calibration record for " << dom);
			continue;
		}

		if(pmtVoltage<0.0 || std::isnan(pmtVoltage)){
			log_warn_stream("Ignoring hits in DOM with nonsensical voltage (" << pmtVoltage << ')');
			continue;
		}
		
		if(domPair.second.empty()){
			log_trace_stream("Ignoring DOM with zero hits but with a PE map entry (" << dom << ')');
			continue;
		}
		
		//get any addition parentage information, if we have it
		const ParticlePulseIndexMap& pePedigree
		  = (inputPedigree && inputPedigree->count(dom) ?
			 inputPedigree->find(dom)->second : dummyPedigree);

		std::pair<std::vector<I3MCPulse>, ParticlePulseIndexMap> pulses=
		processHits(domPair.second, dom, omCalibration->second, omStatus->second, pePedigree);
		/*outputPulses->insert(std::make_pair(domPair.first,pulses.first));*/
		/*outputPIDMap->insert(std::make_pair(domPair.first,pulses.second));*/

		/* Simulating discriminators. */
		const OMKey& omkey = domPair.first;
        if(!pulses.first.empty()){
            int64_t end = pulses.first.size()-1;
            frameStart = pulses.first[0].time < frameStart ? pulses.first[0].time:frameStart;
            frameEnd   = pulses.first[end].time > frameEnd ? pulses.first[end].time:frameEnd;
        }
        
        log_info(std::string("In DOM ").append(std::to_string(omkey.GetString())).append(" ").append(std::to_string(omkey.GetOM())).c_str());
        I3DOMMap::iterator DOMit = domMap_.find(omkey);

        log_info(std::string("In DOM ").append(std::to_string(omkey.GetString())).append(" ").append(std::to_string(omkey.GetOM())).c_str());
        DOMit->second->Discriminator(pulses.first, dcStream);

        log_info(std::string("In DOM ").append(std::to_string(omkey.GetString())).append(" ").append(std::to_string(omkey.GetOM())).c_str());
        activeDOMsMap_[omkey] = DOMit->second;

    }

    if(frameEnd-frameStart> 60*I3Units::second){
        log_warn("MCPulses cover a time span larger than 1 minute (%lf min)",
                 (frameEnd-frameStart)/I3Units::minute);
    }
    //Adding beacon launches to the simulation.
    if(beaconLaunches_){
        log_debug("Adding beacon launches.");
        BOOST_FOREACH(const I3DOMMap::value_type& pair, domMap_){
            if(pair.second->AddBeaconLaunches(frameStart, frameEnd, dcStream))
                activeDOMsMap_[pair.first] = pair.second;
        }
    }
    
    log_debug("Number of discriminator crossings to process: %d",int(dcStream.size()));

    if(snMode_) DOTOutput(frame, dcStream);

    log_debug("Time ordering discriminator crossings.");
    sort(dcStream.begin(), dcStream.end(), domlauncherutils::detail::dcCompare);
    // The major part of the simulation takes place here. Triggers previously
    // simulated by the Discriminator() function of each I3DOM object have been
    // put in triggerStream, which *needs* time ordered (sorted) so that it
    // becomes a stream. This stream is then fed back to the I3DOM objects which
    // when recieving a trigger depending on the information it gets from its
    // 'neighboring' I3DOM objects decides if it will launch and what kind of
    // launch etc. At the end, all I3DOMs are triggered to launch if they still have
    // some triggers that haven't got any launch decision yet.  The main reason for
    // this happening is the way LC is working in the detector since it is in general
    // impossible to determine at launch time if the LC condition is true.
    log_debug("Simulating LC logic.");
    BOOST_FOREACH(DCStream::value_type& discrx, dcStream){
        if(domMap_.find(discrx.DOM) != domMap_.end())
        domMap_.at(discrx.DOM)->AddTrigger( discrx );
    }
    
    log_debug("Finishing Frame and triggering trailing launches.");
    bool force = !multiFrameEvents_;
    BOOST_FOREACH(const I3DOMMap::value_type& pair, activeDOMsMap_)
        pair.second->TriggerLaunch(force);

    // Transfer all launches in the I3DOMMap to the output I3DOMLaunchSeriesMap
    log_debug("Gather DOMLaunches.");
    // This is the object to be filled and put in to frame
    I3DOMLaunchSeriesMapPtr launchMap(new I3DOMLaunchSeriesMap);
    BOOST_FOREACH(const I3DOMMap::value_type& pair, activeDOMsMap_)
        if(! pair.second->GetDOMLaunches().empty() )
            (*launchMap)[pair.first] = pair.second->GetDOMLaunches();

    log_debug("Putting DOMLaunchSeriesMap to frame.");
    frame->Put(domLaunchMapName_, launchMap);

    // Reset the DOMs in the domMap_ if not in SuperNova mode
    // I'm assuming the SN group wants to be able to combine consecutive
    // frames and doesn't want the DOMs reset on each pass.
    log_debug("Reseting the DOMMap.");
    BOOST_FOREACH(const I3DOMMap::value_type& pair, activeDOMsMap_)
        pair.second->Reset(!multiFrameEvents_);
    // Removing inactive DOMs from the activeDOMsMap. 
    std::map<OMKey, boost::shared_ptr<I3DOM> >::iterator itr = activeDOMsMap_.begin();
    while (itr != activeDOMsMap_.end()) {
        if (! itr->second->IsActive()) {
            activeDOMsMap_.erase(itr++);
        } else {
            ++itr;
        }
    }
        
    log_debug("Pushing frame.");
    PushFrame(frame, "OutBox");
}

/**
 * This function provides Discriminator Over Threshold (DOT) output, a feature which
 * was asked by the sn-wg group as they base their analysis on discriminator crossings.
 */
void CombinedSimulator::DOTOutput(I3FramePtr frame, const DCStream& dcStream){
  log_debug("Making DOT output");
  I3MapKeyVectorDoublePtr thresholdCrossingsMap(new I3MapKeyVectorDouble);
  if (dcStream.empty()) return;

  BOOST_FOREACH(const DCStream::value_type& discrx, dcStream)
    (*thresholdCrossingsMap)[discrx.DOM].push_back(discrx.time);

  frame->Put(domLaunchMapName_ + "_DOTOutput", thresholdCrossingsMap);
}

void CombinedSimulator::InitilizeDOMMap(){
    log_info("Configuring DOM Object map for DOMLauncher. If tabulation of the SPE pulse templates"
            " is used this can take a while.");
    domMap_.clear();

    BOOST_FOREACH(const I3DOMCalibrationMap::value_type& p, domCal_){
        const OMKey& omkey = p.first;
        const I3DOMCalibration& domcal = p.second;

        I3DOMStatusMap::iterator stat_iter = domStatus_.find(omkey);
        if(stat_iter != domStatus_.end()){
            const I3DOMStatus& domstat = stat_iter->second;
            if (domstat.pmtHV==0.) {
                log_debug("DOM %s is disabled (pmtHV==0). Skipping.", omkey.str().c_str());
                continue;
            } else if ((domstat.pmtHV<0.) || std::isnan(domstat.pmtHV)) {
                log_fatal("DOM %s has invalid pmtHV. (pmtHV=%fV). Fix your GCD file!",
                        omkey.str().c_str(),
                        domstat.pmtHV/I3Units::V);
            }

            I3OMGeoMap::iterator geo_iter = domGeo_.find(omkey);
            if(geo_iter != domGeo_.end()){
                const I3OMGeo& omgeo = geo_iter->second;
                if(omgeo.omtype == I3OMGeo::IceCube ){
                    I3InIceDOMPtr domobj = I3InIceDOMPtr(new I3InIceDOM(randomService_, 
                                                                        omkey,  
                                                                        globalSimState.globalTime_,
                                                                        globalSimState.speTemplateMap_
                                                                        ));
                    domobj->SetUseTabulation(tabulatePTs_);
                    domobj->SetMergePulses(mergePulses_);
                    domobj->SetUseDroopedTemplate(droopedPulseTemplates_);
                    log_info("DOM %s .",omkey.str().c_str());
                    log_info("Voltage %s ", std::to_string(domstat.pmtHV).c_str());

                    if(domobj->Configure(domcal, domstat))
                        domMap_[omkey] = domobj;
                    else
                        log_error("DOM %s was not successfully configured.  Skipping.",omkey.str().c_str());
                }
                else if(omgeo.omtype == I3OMGeo::IceTop){
                    I3IceTopDOMPtr domobj = I3IceTopDOMPtr(new I3IceTopDOM(randomService_, 
                                                                            omkey,
                                                                            globalSimState.globalTime_,
                                                                            globalSimState.speTemplateMap_
                                                                            ));
                    domobj->SetUseTabulation(tabulatePTs_);
                    domobj->SetMergePulses(mergePulses_);
                    domobj->SetUseDroopedTemplate(droopedPulseTemplates_);
                    if(domobj->Configure(domcal, domstat))
                        domMap_[omkey] = domobj;
                    else
                        log_error("DOM %s was not successfully configured.  Skipping.",omkey.str().c_str());
                }
                else{
                    log_debug("DOM %s is of an unsupported type %d!", omkey.str().c_str(), omgeo.omtype);
		}
            }
            else{
                log_error("DOM %s does not exist in the geometry.", omkey.str().c_str());
            }
        }
        else{
            log_debug("DOM %s does not exist in the detector status.", omkey.str().c_str());
        }
    }

    // When the map of DOMs (detector) is filled we go through the map again to
    // set up the LC lines with information from detector status.
    for(I3DOMMap::iterator domMapIterator = domMap_.begin();
        domMapIterator != domMap_.end(); domMapIterator++){
      domMapIterator->second->CreateLCLinks(domMap_,domGeo_);
    }

    domMapInitialized_ = true;
    log_debug("Done initializing.");
}

std::pair<std::vector<I3MCPulse>, ParticlePulseIndexMap>
CombinedSimulator::processHits(const std::vector<I3MCPE>& inputHits, OMKey dom,
                                  const I3DOMCalibration& calibration, const I3DOMStatus& status,
                                  const ParticlePulseIndexMap& pePedigree){
	log_trace_stream(dom << " has " << inputHits.size() << " input hits");
	std::pair<std::vector<I3MCPulse>, ParticlePulseIndexMap> result;
	std::vector<I3MCPulse>& outputHits=result.first;
	ParticlePulseIndexMap& particleMap=result.second;
	//If the input PE were merged, each one may have multiple parent particles
	//recorded in pePedigree. Keep a set of iterators which will together make
	//one pass over that structure to avoid many binary searches to find the
	//parents of each MCPE. For M parent particles, with an average of A daughter
	//PE each, and N total PE this should be O(M*N) instead of O(M*N*log(A))
	std::vector<ParticlePulseIndexMap::mapped_type::const_iterator> parentIterators, parentEnds;
	parentIterators.reserve(pePedigree.size());
	parentEnds.reserve(pePedigree.size());
	for(const auto& particleData : pePedigree){
		parentIterators.push_back(particleData.second.begin());
		parentEnds.push_back(particleData.second.end());
	}
	std::vector<I3ParticleID> parents;

	//ensure that we have an up-to-date charge distribution for the DOM
	boost::shared_ptr<I3SumGenerator> speDistribution;
	if(!chargeDistributions_.count(dom)){
	    auto rawDistribution=calibration.GetCombinedSPEChargeDistribution();
		if(rawDistribution.IsValid()){
			speDistribution=boost::make_shared<I3SumGenerator>(randomService_,
			                  calibration.GetCombinedSPEChargeDistribution(),
							  0, //minimum value
							  3, //maximum value
							  1000, //number of bins
							  12 //when to switch to gaussian sampling
							  );
		}
		else{
			log_debug_stream("Falling back to generic charge distribution for " << dom);
			speDistribution=genericChargeDistribution_;
		}
		chargeDistributions_.insert(std::make_pair(dom,speDistribution)).first;
	}
	else
		speDistribution=chargeDistributions_.find(dom)->second;

	const double pmtVoltage=status.pmtHV;
	uint32_t peIndex=0; //index of the pe we are processing
	uint32_t pulseIndex=0; //index of the pulse we are generating
	//approximate count of how many pulses generated since last compression
	uint32_t roughCounter=0;
	//iterate over hits
	for(const I3MCPE& hit : inputHits){
		const unsigned int nHits = hit.npe;
		log_trace_stream("splitting a hit with weight " << nHits);
		
		//simple case: pe has one parent particle, stored internally
		I3ParticleID pid=hit.ID;
		//handle complex case for external parents
		size_t parentIndex=0;
		if(!pePedigree.empty()){ //need to update iterators
			parents.clear();
			ParticlePulseIndexMap::const_iterator parentIt=pePedigree.begin();
			for(unsigned int j=0; j<pePedigree.size(); j++,parentIt++){
				while(parentIterators[j]!=parentEnds[j] && *parentIterators[j]<peIndex)
					parentIterators[j]++;
				if(*parentIterators[j]==peIndex)
					parents.push_back(parentIt->first);
			}
			if(!parents.empty())
				pid=parents.front();
		}

		//decompose multi-photon hits into separate, single-photon hits
		//TODO: it might be a good idea to turn this off and fall back on
		//average behavior for very high weight hits
		for(unsigned int i=0; i<nHits; i++){
			I3MCPulse pulse;
			pulse.time=hit.time;
			//decide randomly which kind of hit to make
			double ran=randomService_->Uniform(1);
			bool canMakeAfterpulse=true;
			if(ran<(1.-prePulseProbability_-latePulseProbability_)){ //regular pulse
				log_trace("creating an SPE");
				pulse.source=I3MCPulse::PE;
				pulse.charge=normalHitWeight(1,speDistribution);
				pulse.time+=PMTJitter();
			}
			else if(ran<(1.-latePulseProbability_)){ //prepulse
				log_trace("creating a prepulse");
				pulse.source=I3MCPulse::PRE_PULSE;
				pulse.charge=prePulseWeight(pmtVoltage);
				pulse.time+=PMTJitter()+prePulseTimeShift(pmtVoltage);
				//A prepulse created by a photon passing through the cathode to
				//hit the first dynode cannot generate an afterpulse
				canMakeAfterpulse=false;
			}
			else{ //late pulse
				log_trace("creating a late pulse");
				createLatePulse(pulse,speDistribution,pmtVoltage);
			}
			outputHits.push_back(pulse);
			particleMap[pid].push_back(pulseIndex++);

			//maybe also add one or more afterpulses
			//each additional afterpulse should have its time be defined relative
			//to the hit which generates it, rather than relative to the original hit
			double afterpulseBaseTime=hit.time;
			while(canMakeAfterpulse && randomService_->Uniform(1)<afterPulseProbability_){
				log_trace("adding an afterpulse");
				I3MCPulse afterPulse;
				afterPulse.time=afterpulseBaseTime; //use the parent hit's time
				createAfterPulse(afterPulse,speDistribution,pmtVoltage);
				afterpulseBaseTime=afterPulse.time; //set the base time for a possible subsidiary afterpulse
				outputHits.push_back(afterPulse);
				particleMap[pid].push_back(pulseIndex++);
			}
			roughCounter++;
			if(lowMem_ && roughCounter>=100000){
				MCHitMerging::sortMCHits(outputHits.begin(),outputHits.end(), particleMap);
				MCHitMerging::timeMergeHits(outputHits,particleMap);
				pulseIndex=outputHits.size();
				roughCounter=0;
			}
			
			//in case of multiple parent particles, cycle to the next
			if(!parents.empty()){
				parentIndex++;
				if(parentIndex==parents.size())
					parentIndex=0;
				pid=parents[parentIndex];
			}
		} //end of iteration over single photons
		peIndex++;
	} //end of iteration over hits

	//ensure time ordering
	MCHitMerging::sortMCHits(outputHits.begin(),outputHits.end(), particleMap);
	//applying saturation before time-merging is more precise, but slower
	if(applySaturation_)
		saturate(outputHits,pmtVoltage,calibration);
	//std::cout << dom << " has " << outputHits.size() << " output hits" << std::endl;
	if(mergeHits_){
		MCHitMerging::timeMergeHits(outputHits,particleMap);
		//std::cout << dom << " has " << outputHits.size() << " output hits after merging" << std::endl;
	}
	return(result);
}

double CombinedSimulator::normalHitWeight(unsigned int w, const boost::shared_ptr<I3SumGenerator>& speDistribution){
	if(!useSPEDistribution_)
		return(w);
	return(speDistribution->Generate(w));
}

//Taken directly from the I3HitMaker code
/**
 * Fisher-Tippett variables for well behaved time
 * distribtutions.
 * These values were obtained by fits to Bricktop running
 * at 1345 [V] during DFL studies by C. Wendt. The fits were
 * performed by R. Porrata.
 */
double CombinedSimulator::PMTJitter(){
	if(!usePMTJitter_)
		return(0.0);

	const double mu=0.15304; //ns
	const double beta=1.9184; //ns
	const double ln_time_upper_bound=0.999998;
	const double ln_time_lower_bound=1e-7;

	return(fisherTippett(mu,beta,ln_time_lower_bound,ln_time_upper_bound));
}

//Time shift formula taken from HitMaker
double CombinedSimulator::prePulseTimeShift(double voltage){
	const double reference_time=31.8; //ns
	const double reference_voltage=1345.*I3Units::volt; //V
	return(-reference_time*sqrt(reference_voltage/voltage));
}

double CombinedSimulator::prePulseWeight(double voltage){
	if(!useSPEDistribution_)
		return(1.0);
	return(1/20.);
}

//Weighting formula taken from pmt-simulator
double CombinedSimulator::earlyAfterPulseWeight(){
	if(!useSPEDistribution_)
		return(1.0);
	//Fisher-Tippett fit to Early Afterpulse Charge distribtion, peaks 2 and 3.
	//Data from C. Wendt, fit by R. Porrata
	//https://wiki.icecube.wisc.edu/index.php/Early_Afterpulse_Data
	//The peak of the distribution
	const double peak_charge=13.31;
	//The width of the distribution; the sign is important because the distribution is asymmetric
	const double charge_spread=-3.386;
	//Upper bound of charge distribution corresponding to 26.28 PE
	const double ln_charge_lower_bound=1e-20;
	//Lower bound of charge distribution corresponding to 3.33 PE
	const double ln_charge_upper_bound=0.94883;
	return(fisherTippett(peak_charge,charge_spread,ln_charge_lower_bound,ln_charge_upper_bound));
}

struct pulseComponent{
	I3MCPulse::PulseSource source;
	double amp;
	double scale;
	double location;

	double weight() const{ return(amp*std::abs(scale)); }
};

//This suggests to the compiler that var is 'used', suppressing unhelpful warnings
#define used_var(var) while(0){ var++; }

void CombinedSimulator::createLatePulse(I3MCPulse& hit, const boost::shared_ptr<I3SumGenerator>& speDistribution, double voltage){
	//data for different types of late pulses
	const static unsigned int nComponents=5;
	const static pulseComponent pulseTypes[nComponents]={
		//source,                         amp,    scale,                     location
		{I3MCPulse::INELASTIC_LATE_PULSE, 150.0 , -3.0*I3Units::nanosecond ,  37.0*I3Units::nanosecond},
		{I3MCPulse::INELASTIC_LATE_PULSE, 306.0 , -9.0*I3Units::nanosecond ,  52.6*I3Units::nanosecond},
		{I3MCPulse::ELASTIC_LATE_PULSE  , 530.62,  3.47*I3Units::nanosecond,  66.0*I3Units::nanosecond},
		{I3MCPulse::INELASTIC_LATE_PULSE,  25.0 , 20.0*I3Units::nanosecond , 107.0*I3Units::nanosecond},
		{I3MCPulse::ELASTIC_LATE_PULSE  ,  53.0 ,  2.75*I3Units::nanosecond, 141.0*I3Units::nanosecond}
	};
	static double probabilities[nComponents];
	//compute the prbability for each distribution component,
	//and make a store to a static dummy variable to force the computation to be done only once
	static double* dummy = std::transform(pulseTypes,pulseTypes+nComponents,probabilities,std::mem_fun_ref(&pulseComponent::weight));
	used_var(dummy); //suppress compiler warnings about the dummy varaible being unused
	static discrete_distribution dist(probabilities,probabilities+nComponents);

	//decide which type of pulse to generate, randomly
	random_adapter gen(randomService_.get());
	unsigned int index=dist(gen);
	const pulseComponent& pulseType=pulseTypes[index];
	hit.source=pulseType.source;

	//constants for the time delay calculation
	const double referenceVoltage = 1345*I3Units::V;
	const double ln_time_lower_bound=1e-6;
	const double ln_time_upper_bound=0.999998;

	double timeDelay=0.0;
	timeDelay = fisherTippett(pulseType.location,pulseType.scale,ln_time_lower_bound,ln_time_upper_bound);
	timeDelay *= sqrt(referenceVoltage/voltage);
	hit.time+=timeDelay;

	hit.charge=normalHitWeight(1,speDistribution);
}

void CombinedSimulator::createAfterPulse(I3MCPulse& hit, const boost::shared_ptr<I3SumGenerator>& speDistribution, double voltage){
	const static unsigned int nComponents=11;
	//From hit-maker; should be derived from https://wiki.icecube.wisc.edu/index.php/Afterpulse_Data
	//Note that early after pulse components produce multiple p.e. of charge, so
	//their amplitudes have been divided by the average number of p.e. produced
	//to maintain the total generated charge.
	const static pulseComponent pulseTypes[nComponents]={
		//source,                      amp,           width,                      mean
		{I3MCPulse::AFTER_PULSE      , 10.0         , 200.0*I3Units::nanosecond ,  500.0*I3Units::nanosecond},
		{I3MCPulse::EARLY_AFTER_PULSE,  2.0254382825,  20.0*I3Units::nanosecond ,  540.0*I3Units::nanosecond},
		{I3MCPulse::EARLY_AFTER_PULSE,  3.9628140310,  20.0*I3Units::nanosecond ,  660.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  6.5         , 100.0*I3Units::nanosecond , 1100.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  4.5         , 200.0*I3Units::nanosecond , 1300.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  8.5         , 225.0*I3Units::nanosecond , 1650.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  8.95        , 300.0*I3Units::nanosecond , 2075.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  5.15        , 500.0*I3Units::nanosecond , 2850.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  3.0         , 700.0*I3Units::nanosecond , 4750.0*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      ,  5.0         , 400.0*I3Units::nanosecond , 6150.3*I3Units::nanosecond},
		{I3MCPulse::AFTER_PULSE      , 17.442       , 944.59*I3Units::nanosecond, 7833.4*I3Units::nanosecond}
	};
	static double probabilities[nComponents];
	//compute the probability for each distribution component,
	//and make a store to a static dummy variable to force the computation to be done only once
	static double* dummy = std::transform(pulseTypes,pulseTypes+nComponents,probabilities,std::mem_fun_ref(&pulseComponent::weight));
	used_var(dummy); //suppress compiler warnings about the dummy varaible being unused
	static discrete_distribution dist(probabilities,probabilities+nComponents);

	//decide which type of pulse to generate, randomly
	random_adapter gen(randomService_.get());
	unsigned int index=dist(gen);
	const pulseComponent& pulseType=pulseTypes[index];
	hit.source=pulseType.source;

	const double referenceVoltage = 1345*I3Units::V;
	double timeDelay=0.0;
	while(timeDelay<=0.0)
		timeDelay=randomService_->Gaus(pulseType.location,pulseType.scale);
	timeDelay *= sqrt(referenceVoltage/voltage);
	hit.time+=timeDelay;

	if(pulseType.source==I3MCPulse::EARLY_AFTER_PULSE)
		hit.charge=earlyAfterPulseWeight();
	else
		hit.charge=normalHitWeight(1,speDistribution);
}

struct saturationParams{
private:
	double gain; //the PMT gain
	double A, B, Ap; //the parameters for the saturation
	//where A and B are defined as by T. Waldenmeier and Ap = A/I_s

public:
	saturationParams(double log10Gain, bool newParameterization=true){
		gain=pow(10.0,log10Gain)/I3Units::milliampere;
		if(newParameterization){ //improved fits by T. Feusels
			A = pow(10,-.364+log10Gain*(.4247-.005765*log10Gain));
			B = pow(10,-1.696+log10Gain*(.9985-.06699*log10Gain));
			Ap = A/pow(10,-1.103+log10Gain*(.7194-.0372*log10Gain));
		}
		else{ //original fits by T. Waldenmeier
			A = pow(10,-1.008+log10Gain*(.818-.043*log10Gain));
			B = pow(10,-1.473+log10Gain*(1.008-.067*log10Gain));
			Ap = A/pow(10,-.933+log10Gain*(.713-.037*log10Gain));
		}
	}

	//Calculate the factor by which an ideal PMT output current should be reduced,
	//due to saturation. This is just the 'inverse' parametrization by Tilo Waldenmeier
	double saturate(double I){
		if(I<=0.0)
			return(1.0);
		//I is the current of photoelectrons per ns
		//but we need the current at the anode, in mA (since this unit is assumed by Tilo's formula)
		I*=gain;
		double ln = log(1+A/I);
		return(ln/(ln+Ap*exp(-B/I)));
	}
};

void CombinedSimulator::saturate(std::vector<I3MCPulse>& hits, double pmtVoltage, const I3DOMCalibration& cal){
	if(hits.empty())
		return;
	const double log10Gain = cal.GetHVGainFit().slope*log10(pmtVoltage/I3Units::V)+cal.GetHVGainFit().intercept;
	saturationParams params(log10Gain);

	//To determine the saturation factor for each hit we need an estimate of
	//the the current at the anode. We can compute this by summing up the
	//contributions from all of the hits. The usual template for this is a
	//gaussian, but if we use a form like exp(-abs(t)) the calculation can be
	//made much more efficient with little change in the result. Using such a
	//template, the current is just the sum of the 'past current', from hits
	//in the past and the 'future current' from hits in the future, and each
	//of these components can be updated simply for each new hit encountered.
	//In the case of the 'future current', finding the initial value requires
	//iterating through the hits in reverse, and due to limited floating
	//point precision this caclulation cannot be accurately run in reverse
	//(forward in time), so we just store the values encountered on a stack.
	const double anodePulseTau=2.2; //ns
	double currentNormalization=1./(2.*anodePulseTau);
	std::stack<double> futureCurrents;
	double pastCurrent=0.0;
	//run through the pulses in reverse to compute the future current
	double lastTime=hits.back().time;
	futureCurrents.push(0.0);
	for(auto hit=hits.rbegin(), end=hits.rend(); hit!=end; hit++){
		double time=hit->time;
		double futureCurrent=futureCurrents.top();
		futureCurrent*=exp((time-lastTime)/anodePulseTau);
		futureCurrent+=currentNormalization*hit->charge;
		futureCurrents.push(futureCurrent);
		lastTime=time;
	}
	//run through the pulses in order computing the past current, rewinding
	//the future current computation from before, and applying saturation
	lastTime=hits.front().time;
	for(I3MCPulse& hit : hits){
		double time=hit.time;
		double weight=hit.charge;
		pastCurrent*=exp((lastTime-time)/anodePulseTau);
		pastCurrent+=currentNormalization*weight;
		double futureCurrent=futureCurrents.top();
		futureCurrents.pop();
		futureCurrent-=currentNormalization*weight; //subtract the hit weight to avoid double counting
		hit.charge=weight*params.saturate(pastCurrent+futureCurrent);
		lastTime=time;
	}
}

double CombinedSimulator::fisherTippett(double location, double scale,
                                           double logLowerBound, double logUpperBound){
	return(location - scale * log(-log(randomService_->Uniform(logLowerBound,logUpperBound))));
}

