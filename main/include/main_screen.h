#ifndef main_screen_h
#define main_screen_h

#include "loom_info.h"
#include "screen.h"

namespace hla {
class MainScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screeen
     */
    MainScreen(uint16_t width, uint16_t height);

    /**
     * @brief Destructor
     */
    ~MainScreen() = default;

    /**
     * @brief Build main screen based on data provided by user
     *
     * @return An array containing the main screen matching the screen
     * dimensions
     */
    virtual uint8_t* build() override;

    /**
     * @brief Set Wifi SSID
     *
     * @param[in] value Wifi SSID
     */
    MainScreen& setWifiSsid(const std::string& value);

    /**
     * @brief Set Web portal URL
     *
     * @param[in] value URL
     */
    MainScreen& setUrl(const std::string& value);

    /**
     * @brief Set Loom info
     *
     * @param[in] value loom info
     */
    MainScreen& setLoomInfo(const LoomInfo& value);

    /** @brief Set loom position
     *
     * @param[in] prev Prevous loom position
     * @param[in] cur Current loom position
     * @param[in] next Next loom position
     *
     */
    MainScreen& setLoomPosition(uint8_t prev, uint8_t cur, uint8_t next);

  private:
    void printLoomPosition(uint16_t x, uint16_t y, uint8_t value);

    std::string mWifiSsid;
    std::string mUrl;
    LoomInfo mLoomInfo;
    uint8_t mPrevLoomPosition;
    uint8_t mCurLoomPosition;
    uint8_t mNextLoomPosition;
};
}   // namespace hla

#endif   // main_screen_h