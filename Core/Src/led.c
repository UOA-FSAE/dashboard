//
// Created by tazukiswift on 29/06/23.
//

#include "led.h"
#include "can_id.h"

Vehicle_Data *led_vehicle_data;
SPI_HandleTypeDef *spi_handle;

// TODO: make this buffer include the start and stop bits?
led_frame_type board_led_list[LED_NUMBER] = {0};

static const led_frame_type led_off = {0xE0, 0x00, 0x00, 0x00};

// Call this to initialize the array of LED's
void init_led(Vehicle_Data *ext_vehicle_data, SPI_HandleTypeDef *ext_hspi) {
    led_vehicle_data = ext_vehicle_data;
    spi_handle = ext_hspi;
    for (int i = 0; i < LED_NUMBER; i++) {
        board_led_list[i] = led_off;
    }
    set_all_red();
}

// TODO: make a wrapper for this function to set via color enum
void set_led(enum led_meaning led_id, uint8_t bright, uint8_t red, uint8_t green, uint8_t blue) {
    board_led_list[(int) led_id] = (led_frame_type) {(0b11100000 | bright), red, blue, green};
}

void set_all_red() {
    for (int selected_led = 0; selected_led < LED_NUMBER; selected_led++) {
        set_led((enum led_meaning) selected_led, 1, LED_RED);
    }
}

static uint16_t bytes_to_transmit = 4 * LED_NUMBER;

static const uint32_t start_word = 0x00000000;
static const uint32_t stop_word = 0xFFFFFFFF;

