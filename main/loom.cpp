#include <cstring>

#include "dns_server.h"
#include "driver/gpio.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_system.h"   //esp_init funtions esp_err_t
#include "esp_wifi.h"     //esp_wifi_init functions and wifi operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "mbedtls/base64.h"
#include "mdns.h"
#include "nvs_flash.h"   //non volatile storage

#include "cJSON.h"

#include "config_store.h"
#include "loom.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Loom;
using hla::WifiInfo;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char* kTag = "loom";
static const char* kApSsid = "HandloomController";
static constexpr int kMaxConnectionRetry = 5;
static constexpr gpio_num_t kNextButton = GPIO_NUM_17;
static constexpr gpio_num_t kPrevButton = GPIO_NUM_16;

Loom::Loom()
    : ButtonHandler({kNextButton, kPrevButton}), mWebServer(*this),
      mLiftplanCursor(nullptr) {}

void Loom::initialize() {
    ESP_LOGI(kTag, "Initialize LittleFS...");
    if (setupLittlefs()) {
        ESP_LOGI(kTag, "Initialize LittleFS... done");
    } else {
        ESP_LOGE(kTag, "Initialize LittleFS... failed");
    }

    ESP_LOGI(kTag, "Initialize Loom...");
    mLoomInfo = ConfigStore::loadLoomInfo().value_or(LoomInfo());
    ESP_LOGI(kTag, "Initialize Loom... done");
    if (mLoomInfo.state != LoomState::idle) {
        loadLiftplan(mLoomInfo.liftplanName.value(),
                     mLoomInfo.liftplanIndex.value());
    }

    WifiInfo wi = ConfigStore::loadWifiInfo().value_or(WifiInfo());

    ESP_LOGI(kTag, "Initialize Wifi...");
    setupWifi(wi);
    ESP_LOGI(kTag, "Initialize Wifi... done");

    ESP_LOGI(kTag, "Initialize MDNS service...");
    startMdnsService(wi);
    ESP_LOGI(kTag, "Initialize MDNS service... done");

    ESP_LOGI(kTag, "Initialize Web server...");
    mWebServer.initialize();
    ESP_LOGI(kTag, "Initialize Web server... done");
}

std::optional<WifiInfo> Loom::onGetWifiInfo() const {
    return ConfigStore::loadWifiInfo();
}

void Loom::onSetWifiInfo(const WifiInfo& wifiInfo) {
    ConfigStore::saveWifiInfo(wifiInfo);
}

std::vector<std::string> Loom::onGetLiftplans() const {
    return ConfigStore::listLiftplanFiles();
}

std::optional<std::string> Loom::onGetLiftplan(const std::string& fileName) {
    return ConfigStore::loadLiftplan(fileName);
}

bool Loom::onSetLiftPlan(const std::string& fileName, const std::string& data) {
    return ConfigStore::saveLiftPlan(fileName, data);
}

bool Loom::onDeleteLiftPlan(const std::string& fileName) {
    return ConfigStore::deleteLiftPlan(fileName);
}

bool Loom::onStart(const std::string& liftplanFileName,
                   unsigned int startPosition) {
    // it is only possible to switch to running from idle state
    if (mLoomInfo.state != LoomState::idle) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Not in 'idle' state.");
        return false;
    }
    loadLiftplan(liftplanFileName, startPosition);
    // TODO move shafts to match the first element from the liftplan
    ESP_LOGI(kTag, "Move shatfs to 0x%02x", mLiftplanCursor.value());

    ESP_LOGI(kTag, "Switching to 'running' state.");
    mLoomInfo.state = LoomState::running;
    return true;
}

bool Loom::onPause() {
    if (mLoomInfo.state != LoomState::running) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'pause' state. Not in 'running' state.");
        return false;
    }
    ESP_LOGI(kTag, "Switching to 'paused' state.");
    mLoomInfo.state = LoomState::paused;
    ConfigStore::saveLoomInfo(mLoomInfo);
    return true;
}

