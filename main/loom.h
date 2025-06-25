#ifndef loom_h
#define loom_h

#include "button_handler.h"
#include "circular_deque.h"
#include "loom_iface.h"
#include "wifi_info.h"

namespace hla {

class Loom : public ILoom, public ButtonHandler {
  public:
    enum class State { idle, running, paused };

    Loom();
    ~Loom() = default;

    std::optional<WifiInfo> OnGetWifiInfo() const override;
    void OnSetWifiInfo(const WifiInfo& wifiInfo) override;
    std::vector<std::string> OnGetLiftplans() const override;
    std::optional<std::string>
    OnGetLiftplan(const std::string& fileName) override;
    bool OnSetLiftPlan(const std::string& fileName,
                       const std::string& data) override;
    bool OnDeleteLiftPlan(const std::string& fileName) override;
    bool OnStart(const std::string& liftplanFileName,
                 unsigned int startPosition) override;
    bool OnPause() override;
    bool OnStop() override;

  private:
    void onButtonPressed(gpio_num_t gpio) override;
    void ResetLiftplan();

    State mState;
    std::string mLiftplanName;
    CircularDeque<uint8_t> mLiftplan;
    CircularDeque<uint8_t>::Cursor mLiftplanCursor;
};
}   // namespace hla
#endif   // loom_h