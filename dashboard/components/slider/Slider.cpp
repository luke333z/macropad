#include "Slider.h"


Slider::Slider(adc_channel_t channel) : m_Channel{ channel }
{
	if (!s_ADCUnitHandle)
	{
		adc_oneshot_unit_init_cfg_t config = 
		{
			.unit_id = ADC_UNIT_1
		};
		ESP_ERROR_CHECK(adc_oneshot_new_unit(&config, &s_ADCUnitHandle));
	}

	adc_oneshot_chan_cfg_t config = 
	{
		.atten = ADC_ATTEN_DB_2_5,
		.bitwidth = ADC_BITWIDTH_10
	};

	adc_oneshot_config_channel(s_ADCUnitHandle, m_Channel, &config);
}

uint32_t Slider::GetRaw()
{
	int reading;
	ESP_ERROR_CHECK(adc_oneshot_read(s_ADCUnitHandle, m_Channel, &reading));
	return reading;
}