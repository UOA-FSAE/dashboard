#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include <cstring>
#include <cmath>
#include <string>

#include "disp.h"
#include "commands.h"
#include "../inc/fonts/gfxfont.h"

struct lcd_cmd{
    uint8_t cmd;
    uint8_t buf[36];
    uint8_t len;
};

// Thanks adafruit!
DRAM_ATTR static const struct lcd_cmd init[] = {
    {HX8357_SWRESET, {0}, 0x80 + 120 / 5},              // Soft reset, then delay 10 ms
    {HX8357D_SETC, {0xFF, 0x83, 0x57}, 3},
    {0xFF, {0}, 0x80 + 200/5},                          // No command, just delay 300 ms
    {HX8357_SETRGB, {0x80, 0x00, 0x06, 0x06}, 4},       
    {HX8357D_SETCOM, {0x28}, 1},                        // -1.52V
    {HX8357_SETOSC, {0xFF, 0x01}, 2},                         // Normal mode 70Hz, Idle mode 55 Hz
    {HX8357_SETPANEL, {0x05}, 1},                       // BGR, Gate direction swapped
    {HX8357_SETPWR1, {0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA}, 6}, 
    {HX8357D_SETSTBA, {0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08}, 6}, 
    {HX8357D_SETCYC, {0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78}, 7}, 
    {HX8357D_SETGAMMA,{0x02, 0x0A, 0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b,
      0x42, 0x3A, 0x27, 0x1B, 0x08, 0x09, 0x03, 0x02, 0x0A,
      0x11, 0x1d, 0x23, 0x35, 0x41, 0x4b, 0x4b, 0x42, 0x3A,
      0x27, 0x1B, 0x08, 0x09, 0x03, 0x00, 0x01}, 34},
    {HX8357_COLMOD, {0x55}, 1},
    {HX8357_MADCTL, {0xA0}, 1},
    {HX8357_SLPOUT, {0}, 0x80 + 50/5},
    //{HX8357_DISPON, {0}, 0x80 + 30/5},
    {0xFF, {0}, 0xFF}
};

void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

disp::disp(uint16_t width, uint16_t height) {

    this->width = width;
    this->height = height;

    this->alignment = LEFT;
    
    // Also initialize the SPI bus here
    spi_bus_config_t buscfg = {
        .mosi_io_num=PIN_NUM_MOSI,
        .miso_io_num=PIN_NUM_MISO,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=65535,
        .flags = SPICOMMON_BUSFLAG_MASTER
    };
    spi_device_interface_config_t devcfg={
        .mode = 0,
        .clock_speed_hz = SPI_MASTER_FREQ_20M, // Slooooo
        //.clock_speed_hz = SPI_MASTER_FREQ_40M, // TUFast
        .spics_io_num = PIN_NUM_CS,
        .flags = 0,
        .queue_size = 2048,
        .pre_cb=lcd_spi_pre_transfer_callback
    };
    esp_err_t ret;

    // Actually init the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &this->spi);
    ESP_ERROR_CHECK(ret);

    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
}

DISPLAY_STATUS disp::spiWrite(uint8_t cmd, uint8_t *buf, uint32_t len) {
    if(len == 0) return DISPLAY_ERR_ZERO_LENGTH_SPI_TRANSFER; // Ignore any 0 length transfers

    esp_err_t ret; 
    static spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.user = (void*)0;
    ret = spi_device_polling_transmit(this->spi, &t);
 
    if(ret != ESP_OK) return DISPLAY_SPI_WRITE_FAILED;

    if(len >= 1) {
        t.length = (len) * 8;
        t.tx_buffer = buf;
        t.user = (void*)1;
        ret = spi_device_polling_transmit(this->spi, &t);       
    }

    return (ret == ESP_OK) ? DISPLAY_OK : DISPLAY_SPI_WRITE_FAILED;
}

