#include "polyplopia/MPHitFilter.h"
#include <simclasses/I3MCPE.h>
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "dataclasses/geometry/I3OMGeo.h"


using namespace std;

I3_MODULE(MPHitFilter);

typedef I3Map<I3ParticleID, unsigned> I3MapPIDUInt;
I3_POINTER_TYPEDEFS(I3MapPIDUInt);

/**
 * @brief A filtering module which selects events which 
 * have MCPEs >= threshold
 */

MPHitFilter::MPHitFilter(const I3Context& context) : 
	I3Module(context),
	hitSeriesMapName_("I3MCPESeriesMap"),
	threshold_(1),
	eventCount_(1),
	keptEvents_(0),
	rejectedEvents_(0),
	removedBranches_(0),
	pruneTree_(true),
	removeBackgroundOnly_(true),
	filter_(true),
	mcTreeName_("I3MCTree"),
	polyplopiaInfoMap_("PolyplopiaInfo") 
{ 
	AddParameter("I3MCPESeriesMapName",
		   "Name of HitSeriesMap object to scan",
		   hitSeriesMapName_); 
	AddParameter("HitOMThreshold","Minimum number of hit OMs to pass filter ",threshold_); 
	AddParameter("PruneTree", "Cut dead MCTree branches that don't produce MCPEs", pruneTree_); 
	AddParameter("RemoveBackgroundOnly", "Remove events that don't contain primary", removeBackgroundOnly_); 
	AddParameter("MCTreeName", "Name of I3MCTree object in frame", mcTreeName_); 
	AddParameter("PolyplopiaInfoName", "map of weights to merge", polyplopiaInfoMap_); 
	AddParameter("Filter", "Remove events that don't pass threshold", filter_); 
	AddOutBox("OutBox"); 
}

void 
MPHitFilter::Configure() 
{ 
	GetParameter("I3MCPESeriesMapName", hitSeriesMapName_); 
	GetParameter("HitOMThreshold",threshold_); 
	GetParameter("PruneTree", pruneTree_);
	GetParameter("RemoveBackgroundOnly", removeBackgroundOnly_);
	GetParameter("MCTreeName",  mcTreeName_); 
	GetParameter("PolyplopiaInfoName", polyplopiaInfoMap_); 
	GetParameter("Filter", filter_); 
}

