#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <zephyr.h>

#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0 ""
#define PIN 0
#define FLAGS 0
#endif

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(app);

static void on_gui_event(gui_event_t *event)
{
    //currently empty
}

void main(void)
{
    gui_config_t gui_config = {.event_callback = on_gui_event};
    gui_init(&gui_config);
    gui_set_bt_state(GUI_BT_STATE_ADVERTISING);

    // LED blinking Part
    const struct device *dev;
    bool led_is_on = true;
    int ret;

    dev = device_get_binding(LED0);
    if (dev == NULL) {
        return;
    }

    ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
    if (ret < 0) {
        return;
    }
    // ENDOF LED blinking Part

    while (1)
    {
        // LED blinking
        gpio_pin_set(dev, PIN, (int)led_is_on);
        led_is_on = !led_is_on;
        // sleep
        k_sleep(K_MSEC(SLEEP_TIME_MS));
    }
}