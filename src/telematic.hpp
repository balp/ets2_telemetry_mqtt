
#ifndef TELEMATIC_H
#define TELEMATIC_H
#ifdef _WIN32
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <string.h>
#include <assert.h>

#include "json.hpp"

#include "scssdk_telemetry.h"
#include "eurotrucks2/scssdk_eut2.h"
#include "eurotrucks2/scssdk_telemetry_eut2.h"
#include "amtrucks/scssdk_ats.h"
#include "amtrucks/scssdk_telemetry_ats.h"

class ITelematic
{
  protected:
    const scs_string_t _name;
    const scs_value_type_t _value_type;

    explicit ITelematic(const scs_string_t name,
                        const scs_value_type_t value_type) : _name(name),
                                                             _value_type(value_type) {}

  public:
    virtual ~ITelematic() {}
    /**
     * Return the object as json
     */
    virtual nlohmann::json getJson() = 0;

    /**
     * Register as hander for SCS telematics channel.
     */
    virtual scs_result_t register_for_channel(const scs_telemetry_init_params_v100_t *const telemetry)
    {
        return telemetry->register_for_channel(_name,
                                        SCS_U32_NIL,
                                        _value_type,
                                        SCS_TELEMETRY_CHANNEL_FLAG_no_value,
                                        value_callback,
                                        this);
    }
    static SCSAPI_VOID value_callback(const scs_string_t name,
                                      const scs_u32_t index,
                                      const scs_value_t *const value,
                                      const scs_context_t context)
    {
        assert(value);
        assert(context);
        ITelematic *object = static_cast<ITelematic *>(context);
        object->setValue(value);
    }
    virtual void setValue(const scs_value_t *const value) = 0;
};

class TelematicUint32 : public ITelematic
{
    uint32_t _value;

  public:
    virtual ~TelematicUint32() {}
    TelematicUint32(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_u32),
                                               _value(0) {}

    void setValue(const scs_value_t *const value) override
    {
        assert(value->type == SCS_VALUE_TYPE_u32);
        _value = value->value_u32.value;
    }

    nlohmann::json getJson() override
    {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

class TelematicInt32 : public ITelematic
{
    int32_t _value;

  public:
    virtual ~TelematicInt32() {}
    TelematicInt32(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_s32),
                                              _value(0) {}

    void setValue(const scs_value_t *const value) override
    {
        assert(value->type == SCS_VALUE_TYPE_s32);
        _value = value->value_s32.value;
    }

    nlohmann::json getJson() override
    {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

class TelematicFloat : public ITelematic
{
    float _value;

  public:
    TelematicFloat(const scs_string_t name) : ITelematic(name, SCS_VALUE_TYPE_float),
                                              _value(0.0f) {}

    void setValue(const scs_value_t *const value) override
    {
        assert(value->type == SCS_VALUE_TYPE_float);
        _value = value->value_float.value;
    }

    nlohmann::json getJson() override
    {
        nlohmann::json j;
        j[_name] = _value;
        return j;
    }
};

#endif // TELEMATIC_H