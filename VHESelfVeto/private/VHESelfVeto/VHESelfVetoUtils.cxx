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

#include <boost/foreach.hpp>
#include <algorithm>
#include <cmath>

#include <VHESelfVeto/VHESelfVetoUtils.h>

#include "clipper/clipper.h"

bool
VHESelfVetoUtils::IsInReducedGeometry(const I3Position &pos,
    const reducedgeo_t &reducedgeo)
{
	if (reducedgeo.polygon.size() == 0)
		log_fatal("No geometry yet");

	// Easy check: within the boundary on the top/bottom?
	if (pos.GetZ() <= reducedgeo.minz + reducedgeo.bottomBoundaryWidth  ||
	    pos.GetZ() >= reducedgeo.maxz - reducedgeo.topBoundaryWidth)
		return false;

	// Next check: within boundary margin of each of the outer strings?
	// Since there are no strings beyond the outer strings, the possibility
	// of points passing because they are too far away can't come up
	typedef std::pair<double, double> DoublePair;
	BOOST_FOREACH(const DoublePair &a, reducedgeo.polygon) {
		if (hypot(a.first - pos.GetX(), a.second - pos.GetY()) <
		    reducedgeo.sideBoundaryWidth)
			return false;
	}

	return true;
}

I3GeometryPtr
VHESelfVetoUtils::TrimI3Geometry(const I3Geometry &geo,
    const reducedgeo_t &reducedgeo)
{
	if (reducedgeo.polygon.size() == 0)
		log_fatal("No geometry yet");

	I3GeometryPtr result(new I3Geometry(geo));
	
	// clear the output map and re-insert things that are in
	// the reduced geometry
	result->omgeo.clear();
	
	for (I3OMGeoMap::const_iterator om = geo.omgeo.begin();
	    om != geo.omgeo.end(); om++) {
		if (!IsInReducedGeometry(om->second.position, reducedgeo))
			continue;
		result->omgeo.insert(*om);
	}
	
	return result;
}

I3GeometryPtr
VHESelfVetoUtils::TrimI3Geometry(const I3Geometry &geo,
    const std::set<int> &badStrings)
{
  I3GeometryPtr result(new I3Geometry(geo));
  
  // clear the output map and re-insert things that are in
  // the reduced geometry
  result->omgeo.clear();
  
  for (I3OMGeoMap::const_iterator om = geo.omgeo.begin();
      om != geo.omgeo.end(); om++)
  {
    if (badStrings.count(om->first.GetString()) > 0)
      continue;
    result->omgeo.insert(*om);
  }
  
  return result;
}

namespace {
	// sort 2D points clock-wise around a given center
	class clockwise_less {
	public:
		clockwise_less(const std::pair<double, double> &center) :
		center_(center) 
		{ ; }
		
		bool operator()(const std::pair<double, double> &a,
		                const std::pair<double, double> &b) const
		{
			const double ax = a.first  - center_.first;
			const double ay = a.second - center_.second;
			const double bx = b.first  - center_.first;
			const double by = b.second - center_.second;
			
			const double aphi = std::atan2(ay, ax);
			const double bphi = std::atan2(by, bx);
			
			if (aphi > bphi) return true;
			if (aphi < bphi) return false;
			
			const double ar = std::sqrt(ax*ax + ay*ay);
			const double br = std::sqrt(bx*bx + by*by);

			return ar < br;
		}
		
	private:
		std::pair<double, double> center_;
	};
	
}

