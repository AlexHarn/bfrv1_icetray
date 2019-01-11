#
i3_project(shield
	DOCS_DIR resources/docs
	PYTHON_DIR python)

SET(LIB_${PROJECT_NAME}_SOURCEFILES private/shield/*.cxx)
SET(LIB_${PROJECT_NAME}_PROJECTS serialization icetray dataclasses phys-services recclasses)
SET(LIB_${PROJECT_NAME}_TOOLS boost python)

i3_add_library(${PROJECT_NAME} ${LIB_${PROJECT_NAME}_SOURCEFILES}
	USE_TOOLS ${LIB_${PROJECT_NAME}_TOOLS}
	USE_PROJECTS ${LIB_${PROJECT_NAME}_PROJECTS}
)

LIST(APPEND LIB_${PROJECT_NAME}_TEST_SOURCEFILES
	private/test/main.cxx
	private/test/ShieldTests.cxx
)

i3_test_executable(test
	${LIB_${PROJECT_NAME}_TEST_SOURCEFILES}
	USE_PROJECTS dataclasses shield recclasses
)
