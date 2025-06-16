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

    /**
     * @brief Load liftplan
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @param[out] liftplan Liftplan
     * @return Result of the retrieval
     */
    static bool LoadLiftplan(const String& fileName, JsonDocument& liftplan);

    /**
     * @brief Return a list of available liftplan files
     *
     * @return JSON aray
     */
    static JsonDocument ListLiftplanFiles();

    /**
     * @brief Save a liftplan file to file system
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @param[in] data A JSON array
     * @return True, if the file is saved. False, if the file with a given name
     * already exists or other error...
     */
    static bool SaveLiftPlan(const String& fileName, const JsonArray& data);

    /**
     * @brief Delete a liftplan file
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @return True, if the file is deleted.
     */
    static bool DeleteLiftPlan(const String& fileName);
};
}   // namespace hla
#endif   // config_store_h