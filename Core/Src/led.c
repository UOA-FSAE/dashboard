//
// Created by tazukiswift on 29/06/23.
//

#include "led.h"

// TODO: make this buffer include the start and stop bits?
led_frame_type board_led_list[LED_NUMBER] = {0};

static const led_frame_type led_off = {0xE0, 0x00, 0x00, 0x00};

// Call this to initialize the array of LED's
void init_led() {
    for (int i = 0; i < LED_NUMBER; i++) {
        board_led_list[i] = led_off;
    }
}

// TODO: make a wrapper for this function to set via color enum
void set_led(enum led_meaning led_id, uint8_t bright, uint8_t red, uint8_t green, uint8_t blue) {
    board_led_list[(int) led_id] = (led_frame_type) {(0b11100000 | bright), red, blue, green};
}

void set_all_red() {
    for (int selected_led = 0; selected_led < LED_NUMBER; selected_led++) {
        set_led((enum led_meaning) selected_led, LED_RED);
    }
}

static uint16_t bytes_to_transmit = 4 * LED_NUMBER;

static const uint32_t start_word = 0x00000000;
static const uint32_t stop_word = 0xFFFFFFFF;

// Note: Clock polarity should be 1
void update_led(SPI_HandleTypeDef *spi_handle) {
    // Note: STM32 is little endian, so brightness stored first in struct
    if (HAL_SPI_Transmit(spi_handle, (uint8_t *) &start_word, 4, 100) != HAL_OK) {
        Error_Handler();
    };
    if (HAL_SPI_Transmit(spi_handle, (uint8_t *) board_led_list, bytes_to_transmit, 100) != HAL_OK) {
        Error_Handler();
    };
    if (HAL_SPI_Transmit(spi_handle, (uint8_t *) &stop_word, 4, 100) != HAL_OK) {
        Error_Handler();
    };
}

