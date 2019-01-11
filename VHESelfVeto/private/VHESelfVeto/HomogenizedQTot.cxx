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
#include <icetray/I3Units.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3Double.h>

#include <boost/foreach.hpp>

class HomogenizedQTot : public I3ConditionalModule {
	public:
		HomogenizedQTot(const I3Context &);

		void Configure();
		void Physics(I3FramePtr frame);

	private:
		std::string pulses_;
		std::string outputName_;

		std::string vertexTimeName_;
};

I3_MODULE(HomogenizedQTot);

HomogenizedQTot::HomogenizedQTot(const I3Context &context)
    : I3ConditionalModule(context)
{
	AddParameter("Output", "Name of output object", "QTot");
	AddParameter("Pulses", "Name of pulse series to use", "OfflinePulses");
	AddParameter("VertexTime", "Name of vertex time on which to impose "
	    "charge causality requirements (ignored if blank)", "");
}

void
HomogenizedQTot::Configure()
{
	GetParameter("Output", outputName_);
	GetParameter("Pulses", pulses_);
	GetParameter("VertexTime", vertexTimeName_);
}

static double
CalculateHomogenizedQtot(I3RecoPulseSeriesMapConstPtr pulses,
    I3CalibrationConstPtr cal, I3DoubleConstPtr vertexTime,
    double maxChargePerDom)
{
	const double causality_window = 5000*I3Units::ns; //1.4 km @ c
	double homogenized = 0;

	for (I3RecoPulseSeriesMap::const_iterator i = pulses->begin();
	    i != pulses->end(); i++) {
		I3DOMCalibrationMap::const_iterator domcal =
		    cal->domCal.find(i->first);
		if (domcal == cal->domCal.end())
			continue;
		// Exclude Deep Core
		if (domcal->second.GetRelativeDomEff() > 1.1)
			continue;
		// Exclude tricky Deep Core strings
		if (i->first.GetString() == 79 || i->first.GetString() == 80)
			continue;

		double charge_this_dom = 0;
		BOOST_FOREACH(const I3RecoPulse &pulse, i->second) {
			if (!vertexTime ||
			    (pulse.GetTime() >= vertexTime->value &&
			     pulse.GetTime() < vertexTime->value +
			     causality_window))
				charge_this_dom += pulse.GetCharge();
		}

		// Should we depth-correct the charge somehow?
		if (charge_this_dom < maxChargePerDom)
			homogenized += charge_this_dom;
	}

	return homogenized;
}

void
HomogenizedQTot::Physics(I3FramePtr frame)
{
	I3CalibrationConstPtr cal = frame->Get<I3CalibrationConstPtr>();
	I3RecoPulseSeriesMapConstPtr pulses =
	    frame->Get<I3RecoPulseSeriesMapConstPtr>(pulses_);
	if (!pulses) {
		PushFrame(frame);
		return;
	}

	I3DoubleConstPtr vertexTime =
	    frame->Get<I3DoubleConstPtr>(vertexTimeName_);

	double qtot = CalculateHomogenizedQtot(pulses, cal, vertexTime,
	    INFINITY);
	double homogenized = CalculateHomogenizedQtot(pulses, cal, vertexTime,
	    qtot/2.);

	frame->Put(outputName_, I3DoublePtr(new I3Double(homogenized)));

	PushFrame(frame);
}