DISPLAY_STATUS disp::spiWriteCMD(uint8_t cmd) {
    esp_err_t ret; 
    static spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.user = (void*)0;
    ret = spi_device_polling_transmit(this->spi, &t);

    return (ret == ESP_OK) ? DISPLAY_OK : DISPLAY_SPI_WRITE_FAILED;
}
DISPLAY_STATUS disp::spiWriteDATA(uint8_t *buf, uint32_t len) {
    esp_err_t ret; 
    static spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = (len) * 8;
    t.tx_buffer = buf - 1;
    t.user = (void*)1;
    ret = spi_device_polling_transmit(this->spi, &t);

    return (ret == ESP_OK) ? DISPLAY_OK : DISPLAY_SPI_WRITE_FAILED;
}

DISPLAY_STATUS disp::spiRead(uint8_t *buf, uint32_t len) {
    // Probably don't need this

    return DISPLAY_OK;
}

DISPLAY_STATUS disp::blit(uint16_t *buf) {
    // Must be formatted as big endian RGB565 format
    // This probably will never work as you cannot malloc enough memory for a full screen draw. Perhaps a partial update from bitmap
    for(int x=0; x<320; x+=PARALLEL_LINES) {
        send_lines(x, buf + x);
        send_line_finish();
    }
    return DISPLAY_OK;
}

DISPLAY_STATUS disp::drawPixel(uint16_t xpos, uint16_t ypos, uint16_t colour) {

    return this->draw(xpos, xpos+1, ypos, ypos+1, &colour);
}

void swap(uint16_t &x, uint16_t &y) {
    uint16_t temp = x;
    x = y;
    y = temp;
}

// Try not to draw diagonal lines as they are very costly in SPI time
DISPLAY_STATUS disp::drawLine(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t colour) {
    // Check if we can be speedy by drawing a purely horizontal or vertical line
    if(xstart == xend) {
        uint16_t size = (yend - ystart);
        uint16_t *buf = (uint16_t*) heap_caps_malloc((size) * sizeof(uint16_t), MALLOC_CAP_DMA);
        for(uint16_t x=0; x<size; ++x) {
            buf[x] = colour;
        }
        DISPLAY_STATUS ret = this->draw(xstart, xend, ystart, yend, buf);
        free(buf);
        return ret;
    }
    if(ystart == yend) {
        uint16_t size = (xend - xstart);
        uint16_t *buf = (uint16_t*) heap_caps_malloc((size) * sizeof(uint16_t), MALLOC_CAP_DMA);
        for(uint16_t x=0; x<size; ++x) {
            buf[x] = colour;
        }
        DISPLAY_STATUS ret = this->draw(xstart, xend, ystart, yend, buf);
        free(buf);
        return ret;
    }
    // Bresenham's Algorithm, thx wikipedia!
    bool steep = (abs(yend - ystart) > abs(xend - xstart));

    if(steep) {
        swap(xstart, ystart);
        swap(xend, yend);
    }

    if(xstart > xend) {
        swap(xstart, xend);
        swap(ystart, yend);
    }

    // Rip efficiency

    const float dx = xend - xstart;
    const float dy = fabs(yend - ystart);
    
    float error = dx / 2.0f;
    const int ystep = (ystart < yend) ? 1 : -1;
    int y = (int)ystart;
    
    const int maxX = (int)xend;
 
    for(int x=(int)xstart; x<maxX; x++)
    {
        if(steep)
        {
            DISPLAY_STATUS ret = this->drawPixel(y, x, colour);
            if(ret != DISPLAY_OK) return ret;
        }
        else
        {
            DISPLAY_STATUS ret = this->drawPixel(x, y, colour);
            if(ret != DISPLAY_OK) return ret;
        }
    
        error -= dy;
        if(error < 0)
        {
            y += ystep;
            error += dx;
        }
    }

    return DISPLAY_OK;
}

DISPLAY_STATUS disp::drawRectangle(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend, uint16_t colour) {
    DISPLAY_STATUS ret = this->drawLine(xstart, xend, ystart, ystart, colour);
    if(ret != DISPLAY_OK) return ret;
    ret = this->drawLine(xstart, xend, yend, yend, colour);
    if(ret != DISPLAY_OK) return ret;
    ret = this->drawLine(xstart, xstart, ystart, yend, colour);
    if(ret != DISPLAY_OK) return ret;
    ret = this->drawLine(xend, xend, ystart, yend, colour);
    if(ret != DISPLAY_OK) return ret;

    return DISPLAY_OK;
}

