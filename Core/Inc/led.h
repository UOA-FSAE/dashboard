//
// Created by tazukiswift on 29/06/23.
//

#ifndef DASHBOARD_LED_H
#define DASHBOARD_LED_H

#include <stdint.h>
#include "main.h"

// struct is 4 bytes with no padding
typedef struct {
    uint8_t bright;
    uint8_t red;
    uint8_t blue;
    uint8_t green;
} led_frame_type;

// TODO: fill out LED's on board
#define LED_NUMBER 19
#define LED_MAX_BRIGHTNESS 31
#define LED_MID_BRIGHTNESS 15

#define LED_RED LED_MID_BRIGHTNESS, 255, 0, 0
#define LED_GREEN LED_MID_BRIGHTNESS, 0, 255, 0
#define LED_YELLOW LED_MID_BRIGHTNESS, 255, 180, 0
#define LED_BLUE LED_MID_BRIGHTNESS, 0, 0, 255
#define LED_BLACK LED_MID_BRIGHTNESS, 0, 0, 0

enum led_meaning {
    IMD,
    AMS,
    BSPD,
    ERROR2,
    ERROR1,
    GLV,
    SHUTDOWN,
    TS_ACT,
    READY,
    PDOC,
    RR,
    FR,
    FL,
    RL,
    CTRL1,
    CTRL2,
    INVERTER,
    ACCUMULATOR,
    KIWI
};

void init_led();

void update_led(SPI_HandleTypeDef *spi_handle);

void set_led(enum led_meaning led_id, uint8_t bright, uint8_t red, uint8_t green, uint8_t blue);

void set_all_red();

#endif //DASHBOARD_LED_H
