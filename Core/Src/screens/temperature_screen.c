#include "temperature_screen.h"
#include "styles.h"

#define BAD_MOTOR_TEMP 60
#define OK_MOTOR_TEMP 50
#define BAD_INVERTER_TEMP 50
#define OK_INVERTER_TEMP 40
#define BAD_GEARBOX_TEMP 40
#define OK_GEARBOX_TEMP 30
#define BAD_MOTOR_COOLANT_TEMP 35
#define OK_MOTOR_COOLANT_TEMP 25
#define BAD_INVERTER_COOLANT_TEMP 40
#define OK_INVERTER_COOLANT_TEMP 30

lv_obj_t *temperature_screen;         // Debug Screen and Objects

lv_obj_t *motor_cooling_temp;
lv_obj_t *inverter_cooling_temp;

lv_obj_t *motor_cooling_temp_header;
lv_obj_t *inverter_cooling_temp_header;

lv_obj_t *fl_gearbox_heading;
lv_obj_t *fr_gearbox_heading;
lv_obj_t *rl_gearbox_heading;
lv_obj_t *rr_gearbox_heading;

lv_obj_t *gearbox_heading;
lv_obj_t *inverter_heading;
lv_obj_t *motor_heading;

lv_obj_t *fl_gearbox_temp;
lv_obj_t *fr_gearbox_temp;
lv_obj_t *rl_gearbox_temp;
lv_obj_t *rr_gearbox_temp;

lv_obj_t *fl_inverter_temp;
lv_obj_t *fr_inverter_temp;
lv_obj_t *rl_inverter_temp;
lv_obj_t *rr_inverter_temp;

lv_obj_t *fl_motor_temp;
lv_obj_t *fr_motor_temp;
lv_obj_t *rl_motor_temp;
lv_obj_t *rr_motor_temp;

lv_obj_t **gearbox_temp_map[4] = {&rl_gearbox_temp,&fl_gearbox_temp,&fr_gearbox_temp,&rr_gearbox_temp};
lv_obj_t **inverter_temp_map[4] = {&rl_inverter_temp,&fl_inverter_temp,&fr_inverter_temp,&rr_inverter_temp};
lv_obj_t **motor_temp_map[4] = {&rl_motor_temp,&fl_motor_temp,&fr_motor_temp,&rr_motor_temp};


extern Vehicle_Data the_vehicle;

