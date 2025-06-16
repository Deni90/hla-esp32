#ifndef hla_h
#define hla_h

#include "hla_iface.h"
#include "wifi_info.h"

namespace hla {

class Hla : public IHla {
  public:
    Hla() = default;
    ~Hla() = default;

    /* Implementation of the IHla interface*/
    WifiInfo OnGetWifiInfo() const override;
    void OnSetWifiInfo(const WifiInfo& wifiInfo) override;
    JsonDocument OnGetLiftplans() const override;
    bool OnGetLiftplan(const String& name, JsonDocument& liftplan) override;
    bool OnSetLiftPlan(const String& fileName,
                       const JsonArray& content) override;
    bool OnDeleteLiftPlan(const String& fileName) override;

  private:
};
}   // namespace hla
#endif   // hla_h