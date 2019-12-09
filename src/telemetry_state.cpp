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

void TelemetryState::update_config(const scs_telemetry_configuration_t *const pConfiguration) {
    _logger.message(pConfiguration->id);
    if(register_for_channel && pConfiguration && pConfiguration->id) {
        if(!strcmp(pConfiguration->id, SCS_TELEMETRY_CONFIG_truck)) {
            const scs_named_value_t *const wheel_count_attr = find_attribute(*pConfiguration,
                                                                             SCS_TELEMETRY_CONFIG_ATTRIBUTE_wheel_count,
                                                                             SCS_U32_NIL,
                                                                             SCS_VALUE_TYPE_u32);
            size_t wheel_count = wheel_count_attr ? wheel_count_attr->value.value_u32.value : 0;
            if (wheel_count > kMaxWheelCount) {
                wheel_count = kMaxWheelCount;
            }
            // Update registrations for wheel channels
            if (register_for_channel) {
                for (int i = 0; i < kMaxWheelCount; ++i) {
                    if (i < wheel_count) {
                        _logger.message("  register");
                        _wheel_on_ground[i]->register_for_channel(register_for_channel, i);
                    } else {
                        _logger.message("unregister");
                        _wheel_on_ground[i]->unregister_from_channel(unregister_from_channel, i);

                    }
                }
            }
        }
    }
}