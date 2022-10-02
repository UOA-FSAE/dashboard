#include <cstdlib>
#include <cstdint>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/message_buffer.h"
#include "ledControle.h"
#include "led.h"
#include "vehicle/vehicleData.h"

[[noreturn]] void updateLEDs(void* pvParameters) {

    //Array of LEDs
    CRGB* led = (CRGB*) malloc(sizeof(CRGB) * NUM_LEDS);
    //Instantiate controller
    auto* pSlowLed = new slowLED(NUM_LEDS);


    {   // Instantiate all pixels
        CHSV* hsv = (CHSV*) malloc(sizeof(CHSV) * NUM_LEDS);

        for(uint8_t x = 0; x < NUM_LEDS; x++) {
            hsv[x].h = 128;		//Each LED gets a different base colour
            hsv[x].s = 128;  					//Saturation during rainbow effect is fixed to 100%
            hsv[x].v = 128; 					//Vibrant during rainbow effect is fixed to 100%

            pSlowLed->hsv2rgb_rainbow(hsv[x],led[x]); 	//The desired colour is assigned to the current LED
        }

        // Set The KIWI_LED to Mclauren orange
        led[KIWI_LED]               = MCLAUREN_ORANGE;
    }

    // Message handle and data
    auto message_buffer = (MessageBufferHandle_t) pvParameters;
    struct Vehicle_Data data{};
    resetDataStructure(data);

    while(true) {
        if (xMessageBufferReceive(
                message_buffer,
                &data,
                sizeof(struct Vehicle_Data),
                pdMS_TO_TICKS(1000)
        )) {
            led[GLV_LED] = (data.race.vehicleState.GLV) ? GREEN : RED;
            led[SHUTDOWN_LED] = (data.race.vehicleState.shutdown) ? GREEN : RED;	//TODO: ADD SHUTDOWN STATUS

            if(data.race.vehicleState.precharged) {
                led[TRACTIVE_SYSTEM_LED] = GREEN;
            } else if(data.race.vehicleState.precharging) {
                led[TRACTIVE_SYSTEM_LED] = 0x0000FF;
            } else {
                led[TRACTIVE_SYSTEM_LED] = BLACK;
            }

            led[READY_TO_DRIVE_LED] = (data.race.vehicleState.RTDState) ? GREEN : BLACK;

            //Set top-left LEDs
            led[ERROR_1_LED]			= BLACK;
            led[ACCUMULATOR_WARN_LED] = (data.ts.soc >= 15) ? GREEN : RED;
            led[INVERTER_WARN_LED]		= BLACK;

            uint32_t colour[4];
            //Decide the colour for each of the motor LEDs
            //Based on Temperature or Error Code
            for(uint8_t i = 0; i < 4; ++i) {

                //LED usage priority list: Inactive, Derating, Temperature, Default

                if(!data.drive[i].driveActive) {	//MoTeC is saying the drive is not active, this is most important
                    colour[i] = RED;
                } else if(data.drive[i].derating) {
                    colour[i] = BLUE;	//Blue lmao
                } else if(data.drive[i].inverterTemp > 50 || data.drive[i].motorTemp > 60 || data.drive[i].gearboxTemp > 40) {		//Something is a little warm
                    colour[i] = YELLOW;
                } else {
                    colour[i] = GREEN;	//We all good somehow
                }
            }

            //Set Motor LEDs
            led[MOTOR_REAR_LEFT_LED]	= colour[0];
            led[MOTOR_FRONT_LEFT_LED]	= colour[1];
            led[MOTOR_FRONT_RIGHT_LED]	= colour[2];
            led[MOTOR_REAR_RIGHT_LED]	= colour[3];

            //Set top-right LEDs
            led[CONTROL_1_LED]			= data.errors.throttlePlausability ? RED : BLACK;
            led[CONTROL_2_LED]			= data.errors.brakeThrottleImplausability ? RED : BLACK;
            led[ERROR_2_LED]			= BLACK;

            //Set fault LEDs, these are easy: RED or OFF, and nothing fancy
            led[IMD_ERROR_LED] = (data.errors.IMD) ? RED : BLACK;
            led[AMS_ERROR_LED] = (data.errors.AMS) ? RED : BLACK;
            led[PDOC_ERROR_LED] = (data.errors.PDOC) ? RED : BLACK;
            led[BSPD_ERROR_LED] = (data.errors.BSPD) ? RED : BLACK;
        }
        else { // Case where no can message is received for over 1 second
            set_all_led_red(led);
        }

        //Push to LEDs
        pSlowLed->show(led);
    }
}

