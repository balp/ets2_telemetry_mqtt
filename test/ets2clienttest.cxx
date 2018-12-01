
#include <memory>
#include "catch.hpp"
#include "telematic.hpp"
#include "gmock/gmock.h"

struct ITelemetryInit
{
    virtual ~ITelemetryInit() {}

    virtual scs_result_t register_for_event(
        const scs_string_t name,
        const scs_u32_t index,
        const scs_value_type_t type,
        const scs_u32_t flags,
        const scs_telemetry_channel_callback_t callback,
        const scs_context_t context) = 0;
};

struct TelemetryInitMock : public ITelemetryInit
{
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

scs_result_t
test(
    const scs_string_t name,
    const scs_u32_t index,
    const scs_value_type_t type,
    const scs_u32_t flags,
    const scs_telemetry_channel_callback_t callback,
    const scs_context_t context)
{
    if (telemetryInitMock)
    {
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

TEST_CASE("TelematicUint32 life cycle", "[TelematicUint32]")
{
    TelemetryInitMock _telemetryInitMock;
    telemetryInitMock = &_telemetryInitMock;
    std::shared_ptr<ITelematic> telematic = std::make_shared<TelematicUint32>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;

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