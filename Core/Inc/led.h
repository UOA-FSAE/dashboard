//
// Created by tazukiswift on 29/06/23.
//

#ifndef DASHBOARD_LED_H
#define DASHBOARD_LED_H

#include <stdint.h>

typedef struct {
    uint8_t bright;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_frame_type;

#define LED_NUMBER 13

enum led_meaning {
    TS_ACTIVE,
    ERROR_1,
    ERROR_
};


void update_led(led_frame_type *led_list);

void set_led(enum led_meaning led_id, uint8_t bright, uint8_t red, uint8_t green, uint8_t blue);

#endif //DASHBOARD_LED_H
