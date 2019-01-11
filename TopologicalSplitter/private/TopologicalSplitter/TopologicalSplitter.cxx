/**
 * \file TopologicalSplitter.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id$
 * \version $Revision$
 * \date $Date$
 * \author Chris Weaver <chris.weaver@icecube.wisc.edu>
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <list>
#include <map>
#include <sstream>

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "phys-services/I3Splitter.h"

#include <boost/make_shared.hpp>

namespace{
	///A compact description of a hit
	///This type needs to be as small as possible so that copies are cheap. 
	struct Hit{
		///The index of the DOM on which this hit occurred, within the set of hit DOMs of the current event
		unsigned int domIndex;
		///The index of this hit on the DOM where it occurred
		unsigned int pulseIndex;
		///The actual time of the hit
		float time;
		
		///\param di The index of the DOM where the hit occurred
		///\param pi The index of the hit on the DOM
		///\param t The time of the hit
		Hit(unsigned int di, unsigned int pi, float t):
		domIndex(di),pulseIndex(pi),time(t){}
		
		///Determine whether hits are in time order
		struct timeOrdered{
			bool operator()(const Hit& h1, const Hit& h2) const{
				if (h1.time!=h2.time)
					return(h1.time<h2.time);
				if (h1.domIndex!=h2.domIndex)
					return(h1.domIndex<h2.domIndex);
				return(h1.pulseIndex<h2.pulseIndex);
			}
		};
		
		///Determine whether Hits are in an order more suitable for retrieval 
		///from an I3RecoPulseSeriesMap
		struct retrievalOrdered{
			bool operator()(const Hit& h1, const Hit& h2) const{
				if (h1.domIndex!=h2.domIndex)
					return(h1.domIndex<h2.domIndex);
				return(h1.pulseIndex<h2.pulseIndex);
			}
		};
	};
	
	///A set of hits ordered by their times
	typedef std::set<Hit,Hit::timeOrdered> TimeOrderedHitSet;
	///A set of hits ordered for fast lookup in the original I3RecoPulseSeriesMap
	typedef std::set<Hit,Hit::retrievalOrdered> RetrievalOrderedHitSet;
	
	//a Hit's identity is completely described by its indices
	bool operator==(const Hit& h1, const Hit& h2) {
		return(h1.domIndex==h2.domIndex && h1.pulseIndex==h2.pulseIndex);
	}
	
	//for sorting Hits we're mostly interested in their time, and only use their indices as tie-breakers
	bool operator<(const Hit& h1, const Hit& h2) {
		if (h1.time!=h2.time)
			return(h1.time<h2.time);
		if (h1.domIndex!=h2.domIndex)
			return(h1.domIndex<h2.domIndex);
		return(h1.pulseIndex<h2.pulseIndex);
	}
	
	///A light-weight summary of a DOM's geometric properties
	struct DomGeo{
		///The physical location of the DOM
		float x,y,z;
		///The DOM's string number
		int stringNumber;
		///The DOM's number on its string
		unsigned int omNumber;
		///Whether the DOM is part of IceCube proper, rather than IceTop
		bool isInIce;
		
		///\param k The DOM's logical location
		///\param g the DOm's physical location
		DomGeo(OMKey k, const I3OMGeo& g):
		x(g.position.GetX()),
		y(g.position.GetY()),
		z(g.position.GetZ()),
		stringNumber(k.GetString()),
		omNumber(k.GetOM()),
		isInIce(g.omtype == I3OMGeo::IceCube){}
	};
	
	///Test whether any item in the sorted range [first1,last1) is also in the sorted range [first2,last2)
	template <class InputIterator1, class InputIterator2>
	bool setsIntersect(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2) {
		while (first1!=last1 && first2!=last2) {
			if (*first1<*first2)
				first1++;
			else if (*first2<*first1)
				first2++;
			else
				return(true);
		}
		return(false);
	}
}

///The main splitter module
class I3TopologicalSplitter : public I3ConditionalModule, private I3Splitter{
private:
	//======================================
	// Data Structures and Helper Functions
	//======================================
	
	///An object which keeps track of a group of hits which are (mostly) causally connected to each other, 
	///and the number of distinct DOMs on which those hits occurred
	struct CausalCluster{
		///The ordered queue of hits within this cluster which are still within the time window of the current time
		std::list<Hit> hits;
		///Keeps track of the number of hits on each of the doms present in this cluster, keys are dom indices
		std::map<unsigned int,unsigned int> doms;
		///The hits which have formed a group surpassing the multiplicity and are now outside the time window
		TimeOrderedHitSet complete;
		///The indices of the most recent hit contributing to meeting the multiplicity condition
		unsigned int endDom, endPulse;
		///Whether the multiplicity condition is met
		bool multiplicityMet;
		
		CausalCluster():
		endDom(std::numeric_limits<unsigned int>::max()),
		multiplicityMet(false)
		{}
		
		///Add a new hit to the cluster
		///\param h The hit to add
		///\param multiplicity The threshold for a group of hits within the time window to form a subevent
		void insert(Hit h, unsigned int multiplicity) {
			doms[h.domIndex]++;
			hits.push_back(h);
			//if the total number of DOMs meets the multiplicity threshold, make note, 
			//and also record that this is the last known hit within the cluster contributing
			if (doms.size()>=multiplicity) {
				endDom=h.domIndex;
				endPulse=h.pulseIndex;
				multiplicityMet=true;
			}
		}
		
		///Finds the time of the earliest hit in this cluster
		///\return The earliest hit time or infinity if the cluster is empty
		float earliestTime() const{
			if (!complete.empty() && !hits.empty())
				return(std::min(complete.begin()->time,hits.begin()->time));
			if (!complete.empty())
				return(complete.begin()->time);
			if (!hits.empty())
				return(hits.begin()->time);
			return(std::numeric_limits<float>::infinity());
		}
		
		///Move this cluster forawrd in time to t, dropping hits which are no longer within the time window
		///\param t The current time to which the cluster should be moved
		///\param tt The owning splitter
		bool advanceTime(float t, I3TopologicalSplitter& tt) {
			while (!hits.empty()) {
				Hit h=*hits.begin();
				if (t > h.time + tt.timeWindow_) {
					//decrement the number of hits on the DOM where h occurred
					if ((--doms[h.domIndex])<=0)
						doms.erase(doms.find(h.domIndex));
					//if the mutiplicity threshold was met include h in the finished cluster
					if (multiplicityMet) {
						//insert the hit, and hint that it should go at the end of the complete set
						complete.insert((complete.empty()?complete.begin():--complete.end()),h);
						//if h was the last in the multiplicity window, 
						//shove the finished cluster back to the trigger for merging
						if (h.domIndex==endDom && h.pulseIndex==endPulse) {
							tt.AddSubEvent(complete);
							multiplicityMet=false;
							//no need to bother resetting endDom and endPulse; 
							//they can't give a false positive because we'll never see h again
						}
					}
					hits.pop_front();
				}
				else
					break;
			}
			return(hits.empty());
		}
		
		///Take all hits in other's complete list and merge them into this cluster's complete list
		void takeComplete(CausalCluster& other){
			complete.insert(other.complete.begin(),other.complete.end());
		}
	};
	
	///Test whether c1 is a subset of c2
	static bool isSubset(const CausalCluster& c1, const CausalCluster& c2) {
		if (c2.hits.size()<c1.hits.size())
			return(false);
		std::list<Hit>::const_iterator it1=c1.hits.begin(), end1=c1.hits.end(), 
		  it2=c2.hits.begin(), end2=c2.hits.end();
		for (; it1!=end1 && it2!=end2; it1++, it2++) {
			//if the two current items don't match scan though the (potential) superset looking for a match
			while (it2!=end2 && *it2<*it1)
				it2++;
			//three possible cases arise: 
			//if the items are now equal, c1 still appears to be a subset
			//if the item in c2 is greater than the one in c1, or we've gone off of the end of c2
			// there is no match for this item, so c1 is not a subset
			if (it2==end2 || *it1<*it2)
				return(false);
		}
		//if all of the items matched until we ran off of the end of c2, 
		//but there are still items left in c1, c1 is not a subset
		return(!(it1!=end1 && it2==end2));
	}
	
	///Determine whether two hits are related
	bool causallyConnected(const Hit& h1, const Hit& h2) {
		DomGeo g1=geometryCache_[h1.domIndex];
		DomGeo g2=geometryCache_[h2.domIndex];
		if (g1.isInIce!=g2.isInIce) //IceTop and InIce hits cannot be connected
			return(false);
		if (!g1.isInIce) //All IceTop hits are considered connected (?)
			return(true);
		//DOMs too many DOM spacings apart on the same string are never considered connected
		if (g1.stringNumber==g2.stringNumber && std::abs((long)g1.omNumber-(long)g2.omNumber)>zDomDist_)
			return(false);
		float dr2=(g1.x-g2.x)*(g1.x-g2.x)+(g1.y-g2.y)*(g1.y-g2.y);
		//DOMs too far apart horizontally are never considered connected
		if (dr2>xyDist_*xyDist_)
			return(false);
		
		//Finally, two hits are connected if the difference in their times is within the timeCone_ value of 
		//the time it would take a particle traveling at the speed of light to move from one hit's location
		//to the other hit's location.
		dr2+=(g1.z-g2.z)*(g1.z-g2.z);
		float dt=std::abs(h1.time-h2.time);
		return(std::abs(dt-sqrt(dr2)/I3Constants::c)<timeCone_);
	}
	
	///A function object which facilitates creating pulse masks for given subevents
	struct SubEventPredicate{
		///The lookup table for converting OMKeys to indices of DOMs in the current event
		const std::map<OMKey,unsigned int>& domIndices;
		///The collection of hits making up the subevent
		const RetrievalOrderedHitSet& hits;
		
		SubEventPredicate(const std::map<OMKey,unsigned int>& di, const RetrievalOrderedHitSet& h):
		domIndices(di),hits(h){}
		
		///Determine whether a particular pulse (hit) is part of the subevent
		///\param k The DOM on which the pulse occurred
		///\param pulseIdx The index of the pulse on the DOM
		bool operator()(const OMKey& k, size_t pulseIdx, const I3RecoPulse&) {
			//if the hit is in the set, include it in the mask
			return(hits.count(Hit(domIndices.find(k)->second,pulseIdx,0)));
		}
	};
	
	//==================
	// Member Variables
	//==================
	
	///a fast lookup table of simplified geometry information
	std::vector<DomGeo> geometryCache_;
	///a mapping fomr OMKeys to the indices of DOMs in the current event's pulse series map
	std::map<OMKey,unsigned int> domIndices_;
	///all in-progress causal clusters
	std::list<CausalCluster> clusters_;
	///temporary storage for causal clusters generated while adding a single hit
	std::list<CausalCluster> newClusters_;
	///all in-progress subevents
	std::list<TimeOrderedHitSet> partialSubEvents_;
	///map of start times to complete subevents
	std::map<float,RetrievalOrderedHitSet> subEvents_;

protected:
	///Name of the pulses to split
	std::string inputName_;
	///Name of the pulses to put in the split frames
	std::string outputName_;
	///Required multiplicity of a subevent
	unsigned int multiplicity_;
	///Time span within which the multiplicity requirement must be met
	float timeWindow_;
	///Maximum horizontal distance within which hits will be considered connected
	float xyDist_;
	///Maximum vertical distance within which hits will be considered connected, expressed as a number of DOM spacings
	unsigned int zDomDist_;
	///Maximum deviation from speed of light travel time which will allow a pair of hits to be considered connected
	float timeCone_;
	///Whether to save an integer in the frame indicating the number of subevents generated
	bool saveSplitCount_;
	
public:
	//================
	// Main Interface
	//================
	
	I3TopologicalSplitter(const I3Context& context):
	I3ConditionalModule(context),I3Splitter(configuration_),
	multiplicity_(4),
	timeWindow_(4000*I3Units::ns),
	xyDist_(500*I3Units::m),
	zDomDist_(30),
	timeCone_(1000*I3Units::ns),
	saveSplitCount_(false)
	{
		AddParameter("InputName", "Name of the pulses to split", inputName_);
		AddParameter("OutputName", "Name of the pulses to put in the split frames", outputName_);
		AddParameter("Multiplicity", "Required number of connected hits to form a subevent", multiplicity_);
		AddParameter("TimeWindow", "Time span within which the multiplicity requirement must be met", timeWindow_);
		AddParameter("XYDist", "Maximum horizontal distance within which hits will be considered connected", xyDist_);
		AddParameter("ZDomDist", "Maximum vertical distance within which hits on the same string will be "
		             "considered connected, expressed as a number of DOM spacings (unitless)", zDomDist_);
		AddParameter("TimeCone", "Maximum deviation from speed of light travel time which will "
		             "allow a pair of hits to be considered connected", timeCone_);
		AddParameter("SaveSplitCount", "Whether to save an integer in the frame indicating the number of "
		             "subevents generated", saveSplitCount_);
		AddParameter("SubEventStreamName",
			     "The name of the SubEvent stream.",
			     configuration_.InstanceName());

		AddOutBox("OutBox");
	}
	
	void Configure() {
		GetParameter("InputName", inputName_);
		GetParameter("OutputName", outputName_);
		GetParameter("Multiplicity", multiplicity_);
		GetParameter("TimeWindow", timeWindow_);
		GetParameter("XYDist", xyDist_);
		GetParameter("ZDomDist", zDomDist_);
		GetParameter("TimeCone", timeCone_);
		GetParameter("SaveSplitCount", saveSplitCount_);
		GetParameter("SubEventStreamName", sub_event_stream_name_);
		
		if (multiplicity_==0)
			log_fatal("Multiplicity should be greater than zero");
		if (timeWindow_<=0.0)
			log_fatal("TimeWindow should be greater than zero");
		if (xyDist_<=0.0)
			log_fatal("XYDist should be greater than zero");
		if (timeCone_<=0.0)
			log_fatal("TimeCone should be greater than zero");
	}
	
	void DAQ(I3FramePtr frame) {
		//fetch inputs
		I3RecoPulseSeriesMapConstPtr pulses = frame->Get<I3RecoPulseSeriesMapConstPtr>(inputName_);
		if (!pulses){
			PushFrame(frame);
			return; //nothing to do here
		}
		I3GeometryConstPtr geo = frame->Get<I3GeometryConstPtr>();
		if (!geo)
			log_fatal("Unable to find geometry data!");
		
		//do setup work: put all hits in time order and make the fast geometry lookup table
		std::vector<Hit> hits;
		unsigned int domIndex=0;
		for (I3Map<OMKey,std::vector<I3RecoPulse> >::const_iterator domIt=pulses->begin(), domEnd=pulses->end(); 
		  domIt!=domEnd; domIt++, domIndex++) {
			if (!geo->omgeo.count(domIt->first)){
				std::ostringstream ss;
				ss << domIt->first << "  is not in the geometry!";
				log_fatal("%s", ss.str().c_str());
			}
			geometryCache_.push_back(DomGeo(domIt->first,geo->omgeo.find(domIt->first)->second));
			domIndices_[domIt->first]=domIndex;
			unsigned int pulseIndex=0;
			for (std::vector<I3RecoPulse>::const_iterator pulseIt=domIt->second.begin(), pulseEnd=domIt->second.end();
			  pulseIt!=pulseEnd; pulseIt++, pulseIndex++)
				hits.push_back(Hit(domIndex,pulseIndex,pulseIt->GetTime()));
		}
		std::sort(hits.begin(),hits.end(),Hit::timeOrdered());
		
		//feed the hits into the clustering machinery
		for (std::vector<Hit>::iterator hit=hits.begin(), end=hits.end(); hit!=end; hit++)
			AddHit(*hit);
		FinalizeSubEvents();
		
		//output all subevents
		if (saveSplitCount_) {
			//Some users are concerned with how many subevents we created, so let's save them some trouble
			unsigned int nSubEvents=subEvents_.size();
			std::ostringstream countName;
			countName << GetName() << "SplitCount";
			frame->Put(countName.str(),boost::make_shared<I3Int>(nSubEvents));
		}
		PushFrame(frame); //need to delay this to here in case we save the split count
		for (std::map<float,RetrievalOrderedHitSet>::const_iterator it=subEvents_.begin(), end=subEvents_.end();
		  it!=end; it++) {
			//convert the subevent Hit set into a proper mask on the pulses series
			I3RecoPulseSeriesMapMaskPtr mask = 
			  boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, inputName_, SubEventPredicate(domIndices_,it->second));
			OutputSubEvent(frame,mask);
		}
		
		//clean up
		geometryCache_.clear();
		domIndices_.clear();
		subEvents_.clear();
	}

private:
	//=====================
	// Main Implementation
	//=====================
	
	/* Structural Overview:
	 * 
	 *                          +----------+
	 *                          | AddHit() |
	 *                          +----------+
	 *                               |
	 *                +--------------+--------------+
	 *                |                             |
	 *                v                             v
	 * +------------------------------+   +-------------------+
	 * | CausalCluster::advanceTime() |   | AddHitToCluster() |
	 * +------------------------------+   +-------------------+
	 *                |
	 *                v
	 *        +---------------+
	 *        | AddSubEvent() |
	 *        +---------------+
	 *
	 */
	
	///The main driver for the entire algorithm. 
	///Adds a new hit to all clusters with which it is connected (including subsets of existing clusters)
	///By 'advancing' the clusters this function also causes subevents to be built when possible. 
	void AddHit(Hit h) {
		newClusters_.clear();
		bool addedToCluster=false; //keep track of whether h has been added to any cluster
		for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), next=cluster, end=clusters_.end();
		  cluster!=end; cluster=next) {
			next++;
			if (cluster->advanceTime(h.time,*this))
				clusters_.erase(cluster);
			else
				addedToCluster |= AddHitToCluster(*cluster,h);
		}
		
		//Move all newly generated clusters into the main cluster list, 
		//eliminating clusters which are subsets of other clusters
		for (std::list<CausalCluster>::iterator newCluster=newClusters_.begin(), nend=newClusters_.end();
		  newCluster!=nend; newCluster++) {
			bool add=true;
			for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), next=cluster, end=clusters_.end();
			  cluster!=end; cluster=next) {
				next++;
				//check whether the new cluster is a subset of the old cluster
				//if the old cluster does not contain h, it cannot be a superset of the new cluster which does,
				//and if the old cluster contains h, it will be the last hit in that cluster
				if (cluster->hits.back()==h){
					if (isSubset(*newCluster,*cluster)) {
						add=false;
						break;
					}
				}
				//otherwise, the new cluster may still be a superset of the old cluster
				else if (isSubset(*cluster,*newCluster)) {
					//if replacing, make sure not to lose any hits already shifted to the old cluster's 'complete' list
					newCluster->takeComplete(*cluster);
					clusters_.erase(cluster);
				}
			}
			if (add)
				clusters_.push_back(*newCluster);
		}
		newClusters_.clear();
		
		//if h was not added to any cluster, put it in a cluster by itself
		if (!addedToCluster) {
			clusters_.push_back(CausalCluster());
			clusters_.back().insert(h,multiplicity_);
		}
	}
	
	///Attempt to add Hit h to existing cluster s, or to the subset of s with which it is connected, 
	///if it is not connected to enough of s to meet the multiplicity condition
	///returns true if h was added to s, or to a new subset of s, and false if h was not placed in a cluster
	bool AddHitToCluster(CausalCluster& s, Hit h) {
		//First, find out how many of the doms with hits in this cluster are causally connected to h
		//if there are more than multiplicity_, h will be added to this cluster
		std::set<unsigned int> connectedDOMs;
		bool allConnected=true;
		for (std::list<Hit>::reverse_iterator it=s.hits.rbegin(), end=s.hits.rend(); it!=end; it++) {
			if (causallyConnected(*it,h))
				connectedDOMs.insert(it->domIndex);
			else
				allConnected=false;
			if (connectedDOMs.size()+1>=multiplicity_) //note plus one for h itself, regardless of which DOM it is on!
				break;
		}
		if (connectedDOMs.size()+1>=multiplicity_ || allConnected)  //note plus one for h itself, regardless of which DOM it is on!
			s.insert(h,multiplicity_); //h is conected to enough hits to belong in this cluster
		else if (connectedDOMs.size()>0) {
			//if h is connected to some of the hits in the cluster, but not enough to be a member, 
			//we need to build a new cluster containing only those hits with which it is connected
			CausalCluster newSubset;
			for (std::list<Hit>::iterator it=s.hits.begin(), end=s.hits.end(); it!=end; it++) {
				if (causallyConnected(*it,h))
					newSubset.insert(*it,multiplicity_);
			}
			newSubset.insert(h,multiplicity_);
			//we add the new cluster to our list of newly created clusters, 
			//but if it is a subset of a cluster we already have we discard it as redundant, 
			//and likewise if any of the clusters we have is a subset we discard that one
			bool keep=true;
			for (std::list<CausalCluster>::iterator it=newClusters_.begin(), next=it, end=newClusters_.end();
			  it!=end; it=next) {
				next++; //if we end up erasing it it will be invalidated, so pre-emptively find the next item now
				if (isSubset(*it,newSubset))
					newClusters_.erase(it); //remove a redundant, existing cluster
				else if (isSubset(newSubset,*it)) {
					keep=false; //this cluster is redundant, so abort adding it
					break;
				}
			}
			if (keep) //finally, actually add the new cluster, as long as it isn't redundant
				newClusters_.push_back(newSubset);
		}
		return(connectedDOMs.size()>0);
	}
	
	///Inserts a cluster of hits into the set of subevents, after merging it with any existing subevents 
	///with which it shares at least one hit
	///This functon also moves any subevents which can no longer grow into the finished subevent collection. 
	void AddSubEvent(TimeOrderedHitSet& newSet) {
		//find any existing subevents which overlap the new one, and merge them into it
		for (std::list<TimeOrderedHitSet>::iterator set=partialSubEvents_.begin(), next=set, end=partialSubEvents_.end();
		  set!=end; set=next) {
			next++;
			if (setsIntersect(set->begin(),set->end(),newSet.begin(),newSet.end())) {
				newSet.insert(set->begin(),set->end());
				partialSubEvents_.erase(set);
			}
		}
		partialSubEvents_.push_back(newSet);
		newSet.clear();
		
		//find the earliest time of all hits currently percolating through the clusters
		float earliestUpcomingTime = std::numeric_limits<float>::max();
		for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), end=clusters_.end(); cluster!=end; cluster++)
			earliestUpcomingTime=std::min(earliestUpcomingTime,cluster->earliestTime());
		//any subevent whose last hit time is before the earliest time found above 
		//cannot be merged again, and so is complete
		if (earliestUpcomingTime!=std::numeric_limits<float>::max()) {
			for (std::list<TimeOrderedHitSet>::iterator set=partialSubEvents_.begin(), next=set, end=partialSubEvents_.end();
			  set!=end; set=next) {
				next++;
				if (set->rbegin()->time < earliestUpcomingTime) {
					//copy the contents of this subevent to a new subevent with ordering suitable
					//for retrieval of the actual hits and file it under the time of its first hit
					subEvents_.insert(std::make_pair(set->begin()->time,RetrievalOrderedHitSet(set->begin(),set->end())));
					partialSubEvents_.erase(set);
				}
			}
		}
	}
	
	///Pushes all hits through the clusters and completes all subevents, 
	///on the assumption that no more future hits will be added. 
	void FinalizeSubEvents() {
		//dump all hits out of the clusters
		for (std::list<CausalCluster>::iterator cluster=clusters_.begin(), end=clusters_.end();
		  cluster!=end; cluster++)
			cluster->advanceTime(std::numeric_limits<float>::infinity(),*this);
		clusters_.clear();
		//collect all leftover subevents
		for (std::list<TimeOrderedHitSet>::iterator set=partialSubEvents_.begin(), end=partialSubEvents_.end(); 
		  set!=end; set++)
			subEvents_.insert(std::make_pair(set->begin()->time,RetrievalOrderedHitSet(set->begin(),set->end())));
		partialSubEvents_.clear();
	}
	