VHESelfVetoUtils::reducedgeo_t
VHESelfVetoUtils::FindReducedGeometry(const I3Geometry &geo,
    I3DetectorStatusConstPtr status, double topBoundaryWidth,
    double bottomBoundaryWidth, double sideBoundaryWidth)
{
	reducedgeo_t reducedgeo;
	
	reducedgeo.topBoundaryWidth = topBoundaryWidth;
	reducedgeo.bottomBoundaryWidth = bottomBoundaryWidth;
	reducedgeo.sideBoundaryWidth = sideBoundaryWidth;
	
	std::map<int, std::pair<double, double> > stringpositions;
	std::vector<int> boundarystrings;
	reducedgeo.minz = 0;
	reducedgeo.maxz = 0;
	std::map<int, double> stringtops;

	// geometry sanity checks to avoid crashes later on
	std::size_t num_DOMs = 0;
	std::size_t num_DOMs_good = 0;

	// Get the Z boundaries of the detector and X and Y positions for
	// each string
	for (I3OMGeoMap::const_iterator om = geo.omgeo.begin();
	    om != geo.omgeo.end(); om++) {
		
		num_DOMs++; // track the total number of DOMs
		
		// If DOM was off or not in ice, don't count it
		if (om->second.omtype != I3OMGeo::IceCube)
			continue;
		if (status && (status->domStatus.find(om->first) ==
		    status->domStatus.end() || status->domStatus.find(
		    om->first)->second.pmtHV < 100.*I3Units::V))
			continue;
		
		num_DOMs_good++; // track the total number of DOMs
		
		if (om->second.position.GetZ() < reducedgeo.minz)
			reducedgeo.minz = om->second.position.GetZ();
		if (om->second.position.GetZ() >
		    stringtops[om->first.GetString()])
			stringtops[om->first.GetString()] =
			    om->second.position.GetZ();
		stringpositions[om->first.GetString()] =
		    std::pair<double, double>(om->second.position.GetX(),
		    om->second.position.GetY());
	}

	if (num_DOMs_good==0)
		log_fatal("broked geometry: no good DOMs left (total DOMs: %zu)",
		    num_DOMs_good);

	if (stringtops.size()==0)
		log_fatal("broked geometry: no string top DOMs found (total DOMs: %zu, total good DOMs: %zu)",
		    num_DOMs, num_DOMs_good);

	// Find the string of about the same height as an average string where
	// the top DOM is the lowest. The thing this prevents is one string
	// (34!) sticking out the top of the detector and giving a bad 
	// measurement of where the "top" is.
	std::vector<double> toppositions;
	for (std::map<int, double>::const_iterator i = stringtops.begin();
	    i != stringtops.end(); i++)
		toppositions.push_back(i->second);
	std::sort(toppositions.begin(), toppositions.end());
	double topmedian = toppositions[toppositions.size()/2];
	reducedgeo.maxz = toppositions[toppositions.size()-1];
	BOOST_FOREACH(double z, toppositions) {
		if (fabs(topmedian - z) > topBoundaryWidth)
			continue;
		if (z < reducedgeo.maxz)
			reducedgeo.maxz = z;
	}
	

	// Figure out which strings are boundary strings: all interior strings
	// have at least 6 neighbors, where neighbor is defined as "within
	// 160 meters" to allow for the pork chop hole. All exterior strings
	// have fewer than 6 neighbors by the same definition.
	for (std::map<int, std::pair<double, double> >::const_iterator i =
	    stringpositions.begin(), j; i != stringpositions.end(); i++) {
		int neighbors = -1; // We will double count string i
		for (j = stringpositions.begin(); j != stringpositions.end();
		    j++) {
			if (hypot(i->second.first - j->second.first,
			  i->second.second - j->second.second) < 160*I3Units::m)
				neighbors++;
		}

		if (neighbors < 6)
			boundarystrings.push_back(i->first);
	}

	// Add boundary strings to boundary polygon
	reducedgeo.polygon.clear();
	BOOST_FOREACH(int string, boundarystrings)
		reducedgeo.polygon.push_back(stringpositions[string]);

	// printf("Detector top is at %lf meters, bottom at %lf meters\n",
	//     reducedgeo.maxz, reducedgeo.minz);
	// printf("Boundary strings are: ");
	// BOOST_FOREACH(int string, boundarystrings)
	// 	printf("%d ", string);
	// printf("\n");

	// Sort polygon clock-wise (around (0,0))
	std::sort(
	    reducedgeo.polygon.begin(),
	    reducedgeo.polygon.end(),
	    clockwise_less(std::make_pair(0.,0.))
	    );

	return reducedgeo;
}

