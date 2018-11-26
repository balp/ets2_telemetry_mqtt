#ifdef _WIN32
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <string.h>
#include <assert.h>

#include "json.hpp"

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"

#include "mqttClient.hpp"
#include "telematic.hpp"

#define UNUSED(x)

scs_timestamp_t last_timestamp = static_cast<scs_timestamp_t>(-1);

static const size_t max_hshifter_slots = 10;
static const size_t max_wheel_count = 8;

struct telemetry_state_t
{
    scs_timestamp_t timestamp;
    scs_timestamp_t raw_rendering_timestamp;
    scs_timestamp_t raw_simulation_timestamp;
    scs_timestamp_t raw_paused_simulation_timestamp;
    std::vector<std::shared_ptr<ITelematic>> _common;

    telemetry_state_t() : timestamp(0),
                          raw_rendering_timestamp(0),
                          raw_simulation_timestamp(0),
                          raw_paused_simulation_timestamp(0),
                          _common({std::make_shared<TelematicUint32>(SCS_TELEMETRY_CHANNEL_game_time),
                                   std::make_shared<TelematicFloat>(SCS_TELEMETRY_CHANNEL_local_scale),
                                   std::make_shared<TelematicInt32>(SCS_TELEMETRY_CHANNEL_next_rest_stop)})
    {
    }

    struct truck
    {
        scs_value_dplacement_t world_placement;
        scs_value_fvector_t local_linear_velocity;
        scs_value_fvector_t local_angular_velocity;
        scs_value_fvector_t local_linear_acceleration;
        scs_value_fvector_t local_angular_acceleration;
        scs_value_fplacement_t cabin_offset;
        scs_value_fvector_t cabin_angular_velocity;
        scs_value_fvector_t cabin_angular_acceleration;
        scs_value_fplacement_t head_offset;
        float speed;
        float engine_rpm;
        int32_t engine_gear;
        int32_t displayed_gear;
        float input_steering;
        float input_throttle;
        float input_brake;
        float input_clutch;
        float effective_steering;
        float effective_throttle;
        float effective_brake;
        float effective_clutch;
        float cruise_control;
        //uint32_t hshifter_slot;
        //TelematicUint32 hshifter_slot;
        //truck() : hshifter_slot(SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_slot) {}
        // bool hshifter_selector[max_hshifter_slots];
    } truck;

} telemetry;

Ets2MqttWrapper *mqttHdl = NULL;
scs_log_t game_log = NULL;
bool output_paused = true;

SCSAPI_VOID telemetry_frame_start(const scs_event_t UNUSED(event),
                                  const void *const event_info,
                                  const scs_context_t UNUSED(context))
{
    const struct scs_telemetry_frame_start_t *const info = static_cast<const scs_telemetry_frame_start_t *>(event_info);
    if (last_timestamp == static_cast<scs_timestamp_t>(-1))
    {
        last_timestamp = info->paused_simulation_time;
    }
    if (info->flags & SCS_TELEMETRY_FRAME_START_FLAG_timer_restart)
    {
        last_timestamp = 0;
    }
    telemetry.timestamp += (info->paused_simulation_time - last_timestamp);
    last_timestamp = info->paused_simulation_time;
    telemetry.raw_rendering_timestamp = info->render_time;
    telemetry.raw_simulation_timestamp = info->simulation_time;
    telemetry.raw_paused_simulation_timestamp = info->paused_simulation_time;
}

SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event),
                                const void *const UNUSED(event_info),
                                const scs_context_t UNUSED(context))
{
    if (output_paused)
    {
        return;
    }
    if (mqttHdl == NULL)
    {
        return;
    }
    nlohmann::json j;
    j["timestamp"] = telemetry.timestamp;
    j["raw_rendering_timestamp"] = telemetry.raw_rendering_timestamp;
    j["raw_simulation_timestamp"] = telemetry.raw_simulation_timestamp;
    j["raw_paused_simulation_timestamp"] = telemetry.raw_paused_simulation_timestamp;
    j["common"] = nlohmann::json::array();
    for (auto channel : telemetry._common)
    {
        j["common"].push_back(channel->getJson());
    }

    j["truck"]["world_placement"]["position"]["x"] = telemetry.truck.world_placement.position.x;
    j["truck"]["world_placement"]["position"]["y"] = telemetry.truck.world_placement.position.y;
    j["truck"]["world_placement"]["position"]["z"] = telemetry.truck.world_placement.position.z;
    j["truck"]["world_placement"]["orientation"]["heading"] = telemetry.truck.world_placement.orientation.heading;
    j["truck"]["world_placement"]["orientation"]["pitch"] = telemetry.truck.world_placement.orientation.pitch;
    j["truck"]["world_placement"]["orientation"]["roll"] = telemetry.truck.world_placement.orientation.roll;
    j["truck"]["local_linear_velocity"]["x"] = telemetry.truck.local_linear_velocity.x;
    j["truck"]["local_linear_velocity"]["y"] = telemetry.truck.local_linear_velocity.y;
    j["truck"]["local_linear_velocity"]["z"] = telemetry.truck.local_linear_velocity.z;
    j["truck"]["local_angular_velocity"]["x"] = telemetry.truck.local_angular_velocity.x;
    j["truck"]["local_angular_velocity"]["y"] = telemetry.truck.local_angular_velocity.y;
    j["truck"]["local_angular_velocity"]["z"] = telemetry.truck.local_angular_velocity.z;
    j["truck"]["local_linear_acceleration"]["x"] = telemetry.truck.local_linear_acceleration.x;
    j["truck"]["local_linear_acceleration"]["y"] = telemetry.truck.local_linear_acceleration.y;
    j["truck"]["local_linear_acceleration"]["z"] = telemetry.truck.local_linear_acceleration.z;
    j["truck"]["local_angular_acceleration"]["x"] = telemetry.truck.local_angular_acceleration.x;
    j["truck"]["local_angular_acceleration"]["y"] = telemetry.truck.local_angular_acceleration.y;
    j["truck"]["local_angular_acceleration"]["z"] = telemetry.truck.local_angular_acceleration.z;
    j["truck"]["cabin_offset"]["position"]["x"] = telemetry.truck.cabin_offset.position.x;
    j["truck"]["cabin_offset"]["position"]["y"] = telemetry.truck.cabin_offset.position.y;
    j["truck"]["cabin_offset"]["position"]["z"] = telemetry.truck.cabin_offset.position.z;
    j["truck"]["cabin_offset"]["orientation"]["heading"] = telemetry.truck.cabin_offset.orientation.heading;
    j["truck"]["cabin_offset"]["orientation"]["pitch"] = telemetry.truck.cabin_offset.orientation.pitch;
    j["truck"]["cabin_offset"]["orientation"]["roll"] = telemetry.truck.cabin_offset.orientation.roll;
    j["truck"]["speed"] = telemetry.truck.speed;
    j["truck"]["engine_rpm"] = telemetry.truck.engine_rpm;
    j["truck"]["engine_gear"] = telemetry.truck.engine_gear;
    j["truck"]["displayed_gear"] = telemetry.truck.displayed_gear;
    j["truck"]["input_steering"] = telemetry.truck.input_steering;
    j["truck"]["input_throttle"] = telemetry.truck.input_throttle;
    j["truck"]["input_brake"] = telemetry.truck.input_brake;
    j["truck"]["input_clutch"] = telemetry.truck.input_clutch;
    j["truck"]["effective_steering"] = telemetry.truck.effective_steering;
    j["truck"]["effective_throttle"] = telemetry.truck.effective_throttle;
    j["truck"]["effective_brake"] = telemetry.truck.effective_brake;
    j["truck"]["effective_clutch"] = telemetry.truck.effective_clutch;
    j["truck"]["cruise_control"] = telemetry.truck.cruise_control;
    //j["truck"]["hshifter_slot"] = telemetry.truck.hshifter_slot.get();

    std::string json_string = j.dump();
    mqttHdl->publish(NULL, "ets2/data", strlen(json_string.c_str()), json_string.c_str());
}

