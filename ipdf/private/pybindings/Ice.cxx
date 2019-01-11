#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/preprocessor.hpp>
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/LayeredIce.h"
#include <string>

namespace bp=boost::python;
namespace ip=IPDF::Pandel;

#define TD_ABS_DOCTXT "Absorption length"
#define TD_INVABS_DOCTXT "Absorptivity (inverse of absorption length)"
#define TD_TAU_DOCTXT "Phenomenological andel parameter related to light propagation"
#define TD_LAM_DOCTXT "Effective scattering length"
#define TD_INVLAM_DOCTXT "Inverse of the effective scattering length"
#define TD_DIST_P1_DOCTXT "Hole ice parameters P1 (used for calculating the 'effective distance', using a penalty term to account for DOM orientation w.r.t. track/shower)"
#define TD_DIST_P0_CS0_DOCTXT "Hole ice parameters P0_CS0 (used for calculating the 'effective distance', using a penalty term to account for DOM orientation w.r.t. track/shower)"
#define TD_DIST_P0_CS1_DOCTXT "Hole ice parameters P0_CS1 (used for calculating the 'effective distance', using a penalty term to account for DOM orientation w.r.t. track/shower)"
#define TD_DIST_P0_CS2_DOCTXT "Hole ice parameters P0_CS2 (used for calculating the 'effective distance', using a penalty term to account for DOM orientation w.r.t. track/shower)"
#define DEPTH_DOCTXT " at a given depth"
#define AVERAGED_DOCTXT " averaged for some given layered ice model between emitter (track, shower) and DOM"
#define BULK_DOCTXT " (bulk ice)"
#define EDITBULK_DOCTXT " (bulk ice, editable)"


#define READONLY_REG(bulkice) \
    .def_readonly("TD_ABS", bulkice::TD_ABS, TD_ABS_DOCTXT BULK_DOCTXT ) \
    .def_readonly("TD_TAU", bulkice::TD_TAU, TD_TAU_DOCTXT BULK_DOCTXT ) \
    .def_readonly("TD_LAM", bulkice::TD_LAM, TD_LAM_DOCTXT BULK_DOCTXT ) \
    .def_readonly("TD_DIST_P1", bulkice::TD_DIST_P1, TD_DIST_P1_DOCTXT ) \
    .def_readonly("TD_DIST_P0_CS0", bulkice::TD_DIST_P0_CS0, TD_DIST_P0_CS0_DOCTXT ) \
    .def_readonly("TD_DIST_P0_CS1", bulkice::TD_DIST_P0_CS1, TD_DIST_P0_CS1_DOCTXT ) \
    .def_readonly("TD_DIST_P0_CS2", bulkice::TD_DIST_P0_CS2, TD_DIST_P0_CS2_DOCTXT )

#define BULKICE_REG(bulkice,bulkicename) \
    bp::class_< bulkice, bp::bases<>, boost::shared_ptr<bulkice>, boost::noncopyable >(bulkicename,"Static bulk ice model parameters for the Pandel implementation in ipdf.") \
    .def("tau_scale",&bulkice::TauScale, TD_TAU_DOCTXT BULK_DOCTXT ) \
    .staticmethod("tau_scale") \
    .def("absorption_length",&bulkice::AbsorptionLength, TD_ABS_DOCTXT BULK_DOCTXT ) \
    .staticmethod("absorption_length") \
    .def("absorptivity",&bulkice::Absorptivity, TD_INVABS_DOCTXT BULK_DOCTXT ) \
    .staticmethod("absorptivity") \
    .def("eff_scatt_length",&bulkice::EffScattLength, TD_LAM_DOCTXT BULK_DOCTXT ) \
    .staticmethod("eff_scatt_length") \
    .def("inv_eff_scatt_length",&bulkice::InvEffScattLength, TD_INVLAM_DOCTXT BULK_DOCTXT ) \
    .staticmethod("inv_eff_scatt_length") \
    READONLY_REG(bulkice) \
    .def( freeze() )

