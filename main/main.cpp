

#include "esp_log.h"

#include "loom.h"

using hla::Loom;

static const char* kTag = "main";
static Loom gLoom;

extern "C" void app_main(void) {
    ESP_LOGI(kTag, "Handloom automation controller");
    ESP_LOGI(kTag, "Initialize Loom...");
    gLoom.initialize();
    ESP_LOGI(kTag, "Initialize Loom... done");
}