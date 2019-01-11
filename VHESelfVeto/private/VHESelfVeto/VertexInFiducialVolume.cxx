/*
 * copyright  (C) 2012
 * Nathan Whitehorn, Claudio Kopper
 * The Icecube Collaboration: http://www.icecube.wisc.edu
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author $LastChangedBy$
 */

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Bool.h>
#include <dataclasses/I3Double.h>
#include <icetray/I3Int.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataclasses/physics/I3MCTreeUtils.h>

#include <vector>

#include <boost/foreach.hpp>
#include <algorithm>

#include <VHESelfVeto/VHESelfVetoUtils.h>

class VertexInFiducialVolume : public I3ConditionalModule {
	public:
		VertexInFiducialVolume(const I3Context &);

		void Configure();
		void Physics(I3FramePtr frame);
		void Geometry(I3FramePtr frame);
		void DetectorStatus(I3FramePtr frame) { Geometry(frame); }

	private:
		bool IsPositionBad(const I3Position &position);

		double topBoundaryWidth_, bottomBoundaryWidth_, sideBoundaryWidth_;
		double dustLayer_, dustLayerWidth_;

		std::string mcTreeName_;
		std::string outputBoolName_;
		std::string vertexPosName_;
		std::string vertexTimeName_;
		std::string vertexIncomingNeutrinoName_;
		std::string vertexOutgoingLeptonName_;
		std::string vertexOutgoingHadronName_;
		std::string numVerticesName_;

		VHESelfVetoUtils::reducedgeo_t reducedgeo_;

		std::string geoName_;
		I3GeometryConstPtr geo_;
};

I3_MODULE(VertexInFiducialVolume);

VertexInFiducialVolume::VertexInFiducialVolume(const I3Context &context)
    : I3ConditionalModule(context)
{
	AddParameter("TopBoundaryWidth", "Distance from the top "
	    "detector edge where there should be no vertices.", 90.);
	AddParameter("BottomBoundaryWidth", "Distance from the bottom detector "
	    "edge where there should be no vertices.", 10.);
	AddParameter("SideBoundaryWidth", "Distance from the side "
	    "edge where there should be no vertices.", 70.);
	AddParameter("DustLayer", "Mid-detector layer that should contain no "
	    "hits.", NAN);
	AddParameter("DustLayerWidth", "Mid-detector layer boundary width -- "
	    "only applies below the dust layer", NAN);
	AddParameter("MCTreeName", "Name of I3MCTree containing Monte Carlo"
	    "truth information", "I3MCTree");
	AddParameter("OutputBool", "Name of I3Bool containing the veto pass "
	    "decision", "VertexInFiducialVolume");
	AddParameter("VertexPosName", "Name of I3Position containing the MC "
	    "vertex position", "VertexPosition");
	AddParameter("VertexTimeName", "Name of I3Double containing the MC "
	    "vertex time", "VertexTime");
	AddParameter("VertexIncomingNeutrinoName", "Name of I3Particle containing the MC "
	    "particle corresponding to the incoming neutrino", "VertexIncomingNeutrino");
	AddParameter("VertexOutgoingLeptonName", "Name of I3Particle containing the MC "
	    "particle corresponding to the outgoing lepton", "VertexOutgoingLepton");
	AddParameter("VertexOutgoingHadronName", "Name of I3Particle containing the MC "
	    "particle corresponding to the outgoing hadron", "VertexOutgoingHadron");
	AddParameter("NumVerticesName", "Number of vertices for this event",
	    "NumVerticesInFiducialVolume");
	AddParameter("Geometry", "Name of geometry object to use",
	    I3DefaultName<I3Geometry>::value());
}

void
VertexInFiducialVolume::Configure()
{
	GetParameter("TopBoundaryWidth", topBoundaryWidth_);
	GetParameter("BottomBoundaryWidth", bottomBoundaryWidth_);
	GetParameter("SideBoundaryWidth", sideBoundaryWidth_);
	GetParameter("DustLayer", dustLayer_);
	GetParameter("DustLayerWidth", dustLayerWidth_);
	GetParameter("MCTreeName", mcTreeName_);
	GetParameter("OutputBool", outputBoolName_);
	GetParameter("VertexPosName", vertexPosName_);
	GetParameter("VertexTimeName", vertexTimeName_);
	GetParameter("VertexIncomingNeutrinoName", vertexIncomingNeutrinoName_);
	GetParameter("VertexOutgoingLeptonName", vertexOutgoingLeptonName_);
	GetParameter("VertexOutgoingHadronName", vertexOutgoingHadronName_);
	GetParameter("NumVerticesName", numVerticesName_);
	GetParameter("Geometry", geoName_);
}