protected:
	///Overidable function for storing each subevent
	///This exists pretty much solely for the benefit of the non-splitting backwards-compatibility subclass
	///\param frame The frame from which the pulses are being split
	///\param mask The subset of pulses forming this subevent
	virtual void OutputSubEvent(I3FramePtr frame, I3RecoPulseSeriesMapMaskPtr mask) {
		I3FramePtr subEvent = GetNextSubEvent(frame);
		subEvent->Put(outputName_, mask);
		PushFrame(subEvent);
	}
};

I3_MODULE(I3TopologicalSplitter);

namespace{
	///A shim class used for helping old scripts to continue to work identically as long as they use only 
	///the supported subset of the old options, and warn if the options being used are no longer supported
	class CompatibilityShim : public I3TopologicalSplitter{
		int Topo;
		int LCSpan;
		float LCWindow;
		float CBWindow;
	public:
		CompatibilityShim(const I3Context& context):
		I3TopologicalSplitter(context),
		Topo(1),
		LCSpan(-1),
		LCWindow(0),
		CBWindow(0)
		{
			AddParameter("Topo", "DEPRECATED", Topo);
			AddParameter("LCSpan", "DEPRECATED", LCSpan);
			AddParameter("LCWindow", "DEPRECATED", LCWindow);
			AddParameter("CBWindow", "DEPRECATED", CBWindow);
		}
		void Configure() {
			I3TopologicalSplitter::Configure();
			GetParameter("Topo", Topo);
			GetParameter("LCSpan", LCSpan);
			GetParameter("LCWindow", LCWindow);
			GetParameter("CBWindow", CBWindow);
			//Error out if the user tries to use a mode that no longer exists
			if (Topo!=1)
				log_fatal("Sorry, the 'Topo' parameter for ttrigger is deprecated, "
				          "and only the Topo=1 mode is now supported");
			if (LCSpan!=-1)
				log_fatal("The 'LCSpan' parameter for ttrigger is deprecated and does nothing");
			if (LCWindow!=0)
				log_fatal("Sorry, the 'LCWindow' parameter for ttrigger is deprecated");
			if (CBWindow!=0)
				log_fatal("Sorry, the 'CBWindow' parameter for ttrigger is deprecated");
		}
	};
}

