#include "driver_screen.h"
#include "styles.h"

lv_obj_t *driver_screen;        // Driver Screen Objects

lv_obj_t * rpm_arc;
lv_obj_t * throttle_arc;
lv_obj_t * rpm_label;
lv_obj_t * throttle_label;

extern Vehicle_Data the_vehicle;

void init_driver_screen() {
    //////////////////
    // Driver Screen//
    //////////////////
    driver_screen = lv_obj_create(NULL);    // Driver Screen Init
    lv_obj_set_style_bg_color(driver_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(driver_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&speedometer_style);    // Speedometer foreground style
    lv_style_set_arc_rounded(&speedometer_style,false);
    lv_style_set_arc_width(&speedometer_style,40);

    rpm_label = lv_label_create(driver_screen);     // RPM Label
    lv_obj_align(rpm_label, LV_ALIGN_TOP_LEFT, 76, 136+50);
    lv_obj_set_style_bg_color(rpm_label,lv_palette_main(LV_PALETTE_GREEN),LV_PART_MAIN);
    lv_obj_add_style(rpm_label,&colored_label_style,LV_PART_MAIN);

    throttle_label = lv_label_create(driver_screen);    // THROTTLE Label
    lv_obj_align(throttle_label, LV_ALIGN_TOP_LEFT, 50+240, 136+50);
    lv_obj_set_style_bg_color(throttle_label,lv_palette_main(LV_PALETTE_GREEN),LV_PART_MAIN);
    lv_obj_add_style(throttle_label,&colored_label_style,LV_PART_MAIN);

    rpm_arc = lv_arc_create(driver_screen);   // RPM Arc
    lv_arc_set_bg_angles(rpm_arc, 150, 30);
    lv_arc_set_angles(rpm_arc,150,-20);
    lv_obj_set_size(rpm_arc, 180, 180);

    lv_obj_add_style(rpm_arc,&speedometer_style,LV_PART_INDICATOR);
    lv_obj_add_style(rpm_arc,&speedometer_style,LV_PART_MAIN);
    lv_obj_set_style_arc_color(rpm_arc,lv_palette_main(LV_PALETTE_DEEP_ORANGE),LV_PART_INDICATOR);
    lv_obj_remove_style(rpm_arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(rpm_arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(rpm_arc, LV_ALIGN_LEFT_MID, 40, 0);

    throttle_arc = lv_arc_create(driver_screen);    // THROTTLE Arc
    lv_arc_set_bg_angles(throttle_arc, 150, 30);
    lv_arc_set_angles(throttle_arc,150,-20);
    lv_obj_set_size(throttle_arc, 180, 180);

    lv_obj_add_style(throttle_arc,&speedometer_style,LV_PART_INDICATOR);
    lv_obj_add_style(throttle_arc,&speedometer_style,LV_PART_MAIN);
    lv_obj_set_style_arc_color(throttle_arc,lv_palette_main(LV_PALETTE_DEEP_PURPLE),LV_PART_INDICATOR);
    lv_obj_remove_style(throttle_arc, NULL, LV_PART_KNOB);   // Be sure the knob is not displayed
    lv_obj_clear_flag(throttle_arc, LV_OBJ_FLAG_CLICKABLE);  // To not allow adjusting by click
    lv_obj_align(throttle_arc, LV_ALIGN_RIGHT_MID, -40, 0);
}

void load_driver_screen() {
    lv_scr_load(driver_screen);
}

void update_driver_screen() {
    // Update Driver Screen
    lv_label_set_text_fmt(throttle_label, "Throttle: %d %%", the_vehicle.driver.throttle);
    lv_label_set_text_fmt(rpm_label, "RPM: %d", the_vehicle.driver.rpm);
    lv_arc_set_angles(rpm_arc, 150, 150+(240*(int)the_vehicle.driver.rpm/20000));
    lv_arc_set_angles(throttle_arc, 150, 150+(240*(int)the_vehicle.driver.throttle)/100);
}