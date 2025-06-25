#include "button_handler.h"

#include "esp_log.h"
#include "freertos/task.h"

static constexpr uint32_t kDebounceDelayMs = 50;
static const char* kTag = "button_handler";

using hla::ButtonHandler;

ButtonHandler::ButtonHandler(const std::vector<gpio_num_t>& buttonPins)
    : mActiveButtonIndex(-1) {
    for (const auto& pin : buttonPins) {
        gpio_config_t io_conf = {};
        io_conf.pin_bit_mask = 1ULL << pin;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io_conf);

        Button btn;
        btn.gpio = pin;
        mButtons.push_back(btn);
    }

    xTaskCreate(taskLoop, "button_handler_task", 2048, this, 10, nullptr);
}

void ButtonHandler::onButtonPressed(gpio_num_t gpio) {
    ESP_LOGD(kTag, "Pressed GPIO %d", gpio);
}

void ButtonHandler::onButtonReleased(gpio_num_t gpio) {
    ESP_LOGD(kTag, "Released GPIO %d", gpio);
}

void ButtonHandler::loop() {
    while (true) {
        TickType_t now = xTaskGetTickCount();
        for (size_t i = 0; i < mButtons.size(); ++i) {
            int level = gpio_get_level(mButtons[i].gpio);
            if (level != mButtons[i].lastState) {
                mButtons[i].lastChangeTime = now;
                mButtons[i].lastState = level;
            } else if ((now - mButtons[i].lastChangeTime) >=
                       pdMS_TO_TICKS(kDebounceDelayMs)) {
                if (level != mButtons[i].stableState) {
                    mButtons[i].stableState = level;

                    if (level == 0 && mActiveButtonIndex == -1) {
                        mActiveButtonIndex = i;
                        onButtonPressed(mButtons[i].gpio);
                    } else if (level == 1 && mActiveButtonIndex == (int) i) {
                        onButtonReleased(mButtons[i].gpio);
                        mActiveButtonIndex = -1;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void ButtonHandler::taskLoop(void* param) {
    ButtonHandler* self = static_cast<ButtonHandler*>(param);
    self->loop();
}