#ifndef wifi_info_h
#define wifi_info_h

#include <inttypes.h>
#include <string>

#include "cJSON.h"

namespace hla {
/**
 * @brief Represents a Wifi info class
 *
 */
class WifiInfo {
  public:
    /**
     * @brief Default constructor
     */
    WifiInfo() = default;

    /**
     * @brief Construct a new Wifi Info object
     *
     * @param hostname hostname
     * @param ssid SSID
     * @param password password
     */
    WifiInfo(const std::string& hostname, const std::string& ssid,
             const std::string& password);

    /**
     * @brief Default destructor
     */
    ~WifiInfo() = default;

    /**
     * @brief Default copy constructor
     * @param other WifiInfo object
     */
    WifiInfo(const WifiInfo& other) = default;

    /**
     * @brief Default copy assignment constructor
     * @param other WifiInfo object
     */
    WifiInfo& operator=(const WifiInfo& other) = default;

    /**
     * @brief Getter for hostname
     *
     * @return hostname
     */
    std::string GetHostname() const;

    /**
     * @brief Setter for hostname
     *
     * @param value hostname
     */
    void SetHostname(const std::string& value);

    /**
     * @brief Getter for ssid
     *
     * @return SSID
     */
    std::string GetSSID() const;

    /**
     * @brief Setter for SSID
     *
     * @param value ssid
     */
    void SetSSID(const std::string& value);

    /**
     * @brief Getter for password
     *
     * @return base64 encoded password
     */
    std::string GetPassword() const;

    /**
     * @brief Setter for password
     *
     * @param value base64 encoded password
     */
    void SetPassword(const std::string& value);

  private:
    std::string mHostname;
    std::string mSsid;
    std::string mPassword;
};
}   // namespace hla
#endif   // wifi_info_h