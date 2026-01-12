/*
 * ThermistorModule.cpp
 *
 *  Created on: Jun 12, 2025
 *      Author: ADE1HYD
 */

#include "Thermistor.hpp"
#include <cmath>

ThermistorModule::ThermistorModule(ADC_HandleTypeDef* hadc, GPIO_TypeDef* doPort, uint16_t doPin)
    : _hadc(hadc), _doPort(doPort), _doPin(doPin) {}

uint16_t ThermistorModule::readRawADC() {
    HAL_ADC_Start(_hadc);
    HAL_ADC_PollForConversion(_hadc, HAL_MAX_DELAY);
    uint16_t raw = HAL_ADC_GetValue(_hadc);
    HAL_ADC_Stop(_hadc);
    return raw;
}

float ThermistorModule::readVoltage() {
    uint16_t raw = readRawADC();
    return (raw / 4095.0f) * _Vref;
}

bool ThermistorModule::isThresholdExceeded() {
    return HAL_GPIO_ReadPin(_doPort, _doPin) == GPIO_PIN_RESET;
}

float ThermistorModule::getTemperatureCelsius() {
    float Vout = readVoltage();

    if (Vout <= 0.01f || Vout >= _Vref - 0.01f) {
        // Avoid divide-by-zero or saturation error
        return -273.15f;  // Return absolute zero to indicate error
    }

    float R2 = _R1 * (Vout / (_Vref - Vout));  // Thermistor resistance
    float lnR = log(R2 / _R0);

    float invT = (1.0f / _T0) + (lnR / _Beta);
    float tempK = 1.0f / invT;
    float tempC = tempK - 273.15f;

    return tempC;
}



