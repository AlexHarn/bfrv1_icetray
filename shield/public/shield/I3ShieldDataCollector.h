/**
I3ShieldDataCollector Module

(c) 2012 the IceCube Collaboration
$ID$

\file I3ShieldDataCollector.h
\date $Date$
*/

#ifndef SHIELD_I3SHIELDDATACOLLECTOR_H_INCLUDED
#define SHIELD_I3SHIELDDATACOLLECTOR_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

//forward declare test harness
template<typename ModuleType>
class SingleModuleTestSetup;

class I3ShieldDataCollector : public I3ConditionalModule{
public:
	I3ShieldDataCollector(const I3Context& ctx);
	~I3ShieldDataCollector(){}
	void Configure();
	void Physics(I3FramePtr frame);
private:
	std::string inputRecoPulsesName_;
	std::string inputTrackName_;
	std::string outputParamsName_;
	//bool reportUnhitTanks_;
	bool reportUnhitDOMs_;
	bool reportCharge_;
	bool useCurvatureApproximation_;
	std::vector<double> coefficients_;
	std::string badDOMListName_;
	
	SET_LOGGER("I3ShieldDataCollector");
	
	friend class SingleModuleTestSetup<I3ShieldDataCollector>;
};

#endif //SHIELD_I3SHIELDDATACOLLECTOR_H_INCLUDED
