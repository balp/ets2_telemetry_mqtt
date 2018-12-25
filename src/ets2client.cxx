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
#include "scslog.hpp"

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
    std::vector<std::shared_ptr<ITelematic>> _truck;
    std::vector<std::shared_ptr<ITelematic>> _trailer;
    telemetry_state_t() : timestamp(0),
                          raw_rendering_timestamp(0),
                          raw_simulation_timestamp(0),
                          raw_paused_simulation_timestamp(0),
                          _common({std::make_shared<TelematicUint32>(SCS_TELEMETRY_CHANNEL_game_time),
                                   std::make_shared<TelematicFloat>(SCS_TELEMETRY_CHANNEL_local_scale),
                                   std::make_shared<TelematicInt32>(SCS_TELEMETRY_CHANNEL_next_rest_stop)}),

                          _truck({
                              // Movement.
                              std::make_shared<TelematicDPlacement>(SCS_TELEMETRY_TRUCK_CHANNEL_world_placement),
                              std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_velocity),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_velocity),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_local_linear_acceleration),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_local_angular_acceleration),
                                  std::make_shared<TelematicFPlacement>(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_offset),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_angular_velocity),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRUCK_CHANNEL_cabin_angular_acceleration),
                                  std::make_shared<TelematicFPlacement>(SCS_TELEMETRY_TRUCK_CHANNEL_head_offset),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_speed),
                                  // Powertrain related
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm),
                                  std::make_shared<TelematicInt32>(SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear),
                                  std::make_shared<TelematicInt32>(SCS_TELEMETRY_TRUCK_CHANNEL_displayed_gear),
                                  // Driving
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_input_steering),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_input_throttle),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_input_brake),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_input_clutch),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_effective_steering),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_effective_throttle),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_effective_brake),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_effective_clutch),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_cruise_control),
                                  // Gearbox related
                                  std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_slot),
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_selector
                                  // Brakes.
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_parking_brake),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_motor_brake),
                                  std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRUCK_CHANNEL_retarder_level),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_brake_air_pressure),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_brake_air_pressure_warning),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_brake_air_pressure_emergency),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_brake_temperature),
                                  // Various "consumables"
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_fuel),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_fuel_warning),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_fuel_average_consumption),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_fuel_range),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_adblue),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_adblue_warning),
                                  // unsupported std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_adblue_average_consumption),
                                  // Oil
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_oil_pressure),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_oil_pressure_warning),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_oil_temperature),
                                  // Temperature in various systems.
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_water_temperature),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_water_temperature_warning),
                                  // Battery
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_battery_voltage),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_battery_voltage_warning),
                                  // Enabled state of various elements
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_electric_enabled),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_engine_enabled),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_lblinker),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_rblinker),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_lblinker),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_rblinker),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_parking),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_low_beam),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_high_beam),
                                  std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRUCK_CHANNEL_light_aux_front),
                                  std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRUCK_CHANNEL_light_aux_roof),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_beacon),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_brake),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_light_reverse),
                                  std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wipers),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_dashboard_backlight),
                                  // Wear info
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wear_engine),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wear_transmission),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wear_cabin),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wear_chassis),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wear_wheels),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_odometer),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_navigation_distance),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_navigation_time),
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_navigation_speed_limit),
                                  // Wheels
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_susp_deflection
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_substance
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_velocity
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_velocity
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_rotation
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift
                                  // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift_offset
                                  }),
                            _trailer({
                                std::make_shared<TelematicBool>(SCS_TELEMETRY_TRAILER_CHANNEL_connected),
                                // Movement.
                                std::make_shared<TelematicDPlacement>(SCS_TELEMETRY_TRAILER_CHANNEL_world_placement),
                                std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_linear_velocity),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_angular_velocity),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_linear_acceleration),
                                  std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_angular_acceleration),
                                  // Waer info
                                  std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wear_chassis),
                                    // Wheels
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_susp_deflection
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_on_ground
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_substance
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_velocity
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_steering
                                  // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_rotation
                            })
    {
    }
