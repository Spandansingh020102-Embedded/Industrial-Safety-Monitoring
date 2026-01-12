/*
 * ThermistorModule.hpp
 *
 *  Created on: Jun 12, 2025
 *      Author: ADE1HYD
 */


#ifndef THERMISTOR_MODULE_HPP
#define THERMISTOR_MODULE_HPP

#include "stm32f4xx_hal.h"

class ThermistorModule {
public:
    ThermistorModule(ADC_HandleTypeDef* hadc, GPIO_TypeDef* doPort, uint16_t doPin);

    // Read raw ADC value (0–4095)
    uint16_t readRawADC();

    // Read voltage at AO (in Volts)
    float readVoltage();

    // Check if temperature exceeds threshold (based on DO pin logic)
    bool isThresholdExceeded();

    // Estimate temperature in Celsius using Steinhart-Hart equation
    float getTemperatureCelsius();

private:
    ADC_HandleTypeDef* _hadc;
    GPIO_TypeDef* _doPort;
    uint16_t _doPin;

    // Assumed constants (can be calibrated if needed)
    const float _Vref = 3.3f;       // ADC reference voltage
    const float _R1 = 7300.0f;      // Measured potentiometer value (Ω)
    const float _Beta = 3950.0f;    // Thermistor B-value //3950
    const float _T0 = 298.15f;      // 25°C in Kelvin
    const float _R0 = 10000.0f;     // Thermistor resistance at 25°C
};

#endif


