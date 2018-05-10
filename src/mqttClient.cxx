#ifdef _WIN32
#  define WINVER 0x0500
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <string.h>
#include <assert.h>

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"

#define UNUSED(x)

namespace {
    struct {
        int value;
        const char* name;
    } error_codes[] = {
        { MOSQ_ERR_SUCCESS, "on success." },
        { MOSQ_ERR_INVAL, "if the input parameters were invalid." },
        { MOSQ_ERR_NOMEM, "if an out of memory condition occurred." },
        { MOSQ_ERR_NO_CONN, "if the client isn’t connected to a broker." },
        { MOSQ_ERR_PROTOCOL, "if there is a protocol error communicating with the broker." },
        { MOSQ_ERR_PAYLOAD_SIZE, "if payloadlen is too large." },
        // { MOSQ_ERR_MALFORMED_UTF8, "if the topic is not valid UTF-8" },
        { 0, 0}
    };
    const char* err2msg(int code) {
        for(int i = 0; error_codes[i].name ; ++i) {
            if (error_codes[i].value == code) {
                return error_codes[i].name;
            }
        }
        return "unknown";
    }

};

class Ets2MqttWrapper : public mosqpp::mosquittopp
{
    public:
        Ets2MqttWrapper(const char *id, const char *host, int port) : mosquittopp(id) {
            mosqpp::lib_init();
            auto keepalive = 120;
            connect(host, port, keepalive);
        }
        ~Ets2MqttWrapper() {};

        void on_connect(int rc) {
            std::clog << "Connected with code " << rc << std::endl;
            if (rc == 0)
            {
                subscribe(NULL, "ets2/telematic");
            }
        };
        void on_message(const struct mosquitto_message *message) {};
        void on_subcribe(int mid, int qos_count, const int *granted_qos) {
            std::clog << "Subscription succeeded." << std::endl;
        };
};

#if 0 // mqtt publishing main function, to be removed
int main(int argc, char* argv[])
{
    std::clog << "main::init" << std::endl;
    Ets2MqttWrapper * mqttHdl;
    mqttHdl = new Ets2MqttWrapper("ETS2-MQTT", "127.0.0.1", 1883);
    for(int i = 0; i < 10 ; ++i) {
        const size_t data_size = 5;
        char data[data_size] = {0};
        snprintf(data, data_size, "%02d", i);
        std::clog << "main::counter " << i << std::endl;
        auto res = mqttHdl->publish(NULL, "ets2/counter", data_size, data);
        std::clog << "main::publish " << res
            << " " << err2msg(res) << std::endl;
        sleep(1);
    }

    return -1;
}
#endif 

scs_timestamp_t last_timestamp = static_cast<scs_timestamp_t>(-1);

struct telemetry_state_t
{
    scs_timestamp_t timestamp;
    scs_timestamp_t raw_rendering_timestamp;
    scs_timestamp_t raw_simulation_timestamp;
    scs_timestamp_t raw_paused_simulation_timestamp;

    bool	orientation_available;
    float	heading;
    float	pitch;
    float	roll;

    float	speed;
    float	rpm;
    int	gear;

} telemetry;

Ets2MqttWrapper* mqttHdl = NULL;
scs_log_t game_log = NULL;
bool output_paused = true;

SCSAPI_VOID telemetry_frame_start( const scs_event_t UNUSED(event),
        const void *const event_info,
        const scs_context_t UNUSED(context))
{
    const struct scs_telemetry_frame_start_t *const info
        = static_cast<const scs_telemetry_frame_start_t *>(event_info);
    if (last_timestamp == static_cast<scs_timestamp_t>(-1)) {
        last_timestamp = info->paused_simulation_time;
    }
    if (info->flags & SCS_TELEMETRY_FRAME_START_FLAG_timer_restart) {
        last_timestamp = 0;
    }
    telemetry.timestamp += (info->paused_simulation_time
            - last_timestamp);
    last_timestamp = info->paused_simulation_time;
    telemetry.raw_rendering_timestamp = info->render_time;
    telemetry.raw_simulation_timestamp = info->simulation_time;
    telemetry.raw_paused_simulation_timestamp
        = info->paused_simulation_time;
}

SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event),
        const void *const UNUSED(event_info),
        const scs_context_t UNUSED(context))
{
    if (output_paused) {
        return;
    }
    if (mqttHdl == NULL) {
        return;
    }
    const size_t buffer_size = 150;
    char buffer[buffer_size] = {0};
    snprintf(buffer, buffer_size,
            "%" SCS_PF_U64 ";%" SCS_PF_U64 ";%" SCS_PF_U64 ";%" SCS_PF_U64 ";%f;%f;%f;%f;%f;%d",
            telemetry.timestamp,
            telemetry.raw_rendering_timestamp,
            telemetry.raw_simulation_timestamp,
            telemetry.raw_paused_simulation_timestamp,
            telemetry.heading,
            telemetry.pitch,
            telemetry.roll,
            telemetry.speed,
            telemetry.rpm,
            telemetry.gear
            );
    auto res = mqttHdl->publish(NULL, "ets2/data", buffer_size, buffer);
}

SCSAPI_VOID telemetry_pause(const scs_event_t event,
        const void *const UNUSED(event_info),
        const scs_context_t UNUSED(context))
{
    output_paused = (event == SCS_TELEMETRY_EVENT_paused);
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event,
        const void *const event_info,
        const scs_context_t UNUSED(context))
{
}

SCSAPI_VOID telemetry_store_orientation(const scs_string_t name,
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
    *static_cast<int *>(context) = value->value_s32.value;
}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version,
        const scs_telemetry_init_params_t *const params)
{
    if (version != SCS_TELEMETRY_VERSION_1_00) {
        return SCS_RESULT_unsupported;
    }
    const scs_telemetry_init_params_v100_t *const version_params
        = static_cast<const scs_telemetry_init_params_v100_t *>(params);
    try {
        mqttHdl = new Ets2MqttWrapper("ETS2-MQTT", "127.0.0.1", 1883);
    } catch(...) {
        mqttHdl = NULL;
    }
    if (mqttHdl == NULL) {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to initialize the log file");
        return SCS_RESULT_generic_error;
    }
    const bool events_registered =
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_start, telemetry_frame_start, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_frame_end, telemetry_frame_end, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
        (version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok)
        ;
    if (! events_registered) {
        version_params->common.log(SCS_LOG_TYPE_error, "Unable to register event callbacks");
        return SCS_RESULT_generic_error;
    }
    version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, NULL);

    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_world_placement, SCS_U32_NIL, SCS_VALUE_TYPE_euler, SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_orientation, &telemetry);
    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &telemetry.speed);
    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm, SCS_U32_NIL, SCS_VALUE_TYPE_float, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_float, &telemetry.rpm);
    version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear, SCS_U32_NIL, SCS_VALUE_TYPE_s32, SCS_TELEMETRY_CHANNEL_FLAG_none, telemetry_store_s32, &telemetry.gear);

    game_log = version_params->common.log;
    game_log(SCS_LOG_TYPE_message, "Initializing telemetry mqtt gateway");
    memset(&telemetry, 0, sizeof(telemetry));
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
	DWORD  reason_for_call,
	LPVOID reseved
)
{
	if (reason_for_call == DLL_PROCESS_DETACH) {
		finish_log();
	}
	return TRUE;
}
#endif

#ifdef __linux__
void __attribute__ ((destructor)) unload(void)
{
	
}
#endif
