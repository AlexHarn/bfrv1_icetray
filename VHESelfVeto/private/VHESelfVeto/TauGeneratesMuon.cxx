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
#include <icetray/I3Units.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataclasses/physics/I3MCTreeUtils.h>

#include <vector>

#include <boost/foreach.hpp>
#include <algorithm>

class TauGeneratesMuon : public I3ConditionalModule {
	public:
		TauGeneratesMuon(const I3Context &);

		void Configure();
		void Physics(I3FramePtr frame);

	private:
		std::string mcTreeName_;
		std::string outputBoolName_;
		std::string outgoingMuonName_;
	};

I3_MODULE(TauGeneratesMuon);

TauGeneratesMuon::TauGeneratesMuon(const I3Context &context)
    : I3ConditionalModule(context)
{
	AddParameter("MCTreeName", "Name of I3MCTree containing Monte Carlo"
	    "truth information", "I3MCTree");
	AddParameter("OutputBool", "Name of I3Bool containing the veto pass "
	    "decision", "TauGeneratesMuon");
	AddParameter("OutgoingMuonName", "Name of I3Particle containing the MC "
	    "particle corresponding to the outgoing muon", "OutgoingMuon");
}

void
TauGeneratesMuon::Configure()
{
	GetParameter("MCTreeName", mcTreeName_);
	GetParameter("OutputBool", outputBoolName_);
	GetParameter("OutgoingMuonName", outgoingMuonName_);
}

namespace {
	I3ParticleConstPtr GeneratesAMuon(const I3MCTree &mcTree)
	{
		I3ParticleConstPtr retval;
		
		for (I3MCTree::const_iterator it=mcTree.begin();
		     it!=mcTree.end(); ++it)
		{
			bool incomingTau;
			if ((it->GetType() == I3Particle::TauMinus) ||
			    (it->GetType() == I3Particle::TauPlus))
			{
				incomingTau = true;
			} else {
				incomingTau = false;
			}
			
			if (!incomingTau) continue;
			
			bool outgoingMuon = false;
			I3ParticleConstPtr outgoingMuonParticle;
			
			// scan its children
			for (I3MCTree::sibling_const_iterator j=mcTree.children(it);
			     j!=mcTree.end(it); ++j)
			{
				if ((j->GetType() == I3Particle::MuMinus) ||
				    (j->GetType() == I3Particle::MuPlus))
				{
					outgoingMuon=true;
					// save the muon
					outgoingMuonParticle = I3ParticleConstPtr(new I3Particle(*j));
				}
			}
			
			if (!outgoingMuon) continue;
			
			// if there is already a return muon, replace it if this one has a higher energy
			if (retval) {
				if (std::isnan(outgoingMuonParticle->GetEnergy())) continue;
				if (std::isnan(retval->GetEnergy())) {
					retval = outgoingMuonParticle;
				} else {
					if (outgoingMuonParticle->GetEnergy() > retval->GetEnergy()) {
						retval = outgoingMuonParticle;
					}
				}
			} else {
				retval = outgoingMuonParticle;
			}
		}
		
		return retval;
	}
	
}

void
TauGeneratesMuon::Physics(I3FramePtr frame)
{
	I3MCTreeConstPtr mcTree =
	    frame->Get<I3MCTreeConstPtr>(mcTreeName_);
	if (!mcTree) {
		log_fatal("could not find I3MCTree in frame");
	}

	I3ParticleConstPtr outgoingMuon = GeneratesAMuon(*mcTree);

	if (outgoingMuon) {
		frame->Put(outputBoolName_, I3BoolPtr(new I3Bool(true)));
		frame->Put(outgoingMuonName_, outgoingMuon);
	} else {
		frame->Put(outputBoolName_, I3BoolPtr(new I3Bool(false)));
	}

	PushFrame(frame);
}
