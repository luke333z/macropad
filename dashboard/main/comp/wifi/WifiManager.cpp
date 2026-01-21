#include "WifiManager.h"

#include <esp_system.h>
#include <esp_log.h>
#include <esp_wifi.h>


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static const char* TAG = "WifiManager";

void WifiManager::Init()
{
	if (s_WifiEventGroup)
	{
		ESP_LOGE(TAG, "WifiManager already initialized!");
		return;
	}

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();
	
	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_wifi_init(&config));

	esp_event_handler_instance_t instanceAnyId;
	esp_event_handler_instance_t instanceGotIp;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler, NULL, &instanceAnyId));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EventHandler, NULL, &instanceGotIp));

	ESP_LOGI(TAG, "Init done.");
}

void WifiManager::Connect(const char* ssidd, const char* passwd)
{
	wifi_config_t config = 
	{
		.sta = 
		{
			.threshold =
			{
				.authmode = WIFI_AUTH_WPA2_PSK
			},
			.sae_pwe_h2e = WPA3_SAE_PWE_UNSPECIFIED
		}
	};

	memcpy(config.sta.ssid, ssidd, strlen(ssidd));
	memcpy(config.sta.password, passwd, strlen(passwd));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "Connecting to %s (%s)", ssidd, passwd);

	EventBits_t bits = xEventGroupWaitBits(s_WifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	if (bits & WIFI_CONNECTED_BIT)
		ESP_LOGI(TAG, "Connected!");
	else if (bits & WIFI_FAIL_BIT)
		ESP_LOGE(TAG, "Failed to connect!");
	else
		ESP_LOGE(TAG, "Unexpected event!");
}



void WifiManager::EventHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData)
{
	if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START)
		esp_wifi_connect();
	else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_Retries < s_MaxRetries)
		{
			esp_wifi_connect();
			s_Retries++;
			ESP_LOGI(TAG, "Attempting to reconnect...");
		}
		else
			xEventGroupSetBits(s_WifiEventGroup, WIFI_FAIL_BIT);
		ESP_LOGI(TAG, "Failed to reconnect!");
	}
	else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*)eventData;
		ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
		s_Retries = 0;
		xEventGroupSetBits(s_WifiEventGroup, WIFI_CONNECTED_BIT);
	}
}