bool Loom::onContinue() {
    if (mLoomInfo.state != LoomState::paused) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Not in 'paused' state.");
        return false;
    }
    ESP_LOGI(kTag, "Switching to 'running' state.");
    mLoomInfo.state = LoomState::running;
    ConfigStore::deleteLoomInfo();
    return true;
}

bool Loom::onStop() {
    // if in "idle" there is nothing to do
    if (mLoomInfo.state == LoomState::idle) {
        return true;
    }
    // TODO raise all shatfs back to idle position
    // clear the liftplan buffer, ...
    resetLiftplan();
    // switch back to idle state
    ESP_LOGI(kTag, "Switching to 'idle' state.");
    mLoomInfo.state = LoomState::idle;
    ConfigStore::deleteLoomInfo();
    return true;
}

std::string Loom::onGetLoomState() const {
    switch (mLoomInfo.state) {
    case LoomState::idle:
        return "idle";
    case LoomState::paused:
        return "paused";
    case LoomState::running:
        return "running";
    }
    return "";
}

std::optional<unsigned int> Loom::onGetActiveLiftplanIndex() const {
    return mLoomInfo.liftplanIndex;
}

std::optional<std::string> Loom::onGetActiveLiftplanName() const {
    return mLoomInfo.liftplanName;
}

static EventGroupHandle_t gWifiEventGroup;

extern "C" void wifiEventHandler(void* arg, esp_event_base_t eventBase,
                                 int32_t eventId, void* eventData) {
    static int retryCount = 0;
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (eventBase == WIFI_EVENT &&
               eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryCount < kMaxConnectionRetry) {
            esp_wifi_connect();
            retryCount++;
            ESP_LOGI(kTag, "Retrying connection to Wi-Fi...");
        } else {
            xEventGroupSetBits(gWifiEventGroup, WIFI_FAIL_BIT);
        }

    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(kTag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retryCount = 0;
        xEventGroupSetBits(gWifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

bool Loom::setupLittlefs() {
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/littlefs";
    conf.partition_label = "littlefs";
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

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

void Loom::setupWifi(const WifiInfo& wifiInfo) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    gWifiEventGroup = xEventGroupCreate();
    if (wifiInfo.getSSID() == "" || !initializeWifiInStationMode(wifiInfo)) {
        initializeWifiInApMode(wifiInfo);
        ESP_LOGI(kTag, "Setup captive portal...");
        setupCaptivePortal();
        ESP_LOGI(kTag, "Setup captive portal... done");
        ESP_LOGI(kTag, "Start DNS server...");
        // Start the DNS server that will redirect all queries to the softAP IP
        dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
            "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
        start_dns_server(&config);
        ESP_LOGI(kTag, "Start DNS server... done");
    }
}

bool Loom::initializeWifiInStationMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, nullptr, nullptr));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.ssid),
                 wifiInfo.getSSID().c_str(), sizeof(wifiConfig.sta.ssid));
    unsigned char password[64];
    size_t passwordLenght;
    mbedtls_base64_decode(password, 64, &passwordLenght,
                          (unsigned char*) wifiInfo.getPassword().c_str(),
                          wifiInfo.getPassword().length());
    password[passwordLenght] = '\0';
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.password),
                 reinterpret_cast<char*>(password),
                 sizeof(wifiConfig.sta.password));
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits =
        xEventGroupWaitBits(gWifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(kTag, "Connected to STA: %s", wifiInfo.getSSID().c_str());
        return true;
    }

    ESP_LOGW(kTag, "Failed to connect to STA.");
    return false;
}

void Loom::initializeWifiInApMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.getHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.ap.ssid), kApSsid,
                 sizeof(wifiConfig.ap.ssid));
    wifiConfig.ap.ssid_len = std::strlen(kApSsid);
    wifiConfig.ap.channel = 1;
    wifiConfig.ap.max_connection = 4;
    wifiConfig.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                          &ipInfo);

    ESP_LOGI(kTag, "Started AP mode with SSID: %s", wifiConfig.ap.ssid);
}

