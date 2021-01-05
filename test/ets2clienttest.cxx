
#include <memory>
#include "catch2/catch.hpp"
#include "telematic.hpp"
#include "gmock/gmock.h"


struct ITelemetryInit {
    virtual ~ITelemetryInit() {}

    virtual scs_result_t register_for_event(
            const scs_string_t name,
            const scs_u32_t index,
            const scs_value_type_t type,
            const scs_u32_t flags,
            const scs_telemetry_channel_callback_t callback,
            const scs_context_t context) = 0;
};

struct TelemetryInitMock : public ITelemetryInit {
    TelemetryInitMock() {}

    virtual ~TelemetryInitMock() {}

    MOCK_METHOD6(register_for_event, scs_result_t(const scs_string_t name,
            const scs_u32_t index,
            const scs_value_type_t type,
            const scs_u32_t flags,
            const scs_telemetry_channel_callback_t callback,
            const scs_context_t context));
};

TelemetryInitMock *telemetryInitMock;

scs_result_t __stdcall
test(
        const scs_string_t name,
        const scs_u32_t index,
        const scs_value_type_t type,
        const scs_u32_t flags,
        const scs_telemetry_channel_callback_t callback,
        const scs_context_t context) {
    if (telemetryInitMock) {
        return telemetryInitMock->register_for_event(name,
                                                     index,
                                                     type,
                                                     flags,
                                                     callback,
                                                     context);
    }
    return SCS_RESULT_not_now;
}

using ::testing::_;
using ::testing::Return;

scs_result_t scs_telemetry_register_for_channel_test(const scs_string_t name,
                                                     const scs_u32_t index,
                                                     const scs_value_type_t type,
                                                     const scs_u32_t flags,
                                                     const scs_telemetry_channel_callback_t callback,
                                                     const scs_context_t context) {
    return SCS_RESULT_not_now;
};


TEST_CASE("TelematicUint32 life cycle", "[TelematicUint32]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicUint32>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = reinterpret_cast<scs_telemetry_register_for_channel_t>(test);

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_u32, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_u32;
        value.value_u32.value = 1234;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        REQUIRE(tmp["test"] == 1234);
    }
}

TEST_CASE("TelematicInt32 life cycle", "[TelematicInt32]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicInt32>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_s32, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_s32;
        value.value_u32.value = 12345;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        REQUIRE(tmp["test"] == 12345);
    }
}


TEST_CASE("TelematicFloat life cycle", "[TelematicFloat]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicFloat>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_float, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_float;
        value.value_float.value = 12345.0f;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        REQUIRE(tmp["test"] == 12345.0f);
    }
}

TEST_CASE("TelematicDPlacement life cycle", "[TelematicDPlacement]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicDPlacement>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_dplacement, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_dplacement;
        value.value_dplacement.position.x = 1.0;
        value.value_dplacement.position.y = 2.0;
        value.value_dplacement.position.z = 3.0;
        value.value_dplacement.orientation.heading = 3.0;
        value.value_dplacement.orientation.pitch = 4.0;
        value.value_dplacement.orientation.roll = 5.0;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        nlohmann::json expected =
                {
                        {"position",    {
                                                {"x",       1.0},
                                                {"y",     2.0},
                                                {"z",    3.0}
                                        }},
                        {"orientation", {
                                                {"heading", 3.0},
                                                {"pitch", 4.0},
                                                {"roll", 5.0}
                                        }}
                };
        REQUIRE(tmp["test"] == expected);
    }
}

TEST_CASE("TelematicFPlacement life cycle", "[TelematicFPlacement]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicFPlacement>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_fplacement, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_fplacement;
        value.value_fplacement.position.x = 1.0;
        value.value_fplacement.position.y = 2.0;
        value.value_fplacement.position.z = 3.0;
        value.value_fplacement.orientation.heading = 3.0;
        value.value_fplacement.orientation.pitch = 4.0;
        value.value_fplacement.orientation.roll = 5.0;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        nlohmann::json expected =
                {
                        {"position",    {
                                                {"x",       1.0},
                                                {"y",     2.0},
                                                {"z",    3.0}
                                        }},
                        {"orientation", {
                                                {"heading", 3.0},
                                                {"pitch", 4.0},
                                                {"roll", 5.0}
                                        }}
                };
        REQUIRE(tmp["test"] == expected);
    }
}


TEST_CASE("TelematicFVector life cycle", "[TelematicFVector]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicFVector>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_fvector, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_fvector;
        value.value_fvector.x = 1.0;
        value.value_fvector.y = 2.0;
        value.value_fvector.z = 3.0;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        nlohmann::json expected =
                {
                        {"x", 1.0},
                        {"y", 2.0},
                        {"z", 3.0}
                };
        REQUIRE(tmp["test"] == expected);
    }
}


TEST_CASE("TelematicBool life cycle", "[TelematicBool]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicBool>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

    SECTION("Register for right channel")
    {
        EXPECT_CALL(_telemetryInitMock,
                    register_for_event("test", _, SCS_VALUE_TYPE_bool, _, _, _))
                .WillOnce(Return(SCS_RESULT_ok));

        auto res = telematic->register_for_channel(&params);
        REQUIRE(res == SCS_RESULT_ok);
    }

    SECTION("Handle value callback")
    {
        scs_value_t value;
        value.type = SCS_VALUE_TYPE_bool;
        value.value_bool.value = 1;
        ITelematic::value_callback("test",
                                   0,
                                   &value,
                                   static_cast<scs_context_t>(telematic.get()));
        auto tmp = telematic->getJson();
        REQUIRE(tmp["test"] == true);
    }
}
