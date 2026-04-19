#include "LVGLHelper.h"
#include <esp_lvgl_port.h>
#include <esp_lcd_panel_io.h>
#include <lvgl.h>
#include <esp_log.h>


#define TFT_CS  GPIO_NUM_5
#define TFT_DC  GPIO_NUM_16
#define TFT_RST GPIO_NUM_17



static const char* TAG = "LVGL";


void InitLCD()
{
	ESP_LOGI(TAG, "Initializing LCD...");

	esp_lcd_panel_io_spi_config_t config =
	{
		.cs_gpio_num = TFT_CS,
		.dc_gpio_num = 
	};
	
}

void InitTouch()
{

}

void InitLVGL()
{
	lvgl_port_cfg_t config = 
	{
		.task_priority = 4,
        .task_stack = 6144,
        .task_affinity = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
	};

	ESP_ERROR_CHECK(lvgl_port_init(&config));

	
}