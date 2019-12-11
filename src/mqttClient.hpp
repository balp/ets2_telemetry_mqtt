#ifndef MQTTCLIENT_HPP
#define MQTTCLIENT_HPP

#include <cstdio>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <cerrno>

#include "scslog.hpp" // XXX Change to logging interface

namespace mqttclient {
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
        { MOSQ_ERR_ERRNO, "if a system call returned an error" },
        // { MOSQ_ERR_MALFORMED_UTF8, "if the topic is not valid UTF-8" },
        { 0, nullptr}
    };
    const char* err2msg(int code) {
        for(int i = 0; error_codes[i].name ; ++i) {
            if (error_codes[i].value == code) {
                return error_codes[i].name;
            }
        }
        static char msg[30];
        snprintf(msg, 29, "unknown: %d", code);
        return msg;
    }
}


class Ets2MqttWrapper : public mosqpp::mosquittopp
{
    private:
        const char *_host = nullptr;
        int _port = 1883;
        int _keepalive = 120;
        Logger& _logger;
    public:
        Ets2MqttWrapper(const char *id,
                const char *host,
                int port,
                Logger& logger) :
            mosquittopp(id),
            _logger(logger) {
                mosqpp::lib_init();
                _host = host;
                _port = port;
                connect(_host, _port, _keepalive);
                _logger.message("MQTT: Constructor, connect...");
            }
        ~Ets2MqttWrapper() override = default;;

        void on_connect(int rc) override {
            std::clog << "Connected with code " << rc << std::endl;
            _logger.message("MQTT: on_connect");
            _logger.message(mqttclient::err2msg(rc));
            if (rc == 0)
            {
                subscribe(nullptr, "ets2/telematic");
            }
        };
        void on_message(const struct mosquitto_message *message) override {};
        void on_subscribe(int mid, int qos_count, const int *granted_qos) override {
            _logger.message("MQTT Subscribed");
            std::clog << "Subscription succeeded." << std::endl;
        };
        int publish_mqtt(int *mid, const char *topic, size_t payloadlen= 0, const void *payload= nullptr, int qos= 0, bool retain= false) {
            int res =  mosqpp::mosquittopp::publish(mid, topic, payloadlen, payload, qos, retain);
            if(MOSQ_ERR_SUCCESS != res) {
                _logger.error("MQTT Publish error");
                _logger.message(mqttclient::err2msg(res));
            }
            if(MOSQ_ERR_ERRNO == res) {
                char buffer[50];
                strerror_r(errno, buffer, 50);
                _logger.message(buffer);
                reconnect();
            }
            if(MOSQ_ERR_NO_CONN == res) {
                reconnect();
            }
            return res;
        };
};

#endif // MQTTCLIENT_HPP
