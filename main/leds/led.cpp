#include "led.h"
#include <cstdint>
#include <cstring>

#include "driver/gpio.h"

void set_all_led_red(CRGB* led) {
    for(uint8_t idx = 0; idx < NUM_LEDS; ++idx){
        led[idx] = RED;
    }
}

void set_all_led_green(CRGB* led) {
    for(uint8_t idx = 0; idx < NUM_LEDS; ++idx){
        led[idx] = GREEN;
    }
}

void set_all_led_yellow(CRGB* led) {
    for(uint8_t idx = 0; idx < NUM_LEDS; ++idx){
        led[idx] = YELLOW;
    }
}

slowLED::slowLED(uint16_t num) {
    this->numLEDs = num;
    // Setup GPIO ports

    gpio_set_direction(PIN_NUM_LED_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_LED_DAT, GPIO_MODE_OUTPUT);

    gpio_set_level(PIN_NUM_LED_CLK, 0);
    gpio_set_level(PIN_NUM_LED_DAT, 1);
}

//Send all the data over the bus
LED_STATUS slowLED::show(CRGB *leds) {

    this->startFrame();

    for(uint8_t idx=0; (uint8_t) this->numLEDs > idx; ++idx) {
        this->writeLED(leds[idx].brightness, leds[idx].red, leds[idx].blue, leds[idx].green);
    }

    this->endFrame();

    return LED_OK;
}

//Writes a single byte onto the bus
LED_STATUS slowLED::writeByte(uint8_t data) {

    //copy the data into a local buffer for safety
    uint8_t buffer;
    memcpy(&buffer, &data, sizeof(uint8_t));

    //clock out the byte
    for (int8_t x=7; x>=0; x--) {
        gpio_set_level(PIN_NUM_LED_DAT, ((buffer >> x) & 1));
        gpio_set_level(PIN_NUM_LED_CLK, 1);
        //NOP();  NOP();  NOP();  NOP();  //NOP();  NOP();  NOP();  NOP();
        gpio_set_level(PIN_NUM_LED_CLK, 0);
        //NOP();  NOP();  NOP();  NOP();  //NOP();  NOP();  NOP();  NOP();
    }

    return LED_OK;
}

//Writes a single 32 bit LED frame onto the bus
LED_STATUS slowLED::writeLED(uint8_t brightness, uint8_t b0, uint8_t b1, uint8_t b2) {

    this->writeByte((0xE0 | brightness)); //Brightness is 5 bits with leading 111 MSB
    this->writeByte(b0);
    this->writeByte(b1);
    this->writeByte(b2);

    return LED_OK;
}

//Write the start frame onto the bus
void slowLED::startFrame() {

    this->writeByte(0x00);
    this->writeByte(0x00);
    this->writeByte(0x00);
    this->writeByte(0x00);
}

//Write the end frame onto the bus
void slowLED::endFrame() {

    //Have so far found several implementations of the endFrame or SK9822 LEDs

    //As per datasheet
    //this->writeByte(0xFF);
    //this->writeByte(0xFF);
    //this->writeByte(0xFF);
    //this->writeByte(0xFF);

    //As per https://cpldcpu.wordpress.com/2016/12/13/sk9822-a-clone-of-the-apa102/
    this->writeByte(0x00);
    this->writeByte(0x00);
    this->writeByte(0x00);
    this->writeByte(0x00);
    for (uint8_t i = 0; i < (uint8_t) ((this->numLEDs/2)/8); i++) {
        this->writeByte(0x00);
    }

    //As per https://ecksteinimg.de/Datasheet/Pololu/ledpanel_sk9822_userguide.pdf
    //this->writeByte(0xFF);
    //for (uint8_t i = 0; i < (5+(this->numLEDs)/16); i++) {
    //    this->writeByte(0x00);
    //}

    //As per fastLED library
    //for (uint8_t i = 0; i < ((this->numLEDs)/32); i++) {
    //    this->writeByte(0xFF);
    //    this->writeByte(0x00);
    //    this->writeByte(0x00);
    //    this->writeByte(0x00);
    //}
}


