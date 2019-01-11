/**
 *
 * Implementation of Python bindings for seed services
 *
 * (c) 2018
 * the IceCube Collaboration
 * $Id$
 *
 * @file seedservice.cxx
 * @date $Date$
 * @author kkrings
 *
 */
#include <limits>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/preprocessor.hpp>
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <icetray/I3Units.h>
#include <gulliver/I3SeedServiceBase.h>
#include <lilliput/seedservice/I3BasicSeedService.h>

namespace bp = boost::python;

#define SEEDSERVICES_ENUM_VALUE(r, name, key) \
    .value( \
        BOOST_PP_STRINGIZE(key), \
        name::key \
    )

namespace
{
    /**
     * @brief Wrap the `I3BasicSeedService` constructor.
     *
     * Extract first guesses, input pulsemap, and energy estimates from Python
     * objects.
     *
     */
    I3BasicSeedServicePtr
    InitBasicSeedService(
        const std::string& name,
        const bp::object& firstGuessNames,
        const bp::object& inputReadout,
        const bp::object& fixedEnergy,
        const bp::object& energyGuessPolynomial,
        I3BasicSeedService::I3PositionShiftType posShiftType,
        I3BasicSeedService::I3TimeShiftType timeShiftType,
        I3BasicSeedService::I3TimeShiftType altTimeShiftType,
        bool speedPolice,
        double maxTResMean,
        double chargeFraction,
        I3BasicSeedService::I3SeedAlternatives altMode,
        bool onlyAlternatives
    )
    {
        std::vector<std::string> firstGuessNamesVector;

        if (!firstGuessNames.is_none())
        {
            for (bp::ssize_t i = 0, n = bp::len(firstGuessNames); i < n; ++i)
            {
                firstGuessNamesVector.push_back(
                    bp::extract<std::string>(firstGuessNames[i])
                );
            }
        }

        std::string inputReadoutString = "";

        if (!inputReadout.is_none())
        {
            inputReadoutString = bp::extract<std::string>(inputReadout);
        }

        double fixedEnergyDouble = std::numeric_limits<double>::quiet_NaN();

        if (!fixedEnergy.is_none())
        {
            fixedEnergyDouble = bp::extract<double>(fixedEnergy);
        }

        std::vector<double> energyGuessPolynomialVector;

        if (!energyGuessPolynomial.is_none())
        {
            for (
                bp::ssize_t i = 0, n = bp::len(energyGuessPolynomial);
                i < n; ++i
            )
            {
                energyGuessPolynomialVector.push_back(
                    bp::extract<double>(energyGuessPolynomial[i])
                );
            }
        }

        auto basicSeedService = boost::make_shared<I3BasicSeedService>(
            name,
            firstGuessNamesVector,
            inputReadoutString,
            fixedEnergyDouble,
            energyGuessPolynomialVector,
            posShiftType,
            timeShiftType,
            altTimeShiftType,
            speedPolice,
            maxTResMean,
            chargeFraction,
            altMode,
            onlyAlternatives
        );

        return basicSeedService;
    }
}

