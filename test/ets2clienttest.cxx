
#include <memory>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "telematic.hpp"
#include "gmock/gmock.h"

scs_result_t test(
    const scs_string_t name,
    const scs_u32_t index,
    const scs_value_type_t type,
    const scs_u32_t flags,
    const scs_telemetry_channel_callback_t callback,
    const scs_context_t context)
{
}

TEST_CASE("TelematicUint32 Register")
{
    std::unique_ptr<ITelematic> telematic = std::make_unique<TelematicUint32>("test");
    scs_telemetry_init_params_v100_t params;
    params.register_for_channel = test;
    //telematic->register_for_channel(&params);

}