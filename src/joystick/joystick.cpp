

#include "joystick.h"

#include "logging/logging.h"

#include <cstdio>

Joystick::Joystick(Axis* axis1, Axis* axis2) {

    this->axis1 = axis1;
    this->axis2 = axis2;

    debug("new Joystick() made");

}


uint16_t Joystick::getReadingAxis1(){
    return axis1->getValue();
}

uint16_t Joystick::getReadingAxis2() {
    return axis2->getValue();
}

