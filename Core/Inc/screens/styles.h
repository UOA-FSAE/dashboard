#ifndef DASHBOARD_SIMULATOR_STYLES_H
#define DASHBOARD_SIMULATOR_STYLES_H

#include <lvgl.h>
#include "vehicle.h"

static lv_style_t regular_text;
static lv_style_t accumulator_style;
static lv_style_t regen_style;    // Regen Style
static lv_style_t text_box_style; // Text box style (for border)
static lv_style_t rpm_style;    // RPM style
static lv_style_t speedometer_style;
static lv_style_t colored_label_style;

LV_FONT_DECLARE(bitstream_vera_sans_26)
LV_FONT_DECLARE(bitstream_vera_sans_30)
LV_FONT_DECLARE(bitstream_vera_sans_80)

#endif //DASHBOARD_SIMULATOR_STYLES_H