namespace {
	double IsLeftTurn(const std::pair<double, double> &p,
	                  const std::pair<double, double> &q,
	                  const std::pair<double, double> &r)
	{
		// Three points are a counter-clockwise turn if ccw > 0, clockwise if
		// ccw < 0, and collinear if ccw = 0 because ccw is a determinant that
		// gives the signed area of the triangle formed by p, q and r.
		const double sum1 =
		    q.first*r.second +
		    p.first*q.second +
		    r.first*p.second;
		const double sum2 =
		    q.first*p.second +
		    r.first*q.second +
		    p.first*r.second;
		const double ccw = sum1 - sum2;
		
		return ccw;
	}
}

namespace {
	bool
	IsInReducedGeometryPolygon2D(
	    const std::pair<double, double> point2D,
	    const VHESelfVetoUtils::reducedgeo_t &reducedgeo)
	{
		if (reducedgeo.polygon.size() == 0)
			log_fatal("No geometry yet");

		int wn = 0; // the winding number counter

	    // loop through all edges of the polygon
		for (std::size_t i=0;i<reducedgeo.polygon.size();++i) {
			std::size_t j=i+1;
			if (j==reducedgeo.polygon.size()) j=0;
		
			const std::pair<double, double> &thisPoint = 
			    reducedgeo.polygon[i];
			const std::pair<double, double> &nextPoint = 
			    reducedgeo.polygon[j];

			if (thisPoint.second <= point2D.second) {  // start y <= P.second
				if (nextPoint.second > point2D.second) { // an upward crossing
					if (IsLeftTurn(thisPoint, nextPoint, point2D)>0)
						// P left of edge
						++wn; // have a valid up intersect
				}
			}
			else {  // start y > P.second (no test needed)
				if (nextPoint.second <= point2D.second) { // a downward crossing
					if (IsLeftTurn(thisPoint, nextPoint, point2D)<0)
						// P right of edge
						--wn; // have a valid down intersect
				}
			}
		}

		if (wn == 0) return false;
	
		// the point is inside the polygon. check its distance to the edge

		double closestDistance = NAN;
		for (std::size_t i=0;i<reducedgeo.polygon.size();++i)
		{
			std::size_t j=i+1;
			if (j==reducedgeo.polygon.size()) j=0;
		
			const std::pair<double, double> &thisPoint = 
			    reducedgeo.polygon[i];
			const std::pair<double, double> &nextPoint = 
			    reducedgeo.polygon[j];

			double dx = nextPoint.first -thisPoint.first;
			double dy = nextPoint.second-thisPoint.second;
			const double dl = std::sqrt(dx*dx + dy+dy);
			dx /= dl; dy /= dl;
		
			double u = (point2D.first-thisPoint.first)*dx +
			           (point2D.second-thisPoint.second)*dy;
		
			if (u<0.) u=0.;
			if (u>dl) u=dl;
		
			const double closest_x = u*dx + thisPoint.first;
			const double closest_y = u*dy + thisPoint.second;
		
			const double cp_x = closest_x - point2D.first;
			const double cp_y = closest_y - point2D.second;
		
			const double thisDistance = std::sqrt(cp_x*cp_x + cp_y*cp_y);
		
			if ((std::isnan(closestDistance)) ||
			    (thisDistance < closestDistance))
				closestDistance = thisDistance;
		}

		// too close to the edge?
		if (closestDistance <= reducedgeo.sideBoundaryWidth)
			return false;
	
		return true;
		
	}

}

bool
VHESelfVetoUtils::IsInReducedGeometryPolygon(const I3Position &pos,
    const reducedgeo_t &reducedgeo)
{
	if (reducedgeo.polygon.size() == 0)
		log_fatal("No geometry yet");

	// Easy check: within the boundary on the top/bottom?
	if (pos.GetZ() <= reducedgeo.minz + reducedgeo.bottomBoundaryWidth  ||
	    pos.GetZ() >= reducedgeo.maxz - reducedgeo.topBoundaryWidth)
		return false;

	// now check the 2D polygon
	const std::pair<double, double> point2D(pos.GetX(), pos.GetY());
	return IsInReducedGeometryPolygon2D(point2D, reducedgeo);
}

