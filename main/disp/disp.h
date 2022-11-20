#ifndef __DISP_SPI_H
#define __DISP_SPI_H

#include <cstdint>
#include <string>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "commands.h"
#include "../inc/fonts/gfxfont.h"

#include "../leds/led.h"

// Probably not the best place to put this but it will do for now
#define PIN_NUM_MISO GPIO_NUM_12
#define PIN_NUM_MOSI GPIO_NUM_13
#define PIN_NUM_CLK GPIO_NUM_14
#define PIN_NUM_CS GPIO_NUM_15
#define PIN_NUM_DC GPIO_NUM_18
#define PARALLEL_LINES 40  // Increase this to speed up the SPI transfer at the expense of memory
#define MAX_BLIT_SIZE 39360 // Do not alloc more than a roughly 190*190 area

enum DISPLAY_STATUS{
    DISPLAY_OK,
    DISPLAY_SPI_WRITE_FAILED,
    DISPLAY_ERR_ZERO_LENGTH_SPI_TRANSFER,
    DISPLAY_ERR_BLIT_AREA_TOO_LARGE,
    DISPLAY_ERR_TEXT_OUT_OF_RANGE
};

enum TEXT_ALIGNMENT{
    LEFT,
    RIGHT,
    CENTER
};

// Representation of a pixel in complete 565 bitswapped 
struct colour565 {
    union {
        //16 bit, bitfield
        struct {
            uint8_t g1 : 3;
            uint8_t r : 5;
            uint8_t b : 5;
            uint8_t g2 : 3;
        };
        uint16_t raw;
    };

    // Allow construction from R, G, B
    //Includes byte swap
    inline colour565(const uint8_t ir, const uint8_t ig, const uint8_t ib) __attribute__((always_inline))
        : g1((ig >> 2) & 0x07), r((ir >> 3) & 0x1F), b((ib >> 3) & 0x1F), g2((ig >> 5) & 0x07)
    {
    }

    // Allow construction from CRGB
    //Includes byte swap
    inline colour565(const CRGB rgb) __attribute__((always_inline))
        : g1((rgb.g >> 2) & 0x07), r((rgb.r >> 3) & 0x1F), b((rgb.b >> 3) & 0x1F), g2((rgb.g >> 5) & 0x07)
    {
    }

    // Allow construction from 32-bit 0xRRGGBB color code
    // Includes byte swap
    inline colour565(const uint32_t colourcode) __attribute__((always_inline))
        : g1((colourcode >> 10) & 0x07), r((colourcode >> 19) & 0x1F), b((colourcode >> 3) & 0x1F), g2((colourcode >> 13) & 0x07)
    {
    }

    /// allow assignment from one colour565 struct to another
    inline colour565& operator= (const colour565& rhs) __attribute__((always_inline))
    {
        g1 = rhs.g1;
        r = rhs.r;
        b = rhs.b;
        g2 = rhs.g2;
        return *this;
    }

    /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    inline colour565& operator= (const uint32_t colourcode) __attribute__((always_inline))
    {
        g1 = ((colourcode >> 10) & 0x07);
        r  = ((colourcode >> 19) & 0x1F);
        b  = ((colourcode >> 3) & 0x1F);
        g2 = ((colourcode >> 13) & 0x07);
        return *this;
    }
};

// Hit it with the class
class disp{
    private:
        uint16_t width;
        uint16_t height;

        uint16_t lastxstart;
        uint16_t lastxend;
        uint16_t lastystart;
        uint16_t lastyend;

        TEXT_ALIGNMENT alignment;

        spi_device_handle_t spi;

        DISPLAY_STATUS send_lines(int ypos, uint16_t *linedata);
        DISPLAY_STATUS send_line_finish();
        DISPLAY_STATUS draw(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t *buf);

    public:
        disp(uint16_t width, uint16_t height);

        // Very low level
        DISPLAY_STATUS spiWrite(uint8_t cmd, uint8_t *buf, uint32_t len);
        DISPLAY_STATUS spiWriteCMD(uint8_t cmd);
        DISPLAY_STATUS spiWriteDATA(uint8_t *buf, uint32_t len);
        DISPLAY_STATUS spiRead(uint8_t *buf, uint32_t len);

        // Low level
        DISPLAY_STATUS blit(uint16_t *buf);
        DISPLAY_STATUS drawPixel(uint16_t xpos, uint16_t ypos, uint16_t colour);
        DISPLAY_STATUS drawLine(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t colour);

        // Primitive shapes
        DISPLAY_STATUS drawRectangle(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend, uint16_t colour);
        DISPLAY_STATUS fillRectangle(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend, uint16_t colour);
        DISPLAY_STATUS drawCircle(uint16_t xpos, uint16_t ypos, uint16_t radius, uint16_t colour);
        DISPLAY_STATUS fillCircle(uint16_t xpos, uint16_t ypos, uint16_t radius, uint16_t colour);
        DISPLAY_STATUS drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour);
        DISPLAY_STATUS fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour);

        // Some of the drivers are literate so we should show them some text
        DISPLAY_STATUS drawText(uint16_t xpos, uint16_t ypos, uint8_t sizex, uint8_t sizey, const std::string str, const GFXfont *font, uint16_t colour);
        DISPLAY_STATUS drawTextFast(uint16_t xpos, uint16_t ypos, const std::string str, const GFXfont *font, uint16_t colour, uint16_t bgcolour);

        // Pretty pictures.bmp
        DISPLAY_STATUS drawBitmap(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint16_t *buf);
        DISPLAY_STATUS fillScreen(uint16_t colour);

        DISPLAY_STATUS maskDisplay();
        DISPLAY_STATUS unmaskDisplay();
        DISPLAY_STATUS darkenAllPixels();
        DISPLAY_STATUS recoverDarkenAllPixels();

        DISPLAY_STATUS initDisplay();

        uint16_t getStringLength(uint8_t sizex, const std::string str, const GFXfont *font);
        DISPLAY_STATUS setTextAlignment(TEXT_ALIGNMENT alignment_input);
};

#endif