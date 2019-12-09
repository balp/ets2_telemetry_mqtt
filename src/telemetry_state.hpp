//
// Created by balp on 2019-12-09.
//

#ifndef ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP
#define ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP

#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include "scslog.hpp"
#include "telematic.hpp"

#include "amtrucks/scssdk_telemetry_ats.h"
#include "amtrucks/scssdk_ats.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "eurotrucks2/scssdk_eut2.h"


static const size_t kMaxHShifterSlots = 10;
static const size_t kMaxWheelCount = 8;


struct truck_telemetry_state_t {
    std::vector<std::shared_ptr<ITelematic>> _truck;

    truck_telemetry_state_t() :
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
                           // Type: indexed bool SCS_TELEMETRY_TRUCK_CHANNEL_hshifter_selector
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
                           // unsupported
                           // std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_adblue_average_consumption),
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
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_susp_deflection
                           // indexed bool SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground
                           // indexed u32 SCS_TELEMETRY_TRUCK_CHANNEL_wheel_substance
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_velocity
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_velocity
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_rotation
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift
                           // indexed float SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift_offset
                   }) {}
};

struct trailer_telemetry_state_t {
    std::vector<std::shared_ptr<ITelematic>> _trailer;

    trailer_telemetry_state_t() :
            _trailer({
                             std::make_shared<TelematicBool>(SCS_TELEMETRY_TRAILER_CHANNEL_connected),
                             // Movement.
                             std::make_shared<TelematicDPlacement>(SCS_TELEMETRY_TRAILER_CHANNEL_world_placement),
                             std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_linear_velocity),
                             std::make_shared<TelematicFVector>(SCS_TELEMETRY_TRAILER_CHANNEL_local_angular_velocity),
                             std::make_shared<TelematicFVector>(
                                     SCS_TELEMETRY_TRAILER_CHANNEL_local_linear_acceleration),
                             std::make_shared<TelematicFVector>(
                                     SCS_TELEMETRY_TRAILER_CHANNEL_local_angular_acceleration),
                             // Wear info
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wear_chassis),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wear_wheels),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_cargo_damage),
                             // Wheels
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_susp_deflection
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_on_ground
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_substance
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_velocity
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_steering
                             // SCS_TELEMETRY_TRAILER_CHANNEL_wheel_rotation
                     }) {}
};

class TelemetryState {
private:
    scs_telemetry_register_for_channel_t register_for_channel;
    scs_telemetry_unregister_from_channel_t unregister_from_channel;

    Logger& _logger;

public:
    scs_timestamp_t timestamp;
    scs_timestamp_t raw_rendering_timestamp;
    scs_timestamp_t raw_simulation_timestamp;
    scs_timestamp_t raw_paused_simulation_timestamp;
    std::vector<std::shared_ptr<ITelematic>> _common;
    truck_telemetry_state_t _truck_state;
    trailer_telemetry_state_t _trailer_state;
    std::vector<std::shared_ptr<ITelematic>> _wheel_on_ground; // SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground
    TelemetryState(scs_telemetry_register_for_channel_t register_for_channel,
                   scs_telemetry_unregister_from_channel_t unregister_from_channel,
            Logger& logger) :
            register_for_channel(register_for_channel),
            unregister_from_channel(unregister_from_channel),
            _logger(logger),
            timestamp(0),
            raw_rendering_timestamp(0),
            raw_simulation_timestamp(0),
            raw_paused_simulation_timestamp(0),
            _common({std::make_shared<TelematicUint32>(SCS_TELEMETRY_CHANNEL_game_time),
                     std::make_shared<TelematicFloat>(SCS_TELEMETRY_CHANNEL_local_scale),
                     std::make_shared<TelematicInt32>(SCS_TELEMETRY_CHANNEL_next_rest_stop)}),
            _truck_state(),
            _trailer_state(),
            _wheel_on_ground(
                    {std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                    std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground)}
            ) {
    }


    void update_config(const scs_telemetry_configuration_t *const pConfiguration);
};

#endif //ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP
