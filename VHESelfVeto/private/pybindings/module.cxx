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

#include <icetray/load_project.h>
#include <icetray/I3Logging.h>

#include <boost/foreach.hpp>
#include <boost/python.hpp>

#include "VHESelfVeto/VHESelfVetoUtils.h"
#include <dataclasses/physics/I3Particle.h>

namespace bp = boost::python;

bp::list IntersectionsWithInstrumentedVolume(const I3Geometry &geo, const I3Particle &particle)
{
	const std::vector<I3Position> intersections =
        VHESelfVetoUtils::IntersectionsWithInstrumentedVolume(geo, particle);
	
	bp::list retval;
	BOOST_FOREACH(const I3Position &pos, intersections)
	{
		retval.append(I3PositionPtr(new I3Position(pos)));
	}
	
	return retval;
}

BOOST_PYTHON_MODULE(VHESelfVeto)
{
    load_project("VHESelfVeto", false);
	
	bp::def("IntersectionsWithInstrumentedVolume", &IntersectionsWithInstrumentedVolume);
}
