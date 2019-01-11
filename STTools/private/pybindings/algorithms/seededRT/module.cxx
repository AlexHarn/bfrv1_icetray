/**
 * Copyright (C) 2013 - 2014
 * Martin Wolf <martin.wolf@fysik.su.se>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/pybindings/algorithms/seededRT/module.cxx
 * @date $Date$
 * @brief This file contains the Python bindings for the STTools.seededRT
 *        python module.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"

#include "STTools/pybindings/register_I3Vector_of.hpp"
#include "STTools/algorithms/seededRT/utilities.h"

namespace bp = boost::python;

namespace sttools {
namespace seededRT {
namespace py {

void register_I3SeededRTConfiguration();
void register_I3SeededRTSContext();
void register_I3SeededRTConfigurationService();

//______________________________________________________________________________
// Specialize the seed procedures for the different hit types.
// -- seedWithAllHLCHits
typedef sttools::seededRT::seedWithAllHLCHits<I3RecoPulse> seedWithAllHLCHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithAllHLCHits<I3DOMLaunch> seedWithAllHLCHits_I3DOMLaunch_;

// -- seedWithAllCoreHits
typedef sttools::seededRT::seedWithAllCoreHits<I3RecoPulse> seedWithAllCoreHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithAllCoreHits<I3DOMLaunch> seedWithAllCoreHits_I3DOMLaunch_;

// -- seedWithHLCCoreHits
typedef sttools::seededRT::seedWithHLCCoreHits<I3RecoPulse> seedWithHLCCoreHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithHLCCoreHits<I3DOMLaunch> seedWithHLCCoreHits_I3DOMLaunch_;

/*
// -- seedWithHLCCOGSTHits
typedef sttools::seededRT::seedWithHLCCOGSTHits<I3RecoPulse> seedWithHLCCOGSTHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithHLCCOGSTHits<I3DOMLaunch> seedWithHLCCOGSTHits_I3DOMLaunch_;
*/

// -- seedWithOMKeyHits
typedef sttools::seededRT::seedWithOMKeyHits<I3RecoPulse> seedWithOMKeyHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithOMKeyHits<I3DOMLaunch> seedWithOMKeyHits_I3DOMLaunch_;

// -- seedWithNthOMKeyHits
typedef sttools::seededRT::seedWithNthOMKeyHits<I3RecoPulse> seedWithNthOMKeyHits_I3RecoPulse_;
typedef sttools::seededRT::seedWithNthOMKeyHits<I3DOMLaunch> seedWithNthOMKeyHits_I3DOMLaunch_;

// -- seedWithHitSeriesMapHitsFromFrame
typedef sttools::seededRT::seedWithHitSeriesMapHitsFromFrame<I3RecoPulse> seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_;
typedef sttools::seededRT::seedWithHitSeriesMapHitsFromFrame<I3DOMLaunch> seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_;

