/*
 * MotorDriver.cpp
 *
 *  Created on: Jun 10, 2025
 *      Author: ADE1HYD
 */


#include "MotorDriver.hpp"

MotorDriver::MotorDriver(TIM_HandleTypeDef* htim, uint32_t pwmChannel, GPIO_TypeDef* stbyPort, uint16_t stbyPin)
    : _htim(htim), _pwmChannel(pwmChannel), _stbyPort(stbyPort), _stbyPin(stbyPin) {}

void MotorDriver::begin() {
    HAL_GPIO_WritePin(_stbyPort, _stbyPin, GPIO_PIN_SET);  // Enable standby
    HAL_TIM_PWM_Start(_htim, _pwmChannel);                 // Start PWM
}

void MotorDriver::setSpeed(uint16_t duty) {
    __HAL_TIM_SET_COMPARE(_htim, _pwmChannel, duty);       // Set PWM duty
}

