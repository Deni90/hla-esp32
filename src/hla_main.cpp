#include <Arduino.h>
#include <Ticker.h>

static constexpr unsigned long kUartBaudrate = 115200;
static constexpr uint32_t kTimerPeriod = 1;   // ms

static Ticker gTimer;
static uint32_t gClock = 0;

/**
 * @brief Function that is called every millisecond
 */
static void HandleTimer() { gClock++; }

/**
 * @brief Initialize all the necessary modules
 */
void setup() {
    Serial.begin(kUartBaudrate);

    Serial.println("\nHandloom automation controller\n");

    Serial.printf("Initializing timer(s)... ");
    gTimer.attach_ms(kTimerPeriod, HandleTimer);
    Serial.println("Done");
}

void loop() {
    if(gClock>= 1000) {
        gClock = 0;
        Serial.println("Hello");
    }
}