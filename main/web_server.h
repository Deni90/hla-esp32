#ifndef web_server_h
#define web_server_h

#include <inttypes.h>

#include "esp_http_server.h"

#include "loom_iface.h"

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
    WebServer(ILoom& callback);

    /**
     * @brief Initialize web server
     *
     * Initialize web server paths and load resources
     */
    void initialize();

  private:
    static esp_err_t resourcehandler(httpd_req_t* req);
    static esp_err_t handleGetWifiInfo(httpd_req_t* req);
    static esp_err_t handleSetWifiInfo(httpd_req_t* req);
    static esp_err_t handleGetLiftplan(httpd_req_t* req);
    static esp_err_t handleSetLiftplan(httpd_req_t* req);
    static esp_err_t handleDeleteLiftplan(httpd_req_t* req);

    ILoom& mCallback;
};
}   // namespace hla
#endif   // web_server_h