VHESelfVetoUtils::reducedgeo_t
VHESelfVetoUtils::RemoveBoundariesFromReducedGeometry(
    const VHESelfVetoUtils::reducedgeo_t &input)
{
	if (input.polygon.size() == 0)
		log_fatal("No geometry yet");

	VHESelfVetoUtils::reducedgeo_t output;
	
	// the top and bottom boundaries are easy
	output.maxz = input.maxz - input.topBoundaryWidth;
	output.minz = input.minz + input.bottomBoundaryWidth;
	output.topBoundaryWidth = 0.;
	output.bottomBoundaryWidth = 0.;
	
	// now reduce the polygon in xy by the side boundary width
	// using the clipper library
	
	// clipper works with integer coordinates, so use some unit
	// that has enough precision:
	const double clipperUnits = 0.1*I3Units::mm;
	
	// make a clipper polygon from our polygon
	ClipperLib::Polygon clipperPolygon;
	for (std::size_t i=0;i<input.polygon.size();++i) {
		// reverse the orientation for clipper
		const std::pair<double, double> &thisPoint = input.polygon[input.polygon.size()-i-1];
		
		const ClipperLib::long64 clipperX =
		    static_cast<ClipperLib::long64>(
		        thisPoint.first/clipperUnits);
		const ClipperLib::long64 clipperY =
		    static_cast<ClipperLib::long64>(
		        thisPoint.second/clipperUnits);
		
		// clipper works with integer coordinates,
		// so multiply all of this
		clipperPolygon.push_back(
		    ClipperLib::IntPoint(clipperX, clipperY));
	}
	
	// clipper needs a list of polygons as its input, so make one with a
	// single entry
	const ClipperLib::Polygons inPolygons(1, clipperPolygon);
	ClipperLib::Polygons outPolygons;
	
	// now offset the polygon (reduce in size, thus use -width)
	ClipperLib::OffsetPolygons(inPolygons, outPolygons,
	    -input.sideBoundaryWidth/clipperUnits, ClipperLib::jtRound);
	
	// At the moment, this code only supports a single polygon.
	// So if there is more than one, bail out and tell the user
	// that the author of this code was too lazy.
	if (outPolygons.size()!=1) {
		log_fatal("VHESelfVetoUtilsRemoveBoundariesFromReducedGeometry "
		    "only supports simple geometries that shrink to a single "
		    "polygon. Yours shrinks to %zu. Contact the author or "
		    "improve the code yourself.", outPolygons.size());
	}

	// reset the output polygon and side boundary
	output.sideBoundaryWidth=0.;
	output.polygon.clear();
	
	// convert the output polygon back to doubles
	for (std::size_t i=0;i<outPolygons[0].size();++i) {
		// reverse the order again
		const ClipperLib::IntPoint &thisPoint = outPolygons[0][outPolygons[0].size()-i-1];

		const double x = static_cast<double>(thisPoint.X)*clipperUnits;
		const double y = static_cast<double>(thisPoint.Y)*clipperUnits;
		
		output.polygon.push_back(std::make_pair(x,y));
	}
	
	return output;
}

namespace {
	// these functions assume that all 3 extra boundary widths are zero!
	
	void
	IntersectionsTopBottom(
	    const VHESelfVetoUtils::reducedgeo_t &reducedgeo,
	    std::map<double, I3Position> &intersections,
	    const I3Position &from, const I3Position &to)
	{
		if (reducedgeo.polygon.size() == 0)
			log_fatal("No geometry yet");

		const double dx = to.GetX() - from.GetX();
		const double dy = to.GetY() - from.GetY();
		const double dz = to.GetZ() - from.GetZ();

		std::vector<double> zPlanePos;
		zPlanePos.push_back(reducedgeo.minz);
		zPlanePos.push_back(reducedgeo.maxz);
		BOOST_FOREACH(const double &zPos, zPlanePos)
		{
			const double lambda = (zPos - from.GetZ()) / dz;
			if ((lambda < 0.) || (lambda > 1.)) continue;

			const std::pair<double, double>
			intersectPos2D(
			    from.GetX() + lambda*dx,
			    from.GetY() + lambda*dy);

			// is this (x,y) position inside the polygon?
			if (!IsInReducedGeometryPolygon2D(intersectPos2D, reducedgeo))
			    continue;

			// it is inside! make the 3D position and add it to the list
			const I3Position intersectPos(
				intersectPos2D.first,
				intersectPos2D.second,
				from.GetZ() + lambda*dz);
			intersections.insert(std::make_pair(lambda, intersectPos));
		}
	}

