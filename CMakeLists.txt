cmake_minimum_required (VERSION 3.9.0)
project (ets2_mqtt_connector)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
enable_testing()
include(ExternalProject)

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/MDd>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Debug>>:-O0;-ggdb3>")
add_compile_options("$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-O0;-ggdb3;-fprofile-instr-generate;-fcoverage-mapping>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate;-fcoverage-mapping>")
add_link_options("$<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate;-fcoverage-mapping>")

ExternalProject_Add(catch2
        PREFIX ${CMAKE_BINARY_DIR}/catch2
        URL https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch2/catch.hpp
        DOWNLOAD_NO_EXTRACT 1
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)
include_directories(${CMAKE_BINARY_DIR}/catch2/src)


find_package(Mosquitto)
if(NOT ${MOSQITTO_FOUND})
    set(EXTERNAL_MOSQUITTO_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/mosquitto)
    ExternalProject_Add(mosquitto
        PREFIX ${EXTERNAL_MOSQUITTO_INSTALL_LOCATION}
        GIT_REPOSITORY https://github.com/eclipse/mosquitto.git
        GIT_TAG v1.4.15
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_MOSQUITTO_INSTALL_LOCATION}
        TIMEOUT 10
        LOG_DOWNLOAD ON
        )
    add_dependencies(mqttClient mosquitto)
endif()

ExternalProject_Add(scs_sdk
    URL http://download.eurotrucksimulator2.com/scs_sdk_1_11.zip
    PREFIX ${CMAKE_BINARY_DIR}/external/scs_sdk
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )
include_directories(
    ${CMAKE_BINARY_DIR}/external/scs_sdk/src/scs_sdk/include/
)

ExternalProject_Add(json
    PREFIX ${CMAKE_BINARY_DIR}/external/json
    URL https://github.com/nlohmann/json/releases/download/v3.4.0/json.hpp
    DOWNLOAD_NO_EXTRACT 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
include_directories(${CMAKE_BINARY_DIR}/external/json/src)

add_library(ets2telemery MODULE
	src/mqttClient.hpp
	src/mqttClient.cxx
    src/telemetry_state.hpp
    src/telemetry_state.cpp
	src/ets2client.cxx
	src/scslog.hpp
    src/simulation_timestamp.hpp
    src/simulation_timestamp.cpp
)

add_dependencies(ets2telemery scs_sdk json)
target_link_libraries(ets2telemery 
    PUBLIC ${MOSQUITTOPP_LIBRARY})



add_executable(ets2clienttest
    src/mqttClient.hpp
    src/mqttClient.cxx
    src/telemetry_state.hpp
    src/telemetry_state.cpp
    src/ets2client.cxx
    src/simulation_timestamp.hpp
    src/simulation_timestamp.cpp

    test/ets2clienttest.cxx
    test/gmock_catch_interceptor.hpp
    test/main.cpp
)
target_link_libraries(ets2clienttest 
    PUBLIC
     ${MOSQUITTOPP_LIBRARY}
     gmock
)
set_property(TARGET ets2clienttest 
    PROPERTY 
    CXX_STANDARD 14
)
target_include_directories(ets2clienttest
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_dependencies(ets2clienttest catch2 json scs_sdk)
add_test(NAME ets2client
      COMMAND ets2clienttest
)

find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)

doxygen_add_docs(
    doxygen
    ${PROJECT_SOURCE_DIR} ${EXTERNAL_SCS_SDK_INSTALL_LOCATION}/src/scs_sdk
    "Build Documentation"
)
add_dependencies(doxygen scs_sdk)
add_custom_target(doc ALL
    DEPENDS doxygen
)
