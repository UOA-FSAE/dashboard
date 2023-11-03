#include "energy_screen.h"
#include "styles.h"

lv_obj_t *energy_screen;    // energy screen objects

lv_obj_t * regen_power;
lv_obj_t * regen_label;
lv_obj_t * accumulator_power;
lv_obj_t * accumulator_text;
lv_obj_t * accumulator_label;
lv_obj_t * glv_power;
lv_obj_t * glv_text;
lv_obj_t * glv_label;
lv_obj_t * power_mode;
lv_obj_t * power_mode_label;

extern Vehicle_Data the_vehicle;

static lv_style_t accumulator_style;
static lv_style_t regen_style;    // Regen Style
static lv_style_t regular_text;

void init_energy_screen() {
    ///////////////////
    // Energy Screen //
    ///////////////////
    energy_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(energy_screen, lv_color_hex(0x01121f), LV_PART_MAIN);   // Energy Screen Init
    lv_obj_set_style_text_color(energy_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&regular_text);
    lv_style_set_text_font(&regular_text,&lv_font_montserrat_18);
    lv_style_set_text_align(&regular_text,LV_ALIGN_CENTER);

    lv_style_init(&regen_style);
    lv_style_set_bg_opa(&regen_style, LV_OPA_COVER);
    lv_style_set_bg_color(&regen_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_color(&regen_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&regen_style, LV_GRAD_DIR_VER);
    lv_style_set_radius(&regen_style,6);

    regen_power = lv_bar_create(energy_screen);
    lv_obj_set_style_radius(regen_power,6,LV_PART_MAIN);
    lv_obj_add_style(regen_power, &regen_style, LV_PART_INDICATOR);
    lv_obj_set_size(regen_power, 40, 160);
    lv_obj_align(regen_power, LV_ALIGN_LEFT_MID, 30, 10);
    lv_bar_set_range(regen_power, 0, 100);
    lv_bar_set_value(regen_power, 80, LV_ANIM_OFF);

    regen_label = lv_label_create(energy_screen);    // Regen Label
    lv_obj_add_style(regen_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(regen_label,"Regen");
    lv_obj_align(regen_label, LV_ALIGN_BOTTOM_LEFT, 20, -10);

    lv_style_init(&accumulator_style);
    lv_style_set_bg_opa(&accumulator_style, LV_OPA_COVER);
    lv_style_set_bg_color(&accumulator_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_radius(&accumulator_style, 3);

    accumulator_power = lv_bar_create(energy_screen);  // Accumulator Bar
    lv_obj_set_style_radius(accumulator_power,6,LV_PART_MAIN);
    lv_obj_add_style(accumulator_power, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(accumulator_power, 80, 160);
    lv_obj_align(accumulator_power, LV_ALIGN_CENTER, -70, 10);
    lv_bar_set_range(accumulator_power, 0, 100);
    lv_bar_set_value(accumulator_power, 80, LV_ANIM_OFF);

    accumulator_text = lv_label_create(energy_screen);     // Accumulator Live Text
    lv_obj_align(accumulator_text, LV_ALIGN_CENTER, -70, 0);
    lv_obj_set_style_text_font(accumulator_text,&lv_font_montserrat_26,LV_PART_MAIN);
    lv_label_set_text_fmt(accumulator_text, "%d %%", 80);

    accumulator_label = lv_label_create(energy_screen);    // Accumulator Label
    lv_obj_add_style(accumulator_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(accumulator_label,"Accumulator");
    lv_obj_align(accumulator_label, LV_ALIGN_BOTTOM_MID, -70, -10);

    glv_power = lv_bar_create(energy_screen);    // GLV Bar
    lv_obj_set_style_radius(glv_power,6,LV_PART_MAIN);
    lv_obj_add_style(glv_power, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(glv_power, 80, 140);
    lv_obj_align(glv_power, LV_ALIGN_CENTER, 70, 20);
    lv_bar_set_range(glv_power, 220, 294);
    lv_bar_set_value(glv_power, 0, LV_ANIM_OFF);

    glv_text = lv_label_create(energy_screen);     // GLV Live Text
    lv_obj_align(glv_text, LV_ALIGN_CENTER, 70, 20);
    lv_obj_set_style_text_font(glv_text,&lv_font_montserrat_26,LV_PART_MAIN);
    lv_label_set_text_fmt(glv_text, "%d V", 60);

    glv_label = lv_label_create(energy_screen);    // GLV Label
    lv_obj_add_style(glv_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(glv_label,"GLV");
    lv_obj_align(glv_label, LV_ALIGN_BOTTOM_MID, 70, -10);

    power_mode = lv_bar_create(energy_screen);     // Power Mode Bar
    lv_obj_set_style_radius(power_mode,6,LV_PART_MAIN);
    lv_obj_add_style(power_mode, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(power_mode, 30, 200);
    lv_obj_align(power_mode, LV_ALIGN_RIGHT_MID, -40, -10);
    lv_bar_set_range(power_mode, 0, 6);
    lv_bar_set_value(power_mode, 1, LV_ANIM_OFF);

    power_mode_label = lv_label_create(energy_screen);    // Power Mode Label
    lv_obj_add_style(power_mode_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(power_mode_label,"P-Mode");
    lv_obj_align(power_mode_label, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
}

void load_energy_screen() {
    lv_scr_load(energy_screen);
}

void update_energy_screen() {
    // TODO: Make bars red if below 20%
    lv_bar_set_value(regen_power, 0, LV_ANIM_OFF);  // TODO: CAN MESSAGE NEEDED
    lv_bar_set_value(accumulator_power, the_vehicle.ts.soc, LV_ANIM_OFF);
    lv_label_set_text_fmt(accumulator_text, "%d %%", the_vehicle.ts.soc);
    lv_bar_set_value(glv_power, (int32_t)(the_vehicle.glv.voltage*10), LV_ANIM_OFF);   // TODO: CAN MESSAGE NEEDED
    lv_label_set_text_fmt(glv_text, "%.1f V", (double)the_vehicle.glv.voltage);
    lv_bar_set_value(power_mode, 1, LV_ANIM_OFF);   // TODO: CAN MESSAGE NEEDED
}