	void
	IntersectionsSide(
	    const VHESelfVetoUtils::reducedgeo_t &reducedgeo,
	    std::map<double, I3Position> &intersections,
	    const I3Position &from,
	    const I3Position &to)
	{
		if (reducedgeo.polygon.size() == 0)
			log_fatal("No geometry yet");

		const double x1 = from.GetX();
		const double y1 = from.GetY();
		const double z1 = from.GetZ();
		const double x2 = to.GetX();
		const double y2 = to.GetY();
		const double z2 = to.GetZ();
		
		for (std::size_t i=0;i<reducedgeo.polygon.size();++i)
		{
			std::size_t j=i+1;
			if (j==reducedgeo.polygon.size()) j=0;
		
			const std::pair<double, double> &thisPoint = 
			    reducedgeo.polygon[i];
			const std::pair<double, double> &nextPoint = 
			    reducedgeo.polygon[j];

			const double x3 = thisPoint.first;
			const double y3 = thisPoint.second;
			const double x4 = nextPoint.first;
			const double y4 = nextPoint.second;
			
			const double denom = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
			if (std::abs(denom) < 1e-10) continue; // lines are parallel
			
			const double u12 = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3))/denom;
			if ((u12 < 0.) || (u12 > 1.)) continue; // no intersection

			const double u34 = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3))/denom;
			if ((u34 < 0.) || (u34 > 1.)) continue; // no intersection

			// check z range
			const double intersectionZ = z1 + u12*(z2-z1);
			if ((intersectionZ <= reducedgeo.minz) || 
			    (intersectionZ >= reducedgeo.maxz))
				continue; // intersection is outside the z range
			
			const I3Position thisIntersection(
			    x1 + u12*(x2-x1),
			    y1 + u12*(y2-y1),
			    intersectionZ);

			intersections.insert(std::make_pair(u12, thisIntersection));
		}
	}

}

std::vector<I3Position>
VHESelfVetoUtils::FindIntersectionsWithReducedGeometryBoundary(
    const VHESelfVetoUtils::reducedgeo_t &in_reducedgeo,
    const I3Position &from, const I3Position &to)
{
	if (in_reducedgeo.polygon.size() == 0)
		log_fatal("No geometry yet");

	VHESelfVetoUtils::reducedgeo_t reducedgeo;
	
	// remove all boundaries (if there are any)
	if ((in_reducedgeo.topBoundaryWidth != 0.) ||
	    (in_reducedgeo.bottomBoundaryWidth != 0.) ||
	    (in_reducedgeo.sideBoundaryWidth != 0.))
	{
		reducedgeo =
		VHESelfVetoUtils::RemoveBoundariesFromReducedGeometry(
		    in_reducedgeo);
	} else {
		reducedgeo = in_reducedgeo;
	}

	// get intersections
	std::map<double, I3Position> intersections;
	IntersectionsTopBottom(reducedgeo, intersections, from, to);
	IntersectionsSide(reducedgeo, intersections, from, to);
	
	// convert to vector (sorting is done implicitly by std::map)
	std::vector<I3Position> retVect;
	for (std::map<double, I3Position>::const_iterator it=intersections.begin();
	    it!=intersections.end();++it)
	{
		retVect.push_back(it->second);
	}

	return retVect;
}