#if 0
    struct truck
    {
        //uint32_t hshifter_slot;
        //TelematicUint32 hshifter_slot;
        //truck() : hshifter_slot(SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_slot) {}
        // bool hshifter_selector[max_hshifter_slots];
    } truck;
#endif

} telemetry;

static Logger logger;
static Ets2MqttWrapper *mqttHdl = NULL;
scs_log_t game_log = NULL;
static bool output_paused = true;

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
    j["common"] = nlohmann::json::object();
    for (auto channel : telemetry._common)
    {
        j["common"].update(channel->getJson());
    }
    j["truck"] = nlohmann::json::object();
    for (auto channel : telemetry._truck)
    {
        j["truck"].update(channel->getJson());
    }
    j["trailer"] = nlohmann::json::object();
    for (auto channel : telemetry._trailer)
    {
        j["trailer"].update(channel->getJson());
    }
    
    std::string json_string = j.dump();
    mqttHdl->publish(NULL, "ets2/data", strlen(json_string.c_str()), json_string.c_str());
}

SCSAPI_VOID telemetry_pause(const scs_event_t event,
                            const void *const UNUSED(event_info),
                            const scs_context_t UNUSED(context))
{
    output_paused = (event == SCS_TELEMETRY_EVENT_paused);
    nlohmann::json j;
    if (event == SCS_TELEMETRY_EVENT_paused)
    {
        j["paused"] = true;
    }
    else if (event == SCS_TELEMETRY_EVENT_started)
    {
        j["paused"] = false;
    }
    else
    {
    }
    std::string json_string = j.dump();
    mqttHdl->publish(NULL, "ets2/info", strlen(json_string.c_str()), json_string.c_str(), 0, true);
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
    mqttHdl->publish(NULL, topic.c_str(), strlen(json_string.c_str()), json_string.c_str(), 0, true);
}

void publish_game_info(const scs_telemetry_init_params_v100_t *const params)
{
    if (mqttHdl == NULL)
    {
        return;
    }
    nlohmann::json j;
    j["name"] = params->common.game_name;
    j["id"] = params->common.game_id;
    j["raw_version"] = params->common.game_version;
    j["version"] = {{"major", SCS_GET_MAJOR_VERSION(params->common.game_version)},
        {"minor", SCS_GET_MINOR_VERSION(params->common.game_version)}};


    std::string json_string = j.dump();
    mqttHdl->publish(NULL, "ets2/game", strlen(json_string.c_str()), json_string.c_str(), 0, true);
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version,
                                 const scs_telemetry_init_params_t *const params)
{
    if (version != SCS_TELEMETRY_VERSION_1_00)
    {
        return SCS_RESULT_unsupported;
    }
    const scs_telemetry_init_params_v100_t *const version_params = static_cast<const scs_telemetry_init_params_v100_t *>(params);
    game_log = version_params->common.log;
    logger.setGameLog(game_log);
    logger.message("Initializing telemetry mqtt gateway");
    try
    {
        mqttHdl = new Ets2MqttWrapper("ETS2-MQTT", "127.0.0.1", 1883, logger);
    }
    catch (...)
    {
        mqttHdl = NULL;
    }
    if (mqttHdl == NULL)
    {
        logger.error("No MQTT Connection");
        return SCS_RESULT_generic_error;
    }
    logger.message("MQTT Connected");
    publish_game_info(version_params);
    const bool events_registered =
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_frame_end, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok);
    if (!events_registered)
    {
        logger.error("Unable to register event callbacks");
        return SCS_RESULT_generic_error;
    }
    version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration,
                                       telemetry_configuration, NULL);
    for (auto channel : telemetry._common)
    {
        channel->register_for_channel(version_params);
    }

    for (auto channel : telemetry._truck)
    {
        channel->register_for_channel(version_params);
    }
    for (auto channel : telemetry._trailer)
    {
        channel->register_for_channel(version_params);
    }

    last_timestamp = static_cast<scs_timestamp_t>(-1);
    output_paused = true;
    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void)
{
    logger.message("Shutdown telemetry mqtt gateway");
    game_log = NULL;
    logger.setGameLog(NULL);
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
