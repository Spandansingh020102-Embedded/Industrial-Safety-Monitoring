/*
 * Potentiometer.hpp
 *
 *  Created on: Jun 11, 2025
 *      Author: ADE1HYD
 */

#ifndef ADC_DEVICE_HPP
#define ADC_DEVICE_HPP

#include "stm32f4xx_hal.h"

class ADCDevice {
public:
    ADCDevice(ADC_HandleTypeDef* hadc, uint32_t channel);
    uint16_t readRaw();         // Get raw 12-bit ADC value
    uint8_t readPercentage();     // Return value as 0.0 - 100.0

private:
    ADC_HandleTypeDef* _hadc;
    uint32_t channel;
};

#endif



