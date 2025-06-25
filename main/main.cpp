#include "esp_event.h"   //for wifi event
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_system.h"   //esp_init funtions esp_err_t
#include "esp_wifi.h"     //esp_wifi_init functions and wifi operations
#include "lwip/inet.h"
#include "mbedtls/base64.h"
#include "mdns.h"
#include "nvs_flash.h"   //non volatile storage

#include "config_store.h"
#include "dns_server.h"
#include "hla.h"
#include "web_server.h"
#include "wifi_info.h"

using hla::ConfigStore;
using hla::Hla;
using hla::WebServer;
using hla::WifiInfo;

static const char* kTag = "main";
static const char* kApSsid = "HandloomController";
static const int kMaxConnectionRetry = 5;

static EventGroupHandle_t gWifiEventGroup;
static Hla gHla;
static WebServer gWebServer(gHla);

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#include <cstring>
extern "C" void WifiEventHandler(void* arg, esp_event_base_t eventBase,
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

static bool InitializeWifiInStationMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.GetHostname().c_str()));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiEventHandler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiEventHandler, nullptr, nullptr));

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.ssid),
                 wifiInfo.GetSSID().c_str(), sizeof(wifiConfig.sta.ssid));
    unsigned char password[64];
    size_t passwordLenght;
    mbedtls_base64_decode(password, 64, &passwordLenght,
                          (unsigned char*) wifiInfo.GetPassword().c_str(),
                          wifiInfo.GetPassword().length());
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
        ESP_LOGI(kTag, "Connected to STA: %s", wifiInfo.GetSSID().c_str());
        return true;
    }

    ESP_LOGW(kTag, "Failed to connect to STA.");
    return false;
}

static void InitializeWifiInApMode(const WifiInfo& wifiInfo) {
    esp_netif_t* netif = esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(
        esp_netif_set_hostname(netif, wifiInfo.GetHostname().c_str()));

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

static void SetupCaptivePortal(void) {
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

static void SetupWifi(const WifiInfo& wifiInfo) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    gWifiEventGroup = xEventGroupCreate();
    if (wifiInfo.GetSSID() == "" || !InitializeWifiInStationMode(wifiInfo)) {
        InitializeWifiInApMode(wifiInfo);
        ESP_LOGI(kTag, "Setup captive portal...");
        SetupCaptivePortal();
        ESP_LOGI(kTag, "Setup captive portal... done");
        ESP_LOGI(kTag, "Start DNS server...");
        // Start the DNS server that will redirect all queries to the softAP IP
        dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE(
            "*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
        start_dns_server(&config);
        ESP_LOGI(kTag, "Start DNS server... done");
    }
}

static void StartMdnsService(const WifiInfo& wifiInfo) {
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(wifiInfo.GetHostname().c_str()));
    ESP_ERROR_CHECK(mdns_instance_name_set("HandloomController web server"));
    ESP_LOGI(kTag, "mDNS started: http://%s.local",
             wifiInfo.GetHostname().c_str());
}

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

    ESP_LOGI(kTag, "Initialize Wifi...");
    WifiInfo wi = ConfigStore::LoadWifiInfo().value_or(WifiInfo());
    SetupWifi(wi);
    ESP_LOGI(kTag, "Initialize Wifi... done");
    StartMdnsService(wi);

    ESP_LOGI(kTag, "Initialize Web server...");
    gWebServer.Initialize();
    ESP_LOGI(kTag, "Initialize Web server.. done");
}