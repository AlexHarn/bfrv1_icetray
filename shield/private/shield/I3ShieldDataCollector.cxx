#include <shield/I3ShieldDataCollector.h>

#include <algorithm>
#include <cmath>

#include <dataclasses/I3Constants.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Particle.h>
#include <recclasses/I3ShieldHitRecord.h>


double polynomial(double x,const std::vector<double>& coefficients){
	///coefficients stored in order of decreasing degree
	double result=0;
	for(std::vector<double>::const_iterator it=coefficients.begin(), end=coefficients.end(); it!=end; it++){
		const double& coefficient=*it;
		result=result*x+coefficient;
	}
	return(result);
}

I3_MODULE(I3ShieldDataCollector);

I3ShieldDataCollector::I3ShieldDataCollector(const I3Context& ctx):
I3ConditionalModule(ctx),
inputRecoPulsesName_(""),
inputTrackName_(""),
outputParamsName_("ShieldResults"),
reportUnhitDOMs_(false),
reportCharge_(false),
useCurvatureApproximation_(false),
badDOMListName_("BadDomsList")
{
	AddOutBox("OutBox");
	
	AddParameter("InputRecoPulses",
	             "The pulses to be tested for coincidence with the track",
	             inputRecoPulsesName_);

	AddParameter("InputTrack",
	             "I3Particle to use for input track reconstruction (position, direction, and time)",
	             inputTrackName_);
	
	AddParameter("OutputName",
	             "Name for the output coincidence data, in the form of a list of I3ShieldHitRecords",
	             outputParamsName_);
	
	AddParameter("ReportUnhitDOMs",
	             "Whether to also generate a list of distances to unhit DOMs. "
	             "Note that this is only usful if the input hits are complete, "
	             "without cleaning, such as the union of all SLC hits. "
	             "This module has no knowledge of the cleanings applied for "
	             "IceTop Tank pulses, for instance.",
	             reportUnhitDOMs_);
	
	
	AddParameter("BadDOMList", "List of DOMs not expected to trigger, or to be ignored",
	             badDOMListName_);
	
	AddParameter("ReportCharge", "Whether to report the charge of the DOM in the output",
	             reportCharge_);
	
	AddParameter("UseCurvatureApproximation", "Whether to use curvature approximation rather than flat wave",
	             useCurvatureApproximation_);
	
	AddParameter("Coefficients", "If using curvature, give the list of coefficients "
	             "for polynomial curvature approximation (as a function of lateral distance). "
	             "Coefficients need to be provided in decreasing degree",
	             coefficients_);
}

void I3ShieldDataCollector::Configure(){
	GetParameter("InputRecoPulses",inputRecoPulsesName_);
	GetParameter("InputTrack",inputTrackName_);
	GetParameter("OutputName",outputParamsName_);
	GetParameter("ReportUnhitDOMs",reportUnhitDOMs_);
	GetParameter("BadDOMList", badDOMListName_);
	GetParameter("ReportCharge",reportCharge_);
	GetParameter("UseCurvatureApproximation",useCurvatureApproximation_);
	GetParameter("Coefficients",coefficients_);
	
	if (useCurvatureApproximation_ and coefficients_.empty()){
		log_fatal("Curvature approximation was requested but no coefficients were provided.");}
}

///Do the actual computation of veto-relevant information
///
///\param track The I3Particle representing the track hypothesis
///\param pulses The pulses for which to compute time residuals
///\param geometry The detector geometry
boost::shared_ptr<I3Vector<I3ShieldHitRecord> > ComputeCoincidences(const I3Particle& track,
	                                    const I3Map<OMKey, I3RecoPulseSeries>& pulses,
	                                    const I3Geometry& geometry,
	                                    const I3Vector<OMKey>* badDOMs,
	                                    bool reportCharge_,
	                                    bool useCurvatureApproximation,
	                                    std::vector<double>  coefficients){
	boost::shared_ptr<I3Vector<I3ShieldHitRecord> > coincidences(new I3Vector<I3ShieldHitRecord>());
	
	I3Position trackVertex = track.GetPos();
	//trackTime is the time at which the track passes through the vertex
	double trackTime = track.GetTime();
	//the direction of the track as a cartesian unit vector
	I3Position trackDir(track.GetDir());
	//for each OM compute the distance from the shower and the time offsets of all pulses
	for(I3Map<OMKey, I3RecoPulseSeries>::const_iterator mapIt=pulses.begin(), mapEnd=pulses.end(); mapIt!=mapEnd; mapIt++){
	    OMKey dom=mapIt->first;
	    if(!badDOMs || std::find(badDOMs->begin(),badDOMs->end(),dom)==badDOMs->end()){
            I3OMGeoMap::const_iterator geoIt=geometry.omgeo.find(dom);
            //the position of the OM projected onto the track
            I3Position closestPoint = trackVertex+trackDir*((geoIt->second.position-trackVertex)*trackDir);
            //the distance from the OM to the track
            double lateralDistance = sqrt((geoIt->second.position-closestPoint)*(geoIt->second.position-closestPoint));
            log_trace("Lateral distance to OM (%i,%i) is %f",dom.GetString(),dom.GetOM(),lateralDistance);
            //the vector between the track vertex and the position of the OM projected onto the track
            I3Position displacement = closestPoint-trackVertex;
            //the distance between the track vertex and the position of the OM, projected onto the track
            double projectedDistance = trackDir*displacement;
            double curvature  = 0.0;
            // If useCurvatureApproximation is true, then apply curvature correction using the coefficients provided
            if (useCurvatureApproximation){
                curvature = polynomial(lateralDistance, coefficients);
                log_trace("curvature:  %f",curvature);
                }

            //the time at which a plane oriented perpendicular to the track and traveling along it 
            //at the speed of light would pass the OM
            double expectedTime = trackTime + projectedDistance/I3Constants::c + curvature;
            log_trace("Expected time for hits at longitudinal distance %f is %f",projectedDistance,expectedTime);
            
            //for every pulse, compute the time offset
            for(std::vector<I3RecoPulse>::const_iterator it=mapIt->second.begin(), end=mapIt->second.end(); it!=end; it++){
                if (reportCharge_){
                    coincidences->push_back(I3ShieldHitRecord(it->GetTime()-expectedTime,lateralDistance, dom,it->GetCharge()));
                    }
                else
                coincidences->push_back(I3ShieldHitRecord(it->GetTime()-expectedTime,lateralDistance, dom));   
            }
	    }
    }
	
	return(coincidences);
}

