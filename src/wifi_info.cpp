#include "wifi_info.h"

using hla::WifiInfo;

WifiInfo::WifiInfo(const String& hostname, const String& ssid,
                   const String& password)
    : mHostname(hostname), mSsid(ssid), mPassword(password) {}

String WifiInfo::GetHostname() const { return mHostname; }

void WifiInfo::SetHostname(const String& value) { mHostname = value; }

String WifiInfo::GetSSID() const { return mSsid; }

void WifiInfo::SetSSID(const String& value) { mSsid = value; }

String WifiInfo::GetPassword() const { return mPassword; }

void WifiInfo::SetPassword(const String& value) { mPassword = value; }

JsonDocument WifiInfo::ToJson() const {
    JsonDocument doc;
    doc["hostname"] = mHostname;
    doc["SSID"] = mSsid;
    doc["password"] = mPassword;
    return doc;
}