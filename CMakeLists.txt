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

IF(${TARGET_NAME} STREQUAL "etiss")
	IF(NOT ETISS_PREFIX)
		include(FetchContent)
        set(FETCHCONTENT_QUIET FALSE)
		FetchContent_Declare(etiss_srcs
			GIT_REPOSITORY https://github.com/JoGei/etiss.git
			GIT_TAG vpu
			GIT_PROGRESS TRUE
		)
		if(NOT etiss_srcs_POPULATED)
			FetchContent_Populate(etiss_srcs)
			FetchContent_GetProperties(etiss_srcs)
			set(ETISS_LOCATION ${etiss_srcs_SOURCE_DIR})
		endif()
	ELSE()
		set(ETISS_LOCATION ${ETISS_PREFIX})
	ENDIF()
	
	SET(TARGET_ARCH "RV64GCV" CACHE STRING "No Target specified. Assuming softvector for etiss ArchImpl RV64GCV.")
	set(PLACE_HOLD_VCPUARCH ${TARGET_ARCH})	
	CONFIGURE_FILE(src/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/softvector.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/src/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/softvector.cpp @ONLY)
	SET(TARGET_SRCS
		${CMAKE_CURRENT_BINARY_DIR}/src/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/softvector.cpp
	)
	SET(TARGET_INCLUDES
		${ETISS_LOCATION}/include
		${ETISS_LOCATION}/include_c
		${ETISS_LOCATION}/ArchImpl/${TARGET_ARCH}
	)
ELSE()	
	SET(TARGET_SRCS
		src/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/softvector.cpp
	)   
ENDIF()

ADD_LIBRARY(${PROJECT_NAME} 
	src/vpu/softvector-element.cpp
	src/vpu/softvector-vector.cpp
	src/vpu/softvector-register.cpp
	src/target/${TARGET_PLATFORM_NAME}/base/base.cpp
	src/target/${TARGET_PLATFORM_NAME}/lsu/lsu.cpp
	src/target/${TARGET_PLATFORM_NAME}/arithmetic/integer.cpp
	${TARGET_SRCS}
)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE
	include/target/${TARGET_PLATFORM_NAME}
	include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}
	${TARGET_INCLUDES}
	include/
)

########### INSTALLATION ############

install(FILES ${PROJECT_SOURCE_DIR}/include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/${PROJECT_NAME}.h DESTINATION ${TARGET_PLATFORM_NAME}_${TARGET_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PLATFORM_NAME}_${TARGET_NAME})

########### DOCU ####################

option(SOFTVECTOR_BUILD_DOC "Build doc target for SoftVector project" ON)
set(ETISS_DOX_LOCATIONS ${ETISS_DOX_LOCATIONS} ${PROJECT_SOURCE_DIR}/include)
set(ETISS_DOX_LOCATIONS ${ETISS_DOX_LOCATIONS} ${PROJECT_SOURCE_DIR}/src)
if(SOFTVECTOR_BUILD_DOC)
FIND_PACKAGE(Doxygen)
if(DOXYGEN_FOUND)
	string(REPLACE ";" " " P_ETISS_DOX_LOCATIONS "${ETISS_DOX_LOCATIONS}")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} doc/doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
else()
    message(WARNING "Doxygen could not be found. Build target 'doc' (e.g. make doc) will not be available")
endif()
endif()
########### TESTING #################

ENABLE_TESTING()
SET(TEST_BUILD OFF CACHE STRING "Include Tests in build")
IF(CMAKE_TESTING_ENABLED AND TEST_BUILD)
	FIND_PACKAGE(GTEST)
	IF(NOT GTEST_FOUND)
        include(FetchContent)
        set(FETCHCONTENT_QUIET FALSE)
        FetchContent_Declare(gtest_srcs
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_PROGRESS TRUE
        )
        if(NOT gtest_srcs_POPULATED)
            FetchContent_Populate(gtest_srcs)
            FetchContent_GetProperties(gtest_srcs)
            set(GTEST_LOCATION ${gtest_srcs_SOURCE_DIR})
            set(INSTALL_GTEST OFF CACHE PATH "Do not install GTEST - build gtest locally.")
            ADD_SUBDIRECTORY(${GTEST_LOCATION})
        endif()
    ENDIF() 
	ADD_EXECUTABLE(${PROJECT_NAME}-test
		test/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}/unit_test.cpp
	)
	TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE 
        include/target/${TARGET_PLATFORM_NAME}
        include/target/${TARGET_PLATFORM_NAME}/${TARGET_NAME}
    )
	if(NOT GTEST_FOUND)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE 
            ${GTEST_LOCATION}/
            ${GTEST_LOCATION}/googletest/include
        )
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test PUBLIC gtest )
    else()
		TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PRIVATE 
            ${GTEST_INCLUDE_DIR}/
        )
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test GTest::GTest)
    endif()
    
	TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test PUBLIC ${PROJECT_NAME} )

	ADD_TEST(NAME unit_test 
		COMMAND ${PROJECT_NAME}-test --golden "${CMAKE_CURRENT_SOURCE_DIR}/test/target/${TARGET_PLATFORM_NAME}/golden/" --log "${CMAKE_BINARY_DIR}/log/"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ENDIF()