SCSAPI_VOID telemetry_pause(const scs_event_t event,
                            const void *const UNUSED(event_info),
                            const scs_context_t UNUSED(context))
{
    output_paused = (event == SCS_TELEMETRY_EVENT_paused);
    if (event == SCS_TELEMETRY_EVENT_paused)
    {
        auto msg = "telemetry_pause(): Paused";
        auto res = mqttHdl->publish(NULL, "ets2/info", strlen(msg), msg);
    }
    else if (event == SCS_TELEMETRY_EVENT_started)
    {
        auto msg = "telemetry_pause(): Started";
        auto res = mqttHdl->publish(NULL, "ets2/info", strlen(msg), msg);
    }
    else
    {
        auto msg = "telemetry_pause(): ???";
        auto res = mqttHdl->publish(NULL, "ets2/info", strlen(msg), msg);
    }
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event,
                                    const void *const event_info,
                                    const scs_context_t UNUSED(context))
{
    const struct scs_telemetry_configuration_t *const info = static_cast<const scs_telemetry_configuration_t *>(event_info);
    nlohmann::json j = nlohmann::json::object();
    for (const scs_named_value_t *current = info->attributes; current->name; ++current)
    {
        switch (current->value.type)
        {
        case SCS_VALUE_TYPE_INVALID:
            j[current->name] = "[invalid]";
            break;
        case SCS_VALUE_TYPE_bool:
            j[current->name] = current->value.value_bool.value ? true : false;
            break;
        case SCS_VALUE_TYPE_s32:
            j[current->name] = current->value.value_s32.value;
            break;
        case SCS_VALUE_TYPE_u32:
            j[current->name] = current->value.value_u32.value;
            break;
        case SCS_VALUE_TYPE_u64:
            j[current->name] = current->value.value_u64.value;
            break;
        case SCS_VALUE_TYPE_float:
            j[current->name] = current->value.value_float.value;
            break;
        case SCS_VALUE_TYPE_double:
            j[current->name] = current->value.value_double.value;
            break;
        case SCS_VALUE_TYPE_fvector:
            j[current->name]["x"] = current->value.value_fvector.x;
            j[current->name]["y"] = current->value.value_fvector.y;
            j[current->name]["z"] = current->value.value_fvector.z;
            break;
        case SCS_VALUE_TYPE_dvector:
            j[current->name]["x"] = current->value.value_dvector.x;
            j[current->name]["y"] = current->value.value_dvector.y;
            j[current->name]["z"] = current->value.value_dvector.z;
            break;
        case SCS_VALUE_TYPE_euler:
            j[current->name]["heading"] = current->value.value_euler.heading;
            j[current->name]["pitch"] = current->value.value_euler.pitch;
            j[current->name]["roll"] = current->value.value_euler.roll;
            break;
        case SCS_VALUE_TYPE_fplacement:
            j[current->name]["x"] = current->value.value_fplacement.position.x;
            j[current->name]["y"] = current->value.value_fplacement.position.y;
            j[current->name]["z"] = current->value.value_fplacement.position.z;
            j[current->name]["heading"] = current->value.value_fplacement.orientation.heading;
            j[current->name]["pitch"] = current->value.value_fplacement.orientation.pitch;
            j[current->name]["roll"] = current->value.value_fplacement.orientation.roll;
            break;
        case SCS_VALUE_TYPE_dplacement:
            j[current->name]["x"] = current->value.value_dplacement.position.x;
            j[current->name]["y"] = current->value.value_dplacement.position.y;
            j[current->name]["z"] = current->value.value_dplacement.position.z;
            j[current->name]["heading"] = current->value.value_dplacement.orientation.heading;
            j[current->name]["pitch"] = current->value.value_dplacement.orientation.pitch;
            j[current->name]["roll"] = current->value.value_dplacement.orientation.roll;
            break;
        case SCS_VALUE_TYPE_string:
            j[current->name] = current->value.value_string.value;
            break;
        default:
            break;
        }
    }
    std::string topic = std::string("ets2/info/config/") + info->id;
    std::string json_string = j.dump();
    mqttHdl->publish(NULL, topic.c_str(), strlen(json_string.c_str()), json_string.c_str());
}

#if 0
SCSAPI_VOID telemetry_store_euler(const scs_string_t name,
        const scs_u32_t index,
        const scs_value_t *const value,
        const scs_context_t context)
{
    assert(context);
    telemetry_state_t *const state = static_cast<telemetry_state_t *>(context);
    if (! value) {
        state->orientation_available = false;
        return;
    }
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_euler);
    state->orientation_available = true;
    state->heading = value->value_euler.heading * 360.0f;
    state->pitch = value->value_euler.pitch * 360.0f;
    state->roll = value->value_euler.roll * 360.0f;
}
#endif

SCSAPI_VOID telemetry_store_dplacement(const scs_string_t name,
                                       const scs_u32_t index,
                                       const scs_value_t *const value,
                                       const scs_context_t context)
{
    assert(context);
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_dplacement);
    scs_value_dplacement_t *const placement = static_cast<scs_value_dplacement_t *>(context);
    *placement = value->value_dplacement;
}

SCSAPI_VOID telemetry_store_float(const scs_string_t name,
                                  const scs_u32_t index,
                                  const scs_value_t *const value,
                                  const scs_context_t context)
{
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_float);
    assert(context);
    *static_cast<float *>(context) = value->value_float.value;
}

SCSAPI_VOID telemetry_store_s32(const scs_string_t name,
                                const scs_u32_t index,
                                const scs_value_t *const value,
                                const scs_context_t context)
{
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_s32);
    assert(context);
    *static_cast<int32_t *>(context) = value->value_s32.value;
}