void 
MPHitFilter::DAQ(I3FramePtr frame)
{ 
    unsigned hitcount= 0;
    bool pruneTree = pruneTree_;
    //map<I3ParticleID,unsigned> primarymap;
    I3MapPIDUInt primarymap;
    I3MCTree mctree = frame->Get<I3MCTree>(mcTreeName_);
    I3MCTreePtr mctree_ptr = I3MCTreePtr(new I3MCTree(mctree));
    I3Particle* temp_primary;
    I3MapStringIntConstPtr polyplopiaInfo;

    // Clean hits for missing OMs
    const I3Geometry& geo = frame->Get<I3Geometry>();
    if (!frame->Has(hitSeriesMapName_)){
	    log_fatal("No I3MCPESeriesMap named %s found in frame", 
			    hitSeriesMapName_.c_str());
    }


    I3MCPESeriesMap::const_iterator om_iter;
    I3MCPESeriesMapConstPtr hitmap = 
		frame->Get<I3MCPESeriesMapConstPtr>(hitSeriesMapName_); 

    for(om_iter = hitmap->begin(); om_iter != hitmap->end(); om_iter++) 
    { 
	    // Check that om is part of geometry
            if (geo.omgeo.find(om_iter->first) != geo.omgeo.end()) { 
		    hitcount += om_iter->second.size(); 
		    BOOST_FOREACH(const I3MCPE& pe, om_iter->second){ 
			    temp_primary = I3MCTreeUtils::GetPrimaryPtr(mctree_ptr, pe.ID); 
			    if (temp_primary) 
			       ++primarymap[temp_primary->GetID()]; 
			    else if (pe.npe > 1)
			       // If NPE > 1 we can't know the parent.
			       // Temporarily set prunning to false.
			       pruneTree = false;
			    else {
					I3MCTree::optional_value src_particle=(*mctree_ptr)[pe.ID];
					log_fatal_stream("No primary particle found for light emitting particle "
					  << pe.ID << (!src_particle?"; this particle does not exist in the tree at all\n"
					  "A common cause of this error is failure to run CLSim's I3MuonSliceRemoverAndPulseRelabeler "
					  "before deleting the muon sliced I3MCTree":""));
			    }
		    } 
	    } 
	    // if we have passed the threshold, no need to keep looking 
	    //  (unless we are prunning the mctree)
            if (!pruneTree_ && hitcount >= threshold_ ) { 
		    log_debug("Found at least %d hits. Not filtering", hitcount); 
		    break; 
	    } 
    }

    if (removeBackgroundOnly_ && filter_){ 
	    I3ParticleConstPtr primaryptr = 
		    frame->Get<I3ParticleConstPtr>("PolyplopiaPrimary"); 
	    if (!primaryptr) 
		    log_fatal("PolyplopiaPrimary not found in I3Frame");

	    I3ParticleID pid = primaryptr->GetID();
	    if ( !primarymap.count(pid) || primarymap[pid] < threshold_ )
	    { 
		    log_info("Only found %d hits (threshold=%d). filtering frame.", 
				    hitcount, threshold_); 
		    rejectedEvents_++; 
		    return; // don't push frame. Primary has not produced any hits 
	    }
    } 
    
    

    if (pruneTree) 
    { 
	    I3MapStringIntPtr newPolyplopiaInfo;
	    // replace weights with updated hitcount
	    if (frame->Has(polyplopiaInfoMap_)) {
                 polyplopiaInfo = frame->Get<I3MapStringIntConstPtr>(polyplopiaInfoMap_); 
		 newPolyplopiaInfo = I3MapStringIntPtr(new I3MapStringInt(*polyplopiaInfo)); 
	    } else {
                 log_trace("PolyplopiaInfoMap '%s' not found in I3Frame", polyplopiaInfoMap_.c_str());
		 newPolyplopiaInfo = I3MapStringIntPtr(new I3MapStringInt()); 
	    }

	    I3MapUnsignedUnsignedPtr primHits = 
		    I3MapUnsignedUnsignedPtr(new I3MapUnsignedUnsigned());
	   

	    // Now clean up the MCTree and remove dead branches
	    std::vector<I3Particle>::iterator p;
	    std::vector<I3Particle> primaries = mctree_ptr->get_heads();
	    for (p = primaries.begin(); p != primaries.end(); p++)
	    { 
		    I3ParticleID pid = p->GetID();
		    if ( !primarymap.count(pid) || primarymap[pid] < threshold_ ) 
		    {
			    if (!mctree_ptr->at(*p))
			       log_fatal("Trying to delete a primary that is not in the tree");

			    mctree_ptr->erase(*p);
			    removedBranches_++;
			    if (mctree_ptr->at(*p))
			       log_fatal("Primary is still in tree after deletion");
		    } else { 
			    (*primHits)[pid.minorID] = primarymap[p->GetID()]; 
		    }
		    
	    }
	    frame->Delete(mcTreeName_);
	    frame->Put(mcTreeName_,mctree_ptr);

	    (*newPolyplopiaInfo)[hitSeriesMapName_ + "Count"] = hitcount;
	    (*newPolyplopiaInfo)[mcTreeName_+ "Primaries"] = mctree_ptr->get_heads().size();

	    // Replace object in frame
	    if (frame->Has(polyplopiaInfoMap_))  
	        frame->Delete(polyplopiaInfoMap_);
	    frame->Put(polyplopiaInfoMap_,newPolyplopiaInfo);

	    if (frame->Has(mcTreeName_+"PEcounts"))
	        frame->Delete(mcTreeName_+"PEcounts");
	    frame->Put(mcTreeName_+"PEcounts",primHits);
    }

    
    if ( hitcount >= threshold_ ) { 
	    eventCount_++; 
	    PushFrame(frame,"OutBox"); 
	    keptEvents_++;
    } else { 
	    log_debug("Only found %d hits (threshold=%d). filtering frame.", 
		    hitcount, threshold_); 
	    rejectedEvents_++;

	    if (!filter_) { 
		    eventCount_++; 
		    PushFrame(frame,"OutBox"); 
		    keptEvents_++; 
	    }
    }
    return; 
}

void 
MPHitFilter::Finish()
{
    I3MapStringDoublePtr summary = context_.Get<I3MapStringDoublePtr>("I3SummaryService");
    if (summary) { 
        (*summary)["KeptEvents"] = keptEvents_;
        (*summary)["RejectedEvents"] = rejectedEvents_;
        (*summary)["RemovedTreeBranches"] = removedBranches_;
    } 
    log_info("%u events passed" ,keptEvents_); 
    log_info("%u events rejected" ,rejectedEvents_); 
    log_info("%u tree branches removed" ,removedBranches_);
}

I3_SERIALIZABLE(I3MapPIDUInt);
