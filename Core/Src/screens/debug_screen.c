#include "debug_screen.h"
#include "styles.h"

#define LINE_HEIGHT 14
#define LEFT_COLUMN_TAB 5

lv_obj_t *debug_screen;         // Debug Screen and Objects

lv_obj_t *dbs_battery_voltage;
lv_obj_t *dbs_steering_position;
lv_obj_t *dbs_throttle_position;
lv_obj_t *dbs_brake_pressure_front;
lv_obj_t *dbs_brake_pressure_rear;
lv_obj_t *dbs_acc_soc;
lv_obj_t *dbs_acc_lowest_cell_voltage;
lv_obj_t *dbs_acc_highest_cell_voltage;
lv_obj_t *dbs_fl_motor_inverter_temp_err_code;
lv_obj_t *dbs_fr_motor_inverter_temp_err_code;
lv_obj_t *dbs_rl_motor_inverter_temp_err_code;
lv_obj_t *dbs_rr_motor_inverter_temp_err_code;
lv_obj_t *dbs_motor_loop_coolant_temp;
lv_obj_t *dbs_inverter_loop_coolant_temp;

extern Vehicle_Data the_vehicle;

void init_debug_screen() {
    //////////////////
    // Debug Screen //
    //////////////////

    debug_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(debug_screen, lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_set_style_text_color(debug_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    // Labels:
    // These are separated so that later I can only update the text if required
    // Battery Voltage
    dbs_battery_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %.2fV", (double)the_vehicle.glv.voltage);
    lv_obj_set_style_text_align(dbs_battery_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_battery_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 1 * LINE_HEIGHT);

    // Steering Position
    dbs_steering_position = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_steering_position, "Steering Position: %d deg", the_vehicle.driver.steeringAngle);
    lv_obj_set_style_text_align(dbs_steering_position, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_steering_position, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 2 * LINE_HEIGHT);

    // Throttle Position
    dbs_throttle_position = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_throttle_position, "Throttle Position: %d %% ", the_vehicle.driver.throttle);
    lv_obj_set_style_text_align(dbs_throttle_position, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_throttle_position, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 3 * LINE_HEIGHT);

    // Brake Pressure
    dbs_brake_pressure_front = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d bar",
                          the_vehicle.driver.frontBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 4 * LINE_HEIGHT);

    // Brake Pressure
    dbs_brake_pressure_rear = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d bar", the_vehicle.driver.rearBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 5 * LINE_HEIGHT);

    // Accumulator SOC
    dbs_acc_soc = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
    lv_obj_set_style_text_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 6 * LINE_HEIGHT);

    // Accumulator Lowest Cell Voltage
    dbs_acc_lowest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %.2f V", (double)the_vehicle.ts.minVoltage);
    lv_obj_set_style_text_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 7 * LINE_HEIGHT);

    // Accumulator Highest Cell Voltage
    dbs_acc_highest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %.2f V", (double)the_vehicle.ts.maxVoltage);
    lv_obj_set_style_text_align(dbs_acc_highest_cell_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_highest_cell_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 8 * LINE_HEIGHT);

    // Second Column
    // front left motor temps
    dbs_fl_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_fl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[0].motorTemp, the_vehicle.drive[0].inverterTemp,
                          the_vehicle.drive[0].errorCode);
    lv_obj_set_style_text_align(dbs_fl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_fl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 1 * LINE_HEIGHT);

    // front right motor temps
    dbs_fr_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_fr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[1].motorTemp, the_vehicle.drive[1].inverterTemp,
                          the_vehicle.drive[1].errorCode);
    lv_obj_set_style_text_align(dbs_fr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_fr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 2 * LINE_HEIGHT);

    dbs_rl_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_rl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[2].motorTemp, the_vehicle.drive[2].inverterTemp,
                          the_vehicle.drive[2].errorCode);
    lv_obj_set_style_text_align(dbs_rl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_rl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 3 * LINE_HEIGHT);

    dbs_rr_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_rr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[3].motorTemp, the_vehicle.drive[3].inverterTemp,
                          the_vehicle.drive[3].errorCode);
    lv_obj_set_style_text_align(dbs_rr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_rr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 4 * LINE_HEIGHT);

    dbs_motor_loop_coolant_temp = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_motor_loop_coolant_temp, "motor coolant temp: %d deg", -1);
    lv_obj_set_style_text_align(dbs_motor_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_motor_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 240, 5 * LINE_HEIGHT);

    dbs_inverter_loop_coolant_temp = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_inverter_loop_coolant_temp, "inverter coolant temp: %d deg", -1);
    lv_obj_set_style_text_align(dbs_inverter_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_inverter_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 240, 6 * LINE_HEIGHT);
}

void load_debug_screen() {
    lv_scr_load(debug_screen);
}

void update_debug_screen() {
    // Update Debug Screen
    lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %.2fV", (double)the_vehicle.glv.voltage);
    lv_label_set_text_fmt(dbs_steering_position, "Steering Position: %d deg", the_vehicle.driver.steeringAngle);
    lv_label_set_text_fmt(dbs_throttle_position, "Throttle Position: %d %% ", the_vehicle.driver.throttle);
    lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d bar",
                            the_vehicle.driver.frontBrakePressure);
    lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d bar", the_vehicle.driver.rearBrakePressure);
    lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
    lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %.2f V", (double)the_vehicle.ts.minVoltage);
    lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %.2f V", (double)the_vehicle.ts.maxVoltage);
    lv_label_set_text_fmt(dbs_fl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                            the_vehicle.drive[0].motorTemp, the_vehicle.drive[0].inverterTemp,
                            the_vehicle.drive[0].errorCode);
    lv_label_set_text_fmt(dbs_fr_motor_inverter_temp_err_code, "FRT: MT=%d\tIT=%d\terr=%d",
                            the_vehicle.drive[1].motorTemp, the_vehicle.drive[1].inverterTemp,
                            the_vehicle.drive[1].errorCode);
    lv_label_set_text_fmt(dbs_rl_motor_inverter_temp_err_code, "RLT: MT=%d\tIT=%d\terr=%d",
                            the_vehicle.drive[2].motorTemp, the_vehicle.drive[2].inverterTemp,
                            the_vehicle.drive[2].errorCode);
    lv_label_set_text_fmt(dbs_rr_motor_inverter_temp_err_code, "RRT: MT=%d\tIT=%d\terr=%d",
                            the_vehicle.drive[3].motorTemp, the_vehicle.drive[3].inverterTemp,
                            the_vehicle.drive[3].errorCode);

    lv_label_set_text_fmt(dbs_motor_loop_coolant_temp, "motor coolant temp: %d deg", the_vehicle.driver.motor_cooling_temp);
    lv_label_set_text_fmt(dbs_inverter_loop_coolant_temp, "inverter coolant temp: %d deg", the_vehicle.driver.inverter_cooling_temp);
}