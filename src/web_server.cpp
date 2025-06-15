#include "AsyncJson.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "web_server.h"
#include "wifi_info.h"

static constexpr int kHttp200Ok = 200;
static constexpr int kHttp400BadRequest = 400;
static constexpr int kHttp500InternalServerError = 500;

using hla::IHla;
using hla::WebServer;
using hla::WifiInfo;

WebServer::WebServer(int port, IHla& callback)
    : mServer(port), mCallback(callback) {}

void WebServer::Initialize() {
    mServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/index.html");
    });
    mServer.onNotFound([](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/index.html");
    });
    mServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/style.css", "text/css");
    });
    mServer.on("/server.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/web_server/server.js", "text/javascript");
    });
    mServer.on("/wifi", HTTP_GET, [&](AsyncWebServerRequest* request) {
        this->HandleGetWifiInfo(request);
    });
    mServer.addHandler(new AsyncCallbackJsonWebHandler(
        "/wifi", [&](AsyncWebServerRequest* request, JsonVariant& json) {
            this->HandleSetWifiInfo(request, json);
        }));
    mServer.begin();
}

void WebServer::HandleGetWifiInfo(AsyncWebServerRequest* request) {
    if (!request) {
        return;
    }
    WifiInfo wi = mCallback.OnGetWifiInfo();
    String messageBuffer;
    serializeJson(wi.ToJson(), messageBuffer);
    request->send(kHttp200Ok, "application/json", messageBuffer);
}

void WebServer::HandleSetWifiInfo(AsyncWebServerRequest* request,
                                  JsonVariant& json) {
    if (!request) {
        return;
    }
    if (!json.is<JsonObject>()) {
        request->send(kHttp400BadRequest,
                      "Error HandleSetWifiInfo: expecting a JSON object");
        return;
    }
    JsonDocument requestBody = json.as<JsonObject>();
    if (!requestBody.containsKey("hostname") ||
        !requestBody.containsKey("SSID") ||
        !requestBody.containsKey("password")) {
        request->send(kHttp400BadRequest,
                      "Error HandleSetWifiInfo: missing argument(s)!");
        return;
    }
    WifiInfo wi(requestBody["hostname"], requestBody["SSID"],
                requestBody["password"]);
    request->send(kHttp200Ok, "");
    mCallback.OnSetWifiInfo(wi);
}