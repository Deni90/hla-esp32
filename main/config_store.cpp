#include <filesystem>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <sstream>

#include "config_store.h"

using hla::ConfigStore;
using hla::WifiInfo;

static constexpr const char* kWifiInfoFile = "/littlefs/config/wifi_info.json";
static constexpr const char* kLiftplanDir = "/littlefs/liftplans";

std::optional<WifiInfo> ConfigStore::loadWifiInfo() {
    // check if file exists on kWifiInfoFile path
    if (!std::filesystem::exists(kWifiInfoFile)) {
        return std::nullopt;
    }
    // read and parse JSON file
    std::ifstream wifiInfoFile(kWifiInfoFile);
    std::stringstream buffer;
    buffer << wifiInfoFile.rdbuf();
    cJSON* json = cJSON_Parse(buffer.str().c_str());
    // populate WifiInfo object
    WifiInfo wi;
    wi.setHostname(
        cJSON_GetObjectItemCaseSensitive(json, "hostname")->valuestring);
    wi.setSSID(cJSON_GetObjectItemCaseSensitive(json, "SSID")->valuestring);
    wi.setPassword(
        cJSON_GetObjectItemCaseSensitive(json, "password")->valuestring);
    cJSON_Delete(json);
    return wi;
}

void ConfigStore::saveWifiInfo(const WifiInfo& wifiInfo) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "hostname",
                          cJSON_CreateString(wifiInfo.getHostname().c_str()));
    cJSON_AddItemToObject(json, "SSID",
                          cJSON_CreateString(wifiInfo.getSSID().c_str()));
    cJSON_AddItemToObject(json, "password",
                          cJSON_CreateString(wifiInfo.getPassword().c_str()));
    char* jsonString = cJSON_Print(json);
    if (jsonString != nullptr) {
        std::ofstream wifiInfoFile(kWifiInfoFile);
        wifiInfoFile << jsonString;
        cJSON_free(jsonString);
    }
    cJSON_Delete(json);
}

std::vector<std::string> ConfigStore::listLiftplanFiles() {
    if (!std::filesystem::exists(kLiftplanDir)) {
        return {};
    }
    std::filesystem::path liftplanDir = kLiftplanDir;
    std::vector<std::string> result;
    for (const auto& entry : std::filesystem::directory_iterator(liftplanDir)) {
        result.emplace_back(entry.path().filename());
    }
    return result;
}

std::optional<std::string>
ConfigStore::loadLiftplan(const std::string& fileName) {
    std::filesystem::path liftplanFilePath =
        std::filesystem::path(kLiftplanDir) / std::filesystem::path(fileName);
    // check if file exists on kWifiInfoFile path
    if (!std::filesystem::exists(liftplanFilePath)) {
        return std::nullopt;
    }
    // read JSON file
    std::ifstream liftplanFile(liftplanFilePath);
    std::stringstream buffer;
    buffer << liftplanFile.rdbuf();
    return buffer.str();
}

bool ConfigStore::saveLiftPlan(const std::string& fileName,
                               const std::string& data) {
    std::filesystem::path liftplanFilePath =
        std::filesystem::path(kLiftplanDir) / std::filesystem::path(fileName);
    // check if file exists on kWifiInfoFile path
    if (std::filesystem::exists(liftplanFilePath)) {
        return false;
    }

    if (!std::filesystem::exists(kLiftplanDir)) {
        if (!std::filesystem::create_directory(kLiftplanDir)) {
            return false;
        }
    }

    std::ofstream liftplanFile(liftplanFilePath);
    liftplanFile << data;
    return true;
}

bool ConfigStore::deleteLiftPlan(const std::string& fileName) {
    std::filesystem::path liftplanFilePath =
        std::filesystem::path(kLiftplanDir) / std::filesystem::path(fileName);
    return remove(liftplanFilePath.c_str()) == 0;
}