DISPLAY_STATUS disp::fillRectangle(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend, uint16_t colour) {
    // Lazy square drawing
    DISPLAY_STATUS ret;
    for(uint16_t y = ystart; y <= yend; ++y) {
        ret = this->drawLine(xstart, xend, y, y, colour);
        if(ret != DISPLAY_OK) return ret;
    }
    return DISPLAY_OK;
}

DISPLAY_STATUS disp::drawCircle(uint16_t xpos, uint16_t ypos, uint16_t radius, uint16_t colour){
    // Slooo
    DISPLAY_STATUS ret;
    for(float a = 0; a < (M_PI * 2); a += (M_PI / 360)){
        int sa = sin(a) * radius;
        int ca = cos(a) * radius;
        ret = this->drawPixel(xpos + ca, ypos + sa, colour);
        if(ret != DISPLAY_OK) return ret;
    }
    return DISPLAY_OK;
}

//TODO
DISPLAY_STATUS disp::fillCircle(uint16_t xpos, uint16_t ypos, uint16_t radius, uint16_t colour) {
    DISPLAY_STATUS ret;

    
    ret = DISPLAY_OK;

    return ret;
}

DISPLAY_STATUS disp::drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour) {
    DISPLAY_STATUS ret;

    ret = this->drawLine(x0, y0, x1, y1, colour);
    ret = this->drawLine(x1, y1, x2, y2, colour);
    ret = this->drawLine(x2, y2, x0, y0, colour);

    return ret;
}

//TODO
DISPLAY_STATUS disp::fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour) {
    DISPLAY_STATUS ret;

    ret = this->drawLine(x0, y0, x1, y1, colour);
    ret = this->drawLine(x1, y1, x2, y2, colour);
    ret = this->drawLine(x2, y2, x0, y0, colour);

    return ret;
}

DISPLAY_STATUS disp::draw(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t *buf) {
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[6];

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }

    // Calculate how big our framebuffer is
    uint32_t size = (xend - xstart + 1) * (yend - ystart + 1) * 2;
    if(size > MAX_BLIT_SIZE) return DISPLAY_ERR_BLIT_AREA_TOO_LARGE;

    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=(xstart) >> 8;              //Start Col High
    trans[1].tx_data[1]=(xstart) & 0xFF;              //Start Col Low
    trans[1].tx_data[2]=(xend)>>8;       //End Col High
    trans[1].tx_data[3]=(xend)&0xFF;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ystart>>8;        //Start page high
    trans[3].tx_data[1]=ystart&0xff;      //start page low
    trans[3].tx_data[2]=(yend)>>8;    //end page high
    trans[3].tx_data[3]=(yend)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=buf;        //finally send the line data
    trans[5].length=size * 8;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

    // Run the first 5 small transactions in polling mode to speed up the transfer
    for (x=0; x<5; x++) {
        ret = spi_device_polling_transmit(this->spi, &trans[x]);
        if(ret != ESP_OK) return DISPLAY_SPI_WRITE_FAILED;
    }

    // Last one in DMA mode as it's a long transfer, the CPU can do other things while drawing
    ret=spi_device_queue_trans(this->spi, &trans[5], portMAX_DELAY);
    if(ret != ESP_OK) return DISPLAY_SPI_WRITE_FAILED;
    this->send_line_finish();
    return DISPLAY_OK;
}

DISPLAY_STATUS disp::fillScreen(uint16_t colour) {
    uint16_t *buf = (uint16_t*) heap_caps_malloc((480 * PARALLEL_LINES) * sizeof(uint16_t), MALLOC_CAP_DMA);
    for(int x=0; x<480*PARALLEL_LINES; ++x) {
        buf[x] = colour;
    }

    DISPLAY_STATUS ret;

    for(int x=0; x<320; x+=PARALLEL_LINES - 1) {
        ret = this->send_lines(x, buf);
    }

    free(buf);
    return ret;    
}

