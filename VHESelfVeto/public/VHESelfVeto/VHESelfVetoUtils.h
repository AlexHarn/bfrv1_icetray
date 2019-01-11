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

#ifndef VHESELFVETOUTILS_H_INCLUDED
#define VHESELFVETOUTILS_H_INCLUDED

#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3Particle.h>

#include <vector>
#include <map>
#include <set>

namespace VHESelfVetoUtils {
	///A representation of the boundary of the detector geometry as an extruded
	///polygon, aligned with the z-axis.
	struct reducedgeo_t {
		///The top of the extruded polygon
		double maxz;
		///The bottom of the extruded polygon
		double minz;
		///The ordered collection of x,y coordinate pairs which define the
		///horizontal boundary polygon
		std::vector<std::pair<double, double> > polygon;

		///The distance by which a point must be beneath maxz to be considered
		///inside the detector
		double topBoundaryWidth;
		///The distance by which a point must be above minz to be considered
		///inside the detector
		double bottomBoundaryWidth;
		///The distance by which a point must be inside the polygon to be
		///considered inside the detector
		double sideBoundaryWidth;
	};

	bool IsInReducedGeometry(const I3Position &pos,
	    const reducedgeo_t &reducedgeo);

	// This version is safe for vertices outside the
	// detector. Use for MC particle vertices.
	bool IsInReducedGeometryPolygon(const I3Position &pos,
	    const reducedgeo_t &reducedgeo);

	I3GeometryPtr TrimI3Geometry(const I3Geometry &geo,
	    const reducedgeo_t &reducedgeo);

	I3GeometryPtr TrimI3Geometry(const I3Geometry &geo,
			const std::set<int> &badStrings);

	reducedgeo_t FindReducedGeometry(const I3Geometry &geo,
	    I3DetectorStatusConstPtr status, double topBoundaryWidth,
	    double bottomBoundaryWidth, double sideBoundaryWidth);

	reducedgeo_t RemoveBoundariesFromReducedGeometry(
	    const reducedgeo_t &input);
	
	std::vector<I3Position>
	FindIntersectionsWithReducedGeometryBoundary(
	    const VHESelfVetoUtils::reducedgeo_t &reducedgeo,
	    const I3Position &from,
	    const I3Position &to);

	std::set<int>
	FindMissingStrings(const std::vector<OMKey> &badDOMs,
			const I3Geometry &geo, double missingDOMFractionForBadString = 0.9, bool ignoreDeepCoreStrings = true);

	///Find the intersection points of a particle's path with the boundaries of
	///the detector volume
	///\param geo The relevant detector geometry
	///\param particle The particle whose intersection points are to be found
	///\return A vector of all points where the (forward and backward) projected
	///        path of particle passes through the detector boundary. Note that
	///        these points may not be within the particle's length.
	std::vector<I3Position> IntersectionsWithInstrumentedVolume(
		const I3Geometry& geo, const I3Particle& particle);
	
	///Find the intersection points of a particle's path with the boundaries of
	///the detector volume
	///\param geo The relevant detector geometry.
	///\param particle The particle whose intersection points are to be found
	///\return A vector of all points where the (forward and backward) projected
	///        path of particle passes through the detector boundary. Note that
	///        these points may not be within the particle's length.
	///\pre geo must have all boundary widths set to zero, which can be
	///     accomplished using RemoveBoundariesFromReducedGeometry.
	std::vector<I3Position> IntersectionsWithInstrumentedVolume(
		const reducedgeo_t& geo, const I3Particle& particle);
};

#endif
