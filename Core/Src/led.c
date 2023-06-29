//
// Created by tazukiswift on 29/06/23.
//

#include "led.h"

led_frame_type board_led_list[LED_NUMBER] = {0};

// TODO: saturate brightness between 0 and 31
void set_led(enum led_meaning led_id, uint8_t bright, uint8_t red, uint8_t green, uint8_t blue) {
    bright = 0b11100000 | bright;   // Set first 3 bits to 111 for sign (see datasheet)
    led_frame_type temp = {bright,red,green,blue};
    board_led_list[led_id] = temp;
}


