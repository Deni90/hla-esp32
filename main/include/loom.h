#ifndef loom_h
#define loom_h

#include <optional>

#include "esp_event.h"   //for wifi event
#include "sh1106.h"

#include "button_handler.h"
#include "circular_deque.h"
#include "loom_iface.h"
#include "loom_info.h"
#include "main_screen.h"
#include "web_server.h"
#include "wifi_info.h"

namespace hla {

class Loom : public ILoom, public ButtonHandler {
  public:
    Loom();
    ~Loom() = default;

    void initialize();

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
    std::optional<std::string> onGetActiveLiftplanName() const override;

  private:
    bool setupLittlefs();
    void setupWifi(const WifiInfo& wifiInfo);
    bool initializeWifiInStationMode(const WifiInfo& wifiInfo);
    void initializeWifiInApMode(const WifiInfo& wifiInfo);
    void setupCaptivePortal();
    void startMdnsService(const WifiInfo& wifiInfo);
    void onButtonPressed(gpio_num_t gpio) override;
    void resetLiftplan();
    bool loadLiftplan(const std::string& liftplanFileName,
                      unsigned int startPosition);

    Sh1106 mOled;
    WebServer mWebServer;
    LoomInfo mLoomInfo;
    CircularDeque<uint8_t> mLiftplan;
    CircularDeque<uint8_t>::Cursor mLiftplanCursor;
    MainScreen mMainScreen;
};
}   // namespace hla
#endif   // loom_h