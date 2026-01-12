#ifndef MOTORDRIVER_HPP
#define MOTORDRIVER_HPP

#include "stm32f4xx_hal.h"

class MotorDriver {
public:
    MotorDriver(TIM_HandleTypeDef* htim, uint32_t pwmChannel, GPIO_TypeDef* stbyPort, uint16_t stbyPin);

    void begin();
    void setSpeed(uint16_t duty);  // duty range: 0–timer period (e.g., 0–100)

private:
    TIM_HandleTypeDef* _htim;
    uint32_t _pwmChannel;
    GPIO_TypeDef* _stbyPort;
    uint16_t _stbyPin;
};

#endif
