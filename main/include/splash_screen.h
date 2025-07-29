#ifndef splash_screen_h
#define splash_screen_h

#include "loom_info.h"
#include "screen.h"

namespace hla {
class SplashScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screeen
     */
    SplashScreen(uint16_t width, uint16_t height);

    /**
     * @brief Destructor
     */
    ~SplashScreen() = default;

    /**
     * @brief Build main screen based on data provided by user
     *
     * @return An array containing the main screen matching the screen
     * dimensions
     */
    virtual uint8_t* build() override;
};
}   // namespace hla

#endif   // splash_screen_h