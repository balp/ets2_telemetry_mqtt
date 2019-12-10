//
// Created by balp on 2019-12-10.
//

#include "simulation_timestamp.hpp"

nlohmann::json SimulationTimestamp::getJson() {
    nlohmann::json j;
    j["timestamp"] = timestamp;
    j["raw_rendering_timestamp"] = raw_rendering_timestamp;
    j["raw_simulation_timestamp"] = raw_simulation_timestamp;
    j["raw_paused_simulation_timestamp"] = raw_paused_simulation_timestamp;
    j["common"] = nlohmann::json::object();
    return j;
}