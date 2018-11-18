#include <stdio.h>
#include <unistd.h>
#include <mosquittopp.h>
#include <iostream>
#include <string>
#include <string.h>
#include <assert.h>

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
    private:
        const char *_host = NULL;
        int _port = 1883;
        int _keepalive = 120;
    public:
        Ets2MqttWrapper(const char *id, const char *host, int port) : mosquittopp(id) {
            mosqpp::lib_init();
            _host = host;
            _port = port;
            connect(_host, _port, _keepalive);
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
	int publish(int *mid, const char *topic, int payloadlen=0, const void *payload=NULL, int qos=0, bool retain=false) {
            int res =  mosqpp::mosquittopp::publish(mid, topic, payloadlen, payload, qos, retain);
            if(MOSQ_ERR_NO_CONN == res) {
                connect(_host, _port, _keepalive);
            }
            return res;
	};
};


