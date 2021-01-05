#ifdef _WIN32
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif

#include <string>
#include <vector>
#include <cstring>

#include <nlohmann/json.hpp>
#include <eurotrucks2/scssdk_eut2.h>

#include "telematic.hpp"
#include "scslog.hpp"
#include "telemetry_state.hpp"
#include "simulation_timestamp.hpp"
#include "mqttClient.hpp"

#define UNUSED(x)
#ifdef _WIN32
FILE *log_file = NULL;

bool init_log(void)
{
    if (log_file) {
        return true;
    }
    log_file = fopen("telemetry_position.log", "wt");
    if (! log_file) {
        return false;
    }
    fprintf(log_file, "Log opened\n");
    return true;
}

void finish_log(void)
{
    if (! log_file) {
        return;
    }
    fprintf(log_file, "Log ended\n");
    fclose(log_file);
    log_file = NULL;
}

void log_print(const char *const text, ...)
{
    if (! log_file) {
        return;
    }
    va_list args;
            va_start(args, text);
    vfprintf(log_file, text, args);
            va_end(args);
}

void log_line(const char *const text, ...)
{
    if (! log_file) {
        return;
    }
    va_list args;
            va_start(args, text);
    vfprintf(log_file, text, args);
    fprintf(log_file, "\n");
            va_end(args);
}


#endif
scs_timestamp_t last_timestamp = static_cast<scs_timestamp_t>(-1);
static Logger logger; // NOLINT(cert-err58-cpp)
static Ets2MqttWrapper *mqttHdl = nullptr;
scs_log_t game_log = nullptr;
static bool output_paused = true;

struct ContextData {
    SimulationTimestamp* timestamp_p;
    TelemetryState* telemetry_p;
};

SCSAPI_VOID telemetry_frame_start(const scs_event_t UNUSED(event),
                                  const void *const event_info,
                                  const scs_context_t context) { // NOLINT(misc-misplaced-const)
    const auto *const info = static_cast<const scs_telemetry_frame_start_t *>(event_info);
    auto* contextData = static_cast<ContextData *>(context);
    if (last_timestamp == static_cast<scs_timestamp_t>(-1)) {
        last_timestamp = info->paused_simulation_time;
    }
    if (info->flags & SCS_TELEMETRY_FRAME_START_FLAG_timer_restart) {
        last_timestamp = 0;
    }
    contextData->timestamp_p->timestamp += (info->paused_simulation_time - last_timestamp);
    last_timestamp = info->paused_simulation_time;
    contextData->timestamp_p->raw_rendering_timestamp = info->render_time;
    contextData->timestamp_p->raw_simulation_timestamp = info->simulation_time;
    contextData->timestamp_p->raw_paused_simulation_timestamp = info->paused_simulation_time;
}

SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event),
                                const void *const UNUSED(event_info),
                                const scs_context_t context) { // NOLINT(misc-misplaced-const)
    if (output_paused) {
        return;
    }
    if (mqttHdl == nullptr) {
        return;
    }
    auto* contextData = static_cast<ContextData *>(context);
    std::string json_string = contextData->telemetry_p->getJson().dump();
    mqttHdl->publish_mqtt(nullptr, "ets2/data", strlen(json_string.c_str()), json_string.c_str());
    std::string json_timestamp_string = contextData->timestamp_p->getJson().dump();
    mqttHdl->publish_mqtt(nullptr, "ets2/timestamp", strlen(json_timestamp_string.c_str()),
                          json_timestamp_string.c_str());
}

static nlohmann::json &setNamedValueToJson(nlohmann::json &j, const scs_named_value_t *current);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-misplaced-const"
SCSAPI_VOID telemetry_pause(const scs_event_t event,
                            const void *const UNUSED(event_info),
                            const scs_context_t UNUSED(context)) {
    output_paused = (event == SCS_TELEMETRY_EVENT_paused);
    nlohmann::json j;
    if (event == SCS_TELEMETRY_EVENT_paused) {
        j["paused"] = true;
    } else if (event == SCS_TELEMETRY_EVENT_started) {
        j["paused"] = false;
    } else {
    }
    std::string json_string = j.dump();
    mqttHdl->publish_mqtt(nullptr, "ets2/info", strlen(json_string.c_str()), json_string.c_str());
}
#pragma clang diagnostic pop

