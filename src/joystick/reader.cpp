

#include "controller-config.h"

#include <FreeRTOS.h>
#include <task.h>

#include "joystick.h"
#include "logging/logging.h"


portTASK_FUNCTION(joystick_reader_task, pvParameters) {

    auto joystick = (Joystick*)pvParameters;

    uint16_t x = 0;
    uint16_t y = 0;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        x = joystick->getReadingAxis1();
        y = joystick->getReadingAxis2();


        info("Reading: x: %d, y: %d", x, y);

        vTaskDelay(pdMS_TO_TICKS(500));

    }

#pragma clang diagnostic pop

}
