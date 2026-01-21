#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_event.h>


class WifiManager
{
public:
	WifiManager() = delete;
	~WifiManager() = default;

	static void Init();

	static void Connect(const char* ssid, const char* passwd);


	static void SetMaxRetries(uint8_t maxRetries) { s_MaxRetries = maxRetries; }

private:
	static void EventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

private:
	inline static uint8_t s_MaxRetries{ 5 };
	inline static uint8_t s_Retries{ 0 };

	inline static EventGroupHandle_t s_WifiEventGroup{};
};