
#ifndef TELEMATIC_H
#define TELEMATIC_H
#ifdef _WIN32
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif

#include <cstdio>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <cassert>

#include "json.hpp"

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"

extern scs_log_t game_log;

class ITelematic {
protected:
    const scs_string_t _name;
    const scs_value_type_t _value_type;

    explicit ITelematic(const scs_string_t name,
                        const scs_value_type_t value_type) : _name(name),
                                                             _value_type(value_type) {}

public:
    virtual ~ITelematic() = default;

    /**
     * Return the object as json
     */
    virtual nlohmann::json getJson() = 0;

    /**
     * Register as hander for SCS telematics channel.
     */
    virtual scs_result_t register_for_channel(const scs_telemetry_init_params_v101_t *const telemetry) {
        if (telemetry != nullptr) {
            return telemetry->register_for_channel(_name,
                                                   SCS_U32_NIL,
                                                   _value_type,
                                                   SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                                   value_callback,
                                                   this);
        } else {
            return SCS_RESULT_generic_error;
        }
    }

    virtual scs_result_t register_for_channel(scs_telemetry_register_for_channel_t register_for_channel,
                                              scs_u32_t index) {
        return register_for_channel(_name,
                                    index,
                                    _value_type,
                                    SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                    value_callback,
                                    this);
    }

    /**
     * Unregister as hander from SCS telematics channel.
     */
    virtual scs_result_t unregister_from_channel(const scs_telemetry_init_params_v101_t *const telemetry) {
        if (telemetry != nullptr) {
            return telemetry->unregister_from_channel(_name,
                                                      SCS_U32_NIL,
                                                      _value_type);
        } else {
            return SCS_RESULT_generic_error;
        }

    }

    virtual scs_result_t unregister_from_channel(scs_telemetry_unregister_from_channel_t unregister_from_channel,
                                                 scs_u32_t index) {
        return unregister_from_channel(_name,
                                       index,
                                       _value_type);
    }

    static SCSAPI_VOID value_callback(const scs_string_t name,
                                      const scs_u32_t index,
                                      const scs_value_t *const value,
                                      const scs_context_t context) { // NOLINT(misc-misplaced-const)
        if (value != nullptr && context != nullptr) {
            auto object = static_cast<ITelematic *>(context);
            object->setValue(value, index);
        }
    }

    virtual void setValue(const scs_value_t *value, scs_u32_t index) = 0;
};

class TelematicUint32 : public ITelematic {
    uint32_t _value;

public:
    ~TelematicUint32() override = default;
    explicit TelematicUint32(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_u32),
                                               _value(0) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        if (value->type == SCS_VALUE_TYPE_u32) {
            _value = value->value_u32.value;
        } else {
            game_log(SCS_LOG_TYPE_warning, "TelematicUint32() non U32 data");
        }

    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

class TelematicInt32 : public ITelematic {
    int32_t _value;

public:
    ~TelematicInt32() override = default;
    explicit TelematicInt32(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_s32),
                                              _value(0) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_s32);
        _value = value->value_s32.value;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

class TelematicFloat : public ITelematic {
    float _value;

public:
    ~TelematicFloat() override = default;
    explicit TelematicFloat(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_float),
                                              _value(0.0f) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_float);
        _value = value->value_float.value;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

class TelematicDPlacement : public ITelematic {
    scs_value_dplacement_t _value;

public:
    ~TelematicDPlacement() override = default;
    explicit TelematicDPlacement(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_dplacement),
                                                   _value({{0}, {0}, 0}) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_dplacement);
        _value = value->value_dplacement;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name]["position"]["x"] = _value.position.x;
        j[_name]["position"]["y"] = _value.position.y;
        j[_name]["position"]["z"] = _value.position.z;
        j[_name]["orientation"]["heading"] = _value.orientation.heading;
        j[_name]["orientation"]["pitch"] = _value.orientation.pitch;
        j[_name]["orientation"]["roll"] = _value.orientation.roll;
        return j;
    }
};

class TelematicFPlacement : public ITelematic {
    scs_value_fplacement_t _value;

public:
    ~TelematicFPlacement() override = default;
    explicit TelematicFPlacement(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_fplacement),
                                                   _value({{0},
                                                           {0}}) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_fplacement);
        _value = value->value_fplacement;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name]["position"]["x"] = _value.position.x;
        j[_name]["position"]["y"] = _value.position.y;
        j[_name]["position"]["z"] = _value.position.z;
        j[_name]["orientation"]["heading"] = _value.orientation.heading;
        j[_name]["orientation"]["pitch"] = _value.orientation.pitch;
        j[_name]["orientation"]["roll"] = _value.orientation.roll;
        return j;
    }
};

class TelematicFVector : public ITelematic {
    scs_value_fvector_t _value;

public:
    ~TelematicFVector() override = default;
    explicit TelematicFVector(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_fvector),
                                                _value({0}) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_fvector);
        _value = value->value_fvector;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name]["x"] = _value.x;
        j[_name]["y"] = _value.y;
        j[_name]["z"] = _value.z;
        return j;
    }
};

class TelematicBool : public ITelematic {
    bool _value;

public:
    ~TelematicBool() override = default;

    explicit TelematicBool(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_bool),
                                             _value(false) {}

    void setValue(const scs_value_t *const value, const scs_u32_t index) override {
        assert(value->type == SCS_VALUE_TYPE_bool);
        _value = value->value_bool.value;
    }

    nlohmann::json getJson() override {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

#endif // TELEMATIC_H