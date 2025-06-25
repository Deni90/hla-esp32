#ifndef loom_iface_h
#define loom_iface_h

#include <optional>
#include <vector>

#include "wifi_info.h"

namespace hla {
class ILoom {
  public:
    /**
     * @brief Default destructor
     *
     */
    virtual ~ILoom() = default;

    /**
     * @brief Return wifi info
     *
     * @return WifiInfo object containing wifi info
     */
    virtual std::optional<WifiInfo> onGetWifiInfo() const = 0;

    /**
     * @brief Set wifi info and save changes in config store
     *
     * @param wifiInfo wifi info
     */
    virtual void onSetWifiInfo(const WifiInfo& wifiInfo) = 0;

    /**
     * @brief Get a list of available liftplan files
     * @return An array containing liftplan files
     */
    virtual std::vector<std::string> onGetLiftplans() const = 0;

    /**
     * @brief Get a liftplan by name
     * @param[in] name File name
     * @param[out] liftplan The liftplan in JSON format
     * @return The whether the liftplan is returned successfully
     */
    virtual std::optional<std::string>
    onGetLiftplan(const std::string& fileName) = 0;

    /**
     * @brief Save liftplan to liftplan catalogue
     *
     * @param[in] fileName Name of the liftplan file
     * @param[in] content JSON array containing the actual liftplan
     * @return True if the plan is successfully saved. Otherwise return false
     */
    virtual bool onSetLiftPlan(const std::string& fileName,
                               const std::string& data) = 0;

    /**
     * @brief Save liftplan to liftplan catalogue
     *
     * @param[in] fileName Name of the liftplan file
     * @return True, if the file is deleted.
     */
    virtual bool onDeleteLiftPlan(const std::string& fileName) = 0;

    /**
     * @brief Start loom
     * @param[in] liftplanFileName Liftplan name
     * @param[in] startPosition Start position
     * @return True if successfully switched to state, otherwise false
     */
    virtual bool onStart(const std::string& liftplanFileName,
                         unsigned int startPosition) = 0;

    /**
     * @brief Pause loom
     * @return True if successfully switched to state, otherwise false
     */
    virtual bool onPause() = 0;

    /**
     * @brief Stop loom
     * @return True if successfully switched to state, otherwise false
     */
    virtual bool onStop() = 0;
};
}   // namespace hla
#endif   // loom_iface_h