#include "error_screen.h"
#include "styles.h"

extern Vehicle_Data the_vehicle;

lv_obj_t *error_screen;

lv_obj_t *error_text;

static lv_style_t error_style;

void init_error_screen(void) {
    lv_style_init(&error_style);
    lv_style_set_text_font(&error_style,&rubik_mono_one_regular_46);
    lv_style_set_text_align(&error_style,LV_ALIGN_CENTER);

    error_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(error_screen, lv_color_darken(lv_palette_main(LV_PALETTE_RED),2), LV_PART_MAIN);   // Energy Screen Init
    lv_obj_set_style_text_color(error_screen, lv_color_white(), LV_PART_MAIN);

    error_text = lv_label_create(error_screen);
    lv_obj_align(error_text,LV_ALIGN_CENTER,0,0);
    lv_obj_add_style(error_text,&error_style,LV_PART_MAIN);
    lv_label_set_text(error_text,"GENERIC ERROR");
}

void load_error_screen(void) {
    lv_scr_load(error_screen);
}

void update_error_screen(void) {
    // Put error in here, but for now:
    if (the_vehicle.errors.AMS) {
        lv_label_set_text(error_text,"!AMS ERROR!");
    } else if (the_vehicle.errors.BSPD) {
        lv_label_set_text(error_text,"!BSPD ERROR!");
    } else if (the_vehicle.errors.IMD) {
        lv_label_set_text(error_text,"!IMD ERROR!");
    } else if (the_vehicle.errors.PDOC) {
        lv_label_set_text(error_text,"!PDOC ERROR!");
    }
}