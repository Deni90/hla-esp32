#include "hla.h"
#include "config_store.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Hla;
using hla::WifiInfo;

std::optional<WifiInfo> Hla::OnGetWifiInfo() const {
    return ConfigStore::LoadWifiInfo();
}

void Hla::OnSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::SaveWifiInfo(wifiInfo);
}

std::vector<std::string> Hla::OnGetLiftplans() const {
    return ConfigStore::ListLiftplanFiles();
}

std::optional<std::string> Hla::OnGetLiftplan(const std::string& fileName) {
    return ConfigStore::LoadLiftplan(fileName);
}

bool Hla::OnSetLiftPlan(const std::string& fileName, const std::string& data) {
    return ConfigStore::SaveLiftPlan(fileName, data);
}

bool Hla::OnDeleteLiftPlan(const std::string& fileName) {
    return ConfigStore::DeleteLiftPlan(fileName);
}