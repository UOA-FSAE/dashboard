#include <screens.h>
#include <lvgl.h>
#include <vehicle.h>
#include <ltdc.h>

extern volatile Vehicle_Data the_vehicle;

#define LINE_HEIGHT 14
#define LEFT_COLUMN_TAB 5

// Driver Type
#define SOFTWARE_INVERT_SCREEN 0    // TODO: implement this?
#define USE_DOUBLE_BUFFER 0

// Screen switching flags
enum SCREENS current_screen;

static volatile uint8_t screen_switch_flag = 0;
static volatile uint8_t screen_update_flag = 0;

// Display Driver
static lv_disp_draw_buf_t the_display_buf;
static lv_disp_drv_t the_display_drv;                 /*A variable to hold the drivers.*/

lv_disp_t * disp;

// Static or global buffer(s).
#if USE_DOUBLE_BUFFER
volatile lv_color_t *buf_1 = (lv_color_t *)0xC0000000;
volatile lv_color_t *buf_2 = (lv_color_t *)0xC0000000+272*480*4;
#else
volatile lv_color_t buf_1[100*100];
volatile lv_color_t buf_2[100*100];
#endif

// Create screens
// Driver Screen and Objects
// TODO: Should globals be static?
lv_obj_t *driver_screen;

// Debug screen styles
static lv_style_t speedometer_foreground_style;
static lv_style_t speedometer_background_style;

lv_obj_t * rpm_arc;
lv_obj_t * throttle_arc;

// Debug Screen and Objects
lv_obj_t *debug_screen;

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

// Private local flush buffer function
#if USE_DOUBLE_BUFFER
void flush_callback(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
//	(hltdc.LayerCfg[0]).FBStartAdress = (uint32_t)color_p;
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)color_p, LTDC_LAYER_1);
    /* IMPORTANT!!!
    * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
#else
void flush_callback(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    volatile uint32_t *ram_address = (uint32_t *)0xC0000000;
    int width = area->x2 - area->x1+1;
    for (int i = 0;i<=area->y2-area->y1;i++){
        memcpy(ram_address+(area->y1+i)*480+area->x1,color_p+width*i,4*width);
    }
    /* IMPORTANT!!!
    * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
#endif

void init_displays() {
    lv_init();

    /*Initialize `disp_buf` with the buffer(s) */

#if USE_DOUBLE_BUFFER
    lv_disp_draw_buf_init(&the_display_buf, buf_1, buf_2, 480*272);

    lv_disp_drv_init(&the_display_drv);            /*Basic initialization*/
    the_display_drv.draw_buf = &the_display_buf;            /*Set an initialized buffer*/
    the_display_drv.direct_mode = 1;
    the_display_drv.full_refresh = 1;
    the_display_drv.sw_rotate = 0;
    the_display_drv.hor_res = 480;
    the_display_drv.ver_res = 272;
    the_display_drv.rotated = LV_DISP_ROT_180;
    the_display_drv.flush_cb = flush_callback;
    disp = lv_disp_drv_register(&the_display_drv);
#else
    lv_disp_draw_buf_init(&the_display_buf, buf_1, buf_2, 100*100);

    lv_disp_drv_init(&the_display_drv);            /*Basic initialization*/
    the_display_drv.draw_buf = &the_display_buf;            /*Set an initialized buffer*/
    the_display_drv.direct_mode = 0;
    the_display_drv.full_refresh = 0;
    the_display_drv.sw_rotate = 1;
    the_display_drv.hor_res = 480;
    the_display_drv.ver_res = 272;
    the_display_drv.rotated = LV_DISP_ROT_180;
    the_display_drv.flush_cb = flush_callback;
    disp = lv_disp_drv_register(&the_display_drv);
#endif
}

// Cursor parking lot