SCSAPI_VOID telemetry_store_u32(const scs_string_t name,
                                const scs_u32_t index,
                                const scs_value_t *const value,
                                const scs_context_t context)
{
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_u32);
    assert(context);
    *static_cast<uint32_t *>(context) = value->value_u32.value;
}

SCSAPI_VOID telemetry_store_fplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{
    assert(context);
    assert(value);
    assert(value->type == SCS_VALUE_TYPE_fplacement);
    scs_value_fplacement_t *const placement = static_cast<scs_value_fplacement_t *>(context);
    *placement = value->value_fplacement;
}

/**
 * @brief Vector storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zero if the value is not available.
 */
SCSAPI_VOID telemetry_store_fvector(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{
    assert(context);
    scs_value_fvector_t *const storage = static_cast<scs_value_fvector_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_fvector);
        *storage = value->value_fvector;
    }
    else
    {
        storage->x = 0.0f;
        storage->y = 0.0f;
        storage->z = 0.0f;
    }
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version,
                                 const scs_telemetry_init_params_t *const params)
{
    if (version != SCS_TELEMETRY_VERSION_1_00)
    {
        return SCS_RESULT_unsupported;
    }
    const scs_telemetry_init_params_v100_t *const version_params = static_cast<const scs_telemetry_init_params_v100_t *>(params);
    try
    {
        mqttHdl = new Ets2MqttWrapper("ETS2-MQTT", "127.0.0.1", 1883);
    }
    catch (...)
    {
        mqttHdl = NULL;
    }
    if (mqttHdl == NULL)
    {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to initialize the log file");
        return SCS_RESULT_generic_error;
    }
    const bool events_registered =
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_frame_end, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok);
    if (!events_registered)
    {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to register event callbacks");
        return SCS_RESULT_generic_error;
    }
    version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration,
                                       telemetry_configuration, NULL);
#if 1
    for (auto channel : telemetry._common)
    {
        channel->register_for_channel(version_params);
    }
#endif

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_world_placement,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_dplacement,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_dplacement,
                                         &telemetry.truck.world_placement);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_velocity,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.local_linear_velocity);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_velocity,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.local_angular_velocity);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_acceleration,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.local_linear_acceleration);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_acceleration,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.local_angular_acceleration);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_offset,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fplacement,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fplacement,
                                         &telemetry.truck.cabin_offset);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_angular_velocity,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.cabin_angular_velocity);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_angular_acceleration,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fvector,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fvector,
                                         &telemetry.truck.cabin_angular_acceleration);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_head_offset,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_fplacement,
                                         SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                         telemetry_store_fplacement,
                                         &telemetry.truck.head_offset);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_speed,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.speed);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.engine_rpm);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_s32,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_s32,
                                         &telemetry.truck.engine_gear);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_displayed_gear,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_s32,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_s32,
                                         &telemetry.truck.displayed_gear);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_input_steering,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.input_steering);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_input_throttle,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.input_throttle);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_input_brake,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.input_brake);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_input_clutch,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.input_clutch);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_effective_steering,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.effective_steering);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_effective_throttle,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.effective_throttle);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_effective_brake,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.effective_brake);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_effective_clutch,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.effective_clutch);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control,
                                         SCS_U32_NIL,
                                         SCS_VALUE_TYPE_float,
                                         SCS_TELEMETRY_CHANNEL_FLAG_none,
                                         telemetry_store_float,
                                         &telemetry.truck.cruise_control);
#if 0
    telemetry.truck.hshifter_slot.register_for_channel(version_params);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_slot,
            SCS_U32_NIL,
            SCS_VALUE_TYPE_u32,
            SCS_TELEMETRY_CHANNEL_FLAG_no_value,
            telemetry_store_u32, 
            &telemetry.truck.hshifter_slot);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_selector,
            SCS_U32_NIL,
            SCS_VALUE_TYPE_u32,
            SCS_TELEMETRY_CHANNEL_FLAG_no_value,
            telemetry_store_u32, 
            &telemetry.common.hshifter_selector);
#endif

    game_log = version_params->common.log;
    game_log(SCS_LOG_TYPE_message, "Initializing telemetry mqtt gateway");
    //memset(&telemetry, 0, sizeof(telemetry));
    last_timestamp = static_cast<scs_timestamp_t>(-1);
    output_paused = true;
    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void)
{
    game_log = NULL;
}

#ifdef _WIN32
BOOL APIENTRY DllMain(
    HMODULE module,
    DWORD reason_for_call,
    LPVOID reseved)
{
    if (reason_for_call == DLL_PROCESS_DETACH)
    {
        finish_log();
    }
    return TRUE;
}
#endif

#ifdef __linux__
void __attribute__((destructor)) unload(void)
{
}
#endif