void
VertexInFiducialVolume::Geometry(I3FramePtr frame)
{
	geo_ = frame->Get<I3GeometryConstPtr>(geoName_);
	I3DetectorStatusConstPtr status =
	    frame->Get<I3DetectorStatusConstPtr>();

	reducedgeo_ = VHESelfVetoUtils::FindReducedGeometry(*geo_, status,
	    topBoundaryWidth_, bottomBoundaryWidth_, sideBoundaryWidth_);

	PushFrame(frame);
}

bool VertexInFiducialVolume::IsPositionBad(const I3Position &pos)
{
	if ((!std::isnan(dustLayer_)) && (!std::isnan(dustLayerWidth_))) {
		// Is it in the vetoed dust region?
		if (pos.GetZ() < dustLayer_ && pos.GetZ() >
			dustLayer_ - dustLayerWidth_)
			return true;
	}

	// Positions that are not inside the reduced geometry are bad
	if (!IsInReducedGeometryPolygon(pos, reducedgeo_))
		return true;
	
	return false;
}

namespace {
	struct vertexRetVal_t {
		I3PositionConstPtr position;
		I3ParticleConstPtr incoming;
		I3ParticleConstPtr outgoingLepton;
		I3ParticleConstPtr outgoingHadron;
		I3DoubleConstPtr vertexTime;
	};
	
	// it turns out to be irritatingly hard to find the interaction vertex
	// consistently for all interaction types and generators..
	std::vector<vertexRetVal_t> GetNeutrinoVertices(const I3MCTree &mcTree)
	{
		std::vector<vertexRetVal_t> retVector;
		
		for (I3MCTree::const_iterator it=mcTree.begin();
		     it!=mcTree.end(); ++it)
		{
			// find neutrinos
			if (!it->IsNeutrino()) continue;
			
			// scan its children
			I3Position childPos;
			double childTime=NAN;
			I3ParticleConstPtr outgoingLepton;
			I3ParticleConstPtr outgoingHadron;
			bool first=true;
			
			for (I3MCTree::sibling_const_iterator j=mcTree.children(it);
			     j!=mcTree.end(it); ++j)
			{
				if (j->IsNeutrino()) continue; // skip neutrinos
				if (j->GetLocationType() != I3Particle::InIce)
					continue; // skip non in-ice particles
				
				if (first) {
					childPos = j->GetPos();
					childTime = j->GetTime();
					first=false;
				} else {
					if ((std::abs(j->GetPos().GetX()-childPos.GetX()) > 1e-5) ||
					    (std::abs(j->GetPos().GetX()-childPos.GetX()) > 1e-5) ||
					    (std::abs(j->GetPos().GetX()-childPos.GetX()) > 1e-5))
					{
						log_fatal("broken MCTree");
					}
				}
				
				if ((j->GetType() == I3Particle::EMinus) ||
				    (j->GetType() == I3Particle::EPlus) ||
				    (j->GetType() == I3Particle::MuMinus) ||
				    (j->GetType() == I3Particle::MuPlus) ||
				    (j->GetType() == I3Particle::TauMinus) ||
				    (j->GetType() == I3Particle::TauPlus))
				{
					// it's a lepton
					if (outgoingLepton) log_error("bogus MCTree. Neutrino has multiple outgoing leptons");
					
					outgoingLepton = I3ParticleConstPtr(new I3Particle( (*j) ));
				}
				else
				{
					if (outgoingHadron) log_error("bogus MCTree. Neutrino has multiple outgoing hadrons (i.e. more than one I3Particle::Hadron). This does not seem to be nugen data.");
					
					outgoingHadron = I3ParticleConstPtr(new I3Particle( (*j) ));
				}
			}
			
			if (first) continue; // there is no visible outgoing particle
			
			// add a vector entry
			retVector.push_back(vertexRetVal_t());
			vertexRetVal_t &newEntry = retVector.back();
			
			I3PositionPtr outputPos = I3PositionPtr(new I3Position());
			*outputPos = childPos;
			newEntry.position = outputPos;
			newEntry.incoming = I3ParticleConstPtr(new I3Particle( *it ));
			newEntry.outgoingLepton = outgoingLepton;
			newEntry.outgoingHadron = outgoingHadron;
			newEntry.vertexTime = I3DoubleConstPtr(new I3Double(childTime));
		}
		
		return retVector;
	}
	
}

