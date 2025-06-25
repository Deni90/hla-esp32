#ifndef button_handler_h
#define button_handler_h

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#include <vector>

namespace hla {
/**
 * @brief Button handler class
 * Button handler is a class used for managing buttons. Gpio pin state and
 * debouncing are implemented with polling. This handler is processing only one
 * button at a time. While one button is pressed others are ignored.
 */
class ButtonHandler {
  public:
    /**
     * @brief Constructor
     * @param[in] buttonPins Vector of button gpio pins
     */
    ButtonHandler(const std::vector<gpio_num_t>& buttonPins);

  protected:
    /**
     * @brief Function called when when a button is pressed
     * @param[in] gpio gpio pin of the pressed button
     *
     */
    virtual void onButtonPressed(gpio_num_t gpio);

    /**
     * @brief Function called when when a button is released
     * @param[in] gpio gpio pin of the released button
     */
    virtual void onButtonReleased(gpio_num_t gpio);

  private:
    struct Button {
        gpio_num_t gpio;
        int lastState = 1;
        int stableState = 1;
        TickType_t lastChangeTime = 0;
    };

    void loop();
    static void taskLoop(void* param);

    std::vector<Button> mButtons;
    int mActiveButtonIndex;

};
}   // namespace hla

#endif   // button_handler_h
