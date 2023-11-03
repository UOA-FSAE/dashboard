#include <lvgl.h>
#include "screens.h"
#include "vehicle.h"

#include "driver_screen.h"
#include "debug_screen.h"
#include "energy_screen.h"
#include "lap_screen.h"

#ifndef USE_SIMULATOR
#include <ltdc.h>
#endif

extern volatile Vehicle_Data the_vehicle;

// Driver Type
#define SOFTWARE_INVERT_SCREEN 0    // TODO: implement this?
#define USE_DOUBLE_BUFFER 0

// Screen switching flags
enum SCREENS current_screen;

static volatile uint8_t screen_switch_flag = 0;
static volatile uint8_t screen_update_flag = 0;

#ifndef USE_SIMULATOR
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
#endif
///////////
// Fonts //
///////////
LV_FONT_DECLARE(bitstream_vera_sans_26)
LV_FONT_DECLARE(bitstream_vera_sans_30)
LV_FONT_DECLARE(bitstream_vera_sans_80)

////////////
// Styles //
////////////
static lv_style_t speedometer_style;
static lv_style_t colored_label_style;
static lv_style_t regular_text;
static lv_style_t title_text;       // Style for title text (unused)
static lv_style_t accumulator_style;
static lv_style_t text_box_style; // Text box style (for border)
static lv_style_t regen_style;    // Regen Style
static lv_style_t rpm_style;    // RPM style

/////////////
// Objects //
/////////////

void init_screens() {
    ////////////
    // Styles //
    ////////////

    lv_style_init(&regular_text);
    lv_style_set_text_font(&regular_text,&lv_font_montserrat_18);
    lv_style_set_text_align(&regular_text,LV_ALIGN_CENTER);

    lv_style_init(&title_text);
    lv_style_set_text_font(&title_text,&lv_font_montserrat_28);
    lv_style_set_text_align(&title_text,LV_ALIGN_CENTER);

    lv_style_init(&colored_label_style);
    lv_style_set_text_align(&colored_label_style,LV_ALIGN_TOP_LEFT);
    lv_style_set_radius(&colored_label_style,5);
    lv_style_set_pad_all(&colored_label_style,10);
    lv_style_set_bg_opa(&colored_label_style,LV_OPA_COVER);

    lv_style_init(&rpm_style);
    lv_style_set_bg_opa(&rpm_style, LV_OPA_COVER);
    lv_style_set_bg_color(&rpm_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&rpm_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_dir(&rpm_style, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&rpm_style,3);

    init_driver_screen();
    init_debug_screen();
    init_energy_screen();
    init_lap_screen();
}

void try_update_screen() {
    if (!screen_update_flag) return;
    screen_update_flag = 0;
    switch(current_screen) {
        case DRIVER_SCREEN:
            update_driver_screen();
            break;
        case DEBUG_SCREEN:
            update_debug_screen();
            break;
        case ENERGY_SCREEN:
            update_energy_screen();
            break;
        case LAP_SCREEN:
            update_lap_screen();
            break;
    }
}

void ms_to_minutes_seconds(uint32_t ms, uint32_t * minutes, uint32_t * seconds, uint32_t * milliseconds) {
    *minutes = (uint32_t)(0.00001666666*ms);
    *seconds = (uint32_t)(0.001*ms)-60*(*minutes);
    *milliseconds = ms%1000;
}

void update_screen() {
    screen_update_flag = 1;
}

void change_screens(enum SCREENS screen) {
    current_screen = screen;
    switch (screen) {
        case DRIVER_SCREEN:
            load_driver_screen();
            break;
        case DEBUG_SCREEN:
            load_debug_screen();
            break;
        case ENERGY_SCREEN:
            load_energy_screen();
            break;
        case LAP_SCREEN:
            load_lap_screen();
            break;
    }
}

void cycle_screens() {
    screen_switch_flag = 1;
}

void try_cycle_screens() {
    if (screen_switch_flag){
        screen_switch_flag = 0;
        current_screen = (current_screen + 1) % 4;
        change_screens(current_screen);
    }
}

#ifndef USE_SIMULATOR

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

#endif

