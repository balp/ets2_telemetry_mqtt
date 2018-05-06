cmake_minimum_required (VERSION 3.9.0)
project (ets2_mqtt_connector)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
enable_testing()
include(ExternalProject)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no official C++11 support.")
endif()


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

set(EXTERNAL_SCS_SDK_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/scs_sdk)
ExternalProject_Add(scs_sdk
    URL http://download.eurotrucksimulator2.com/scs_sdk_1_9.zip
    PREFIX ${EXTERNAL_SCS_SDK_INSTALL_LOCATION}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )
include_directories(
    ${EXTERNAL_SCS_SDK_INSTALL_LOCATION}/src/scs_sdk/include/
)

add_library(ets2telemery MODULE src/mqttClient.cxx)
add_dependencies(ets2telemery scs_sdk)
target_link_libraries(ets2telemery ${MOSQUITTOPP_LIBRARY})