#define LAYERED_ICE_REG(HoleIce) \
    bp::class_< ip::LayeredIce<ip::HoleIce>, boost::shared_ptr< ip::LayeredIce<ip::HoleIce> >, boost::noncopyable > \
      ("LayeredIce"#HoleIce,"Layered ice model for Pandel reconstruction, using " # HoleIce " for the hole ice modeling (input taken from a photonics-formatted ice file).", bp::init<std::string>((bp::arg("icefile")))) \
    .def("absorption_length",  &ip::LayeredIce<ip::HoleIce>::LocalAbsorptionLength, TD_ABS_DOCTXT DEPTH_DOCTXT ) \
    .def("absorptivity",  &ip::LayeredIce<ip::HoleIce>::LocalAbsorptivity, TD_INVABS_DOCTXT DEPTH_DOCTXT ) \
    .def("eff_scatt_length",  &ip::LayeredIce<ip::HoleIce>::LocalEffScattLength, TD_LAM_DOCTXT DEPTH_DOCTXT ) \
    .def("inv_eff_scatt_length",  &ip::LayeredIce<ip::HoleIce>::LocalInvEffScattLength, TD_INVLAM_DOCTXT DEPTH_DOCTXT ) \
    .def("averaged_absorption_length",  &ip::LayeredIce<ip::HoleIce>::AveragedAbsorptionLength, TD_ABS_DOCTXT AVERAGED_DOCTXT ) \
    .def("averaged_absorptivity",  &ip::LayeredIce<ip::HoleIce>::AveragedAbsorptivity, TD_INVABS_DOCTXT AVERAGED_DOCTXT ) \
    .def("averaged_eff_scatt_length",  &ip::LayeredIce<ip::HoleIce>::AveragedEffScattLength, TD_LAM_DOCTXT AVERAGED_DOCTXT ) \
    .def("averaged_inv_eff_scatt_length",  &ip::LayeredIce<ip::HoleIce>::AveragedInvEffScattLength, TD_INVLAM_DOCTXT AVERAGED_DOCTXT ) \
    READONLY_REG(ip::HoleIce)

void register_Ice(){
    BULKICE_REG(ip::H0,"H0");
    BULKICE_REG(ip::H1,"H1");
    BULKICE_REG(ip::H2,"H2");
    BULKICE_REG(ip::H3,"H3");
    BULKICE_REG(ip::H4,"H4");
    BULKICE_REG(ip::CascIce0<ip::H0>,"CascadeH0");
    BULKICE_REG(ip::CascIce0<ip::H1>,"CascadeH1");
    BULKICE_REG(ip::CascIce0<ip::H2>,"CascadeH2");
    BULKICE_REG(ip::CascIce0<ip::H3>,"CascadeH3");
    BULKICE_REG(ip::CascIce0<ip::H4>,"CascadeH4");

    bp::class_<ip::UserDefIce,bp::bases<>,boost::shared_ptr<ip::UserDefIce>, boost::noncopyable >("UserDefIce","Modifiable bulk ice model parameters for the Pandel implementation in ipdf.")
        .def(bp::init<>())
        .def(bp::init<boost::shared_ptr<const ip::H0>,const std::string>( (bp::arg("H0"),bp::arg("name")),"constructor with H0 initial values"))
        .def(bp::init<boost::shared_ptr<const ip::H1>,const std::string>( (bp::arg("H1"),bp::arg("name")),"constructor with H1 initial values"))
        .def(bp::init<boost::shared_ptr<const ip::H2>,const std::string>( (bp::arg("H2"),bp::arg("name")),"constructor with H2 initial values"))
        .def(bp::init<boost::shared_ptr<const ip::H3>,const std::string>( (bp::arg("H3"),bp::arg("name")),"constructor with H3 initial values"))
        .def(bp::init<boost::shared_ptr<const ip::H4>,const std::string>( (bp::arg("H4"),bp::arg("name")),"constructor with H4 initial values"))
        .def("tau_scale",&ip::UserDefIce::TauScale, TD_TAU_DOCTXT EDITBULK_DOCTXT )
        .def("absorption_length",&ip::UserDefIce::AbsorptionLength,TD_ABS_DOCTXT EDITBULK_DOCTXT )
        .def("absorptivity",&ip::UserDefIce::Absorptivity,TD_INVABS_DOCTXT EDITBULK_DOCTXT )
        .def("eff_scatt_length",&ip::UserDefIce::EffScattLength,TD_LAM_DOCTXT EDITBULK_DOCTXT )
        .def("inv_eff_scatt_length",&ip::UserDefIce::InvEffScattLength,TD_INVLAM_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_ABS", &ip::UserDefIce::TD_ABS,  TD_ABS_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_TAU", &ip::UserDefIce::TD_TAU,  TD_TAU_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_LAM", &ip::UserDefIce::TD_LAM,  TD_LAM_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_DIST_P1", &ip::UserDefIce::TD_DIST_P1,  TD_DIST_P1_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_DIST_P0_CS0", &ip::UserDefIce::TD_DIST_P0_CS0,  TD_DIST_P0_CS0_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_DIST_P0_CS1", &ip::UserDefIce::TD_DIST_P0_CS1,  TD_DIST_P0_CS1_DOCTXT EDITBULK_DOCTXT )
        .def_readwrite("TD_DIST_P0_CS2", &ip::UserDefIce::TD_DIST_P0_CS2,  TD_DIST_P0_CS2_DOCTXT EDITBULK_DOCTXT )
        ;

    LAYERED_ICE_REG(H0);
    LAYERED_ICE_REG(H1);
    LAYERED_ICE_REG(H2);
    LAYERED_ICE_REG(H3);
    LAYERED_ICE_REG(H4);
}
