#include "lap_screen.h"
#include "screens.h"
#include "styles.h"

lv_obj_t * lap_screen;  //  Lap time screen objects

lv_obj_t * lap_screen_rpm;
lv_obj_t * lap_rpm_text;
lv_obj_t * current_lap_time;
lv_obj_t * lap_delta;
lv_obj_t * best_lap_time;
lv_obj_t * previous_lap_time;
lv_obj_t * lap_number;
lv_obj_t * lap_label;

extern Vehicle_Data the_vehicle;

static lv_style_t text_box_style;
static lv_style_t rpm_style;

void init_lap_screen() {

    //////////////////////
    //  Lap Time Screen //
    //////////////////////
    lap_screen = lv_obj_create(NULL);    // Lap screen init
    lv_obj_set_style_bg_color(lap_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(lap_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&rpm_style);
    lv_style_set_bg_opa(&rpm_style, LV_OPA_COVER);
    lv_style_set_bg_color(&rpm_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&rpm_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_dir(&rpm_style, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&rpm_style,3);

    lv_style_init(&text_box_style);

    lv_style_set_radius(&text_box_style, 0);
    lv_style_set_outline_width(&text_box_style, 5);
    lv_style_set_outline_color(&text_box_style, lv_palette_main(LV_PALETTE_BLUE_GREY));
    lv_style_set_outline_pad(&text_box_style, 10);

    lap_screen_rpm = lv_bar_create(lap_screen);  // RPM Bar
    lv_obj_set_style_radius(lap_screen_rpm,3,LV_PART_MAIN);
    lv_obj_add_style(lap_screen_rpm, &rpm_style, LV_PART_INDICATOR);
    lv_obj_set_size(lap_screen_rpm, 262, 40);
    lv_obj_align(lap_screen_rpm, LV_ALIGN_TOP_LEFT, 25, 10);
    lv_bar_set_range(lap_screen_rpm, 0, 20000);
    lv_bar_set_value(lap_screen_rpm, 0, LV_ANIM_OFF);

    lap_rpm_text = lv_label_create(lap_screen);    // Current lap time
    lv_obj_set_style_text_font(lap_rpm_text,&bitstream_vera_sans_30,LV_PART_MAIN);
    lv_obj_align(lap_rpm_text, LV_ALIGN_TOP_LEFT,300, 20);
    lv_label_set_text_fmt(lap_rpm_text,"%d",the_vehicle.driver.rpm);

    current_lap_time = lv_label_create(lap_screen);    // Current lap time
    lv_obj_add_style(current_lap_time,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(current_lap_time,&bitstream_vera_sans_30,LV_PART_MAIN);

    uint32_t minutes = the_vehicle.race.currentLapTime/60000;
    uint32_t seconds = the_vehicle.race.currentLapTime/1000-60*minutes;
    lv_label_set_text_fmt(current_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.currentLapTime%1000);

    lv_obj_align(current_lap_time, LV_ALIGN_TOP_LEFT,40, 80);

    lap_delta = lv_label_create(lap_screen);    // Lap Delta
    lv_obj_add_style(lap_delta,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(lap_delta,&bitstream_vera_sans_30,LV_PART_MAIN);
    lv_label_set_text_fmt(lap_delta,"%.2f",0.001 * the_vehicle.race.deltaLapTime);
    lv_obj_align(lap_delta, LV_ALIGN_TOP_RIGHT,-40, 80);

    best_lap_time = lv_label_create(lap_screen);    // Best Lap Time
    lv_obj_add_style(best_lap_time,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(best_lap_time,&bitstream_vera_sans_26,LV_PART_MAIN);
    lv_obj_set_style_text_color(best_lap_time,lv_palette_lighten(LV_PALETTE_PURPLE,2),LV_PART_MAIN);

    minutes = the_vehicle.race.bestLapTime/60000;
    seconds = the_vehicle.race.bestLapTime/1000-60*minutes;
    lv_label_set_text_fmt(best_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.bestLapTime%1000);

    lv_obj_align(best_lap_time, LV_ALIGN_TOP_LEFT,40, 140);

    previous_lap_time = lv_label_create(lap_screen);    // Previous Lap Time
    lv_obj_add_style(previous_lap_time, &text_box_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(previous_lap_time, &bitstream_vera_sans_26, LV_PART_MAIN);

    minutes = the_vehicle.race.previousLapTime/60000;
    seconds = the_vehicle.race.previousLapTime/1000-60*minutes;
    lv_label_set_text_fmt(previous_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.previousLapTime%1000);

    lv_obj_align(previous_lap_time, LV_ALIGN_TOP_LEFT, 40, 200);

    lap_number = lv_label_create(lap_screen);    // Lap Number
    lv_obj_add_style(lap_number,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(lap_number,&bitstream_vera_sans_80,LV_PART_MAIN);
    lv_label_set_text_fmt(lap_number,"%02d ",the_vehicle.race.lapNumber);
    lv_obj_align(lap_number, LV_ALIGN_TOP_RIGHT,-40, 158);

    lap_label = lv_label_create(lap_screen);    // Laps Text
    lv_obj_set_style_text_font(lap_label,&lv_font_montserrat_18,LV_PART_MAIN);
    lv_label_set_text(lap_label,"laps");
    lv_obj_align(lap_label, LV_ALIGN_TOP_RIGHT,-45, 198);
}

void load_lap_screen() {
    lv_scr_load(lap_screen);
}

void update_lap_screen() {
    uint32_t minutes;
    uint32_t seconds;
    uint32_t milliseconds;

    lv_bar_set_value(lap_screen_rpm, the_vehicle.race.rpmAverage, LV_ANIM_OFF);
    lv_label_set_text_fmt(lap_rpm_text,"%d",the_vehicle.race.rpmAverage);

    ms_to_minutes_seconds(the_vehicle.race.currentLapTime,&minutes,&seconds,&milliseconds);
    lv_label_set_text_fmt(current_lap_time,"%02d : %02d : %03d",minutes,seconds,milliseconds);

    lv_label_set_text_fmt(lap_delta,"%.2f",the_vehicle.race.deltaLapTime);

    if(the_vehicle.race.deltaLapTime > 0.0){
    	lv_obj_set_style_text_color(lap_delta,lv_palette_lighten(LV_PALETTE_RED,1),LV_PART_MAIN);
    }
    else{
    	lv_obj_set_style_text_color(lap_delta,lv_palette_lighten(LV_PALETTE_LIGHT_GREEN,2),LV_PART_MAIN);
    }

    ms_to_minutes_seconds(the_vehicle.race.bestLapTime,&minutes,&seconds,&milliseconds);
    lv_label_set_text_fmt(best_lap_time,"%02d : %02d : %03d",minutes,seconds,milliseconds);

    ms_to_minutes_seconds(the_vehicle.race.previousLapTime,&minutes,&seconds,&milliseconds);
    lv_label_set_text_fmt(previous_lap_time,"%02d : %02d : %03d",minutes,seconds,milliseconds);

    lv_label_set_text_fmt(lap_number,"%02d ",the_vehicle.race.lapNumber);
}
