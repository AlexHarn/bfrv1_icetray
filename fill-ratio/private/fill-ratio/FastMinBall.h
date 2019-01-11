
#ifndef FILLRATIO_FASTMINBALL_H_INCLUDED
#define FILLRATIO_FASTMINBALL_H_INCLUDED

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/I3Position.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iterator/filter_iterator.hpp>

// Efficiently iterate over DOMs inside a sphere on the assumption that
// OMs on the same string have the same x,y position.

class FastMinBall {
public:
	template <class Iterator>
	class segmented_iterator : public boost::iterator_facade<
	    segmented_iterator<Iterator>,
	    typename Iterator::value_type::first_type::value_type,
	    boost::forward_traversal_tag,
	    const typename Iterator::value_type::first_type::value_type&,
	    ptrdiff_t> {
	public:
		segmented_iterator<Iterator>(Iterator begin_range, Iterator end_range)
		    : begin_(begin_range), current_range_(begin_), end_(end_range),
		    current_(begin_->first)
		{}
	private:
		friend class boost::iterator_core_access;
		
		void increment()
		{
			if (++current_ == current_range_->second)
				current_ = (++current_range_)->first;
		}
	
		bool equal(const segmented_iterator<Iterator> &other) const
		{
			return current_ == other.current_;
		}
	
		typename segmented_iterator<Iterator>::reference dereference() const
		{
			return *current_;
		}
	
		Iterator begin_, current_range_, end_;
		typename Iterator::value_type::first_type current_;
	};
	
	typedef std::vector<std::pair<OMKey, I3OMGeo> >::const_iterator geo_iter_t;
	typedef std::pair<geo_iter_t, geo_iter_t> string_range_t;
	
	struct InXYCircle {
		const I3Position &pos_;
		const double radius_;
		InXYCircle(const I3Position &pos, double radius) :
		    pos_(pos), radius_(radius)
		{}
		bool operator()(const FastMinBall::string_range_t &string_range)
		{
			const I3Position &head = string_range.first->second.position;
			return hypot(pos_.GetX()-head.GetX(), pos_.GetY()-head.GetY()) <= radius_;
		}
	};

	struct InRZCircle {
		const I3Position &pos_;
		const double radius_;
		InRZCircle(const I3Position &pos, double radius) :
		    pos_(pos), radius_(radius)
		{}
		bool operator()(const I3OMGeoMap::value_type &entry)
		{
			const I3Position &head = entry.second.position;
			return hypot(hypot(pos_.GetX()-head.GetX(), pos_.GetY()-head.GetY()),
			    pos_.GetZ()-head.GetZ()) <= radius_;
		}
	};	
	
	typedef boost::filter_iterator<InXYCircle, std::vector<string_range_t>::const_iterator> filtered_string_iterator;
	typedef boost::filter_iterator<InRZCircle, segmented_iterator<filtered_string_iterator> > filtered_dom_iterator;
	typedef boost::iterator_range<filtered_dom_iterator> geo_range;
	
	// Fill internal geometry vector, and cache iterators to the beginning
	// and end of each string.
	FastMinBall(const I3OMGeoMap &geo);
	
	// Get a range representing only the OMs within the given radius
	geo_range GetMinBallGeometry(const I3Position &, double);
	
	// Return a lower bound to the geometry entry for an OMKey, restricting
	// the search range to begin at the given position.
	// This can be used to efficiently look up geometries in order.
	geo_iter_t LowerBound(const OMKey &, geo_iter_t);	

private:
	FastMinBall(const FastMinBall&) {};
	void operator=(const FastMinBall&) {};
	std::vector<std::pair<OMKey, I3OMGeo> > geometry_;
	
	std::vector<string_range_t> strings_;	
	
};

#endif // FILLRATIO_FASTMINBALL_H_INCLUDED
