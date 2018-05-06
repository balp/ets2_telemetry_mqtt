#include <mosquittopp.h>

class NRMKMqttWrapper : public mosqpp::mosquittopp
{
    public:
        NRMKMqttWrapper(const char *id, const char *host, int port);
        ~NRMKMqttWrapper();

        void on_connect(int rc);
        void on_message(const struct mosquitto_message *message);
        void on_subcribe(int mid, int qos_count, const int *granted_qos);
};
