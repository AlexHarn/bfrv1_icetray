
#include "fill-ratio/FastMinBall.h"

#include "boost/foreach.hpp"

FastMinBall::FastMinBall(const I3OMGeoMap &geo) {
	
	BOOST_FOREACH(const I3OMGeoMap::value_type &pair, geo)
		geometry_.push_back(std::make_pair(pair.first, pair.second));
	// Cache iterators over the strings
	geo_iter_t geoit = geometry_.begin();
	geo_iter_t geoend = geometry_.end();
	string_range_t string_range(geoit, geoend);
	for ( ; geoit != geoend; )
		if (geoit->first.GetString() != string_range.first->first.GetString()) {
			string_range.second = geoit;
			strings_.push_back(string_range);
			string_range.first = geoit++;
		} else
			++geoit;
	if (string_range.first != geoend) {
		string_range.second = geoend;
		strings_.push_back(string_range);
	}
}

inline bool
Compare(const std::pair<OMKey, I3OMGeo> &p1, const std::pair<OMKey, I3OMGeo> &p2)
{
	return p1.first < p2.first;
}

FastMinBall::geo_iter_t FastMinBall::LowerBound(const OMKey &key, geo_iter_t begin)
{
	if (begin < geometry_.begin() || begin >= geometry_.end())
		begin = geometry_.begin();
	return std::lower_bound(begin, geo_iter_t(geometry_.end()), std::make_pair(key, I3OMGeo()), Compare);
}

FastMinBall::geo_range FastMinBall::GetMinBallGeometry(const I3Position &pos, double radius)
{
	InXYCircle use_string(pos, radius);
	InRZCircle use_dom(pos, radius); 
	filtered_string_iterator string_begin(use_string, strings_.begin(), strings_.end());
	filtered_string_iterator string_end(use_string, strings_.end(), strings_.end());
	
	filtered_dom_iterator dom_begin = make_filter_iterator(use_dom,
	    segmented_iterator<filtered_string_iterator>(string_begin, string_end),
	    segmented_iterator<filtered_string_iterator>(string_end, string_end));
	filtered_dom_iterator dom_end = make_filter_iterator(use_dom,
	    segmented_iterator<filtered_string_iterator>(string_end, string_end),
	    segmented_iterator<filtered_string_iterator>(string_end, string_end));
		
	return boost::make_iterator_range(dom_begin, dom_end);
}