std::set<int>
VHESelfVetoUtils::FindMissingStrings(const std::vector<OMKey> &badDOMs,
    const I3Geometry &geo, double missingDOMFractionForBadString, bool ignoreDeepCoreStrings)
{ 
  std::map<int, unsigned int> all_DOM_count;
  std::map<int, unsigned int> bad_DOM_count;
  
  for (I3OMGeoMap::const_iterator it = geo.omgeo.begin();
       it != geo.omgeo.end(); ++it)
  {
    const int string = it->first.GetString();
    
    // ignore non-IceCube DOMs
    if (it->second.omtype != I3OMGeo::IceCube)
      continue;

    // Exclude Deep Core strings
    if (ignoreDeepCoreStrings) 
    {
        if ((string == 79) || 
            (string == 80) ||
            (string == 81) ||
            (string == 82) ||
            (string == 83) ||
            (string == 84) ||
            (string == 85) ||
            (string == 86))
          continue;
    }
    
    // insert 0 if not already there, retrieve reference to element and 
    // increment by 1.
    (*((all_DOM_count.insert(std::make_pair(string, 0))).first)).second
      += 1;
  }
  
  for (std::vector<OMKey>::const_iterator it = badDOMs.begin();
       it != badDOMs.end(); ++it)
  {
    const int string = it->GetString();

    // Exclude Deep Core strings
    if (ignoreDeepCoreStrings) 
    {
        if ((string == 79) || 
            (string == 80) ||
            (string == 81) ||
            (string == 82) ||
            (string == 83) ||
            (string == 84) ||
            (string == 85) ||
            (string == 86))
          continue;
    }

    // sanity check
    I3OMGeoMap::const_iterator geo_it = geo.omgeo.find(*it);
    if (geo_it == geo.omgeo.end())
      log_fatal("OM in bad DOM list is not in geometry!");
    
    // ignore non-IceCube DOMs
    if (geo_it->second.omtype != I3OMGeo::IceCube)
      continue;
    
    // insert 0 if not already there, retrieve reference to element and 
    // increment by 1.
    (*((bad_DOM_count.insert(std::make_pair(string, 0))).first)).second
      += 1;
  }

  std::set<int> retval;

  for (std::map<int, unsigned int>::const_iterator it=bad_DOM_count.begin();
      it != bad_DOM_count.end(); ++it)
  {
    // this is a string with bad DOMs on it, it might potentially be bad
    
    const unsigned int numberOfBadDOMs = it->second;
    
    std::map<int, unsigned int>::const_iterator other_it = 
      all_DOM_count.find(it->first);
    if (other_it==all_DOM_count.end())
      log_fatal("internal error, string should exist in list");

    const unsigned int numberOfDOMs = other_it->second;
    
    const double bad_fraction = 
      static_cast<double>(numberOfBadDOMs)/static_cast<double>(numberOfDOMs);
    
    if (bad_fraction > missingDOMFractionForBadString)
    {
      // this string is bad!
      retval.insert(it->first);
    }
  }
  
  return retval;
}

std::vector<I3Position> VHESelfVetoUtils::IntersectionsWithInstrumentedVolume(
 const reducedgeo_t& geo, const I3Particle& particle){
  assert(geo.topBoundaryWidth==0);
  assert(geo.bottomBoundaryWidth==0);
  assert(geo.sideBoundaryWidth==0);
  
  const double backTrackLength = 10000.*I3Units::m;
  const I3Position from = particle.GetPos() - particle.GetDir()*backTrackLength;
  
  double forwardTrackLength = 10000.*I3Units::m;
  if (!std::isnan(particle.GetLength()))
    forwardTrackLength += particle.GetLength();
  
  const I3Position to = particle.GetPos() + particle.GetDir()*forwardTrackLength;
  
  const std::vector<I3Position> intersections =
  VHESelfVetoUtils::FindIntersectionsWithReducedGeometryBoundary(geo, from, to);
  
  return intersections;
}

std::vector<I3Position>
VHESelfVetoUtils::IntersectionsWithInstrumentedVolume(const I3Geometry &geo,
    const I3Particle &particle)
{
  VHESelfVetoUtils::reducedgeo_t reducedgeoWithBorders =
  VHESelfVetoUtils::FindReducedGeometry(geo, I3DetectorStatusConstPtr(), 0., 0., 0.);
  
  // remove top/bottom and side borders
  VHESelfVetoUtils::reducedgeo_t reducedgeoNoBorders =
  VHESelfVetoUtils::RemoveBoundariesFromReducedGeometry(reducedgeoWithBorders);
  
  return IntersectionsWithInstrumentedVolume(reducedgeoNoBorders,particle);
}