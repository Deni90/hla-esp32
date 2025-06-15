#ifndef web_server_h
#define web_server_h

#include <inttypes.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "hla_iface.h"

namespace hla {
/**
 * @brief A wrapper around an AsyncWebServer
 */
class WebServer {
  public:
    /**
     * @brief Construct a new Web Server object
     *
     * @param port port
     * @param callback clock interface object
     */
    WebServer(int port, IHla& calback);

    /**
     * @brief Initialize web server
     *
     * Initialize web server paths and load resources
     */
    void Initialize();

  private:
    AsyncWebServer mServer;
    IHla& mCallback;

    void HandleGetWifiInfo(AsyncWebServerRequest* request);
    void HandleSetWifiInfo(AsyncWebServerRequest* request, JsonVariant& json);
};
}   // namespace hla
#endif   // web_server_h