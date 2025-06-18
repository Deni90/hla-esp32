#include <fstream>
#include <iostream>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "web_server.h"
#include "wifi_info.h"

// using hla::IHla;
using hla::IHla;
using hla::WebServer;
using hla::WifiInfo;

static const char* kTag = "web_server";
static char gScratch[10240];

WebServer::WebServer(IHla& callback) : mCallback(callback) {}

void WebServer::Initialize() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) != ESP_OK) {
        return;
    }

    httpd_uri_t wifiGetUri = {.uri = "/api/v1/wifi",
                              .method = HTTP_GET,
                              .handler = HandleGetWifiInfo,
                              .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiGetUri);

    httpd_uri_t wifiPostUri = {.uri = "/api/v1/wifi",
                               .method = HTTP_POST,
                               .handler = HandleSetWifiInfo,
                               .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiPostUri);

    httpd_uri_t liftplanGetUri = {.uri = "/api/v1/liftplan",
                                  .method = HTTP_GET,
                                  .handler = HandleGetLiftplan,
                                  .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanGetUri);

    httpd_uri_t liftplanPostUri = {.uri = "/api/v1/liftplan",
                                   .method = HTTP_POST,
                                   .handler = HandleSetLiftplan,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanPostUri);

    httpd_uri_t liftplanDeleteUri = {.uri = "/api/v1/liftplan",
                                     .method = HTTP_DELETE,
                                     .handler = HandleDeleteLiftplan,
                                     .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanDeleteUri);

    httpd_uri_t commonGetUri = {.uri = "/*",
                                .method = HTTP_GET,
                                .handler = ResourceHandler,
                                .user_ctx = nullptr};
    httpd_register_uri_handler(server, &commonGetUri);
}

esp_err_t WebServer::ResourceHandler(httpd_req_t* req) {
    std::string filepath;
    std::string uri = req->uri;
    if (uri == "/") {
        filepath = "/littlefs/web_server/index.html";
        httpd_resp_set_type(req, "text/html");
    } else if (uri == "/style.css") {
        filepath = "/littlefs/web_server/style.css";
        httpd_resp_set_type(req, "text/css");
    } else if (uri == "/server.js") {
        filepath = "/littlefs/web_server/server.js";
        httpd_resp_set_type(req, "application/javascript");
    } else {
        filepath = req->uri;
    }
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        ESP_LOGE(kTag, "ResourceHandler - Failed to open file : %s",
                 filepath.c_str());
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to read existing file");
        return ESP_FAIL;
    }
    while (file.read(gScratch, sizeof(gScratch)) || file.gcount() > 0) {
        size_t bytes_read = file.gcount();
        // Send the buffer contents as HTTP response chunk
        if (httpd_resp_send_chunk(req, gScratch, bytes_read) != ESP_OK) {
            ESP_LOGE(kTag, "ResourceHandler - File sending failed!");
            // Abort sending file
            httpd_resp_sendstr_chunk(req, NULL);
            // Respond with 500 Internal Server Error
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to send file");
            return ESP_FAIL;
        }
    }
    ESP_LOGI(kTag, "ResourceHandler - Sending file '%s' completed",
             filepath.c_str());
    // Respond with an empty chunk to signal HTTP response completion
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t WebServer::HandleGetWifiInfo(httpd_req_t* req) {
    IHla* callback = static_cast<IHla*>(req->user_ctx);
    auto maybeWifiInfo = callback->OnGetWifiInfo();
    if (!maybeWifiInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get WifiInfo");
        return ESP_FAIL;
    }
    auto wifiInfo = maybeWifiInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "hostname", wifiInfo.GetHostname().c_str());
    cJSON_AddStringToObject(root, "SSID", wifiInfo.GetSSID().c_str());
    cJSON_AddStringToObject(root, "password", wifiInfo.GetPassword().c_str());
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::HandleSetWifiInfo(httpd_req_t* req) {
    IHla* callback = static_cast<IHla*>(req->user_ctx);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    cJSON* root = cJSON_Parse(gScratch);
    WifiInfo wifiInfo;
    wifiInfo.SetHostname(cJSON_GetObjectItem(root, "hostname")->valuestring);
    wifiInfo.SetSSID(cJSON_GetObjectItem(root, "SSID")->valuestring);
    wifiInfo.SetPassword(cJSON_GetObjectItem(root, "password")->valuestring);
    callback->OnSetWifiInfo(wifiInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

esp_err_t WebServer::HandleGetLiftplan(httpd_req_t* req) {
    IHla* callback = static_cast<IHla*>(req->user_ctx);
    char query[100];
    char name[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        if (httpd_query_key_value(query, "name", name, sizeof(name)) !=
            ESP_OK) {
            ESP_LOGW(kTag, "HandleGetLiftplan - No 'name' param");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                       "Missing 'name' param");
        }
        ESP_LOGD(kTag, "HandleGetLiftplan - Got name param: %s", name);
        auto liftplan = callback->OnGetLiftplan(name);
        if (liftplan.has_value()) {
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr(req, liftplan.value().c_str());
        } else {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Cannot find liftplan");
        }
    } else {
        auto liftplans = callback->OnGetLiftplans();
        httpd_resp_set_type(req, "application/json");
        cJSON* root = cJSON_CreateArray();
        for (const auto& liftplan : liftplans) {
            cJSON_AddItemToArray(root, cJSON_CreateString(liftplan.c_str()));
        }
        char* jsonStr = cJSON_Print(root);
        httpd_resp_sendstr(req, jsonStr);
        free(static_cast<void*>(jsonStr));
        cJSON_Delete(root);
    }
    return ESP_OK;
}

esp_err_t WebServer::HandleSetLiftplan(httpd_req_t* req) {
    IHla* callback = static_cast<IHla*>(req->user_ctx);
    char query[100];
    char name[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "No query params");
    }
    if (httpd_query_key_value(query, "name", name, sizeof(name)) != ESP_OK) {
        ESP_LOGW(kTag, "No 'name' param");
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "Missing 'name' param");
    }
    ESP_LOGD(kTag, "HandleSetLiftplan - Got name param: %s", name);
    int cur_len = 0;
    int received = 0;
    if (req->content_len >= sizeof(gScratch)) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < req->content_len) {
        received = httpd_req_recv(req, gScratch + cur_len, req->content_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    gScratch[req->content_len] = '\0';
    callback->OnSetLiftPlan(name, gScratch);
    return ESP_OK;
}

esp_err_t WebServer::HandleDeleteLiftplan(httpd_req_t* req) {
    IHla* callback = static_cast<IHla*>(req->user_ctx);
    char query[100];
    char name[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "No query params");
    }
    ESP_LOGD(kTag, "HandleDeleteLiftplan - Found URL query: %s", query);
    if (httpd_query_key_value(query, "name", name, sizeof(name)) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "No query params");
    }
    ESP_LOGD(kTag, "HandleDeleteLiftplan - Got name param: %s", name);
    callback->OnDeleteLiftPlan(name);
    return httpd_resp_sendstr(req, "File deleted successfully");
}