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
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} led_frame_type;

// TODO: fill out LED's on board
#define LED_NUMBER 19

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

#endif //DASHBOARD_LED_H
