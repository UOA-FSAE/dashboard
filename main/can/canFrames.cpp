#include <cstdint>
#include <cstdio>

#include "canFrames.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "vehicle/vehicleData.h"

#include "driver/gpio.h"
#include "driver/can.h"

void initCAN() {
    //Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();  //Change this to whatever the car is running at the time!

    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    t_config.triple_sampling = true;  //Enable triple sampling to try help noise?

    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("CAN Driver installed\n");
    } else {
        printf("Failed to install CAN driver\n");
        return;
    }

    //Start CAN driver
    if (can_start() == ESP_OK) {
        printf("CAN Driver started\n");
    } else {
        printf("Failed to start CAN driver\n");
        return;
    }

    //Prepare to trigger errors, reconfigure alerts to detect change in error state
    can_reconfigure_alerts(CAN_ALERT_ABOVE_ERR_WARN | CAN_ALERT_ERR_PASS | CAN_ALERT_BUS_OFF | CAN_ALERT_BUS_RECOVERED, NULL);
}

[[noreturn]] void updateCAN(void *pvParameters) {
    can_message_t message;

    auto message_buffers = (MessageBufferHandle_t *) pvParameters;

    struct Vehicle_Data data{};
    resetDataStructure(data);

    while(true) {

        //CAN recovery subroutine
        uint32_t alerts;
        can_read_alerts(&alerts, pdMS_TO_TICKS(0));
        if (alerts & CAN_ALERT_ABOVE_ERR_WARN) {
            printf("Surpassed Error Warning Limit\n");
        }

        if (alerts & CAN_ALERT_ERR_PASS) {
            printf("Error Passive\n");
        }

        if (alerts & CAN_ALERT_BUS_OFF) {
            printf("Bus Off!\n");
            printf("Recovering...\n");
            can_initiate_recovery();    //Needs 128 occurrences of bus free signal
        }

        if (alerts & CAN_ALERT_BUS_RECOVERED) {
            printf("Recovered!\n");     //Bus recovery was successful
        }

        //CAN Driver is interrupt based
        //Therefore no blocking is needed
        //Old dash 'polled?' 6 times in a row, running in a nonblocking RTOS task at 40Hz, so shall we
        for(uint8_t i = 0; i<7; ++i) {
            if (can_receive(&message, pdMS_TO_TICKS(0)) == ESP_OK) {

                //Print for debug
                //printf("ID 0x%03x\tData: ", message.identifier);
                //for (uint8_t i = 0; i < 8; ++i) {
                //  printf("0x%03x ", message.data[i]);
                //}
                //printf("\n");

                //Process the received message
                switch (message.identifier) {
                    case CAN_ID_ACCUMULATOR_PACK_DATA:
                        data.ts.current                 = (int16_t)((message.data[0]<<8 & 0xFF00) + message.data[1]);
                        data.ts.current                 = ((float)data.ts.current) / 10.0f;

                        data.ts.voltage                 = (message.data[2]<<8 & 0xFF00) + message.data[3];
                        data.ts.voltage                 = ((float)data.ts.current) / 10.0f;

                        data.ts.soc                     = message.data[4] / 2;

                        break;

                    case CAN_ID_ACCUMULATOR_VOLTAGES:
                        data.ts.maxVoltage              = (message.data[1]<<8 & 0xFF00) + message.data[0];
                        data.ts.maxVoltage              = ((float)data.ts.maxVoltage) / 10000.0f;

                        data.ts.avgVoltage              = (message.data[3]<<8 & 0xFF00) + message.data[2];
                        data.ts.avgVoltage              = ((float)data.ts.avgVoltage) / 10000.0f;

                        data.ts.minVoltage              = (message.data[5]<<8 & 0xFF00) + message.data[4];
                        data.ts.minVoltage              = ((float)data.ts.minVoltage) / 10000.0f;

                        data.ts.numCells                = message.data[6];

                        break;

                    case CAN_ID_ACCUMULATOR_TEMPERATURES:
                        data.ts.maxTemp                 = (message.data[1]<<8 & 0xFF00) + message.data[0];
                        data.ts.maxTemp                 = ((float)data.ts.maxTemp) / 10000.0f;

                        data.ts.avgTemp                 = (message.data[3]<<8 & 0xFF00) + message.data[2];
                        data.ts.avgTemp                 = ((float)data.ts.avgTemp) / 10000.0f;

                        data.ts.minTemp                 = (message.data[5]<<8 & 0xFF00) + message.data[4];
                        data.ts.minTemp                 = ((float)data.ts.minTemp) / 10000.0f;

                        break;

                    case CAN_ID_DRIVER_DATA:
                        data.driver.steeringAngle       = message.data[0];
                        data.driver.brakeBias           = message.data[1];
                        data.driver.frontBrakePressure  = message.data[2];
                        data.driver.rearBrakePressure   = message.data[3];
                        data.driver.throttle            = message.data[4];

                        //Override displayed brake bias if it's not even sent
                        if (data.driver.brakeBias == 0) {
                            data.driver.brakeBias = ((data.driver.frontBrakePressure * 100.0) / data.driver.rearBrakePressure);
                        }

                        break;

                    case CAN_ID_RACE_DATA:
                        data.race.currentLapTime      = (message.data[3]<<24 & 0xFF000000) + (message.data[2]<<16 & 0x00FF0000) + (message.data[1]<<8 & 0X0000FF00) + (message.data[0] & 0xFF) ;
                        data.race.currentSpeed        = message.data[4];
                        data.race.lapNumber           = message.data[5];

                        //Update the best lap time if it is actually better, and update the deltaLapTime
                        data.race.bestLapTime         = (data.race.bestLapTime <= data.race.currentLapTime) ? data.race.currentLapTime : data.race.bestLapTime;
                        data.race.deltaLapTime        = (data.race.currentLapTime - data.race.bestLapTime);

                        data.race.vehicleState.GLV            = (message.data[7] & 0x01) >> 0;
                        data.race.vehicleState.shutdown       = (message.data[7] & 0x02) >> 1;
                        data.race.vehicleState.precharging    = (message.data[7] & 0x04) >> 2;
                        data.race.vehicleState.precharged     = (message.data[7] & 0x08) >> 3;
                        data.race.vehicleState.RTDState       = (message.data[7] & 0x10) >> 4;

                        break;

                    case CAN_ID_GLV:
                        data.glv.soc                  = message.data[0];
                        data.glv.voltage              = (message.data[2]<<8 & 0xFF00) | (message.data[1] & 0x00FF);
                        data.glv.voltage              = ((float)data.glv.voltage) / 1000.0f;
                        data.glv.current              = (message.data[4]<<8 & 0xFF00) | (message.data[3] & 0x00FF);
                        data.glv.current              = ((float)data.glv.current) / 1000.0f;

                        break;

                    case CAN_ID_REAR_LEFT_DRIVE:
                        data.drive[0].gearboxTemp     = message.data[0];
                        data.drive[0].motorTemp       = message.data[1];
                        data.drive[0].inverterTemp    = message.data[2];
                        data.drive[0].derating        = (message.data[5] & 0x01) >> 0;
                        data.drive[0].driveActive     = (message.data[5] & 0x02) >> 1;
                        data.drive[0].errorCode       = (message.data[7]<<8 & 0xFF00) | message.data[6];

                        break;

                    case CAN_ID_FRONT_LEFT_DRIVE:
                        data.drive[1].gearboxTemp     = message.data[0];
                        data.drive[1].motorTemp       = message.data[1];
                        data.drive[1].inverterTemp    = message.data[2];
                        data.drive[1].derating        = (message.data[5] & 0x01) >> 0;
                        data.drive[1].driveActive     = (message.data[5] & 0x02) >> 1;
                        data.drive[1].errorCode       = (message.data[7]<<8 & 0xFF00) | message.data[6];

                        break;

                    case CAN_ID_FRONT_RIGHT_DRIVE:
                        data.drive[2].gearboxTemp     = message.data[0];
                        data.drive[2].motorTemp       = message.data[1];
                        data.drive[2].inverterTemp    = message.data[2];
                        data.drive[2].derating        = (message.data[5] & 0x01) >> 0;
                        data.drive[2].driveActive     = (message.data[5] & 0x02) >> 1;
                        data.drive[2].errorCode       = (message.data[7]<<8 & 0xFF00) | message.data[6];

                        break;

                    case CAN_ID_REAR_RIGHT_DRIVE:
                        data.drive[3].gearboxTemp     = message.data[0];
                        data.drive[3].motorTemp       = message.data[1];
                        data.drive[3].inverterTemp    = message.data[2];
                        data.drive[3].derating        = (message.data[5] & 0x01) >> 0;
                        data.drive[3].driveActive     = (message.data[5] & 0x02) >> 1;
                        data.drive[3].errorCode       = (message.data[7]<<8 & 0xFF00) | message.data[6];

                        break;

                    case CAN_ID_FAULT:
                        data.errors.PDOC              = (message.data[0] & 0x01) >> 0;
                        data.errors.AMS               = (message.data[0] & 0x02) >> 1;
                        data.errors.IMD               = (message.data[0] & 0x04) >> 2;
                        data.errors.BSPD              = (message.data[0] & 0x08) >> 3;

                        data.errors.throttlePlausability      =  (message.data[1] & 0x01) >> 0;
                        data.errors.brakeThrottleImplausability =  (message.data[1] & 0x02) >> 1;

                        break;

                }
            }
        }

        xMessageBufferSend(
                *(message_buffers),
                &data,
                sizeof(struct Vehicle_Data),
                0
        );

        xMessageBufferSend(
                *(message_buffers + 1),
                &data,
                sizeof(struct Vehicle_Data),
                0
        );

        vTaskDelay(((1/updateCANFreq)*1000)/portTICK_PERIOD_MS);
    }
}