/*
 * Potentiometer.cpp
 *
 *  Created on: Jun 11, 2025
 *      Author: ADE1HYD
 */


#include "Potentiometer.hpp"

ADCDevice::ADCDevice(ADC_HandleTypeDef* hadc, uint32_t channel)
    : _hadc(hadc) {}

uint16_t ADCDevice::readRaw() {
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    HAL_ADC_ConfigChannel(_hadc, &sConfig);

    HAL_ADC_Start(_hadc);
    HAL_ADC_PollForConversion(_hadc, HAL_MAX_DELAY);
    uint16_t adcValue = HAL_ADC_GetValue(_hadc);
    HAL_ADC_Stop(_hadc);

    return adcValue;
}

uint8_t ADCDevice::readPercentage() {
    uint16_t value = readRaw();
    return (value / 4095.0f) * 100.0f;
}

