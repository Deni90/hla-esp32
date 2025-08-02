#include "uart.h"

#include <cstring>
#include <utility>

#include "esp_log.h"
#include "freertos/task.h"

using hla::Uart;

static const char* kTag = "uart";

Uart::Uart(uart_port_t port, gpio_num_t txPin, gpio_num_t rxPin)
    : mPort(port), mTxPin(txPin), mRxPin(rxPin), mResponseQueue(nullptr) {}

void Uart::initialize() {
    uart_config_t config = {};
    config.baud_rate = 9600;
    config.data_bits = UART_DATA_8_BITS;
    config.parity = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;

    uart_param_config(mPort, &config);
    uart_set_pin(mPort, mTxPin, mRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(mPort, 1024 * 2, 0, 0, nullptr, 0);

    mResponseQueue = xQueueCreate(1, sizeof(uint16_t));
    xTaskCreate(rxTask, "uart_rx_task", 2048, this, 10, nullptr);
}

void Uart::send(uint8_t cmd, uint8_t data) {
    uint8_t buf[3] = {cmd, data, 0};
    buf[2] = crc8(buf, 2);
    uart_write_bytes(mPort, reinterpret_cast<const char*>(buf), 3);
    ESP_LOGI(kTag, "Sent: cmd=0x%02X, data=0x%02X, crc=0x%02X", cmd, data,
             buf[2]);
}

bool Uart::recv(uint8_t* cmd, uint8_t* data, TickType_t timeout) {
    std::pair<uint8_t, uint8_t> combinedData;
    bool result =
        xQueueReceive(mResponseQueue, &combinedData, timeout) == pdTRUE;
    *cmd = combinedData.first;
    *data = combinedData.second;
    return result;
}

void Uart::rxTask(void* param) {
    Uart* self = static_cast<Uart*>(param);
    uint8_t buf[3];

    while (true) {
        int len = uart_read_bytes(self->mPort, buf, sizeof(buf),
                                  pdMS_TO_TICKS(20000));
        if (len == 3) {
            uint8_t crc = self->crc8(buf, 2);
            if (crc == buf[2]) {
                auto combined = std::make_pair(buf[0], buf[1]);
                xQueueSend(self->mResponseQueue, &combined, portMAX_DELAY);
                ESP_LOGI(kTag, "Valid response: 0x%02X 0x%02X", buf[0], buf[1]);
            } else {
                ESP_LOGW(kTag, "CRC error");
            }
        }
    }
}

uint8_t Uart::crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0x00;   // Initial value
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}