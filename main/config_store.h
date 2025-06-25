#ifndef config_store_h
#define config_store_h

#include <optional>
#include <vector>

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
     * @param return WifiInfo if the file is successfully read
     */
    static std::optional<WifiInfo> loadWifiInfo();

    /**
     * @brief Save wifi info
     *
     * @param wifiInfo wifi info
     */
    static void saveWifiInfo(const WifiInfo& wifiInfo);

    /**
     * @brief Return a list of available liftplan files
     *
     * @return A vector containing liftplan filenames relative to liftplan dir
     */
    static std::vector<std::string> listLiftplanFiles();

    /**
     * @brief Load liftplan
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @param[out] liftplan Liftplan
     * @return Content of the liftplan if successfully read
     */
    static std::optional<std::string> loadLiftplan(const std::string& fileName);

    /**
     * @brief Save a liftplan file to file system
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @param[in] data A JSON array
     * @return True, if the file is saved. False, if the file with a given name
     * already exists or other error...
     */
    static bool saveLiftPlan(const std::string& fileName,
                             const std::string& data);

    /**
     * @brief Delete a liftplan file
     *
     * @param[in] fileName Name of the file. It should have a .json extension
     * @return True, if the file is deleted.
     */
    static bool deleteLiftPlan(const std::string& fileName);
};
}   // namespace hla
#endif   // config_store_h