
#include <screens.h>
#include <lvgl.h>
#include <vehicle.h>

extern Vehicle_Data the_vehicle;

enum SCREENS current_screen;

// Create screens
// Driver Screen and Objects
lv_obj_t * driver_screen;
lv_obj_t * spinner;
lv_obj_t * ds_glv_voltage;
lv_obj_t * ds_glv_soc;

lv_obj_t * dbs_spinner;
lv_obj_t * dbs_throttle;
lv_obj_t * dbs_rpm;


// Debug Screen and Objects
lv_obj_t * debug_screen;

void init_screens() {
	// Driver Screen
	driver_screen = lv_obj_create(NULL);
	lv_scr_load(driver_screen);
	current_screen = DRIVER_SCREEN;

	lv_obj_set_style_bg_color(driver_screen, lv_color_hex(0x003a57), LV_PART_MAIN);
	lv_obj_set_style_text_color(driver_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

	spinner = lv_spinner_create(driver_screen, 1000, 60);
	lv_obj_set_size(spinner, 64, 64);
	lv_obj_align(spinner, LV_ALIGN_BOTTOM_MID, 0, 0);

	// Labels
	ds_glv_voltage = lv_label_create(driver_screen);
	lv_label_set_text_fmt(ds_glv_voltage, "GLV Voltage: %.3f V", the_vehicle.glv.voltage);
	lv_obj_set_style_text_align(ds_glv_voltage, LV_ALIGN_TOP_MID,0);
	lv_obj_align(ds_glv_voltage, LV_ALIGN_CENTER, 0, 10);


	ds_glv_soc = lv_label_create(driver_screen);
	lv_label_set_text_fmt(ds_glv_soc, "SOC: %d%%", the_vehicle.glv.soc);
	lv_obj_set_style_text_align(ds_glv_soc, LV_ALIGN_TOP_MID,0);
	lv_obj_align(ds_glv_soc, LV_ALIGN_CENTER, 0, 20);

	//////////////////
	// Debug Screen //
	//////////////////

	debug_screen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(debug_screen, lv_color_hex(0x003a57), LV_PART_MAIN);
	lv_obj_set_style_text_color(debug_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

	// Labels
	dbs_throttle = lv_label_create(debug_screen);
	lv_label_set_text_fmt(dbs_throttle, "GLV Voltage: %d", the_vehicle.driver.throttle);
	lv_obj_set_style_text_align(dbs_throttle, LV_ALIGN_TOP_LEFT,0);
	lv_obj_align(dbs_throttle, LV_ALIGN_TOP_LEFT, 0, 10);


	dbs_rpm = lv_label_create(debug_screen);
	lv_label_set_text_fmt(dbs_rpm, "SOC: %d rpm", the_vehicle.driver.rpm);
	lv_obj_set_style_text_align(dbs_rpm, LV_ALIGN_TOP_LEFT,0);
	lv_obj_align(dbs_rpm, LV_ALIGN_TOP_LEFT, 0, 20);
}

void update_screen() {

	if (lv_scr_act() == driver_screen) {
		// Update Driver Screen
		lv_label_set_text_fmt(ds_glv_voltage, "GLV Voltage: %.3f V", the_vehicle.glv.voltage);
		lv_label_set_text_fmt(ds_glv_soc, "SOC: %d%%", the_vehicle.glv.soc);
	} else if (lv_scr_act() == debug_screen) {
		// Update Debug Screen
		lv_obj_set_style_text_align(dbs_throttle, LV_ALIGN_TOP_LEFT,0);
		lv_obj_align(dbs_throttle, LV_ALIGN_TOP_LEFT, 0, 10);

		lv_obj_set_style_text_align(dbs_rpm, LV_ALIGN_TOP_LEFT,0);
		lv_obj_align(dbs_rpm, LV_ALIGN_TOP_LEFT, 0, 20);
	}
}

void cycle_screens() {
	current_screen = (current_screen+1)%2;
	change_screens(current_screen);
}

void change_screens(enum SCREENS screen) {
	current_screen = screen;
	switch(screen) {
	case DRIVER_SCREEN:
		lv_scr_load(driver_screen);
		break;
	case DEBUG_SCREEN:
		lv_scr_load(debug_screen);
		break;
	}
}


