//
// Created by balp on 2019-12-09.
//

#ifndef ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP
#define ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP

#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include <scssdk.h>
#include "amtrucks/scssdk_telemetry_ats.h"
#include "amtrucks/scssdk_ats.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "eurotrucks2/scssdk_eut2.h"

#include "scslog.hpp"
#include "telematic.hpp"




static const size_t kMaxHShifterSlots = 10;
static const size_t kMaxWheelCount = 14;


class TruckWheel {
private:
    int index;
    std::vector<std::shared_ptr<ITelematic>> channels;
public:
    explicit TruckWheel(int index) :
            index(index),
            channels({
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_susp_deflection),
                             std::make_shared<TelematicBool>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_on_ground),
                             std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_substance),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_velocity),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_steering),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_rotation),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRUCK_CHANNEL_wheel_lift_offset),
                     }) {}

    nlohmann::json getJson() {
        auto json = nlohmann::json::object();
        json["index"] = index;
        for (const auto& channel : channels) {
            json.update(channel->getJson());
        }
        return json;
    }

    scs_result_t register_for_channel(scs_telemetry_register_for_channel_t register_for_channel) {
        scs_result_t result = SCS_RESULT_ok;
        for (const auto &channel : channels) {
            auto tmp = channel->register_for_channel(register_for_channel, index);
            if (tmp != SCS_RESULT_ok) { result = tmp; }
        }
        return result;
    }

    virtual scs_result_t unregister_from_channel(scs_telemetry_unregister_from_channel_t unregister_from_channel) {
        scs_result_t result = SCS_RESULT_ok;
        for (const auto &channel : channels) {
            auto tmp = channel->unregister_from_channel(unregister_from_channel, index);
            if (tmp != SCS_RESULT_ok) { result = tmp; }
        }
        return result;
    }
};


class Truck {
public:
    std::vector<std::shared_ptr<ITelematic>> _truck;

    Truck() :
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

                   }) {}
};


class TrailerWheel {
private:
    int index;
    std::vector<std::shared_ptr<ITelematic>> channels;
public:
    explicit TrailerWheel(int index) :
            index(index),
            channels({
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_susp_deflection),
                             std::make_shared<TelematicBool>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_on_ground),
                             std::make_shared<TelematicUint32>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_substance),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_velocity),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_steering),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_rotation),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_lift),
                             std::make_shared<TelematicFloat>(SCS_TELEMETRY_TRAILER_CHANNEL_wheel_lift_offset),
                     }) {}

    nlohmann::json getJson() {
        auto json = nlohmann::json::object();
        json["index"] = index;
        for (const auto& channel : channels) {
            json.update(channel->getJson());
        }
        return json;
    }

    scs_result_t register_for_channel(scs_telemetry_register_for_channel_t register_for_channel) {
        scs_result_t result = SCS_RESULT_ok;
        for (const auto &channel : channels) {
            auto tmp = channel->register_for_channel(register_for_channel, index);
            if (tmp != SCS_RESULT_ok) { result = tmp; }
        }
        return result;
    }

    scs_result_t unregister_from_channel(scs_telemetry_unregister_from_channel_t unregister_from_channel) {
        scs_result_t result = SCS_RESULT_ok;
        for (const auto &channel : channels) {
            auto tmp = channel->unregister_from_channel(unregister_from_channel, index);
            if (tmp != SCS_RESULT_ok) { result = tmp; }
        }
        return result;
    }
};


class Trailer {
private:
    scs_telemetry_register_for_channel_t registerForChannel;
    scs_telemetry_unregister_from_channel_t unregisterFromChannel;

    size_t no_trailer_wheels;
    std::vector<std::shared_ptr<TrailerWheel>> trailer_wheels;
public:
    std::vector<std::shared_ptr<ITelematic>> _trailer;

    Trailer(scs_telemetry_register_for_channel_t register_for_channel,
            scs_telemetry_unregister_from_channel_t unregister_from_channel) :
            registerForChannel(register_for_channel),
            unregisterFromChannel(unregister_from_channel),
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
                     }),
            no_trailer_wheels(0),
            trailer_wheels({std::make_shared<TrailerWheel>(0),
                            std::make_shared<TrailerWheel>(1),
                            std::make_shared<TrailerWheel>(2),
                            std::make_shared<TrailerWheel>(3),
                            std::make_shared<TrailerWheel>(4),
                            std::make_shared<TrailerWheel>(5),
                            std::make_shared<TrailerWheel>(6),
                            std::make_shared<TrailerWheel>(7),
                            std::make_shared<TrailerWheel>(8),
                            std::make_shared<TrailerWheel>(9),
                            std::make_shared<TrailerWheel>(10),
                            std::make_shared<TrailerWheel>(11),
                            std::make_shared<TrailerWheel>(12),
                            std::make_shared<TrailerWheel>(13)}) {}

    void update_config(const scs_telemetry_configuration_t *pConfiguration);
    nlohmann::json getJson();
};

class TelemetryState {
private:
    scs_telemetry_register_for_channel_t registerForChannel;
    scs_telemetry_unregister_from_channel_t unregisterFromChannel;

    Logger &_logger;

    std::vector<std::shared_ptr<ITelematic>> _common;
    Truck _truck_state;
    Trailer _trailer_state;
    size_t no_truck_wheels;
    std::vector<std::shared_ptr<TruckWheel>> truck_wheels;


public:
    TelemetryState(scs_telemetry_register_for_channel_t register_for_channel,
                   scs_telemetry_unregister_from_channel_t unregister_from_channel,
                   Logger &logger) :
            registerForChannel(register_for_channel),
            unregisterFromChannel(unregister_from_channel),
            _logger(logger),
            _common({std::make_shared<TelematicUint32>(SCS_TELEMETRY_CHANNEL_game_time),
                     std::make_shared<TelematicFloat>(SCS_TELEMETRY_CHANNEL_local_scale),
                     std::make_shared<TelematicInt32>(SCS_TELEMETRY_CHANNEL_next_rest_stop)}),
            _truck_state(),
            _trailer_state(register_for_channel, unregister_from_channel),
            no_truck_wheels(0),
            truck_wheels({std::make_shared<TruckWheel>(0),
                          std::make_shared<TruckWheel>(1),
                          std::make_shared<TruckWheel>(2),
                          std::make_shared<TruckWheel>(3),
                          std::make_shared<TruckWheel>(4),
                          std::make_shared<TruckWheel>(5),
                          std::make_shared<TruckWheel>(6),
                          std::make_shared<TruckWheel>(7),
                          std::make_shared<TruckWheel>(8),
                          std::make_shared<TruckWheel>(9),
                          std::make_shared<TruckWheel>(10),
                          std::make_shared<TruckWheel>(11),
                          std::make_shared<TruckWheel>(12),
                          std::make_shared<TruckWheel>(13)}) {}


    void update_config(const scs_telemetry_configuration_t *pConfiguration);
    scs_result_t register_for_channel();
    scs_result_t unregister_from_channel();
    nlohmann::json getJson();
};

#endif //ETS2_MQTT_CONNECTOR_TELEMETRY_STATE_HPP
