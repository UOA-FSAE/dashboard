#ifndef __LED_H
#define __LED_H

#define PIN_NUM_LED_CLK GPIO_NUM_22
#define PIN_NUM_LED_DAT GPIO_NUM_23

#define KIWI_LED              0

#define GLV_LED               1
#define SHUTDOWN_LED          2
#define TRACTIVE_SYSTEM_LED   3
#define READY_TO_DRIVE_LED    4

#define ERROR_1_LED           5
#define ACCUMULATOR_WARN_LED  6
#define INVERTER_WARN_LED     7

#define MOTOR_REAR_LEFT_LED   8
#define MOTOR_FRONT_LEFT_LED  9
#define MOTOR_FRONT_RIGHT_LED 10
#define MOTOR_REAR_RIGHT_LED  11

#define CONTROL_1_LED         12
#define CONTROL_2_LED         13
#define ERROR_2_LED           14

#define BSPD_ERROR_LED        15
#define AMS_ERROR_LED         16
#define IMD_ERROR_LED         17
#define PDOC_ERROR_LED        18

#define NUM_LEDS              19

// Colours
#define MCLAUREN_ORANGE       0xF68F21
#define WIKIPEDIA_ORANGE      0xFF7F00
#define SAFETY_ORANGE         0xFF6600

#define RED                   0xFF0000
#define YELLOW                0xFFFF00
#define GREEN                 0x00FF00
#define BLUE                  0x0000FF
#define BLACK                 0x000000

#include <cstdint>

enum LED_STATUS{
    LED_OK
};

//forward declaration
struct CRGB;
struct CHSV;

void hsv2rgb_rainbow( const CHSV& hsv, CRGB& rgb);

/// Representation of an 888 HSV pixel (hue, saturation, value (aka brightness)).
struct CHSV {
    union {
        struct {
            union {
                uint8_t hue;
                uint8_t h; };
            union {
                uint8_t saturation;
                uint8_t sat;
                uint8_t s; };
            union {
                uint8_t value;
                uint8_t val;
                uint8_t v; };
        };
        uint8_t raw[3];
    };

    /// default values are UNITIALIZED
    inline CHSV() __attribute__((always_inline))
    {
    }

    /// allow construction from H, S, V
    inline CHSV( uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline))
    : h(ih), s(is), v(iv)
    {
    }

    /// allow copy construction
    inline CHSV(const CHSV& rhs) __attribute__((always_inline))
    {
        h = rhs.h;
        s = rhs.s;
        v = rhs.v;
    }
};

/// Representation of an 888 RGB pixel (Red, Green, Blue) with extra 5-bit brightness used when writing to SK9822 LEDs
struct CRGB {
    struct {
        union {
            uint8_t r;
            uint8_t red;
        };
        union {
            uint8_t g;
            uint8_t green;
        };
        union {
            uint8_t b;
            uint8_t blue;
        };
    };
    uint8_t brightness;

    // default values are UNINITIALIZED
    inline CRGB() __attribute__((always_inline)){}

    /// allow construction from R, G, B
    inline CRGB( uint8_t ir, uint8_t ig, uint8_t ib)
    __attribute__((always_inline)): r(ir), g(ig), b(ib), brightness(31)
    {}

