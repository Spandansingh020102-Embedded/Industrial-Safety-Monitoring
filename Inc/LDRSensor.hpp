/*
 * LDRSensor.hpp
 *
 *  Created on: Jun 11, 2025
 *      Author: ADE1HYD
 */

#ifndef LDR_SENSOR_HPP
#define LDR_SENSOR_HPP

#include "stm32f4xx_hal.h"

class LDRSensor {
public:
    LDRSensor(GPIO_TypeDef* port, uint16_t pin);

    // Returns true if light is detected, false if it's dark
    bool isLightDetected();
private:
    GPIO_TypeDef* _port;
    uint16_t _pin;
};

#endif // LDR_SENSOR_HPP