void Loom::setupCaptivePortal() {
    // get the IP of the access point to redirect to
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"),
                          &ipInfo);

    char ipAddr[16];
    inet_ntoa_r(ipInfo.ip.addr, ipAddr, 16);
    ESP_LOGI(kTag, "Set up softAP with IP: %s", ipAddr);

    // turn the IP into a URI
    char* captiveportal_uri = (char*) malloc(32 * sizeof(char));
    assert(captiveportal_uri && "Failed to allocate captiveportal_uri");
    strcpy(captiveportal_uri, "http://");
    strcat(captiveportal_uri, ipAddr);

    // get a handle to configure DHCP with
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    // set the DHCP option 114
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(netif));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(
        netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, captiveportal_uri,
        strlen(captiveportal_uri)));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(netif));
}

void Loom::startMdnsService(const WifiInfo& wifiInfo) {
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(wifiInfo.getHostname().c_str()));
    ESP_ERROR_CHECK(mdns_instance_name_set("HandloomController web server"));
    ESP_LOGI(kTag, "mDNS started: http://%s.local",
             wifiInfo.getHostname().c_str());
}

void Loom::onButtonPressed(gpio_num_t gpio) {
    ESP_LOGI(kTag, "Pressed GPIO %d", gpio);
    if (mLoomInfo.state != LoomState::running || !mLiftplanCursor.isValid()) {
        return;
    }
    if (gpio == kNextButton) {
        mLiftplanCursor = mLiftplanCursor.next();
        ++(*mLoomInfo.liftplanIndex);
        (*mLoomInfo.liftplanIndex) %= mLiftplan.length();
    } else if (gpio == kPrevButton) {
        mLiftplanCursor = mLiftplanCursor.prev();
        if (mLoomInfo.liftplanIndex > 0) {
            --(*mLoomInfo.liftplanIndex);
        } else {
            mLoomInfo.liftplanIndex = mLiftplan.length() - 1;
        }
    } else {
        return;
    }
    // TODO implement me
    ESP_LOGI(kTag, "Move shatfs to 0x%02x", mLiftplanCursor.value());
}

void Loom::resetLiftplan() {
    mLoomInfo.liftplanName.reset();
    mLiftplan.empty();
    mLiftplanCursor.reset();
    mLoomInfo.liftplanIndex = std::nullopt;
}

bool Loom::loadLiftplan(const std::string& liftplanFileName,
                        unsigned int startPosition) {
    // load the raw liftplan from file
    auto maybeLiftplan = ConfigStore::loadLiftplan(liftplanFileName);
    if (!maybeLiftplan.has_value()) {
        ESP_LOGW(kTag,
                 "Failed to switch to 'running' state. Liftplan not found.");
        return false;
    }
    auto liftplan = maybeLiftplan.value();
    // parse the liftplan - and load it
    if (mLiftplan.length()) {
        resetLiftplan();
    }
    cJSON* root = cJSON_Parse(liftplan.c_str());
    if (!root || !cJSON_IsArray(root)) {
        ESP_LOGW(
            kTag,
            "Failed to switch to 'running' state. Failed to parse liftplan.");
        cJSON_Delete(root);
        return false;
    }
    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; i++) {
        cJSON* item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsString(item)) {
            ESP_LOGW(kTag, "Failed to switch to 'running' state. Failed to "
                           "parse liftplan.");
            cJSON_Delete(root);
            return false;
        }
        uint8_t value = (uint8_t) strtoul(item->valuestring, nullptr, 16);
        mLiftplan.pushBack(value);
    }
    cJSON_Delete(root);
    mLoomInfo.liftplanName = liftplanFileName;
    // setup the cursor
    mLiftplanCursor = mLiftplan.frontCursor();
    for (unsigned int i = 0; i < startPosition; ++i) {
        mLiftplanCursor = mLiftplanCursor.next();
    }
    mLoomInfo.liftplanIndex = startPosition % mLiftplan.length();
    return true;
}