void register_seedservice()
{
    bp::class_<
        I3BasicSeedService,
        bp::bases<I3SeedServiceBase>,
        I3BasicSeedServicePtr,
        boost::noncopyable
    >
    basicSeedService(
        "I3BasicSeedService",
        "Basic seed service\n"
        "\n"
        "The basic seed service collects first-guess tracks and can do a\n"
        "simple vertex correction.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "    Logging identifier\n"
        "first_guess_names : list(str), optional\n"
        "    Names of results of first-guess modules.\n"
        "input_readout : str, optional\n"
        "    Name of pulses to use for vertex correction\n"
        "fixed_energy : float, optional\n"
        "    If the first guesses do not provide an energy estimate,\n"
        "    give the energy reconstruction a reasonable starting value.\n"
        "energy_guess_polynomial : array_like, optional\n"
        "    Provide a sequence of coefficients of the polynomial\n"
        "    ``log(E/1 GeV) = p0 + p1*x + p2*x^2 + ...`` where ``x =\n"
        "    log10(NCh)``, which will be used to obtain an energy\n"
        "    estimate from the number of hit DOMs ``NCh``.\n"
        "pos_shift_type : I3PositionShiftType, optional\n"
        "    Vertex position correction method; only relevant for\n"
        "    infinite tracks. For other particle shapes, the vertex\n"
        "    position is taken from the first guesses.\n"
        "time_shift_type : I3TimeShiftType, optional\n"
        "    Vertex time correction method\n"
        "alt_time_shift_type : I3TimeShiftType, optional\n"
        "    Vertex time correction method for alternative seeds;\n"
        "    see `alt_mode`.\n"
        "speed_police : bool, optional\n"
        "    If `True`, set the speed of infinite tracks to the vacuum\n"
        "    speed of light and that of cascades to zero. Otherwise,\n"
        "    keep the value of the first guess.\n"
        "max_tres_mean : float, optional\n"
        "    If the time correction method `TFirst` is used, ignore\n"
        "    pulses with a time residual less than `max_tres_mean` below\n"
        "    the mean.\n"
        "charge_fraction : float, optional\n"
        "    If the time correction methods `TChargeFraction`\n"
        "    or `TDirectChargeFraction` are used, configure the fraction\n"
        "    of the charge that should have a positive time residual.\n"
        "alt_mode : I3SeedAlternatives, optional\n"
        "    Alternative seeding method\n"
        "only_alternatives : bool, optional\n"
        "    If `True`, use only the alternative seeds.",
        bp::no_init
    );

    {
        bp::scope basicSeedServiceScope(basicSeedService);

        bp::enum_<I3BasicSeedService::I3TimeShiftType>(
            "I3TimeShiftType",
            "Vertex time correction methods\n"
            "\n"
            "Attributes\n"
            "----------\n"
            "TNone\n"
            "    No correction\n"
            "TMean\n"
            "    Set the vertex time such that the mean time residual of\n"
            "    the pulses is zero.\n"
            "TFirst\n"
            "    Set the vertex time such that the minimum time time\n"
            "    residual of the pulses is zero.\n"
            "TChargeFraction\n"
            "    Choose the vertex time such that the charge-weighted\n"
            "    fraction of all pulses has a positive time residual.\n"
            "TDirectChargeFraction\n"
            "    Do the same as for `TChargeFraction` but use only the\n"
            "    first pulse in every DOM.\n"
            "\n"
        )
        BOOST_PP_SEQ_FOR_EACH(
            SEEDSERVICES_ENUM_VALUE,
            I3BasicSeedService::I3TimeShiftType,
            I3BASICSEEDSERVICE_I3TIMESHIFTTYPE_SEQ
        );

        bp::enum_<I3BasicSeedService::I3PositionShiftType>(
            "I3PositionShiftType",
            "Vertex position correction methods for infinite tracks\n"
            "\n"
            "Attributes\n"
            "----------\n"
            "PNone\n"
            "    No correcion\n"
            "PCOG\n"
            "    Shift the vertex position along the track to the\n"
            "    closest point to the center of gravity of the pulses.\n"
            "\n"
        )
        BOOST_PP_SEQ_FOR_EACH(
            SEEDSERVICES_ENUM_VALUE,
            I3BasicSeedService::I3PositionShiftType,
            I3BASICSEEDSERVICE_I3POSITIONSHIFTTYPE_SEQ
        );

        bp::enum_<I3BasicSeedService::I3SeedAlternatives>(
            "I3SeedAlternatives",
            "Seeding alternatives\n"
            "\n"
            "Attributes\n"
            "----------\n"
            "SeedAlt_None\n"
            "    No extra seeds\n"
            "SeedAlt_Reverse\n"
            "    Add a track through the same vertex but with opposite\n"
            "    direction for each first guess.\n"
            "SeedAlt_Tetrahedron\n"
            "    Add three more tracks with tetrahedric angles for each\n"
            "    first guess.\n"
            "SeedAlt_Cube\n"
            "    Add a track with opposite direction and four tracks\n"
            "    with perpendicular directions for each first guess.\n"
            "\n"
        )
        BOOST_PP_SEQ_FOR_EACH(
            SEEDSERVICES_ENUM_VALUE,
            I3BasicSeedService::I3SeedAlternatives,
            I3BASICSEEDSERVICE_I3SEEDALTERNATIVES_SEQ
        );
    }

    static bp::object none;

    basicSeedService.def(
        "__init__",
        bp::make_constructor(
            InitBasicSeedService,
            bp::default_call_policies(),
            (
                bp::arg("name")="I3BasicSeedService",
                bp::arg("first_guess_names")=none,
                bp::arg("input_readout")=none,
                bp::arg("fixed_energy")=none,
                bp::arg("energy_guess_polynomial")=none,
                bp::arg("pos_shift_type")=
                    I3BasicSeedService::I3PositionShiftType::PCOG,
                bp::arg("time_shift_type")=
                    I3BasicSeedService::I3TimeShiftType::TMean,
                bp::arg("alt_time_shift_type")=
                    I3BasicSeedService::I3TimeShiftType::TFirst,
                bp::arg("speed_police")=true,
                bp::arg("max_tres_mean")=1000.*I3Units::ns,
                bp::arg("charge_fraction")=0.9,
                bp::arg("alt_mode")=
                    I3BasicSeedService::I3SeedAlternatives::SeedAlt_None,
                bp::arg("only_alternatives")=false
            )
        )
    );
}
