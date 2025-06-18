#ifndef hla_h
#define hla_h

#include "hla_iface.h"
#include "wifi_info.h"

namespace hla {

class Hla : public IHla {
  public:
    Hla() = default;
    ~Hla() = default;

    std::optional<WifiInfo> OnGetWifiInfo() const override;
    void OnSetWifiInfo(const WifiInfo& wifiInfo) override;
    std::vector<std::string> OnGetLiftplans() const override;
    std::optional<std::string>
    OnGetLiftplan(const std::string& fileName) override;
    bool OnSetLiftPlan(const std::string& fileName,
                       const std::string& data) override;
    bool OnDeleteLiftPlan(const std::string& fileName) override;

  private:
};
}   // namespace hla
#endif   // hla_h