#include "config_store.h"

#include <inttypes.h>

#include <ArduinoJson.h>
#include <LittleFS.h>

using hla::ConfigStore;
using hla::WifiInfo;

static constexpr const char* kWifiInfoFile = "/config/wifi_info.json";

void ConfigStore::LoadWifiInfo(WifiInfo& wifiInfo) {
    File file = LittleFS.open(kWifiInfoFile, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", kWifiInfoFile);
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file))
        Serial.printf("Failed to read %s file.\n", kWifiInfoFile);
    file.close();
    wifiInfo.SetHostname(doc["hostname"]);
    wifiInfo.SetSSID(doc["SSID"]);
    wifiInfo.SetPassword(doc["password"]);
}

void ConfigStore::SaveWifiInfo(const WifiInfo& wifiInfo) {
    String messageBuffer;
    serializeJson(wifiInfo.ToJson(), messageBuffer);
    File file = LittleFS.open(kWifiInfoFile, "w");
    file.print(messageBuffer);
    file.close();
}