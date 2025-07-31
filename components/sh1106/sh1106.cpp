#include "sh1106.h"

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
static constexpr uint8_t kHeight = 64;
static constexpr uint8_t kPageHeight = 8;

void Sh1106::i2cMasterInit(i2c_port_num_t i2cPort, gpio_num_t sdaPin,
                           gpio_num_t sclPin) {
    i2c_master_bus_config_t i2cMstConfig = {};
    i2cMstConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    i2cMstConfig.glitch_ignore_cnt = 7;
    i2cMstConfig.i2c_port = i2cPort;
    i2cMstConfig.sda_io_num = sdaPin;
    i2cMstConfig.scl_io_num = sclPin;
    i2cMstConfig.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2cMstConfig, &mI2cBusHandle));

    i2c_device_config_t devCfg = {};
    devCfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devCfg.device_address = kI2cAddr;
    devCfg.scl_speed_hz = 400000;

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(mI2cBusHandle, &devCfg, &mI2cDevHandle));
}

void Sh1106::initialize(i2c_port_num_t i2cPort, gpio_num_t sdaPin,
                        gpio_num_t sclPin) {
    i2cMasterInit(i2cPort, sdaPin, sclPin);

    uint8_t cmds[] = {
        kSetDisp,              // set display off
        kSetDispClkDiv,        // set display clock divide ratio
        0x80,                  // div ratio of 1, standard freq
        kSetMuxRatio,          // set multiplex ratio
        kHeight - 1,           // display height - 1
        kSetDispOffset,        // set display offset
        0x00,                  // no offset
        kDispStartLine,        // set display start line to 0
        kSetSegRemap | 0x01,   // set segment re-map, column address 127 is
                               // mapped to SEG0);
        0xC8,                  // COM scan dir
        kSetContrast,          // set contrast control
        0xFF,                  // contrast value
        kSetPrecharge,         // set pre-charge period
        0x22,                  // pre-charge value
        0x20,                  // VCOM deselect (note: used differently)
        kSetEntireOn,          // set entire display on to follow RAM content
        kSetNormDisp,          // set normal (not inverted) display
        kSetDisp | 0x01        // turn display on
    };
    sendCommands(cmds, sizeof(cmds));
}

void Sh1106::display(const uint8_t* buffer) {
    for (uint8_t page = 0; page < kHeight / kPageHeight; page++) {
        uint8_t cmds[] = {
            static_cast<uint8_t>(0xB0 + page),   // Set page address (B0 to B7)
            0x02,   // Set lower column address (start at 2)
            0x10    // Set higher column address
        };
        sendCommands(cmds, sizeof(cmds));

        uint8_t temp_buf[kWidth + 1];
        temp_buf[0] = 0x40;
        memcpy(temp_buf + 1, &buffer[page * kWidth], kWidth);
        i2c_master_transmit(mI2cDevHandle, temp_buf, kWidth + 1,
                            i2cTicksToWait);
    }
}

uint16_t Sh1106::getWidth() const { return kWidth; }

uint16_t Sh1106::getHeight() const { return kHeight; }

void Sh1106::sendCommand(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_master_transmit(mI2cDevHandle, buf, 2, i2cTicksToWait);
}

void Sh1106::sendCommands(const uint8_t* cmds, uint16_t len) {
    for (int i = 0; i < len; ++i) {
        sendCommand(cmds[i]);
    }
}