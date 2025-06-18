#ifndef web_server_h
#define web_server_h

#include <inttypes.h>

#include "esp_http_server.h"

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
     * @param callback clock interface object
     */
    WebServer(IHla& callback);

    /**
     * @brief Initialize web server
     *
     * Initialize web server paths and load resources
     */
    void Initialize();

  private:
    static esp_err_t ResourceHandler(httpd_req_t* req);
    static esp_err_t HandleGetWifiInfo(httpd_req_t* req);
    static esp_err_t HandleSetWifiInfo(httpd_req_t* req);
    static esp_err_t HandleGetLiftplan(httpd_req_t* req);
    static esp_err_t HandleSetLiftplan(httpd_req_t* req);
    static esp_err_t HandleDeleteLiftplan(httpd_req_t* req);

    IHla& mCallback;
};
}   // namespace hla
#endif   // web_server_h