

#include "controller-config.h"

#include <FreeRTOS.h>
#include <task.h>



TaskHandle_t joystick_reader_task_handle;
TaskHandle_t log_queue_reader_task_handle;
