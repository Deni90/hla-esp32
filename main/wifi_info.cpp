#include "wifi_info.h"

using hla::WifiInfo;

WifiInfo::WifiInfo(const std::string& hostname, const std::string& ssid,
                   const std::string& password)
    : mHostname(hostname), mSsid(ssid), mPassword(password) {}

std::string WifiInfo::GetHostname() const { return mHostname; }

void WifiInfo::SetHostname(const std::string& value) { mHostname = value; }

std::string WifiInfo::GetSSID() const { return mSsid; }

void WifiInfo::SetSSID(const std::string& value) { mSsid = value; }

std::string WifiInfo::GetPassword() const { return mPassword; }

void WifiInfo::SetPassword(const std::string& value) { mPassword = value; }