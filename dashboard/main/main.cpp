#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Slider.h"


static void Delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}


extern "C" void app_main(void)
{
	Slider slider{ ADC_CHANNEL_0 };

	static_assert(sizeof(short) == 2);
	static_assert(sizeof(uint32_t) == 4);
	static_assert(sizeof(long) == 4);
	static_assert(sizeof(long long) == 8);

	while (true)
	{
		printf("%i", (int)slider.GetRaw());
		Delay(200);
	}
}