void
VertexInFiducialVolume::Physics(I3FramePtr frame)
{
	I3MCTreeConstPtr mcTree =
	    frame->Get<I3MCTreeConstPtr>(mcTreeName_);
	if (!mcTree) {
		log_fatal("could not find I3MCTree in frame");
	}

	std::vector<vertexRetVal_t> vertices = GetNeutrinoVertices(*mcTree);
	
	if (vertices.size()==0)
	{
		frame->Put(vertexPosName_,
		    I3PositionConstPtr(new I3Position()));
		frame->Put(numVerticesName_,
		    I3IntConstPtr(new I3Int(0)));
		PushFrame(frame);
		return;
	}
	
	// there's at least one vertex at this point
	
	std::vector<bool> containedVertices(vertices.size(), false);
	std::size_t numContainedVertices = 0;
	for (std::size_t i=0; i<vertices.size(); ++i)
	{
		// is this vertex inside the fiducial volume?
		const bool vertexInFiducialVolume = !IsPositionBad(*(vertices[i].position));
		
		if (vertexInFiducialVolume) ++numContainedVertices;
		containedVertices[i] = vertexInFiducialVolume;
	}
	
	if (numContainedVertices==0)
	{
		// no vertex is inside the fiducial volume. save the first vertex
		// position we found as the output.
		
		frame->Put(outputBoolName_, I3BoolPtr(new I3Bool(false)));
		frame->Put(numVerticesName_, I3IntConstPtr(new I3Int(0)));

		frame->Put(vertexPosName_, vertices[0].position);
		
		if (vertices[0].outgoingLepton)
			frame->Put(vertexOutgoingLeptonName_, vertices[0].outgoingLepton);
		if (vertices[0].outgoingHadron)
			frame->Put(vertexOutgoingHadronName_, vertices[0].outgoingHadron);
		if (vertices[0].incoming)
			frame->Put(vertexIncomingNeutrinoName_, vertices[0].incoming);
		if (vertices[0].vertexTime)
			frame->Put(vertexTimeName_, vertices[0].vertexTime);
		
		PushFrame(frame);
		return;
	}
	
	// there's one or more contained neutrino vertices in this MCTree.
	// use the first one we found as the output
	std::size_t vertexIndex=0;
	bool sanityCheck=false;
	for (std::size_t i=0; i<containedVertices.size(); ++i)
	{
		if (containedVertices[i]) {
			vertexIndex=i;
			sanityCheck=true;
			break;
		}
		frame->Put(vertexTimeName_,
		    I3DoublePtr(new I3Double(NAN)));
	}
	if (!sanityCheck) log_fatal("internal logic error");
	
	frame->Put(outputBoolName_, I3BoolPtr(new I3Bool(true)));
	frame->Put(vertexPosName_, vertices[vertexIndex].position);

	if (vertices[vertexIndex].outgoingLepton)
		frame->Put(vertexOutgoingLeptonName_, vertices[vertexIndex].outgoingLepton);
	if (vertices[vertexIndex].outgoingHadron)
		frame->Put(vertexOutgoingHadronName_, vertices[vertexIndex].outgoingHadron);
	if (vertices[vertexIndex].incoming)
		frame->Put(vertexIncomingNeutrinoName_, vertices[vertexIndex].incoming);
	if (vertices[vertexIndex].vertexTime)
		frame->Put(vertexTimeName_, vertices[vertexIndex].vertexTime);

	frame->Put(numVerticesName_, I3IntConstPtr(new I3Int(numContainedVertices)));

	PushFrame(frame);
}
