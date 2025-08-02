#ifndef uart_h
#define uart_h

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace hla {
class Uart {
  public:
    Uart(uart_port_t port, gpio_num_t txPin, gpio_num_t rxPin);
    void initialize();
    void send(uint8_t cmd, uint8_t data);
    bool recv(uint8_t* cmd, uint8_t* data,
              TickType_t timeout = pdMS_TO_TICKS(20000));

  private:
    static void rxTask(void* param);
    uint8_t crc8(const uint8_t* data, size_t len);

    uart_port_t mPort;
    gpio_num_t mTxPin;
    gpio_num_t mRxPin;
    QueueHandle_t mResponseQueue;
};
}   // namespace hla
#endif   // uart_h