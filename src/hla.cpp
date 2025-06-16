#include "hla.h"
#include "config_store.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Hla;
using hla::WifiInfo;

WifiInfo Hla::OnGetWifiInfo() const {
    WifiInfo wi;
    ConfigStore::LoadWifiInfo(wi);
    return wi;
}

void Hla::OnSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::SaveWifiInfo(wifiInfo);
    ESP.restart();
}

JsonDocument Hla::OnGetLiftplans() const {
    return ConfigStore::ListLiftplanFiles();
}

bool Hla::OnGetLiftplan(const String& name, JsonDocument& liftplan) {
    return ConfigStore::LoadLiftplan(name, liftplan);
}

bool Hla::OnSetLiftPlan(const String& fileName, const JsonArray& content) {
    return ConfigStore::SaveLiftPlan(fileName, content);
}

bool Hla::OnDeleteLiftPlan(const String& fileName) {
    return ConfigStore::DeleteLiftPlan(fileName);
}