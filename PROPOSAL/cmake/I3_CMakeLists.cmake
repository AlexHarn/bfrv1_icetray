# - Try to find Log4cplus
# Once done this will define
#  LOG4CPLUS_FOUND - System has Log4cplus
#  LOG4CPLUS_INCLUDE_DIRS - The Log4cplus include directories
#  LOG4CPLUS_LIBRARIES - The libraries needed to use Log4cplus

# TODO: the PROPOSAL_STANDALONE definition is not needed anymore
# add_definitions(-DPROPOSAL_STANDALONE=0)

# file(GLOB_RECURSE PROPOSAL_SRC_FILES ${PROJECT_SOURCE_DIR}/PROPOSAL/private/PROPOSAL/*)
# message(STATUS ${PROPOSAL_SRC_FILES})
i3_project(PROPOSAL
    DOCS_DIR doc
)

# file(GLOB_RECURSE PROPOSAL_SRC_FILES ${PROJECT_SOURCE_DIR}/private/PROPOSAL/*)
set (PROPOSAL_SRC_FILES
    private/PROPOSAL/Constants.cxx
    private/PROPOSAL/EnergyCutSettings.cxx
    private/PROPOSAL/Output.cxx
    private/PROPOSAL/Propagator.cxx
    private/PROPOSAL/PropagatorService.cxx
    private/PROPOSAL/crossection/BremsIntegral.cxx
    private/PROPOSAL/crossection/BremsInterpolant.cxx
    private/PROPOSAL/crossection/CrossSection.cxx
    private/PROPOSAL/crossection/CrossSectionIntegral.cxx
    private/PROPOSAL/crossection/CrossSectionInterpolant.cxx
    private/PROPOSAL/crossection/EpairIntegral.cxx
    private/PROPOSAL/crossection/EpairInterpolant.cxx
    private/PROPOSAL/crossection/IonizIntegral.cxx
    private/PROPOSAL/crossection/IonizInterpolant.cxx
    private/PROPOSAL/crossection/PhotoIntegral.cxx
    private/PROPOSAL/crossection/PhotoInterpolant.cxx
    private/PROPOSAL/crossection/factories/BremsstrahlungFactory.cxx
    private/PROPOSAL/crossection/factories/EpairProductionFactory.cxx
    private/PROPOSAL/crossection/factories/IonizationFactory.cxx
    private/PROPOSAL/crossection/factories/PhotonuclearFactory.cxx
    private/PROPOSAL/crossection/parametrization/Bremsstrahlung.cxx
    private/PROPOSAL/crossection/parametrization/EpairProduction.cxx
    private/PROPOSAL/crossection/parametrization/Ionization.cxx
    private/PROPOSAL/crossection/parametrization/Parametrization.cxx
    private/PROPOSAL/crossection/parametrization/PhotoQ2Integration.cxx
    private/PROPOSAL/crossection/parametrization/PhotoRealPhotonAssumption.cxx
    private/PROPOSAL/crossection/parametrization/Photonuclear.cxx
    private/PROPOSAL/decay/DecayChannel.cxx
    private/PROPOSAL/decay/DecayTable.cxx
    private/PROPOSAL/decay/LeptonicDecayChannel.cxx
    private/PROPOSAL/decay/ManyBodyPhaseSpace.cxx
    private/PROPOSAL/decay/StableChannel.cxx
    private/PROPOSAL/decay/TwoBodyPhaseSpace.cxx
    private/PROPOSAL/geometry/Box.cxx
    private/PROPOSAL/geometry/Cylinder.cxx
    private/PROPOSAL/geometry/Geometry.cxx
    private/PROPOSAL/geometry/GeometryFactory.cxx
    private/PROPOSAL/geometry/Sphere.cxx
    private/PROPOSAL/math/Integral.cxx
    private/PROPOSAL/math/Interpolant.cxx
    private/PROPOSAL/math/InterpolantBuilder.cxx
    private/PROPOSAL/math/RandomGenerator.cxx
    private/PROPOSAL/math/Vector3D.cxx
    private/PROPOSAL/medium/Components.cxx
    private/PROPOSAL/medium/Medium.cxx
    private/PROPOSAL/medium/MediumFactory.cxx
    private/PROPOSAL/methods.cxx
    private/PROPOSAL/particle/Particle.cxx
    private/PROPOSAL/particle/ParticleDef.cxx
    private/PROPOSAL/propagation_utility/ContinuousRandomizer.cxx
    private/PROPOSAL/propagation_utility/PropagationUtility.cxx
    private/PROPOSAL/propagation_utility/PropagationUtilityFactory.cxx
    private/PROPOSAL/propagation_utility/PropagationUtilityIntegral.cxx
    private/PROPOSAL/propagation_utility/PropagationUtilityInterpolant.cxx
    private/PROPOSAL/scattering/Coefficients.cxx
    private/PROPOSAL/scattering/Scattering.cxx
    private/PROPOSAL/scattering/ScatteringFactory.cxx
    private/PROPOSAL/scattering/ScatteringHighland.cxx
    private/PROPOSAL/scattering/ScatteringHighlandIntegral.cxx
    private/PROPOSAL/scattering/ScatteringMoliere.cxx
    private/PROPOSAL/scattering/ScatteringNoScattering.cxx
    private/PROPOSAL/sector/Sector.cxx
)

