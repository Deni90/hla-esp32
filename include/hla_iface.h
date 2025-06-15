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
};
}   // namespace hla
#endif   // hla_iface_h