void init_temperature_screen() {
    ////////////////////////
    // Temperature Screen //
    ////////////////////////

    temperature_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(temperature_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(temperature_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    gearbox_heading = lv_label_create(temperature_screen);
    lv_label_set_text(gearbox_heading, "Gearbox");
    lv_obj_set_style_text_align(gearbox_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(gearbox_heading, LV_ALIGN_TOP_RIGHT, -20, 10);

    inverter_heading = lv_label_create(temperature_screen);
    lv_label_set_text(inverter_heading, "Inverter");
    lv_obj_set_style_text_align(inverter_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(inverter_heading, LV_ALIGN_TOP_RIGHT, -110, 10);

    motor_heading = lv_label_create(temperature_screen);
    lv_label_set_text(motor_heading, "Motor");
    lv_obj_set_style_text_align(motor_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(motor_heading, LV_ALIGN_TOP_RIGHT, -200, 10);

    fl_gearbox_heading = lv_label_create(temperature_screen);
    lv_label_set_text(fl_gearbox_heading, "FL Motor: ");
    lv_obj_set_style_text_align(fl_gearbox_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fl_gearbox_heading, LV_ALIGN_TOP_LEFT, 110, 40);

    fr_gearbox_heading = lv_label_create(temperature_screen);
    lv_label_set_text(fr_gearbox_heading, "FR Motor: ");
    lv_obj_set_style_text_align(fr_gearbox_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fr_gearbox_heading, LV_ALIGN_TOP_LEFT, 110, 70);

    rl_gearbox_heading = lv_label_create(temperature_screen);
    lv_label_set_text(rl_gearbox_heading, "RL Motor: ");
    lv_obj_set_style_text_align(rl_gearbox_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rl_gearbox_heading, LV_ALIGN_TOP_LEFT, 110, 100);

    rr_gearbox_heading = lv_label_create(temperature_screen);
    lv_label_set_text(rr_gearbox_heading, "RR Motor: ");
    lv_obj_set_style_text_align(rr_gearbox_heading, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rr_gearbox_heading, LV_ALIGN_TOP_LEFT, 110, 130);

    // Gearbox Temps
    fl_gearbox_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fl_gearbox_temp, "69");
    lv_obj_set_style_text_align(fl_gearbox_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fl_gearbox_temp, LV_ALIGN_TOP_RIGHT, -20, 40);

    fr_gearbox_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fr_gearbox_temp, "69");
    lv_obj_set_style_text_align(fr_gearbox_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fr_gearbox_temp, LV_ALIGN_TOP_RIGHT, -20, 70);

    rl_gearbox_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rl_gearbox_temp, "69");
    lv_obj_set_style_text_align(rl_gearbox_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rl_gearbox_temp, LV_ALIGN_TOP_RIGHT, -20, 100);

    rr_gearbox_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rr_gearbox_temp, "69");
    lv_obj_set_style_text_align(rr_gearbox_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rr_gearbox_temp, LV_ALIGN_TOP_RIGHT, -20, 130);

    // Inverter Temps
    fl_inverter_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fl_inverter_temp, "69");
    lv_obj_set_style_text_align(fl_inverter_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fl_inverter_temp, LV_ALIGN_TOP_RIGHT, -110, 40);

    // Inverter Temps
    fr_inverter_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fr_inverter_temp, "69");
    lv_obj_set_style_text_align(fr_inverter_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fr_inverter_temp, LV_ALIGN_TOP_RIGHT, -110, 70);

    // Inverter Temps
    rl_inverter_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rl_inverter_temp, "69");
    lv_obj_set_style_text_align(rl_inverter_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rl_inverter_temp, LV_ALIGN_TOP_RIGHT, -110, 100);

    // Inverter Temps
    rr_inverter_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rr_inverter_temp, "69");
    lv_obj_set_style_text_align(rr_inverter_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rr_inverter_temp, LV_ALIGN_TOP_RIGHT, -110, 130);

    // Motor Temps
    fl_motor_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fl_motor_temp, "69");
    lv_obj_set_style_text_align(fl_motor_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fl_motor_temp, LV_ALIGN_TOP_RIGHT, -200, 40);

    fr_motor_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(fr_motor_temp, "69");
    lv_obj_set_style_text_align(fr_motor_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(fr_motor_temp, LV_ALIGN_TOP_RIGHT, -200, 70);

    rl_motor_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rl_motor_temp, "69");
    lv_obj_set_style_text_align(rl_motor_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rl_motor_temp, LV_ALIGN_TOP_RIGHT, -200, 100);

    rr_motor_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(rr_motor_temp, "69");
    lv_obj_set_style_text_align(rr_motor_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(rr_motor_temp, LV_ALIGN_TOP_RIGHT, -200, 130);


    // Motor Cooling Stuff
    motor_cooling_temp_header = lv_label_create(temperature_screen);
    lv_label_set_text(motor_cooling_temp_header, "Motor Cooling Temperature: ");
    lv_obj_set_style_text_align(motor_cooling_temp_header, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(motor_cooling_temp_header, LV_ALIGN_TOP_LEFT, 20, 180);

    inverter_cooling_temp_header = lv_label_create(temperature_screen);
    lv_label_set_text(inverter_cooling_temp_header, "Inverter Cooling Temperature: ");
    lv_obj_set_style_text_align(inverter_cooling_temp_header, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(inverter_cooling_temp_header, LV_ALIGN_TOP_LEFT, 20, 210);

    motor_cooling_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(motor_cooling_temp, "69");
    lv_obj_set_style_text_align(motor_cooling_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(motor_cooling_temp, LV_ALIGN_TOP_LEFT, 270, 180);

    inverter_cooling_temp = lv_label_create(temperature_screen);
    lv_label_set_text_fmt(inverter_cooling_temp, "69");
    lv_obj_set_style_text_align(inverter_cooling_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(inverter_cooling_temp, LV_ALIGN_TOP_LEFT, 270, 210);
}

void load_temperature_screen() {
    lv_scr_load(temperature_screen);
}

void update_temperature_screen() {
    for (int i = 0;i<4;i++) {
        lv_label_set_text_fmt(*gearbox_temp_map[i],"%d", the_vehicle.drive[i].gearboxTemp);
        if (the_vehicle.drive[i].gearboxTemp < OK_GEARBOX_TEMP) {
            lv_obj_set_style_text_color(*gearbox_temp_map[i],lv_palette_lighten(LV_PALETTE_GREEN,2),LV_PART_MAIN);
        } else if (the_vehicle.drive[i].gearboxTemp < BAD_GEARBOX_TEMP) {
            lv_obj_set_style_text_color(*gearbox_temp_map[i],lv_palette_lighten(LV_PALETTE_YELLOW,2),LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(*gearbox_temp_map[i],lv_palette_lighten(LV_PALETTE_RED,2),LV_PART_MAIN);
        }
    }

    for (int i = 0;i<4;i++) {
        lv_label_set_text_fmt(*inverter_temp_map[i],"%d", the_vehicle.drive[i].inverterTemp);
        if (the_vehicle.drive[i].inverterTemp < OK_INVERTER_TEMP) {
            lv_obj_set_style_text_color(*inverter_temp_map[i],lv_palette_lighten(LV_PALETTE_GREEN,2),LV_PART_MAIN);
        } else if (the_vehicle.drive[i].inverterTemp < BAD_INVERTER_TEMP) {
            lv_obj_set_style_text_color(*inverter_temp_map[i],lv_palette_lighten(LV_PALETTE_YELLOW,2),LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(*inverter_temp_map[i],lv_palette_lighten(LV_PALETTE_RED,2),LV_PART_MAIN);
        }
    }

    for (int i = 0;i<4;i++) {
        lv_label_set_text_fmt(*motor_temp_map[i],"%d", the_vehicle.drive[i].motorTemp);
        if (the_vehicle.drive[i].motorTemp < OK_MOTOR_TEMP) {
            lv_obj_set_style_text_color(*motor_temp_map[i],lv_palette_lighten(LV_PALETTE_GREEN,2),LV_PART_MAIN);
        } else if (the_vehicle.drive[i].motorTemp < BAD_MOTOR_TEMP) {
            lv_obj_set_style_text_color(*motor_temp_map[i],lv_palette_lighten(LV_PALETTE_YELLOW,2),LV_PART_MAIN);
        } else {
            lv_obj_set_style_text_color(*motor_temp_map[i],lv_palette_lighten(LV_PALETTE_RED,2),LV_PART_MAIN);
        }
    }

    lv_label_set_text_fmt(motor_cooling_temp, "%d",the_vehicle.driver.motor_cooling_temp);
    if (the_vehicle.driver.motor_cooling_temp < OK_MOTOR_COOLANT_TEMP) {
        lv_obj_set_style_text_color(motor_cooling_temp,lv_palette_lighten(LV_PALETTE_GREEN,2),LV_PART_MAIN);
    } else if (the_vehicle.driver.motor_cooling_temp < BAD_MOTOR_COOLANT_TEMP) {
        lv_obj_set_style_text_color(motor_cooling_temp,lv_palette_lighten(LV_PALETTE_YELLOW,2),LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(motor_cooling_temp,lv_palette_lighten(LV_PALETTE_RED,2),LV_PART_MAIN);
    }

    lv_label_set_text_fmt(inverter_cooling_temp, "%d",the_vehicle.driver.inverter_cooling_temp);
    if (the_vehicle.driver.inverter_cooling_temp < OK_INVERTER_COOLANT_TEMP) {
        lv_obj_set_style_text_color(inverter_cooling_temp,lv_palette_lighten(LV_PALETTE_GREEN,2),LV_PART_MAIN);
    } else if (the_vehicle.driver.inverter_cooling_temp < BAD_INVERTER_COOLANT_TEMP) {
        lv_obj_set_style_text_color(inverter_cooling_temp,lv_palette_lighten(LV_PALETTE_YELLOW,2),LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(inverter_cooling_temp,lv_palette_lighten(LV_PALETTE_RED,2),LV_PART_MAIN);
    }
}