///A module with the same interface as the TTriggerSplitter module provided by the ttrigger project
class TTriggerSplitter : public CompatibilityShim{
public:
	TTriggerSplitter(const I3Context& context):CompatibilityShim(context){}
};
I3_MODULE(TTriggerSplitter);

///\brief A module with the same interface as the ttrigger module provided by the ttrigger project
///
///This module runs on P frames rather than Q frames, and outputs each subevent as a numbered 
///pulse series in the original P frame. 
template<class Record> class ttrigger : public CompatibilityShim{
	unsigned int subEventNumber;
	std::vector<std::string> InputNames;
public:
	ttrigger(const I3Context& context):CompatibilityShim(context){
		AddParameter("InputNames", "DEPRECATED", InputNames);
	}
	void Configure() {
		CompatibilityShim::Configure();
		GetParameter("InputNames", InputNames);
		if(!(inputName_.empty() || InputNames.empty()))
			log_fatal("Please set only one of the 'InputName' or 'InputNames' parameters, not both");
		if(InputNames.size()>1)
			log_fatal("Sorry, splitting multiple pulse series at the same time is not supported");
		if(!InputNames.empty())
			inputName_=InputNames.front();
	}
	void DAQ(I3FramePtr frame) {
		PushFrame(frame); //Q frame? What Q frame?
	}
	void Physics(I3FramePtr frame) {
		subEventNumber=0;
		I3TopologicalSplitter::DAQ(frame);
	}
	virtual void OutputSubEvent(I3FramePtr frame, I3RecoPulseSeriesMapMaskPtr mask) {
		std::ostringstream outputName;
		outputName << outputName_ << subEventNumber;
		frame->Put(outputName.str(),mask);
		subEventNumber++;
	}
};
I3_MODULE(ttrigger<I3RecoPulse>);