///Compute the distances between the track an all DOMs which do not have pulses
///
///\param track The I3Particle representing the track hypothesis
///\param pulses The pulses for which to compute time residuals
///\param geometry The detector geometry
///\param badDOMs If non-NULL, a collection of DOMs which are not expected to work or are otherwise to be ignored
boost::shared_ptr<I3Vector<I3ShieldHitRecord> > ComputeUnhitDistances(const I3Particle& track,
	                                                        const I3Map<OMKey, I3RecoPulseSeries>& pulses,
	                                                        const I3Geometry& geometry,
	                                                        const I3Vector<OMKey>* badDOMs){
	boost::shared_ptr<I3Vector<I3ShieldHitRecord> > distances(new I3Vector<I3ShieldHitRecord>());
	const I3Map<int, I3Vector<I3TankGeo> >& stations = geometry.stationgeo;
	I3Position trackVertex = track.GetPos();
	//the direction of the track as a cartesian unit vector
	I3Position trackDir(track.GetDir());
	for(I3Map<int, I3Vector<I3TankGeo> >::const_iterator station=stations.begin(), stationEnd=stations.end();
		station!=stationEnd; station++){
		for(I3Vector<I3TankGeo>::const_iterator tank=station->second.begin(), tankEnd=station->second.end(); tank!=tankEnd; tank++){
			for(I3Vector<OMKey>::const_iterator om=tank->omKeyList_.begin(), omEnd=tank->omKeyList_.end(); om!=omEnd; om++){
				bool hit=(pulses.find(*om)!=pulses.end());
				bool bad=!(!badDOMs || std::find(badDOMs->begin(),badDOMs->end(),*om)==badDOMs->end());
				if (!hit && !bad){
					I3OMGeoMap::const_iterator geoIt=geometry.omgeo.find(*om);
					I3Position closestPoint = trackVertex+trackDir*((geoIt->second.position-trackVertex)*trackDir);
					double d = sqrt((geoIt->second.position-closestPoint)*(geoIt->second.position-closestPoint));
					double notime = std::numeric_limits<double>::quiet_NaN();
					distances->push_back(I3ShieldHitRecord(notime, d, *om));
				}
			}
			
		}
	}
	return(distances);
}


void I3ShieldDataCollector::Physics(I3FramePtr frame){
	if(!frame->Has("I3Geometry")){
		log_fatal("Unable to find geometry information");
		PushFrame(frame);
		return;
	}
	if(!frame->Has(inputRecoPulsesName_) || !frame->Has(inputTrackName_)){
		log_info("Skipping frame; did not find input %s",(!frame->Has(inputRecoPulsesName_)?inputRecoPulsesName_.c_str():inputTrackName_.c_str()));
		PushFrame(frame);
		return;
	}
		
	boost::shared_ptr<const I3Vector<OMKey> > badDOMs = frame->Get<boost::shared_ptr<const I3Vector<OMKey> > >(badDOMListName_);
	if(!badDOMs)
	log_warn("No BadDOMList available; all DOMs will be assumed good");
	boost::shared_ptr<I3Vector<I3ShieldHitRecord> > result = ComputeCoincidences(frame->Get<I3Particle>(inputTrackName_),
	                                                                             frame->Get<I3RecoPulseSeriesMap>(inputRecoPulsesName_),
	                                                                             frame->Get<I3Geometry>("I3Geometry"),
	                                                                             badDOMs.get(),
	                                                                             reportCharge_,
	                                                                             useCurvatureApproximation_,
	                                                                             coefficients_);
	frame->Put(outputParamsName_, result);
	
	if(reportUnhitDOMs_){
	boost::shared_ptr<I3Vector<I3ShieldHitRecord> > unhit_result = ComputeUnhitDistances(frame->Get<I3Particle>(inputTrackName_),
	     frame->Get<I3RecoPulseSeriesMap>(inputRecoPulsesName_),
	     frame->Get<I3Geometry>("I3Geometry"),
	     badDOMs.get());
	frame->Put(outputParamsName_+"_UnHit", unhit_result);
	}
	
	PushFrame(frame);
}

typedef I3Vector<I3ShieldHitRecord> I3VectorI3ShieldHitRecord;
I3_SERIALIZABLE(I3VectorI3ShieldHitRecord);
