#ifndef sh1106_h
#define sh1106_h

#include <cstdint>

#include "driver/i2c_master.h"

/**
 * @brief Class representing SH1106 type 128x64 oled display
 */
class Sh1106 {
  public:
    /**
     * @brief Constructor
     */
    Sh1106() = default;

    /**
     * @brief Initialize oled display
     *
     * @param[in] i2cPort I2C port
     * @param[in] sdaPin SDA pin
     * @param[in] sclPin SCL pin
     */
    void initialize(i2c_port_num_t i2cPort, gpio_num_t sdaPin,
                    gpio_num_t sclPin);

    /**
     * @brief Send a buffer to display.
     *
     * @param[in] buffer Pointer to a buffer containing data meant to be shown
     * on the display
     * @param[in] len Length of the buffer in bytes
     */
    void display(const uint8_t* buffer);

    /**
     * @brief Return screen width
     *
     * @return Screen width in pixels
     */
    uint16_t getWidth() const;

    /**
     * @brief Return screen height
     *
     * @return Screen height in pixels
     */
    uint16_t getHeight() const;

  private:
    /**
     * @brief Send command to display via I2C
     *
     * @param[in] cmd Command
     */
    void sendCommand(uint8_t cmd);

    /**
     * @brief Send multiple commands to display via I2C
     *
     * @param[in] cmds Buffer containing commands
     * @param[in] len Size of the buffer
     */
    void sendCommands(const uint8_t* cmds, uint16_t len);

    /**
     * @brief Initialize I2C master
     *
     * @param[in] i2cPort I2C port
     * @param[in] sdaPin SDA pin
     * @param[in] sclPin SCL pin
     */
    void i2cMasterInit(i2c_port_num_t i2cPort, gpio_num_t sda, gpio_num_t scl);

    i2c_master_bus_handle_t mI2cBusHandle;
    i2c_master_dev_handle_t mI2cDevHandle;
};

#endif   // sh1106_h