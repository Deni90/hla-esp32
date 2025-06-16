#include "config_store.h"

#include <inttypes.h>

#include <ArduinoJson.h>
#include <LittleFS.h>

using hla::ConfigStore;
using hla::WifiInfo;

static constexpr const char* kWifiInfoFile = "/config/wifi_info.json";
static constexpr const char* kLiftplanDir = "/liftplans";

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

bool ConfigStore::LoadLiftplan(const String& fileName, JsonDocument& liftplan) {
    String liftplanFile = String(kLiftplanDir) + "/" + String(fileName);
    File file = LittleFS.open(liftplanFile, "r");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", liftplanFile);
        return false;
    }
    JsonDocument doc;
    if (deserializeJson(doc, file)) {
        Serial.printf("Failed to read %s file.\n", kWifiInfoFile);
        return false;
    }
    file.close();
    liftplan = doc;
    return true;
}

JsonDocument ConfigStore::ListLiftplanFiles() {
    File dir = LittleFS.open(kLiftplanDir);
    if (!dir || !dir.isDirectory()) {
        return JsonDocument();
    }
    JsonDocument doc;
    JsonArray fileList = doc.to<JsonArray>();
    File file = dir.openNextFile();
    while (file) {
        fileList.add(String(file.name()));
        file = dir.openNextFile();
    }
    return doc;
}

bool ConfigStore::SaveLiftPlan(const String& fileName, const JsonArray& data) {
    String liftplanFile = String(kLiftplanDir) + "/" + String(fileName);
    if (!LittleFS.exists(kLiftplanDir)) {
        LittleFS.mkdir(kLiftplanDir);
    }

    if (LittleFS.exists(liftplanFile)) {
        Serial.println("File already exists");
        return false;
    }
    File file = LittleFS.open(liftplanFile, "w");
    if (!file) {
        Serial.printf("Failed to open %s file.\n", liftplanFile);
        return false;
    }
    String output;
    serializeJson(data, output);
    file.print(output);
    file.close();
    return true;
}

bool ConfigStore::DeleteLiftPlan(const String& fileName) {
    String liftplanFile = String(kLiftplanDir) + "/" + String(fileName);
    if (!LittleFS.exists(liftplanFile)) {
        Serial.println("File doesn't exist");
        return false;
    }
    return LittleFS.remove(liftplanFile);
}