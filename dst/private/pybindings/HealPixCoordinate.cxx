
#include<dst/HealPixCoordinates.h>

using namespace boost::python;


void register_HealPixCoordinate()
{

#define FIELDS (ComputeBins)
#define RO_PROPS (NSide) (PixSize) 

	class_<HealPixCoordinate,
	boost::shared_ptr<HealPixCoordinate> >("HealPixCoordinate")
    BOOST_PP_SEQ_FOR_EACH(WRAP_PROP_RO, HealPixCoordinate, RO_PROPS)	
	BOOST_PP_SEQ_FOR_EACH(WRAP_DEF, HealPixCoordinate, FIELDS)
    .def("GetCoords",&HealPixCoordinate::GetCoords)
    .def("index",    &HealPixCoordinate::GetIndex)
    .def("bins",     &HealPixCoordinate::NumberOfBins)
	.def("SetRNG",&HealPixCoordinate::SetRNG)
    ;

}

