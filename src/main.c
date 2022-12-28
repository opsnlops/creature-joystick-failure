
#ifdef __cplusplus
extern "C"
{
#endif


#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>


#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "logging/logging.h"
//#include "joystick/joystick.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "common/tusb_common.h"
#include "device/usbd.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"


// Increase stack size when debug log is enabled
#define USBD_STACK_SIZE    (3*configMINIMAL_STACK_SIZE/2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define HID_STACK_SZIE      configMINIMAL_STACK_SIZE


enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

StaticTimer_t blinky_tmdef;
StackType_t usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;
TimerHandle_t blinky_tm;
StackType_t hid_stack[HID_STACK_SZIE];
StaticTask_t hid_taskdef;


void led_blinky_cb(TimerHandle_t xTimer);
void usb_device_task(void *param);
void hid_task(void *params);



int main(void) {

    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    //stdio_init_all();
    board_init();
    logger_init();
    debug("Logging running!");

    // Get the ADCs up and running
    adc_init();

    blinky_tm = xTimerCreateStatic(NULL,
                                   pdMS_TO_TICKS(BLINK_NOT_MOUNTED),
                                   true,
                                   NULL,
                                   led_blinky_cb,
                                   &blinky_tmdef);

    // Create a task for tinyusb device stack
    xTaskCreateStatic(usb_device_task,
                      "usbd",
                      USBD_STACK_SIZE,
                      NULL,
                      configMAX_PRIORITIES - 1,
                      usb_device_stack,
                      &usb_device_taskdef);

    // Create HID task
    xTaskCreateStatic(hid_task,
                      "hid",
                      HID_STACK_SZIE,
                      NULL,
                      configMAX_PRIORITIES - 2,
                      hid_stack,
                      &hid_taskdef);

    // And fire up the tasks!
    vTaskStartScheduler();

}


// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void usb_device_task(void *param) {
    (void) param;

    tusb_init();

    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    //tud_init(BOARD_TUD_RHPORT);

    // RTOS forever loop
    while (1) {
        // put this thread to waiting state until there is new events
        tud_task();

        // following code only run if tud_task() process at least 1 event
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn) {
    // skip if hid is not ready yet
    if (!tud_hid_ready()) return;

    switch (report_id) {

        case REPORT_ID_GAMEPAD: {
            // use to avoid send multiple consecutive zero report for keyboard
            static bool has_gamepad_key = false;

            hid_gamepad_report_t report =
                    {
                            .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
                            .hat = 0, .buttons = 0
                    };

            if (btn) {
                report.hat = GAMEPAD_HAT_UP;
                report.buttons = GAMEPAD_BUTTON_A;
                tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

                has_gamepad_key = true;
            } else {
                report.hat = GAMEPAD_HAT_CENTERED;
                report.buttons = 0;
                if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
                has_gamepad_key = false;
            }
        }
            break;

        default:
            break;
    }
}

void hid_task(void *param) {
    (void) param;

    while (1) {
        // Poll every 10ms
        vTaskDelay(pdMS_TO_TICKS(10));

        uint32_t const btn = board_button_read();

        // Remote wakeup
        if (tud_suspended() && btn) {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        } else {
            // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
            send_hid_report(REPORT_ID_KEYBOARD, btn);
        }
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, /*uint16_t*/ uint8_t len) {
    (void) instance;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT) {
        send_hid_report(next_report_id, board_button_read());
    }
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1) return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On: disable blink, turn led on
                xTimerStop(blinky_tm, portMAX_DELAY);
                board_led_write(true);
            } else {
                // Caplocks Off: back to normal blink
                board_led_write(false);
                xTimerStart(blinky_tm, portMAX_DELAY);
            }
        }
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinky_cb(TimerHandle_t xTimer) {
    (void) xTimer;
    static bool led_state = false;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

#ifdef __cplusplus
}
#endif