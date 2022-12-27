

#include <FreeRTOS.h>
#include <task.h>


#include "tasks.h"

#include "pico/stdlib.h"


#include "logging/logging.h"



int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");


    // And fire up the tasks!
    vTaskStartScheduler();
}