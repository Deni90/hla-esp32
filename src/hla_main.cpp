#include <inttypes.h>

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <base64.hpp>

#include "config_store.h"
#include "hla.h"
#include "web_server.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Hla;
using hla::WebServer;
using hla::WifiInfo;

static constexpr unsigned long kUartBaudrate = 115200;
static constexpr uint32_t kTimerPeriod = 1;   // ms
static constexpr int kWebserverPort = 80;
static constexpr uint8_t kDnsPort = 53;
static constexpr const char* kDefaultApSsid = "HandloomController";

static Ticker gTimer;
static DNSServer gDnsServer;
static Hla gHla;
static WebServer gWebServer(kWebserverPort, gHla);
static uint32_t gClock = 0;
static bool gIsApMode = false;

/**
 * @brief Function that is called every millisecond
 */
static void HandleTimer() { gClock++; }

/**
 * @brief Initialize Wifi in Station Mode
 *
 * @return true if successfully connected to a network
 */
bool InitializeWifiInStationMode(const WifiInfo& wifiInfo) {
    int passwordLenght = decode_base64_length(
        reinterpret_cast<const unsigned char*>(wifiInfo.GetPassword().c_str()),
        wifiInfo.GetPassword().length());
    char password[passwordLenght + 1];
    decode_base64(
        reinterpret_cast<const unsigned char*>(wifiInfo.GetPassword().c_str()),
        wifiInfo.GetPassword().length(),
        reinterpret_cast<unsigned char*>(password));
    password[passwordLenght] = '\0';
    Serial.printf("Connecting to %s", wifiInfo.GetSSID().c_str());
    WiFi.setHostname(wifiInfo.GetHostname().c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiInfo.GetSSID(), password);
    uint8_t counter = 100;
    IPAddress myIP;
    while (WiFi.status() != WL_CONNECTED) {
        printf(".");
        delay(200);
        if (counter-- == 0) {
            break;
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(" Failed");
        WiFi.disconnect();
        return false;
    }
    Serial.println(" Done");
    return true;
}

/**
 * @brief Initialize Wifi in AP mode
 */
void InitializeWifiInApMode() {
    Serial.printf("Setting up soft AP ...");
    WiFi.softAP(kDefaultApSsid);
    gIsApMode = true;
    Serial.println("Done");
}

/**
 * @brief Initialize all the necessary modules
 */
void setup() {
    Serial.begin(kUartBaudrate);

    Serial.println("\nHandloom automation controller\n");

    Serial.print("Mounting LittleFS... ");
    if (!LittleFS.begin()) {
        Serial.println("Failed");
    } else {
        Serial.println("Done");
    }

    WifiInfo wi;
    ConfigStore::LoadWifiInfo(wi);
    if (wi.GetSSID() == "" || !InitializeWifiInStationMode(wi)) {
        InitializeWifiInApMode();
    }

    Serial.printf("Initializing timer... ");
    gTimer.attach_ms(kTimerPeriod, HandleTimer);
    Serial.println("Done");

    if (gIsApMode) {
        Serial.printf("Initializing captive portal... ");
        gDnsServer.start(kDnsPort, "*", WiFi.softAPIP());
    }

    Serial.printf("Initializing mDNS... ");
    MDNS.begin(wi.GetHostname());
    Serial.println("Done");

    Serial.print("Initializing Web server... ");
    gWebServer.Initialize();
    Serial.println("Done");

    Serial.printf("\nWifi SSID: %s\n", wi.GetSSID().c_str());
    Serial.printf("Hostname:  %s\n\n", wi.GetHostname().c_str());
}

void loop() {}