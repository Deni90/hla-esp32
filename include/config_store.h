#ifndef config_store_h
#define config_store_h

#include "wifi_info.h"

namespace hla {
/**
 * @brief Class used for reading and writing config parameters
 */
class ConfigStore {
  public:
    /**
     * @brief Load wifi info
     *
     * @param wifiInfo wifi info
     */
    static void LoadWifiInfo(WifiInfo& wifiInfo);

    /**
     * @brief Save wifi info
     *
     * @param wifiInfo wifi info
     */
    static void SaveWifiInfo(const WifiInfo& wifiInfo);
};
}   // namespace hla
#endif   // config_store_h