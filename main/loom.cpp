#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"

#include "config_store.h"
#include "loom.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Loom;
using hla::WifiInfo;

#define NEXT_BUTTON GPIO_NUM_22
#define PREV_BUTTON GPIO_NUM_23

static const char* kTag = "loom";

Loom::Loom()
    : ButtonHandler({NEXT_BUTTON, PREV_BUTTON}), mState(State::idle),
      mLiftplanName(""), mLiftplanCursor(nullptr), mLiftplanIndex(-1) {}

std::optional<WifiInfo> Loom::onGetWifiInfo() const {
    return ConfigStore::loadWifiInfo();
}

void Loom::onSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::saveWifiInfo(wifiInfo);
}

std::vector<std::string> Loom::onGetLiftplans() const {
    return ConfigStore::listLiftplanFiles();
}

std::optional<std::string> Loom::onGetLiftplan(const std::string& fileName) {
    return ConfigStore::loadLiftplan(fileName);
}

bool Loom::onSetLiftPlan(const std::string& fileName, const std::string& data) {
    return ConfigStore::saveLiftPlan(fileName, data);
}

bool Loom::onDeleteLiftPlan(const std::string& fileName) {
    return ConfigStore::deleteLiftPlan(fileName);
}

bool Loom::onStart(const std::string& liftplanFileName,
                   unsigned int startPosition) {
    // it is only possible to switch to running from idle state
    if (mState != State::idle) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Not in 'idle' state.");
        return false;
    }
    // load the raw liftplan from file
    auto maybeLiftplan = ConfigStore::loadLiftplan(liftplanFileName);
    if (!maybeLiftplan.has_value()) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Liftplan not found.");
        return false;
    }
    auto liftplan = maybeLiftplan.value();
    // parse the liftplan - and load it
    if (mLiftplan.length()) {
        resetLiftplan();
    }
    cJSON* root = cJSON_Parse(liftplan.c_str());
    if (!root || !cJSON_IsArray(root)) {
        ESP_LOGW(
            kTag,
            "Failed to switch to 'running' state. Failed to parse liftplan.");
        cJSON_Delete(root);
        return false;
    }
    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; i++) {
        cJSON* item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsString(item)) {
            ESP_LOGW(kTag, "Failed to switch to 'running' state. Failed to "
                           "parse liftplan.");
            cJSON_Delete(root);
            return false;
        }
        uint8_t value = (uint8_t) strtoul(item->valuestring, nullptr, 16);
        mLiftplan.pushBack(value);
    }
    cJSON_Delete(root);
    // setup the cursor
    mLiftplanCursor = mLiftplan.frontCursor();
    for (unsigned int i = 0; i < startPosition; ++i) {
        mLiftplanCursor = mLiftplanCursor.next();
    }
    mLiftplanIndex = startPosition % mLiftplan.length();
    // TODO move shafts to match the first element from the liftplan
    ESP_LOGI(kTag, "Move shatfs to 0x%02x", mLiftplanCursor.value());

    ESP_LOGD(kTag, "Switching to 'running' state.");
    mState = State::running;
    return true;
}

bool Loom::onPause() {
    if (mState != State::running) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'pause' state. Not in 'running' state.");
        return false;
    }
    ESP_LOGD(kTag, "Switching to 'paused' state.");
    mState = State::paused;
    return true;
}

bool Loom::onContinue() {
    if (mState != State::paused) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Not in 'paused' state.");
        return false;
    }
    ESP_LOGD(kTag, "Switching to 'running' state.");
    mState = State::running;
    return true;
}

bool Loom::onStop() {
    // if in "idle" there is nothing to do
    if (mState == State::idle) {
        return true;
    }
    // TODO raise all shatfs back to idle position
    // clear the liftplan buffer, ...
    resetLiftplan();
    // switch back to idle state
    ESP_LOGD(kTag, "Switching to 'idle' state.");
    mState = State::idle;
    return true;
}

std::string Loom::onGetLoomState() const {
    switch (mState) {
    case State::idle:
        return "idle";
    case State::paused:
        return "paused";
    case State::running:
        return "running";
    }
    return "";
}

void Loom::onButtonPressed(gpio_num_t gpio) {
    ESP_LOGD(kTag, "Pressed GPIO %d", gpio);
    if (mState != State::running || !mLiftplanCursor.isValid()) {
        return;
    }
    if (gpio == NEXT_BUTTON) {
        mLiftplanCursor = mLiftplanCursor.next();
        ++mLiftplanIndex;
        mLiftplanIndex %= mLiftplan.length();
    } else if (gpio == PREV_BUTTON) {
        mLiftplanCursor = mLiftplanCursor.prev();
        if (mLiftplanIndex > 0) {
            --mLiftplanIndex;
        } else {
            mLiftplanIndex = mLiftplan.length() - 1;
        }
    } else {
        return;
    }
    // TODO implement me
    ESP_LOGI(kTag, "Move shatfs to 0x%02x", mLiftplanCursor.value());
}

void Loom::resetLiftplan() {
    mLiftplanName.clear();
    mLiftplan.empty();
    mLiftplanCursor.reset();
}