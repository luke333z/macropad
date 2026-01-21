#pragma once

#include <esp_adc/adc_oneshot.h>

class Slider
{
public:
	Slider(adc_channel_t channel);
	~Slider() = default;

	uint32_t GetRaw();

	float GetPosition();

private:
	adc_channel_t m_Channel;

	inline static adc_oneshot_unit_handle_t s_ADCUnitHandle{};
};