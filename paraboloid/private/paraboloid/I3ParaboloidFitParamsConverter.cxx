/**
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de>, last changed by $LastChangedBy$
 */

#include "I3ParaboloidFitParamsConverter.h"

I3TableRowDescriptionPtr
I3ParaboloidFitParamsConverter::CreateDescription(const I3ParaboloidFitParams &p) {
	I3TableRowDescriptionPtr desc(new I3TableRowDescription());
	desc->AddField<double>("zenith", "radian", "Zenith");
	desc->AddField<double>("azimuth", "radian", "Azimuth");
	desc->AddField<double>("err1", "radian", "Major Axis Width");
	desc->AddField<double>("err2", "radian", "Minor Axis Width");
	desc->AddField<double>("rotang", "radian",
	    "Rotation Angle of Error Ellipse");
	desc->AddField<double>("center_llh", "", "LLH Value at Ellipse Center");

	MAKE_ENUM_VECTOR(status, I3ParaboloidFitParams,
	    I3ParaboloidFitParams::ParaboloidFitStatus, (PBF_UNDEFINED)
	    (PBF_NO_SEED)(PBF_INCOMPLETE_GRID)(PBF_FAILED_PARABOLOID_FIT)
	    (PBF_SINGULAR_CURVATURE_MATRIX)(PBF_SUCCESS)(PBF_NON_POSITIVE_ERRS)
	    (PBF_NON_POSITIVE_ERR_1)(PBF_NON_POSITIVE_ERR_2)
	    (PBF_TOO_SMALL_ERRS));
	desc->AddEnumField<I3ParaboloidFitParams::ParaboloidFitStatus>(
	    "status", status, "", "Status of Error Estimation");

	return desc;
}

size_t
I3ParaboloidFitParamsConverter::FillRows(const I3ParaboloidFitParams &p, I3TableRowPtr rows) {
	rows->Set<double>("zenith", p.pbfZen_);
	rows->Set<double>("azimuth", p.pbfAzi_);
	rows->Set<double>("err1", p.pbfErr1_);
	rows->Set<double>("err2", p.pbfErr2_);
	rows->Set<double>("rotang", p.pbfRotAng_);
	rows->Set<double>("center_llh", p.pbfCenterLlh_);
	rows->Set<I3ParaboloidFitParams::ParaboloidFitStatus>("status",
	    p.pbfStatus_);

	return 1;
}