// Note: Clock polarity should be 1
// Updates all LED's, should only be called on CAN Interrupt
void update_led() {
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

// This function should probably update the LED values as well tbh
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // Probably need a more robust check :skull:
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
        Error_Handler();
    }
    switch (RxHeader.StdId) {
        case CAN_ID_ACCUMULATOR_PACK_DATA:
            led_vehicle_data->ts.current                 = (int16_t)((RxData[0]<<8 & 0xFF00) + RxData[1]);
            led_vehicle_data->ts.current                 = ((float)led_vehicle_data->ts.current) / 10.0f;

            led_vehicle_data->ts.voltage                 = (RxData[2]<<8 & 0xFF00) + RxData[3];
            led_vehicle_data->ts.voltage                 = ((float)led_vehicle_data->ts.current) / 10.0f;

            led_vehicle_data->ts.soc                     = RxData[4] / 2;


            // Update relevant LED's
            set_led(ERROR1,LED_MIN_BRIGHTNESS,LED_OFF);
            set_led(INVERTER,LED_MIN_BRIGHTNESS,LED_OFF);
            if(led_vehicle_data->ts.soc >= 15) {
                set_led(ACCUMULATOR,LED_MID_BRIGHTNESS,LED_GREEN);
            } else {
                set_led(ACCUMULATOR,LED_MID_BRIGHTNESS,LED_RED);
            }


            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_2:

            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_3:

            break;
        case CAN_ID_ACCUMULATOR_VOLTAGES:
            led_vehicle_data->ts.maxVoltage              = (RxData[1]<<8 & 0xFF00) + RxData[0];
            led_vehicle_data->ts.maxVoltage              = ((float)led_vehicle_data->ts.maxVoltage) / 10000.0f;

            led_vehicle_data->ts.avgVoltage              = (RxData[3]<<8 & 0xFF00) + RxData[2];
            led_vehicle_data->ts.avgVoltage              = ((float)led_vehicle_data->ts.avgVoltage) / 10000.0f;

            led_vehicle_data->ts.minVoltage              = (RxData[5]<<8 & 0xFF00) + RxData[4];
            led_vehicle_data->ts.minVoltage              = ((float)led_vehicle_data->ts.minVoltage) / 10000.0f;

            led_vehicle_data->ts.numCells                = RxData[6];

            break;

        case CAN_ID_ACCUMULATOR_TEMPERATURES:
            led_vehicle_data->ts.maxTemp                 = (RxData[1]<<8 & 0xFF00) + RxData[0];
            led_vehicle_data->ts.maxTemp                 = ((float)led_vehicle_data->ts.maxTemp) / 10000.0f;

            led_vehicle_data->ts.avgTemp                 = (RxData[3]<<8 & 0xFF00) + RxData[2];
            led_vehicle_data->ts.avgTemp                 = ((float)led_vehicle_data->ts.avgTemp) / 10000.0f;

            led_vehicle_data->ts.minTemp                 = (RxData[5]<<8 & 0xFF00) + RxData[4];
            led_vehicle_data->ts.minTemp                 = ((float)led_vehicle_data->ts.minTemp) / 10000.0f;
            break;
        case CAN_ID_GLV:
            led_vehicle_data->glv.soc                  = RxData[0];
            led_vehicle_data->glv.voltage              = (RxData[2]<<8 & 0xFF00) | (RxData[1] & 0x00FF);
            led_vehicle_data->glv.voltage              = ((float)led_vehicle_data->glv.voltage) / 1000.0f;
            led_vehicle_data->glv.current              = (RxData[4]<<8 & 0xFF00) | (RxData[3] & 0x00FF);
            led_vehicle_data->glv.current              = ((float)led_vehicle_data->glv.current) / 1000.0f;
            break;
        case CAN_ID_FAULT:

            break;
        case CAN_ID_RACE_DATA:
            led_vehicle_data->race.currentLapTime      = (RxData[3]<<24 & 0xFF000000) + (RxData[2]<<16 & 0x00FF0000) + (RxData[1]<<8 & 0X0000FF00) + (RxData[0] & 0xFF) ;
            led_vehicle_data->race.currentSpeed        = RxData[4];
            led_vehicle_data->race.lapNumber           = RxData[5];

            //Update the best lap time if it is actually better, and update the deltaLapTime
            led_vehicle_data->race.bestLapTime         = (led_vehicle_data->race.bestLapTime <= led_vehicle_data->race.currentLapTime) ? led_vehicle_data->race.currentLapTime : led_vehicle_data->race.bestLapTime;
            led_vehicle_data->race.deltaLapTime        = (led_vehicle_data->race.currentLapTime - led_vehicle_data->race.bestLapTime);

            led_vehicle_data->race.vehicleState.GLV            = (RxData[7] & 0x01) >> 0;
            led_vehicle_data->race.vehicleState.shutdown       = (RxData[7] & 0x02) >> 1;
            led_vehicle_data->race.vehicleState.precharging    = (RxData[7] & 0x04) >> 2;
            led_vehicle_data->race.vehicleState.precharged     = (RxData[7] & 0x08) >> 3;
            led_vehicle_data->race.vehicleState.RTDState       = (RxData[7] & 0x10) >> 4;

            // Handle LED's
            if (led_vehicle_data->race.vehicleState.GLV) {
                set_led(GLV,LED_MID_BRIGHTNESS,LED_GREEN);
            } else {
                set_led(GLV,LED_MID_BRIGHTNESS,LED_RED);
            }
            if (led_vehicle_data->race.vehicleState.shutdown) {
                set_led(SHUTDOWN,LED_MID_BRIGHTNESS,LED_GREEN);
            } else {
                set_led(SHUTDOWN,LED_MID_BRIGHTNESS,LED_RED);
            }
            if(led_vehicle_data->race.vehicleState.precharged) {
                set_led(TS_ACT,LED_MID_BRIGHTNESS,LED_GREEN);
            } else if(led_vehicle_data->race.vehicleState.precharged) {
                set_led(TS_ACT,LED_MID_BRIGHTNESS,LED_BLUE);
            } else {
                set_led(TS_ACT,LED_MIN_BRIGHTNESS,LED_OFF);
            }

            if(led_vehicle_data->race.vehicleState.RTDState) {
                set_led(READY,LED_MID_BRIGHTNESS,LED_GREEN);
            } else {
                set_led(READY,LED_MIN_BRIGHTNESS,LED_OFF);
            }

            break;
        case CAN_ID_DRIVER_DATA:
            led_vehicle_data->driver.steeringAngle       = RxData[0];
            led_vehicle_data->driver.brakeBias           = RxData[1];
            led_vehicle_data->driver.frontBrakePressure  = RxData[2];
            led_vehicle_data->driver.rearBrakePressure   = RxData[3];
            led_vehicle_data->driver.throttle            = RxData[4];

            //Override displayed brake bias if it's not even sent
            if (led_vehicle_data->driver.brakeBias == 0) {
                led_vehicle_data->driver.brakeBias = ((led_vehicle_data->driver.frontBrakePressure * 100.0) / led_vehicle_data->driver.rearBrakePressure);
            }

            break;

        case CAN_ID_REAR_LEFT_DRIVE:
            led_vehicle_data->drive[0].gearboxTemp     = RxData[0];
            led_vehicle_data->drive[0].motorTemp       = RxData[1];
            led_vehicle_data->drive[0].inverterTemp    = RxData[2];
            led_vehicle_data->drive[0].derating        = (RxData[5] & 0x01) >> 0;
            led_vehicle_data->drive[0].driveActive     = (RxData[5] & 0x02) >> 1;
            led_vehicle_data->drive[0].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];

            //LED usage priority list: Inactive, Derating, Temperature, Default
            if(!led_vehicle_data->drive[0].driveActive) {	//MoTeC is saying the drive is not active, this is most important
                set_led(RL,LED_MID_BRIGHTNESS,LED_RED);
            } else if(led_vehicle_data->drive[0].derating) {
                set_led(RL,LED_MID_BRIGHTNESS,LED_BLUE);    // Lmao indeed
            } else if(led_vehicle_data->drive[0].inverterTemp > 50 || led_vehicle_data->drive[0].motorTemp > 60 || led_vehicle_data->drive[0].gearboxTemp > 40) {		//Something is a little warm
                set_led(RL,LED_MID_BRIGHTNESS,LED_YELLOW);    // Lmao indeed
            } else {
                set_led(RL,LED_MID_BRIGHTNESS,LED_GREEN);	// Everything works
            }
            break;

        case CAN_ID_FRONT_LEFT_DRIVE:
            led_vehicle_data->drive[1].gearboxTemp     = RxData[0];
            led_vehicle_data->drive[1].motorTemp       = RxData[1];
            led_vehicle_data->drive[1].inverterTemp    = RxData[2];
            led_vehicle_data->drive[1].derating        = (RxData[5] & 0x01) >> 0;
            led_vehicle_data->drive[1].driveActive     = (RxData[5] & 0x02) >> 1;
            led_vehicle_data->drive[1].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            //LED usage priority list: Inactive, Derating, Temperature, Default
            if(!led_vehicle_data->drive[1].driveActive) {	//MoTeC is saying the drive is not active, this is most important
                set_led(FL,LED_MID_BRIGHTNESS,LED_RED);
            } else if(led_vehicle_data->drive[1].derating) {
                set_led(FL,LED_MID_BRIGHTNESS,LED_BLUE);    // Lmao indeed
            } else if(led_vehicle_data->drive[1].inverterTemp > 50 || led_vehicle_data->drive[1].motorTemp > 60 || led_vehicle_data->drive[1].gearboxTemp > 40) {		//Something is a little warm
                set_led(FL,LED_MID_BRIGHTNESS,LED_YELLOW);    // Lmao indeed
            } else {
                set_led(FL,LED_MID_BRIGHTNESS,LED_GREEN);	// Everything works
            }
            break;

        case CAN_ID_FRONT_RIGHT_DRIVE:
            led_vehicle_data->drive[2].gearboxTemp     = RxData[0];
            led_vehicle_data->drive[2].motorTemp       = RxData[1];
            led_vehicle_data->drive[2].inverterTemp    = RxData[2];
            led_vehicle_data->drive[2].derating        = (RxData[5] & 0x01) >> 0;
            led_vehicle_data->drive[2].driveActive     = (RxData[5] & 0x02) >> 1;
            led_vehicle_data->drive[2].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            //LED usage priority list: Inactive, Derating, Temperature, Default
            if(!led_vehicle_data->drive[2].driveActive) {	//MoTeC is saying the drive is not active, this is most important
                set_led(FR,LED_MID_BRIGHTNESS,LED_RED);
            } else if(led_vehicle_data->drive[2].derating) {
                set_led(FL,LED_MID_BRIGHTNESS,LED_BLUE);    // Lmao indeed
            } else if(led_vehicle_data->drive[2].inverterTemp > 50 || led_vehicle_data->drive[2].motorTemp > 60 || led_vehicle_data->drive[2].gearboxTemp > 40) {		//Something is a little warm
                set_led(FL,LED_MID_BRIGHTNESS,LED_YELLOW);    // Lmao indeed
            } else {
                set_led(FL,LED_MID_BRIGHTNESS,LED_GREEN);	// Everything works
            }
            break;

        case CAN_ID_REAR_RIGHT_DRIVE:
            led_vehicle_data->drive[3].gearboxTemp     = RxData[0];
            led_vehicle_data->drive[3].motorTemp       = RxData[1];
            led_vehicle_data->drive[3].inverterTemp    = RxData[2];
            led_vehicle_data->drive[3].derating        = (RxData[5] & 0x01) >> 0;
            led_vehicle_data->drive[3].driveActive     = (RxData[5] & 0x02) >> 1;
            led_vehicle_data->drive[3].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            //LED usage priority list: Inactive, Derating, Temperature, Default
            if(!led_vehicle_data->drive[3].driveActive) {	//MoTeC is saying the drive is not active, this is most important
                set_led(RR,LED_MID_BRIGHTNESS,LED_RED);
            } else if(led_vehicle_data->drive[3].derating) {
                set_led(RR,LED_MID_BRIGHTNESS,LED_BLUE);    // Lmao indeed
            } else if(led_vehicle_data->drive[3].inverterTemp > 50 || led_vehicle_data->drive[3].motorTemp > 60 || led_vehicle_data->drive[3].gearboxTemp > 40) {		//Something is a little warm
                set_led(RR,LED_MID_BRIGHTNESS,LED_YELLOW);    // Lmao indeed
            } else {
                set_led(RR,LED_MID_BRIGHTNESS,LED_GREEN);	// Everything works
            }
            break;
        default:
            return; // We don't want to update_led on every can message
    }
    // Update LED on interrupt, should be lower frequency than main loop
    update_led();
}