// Sometimes the compiler will do clever things to reduce
// code size that result in a net slowdown, if it thinks that
// a variable is not used in a certain location.
// This macro does its best to convince the compiler that
// the variable is used in this location, to help control
// code motion and de-duplication that would result in a slowdown.
#define FORCE_REFERENCE(var)  asm volatile( "" : : "r" (var) )

#define K255 255
#define K171 171
#define K170 170
#define K85  85

//Stripped down version of the scale8_video function from fastLED
uint8_t slowLED::scale8(uint8_t value, uint8_t scale) {
    return (((uint16_t)value * (uint16_t)scale) >> 8) + ((value && scale) ? 1 : 0);
}

//Stripped down version of the hsv2rgb_rainbow function from fastLED
void slowLED::hsv2rgb_rainbow( const CHSV& hsv, CRGB& rgb) {
    // Yellow has a higher inherent brightness than
    // any other color; 'pure' yellow is perceived to
    // be 93% as bright as white.  In order to make
    // yellow appear the correct relative brightness,
    // it has to be rendered brighter than all other
    // colors.

    uint8_t hue = hsv.hue;
    uint8_t sat = hsv.sat;
    uint8_t val = hsv.val;

    uint8_t offset = hue & 0x1F; // 0..31

    // offset8 = offset * 8
    uint8_t offset8 = offset;

    offset8 <<= 3;

    uint8_t third = scale8( offset8, (256 / 3)); // max = 85
    uint8_t twothirds = scale8( offset8, ((256 * 2) / 3)); // max=170

    uint8_t r, g, b;

    //Nested If/else tree to set r,g,b based on the colour wheel section
    if( ! (hue & 0x80) ) {
        // 0XX
        if( ! (hue & 0x40) ) {
            // 00X
            //section 0-1
            if( ! (hue & 0x20) ) {
                // 000
                r = K255 - third;
                g = third;
                b = 0;
                FORCE_REFERENCE(b);
            } else {
                // 001
                r = K171;
                g = K85 + third ;
                b = 0;
                FORCE_REFERENCE(b);

            }
        } else {
            //01X
            // section 2-3
            if( !  (hue & 0x20) ) {
                // 010
                r = K171 - twothirds;
                g = K170 + third;
                b = 0;
                FORCE_REFERENCE(b);

            } else {
                // 011
                r = 0;
                FORCE_REFERENCE(r);
                g = K255 - third;
                b = third;
            }
        }
    } else {
        // section 4-7
        // 1XX
        if( ! (hue & 0x40) ) {
            // 10X
            if( ! ( hue & 0x20) ) {
                // 100
                r = 0;
                FORCE_REFERENCE(r);
                g = K171 - twothirds; //K170?
                b = K85  + twothirds;

            } else {
                // 101
                r = third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K255 - third;

            }
        } else {
            if( !  (hue & 0x20)  ) {
                // 110
                r = K85 + third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K171 - third;

            } else {
                // 111
                r = K170 + third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K85 - third;

            }
        }
    }

    // Scale down colors if we're desaturated at all
    // and add the brightness_floor to r, g, and b.
    if( sat != 255 ) {
        if( sat == 0) {
            r = 255; b = 255; g = 255;
        } else {

            if( r ) r = scale8( r, sat);
            if( g ) g = scale8( g, sat);
            if( b ) b = scale8( b, sat);

            uint8_t desat = 255 - sat;
            desat = scale8( desat, desat);

            uint8_t brightness_floor = desat;
            r += brightness_floor;
            g += brightness_floor;
            b += brightness_floor;
        }
    }

    // Now scale everything down if we're at value < 255.
    if( val != 255 ) {

        val = scale8( val, val);
        if( val == 0 ) {
            r=0; g=0; b=0;
        } else {

            if( r ) r = scale8( r, val);
            if( g ) g = scale8( g, val);
            if( b ) b = scale8( b, val);
        }
    }

    //Finally write the r/g/b values to the CRGB pixel
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    rgb.brightness = 31;
}

//void slowLED::setBrightness( const CRGB& rgb, uint8_t value) {
//    rgb.brightness = value;
//}