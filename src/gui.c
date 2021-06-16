#include "gui.h"
#include <zephyr.h>
#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(gui);

uint32_t count = 0U;
char count_str[11] = {0};
const struct device *display_dev;

static gui_event_t m_gui_event;
static gui_callback_t m_gui_callback = 0;

// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_gui_cmd_queue, sizeof(gui_message_t), 8, 4);

// Define a timer to update the GUI periodically
static void on_gui_blink_timer(struct k_timer *dummy);
K_TIMER_DEFINE(gui_blink_timer, on_gui_blink_timer, NULL);

// GUI objects
lv_obj_t *top_header;

// Styles
lv_style_t style_btn, style_label, style_label_value, style_checkbox;
lv_style_t style_header, style_con_bg;

// Fonts
LV_FONT_DECLARE(arial_20bold);
LV_FONT_DECLARE(calibri_20b);
LV_FONT_DECLARE(calibri_20);
LV_FONT_DECLARE(calibri_24b);
LV_FONT_DECLARE(calibri_32b);

static void init_styles(void)
{
    /*Create background style*/
    static lv_style_t style_screen;
    lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xcb, 0xca, 0xff));
    lv_obj_add_style(lv_scr_act(), LV_BTN_PART_MAIN, &style_screen);

    /*Create the screen header label style*/
    lv_style_init(&style_header);
    lv_style_set_bg_opa(&style_header, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x1C, 0x11, 0xFD));
    lv_style_set_radius(&style_header, LV_STATE_DEFAULT, 8);
    //lv_style_set_bg_grad_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_TEAL);
    //lv_style_set_bg_grad_dir(&style_header, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_pad_left(&style_header, LV_STATE_DEFAULT, 70);
    lv_style_set_pad_top(&style_header, LV_STATE_DEFAULT, 30);
    lv_style_set_shadow_spread(&style_header, LV_STATE_DEFAULT, 1);
    lv_style_set_shadow_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_shadow_opa(&style_header, LV_STATE_DEFAULT, 255);
    lv_style_set_shadow_width(&style_header, LV_STATE_DEFAULT, 1);
    lv_style_set_shadow_ofs_x(&style_header, LV_STATE_DEFAULT, 1);
    lv_style_set_shadow_ofs_y(&style_header, LV_STATE_DEFAULT, 2);
    lv_style_set_shadow_opa(&style_header, LV_STATE_DEFAULT, LV_OPA_50);

    /*Screen header text style*/
    lv_style_set_text_color(&style_header, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x7d, 0xce, 0xfd));
    lv_style_set_text_font(&style_header, LV_STATE_DEFAULT, &calibri_32b);
    

    lv_style_init(&style_con_bg);
    lv_style_copy(&style_con_bg, &style_header);
    lv_style_set_bg_color(&style_con_bg, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x69, 0xb0, 0x5a));
    lv_style_set_bg_opa(&style_con_bg, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_radius(&style_header, LV_STATE_DEFAULT, 4);
    
    /*Create a label style*/
    lv_style_init(&style_label);
    lv_style_set_bg_opa(&style_label, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_bg_grad_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_grad_dir(&style_label, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 5);
    lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 10);

    /*Add a border*/
    lv_style_set_border_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_opa(&style_label, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 3);

    /*Set the text style*/
    lv_style_set_text_color(&style_label, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x00, 0x30));
    lv_style_set_text_font(&style_label, LV_STATE_DEFAULT, &calibri_20b);


    /*Create a label value style*/
    lv_style_init(&style_label_value);
    lv_style_set_bg_opa(&style_label_value, LV_STATE_DEFAULT, LV_OPA_20);
    lv_style_set_bg_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_bg_grad_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_TEAL);
    lv_style_set_bg_grad_dir(&style_label_value, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_pad_left(&style_label_value, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&style_label_value, LV_STATE_DEFAULT, 3);

    /*Set the text style*/
    lv_style_set_text_color(&style_label_value, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x00, 0x30));
    lv_style_set_text_font(&style_label_value, LV_STATE_DEFAULT, &calibri_20);


    /*Create a simple button style*/
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_bg_grad_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_bg_grad_dir(&style_btn, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
    lv_style_set_shadow_spread(&style_btn, LV_STATE_DEFAULT, 1);
    lv_style_set_shadow_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_style_set_shadow_opa(&style_btn, LV_STATE_DEFAULT, 255);
    lv_style_set_shadow_width(&style_btn, LV_STATE_DEFAULT, 1);

    /*Swap the colors in pressed state*/
    lv_style_set_bg_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_grad_color(&style_btn, LV_STATE_PRESSED, LV_COLOR_SILVER);

    /*Add a border*/
    lv_style_set_border_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 3);

    /*Different border color in focused state*/
    lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_border_color(&style_btn, LV_STATE_FOCUSED | LV_STATE_PRESSED, LV_COLOR_NAVY);

    /*Set the text style*/
    lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, LV_COLOR_TEAL);
    lv_style_set_text_font(&style_btn, LV_STATE_DEFAULT, &calibri_24b);

    /*Make the button smaller when pressed*/
    lv_style_set_transform_height(&style_btn, LV_STATE_PRESSED, -4);
    lv_style_set_transform_width(&style_btn, LV_STATE_PRESSED, -8);
#if LV_USE_ANIMATION
    /*Add a transition to the size change*/
    static lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, lv_anim_path_overshoot);

    lv_style_set_transition_prop_1(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_HEIGHT);
    lv_style_set_transition_prop_2(&style_btn, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_WIDTH);
    lv_style_set_transition_time(&style_btn, LV_STATE_DEFAULT, 300);
    lv_style_set_transition_path(&style_btn, LV_STATE_DEFAULT, &path);
#endif
}

void gui_init(gui_config_t * config)
{
    m_gui_callback = config->event_callback;
}

void gui_set_bt_state(gui_bt_state_t state)
{
    static gui_message_t set_bt_state_msg;
    set_bt_state_msg.type = GUI_MSG_SET_BT_STATE;
    set_bt_state_msg.params.bt_state = state;
    k_msgq_put(&m_gui_cmd_queue, &set_bt_state_msg, K_NO_WAIT);
}

static void process_cmd_msg_queue(void)
{
    // empty for now
}

static void init_blinky_gui(void)
{
    top_header = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(top_header, LV_LABEL_PART_MAIN, &style_header);
    lv_label_set_long_mode(top_header, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(top_header, 3, 3);
    lv_obj_set_size(top_header, 314, 68);
    lv_label_set_text(top_header, "nRF Blinky");
    //lv_label_set_align(top_header, LV_LABEL_ALIGN_CENTER);
}

void gui_run(void)
{
	display_dev = device_get_binding(CONFIG_LVGL_DISPLAY_DEV_NAME);

	if (display_dev == NULL) {
		LOG_ERR("Display device not found!");
		return;
	}

	init_styles();

	init_blinky_gui();

	display_blanking_off(display_dev);

	while(1){
		process_cmd_msg_queue();
		lv_task_handler();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(gui_thread, 4096, gui_run, NULL, NULL, NULL, 7, 0, 0);