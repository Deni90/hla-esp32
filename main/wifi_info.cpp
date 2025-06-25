#include "wifi_info.h"

using hla::WifiInfo;

WifiInfo::WifiInfo(const std::string& hostname, const std::string& ssid,
                   const std::string& password)
    : mHostname(hostname), mSsid(ssid), mPassword(password) {}

std::string WifiInfo::getHostname() const { return mHostname; }

void WifiInfo::setHostname(const std::string& value) { mHostname = value; }

std::string WifiInfo::getSSID() const { return mSsid; }

void WifiInfo::setSSID(const std::string& value) { mSsid = value; }

std::string WifiInfo::getPassword() const { return mPassword; }

void WifiInfo::setPassword(const std::string& value) { mPassword = value; }