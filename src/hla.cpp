#include "config_store.h"
#include "hla.h"
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