DISPLAY_STATUS disp::drawText(uint16_t xpos, uint16_t ypos, uint8_t sizex, uint8_t sizey, const std::string str, const GFXfont *font, uint16_t colour) {

    uint16_t x = xpos, y = ypos;

    for(uint16_t pos = 0; pos < str.length(); ++pos) {

        uint8_t c = str[pos];

        // Check if our character is a newline
        if(c == '\n') {

            y+= font->yAdvance * sizey;
            x = xpos;
            continue;

        }

        // Quit once we hit the first invalid character
        if(c > font->last || c < font->first) return DISPLAY_ERR_TEXT_OUT_OF_RANGE;

        c -= font->first;
        uint16_t offset = font->glyph[c].bitmapOffset - 1;
        uint8_t ovfcounter = 0, fontByte = font->bitmap[offset];
        offset ++;

        for(int yfont = 0; yfont < font->glyph[c].height; ++yfont) {

            for(int xfont = 0; xfont < font->glyph[c].width; ++xfont) {

                if(!(ovfcounter++& 7)) fontByte = font->bitmap[offset++];

                if(fontByte & 0x80) {
                    DISPLAY_STATUS ret;
                    if(sizex == 1 && sizey == 1) {
                        ret = this->drawPixel(x+font->glyph[c].xOffset+xfont, y+font->glyph[c].yOffset+yfont, colour);
                    } else {
                        uint16_t xpos = x + (font->glyph[c].xOffset + xfont) * sizex;
                        uint16_t ypos = y + (font->glyph[c].yOffset + yfont) * sizey;
                        ret = this->drawRectangle(xpos, ypos, xpos + sizex, ypos + sizey, colour);
                    }

                    if(ret != DISPLAY_OK) return ret;
                }
                fontByte <<= 1;
            }
        }
        x += (font->glyph[c].xAdvance) * sizex;
    }

    return DISPLAY_OK;
}

DISPLAY_STATUS disp::drawTextFast(uint16_t xpos, uint16_t ypos, const std::string str, const GFXfont *font, uint16_t colour, uint16_t bgcolour) {
    // Fast mode ONLY works for no text scaling!
    uint16_t x = xpos, y = ypos;

    //offset cursor position if we're aligning the text differently
    if (this->alignment == RIGHT) {
        x -= this->getStringLength(1, str, font);
    } else if (this->alignment == CENTER) {
        x -= (this->getStringLength(1, str, font) / 2);
    }

    // Iterate through each character
    for(uint16_t pos = 0; pos < str.length(); ++pos) {

        //Basic Clipping
        //if(x >= this->width || y >= this->height) {
        //    return DISPLAY_ERR_TEXT_OUT_OF_RANGE; //Not exactly the most accurate error message
        //}

        uint8_t c = str[pos];

        // Check if our character is a newline
        if(c == '\n') {
            y+= font->yAdvance;
            x = xpos;
            continue;
        }
        // Quit once we hit the first invalid character
        if(c > font->last || c < font->first) return DISPLAY_ERR_TEXT_OUT_OF_RANGE;


        c -= font->first;
        uint16_t offset = font->glyph[c].bitmapOffset - 1;
        uint8_t ovfcounter = 0, fontByte = font->bitmap[offset];
        offset ++;

        uint32_t size = font->glyph[c].height * font->glyph[c].width;
        uint16_t *buf = (uint16_t*) heap_caps_malloc((size) * sizeof(uint16_t), MALLOC_CAP_DMA);

        // fill the buffer with the background colour
        for(int i = 0; i < size; ++i) {
            buf[i] = bgcolour;
        }

        // draw character to the buffer
        for(int yfont = 0; yfont < font->glyph[c].height; ++yfont) {
            for(int xfont = 0; xfont < font->glyph[c].width; ++xfont) {

                if(!(ovfcounter++& 7))  fontByte = font->bitmap[offset++];
                if(fontByte & 0x80)     buf[xfont + (font->glyph[c].width * yfont)] = colour;

                fontByte <<= 1;
            }
        }

        // draw buffer to the display
        DISPLAY_STATUS ret = this->draw(x + font->glyph[c].xOffset , x + font->glyph[c].xOffset + (font->glyph[c].width ) - 1, 
                   y + font->glyph[c].yOffset , y + font->glyph[c].yOffset + (font->glyph[c].height ) - 1, 
                   buf);

        if(ret != DISPLAY_OK) return ret;

        free(buf);

        x += (font->glyph[c].xAdvance);
    }

    return DISPLAY_OK;
}

