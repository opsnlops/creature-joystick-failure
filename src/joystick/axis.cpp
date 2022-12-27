

#include "joystick.h"

#include "logging/logging.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

Axis::Axis(uint8_t gpioPin){

    this->gpioPin = gpioPin;

    adc_gpio_init(gpioPin);
}

/**
 * Read a value from the ADC
 *
 * Warning, this isn't thread safe. Only read one at a time.
 *
 * @return the value from the ADC
 */
uint16_t Axis::getValue() {

    adc_select_input(gpioPin - 26);
    uint16_t result = adc_read();

    debug("read value %d on gpio pin %d", result, gpioPin);

    return result;
}