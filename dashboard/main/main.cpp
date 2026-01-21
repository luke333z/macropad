#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "comp/slider/Slider.h"
#include "comp/wifi/WifiManager.h"


static void Delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}


extern "C" void app_main(void)
{
	WifiManager::Init();

	WifiManager::Connect("", "");

	while (true)
	{
		Delay(1000);
	}
	
}