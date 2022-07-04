CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(softvector)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(NOT TARGET_NAME)
	SET(TARGET_NAME soft)
ENDIF()

STRING(TOLOWER ${TARGET_NAME} TARGET_NAME )
SET(TARGET_PLATFORM_NAME riscv CACHE STRING "No Target Architecture specified. Assuming softvector for riscv (RVV).")

########### softvector lib ############

IF(WIN32)
	SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
ENDIF()

ADD_LIBRARY(${PROJECT_NAME} SHARED
	src/target/${TARGET_PLATFORM_NAME}/softvector.cpp
	src/vpu/softvector-element.cpp
	src/vpu/softvector-vector.cpp
	src/vpu/softvector-register.cpp
	src/target/${TARGET_PLATFORM_NAME}/base/base.cpp
	src/target/${TARGET_PLATFORM_NAME}/lsu/lsu.cpp
	src/target/${TARGET_PLATFORM_NAME}/arithmetic/integer.cpp
	#src/target/${TARGET_PLATFORM_NAME}/arithmetic/fixedpoint.cpp
	#src/target/${TARGET_PLATFORM_NAME}/arithmetic/floatingpoint.cpp
	#src/target/${TARGET_PLATFORM_NAME}/misc/mask.cpp
	src/target/${TARGET_PLATFORM_NAME}/misc/permutation.cpp
	#src/target/${TARGET_PLATFORM_NAME}/misc/reduction.cpp
	${TARGET_SRCS}
)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
	include/target/${TARGET_PLATFORM_NAME}
	include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}
	${TARGET_INCLUDES}
	include/
)

########### INSTALLATION ############

#INSTALL(FILES ${PROJECT_SOURCE_DIR}/include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/${PROJECT_NAME}.h DESTINATION ${TARGET_PLATFORM_NAME}_${TARGET_NAME})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PLATFORM_NAME})
INSTALL(FILES ${PROJECT_SOURCE_DIR}/include/target/${TARGET_PLATFORM_NAME}/${PROJECT_NAME}.h DESTINATION ${TARGET_PLATFORM_NAME})

########### DOCU ####################

OPTION(SOFTVECTOR_BUILD_DOC "Build doc target for SoftVector project" ON)
SET(DOX_LOCATIONS ${DOX_LOCATIONS} ${PROJECT_SOURCE_DIR}/include)
SET(DOX_LOCATIONS ${DOX_LOCATIONS} ${PROJECT_SOURCE_DIR}/src)
IF(SOFTVECTOR_BUILD_DOC)
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
	STRING(REPLACE ";" " " P_DOX_LOCATIONS "${DOX_LOCATIONS}")
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/doxyfile @ONLY)
	ADD_CUSTOM_TARGET(doc
		${DOXYGEN_EXECUTABLE} doc/doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
ELSE()
    MESSAGE(WARNING "Doxygen could not be found. Build target 'doc' (e.g. make doc) will not be available")
ENDIF()
ENDIF()
########### TESTING #################

ENABLE_TESTING()
SET(TEST_BUILD OFF CACHE STRING "Include Tests in build")
IF(CMAKE_TESTING_ENABLED AND TEST_BUILD)
	FIND_PACKAGE(GTEST QUIET)
	IF(NOT GTEST_FOUND)
		INCLUDE(FetchContent)
		SET(FETCHCONTENT_QUIET FALSE)
		FetchContent_Declare(gtest_srcs
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_PROGRESS TRUE
			GIT_TAG main
		)
		IF(NOT gtest_srcs_POPULATED)
			FetchContent_Populate(gtest_srcs)
			FetchContent_GetProperties(gtest_srcs)
			SET(GTEST_LOCATION ${gtest_srcs_SOURCE_DIR})
			SET(INSTALL_GTEST OFF CACHE PATH "Do not install GTEST - build gtest locally.")
			ADD_SUBDIRECTORY(${GTEST_LOCATION} gtest-build)
		ENDIF()
	ENDIF()
	ADD_EXECUTABLE(${PROJECT_NAME}-test
		test/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/unit_test.cpp
	)
	TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE
		include/target/${TARGET_PLATFORM_NAME}
		include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}
	)
	IF(NOT GTEST_FOUND)
		TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE
			${GTEST_LOCATION}/
			${GTEST_LOCATION}/googletest/include
		)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test PUBLIC gtest )
	ELSE()
		TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE
			${GTEST_INCLUDE_DIR}/
		)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test GTest::GTest)
	ENDIF()

	TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test PUBLIC ${PROJECT_NAME} )

	ADD_TEST(NAME unit_test
		COMMAND ${PROJECT_NAME}-test --golden "${CMAKE_CURRENT_SOURCE_DIR}/test/target/${TARGET_PLATFORM_NAME}/golden/" --log "${CMAKE_BINARY_DIR}/log/"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ENDIF()
