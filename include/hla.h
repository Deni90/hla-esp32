#ifndef hla_h
#define hla_h

#include "hla_iface.h"
#include "wifi_info.h"

namespace hla {

class Hla : public IHla {
  public:
    Hla() = default;
    ~Hla() = default;

    /**
     * @brief Return wifi info
     *
     * @return WifiInfo object containing wifi info
     */
    WifiInfo OnGetWifiInfo() const;

    /**
     * @brief Set wifi info and save changes in config store
     *
     * @param wifiInfo wifi info
     */
    void OnSetWifiInfo(const WifiInfo& wifiInfo);

  private:
};
}   // namespace hla
#endif   // hla_h