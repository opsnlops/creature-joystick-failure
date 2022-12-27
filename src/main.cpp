

#include <FreeRTOS.h>
#include <task.h>


#include "tasks.h"



#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "logging/logging.h"
#include "joystick/joystick.h"


extern TaskHandle_t joystick_reader_task_handle;


int main() {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // Get the ADCs up and running
    adc_init();


    Axis* axis1 = new Axis(26);
    Axis* axis2 = new Axis(27);
    Joystick* joystick = new Joystick(axis1, axis2);

    xTaskCreate(joystick_reader_task,
                "joystick_reader_task",
                1512,
                (void*)joystick,
                1,
                &joystick_reader_task_handle);



    // And fire up the tasks!
    vTaskStartScheduler();



}