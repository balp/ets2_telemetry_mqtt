cmake_minimum_required(VERSION 3.14)
project(ets2_mqtt_connector)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
enable_testing()
include(FetchContent)
include(ExternalProject)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.10.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.1)
FetchContent_MakeAvailable(Catch2)

find_package(Mosquitto)
if (NOT ${MOSQUITTO_FOUND})
    option(WITH_TLS "Include SSL/TLS support?" OFF)
    option(WITH_TLS_PSK "Include TLS-PSK support (requires WITH_TLS)?" OFF)
    option(WITH_EC "Include Elliptic Curve support (requires WITH_TLS)?" OFF)
    option(WITH_THREADING "Include client library threading support?" OFF)

    FetchContent_Declare(
            mosquitto
            GIT_REPOSITORY https://github.com/eclipse/mosquitto.git
            GIT_TAG v2.0.4
    )
    FetchContent_MakeAvailable(mosquitto)
endif ()

FetchContent_Declare(scs_sdk_dl
        URL http://download.eurotrucksimulator2.com/scs_sdk_1_11.zip
        )
FetchContent_GetProperties(scs_sdk_dl)
message(STATUS "1 scs_sdk_dl_POPULATED: ${scs_sdk_dl_POPULATED} scs_sdk_dl_SOURCE_DIR: ${scs_sdk_dl_SOURCE_DIR} scs_sdk_dl_BINARY_DIR: ${scs_sdk_dl_BINARY_DIR}  ")

if (NOT scs_sdk_dl_POPULATED)
    FetchContent_Populate(scs_sdk_dl)
endif ()
message(STATUS "2 scs_sdk_dl_POPULATED: ${scs_sdk_dl_POPULATED} scs_sdk_dl_SOURCE_DIR: ${scs_sdk_dl_SOURCE_DIR} scs_sdk_dl_BINARY_DIR: ${scs_sdk_dl_BINARY_DIR}  ")

add_library(scs_sdk INTERFACE)
target_include_directories(scs_sdk
        INTERFACE "${scs_sdk_dl_SOURCE_DIR}/include")

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.9.1)
if (NOT json_POPULATED)
    FetchContent_Populate(json)
endif ()
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json
        INTERFACE "${json_SOURCE_DIR}/single_include")

add_library(ets2telemery MODULE
        src/ets2telemery.def
        src/mqttClient.hpp
        src/mqttClient.cxx
        src/telemetry_state.hpp
        src/telemetry_state.cpp
        src/ets2client.cxx
        src/scslog.hpp
        src/simulation_timestamp.hpp
        src/simulation_timestamp.cpp
        )


target_link_libraries(ets2telemery
        PUBLIC mosquitto
        PUBLIC mosquittopp
        PRIVATE scs_sdk
        PRIVATE nlohmann_json
        )

target_include_directories(ets2telemery
        PRIVATE ${mosquitto_SOURCE_DIR}/lib
        PRIVATE ${mosquitto_SOURCE_DIR}/lib/cpp
        PRIVATE ${mosquitto_SOURCE_DIR}/include
        )

add_library(hello_ets2 MODULE
        src/hello_ets2.cpp
        src/hello_ets2.def
        )
target_link_libraries(hello_ets2
        PRIVATE scs_sdk)

add_library(telemetry MODULE
        scs_examples/telemetry.cpp
        scs_examples/telemetry.def
        )
target_link_libraries(telemetry
        PRIVATE scs_sdk)

if (NOT WIN32)
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
            PUBLIC mosquitto
            PUBLIC mosquittopp
            PUBLIC gmock
            PUBLIC Catch2::Catch2
            PRIVATE scs_sdk
            PRIVATE nlohmann_json
            )
    set_property(TARGET ets2clienttest
            PROPERTY
            CXX_STANDARD 14
            )
    target_include_directories(ets2clienttest
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
            PRIVATE ${mosquitto_SOURCE_DIR}/lib
            PRIVATE ${mosquitto_SOURCE_DIR}/lib/cpp
            PRIVATE ${mosquitto_SOURCE_DIR}/include
            )
    add_test(NAME ets2client
            COMMAND ets2clienttest --reporter xml --out ets2clienttest.xml
            )
endif ()

#find_package(Doxygen
#             REQUIRED dot
#             OPTIONAL_COMPONENTS mscgen dia)
#
#doxygen_add_docs(
#    doxygen
#    ${PROJECT_SOURCE_DIR} ${EXTERNAL_SCS_SDK_INSTALL_LOCATION}/src/scs_sdk
#    "Build Documentation"
#)
#add_dependencies(doxygen scs_sdk)
#add_custom_target(doc ALL
#    DEPENDS doxygen
#)
