#include "ssd1306.h"

#include <cstring>

#include "freertos/FreeRTOS.h"

static constexpr uint8_t i2cTicksToWait = 100;

// SSD1306 commands
static constexpr uint8_t kI2cAddr = 0x3C;
static constexpr uint8_t kSetMemMode = 0x20;
static constexpr uint8_t kColAddr = 0x21;
static constexpr uint8_t kPageAddr = 0x22;
static constexpr uint8_t kHorizScroll = 0x26;
static constexpr uint8_t kSetScroll = 0x2E;

static constexpr uint8_t kDispStartLine = 0x40;

static constexpr uint8_t kSetContrast = 0x81;
static constexpr uint8_t kSetChargePump = 0x8D;

static constexpr uint8_t kSetSegRemap = 0xA0;
static constexpr uint8_t kSetEntireOn = 0xA4;
static constexpr uint8_t kSetAllOn = 0xA5;
static constexpr uint8_t kSetNormDisp = 0xA6;
static constexpr uint8_t kSetInvDisp = 0xA7;
static constexpr uint8_t kSetMuxRatio = 0xA8;
static constexpr uint8_t kSetDisp = 0xAE;
static constexpr uint8_t kSetComOutDir = 0xC0;
static constexpr uint8_t kSetComOutDirFlip = 0xC0;

static constexpr uint8_t kSetDispOffset = 0xD3;
static constexpr uint8_t kSetDispClkDiv = 0xD5;
static constexpr uint8_t kSetPrecharge = 0xD9;
static constexpr uint8_t kSetComPinCfg = 0xDA;
static constexpr uint8_t kSetVcomDesel = 0xDB;

// Other helper constants
static constexpr uint8_t kWidth = 128;
static constexpr uint8_t kPageHeight = 8;

void Ssd1306::i2cMasterInit(i2c_port_num_t i2cPort, gpio_num_t sdaPin,
                            gpio_num_t sclPin) {
    i2c_master_bus_config_t i2cMstConfig = {};
    i2cMstConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    i2cMstConfig.glitch_ignore_cnt = 7;
    i2cMstConfig.i2c_port = i2cPort;
    i2cMstConfig.sda_io_num = sdaPin;
    i2cMstConfig.scl_io_num = sclPin;
    i2cMstConfig.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2cMstConfig, &mI2cBusHandle));

    i2c_device_config_t devCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = kI2cAddr,
        .scl_speed_hz = 400000,
    };

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(mI2cBusHandle, &devCfg, &mI2cDevHandle));
}

Ssd1306::Ssd1306(i2c_port_num_t i2cPort, gpio_num_t sdaPin, gpio_num_t sclPin,
                 Ssd1306::Type oledType)
    : mType(oledType) {
    i2cMasterInit(i2cPort, sdaPin, sclPin);
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t comPinCfg = 0x12;
    uint8_t displayHeight = 64;
    if (mType == Type::ssd1306_128x32) {
        comPinCfg = 0x02;
        displayHeight = 32;
    }

    uint8_t cmds[] = {
        kSetDisp,   // set display off
        /* memory mapping */
        kSetMemMode,   // set memory address mode 0 = horizontal, 1 =
                       // vertical, 2 = page
        0x00,          // horizontal addressing mode
        /* resolution and layout */
        kDispStartLine,   // set display start line to 0
        kSetSegRemap |
            0x01,   // set segment re-map, column address 127 is mapped to SEG0
        kSetMuxRatio,           // set multiplex ratio
        --displayHeight,        // Display height - 1
        kSetComOutDir | 0x08,   // set COM (common) output scan direction.
                                // Scan from bottom up, COM[N-1] to COM0
        kSetDispOffset,         // set display offset
        0x00,                   // no offset
        kSetComPinCfg,          // set COM (common) pins hardware configuration.
                                // Board specific magic number. 0x02 Works for
                                // 128x32, 0x12 Possibly works for 128x64. Other
                                // options 0x22, 0x32
        comPinCfg,
        /* timing and driving scheme */
        kSetDispClkDiv,   // set display clock divide ratio
        0x80,             // div ratio of 1, standard freq
        kSetPrecharge,    // set pre-charge period
        0xF1,             // Vcc internally generated on our board
        kSetVcomDesel,    // set VCOMH deselect level
        0x30,             // 0.83xVcc
        /* display */
        kSetContrast,   // set contrast control
        0xFF,
        kSetEntireOn,     // set entire display on to follow RAM content
        kSetNormDisp,     // set normal (not inverted) display
        kSetChargePump,   // set charge pump
        0x14,             // Vcc internally generated on our board
        kSetScroll |
            0x00,   // deactivate horizontal scrolling if set. This is necessary
                    // as memory writes will corrupt if scrolling was enabled
        kSetDisp | 0x01,   // turn display on
    };
    sendCommands(cmds, sizeof(cmds));
}

void Ssd1306::display(const uint8_t* buffer) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    uint8_t pageEndAddr = mType == Type::ssd1306_128x64 ? 7 : 3;

    // copy our frame buffer into a new buffer because we need to add the
    // control byte to the beginning
    uint8_t cmds[] = {
        kColAddr,
        0x00,         // Column start address (0 = reset)
        kWidth - 1,   // Column end address (127 = reset)
        kPageAddr,
        0x00,         // Page start address (0 = reset)
        pageEndAddr   // Page end address
    };
    sendCommands(cmds, sizeof(cmds));

    auto len = kWidth * (mType == Type::ssd1306_128x64 ? 8 : 4);
    uint8_t temp_buf[len + 1];
    temp_buf[0] = 0x40;
    memcpy(temp_buf + 1, buffer, len);
    i2c_master_transmit(mI2cDevHandle, temp_buf, len + 1, i2cTicksToWait);
}

uint16_t Ssd1306::getWidth() const { return kWidth; }

uint16_t Ssd1306::getHeight() const {
    return mType == Type::ssd1306_128x64 ? 64 : 32;
}

void Ssd1306::sendCommand(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_master_transmit(mI2cDevHandle, buf, 2, i2cTicksToWait);
}

void Ssd1306::sendCommands(const uint8_t* cmds, uint16_t len) {
    for (int i = 0; i < len; ++i) {
        sendCommand(cmds[i]);
    }
}