# execute_process(
#   COMMAND touch ${PROPOSAL_SRC_FILES}
# )
i3_add_library(PROPOSAL
    ${PROPOSAL_SRC_FILES}
    private/PROPOSAL-icetray/I3PropagatorServicePROPOSAL.cxx
    private/PROPOSAL-icetray/SimplePropagator.cxx
    private/PROPOSAL-icetray/Converter.cxx
    # ${I3_PROPOSAL_SRC_FILES}

    USE_TOOLS boost
    USE_PROJECTS icetray serialization dataclasses sim-services simclasses phys-services
)

set_target_properties(PROPOSAL PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror -std=c++11")

i3_add_pybindings(PROPOSAL
    private/PROPOSAL-icetray/pybindings.cxx
    USE_TOOLS boost python
    USE_PROJECTS PROPOSAL
)

# Pre-generate parameterization tables for use in read-only environments
add_executable(PROPOSAL_table_creation
    ${PROJECT_SOURCE_DIR}/private/PROPOSAL-icetray/table_creation.cxx
)
set_target_properties(PROPOSAL_table_creation PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Werror -std=c++11")
target_link_libraries(PROPOSAL_table_creation PROPOSAL)

add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/resources/tables/.tables.auto_generated
    COMMAND ${CMAKE_BINARY_DIR}/env-shell.sh ${CMAKE_BINARY_DIR}/bin/PROPOSAL_table_creation
    DEPENDS icetray PROPOSAL PROPOSAL_table_creation
)
add_custom_target(
    PROPOSAL_tables ALL 
    DEPENDS ${CMAKE_BINARY_DIR}/bin/PROPOSAL_table_creation
    ${PROJECT_SOURCE_DIR}/resources/tables/.tables.auto_generated
)
add_dependencies(PROPOSAL_tables PROPOSAL_table_creation)

set(LIB_${PROJECT_NAME}_TESTS
    private/PROPOSAL-icetray/test/main.cxx
)

# FIXME: See https://code.icecube.wisc.edu/projects/icecube/ticket/2194
if (SPRNG_FOUND)
   # this test requires SPRNG
   LIST(APPEND LIB_${PROJECT_NAME}_TESTS
       private/PROPOSAL-icetray/test/Repeatability.cxx
   )
endif (SPRNG_FOUND)


i3_test_executable(test
    ${LIB_${PROJECT_NAME}_TESTS}
    USE_TOOLS boost gsl python
    USE_PROJECTS PROPOSAL icetray dataclasses phys-services
)
