#ifndef hla_iface_h
#define hla_iface_h

#include "wifi_info.h"

namespace hla {
class IHla {
  public:
    /**
     * @brief Default destructor
     *
     */
    virtual ~IHla() = default;

    /**
     * @brief Return wifi info
     *
     * @return WifiInfo object containing wifi info
     */
    virtual WifiInfo OnGetWifiInfo() const = 0;

    /**
     * @brief Set wifi info and save changes in config store
     *
     * @param wifiInfo wifi info
     */
    virtual void OnSetWifiInfo(const WifiInfo& wifiInfo) = 0;

    /**
     * @brief Get a list of available liftplan files
     * @return The JSON array containing liftplan files
     */
    virtual JsonDocument OnGetLiftplans() const = 0;

    /**
     * @brief Get a liftplan by name
     * @param[in] name File name
     * @param[out] liftplan The liftplan in JSON format
     * @return The whether the liftplan is returned successfully
     */
    virtual bool OnGetLiftplan(const String& name, JsonDocument& liftplan) = 0;

    /**
     * @brief Save liftplan to liftplan catalogue
     *
     * @param[in] fileName Name of the liftplan file
     * @param[in] content JSON array containing the actual liftplan
     * @return True if the plan is successfully saved. Otherwise return false
     */
    virtual bool OnSetLiftPlan(const String& fileName,
                               const JsonArray& content) = 0;

    /**
     * @brief Save liftplan to liftplan catalogue
     *
     * @param[in] fileName Name of the liftplan file
     * @return True, if the file is deleted.
     */
    virtual bool OnDeleteLiftPlan(const String& fileName) = 0;
};
}   // namespace hla
#endif   // hla_iface_h