    /// allow construction from 32-bit (really 24-bit) bit 0xRRGGBB color code
    inline CRGB( uint32_t colorcode)
    __attribute__((always_inline)): r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF), brightness(31)
    {}

    /// allow copy construction
    inline CRGB(const CRGB& rhs) __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        brightness = rhs.brightness;
    }

    /// allow construction from HSV color
    inline CRGB(const CHSV& rhs) __attribute__((always_inline))
    {
        hsv2rgb_rainbow( rhs, *this);
    }

    /// allow assignment from one RGB struct to another
    inline CRGB& operator= (const CRGB& rhs) __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        brightness = rhs.brightness;
        return *this;
    }

    /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    inline CRGB& operator= (const uint32_t colorcode) __attribute__((always_inline))
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >>  8) & 0xFF;
        b = (colorcode >>  0) & 0xFF;
        return *this;
    }

    // allow assignment from HSV color
    inline CRGB& operator= (const CHSV& rhs) __attribute__((always_inline))
    {
        hsv2rgb_rainbow( rhs, *this);
        return *this;
    }

    /// allow assignment from R, G, and B
    inline CRGB& setRGB (uint8_t nr, uint8_t ng, uint8_t nb) __attribute__((always_inline))
    {
        r = nr;
        g = ng;
        b = nb;
        return *this;
    }

    /// allow assignment from H, S, and V
    inline CRGB& setHSV (uint8_t hue, uint8_t sat, uint8_t val) __attribute__((always_inline))
    {
        hsv2rgb_rainbow( CHSV(hue, sat, val), *this);
        return *this;
    }

    /// allow assignment from just a Hue, saturation and value automatically at max.
    inline CRGB& setHue (uint8_t hue) __attribute__((always_inline))
    {
        hsv2rgb_rainbow( CHSV(hue, 255, 255), *this);
        return *this;
    }

    /// Predefined RGB colors
    typedef enum {
        AliceBlue=0xF0F8FF,
        Amethyst=0x9966CC,
        AntiqueWhite=0xFAEBD7,
        Aqua=0x00FFFF,
        Aquamarine=0x7FFFD4,
        Azure=0xF0FFFF,
        Beige=0xF5F5DC,
        Bisque=0xFFE4C4,
        Black=0x000000,
        BlanchedAlmond=0xFFEBCD,
        Blue=0x0000FF,
        BlueViolet=0x8A2BE2,
        Brown=0xA52A2A,
        BurlyWood=0xDEB887,
        CadetBlue=0x5F9EA0,
        Chartreuse=0x7FFF00,
        Chocolate=0xD2691E,
        Coral=0xFF7F50,
        CornflowerBlue=0x6495ED,
        Cornsilk=0xFFF8DC,
        Crimson=0xDC143C,
        Cyan=0x00FFFF,
        DarkBlue=0x00008B,
        DarkCyan=0x008B8B,
        DarkGoldenrod=0xB8860B,
        DarkGray=0xA9A9A9,
        DarkGrey=0xA9A9A9,
        DarkGreen=0x006400,
        DarkKhaki=0xBDB76B,
        DarkMagenta=0x8B008B,
        DarkOliveGreen=0x556B2F,
        DarkOrange=0xFF8C00,
        DarkOrchid=0x9932CC,
        DarkRed=0x8B0000,
        DarkSalmon=0xE9967A,
        DarkSeaGreen=0x8FBC8F,
        DarkSlateBlue=0x483D8B,
        DarkSlateGray=0x2F4F4F,
        DarkSlateGrey=0x2F4F4F,
        DarkTurquoise=0x00CED1,
        DarkViolet=0x9400D3,
        DeepPink=0xFF1493,
        DeepSkyBlue=0x00BFFF,
        DimGray=0x696969,
        DimGrey=0x696969,
        DodgerBlue=0x1E90FF,
        FireBrick=0xB22222,
        FloralWhite=0xFFFAF0,
        ForestGreen=0x228B22,
        Fuchsia=0xFF00FF,
        Gainsboro=0xDCDCDC,
        GhostWhite=0xF8F8FF,
        Gold=0xFFD700,
        Goldenrod=0xDAA520,
        Gray=0x808080,
        Grey=0x808080,
        Green=0x008000,
        GreenYellow=0xADFF2F,
        Honeydew=0xF0FFF0,
        HotPink=0xFF69B4,
        IndianRed=0xCD5C5C,
        Indigo=0x4B0082,
        Ivory=0xFFFFF0,
        Khaki=0xF0E68C,
        Lavender=0xE6E6FA,
        LavenderBlush=0xFFF0F5,
        LawnGreen=0x7CFC00,
        LemonChiffon=0xFFFACD,
        LightBlue=0xADD8E6,
        LightCoral=0xF08080,
        LightCyan=0xE0FFFF,
        LightGoldenrodYellow=0xFAFAD2,
        LightGreen=0x90EE90,
        LightGrey=0xD3D3D3,
        LightPink=0xFFB6C1,
        LightSalmon=0xFFA07A,
        LightSeaGreen=0x20B2AA,
        LightSkyBlue=0x87CEFA,
        LightSlateGray=0x778899,
        LightSlateGrey=0x778899,
        LightSteelBlue=0xB0C4DE,
        LightYellow=0xFFFFE0,
        Lime=0x00FF00,
        LimeGreen=0x32CD32,
        Linen=0xFAF0E6,
        Magenta=0xFF00FF,
        Maroon=0x800000,
        MediumAquamarine=0x66CDAA,
        MediumBlue=0x0000CD,
        MediumOrchid=0xBA55D3,
        MediumPurple=0x9370DB,
        MediumSeaGreen=0x3CB371,
        MediumSlateBlue=0x7B68EE,
        MediumSpringGreen=0x00FA9A,
        MediumTurquoise=0x48D1CC,
        MediumVioletRed=0xC71585,
        MidnightBlue=0x191970,
        MintCream=0xF5FFFA,
        MistyRose=0xFFE4E1,
        Moccasin=0xFFE4B5,
        NavajoWhite=0xFFDEAD,
        Navy=0x000080,
        OldLace=0xFDF5E6,
        Olive=0x808000,
        OliveDrab=0x6B8E23,
        Orange=0xFFA500,
        OrangeRed=0xFF4500,
        Orchid=0xDA70D6,
        PaleGoldenrod=0xEEE8AA,
        PaleGreen=0x98FB98,
        PaleTurquoise=0xAFEEEE,
        PaleVioletRed=0xDB7093,
        PapayaWhip=0xFFEFD5,
        PeachPuff=0xFFDAB9,
        Peru=0xCD853F,
        Pink=0xFFC0CB,
        Plaid=0xCC5533,
        Plum=0xDDA0DD,
        PowderBlue=0xB0E0E6,
        Purple=0x800080,
        Red=0xFF0000,
        RosyBrown=0xBC8F8F,
        RoyalBlue=0x4169E1,
        SaddleBrown=0x8B4513,
        Salmon=0xFA8072,
        SandyBrown=0xF4A460,
        SeaGreen=0x2E8B57,
        Seashell=0xFFF5EE,
        Sienna=0xA0522D,
        Silver=0xC0C0C0,
        SkyBlue=0x87CEEB,
        SlateBlue=0x6A5ACD,
        SlateGray=0x708090,
        SlateGrey=0x708090,
        Snow=0xFFFAFA,
        SpringGreen=0x00FF7F,
        SteelBlue=0x4682B4,
        Tan=0xD2B48C,
        Teal=0x008080,
        Thistle=0xD8BFD8,
        Tomato=0xFF6347,
        Turquoise=0x40E0D0,
        Violet=0xEE82EE,
        Wheat=0xF5DEB3,
        White=0xFFFFFF,
        WhiteSmoke=0xF5F5F5,
        Yellow=0xFFFF00,
        YellowGreen=0x9ACD32,

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight=0xFFE42D,
        // If you are using no color correction, use this
        FairyLightNCC=0xFF9D2A

    } HTMLColorCode;
};

//Colour code for typical LED colour correction from fastLED
const uint32_t colourCorrection = 0xFFB0F0;

class slowLED{
        private:

        uint16_t numLEDs;

        static uint8_t scale8(uint8_t value, uint8_t scale);

        static LED_STATUS writeByte(uint8_t data);
        LED_STATUS writeLED(uint8_t brightness, uint8_t b0, uint8_t b1, uint8_t b2);

        void startFrame();
        void endFrame();

        public:

        explicit slowLED(uint16_t num);
        LED_STATUS show(CRGB *leds);
        void hsv2rgb_rainbow( const struct CHSV& hsv, struct CRGB& rgb);

};

void set_all_led_red(CRGB*);
void set_all_led_green(CRGB*);
void set_all_led_yellow(CRGB*);


#endif