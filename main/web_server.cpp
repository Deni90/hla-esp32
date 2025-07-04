#include <fstream>
#include <iostream>

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"

#include "web_server.h"
#include "wifi_info.h"

using hla::ILoom;
using hla::WebServer;
using hla::WifiInfo;

static const char* kTag = "web_server";
static char gScratch[10240];

WebServer::WebServer(ILoom& callback) : mCallback(callback) {}

void WebServer::initialize() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 12;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) != ESP_OK) {
        return;
    }

    httpd_uri_t wifiGetUri = {.uri = "/api/v1/wifi",
                              .method = HTTP_GET,
                              .handler = handleGetWifiInfo,
                              .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiGetUri);

    httpd_uri_t wifiPostUri = {.uri = "/api/v1/wifi",
                               .method = HTTP_POST,
                               .handler = handleSetWifiInfo,
                               .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &wifiPostUri);

    httpd_uri_t liftplanGetUri = {.uri = "/api/v1/liftplan",
                                  .method = HTTP_GET,
                                  .handler = handleGetLiftplan,
                                  .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanGetUri);

    httpd_uri_t liftplanPostUri = {.uri = "/api/v1/liftplan",
                                   .method = HTTP_POST,
                                   .handler = handleSetLiftplan,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanPostUri);

    httpd_uri_t liftplanDeleteUri = {.uri = "/api/v1/liftplan",
                                     .method = HTTP_DELETE,
                                     .handler = handleDeleteLiftplan,
                                     .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &liftplanDeleteUri);

    httpd_uri_t loomGetUri = {.uri = "/api/v1/loom",
                              .method = HTTP_GET,
                              .handler = handleGetLoomStatus,
                              .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomGetUri);

    httpd_uri_t loomStartPostUri = {.uri = "/api/v1/loom/start",
                                    .method = HTTP_POST,
                                    .handler = handleStartLoom,
                                    .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomStartPostUri);

    httpd_uri_t loomPausePostUri = {.uri = "/api/v1/loom/pause",
                                    .method = HTTP_POST,
                                    .handler = handlePauseLoom,
                                    .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomPausePostUri);

    httpd_uri_t loomContinuePostUri = {.uri = "/api/v1/loom/continue",
                                       .method = HTTP_POST,
                                       .handler = handleContinueLoom,
                                       .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomContinuePostUri);

    httpd_uri_t loomStopPostUri = {.uri = "/api/v1/loom/stop",
                                   .method = HTTP_POST,
                                   .handler = handleStopLoom,
                                   .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomStopPostUri);

    httpd_uri_t loomLiftplanIndexPostUri = {.uri =
                                                "/api/v1/loom/liftplan_index",
                                            .method = HTTP_GET,
                                            .handler = handleLoomLiftplanIndex,
                                            .user_ctx = &mCallback};
    httpd_register_uri_handler(server, &loomLiftplanIndexPostUri);

    httpd_uri_t commonGetUri = {.uri = "/*",
                                .method = HTTP_GET,
                                .handler = resourcehandler,
                                .user_ctx = nullptr};
    httpd_register_uri_handler(server, &commonGetUri);
}

