
#include "esp_littlefs.h"
#include "esp_log.h"

#include "loom.h"

using hla::Loom;

static const char* kTag = "main";
static Loom gLoom;

static bool SetupLittlefs() {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "littlefs",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(kTag, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(kTag, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(kTag, "Failed to initialize LittleFS (%s)",
                     esp_err_to_name(ret));
        }
        return false;
    }
    return true;
}

extern "C" void app_main(void) {
    ESP_LOGI(kTag, "Handloom automation controller");
    ESP_LOGI(kTag, "Initialize LittleFS...");
    if (SetupLittlefs()) {
        ESP_LOGI(kTag, "Initialize LittleFS... done");
    } else {
        ESP_LOGE(kTag, "Initialize LittleFS... failed");
    }

    ESP_LOGI(kTag, "Initialize Loom...");
    gLoom.initialize();
    ESP_LOGI(kTag, "Initialize Loom... done");
}