SCSAPI_VOID telemetry_configuration(const scs_event_t UNUSED(event),
                                    const void *const event_info,
                                    const scs_context_t context) { // NOLINT(misc-misplaced-const)
    const auto *const info = static_cast<const scs_telemetry_configuration_t *>(event_info);
    auto* contextData = static_cast<ContextData *>(context);
    contextData->telemetry_p->update_config(info);
    nlohmann::json j = nlohmann::json::object();
    for (const scs_named_value_t *current = info->attributes; current->name; ++current) {
        j = setNamedValueToJson(j, current);
    }
    std::string topic = std::string("ets2/info/config/") + info->id;
    std::string json_string = j.dump();
    mqttHdl->publish_mqtt(nullptr, topic.c_str(), strlen(json_string.c_str()), json_string.c_str(), 0, true);
}


static nlohmann::json &setNamedValueToJson(nlohmann::json &j, const scs_named_value_t *current) {
    switch (current->value.type) {
        case SCS_VALUE_TYPE_INVALID:
            j[current->name] = "[invalid]";
            break;
        case SCS_VALUE_TYPE_bool:
            j[current->name] = current->value.value_bool.value != 0;
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
        case SCS_VALUE_TYPE_s64:
            j[current->name] = current->value.value_s64.value;
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
    return j;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-misplaced-const"
SCSAPI_VOID telemetry_gameplay(const scs_event_t UNUSED(event),
                               const void *const event_info,
                               const scs_context_t UNUSED(context)) {
    const auto *const info = static_cast<const scs_telemetry_gameplay_event_t *>(event_info);
    nlohmann::json j = nlohmann::json::object();
    for (const scs_named_value_t *current = info->attributes; current->name; ++current) {
        j = setNamedValueToJson(j, current);
    }
    std::string topic = std::string("ets2/info/gameplay/") + info->id;
    std::string json_string = j.dump();
    mqttHdl->publish_mqtt(nullptr, topic.c_str(),
            strlen(json_string.c_str()),
            json_string.c_str());
}


void publish_game_info(const scs_telemetry_init_params_v100_t *const params) {
    if (mqttHdl == nullptr) {
        return;
    }
    nlohmann::json j;
    j["name"] = params->common.game_name;
    j["id"] = params->common.game_id;
    j["raw_version"] = params->common.game_version;
    j["version"] = {{"major", SCS_GET_MAJOR_VERSION(params->common.game_version)}, // NOLINT(hicpp-signed-bitwise) [Part of scs interface]
                    {"minor", SCS_GET_MINOR_VERSION(params->common.game_version)}}; // NOLINT(hicpp-signed-bitwise) [Part of scs interface]


    std::string json_string = j.dump();
    mqttHdl->publish_mqtt(nullptr, "ets2/game", strlen(json_string.c_str()), json_string.c_str(), 0, true);
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version,
                                 const scs_telemetry_init_params_t *const params) {
    if (version != SCS_TELEMETRY_VERSION_1_01) { // NOLINT(hicpp-signed-bitwise) [Part of scs interface]
        return SCS_RESULT_unsupported;
    }
    const auto *const version_params = reinterpret_cast<const scs_telemetry_init_params_v101_t *>(params);
#ifdef _WIN32
    if (! init_log()) {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to initialize the log file");
        return SCS_RESULT_generic_error;
    }
#endif
    game_log = version_params->common.log;
    logger.setGameLog(game_log);
    logger.message("Initializing telemetry mqtt gateway");
    try {
        mqttHdl = new Ets2MqttWrapper("ETS2-MQTT", "127.0.0.1", 1883, logger);
    }
    catch (...) {
        mqttHdl = nullptr;
    }
    if (mqttHdl == nullptr) {
        logger.error("No MQTT Connection");
        return SCS_RESULT_generic_error;
    }
    logger.message("MQTT Connected");
    publish_game_info(version_params);
    auto contextData = new ContextData();
    contextData->telemetry_p = new TelemetryState(version_params->register_for_channel,
            version_params->unregister_from_channel,
            logger);
    contextData->timestamp_p = new SimulationTimestamp();

    version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, contextData);
    version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_frame_end, contextData);
    version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, contextData);
    version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, contextData);
    version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, contextData);
    version_params->register_for_event(SCS_TELEMETRY_EVENT_gameplay, telemetry_gameplay, contextData);

    contextData->telemetry_p->register_for_channel();

    last_timestamp = static_cast<scs_timestamp_t>(-1);
    output_paused = true;
    return SCS_RESULT_ok;
}

SCSAPI_VOID scs_telemetry_shutdown(void) {
    logger.message("Shutdown telemetry mqtt gateway");
    game_log = nullptr;
    logger.setGameLog(nullptr);
}

#ifdef _WIN32
BOOL APIENTRY DllMain(
    HMODULE module,
    DWORD reason_for_call,
    LPVOID reseved)
{
    if (reason_for_call == DLL_PROCESS_DETACH) {
        finish_log();
    }
    return TRUE;
}
#endif

#ifdef __linux__

void __attribute__((destructor)) unload() {
}

#endif
