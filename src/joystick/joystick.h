
#pragma once

#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

class Axis {

public:
    Axis(uint8_t gpioPin);


    uint16_t getValue();

private:

    uint8_t gpioPin;

};

class Joystick {

public:
    Joystick(Axis* axis1, Axis* axis2);


    uint16_t getReadingAxis1();
    uint16_t getReadingAxis2();

private:
    Axis* axis1;
    Axis* axis2;

};