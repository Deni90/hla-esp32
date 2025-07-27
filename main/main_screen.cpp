#include <algorithm>

#include "main_screen.h"

using hla::MainScreen;
using hla::Screen;

MainScreen::MainScreen(uint16_t width, uint16_t height)
    : Screen(width, height) {}

uint8_t* MainScreen::build() {
    clear();
    std::string foo = "foo";
    int16_t y = 0;
    printString(0, y, "wifi: " + mWifiSsid);
    y += 8;
    printString(0, y, "url: " + mUrl);
    y += 8;
    std::string state = hla::loomStateToString(mLoomInfo.state);
    transform(state.begin(), state.end(), state.begin(), ::toupper);
    printString(0, y, "state: " + state);
    y += 8;
    if (mLoomInfo.state != LoomState::idle) {
        if (mLoomInfo.liftplanName.has_value()) {
            const std::string subStringToRemove = ".json";
            std::string liftplanName = mLoomInfo.liftplanName.value();
            auto pos = liftplanName.find(subStringToRemove);
            if (pos != std::string::npos) {
                liftplanName.erase(pos, subStringToRemove.length());
            }
            printString(0, y, "liftplan: " + liftplanName);
            y += 8;
        }
        if (mLoomInfo.liftplanIndex.has_value()) {
            printString(
                0, y,
                "step: " + std::to_string(mLoomInfo.liftplanIndex.value() + 1));
            y += 8;
        }
        const uint8_t x = 21;
        printLoomPosition(x, y + 1, mPrevLoomPosition);
        y += 8;
        printString(0, y, "  -->");
        printLoomPosition(x, y + 1, mCurLoomPosition);
        y += 8;
        printLoomPosition(x, y + 1, mNextLoomPosition);
    }

    return mFrameBuffer;
}

MainScreen& MainScreen::setWifiSsid(const std::string& value) {
    mWifiSsid = value;
    return *this;
}

MainScreen& MainScreen::setUrl(const std::string& value) {
    mUrl = value;
    return *this;
}

MainScreen& MainScreen::setLoomInfo(const LoomInfo& value) {
    mLoomInfo = value;
    return *this;
}

MainScreen& MainScreen::setLoomPosition(uint8_t prev, uint8_t cur,
                                        uint8_t next) {
    mPrevLoomPosition = prev;
    mCurLoomPosition = cur;
    mNextLoomPosition = next;
    return *this;
}

void MainScreen::printLoomPosition(uint16_t x, uint16_t y, uint8_t value) {
    for (uint8_t i = 0; i < 8; ++i) {
        bool fill = value & (1 << (7 - i));
        drawRectangle(x, y, 6, 6, fill);
        x += 8;
    }
}