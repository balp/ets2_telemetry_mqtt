//
// Created by balp on 2019-12-09.
//

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
#include "telemetry_state.hpp"

#include "scslog.hpp" // XXX Change to logging interface

/**
 * @brief Finds attribute with specified name in the configuration structure.
 *
 * Returns NULL if the attribute was not found or if it is not of the expected type.
 */
const scs_named_value_t *find_attribute(const scs_telemetry_configuration_t &configuration,
                                        const char *const name,
                                        const scs_u32_t index,
                                        const scs_value_type_t expected_type) {
    for (const scs_named_value_t *current = configuration.attributes; current->name; ++current) {
        if ((current->index != index) || (strcmp(current->name, name) != 0)) {
            continue;
        }
        if (current->value.type == expected_type) {
            return current;
        }
        break;
    }
    return nullptr;
}

void Trailer::update_config(const scs_telemetry_configuration_t *const pConfiguration) {

    if (registerForChannel && pConfiguration && pConfiguration->id) {
        if(!strcmp(pConfiguration->id, SCS_TELEMETRY_CONFIG_trailer)) {
            const scs_named_value_t *const wheel_count_attr = find_attribute(*pConfiguration,
                                                                             SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count,
                                                                             SCS_U32_NIL,
                                                                             SCS_VALUE_TYPE_u32);
            size_t wheel_count = wheel_count_attr ? wheel_count_attr->value.value_u32.value : 0;
            if (wheel_count > kMaxWheelCount) {
                wheel_count = kMaxWheelCount;
            }
            no_trailer_wheels = wheel_count;
            // Update registrations for wheel channels
            if (registerForChannel && unregisterFromChannel) {
                for (int i = 0; i < kMaxWheelCount; ++i) {
                    if (i < wheel_count) {
                        trailer_wheels[i]->register_for_channel(registerForChannel);
                    } else {
                        trailer_wheels[i]->unregister_from_channel(unregisterFromChannel);
                    }
                }
            }
        }
    }
}

nlohmann::json Trailer::getJson() {
    nlohmann::json j;
    for (const auto& channel : _trailer) { // XXX Move into Trailer
        j.update(channel->getJson());
    }
    j["wheels"] = nlohmann::json::array();
    for (int index = 0; index < no_trailer_wheels; ++index) { // XXX Move into Trailer
        j["wheels"] += trailer_wheels[index]->getJson();
    }
    return j;
}

void TelemetryState::update_config(const scs_telemetry_configuration_t *const pConfiguration) {
    _trailer_state.update_config(pConfiguration);
    if(registerForChannel && pConfiguration && pConfiguration->id) {
        if(!strcmp(pConfiguration->id, SCS_TELEMETRY_CONFIG_truck)) {
            const scs_named_value_t *const wheel_count_attr = find_attribute(*pConfiguration,
                                                                             SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count,
                                                                             SCS_U32_NIL,
                                                                             SCS_VALUE_TYPE_u32);
            size_t wheel_count = wheel_count_attr ? wheel_count_attr->value.value_u32.value : 0;
            if (wheel_count > kMaxWheelCount) {
                wheel_count = kMaxWheelCount;
            }
            no_truck_wheels = wheel_count;
            // Update registrations for wheel channels
            if (registerForChannel && unregisterFromChannel) {
                for (int i = 0; i < kMaxWheelCount; ++i) {
                    if (i < wheel_count) {
                        truck_wheels[i]->register_for_channel(registerForChannel);
                    } else {
                        truck_wheels[i]->unregister_from_channel(unregisterFromChannel);
                    }
                }
            }
        }


    }
}

nlohmann::json TelemetryState::getJson() {
    nlohmann::json j;
    j["common"] = nlohmann::json::object();
    for (const auto& channel : _common) {
        j["common"].update(channel->getJson());
    }
    j["truck"] = nlohmann::json::object();
    for (const auto& channel : _truck_state._truck) { // XXX Move into Truck
        j["truck"].update(channel->getJson());
    }
    j["truck_wheels"] = nlohmann::json::array();
    for (int index = 0; index < no_truck_wheels; ++index) {  // XXX Move into Truck
        j["truck_wheels"] += truck_wheels[index]->getJson();
    }
    j["trailer"] = _trailer_state.getJson();

    return j;
}

scs_result_t TelemetryState::register_for_channel() {
    scs_result_t result = SCS_RESULT_ok;
    for (const auto &channel : _common) {
        auto tmp = channel->register_for_channel(registerForChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    for (const auto &channel : _truck_state._truck) {
        auto tmp = channel->register_for_channel(registerForChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    for (const auto &channel : _trailer_state._trailer) {
        auto tmp = channel->register_for_channel(registerForChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    return result;
}

scs_result_t TelemetryState::unregister_from_channel() {
    scs_result_t result = SCS_RESULT_ok;
    for (const auto &channel : _common) {
        auto tmp = channel->unregister_from_channel(unregisterFromChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    for (const auto &channel : _truck_state._truck) {
        auto tmp = channel->unregister_from_channel(unregisterFromChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    for (const auto &channel : _trailer_state._trailer) {
        auto tmp = channel->unregister_from_channel(unregisterFromChannel, SCS_U32_NIL);
        if (tmp != SCS_RESULT_ok) { result = tmp; }
    }
    return result;
}
