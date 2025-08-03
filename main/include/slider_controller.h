#ifndef slider_controller_h
#define slider_controller_h

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace hla {
class SliderController {
  public:
    /**
     * @brief Enumeration representing the controller's state
     */
    enum State { Unknown = 0x00, Init = 0x01, Ready = 0x02 };

    /**
     * @brief Constructor
     *
     * @param[in] port UART port
     * @param[in] txPin TX pin
     * @param[in] rxPin RX pin
     */
    SliderController(uart_port_t port, gpio_num_t txPin, gpio_num_t rxPin);

    /**
     * @brief Initialize the controller
     */
    void initialize();

    /**
     * @brief Get state
     *
     * @param[out] state State of the slider controller
     * @return true if the message is successfully received
     */
    bool getState(State& state);

    /**
     * @brief Send command to slider controller to move shafts
     *
     * @param[in] value Position of shafts
     * @return true if the message is successfully received
     */
    bool sendCommand(uint8_t value);

  private:
    enum UartMessages {
        StateRequest = 0x10,
        StateResponse = 0x11,
        CommandRequest = 0x20,
        CommandResponse = 0x21
    };
    enum StatusCode { Ok = 0x00, BadState = 0x01, Busy = 0x02 };
    void send(uint8_t cmd, uint8_t data);
    bool recv(uint8_t* cmd, uint8_t* data,
              TickType_t timeout = pdMS_TO_TICKS(20000));
    static void rxTask(void* param);
    uint8_t crc8(const uint8_t* data, size_t len);

    uart_port_t mPort;
    gpio_num_t mTxPin;
    gpio_num_t mRxPin;
    QueueHandle_t mResponseQueue;
};
}   // namespace hla
#endif   // slider_controller_h