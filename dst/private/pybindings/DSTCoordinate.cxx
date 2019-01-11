
#include<dst/DSTCoordinates.h>
#include<dst/HealPixCoordinates.h>

using namespace boost::python;

void register_DSTCoordinate()
{

    class_<DSTCoordinate, boost::shared_ptr<DSTCoordinate> >
    ("DSTCoordinate",init<float, float, uint8_t>(args("dtheta","dphi","digits")))
    .def("ComputeBins", &DSTCoordinate::ComputeBins) 
    .def("compute_dph", &DSTCoordinate::compute_dphi)
    .def("mkpair",      &DSTCoordinate::mkpair)
    .def("GetCoords",   &DSTCoordinate::GetCoords)
    .def("GetIndex",    &DSTCoordinate::GetIndex)
    .def("NumberofBins",&DSTCoordinate::NumberOfBins)
    ;
}


