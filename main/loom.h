#ifndef loom_h
#define loom_h

#include <optional>

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

    std::optional<WifiInfo> onGetWifiInfo() const override;
    void onSetWifiInfo(const WifiInfo& wifiInfo) override;
    std::vector<std::string> onGetLiftplans() const override;
    std::optional<std::string>
    onGetLiftplan(const std::string& fileName) override;
    bool onSetLiftPlan(const std::string& fileName,
                       const std::string& data) override;
    bool onDeleteLiftPlan(const std::string& fileName) override;
    bool onStart(const std::string& liftplanFileName,
                 unsigned int startPosition) override;
    bool onPause() override;
    bool onContinue() override;
    bool onStop() override;
    std::string onGetLoomState() const override;
    std::optional<unsigned int> onGetActiveLiftplanIndex() const override;

  private:
    void onButtonPressed(gpio_num_t gpio) override;
    void resetLiftplan();

    State mState;
    std::string mLiftplanName;
    CircularDeque<uint8_t> mLiftplan;
    CircularDeque<uint8_t>::Cursor mLiftplanCursor;
    std::optional<unsigned int> mLiftplanIndex;
};
}   // namespace hla
#endif   // loom_h