esp_err_t WebServer::resourcehandler(httpd_req_t* req) {
    std::string filepath;
    std::string uri = req->uri;
    if (uri == "/") {
        filepath = "/littlefs/frontend/index.html";
        httpd_resp_set_type(req, "text/html");
    } else if (uri == "/style.css") {
        filepath = "/littlefs/frontend/style.css";
        httpd_resp_set_type(req, "text/css");
    } else if (uri == "/server.js") {
        filepath = "/littlefs/frontend/server.js";
        httpd_resp_set_type(req, "application/javascript");
    } else {
        filepath = "/littlefs/frontend/index.html";
        httpd_resp_set_type(req, "text/html");
    }
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        ESP_LOGE(kTag, "Resourcehandler - Failed to open file : %s",
                 filepath.c_str());
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to read existing file");
        return ESP_FAIL;
    }
    while (file.read(gScratch, sizeof(gScratch)) || file.gcount() > 0) {
        size_t bytes_read = file.gcount();
        // Send the buffer contents as HTTP response chunk
        if (httpd_resp_send_chunk(req, gScratch, bytes_read) != ESP_OK) {
            ESP_LOGE(kTag, "Resourcehandler - File sending failed!");
            // Abort sending file
            httpd_resp_sendstr_chunk(req, NULL);
            // Respond with 500 Internal Server Error
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to send file");
            return ESP_FAIL;
        }
    }
    ESP_LOGI(kTag, "Resourcehandler - Sending file '%s' completed",
             filepath.c_str());
    // Respond with an empty chunk to signal HTTP response completion
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t WebServer::handleGetWifiInfo(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    auto maybeWifiInfo = callback->onGetWifiInfo();
    if (!maybeWifiInfo.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get WifiInfo");
        return ESP_FAIL;
    }
    auto wifiInfo = maybeWifiInfo.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "hostname", wifiInfo.getHostname().c_str());
    cJSON_AddStringToObject(root, "SSID", wifiInfo.getSSID().c_str());
    cJSON_AddStringToObject(root, "password", wifiInfo.getPassword().c_str());
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleSetWifiInfo(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
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
    wifiInfo.setHostname(cJSON_GetObjectItem(root, "hostname")->valuestring);
    wifiInfo.setSSID(cJSON_GetObjectItem(root, "SSID")->valuestring);
    wifiInfo.setPassword(cJSON_GetObjectItem(root, "password")->valuestring);
    callback->onSetWifiInfo(wifiInfo);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

esp_err_t WebServer::handleGetLiftplan(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    char query[100];
    char name[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        if (httpd_query_key_value(query, "name", name, sizeof(name)) !=
            ESP_OK) {
            ESP_LOGW(kTag, "handleGetLiftplan - No 'name' param");
            return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                       "Missing 'name' param");
        }
        ESP_LOGD(kTag, "handleGetLiftplan - Got name param: %s", name);
        auto liftplan = callback->onGetLiftplan(name);
        if (liftplan.has_value()) {
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr(req, liftplan.value().c_str());
        } else {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Cannot find liftplan");
        }
    } else {
        auto liftplans = callback->onGetLiftplans();
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

esp_err_t WebServer::handleSetLiftplan(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
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
    ESP_LOGD(kTag, "handleSetLiftplan - Got name param: %s", name);
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
    callback->onSetLiftPlan(name, gScratch);
    return ESP_OK;
}

esp_err_t WebServer::handleDeleteLiftplan(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    char query[100];
    char name[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "No query params");
    }
    ESP_LOGD(kTag, "handleDeleteLiftplan - Found URL query: %s", query);
    if (httpd_query_key_value(query, "name", name, sizeof(name)) != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "No query params");
    }
    ESP_LOGD(kTag, "handleDeleteLiftplan - Got name param: %s", name);
    callback->onDeleteLiftPlan(name);
    return httpd_resp_sendstr(req, "File deleted successfully");
}

esp_err_t WebServer::handleGetLoomStatus(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    std::string loomState = callback->onGetLoomState();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to allocate json");
        return ESP_FAIL;
    }
    cJSON_AddStringToObject(root, "loom_state", loomState.c_str());
    auto maybeLiftplanName = callback->onGetActiveLiftplanName();
    if (maybeLiftplanName.has_value()) {
        cJSON_AddStringToObject(root, "active_liftplan",
                                maybeLiftplanName.value().c_str());
    }

    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleStartLoom(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    // parse request
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
    cJSON* request = cJSON_Parse(gScratch);
    // set state
    bool result = callback->onStart(
        cJSON_GetObjectItem(request, "liftplan")->valuestring,
        cJSON_GetObjectItem(request, "start_position")->valueint);
    cJSON_Delete(request);
    // create response
    httpd_resp_set_type(req, "application/json");
    cJSON* response = cJSON_CreateObject();
    if (!response) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to allocate json");
        return ESP_FAIL;
    }
    cJSON_AddBoolToObject(response, "status", result);
    char* jsonStr = cJSON_Print(response);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(response);
    return ESP_OK;
}

esp_err_t WebServer::handlePauseLoom(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    bool result = callback->onPause();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to allocate json");
        return ESP_FAIL;
    }
    cJSON_AddBoolToObject(root, "status", result);
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleContinueLoom(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    bool result = callback->onContinue();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to allocate json");
        return ESP_FAIL;
    }
    cJSON_AddBoolToObject(root, "status", result);
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleStopLoom(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    bool result = callback->onStop();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to allocate json");
        return ESP_FAIL;
    }
    cJSON_AddBoolToObject(root, "status", result);
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t WebServer::handleLoomLiftplanIndex(httpd_req_t* req) {
    ILoom* callback = static_cast<ILoom*>(req->user_ctx);
    auto maybeLiftplanIndex = callback->onGetActiveLiftplanIndex();
    if (!maybeLiftplanIndex.has_value()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to get active liftplan index");
        return ESP_FAIL;
    }
    auto liftplanIndex = maybeLiftplanIndex.value();
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "index", liftplanIndex);
    char* jsonStr = cJSON_Print(root);
    httpd_resp_sendstr(req, jsonStr);
    free(static_cast<void*>(jsonStr));
    cJSON_Delete(root);
    return ESP_OK;
}