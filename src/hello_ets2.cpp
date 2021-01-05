#ifdef _WIN32
#  define WINVER 0x0500
#  define _WIN32_WINNT 0x0500
#  include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"

scs_log_t game_log = NULL;
#define UNUSED(x)
void log_line(const scs_log_type_t type, const char *const text, ...)
{
    if (! game_log) {
        return;
    }
    char formated[1000];

    va_list args;
            va_start(args, text);
    vsnprintf_s(formated, sizeof(formated), _TRUNCATE, text, args);
    formated[sizeof(formated) - 1] = 0;
            va_end(args);

    game_log(type, formated);
}

SCSAPI_VOID telemetry_frame_start(const scs_event_t UNUSED(event), const void *const event_info, const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_frame_start.");
}

SCSAPI_VOID telemetry_frame_end(const scs_event_t UNUSED(event), const void *const UNUSED(event_info), const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_frame_end.");
}

SCSAPI_VOID telemetry_pause(const scs_event_t event, const void *const UNUSED(event_info), const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_pause.");
}

SCSAPI_VOID telemetry_start(const scs_event_t event, const void *const UNUSED(event_info), const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_start.");
}

SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void *const event_info, const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_configuration.");
}

SCSAPI_VOID telemetry_gameplay_event(const scs_event_t event, const void *const event_info, const scs_context_t UNUSED(context))
{
    log_line(SCS_LOG_TYPE_message, "telemetry_gameplay_event.");
}

SCSAPI_VOID telemetry_store_orientation(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{

}

SCSAPI_VOID telemetry_store_float(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{

}

SCSAPI_VOID telemetry_store_s32(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{

}

SCSAPI_VOID telemetry_store_dplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value, const scs_context_t context)
{

}

SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t *const params)
{
    if (version != SCS_TELEMETRY_VERSION_1_01) {
        return SCS_RESULT_unsupported;
    }
    const scs_telemetry_init_params_v101_t *const version_params = static_cast<const scs_telemetry_init_params_v101_t *>(params);
    version_params->common.log(SCS_LOG_TYPE_message, "Hello World!");
    if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0) {
        version_params->common.log(SCS_LOG_TYPE_message, "Hello Europe!");
    } else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0) {
        version_params->common.log(SCS_LOG_TYPE_message, "Hello USA!");
    }
    game_log = version_params->common.log;
    if (version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) != SCS_RESULT_ok) {
        game_log(SCS_LOG_TYPE_error, "Unable to register for pause events");
    }
    if (version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_start, NULL) != SCS_RESULT_ok) {
        game_log(SCS_LOG_TYPE_error, "Unable to register for start events");
    }
    if (version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, NULL) != SCS_RESULT_ok) {
        game_log(SCS_LOG_TYPE_error, "Unable to register for configuration events");
    }
    if (version_params->register_for_event(SCS_TELEMETRY_EVENT_gameplay, telemetry_gameplay_event, NULL) != SCS_RESULT_ok) {
        game_log(SCS_LOG_TYPE_error, "Unable to register for gameplay events");
    }

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
//        finish_log();
    }
    return TRUE;
}
#endif

#ifdef __linux__
void __attribute__ ((destructor)) unload(void)
{
//	finish_log();
}
#endif

