//
// Created by balp on 2019-12-10.
//

#ifndef ETS2_MQTT_CONNECTOR_SIMULATION_TIMESTAMP_HPP
#define ETS2_MQTT_CONNECTOR_SIMULATION_TIMESTAMP_HPP


#include <scssdk.h>
#include <nlohmann/json.hpp>

class SimulationTimestamp {
public:
    scs_timestamp_t timestamp;
    scs_timestamp_t raw_rendering_timestamp;
    scs_timestamp_t raw_simulation_timestamp;
    scs_timestamp_t raw_paused_simulation_timestamp;

    SimulationTimestamp() : timestamp(0),
                            raw_rendering_timestamp(0),
                            raw_simulation_timestamp(0),
                            raw_paused_simulation_timestamp(0) {}

    nlohmann::json getJson();
};


#endif //ETS2_MQTT_CONNECTOR_SIMULATION_TIMESTAMP_HPP
