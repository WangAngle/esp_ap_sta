/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "lwip/inet.h"
#include "lwip/lwip_napt.h"

#include "ap_nvs.h"
#include "ap_console.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

#define DEFAULT_AP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define DEFAULT_AP_MAX_STA_CONN   CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "wifi softAP";

static const uint8_t default_ap_mac[6] = {0x0A, 0x00, 0x27, 0x00, 0x00, 0x15};
static const char *default_ap_ssid = "AP-WiFi";
static const char *default_ap_pass = "12345678";

static const char *default_sta_ssid = "STA-WiFi";
static const char *default_sta_pass = "12345678";

static ap_info_t ap_info = {0};
static sta_info_t sta_info = {0};

static esp_netif_t* _esp_netif_sta = NULL;
static esp_netif_t* _esp_netif_ap = NULL;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if(event_id == WIFI_EVENT_STA_START) {
    	esp_err_t status = esp_wifi_connect();
    	ESP_LOGI(TAG, "esp_wifi_connect %d....", status);
    }
}

// static void sta_start_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     esp_err_t status = esp_wifi_connect();
//     ESP_LOGI(TAG, "esp_wifi_connect %d....", status);
// }

// static void got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     esp_netif_dns_info_t dns;
//     if (esp_netif_get_dns_info(_esp_netif_sta, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK) {
//         dhcps_dns_setserver((const ip_addr_t *)&dns.ip);
//         ESP_LOGI(TAG, "set dns to:" IPSTR, IP2STR(&dns.ip.u_addr.ip4));
//     }
// }

static void wifi_init_apsta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	
    _esp_netif_ap = esp_netif_create_default_wifi_ap();
	_esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_err_t ret = 0;

    ret = esp_wifi_set_mac(WIFI_IF_AP, ap_info.mac);
    ESP_LOGI(TAG, "==============================esp_wifi_set_mac ret:%d", ret);

uint8_t wifi_mac[6]={0};
esp_wifi_get_mac(WIFI_IF_AP, wifi_mac);
ESP_LOGI( TAG, "esp_wifi_get_mac:%02x:%02x:%02x:%02x:%02x:%02x", wifi_mac[0],wifi_mac[1],wifi_mac[2],wifi_mac[3],wifi_mac[4],wifi_mac[5]); 

    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t config_ap = {
        .ap = {
            // .ssid = DEFAULT_AP_WIFI_SSID,
            // .ssid_len = strlen(DEFAULT_AP_WIFI_SSID),
            // .password = DEFAULT_AP_WIFI_PASS,
            .channel = DEFAULT_AP_WIFI_CHANNEL,
            .max_connection = DEFAULT_AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            // .pmf_cfg = {
            //         .required = false,
            // },
        },
    };

    memcpy(config_ap.ap.ssid, ap_info.ssid, sizeof(config_ap.ap.ssid));
    config_ap.ap.ssid_len = strlen((char *)config_ap.ap.ssid);
    
    memcpy(config_ap.ap.password, ap_info.pass, sizeof(config_ap.ap.password));
    if (strlen((char *)config_ap.ap.password) == 0) {
        config_ap.ap.authmode = WIFI_AUTH_OPEN;
    }

    wifi_config_t config_sta = {
		.sta = {
			// .ssid = STA_WIFI_SSID,
			// .password = STA_WIFI_PASS,
			.threshold = {.rssi=0, .authmode = WIFI_AUTH_WPA2_PSK},
			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};

    memcpy(config_sta.sta.ssid, sta_info.ssid, sizeof(config_sta.sta.ssid));
    memcpy(config_sta.sta.password, sta_info.pass, sizeof(config_sta.sta.password));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             config_ap.ap.ssid, config_ap.ap.password, DEFAULT_AP_WIFI_CHANNEL);

    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config_ap));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config_sta));
	// ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, sta_start_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_start());

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

	char ip_addr[16];
	inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
	ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ret = get_ap_info(&ap_info);
    if(ret == ESP_ERR_NVS_NOT_FOUND)
    {
        memcpy(ap_info.mac, default_ap_mac, sizeof(ap_info.mac));
        memcpy(ap_info.ssid, default_ap_ssid, sizeof(ap_info.ssid));
        memcpy(ap_info.pass, default_ap_pass, sizeof(ap_info.pass));
        set_ap_info(&ap_info);
    }
    ESP_LOGI(TAG, "ap_info mac:%02X", ap_info.mac[0]);
    ESP_LOGI(TAG, "ap_info ssid:%s", ap_info.ssid);
    ESP_LOGI(TAG, "ap_info pass:%s", ap_info.pass);

    ret = get_sta_info(&sta_info);
    if(ret == ESP_ERR_NVS_NOT_FOUND)
    {
        memcpy(sta_info.ssid, default_sta_ssid, sizeof(sta_info.ssid));
        memcpy(sta_info.pass, default_sta_pass, sizeof(sta_info.pass));
        set_sta_info(&sta_info);
    }
    ESP_LOGI(TAG, "sta_info ssid:%s", sta_info.ssid);
    ESP_LOGI(TAG, "sta_info pass:%s", sta_info.pass);

    console_task_init();

    ESP_LOGI(TAG, "ESP_WIFI_MODE_APSTA");
    wifi_init_apsta();

    ip_addr_t dnsserver;
	// Enable DNS (offer) for dhcp server
	dhcps_offer_t dhcps_dns_value = OFFER_DNS;
	dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));
	// Set custom dns server address for dhcp server
	dnsserver.u_addr.ip4.addr = htonl(0xC0A80301);
	dnsserver.type = IPADDR_TYPE_V4;
	dhcps_dns_setserver(&dnsserver);

#if IP_NAPT
	// !!! 必须启动sta后再设置，不然ap无网络 !!! Set to ip address of softAP netif (Default is 192.168.4.1)
	u32_t napt_netif_ip = 0xC0A80401;
	ip_napt_enable(htonl(napt_netif_ip), 1);
    ESP_LOGI(TAG, "=====================================ip_napt_enable");
#endif
}
