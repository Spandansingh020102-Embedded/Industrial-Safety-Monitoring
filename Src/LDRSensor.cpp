/*
 * LDRSensor.cpp
 *
 *  Created on: Jun 11, 2025
 *      Author: ADE1HYD
 */

#include "LDRSensor.hpp"

LDRSensor::LDRSensor(GPIO_TypeDef* port, uint16_t pin)
    : _port(port), _pin(pin) {}

bool LDRSensor::isLightDetected() {
    GPIO_PinState state = HAL_GPIO_ReadPin(_port, _pin);
    return (state == GPIO_PIN_SET);
}