DISPLAY_STATUS disp::drawBitmap(uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint16_t *buf) {
    // Draw up to 40 lines at once
    uint32_t vert = ((40 > height) ? height : 40);
    uint32_t size = width * vert;
    uint16_t *b = (uint16_t*) heap_caps_malloc((size) * sizeof(uint16_t), MALLOC_CAP_DMA);
    DISPLAY_STATUS ret = DISPLAY_OK;
    for(int y=0; y<height; y+=40) {
        if((height - y) < (40)) {
            memcpy(b, buf + (y * width), (height - y) * (width) * (sizeof(uint16_t)));
            ret = this->draw(xpos, xpos + width - 1, ypos + y, ypos + y + (height - y) - 1, b);
        }else{
            memcpy(b, buf + (y * width), size * (sizeof(uint16_t)));
            ret = this->draw(xpos, xpos + width - 1, ypos + y, ypos + y + vert - 1, b);
        }
    }

    return ret;
}

DISPLAY_STATUS disp::maskDisplay() {
    return this->spiWriteCMD(HX8357_DISPOFF);
}

DISPLAY_STATUS disp::unmaskDisplay() {
    return this->spiWriteCMD(HX8357_DISPON);
}

DISPLAY_STATUS disp::darkenAllPixels() {
    return this->spiWriteCMD(HX8357D_ALL_PIXEL_OFF);
}

DISPLAY_STATUS disp::recoverDarkenAllPixels() {
    return this->spiWriteCMD(HX8357B_NORON);
}

DISPLAY_STATUS disp::send_lines(int ypos, uint16_t *linedata) {
    return this->draw(0, 479, ypos, ypos+PARALLEL_LINES - 1, linedata);
}

DISPLAY_STATUS disp::send_line_finish() {
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<1; x++) {
        ret=spi_device_get_trans_result(this->spi, &rtrans, portMAX_DELAY);
        if(ret != ESP_OK) return DISPLAY_SPI_WRITE_FAILED;
    }
    return DISPLAY_OK;
}

DISPLAY_STATUS disp::initDisplay() {
    int cmd = 0;
    DISPLAY_STATUS ret = DISPLAY_OK;
    while(!(init[cmd].cmd == 0xFF && init[cmd].len == 0xFF)) {
        if(init[cmd].len > 0x80) {
            if(init[cmd].cmd != 0xFF) ret = this->spiWrite(init[cmd].cmd, (uint8_t*) &init[cmd].buf, 1);
            vTaskDelay(((init[cmd].len - 0x80) * 5) / portTICK_RATE_MS);
        } else {
            ret = this->spiWrite(init[cmd].cmd, (uint8_t*) &init[cmd].buf, init[cmd].len);
        }
        if(ret != DISPLAY_OK) return DISPLAY_SPI_WRITE_FAILED;
        cmd++;
    }

    // Fill the display with 0x0000 before we turn it on to prevent it from showing noise
    ret = this->fillScreen(0xFFFF);
    ret = this->spiWriteCMD(HX8357_DISPON);
    vTaskDelay(500 / portTICK_RATE_MS);
    if(ret != DISPLAY_OK) return DISPLAY_SPI_WRITE_FAILED;

    return DISPLAY_OK;
}

//Helper function returns the length in pixels of the string, used for alignment and spacing
uint16_t disp::getStringLength(uint8_t sizex, const std::string str, const GFXfont *font) {

    uint16_t length = 0;

    //Iterate through the string, summing the xAdvance values
    for(uint16_t pos = 0; pos < str.length(); ++pos) {

        uint8_t character = str[pos];

        character -= font->first;

        length += (font->glyph[character].xAdvance) * sizex;

    }

    return length;
}

DISPLAY_STATUS disp::setTextAlignment(TEXT_ALIGNMENT alignment_input) {

    this->alignment = alignment_input;
    return DISPLAY_OK;
}