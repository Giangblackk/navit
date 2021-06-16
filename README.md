# Navit

### Overview
Motocycle Navigation App
### Requirements

The example is tested using the [adafruit_2_8_tft_touch_v2](https://docs.zephyrproject.org/latest/boards/shields/adafruit_2_8_tft_touch_v2/doc/index.html) display, and one of the following boards:

- nrf52840dk_nrf52840

- nrf5340dk_nrf5340_cpuapp 

NOTE: The current version of the nrf5340 board files in Zephyr does not fully support the Arduino definitions required by the adafruit display. In order to correct this a patch is included, which can be applied as follows:
  1) Move the nrf5340_display.patch file from this repo into your Zephyr base directory
  2) From the Unix bash (or Git command line in Windows) run *"git apply nrf5340_display.patch"*

### Building and Running

Build the example as normal (setting the shield explicitly is not necessary, as this is configured in the CMakeLists.txt file):
*west build -b nrf5340dk_nrf5340_cpuapp

### LVGL documentation

The full documentation for the LVGL library can be found [here](https://docs.lvgl.io/latest/en/html/index.html).

For generating images and fonts for use with LVGL please refer to the [Online Image Converter](https://lvgl.io/tools/imageconverter) and the [Online Font Converter](https://lvgl.io/tools/fontconverter). The example includes a couple of images and fonts generated using these tools. 