//______________________________________________________________________________
void register_seed_procedures()
{
    // Tell boost::python that it should create the python docstrings with
    // user-defined docstrings, python signatures, but no C++ signatures.
    bp::docstring_options docstring_opts(true, true, false);

    //--------------------------------------------------------------------------
    // -- seedWithAllHLCHits
    // ---- for I3RecoPulse
    bp::class_<seedWithAllHLCHits_I3RecoPulse_, boost::shared_ptr<seedWithAllHLCHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_all_HLC_hits_I3RecoPulse",
        bp::init<>(
            (bp::arg("self")),
            "Creates an object to seed with all reco pulses, that are HLC reco     \n"
            "pulses.                                                               \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithAllHLCHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithAllHLCHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithAllHLCHits_I3DOMLaunch_, boost::shared_ptr<seedWithAllHLCHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_all_HLC_hits_I3DOMLaunch",
        bp::init<>(
            (bp::arg("self")),
            "Creates an object to seed with all DOM launches, that are HLC DOM     \n"
            "launches.                                                             \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithAllHLCHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithAllHLCHits_I3DOMLaunch_>
        >();

    //--------------------------------------------------------------------------
    // -- seedWithAllCoreHits
    // ---- for I3RecoPulse
    bp::class_<seedWithAllCoreHits_I3RecoPulse_, boost::shared_ptr<seedWithAllCoreHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_all_core_hits_I3RecoPulse",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, uint32_t, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("nHitsThreshold"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed all reco pulses that have at least          \n"
            "nHitsThreshold partner ST reco pulses.                                \n"
            "                                                                      \n"
            ".. note::                                                             \n"
            "                                                                      \n"
            "    If the allowNoSeedHits option is set to ``False`` and no reco     \n"
            "    pulses fulfill the requirements above, all reco pulses will be    \n"
            "    used as seed hits!                                                \n"
            "                                                                      \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithAllCoreHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithAllCoreHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithAllCoreHits_I3DOMLaunch_, boost::shared_ptr<seedWithAllCoreHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_all_core_hits_I3DOMLaunch",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, uint32_t, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("nHitsThreshold"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed all DOM launches that have at least         \n"
            "nHitsThreshold partner ST DOM launches.                               \n"
            "                                                                      \n"
            ".. note::                                                             \n"
            "                                                                      \n"
            "    If the allowNoSeedHits option is set to ``False`` and no DOM      \n"
            "    launches fulfill the requirements above, all DOM launches will be \n"
            "    used as seed hits!                                                \n"
            "                                                                      \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithAllCoreHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithAllCoreHits_I3DOMLaunch_>
        >();

    //--------------------------------------------------------------------------
    // -- seedWithHLCCoreHits
    // ---- for I3RecoPulse
    bp::class_<seedWithHLCCoreHits_I3RecoPulse_, boost::shared_ptr<seedWithHLCCoreHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_HLC_core_hits_I3RecoPulse",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, uint32_t, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("nHitsThreshold"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed with all HLC reco pulses that have          \n"
            "at least nHitsThreshold partner ST HLC reco pulses.                   \n"
            "                                                                      \n"
            ".. note::                                                             \n"
            "                                                                      \n"
            "    If the allowNoSeedHits option is set to ``False`` and no HLC      \n"
            "    reco pulses fulfill the requirements above, all HLC reco pulses   \n"
            "    will be used as seed hits!                                        \n"
            "                                                                      \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHLCCoreHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithHLCCoreHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithHLCCoreHits_I3DOMLaunch_, boost::shared_ptr<seedWithHLCCoreHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_HLC_core_hits_I3DOMLaunch",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, uint32_t, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("nHitsThreshold"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed with all HLC DOM launches that have         \n"
            "at least nHitsThreshold partner ST HLC DOM launches.                  \n"
            "                                                                      \n"
            ".. note::                                                             \n"
            "                                                                      \n"
            "    If the allowNoSeedHits option is set to ``False`` and no HLC      \n"
            "    DOM launches fulfill the requirements above, all HLC DOM launches \n"
            "    will be used as seed hits!                                        \n"
            "                                                                      \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHLCCoreHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithHLCCoreHits_I3DOMLaunch_>
        >();

    /*
    //--------------------------------------------------------------------------
    // -- seedWithHLCCOGSTHits
    // ---- for I3RecoPulse
    bp::class_<seedWithHLCCOGSTHits_I3RecoPulse_, boost::shared_ptr<seedWithHLCCOGSTHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_HLC_COG_ST_hits_I3RecoPulse",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed with reco pulses, that fulfill the ST       \n"
            "conditions around the Center-of-Gravity (COG) of all HLC reco pulses. \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHLCCOGSTHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithHLCCOGSTHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithHLCCOGSTHits_I3DOMLaunch_, boost::shared_ptr<seedWithHLCCOGSTHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_HLC_COG_ST_hits_I3DOMLaunch",
        bp::init<const sttools::seededRT::I3SeededRTConfigurationService&, bool>(
            (bp::arg("self"),
             bp::arg("stConfigService"),
             bp::arg("allowNoSeedHits")
            ),
            "Creates an object to seed with DOM launches, that fulfill the ST      \n"
            "conditions around the Center-of-Gravity (COG) of all HLC DOM launches.\n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHLCCOGSTHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithHLCCOGSTHits_I3DOMLaunch_>
        >();
    */

    //--------------------------------------------------------------------------
    // -- seedWithOMKeyHits
    // ---- for I3RecoPulse
    bp::class_<seedWithOMKeyHits_I3RecoPulse_, boost::shared_ptr<seedWithOMKeyHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_OMKey_hits_I3RecoPulse",
        bp::init<const std::vector<OMKey>&>(
            (bp::arg("self"),
             bp::arg("omKeys")
            ),
            "Creates an object to seed with all reco pulses that belong to the     \n"
            "OMs present in the given OMKey object list.                           \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithOMKeyHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithOMKeyHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithOMKeyHits_I3DOMLaunch_, boost::shared_ptr<seedWithOMKeyHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_OMKey_hits_I3DOMLaunch",
        bp::init<const std::vector<OMKey>&>(
            (bp::arg("self"),
             bp::arg("omKeys")
            ),
            "Creates an object to seed with all DOM launches that belong to the    \n"
            "OMs present in the given OMKey object list.                           \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithOMKeyHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithOMKeyHits_I3DOMLaunch_>
        >();

    //--------------------------------------------------------------------------
    // -- seedWithNthOMKeyHits
    // ---- for I3RecoPulse
    bp::class_<seedWithNthOMKeyHits_I3RecoPulse_, boost::shared_ptr<seedWithNthOMKeyHits_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_Nth_OMKey_hits_I3RecoPulse",
        bp::init<uint32_t, const std::vector<OMKey>&>(
            (bp::arg("self"),
             bp::arg("nth"),
             bp::arg("omKeys")
            ),
            "Creates an object to seed with all reco pulses that belong to the OMs \n"
            "present in the given OMKey object list and that are the N-th reco     \n"
            "pulses within the OM's reco pulse series.                             \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithNthOMKeyHits_I3RecoPulse_>,
            boost::shared_ptr<const seedWithNthOMKeyHits_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithNthOMKeyHits_I3DOMLaunch_, boost::shared_ptr<seedWithNthOMKeyHits_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_Nth_OMKey_hits_I3DOMLaunch",
        bp::init<uint32_t, const std::vector<OMKey>&>(
            (bp::arg("self"),
             bp::arg("nth"),
             bp::arg("omKeys")
            ),
            "Creates an object to seed with all DOM launches that belong to the    \n"
            "OMs present in the given OMKey object list and that are the N-th DOM  \n"
            "launches within the OM's DOM launch series.                           \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithNthOMKeyHits_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithNthOMKeyHits_I3DOMLaunch_>
        >();

    //--------------------------------------------------------------------------
    // -- seedWithHitSeriesMapHitsFromFrame
    // ---- for I3RecoPulse
    bp::class_<seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_, boost::shared_ptr<seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_>, boost::noncopyable>(
        "seed_with_hit_series_map_hits_from_frame_I3RecoPulse",
        bp::init<std::string>(
            (bp::arg("self"),
             bp::arg("seedHitSeriesMapName")
            ),
            "Creates a class object to seed with all hits which are contained      \n"
            "inside an I3RecoPulseSeriesMap or I3RecoPulseSeriesMapMask object     \n"
            "named ``seedHitSeriesMapName`` present in the current processed frame.\n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_>,
            boost::shared_ptr<const seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_>
        >();

    // ---- for I3DOMLaunch
    bp::class_<seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_, boost::shared_ptr<seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_>, boost::noncopyable>(
        "seed_with_hit_series_map_hits_from_frame_I3DOMLaunch",
        bp::init<std::string>(
            (bp::arg("self"),
             bp::arg("seedHitSeriesMapName")
            ),
            "Creates a class object to seed with all hits which are contained      \n"
            "inside an I3DOMLaunchSeriesMap object named ``seedHitSeriesMapName``  \n"
            "present in the current processed frame.                               \n"
        ))
    ;
    bp::implicitly_convertible<
            boost::shared_ptr<seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_>,
            boost::shared_ptr<const seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_>
        >();
}

//______________________________________________________________________________
/** Define a wrapper function template to call the C++
 *  sttools::seededRT::doSeededRTCleaning utility function for each different
 *  possible HitType, OutType, and SeedProcedureType.
 */
template <
    class HitType,
    class OutType,
    typename SeedProcedureType
>
static
boost::shared_ptr< OutType >
doSeededRTCleaning(
    const sttools::seededRT::I3SeededRTConfigurationService&  stConfigService,
    const SeedProcedureType&                                  seedProcedure,
    const I3Frame&                                            frame,
    const std::string&                                        inputHitSeriesMapName,
    int32_t                                                   maxNIterations,
    bool                                                      allowNoSeedHits
)
{
    return sttools::seededRT::doSeededRTCleaning<HitType, OutType>(
        stConfigService, seedProcedure, frame, inputHitSeriesMapName,
        maxNIterations, allowNoSeedHits);
}

#define SIGNATURE__doSeededRTCleaning(HitType, OutType, SeedProcedureType)      \
    doSeededRTCleaning<HitType, OutType, SeedProcedureType>,                    \
        (bp::arg("stConfigService"),                                            \
         bp::arg("seedProcedure"),                                              \
         bp::arg("frame"),                                                      \
         bp::arg("inputHitSeriesMapName"),                                      \
         bp::arg("maxNIterations")=-1,                                          \
         bp::arg("allowNoSeedHits")=false                                       \
        )

//______________________________________________________________________________
void register_doSeededRTCleaning_utility_function()
{
    // Tell boost::python that it should create the python docstrings with
    // user-defined docstrings, python signatures, but no C++ signatures.
    bp::docstring_options docstring_opts(true, true, false);

    // Expose the doSeededRTCleaning utility function to Python. Since in C++
    // this utility function is a function template and Python does not know
    // about templates, we need to expose the
    // sttools::seededRT::doSeededRTCleaning utility
    // function for each single hit type, output type and seed procedure type.

    //--------------------------------------------------------------------------
    // -- HitType = I3RecoPulse, OutType = I3RecoPulseSeriesMap
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithAllHLCHits_I3RecoPulse_),
        "Does a seededRT cleaning on a given I3RecoPulseSeriesMap or            \n"
        "I3RecoPulseSeriesMapMask named ``inputHitSeriesMapName`` stored inside \n"
        "a given I3Frame object using a given I3SeededRTConfigurationService    \n"
        "object and a given seed procedure object.                              \n"
        "                                                                       \n"
        ":return I3RecoPulseSeriesMap: The seededRT cleaned hit series map as   \n"
        "    an I3RecoPulseSeriesMap object.                                    \n"
    );
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithAllCoreHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithHLCCoreHits_I3RecoPulse_)
    );
//     bp::def("doSeededRTCleaning_RecoPulse_",
//         SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithHLCCOGSTHits_I3RecoPulse_)
//     );
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithOMKeyHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithNthOMKeyHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulse",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMap, seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_)
    );

    // -- HitType = I3RecoPulse, OutType = I3RecoPulseSeriesMapMask
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithAllHLCHits_I3RecoPulse_),
        "Does a seededRT cleaning on a given I3RecoPulseSeriesMap or            \n"
        "I3RecoPulseSeriesMapMask named ``inputHitSeriesMapName`` stored inside \n"
        "a given I3Frame object using a given I3SeededRTConfigurationService    \n"
        "object and a given seed procedure object.                              \n"
        "                                                                       \n"
        ":return I3RecoPulseSeriesMapMask: The seededRT cleaned hit series map  \n"
        "    as an I3RecoPulseSeriesMapMask object.                             \n"
    );
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithAllCoreHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithHLCCoreHits_I3RecoPulse_)
    );
//     bp::def("doSeededRTCleaning_RecoPulseMask",
//         SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithHLCCOGSTHits_I3RecoPulse_)
//     );
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithOMKeyHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithNthOMKeyHits_I3RecoPulse_)
    );
    bp::def("doSeededRTCleaning_RecoPulseMask",
        SIGNATURE__doSeededRTCleaning(I3RecoPulse, I3RecoPulseSeriesMapMask, seedWithHitSeriesMapHitsFromFrame_I3RecoPulse_)
    );

    //--------------------------------------------------------------------------
    // -- HitType = I3DOMLaunch, OutType = I3DOMLaunchSeriesMap
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithAllHLCHits_I3DOMLaunch_),
        "Does a seededRT cleaning on a given I3DOMLaunchSeriesMap named         \n"
        "``inputHitSeriesMapName`` stored inside a given I3Frame object using   \n"
        "a given I3SeededRTConfigurationService object and a given seed         \n"
        "procedure object.                                                      \n"
        "                                                                       \n"
        ":return I3DOMLaunchSeriesMap: The seededRT cleaned hit series map as   \n"
        "    an I3DOMLaunchSeriesMap object.                                    \n"
    );
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithAllCoreHits_I3DOMLaunch_)
    );
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithHLCCoreHits_I3DOMLaunch_)
    );
//     bp::def("doSeededRTCleaning_DOMLaunch",
//         SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithHLCCOGSTHits_I3DOMLaunch_)
//     );
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithOMKeyHits_I3DOMLaunch_)
    );
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithNthOMKeyHits_I3DOMLaunch_)
    );
    bp::def("doSeededRTCleaning_DOMLaunch",
        SIGNATURE__doSeededRTCleaning(I3DOMLaunch, I3DOMLaunchSeriesMap, seedWithHitSeriesMapHitsFromFrame_I3DOMLaunch_)
    );
}

//______________________________________________________________________________
void register_module()
{
    // Import the seededRT python module and register all the C++ classes in it.
    bp::object seededRT_module(bp::handle<>(PyImport_ImportModule("icecube.STTools.seededRT")));
    {
        bp::scope seededRT_module_scope(seededRT_module);

        register_I3SeededRTConfiguration();
        register_I3SeededRTSContext();
        register_I3SeededRTConfigurationService();
        register_seed_procedures();
        register_doSeededRTCleaning_utility_function();
    }
}

}// namespace py
}// namespace seededRT
}// namespace sttools