void init_screens() {
    // Driver Screen
    driver_screen = lv_obj_create(NULL);
    lv_scr_load(driver_screen);
    current_screen = DRIVER_SCREEN;

    lv_obj_set_style_bg_color(driver_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(driver_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    // Speedometer foreground style
    lv_style_init(&speedometer_foreground_style);
    lv_style_set_arc_rounded(&speedometer_foreground_style,false);
    lv_style_set_arc_color(&speedometer_foreground_style,lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_style_set_arc_width(&speedometer_foreground_style,40);

    lv_style_init(&speedometer_background_style);
    lv_style_set_arc_rounded(&speedometer_background_style,false);
    lv_style_set_arc_width(&speedometer_background_style,40);


    rpm_arc = lv_arc_create(driver_screen);
    lv_arc_set_bg_angles(rpm_arc, 150, 30);
    lv_arc_set_angles(rpm_arc, 150, 150+70);
    lv_obj_set_size(rpm_arc, 180, 180);

    lv_obj_add_style(rpm_arc,&speedometer_foreground_style,LV_PART_INDICATOR);
    lv_obj_add_style(rpm_arc,&speedometer_background_style,LV_PART_MAIN);
    lv_obj_remove_style(rpm_arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(rpm_arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(rpm_arc, LV_ALIGN_LEFT_MID, 40, 0);

    throttle_arc = lv_arc_create(driver_screen);
    lv_arc_set_bg_angles(throttle_arc, 150, 30);
    lv_arc_set_angles(throttle_arc, 150, 150+70);
    lv_obj_set_size(throttle_arc, 180, 180);

    lv_obj_add_style(throttle_arc,&speedometer_foreground_style,LV_PART_INDICATOR);
    lv_obj_add_style(throttle_arc,&speedometer_background_style,LV_PART_MAIN);
    lv_obj_remove_style(throttle_arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(throttle_arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(throttle_arc, LV_ALIGN_RIGHT_MID, -40, 0);


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
    lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %dV", the_vehicle.glv.voltage);
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
    lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d kPa",
                          the_vehicle.driver.frontBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 4 * LINE_HEIGHT);

    // Brake Pressure
    dbs_brake_pressure_rear = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d kPa", the_vehicle.driver.rearBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 5 * LINE_HEIGHT);

    // Accumulator SOC
    dbs_acc_soc = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
    lv_obj_set_style_text_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 6 * LINE_HEIGHT);

    // Accumulator Lowest Cell Voltage
    dbs_acc_lowest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %d V", the_vehicle.ts.minVoltage);
    lv_obj_set_style_text_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 7 * LINE_HEIGHT);

    // Accumulator Highest Cell Voltage
    dbs_acc_highest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %d V", the_vehicle.ts.maxVoltage);
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

void try_update_screen() {
    if (!screen_update_flag) return;
    screen_update_flag = 0;
    if (lv_scr_act() == driver_screen) {
        // Update Driver Screen

    } else if (lv_scr_act() == debug_screen) {
        // Update Debug Screen
        lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %.2fV", the_vehicle.glv.voltage);
        lv_label_set_text_fmt(dbs_steering_position, "Steering Position: %d deg", the_vehicle.driver.steeringAngle);
        lv_label_set_text_fmt(dbs_throttle_position, "Throttle Position: %d %% ", the_vehicle.driver.throttle);
        lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d kPa",
                              the_vehicle.driver.frontBrakePressure);
        lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d kPa", the_vehicle.driver.rearBrakePressure);
        lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
        lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %d V", the_vehicle.ts.minVoltage);
        lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %d V", the_vehicle.ts.maxVoltage);
        lv_label_set_text_fmt(dbs_fl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[0].motorTemp, the_vehicle.drive[0].inverterTemp,
                              the_vehicle.drive[0].errorCode);
        lv_label_set_text_fmt(dbs_fr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[1].motorTemp, the_vehicle.drive[1].inverterTemp,
                              the_vehicle.drive[1].errorCode);
        lv_label_set_text_fmt(dbs_rl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[2].motorTemp, the_vehicle.drive[2].inverterTemp,
                              the_vehicle.drive[2].errorCode);
        lv_label_set_text_fmt(dbs_rr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[3].motorTemp, the_vehicle.drive[3].inverterTemp,
                              the_vehicle.drive[3].errorCode);
    }
}

void update_screen() {
    screen_update_flag = 1;
}

void change_screens(enum SCREENS screen) {
    current_screen = screen;
    switch (screen) {
        case DRIVER_SCREEN:
            lv_scr_load(driver_screen);
            break;
        case DEBUG_SCREEN:
            lv_scr_load(debug_screen);
            break;
    }
}

void cycle_screens() {
    screen_switch_flag = 1;
}

void try_cycle_screens() {
    if (screen_switch_flag){
        screen_switch_flag = 0;
        current_screen = (current_screen + 1) % 2;
        change_screens(current_screen);
    }
}

