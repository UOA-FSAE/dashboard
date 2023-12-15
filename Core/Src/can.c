/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "can_ids.h"
#include "vehicle.h"
#include "led.h"
#include "screens.h"
#include "tim.h"
#include "dash_error.h"
#include <stdint.h>
#include <string.h>

extern volatile Vehicle_Data the_vehicle;
extern volatile enum DASH_ERROR_TYPE current_error;
volatile bool RTDS_FLAG = false;
volatile bool CYCLE_SCREEN_FLAG = false;
volatile bool TOGGLE_POPUP_FLAG = false;

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

int16_t twos_complement_buffer;

/* CAN1 init function */
void MX_CAN1_Init(void) {

    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 9;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */
    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;


    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        Error_Handler();
    }


    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }


    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspInit 0 */

        /* USER CODE END CAN1_MspInit 0 */
        /* CAN1 clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**CAN1 GPIO Configuration
        PA11     ------> CAN1_RX
        PA12     ------> CAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle) {

    if (canHandle->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
        PA11     ------> CAN1_RX
        PA12     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
static volatile CAN_TxHeaderTypeDef TxHeader;
static volatile uint32_t TxMailbox;

void send_can_message(int std_id, uint8_t *buffer, int length) {

    TxHeader.IDE = CAN_ID_STD;
    TxHeader.StdId = std_id;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = length;

//      HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buffer, &TxMailbox);
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buffer, &TxMailbox) != HAL_OK) {
        current_error = CAN_ERROR;
        Error_Handler();
    }
}

float exp_lookup[5] = {1.0f, 0.1f, 0.01f, 0.001f, 0.0001f}; // Don't tell Chris

float process_can_decimal(int integer, int neg_exponent) {
    return (float) integer * exp_lookup[neg_exponent];
}

enum led_meaning drive_led_map[4] = {RL,FL,FR,RR};

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    uint32_t lap_time;  // Used in the case statement

    // Probably need a more robust check :skull:
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
        Error_Handler();
    }
    int i = 3;

    if (RxHeader.IDE != CAN_ID_STD) return;

    switch (RxHeader.StdId) {
        case CAN_ID_ACCUMULATOR_PACK_DATA:
            the_vehicle.ts.current = (int16_t) ((RxData[0] << 8 & 0xFF00) + RxData[1]);
            the_vehicle.ts.current = ((float) the_vehicle.ts.current) / 10.0f;

            the_vehicle.ts.voltage = (RxData[2] << 8 & 0xFF00) + RxData[3];
            the_vehicle.ts.voltage = ((float) the_vehicle.ts.current) / 10.0f;

//            the_vehicle.ts.soc = RxData[4] / 2;       LMAO

            set_led(ERROR1, LED_BLACK);
            if (the_vehicle.ts.soc) {
                set_led(ACCUMULATOR, LED_GREEN);
            } else {
                set_led(ACCUMULATOR, LED_RED);
            }
            set_led(INVERTER, LED_BLACK);


            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_2:

            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_3:

            break;

        case CAN_ID_ACCUMULATOR_VOLTAGES:
            the_vehicle.ts.maxVoltage = (RxData[0] << 8 & 0xFF00) + RxData[1];
            the_vehicle.ts.maxVoltage = ((float) the_vehicle.ts.maxVoltage) / 10000.0f;

            the_vehicle.ts.avgVoltage = (RxData[2] << 8 & 0xFF00) + RxData[3];
            the_vehicle.ts.avgVoltage = ((float) the_vehicle.ts.avgVoltage) / 10000.0f;

            the_vehicle.ts.minVoltage = (RxData[4] << 8 & 0xFF00) + RxData[5];
            the_vehicle.ts.minVoltage = ((float) the_vehicle.ts.minVoltage) / 10000.0f;

            the_vehicle.ts.numCells = RxData[6];
            break;

        case CAN_ID_ACCUMULATOR_TEMPERATURES:
            the_vehicle.ts.maxTemp = (RxData[1] << 8 & 0xFF00) + RxData[0];
            the_vehicle.ts.maxTemp = ((float) the_vehicle.ts.maxTemp) / 10000.0f;

            the_vehicle.ts.avgTemp = (RxData[3] << 8 & 0xFF00) + RxData[2];
            the_vehicle.ts.avgTemp = ((float) the_vehicle.ts.avgTemp) / 10000.0f;

            the_vehicle.ts.minTemp = (RxData[5] << 8 & 0xFF00) + RxData[4];
            the_vehicle.ts.minTemp = ((float) the_vehicle.ts.minTemp) / 10000.0f;
            break;
        case CAN_ID_GLV:
            the_vehicle.glv.soc = RxData[0];
            the_vehicle.glv.voltage = (RxData[2] << 8 & 0xFF00) | (RxData[1] & 0x00FF);
            the_vehicle.glv.voltage = ((float) the_vehicle.glv.voltage) / 1000.0f;
            the_vehicle.glv.current = (RxData[4] << 8 & 0xFF00) | (RxData[3] & 0x00FF);
            the_vehicle.glv.current = ((float) the_vehicle.glv.current) / 1000.0f;
            break;
        case CAN_ID_FAULT:
            the_vehicle.errors.PDOC = (RxData[0] & 0x01) >> 0;
            the_vehicle.errors.AMS = (RxData[0] & 0x02) >> 1;
            the_vehicle.errors.IMD = (RxData[0] & 0x04) >> 2;
            the_vehicle.errors.BSPD = (RxData[0] & 0x08) >> 3;

            the_vehicle.errors.throttlePlausability = (RxData[1] & 0x01) >> 0;
            the_vehicle.errors.brakeThrottleImplausability = (RxData[1] & 0x02) >> 1;

            if (the_vehicle.errors.throttlePlausability) {
                set_led(CTRL1, LED_RED);
            } else {
                set_led(CTRL1, LED_BLACK);
            }

            if (the_vehicle.errors.brakeThrottleImplausability) {
                set_led(CTRL2, LED_RED);
            } else {
                set_led(CTRL2, LED_BLACK);
            }

            //Set fault LEDs, these are easy: RED or OFF, and nothing fancy
            if (the_vehicle.errors.IMD) {
                set_led(IMD, LED_RED);
                queue_change_screens(ERROR_SCREEN);
            } else {
                set_led(IMD, LED_BLACK);
            }

            if (the_vehicle.errors.AMS) {
                set_led(AMS, LED_RED);
                queue_change_screens(ERROR_SCREEN);
            } else {
                set_led(AMS, LED_BLACK);
            }

            if (the_vehicle.errors.PDOC) {
                set_led(PDOC, LED_RED);
                queue_change_screens(ERROR_SCREEN);
            } else {
                set_led(PDOC, LED_BLACK);
            }

            if (the_vehicle.errors.BSPD) {
                set_led(BSPD, LED_RED);
                queue_change_screens(ERROR_SCREEN);
            } else {
                set_led(BSPD, LED_BLACK);
            }

            set_led(ERROR2, LED_BLACK);

            break;
        case CAN_ID_RACE_DATA:  // TODO: Implement bytes 0->6
            the_vehicle.race.lapTravel = (((RxData[2] << 8) & 0xFF00) + ((RxData[1]) & 0x00FF));
            the_vehicle.race.sessionTravel = (((RxData[4] << 8) & 0xFF00) + ((RxData[3]) & 0x00FF));
            the_vehicle.race.rpmAverage = (((RxData[6] << 8) & 0xFF00) + ((RxData[5]) & 0x00FF));

            the_vehicle.race.vehicleState.GLV = (RxData[7] & 0x01) >> 0;
            the_vehicle.race.vehicleState.shutdown = (RxData[7] & 0x02) >> 1;
            the_vehicle.race.vehicleState.precharging = (RxData[7] & 0x04) >> 2;
            the_vehicle.race.vehicleState.precharged = (RxData[7] & 0x08) >> 3;
            the_vehicle.race.vehicleState.RTDState = (RxData[7] & 0x10) >> 4;

            if (the_vehicle.race.vehicleState.precharged) {
                set_led(TS_ACT, LED_GREEN);
            } else if (the_vehicle.race.vehicleState.precharging) {
                set_led(TS_ACT, LED_BLUE);
            } else {
                set_led(TS_ACT, LED_BLACK);
            }

            if (the_vehicle.race.vehicleState.GLV) {
                set_led(GLV, LED_GREEN);
            } else {
                set_led(GLV, LED_RED);
            }
            if (the_vehicle.race.vehicleState.shutdown) {
                set_led(SHUTDOWN, LED_GREEN);
            } else {
                set_led(SHUTDOWN, LED_RED);
            }    // TODO: add shutdown status
            if (the_vehicle.race.vehicleState.RTDState) {
                if (!RTDS_FLAG) {
                    RTDS_FLAG = true;
                    HAL_TIM_Base_Start_IT(&htim13);
                }
                set_led(READY, LED_GREEN);
            } else {
                RTDS_FLAG = false;
                set_led(READY, LED_BLACK);
            }
            break;
        case CAN_ID_LAP_DATA:
            the_vehicle.race.currentLapTime =
                    (RxData[3] << 24 & 0xFF000000) + (RxData[2] << 16 & 0x00FF0000) + (RxData[1] << 8 & 0X0000FF00) +
                    (RxData[0] & 0xFF);
            memcpy(&twos_complement_buffer,&RxData[5],2*sizeof(uint8_t));
            the_vehicle.race.deltaLapTime = 0.01f*(float)(twos_complement_buffer);
            the_vehicle.race.lapNumber = RxData[7];
            break;

        case CAN_ID_LAP_DATA_2:
            the_vehicle.race.bestLapTime =
                    (RxData[3] << 24 & 0xFF000000) + (RxData[2] << 16 & 0x00FF0000) + (RxData[1] << 8 & 0X0000FF00) +
                    (RxData[0] & 0xFF);
            the_vehicle.race.previousLapTime =
                    (RxData[7] << 24 & 0xFF000000) + (RxData[6] << 16 & 0x00FF0000) + (RxData[5] << 8 & 0X0000FF00) +
                    (RxData[4] & 0xFF);
            break;
        case CAN_ID_DRIVER_DATA:
            memcpy(&the_vehicle.driver.steeringAngle,&RxData[0],sizeof(uint8_t));   // I am the binary god
            the_vehicle.driver.brakeBias = RxData[1];
            the_vehicle.driver.frontBrakePressure = RxData[2];
            the_vehicle.driver.rearBrakePressure = RxData[3];
            the_vehicle.driver.throttle = RxData[4];

//            //Override displayed brake bias if it's not even sent
//            if (the_vehicle.driver.brakeBias == 0) {
//                the_vehicle.driver.brakeBias = ((the_vehicle.driver.frontBrakePressure * 100.0) /
//                                                the_vehicle.driver.rearBrakePressure);
//            }
            break;
        case CAN_ID_DRIVER_DATA_2:
            the_vehicle.driver.torque = 0.1f * (float)(RxData[0]);
            the_vehicle.driver.speed_constant = ((RxData[2] << 8) & 0xFF00) + ((RxData[1]) & 0xFF);
            the_vehicle.driver.inverter_cooling_temp = RxData[4];
            the_vehicle.driver.motor_cooling_temp = RxData[5];
            the_vehicle.ts.soc = RxData[3];
            break;
        case CAN_ID_REAR_LEFT_DRIVE:
            i--;
        case CAN_ID_FRONT_LEFT_DRIVE:
            i--;
        case CAN_ID_FRONT_RIGHT_DRIVE:
            i--;
        case CAN_ID_REAR_RIGHT_DRIVE:
            the_vehicle.drive[i].gearboxTemp = RxData[0];
            the_vehicle.drive[i].motorTemp = RxData[1];
            the_vehicle.drive[i].inverterTemp = RxData[2];
            the_vehicle.drive[i].derating = (RxData[5] & 0x01) >> 0;
            the_vehicle.drive[i].driveActive = (RxData[5] & 0x02) >> 1;
            the_vehicle.drive[i].errorCode = (RxData[7] << 8 & 0xFF00) | RxData[6];

            if (!the_vehicle.drive[i].driveActive) {    //MoTeC is saying the drive is not active, this is most important
                set_led(drive_led_map[i], LED_RED);
            } else if (the_vehicle.drive[i].derating) {
                set_led(drive_led_map[i], LED_BLUE);
            } else if (the_vehicle.drive[i].inverterTemp > 50 || the_vehicle.drive[i].motorTemp > 60 ||
                       the_vehicle.drive[i].gearboxTemp > 40) {        //Something is a little warm
                set_led(drive_led_map[i], LED_YELLOW);
            } else {
                set_led(drive_led_map[i], LED_GREEN);
            }
            break;
        case CAN_ID_VGPIO_MOTEC:
            if ((RxData[0] & 0b00000001) && (!CYCLE_SCREEN_FLAG)) {
                cycle_screens();
                CYCLE_SCREEN_FLAG = true;
            } else if (!(RxData[0] & 0b00000001)) {
                CYCLE_SCREEN_FLAG = false;
            }
            if ((RxData[0] & 0b00000010) && (!TOGGLE_POPUP_FLAG)) {
                enable_popups();
                TOGGLE_POPUP_FLAG = true;
            } else if (!(RxData[0] & 0b00000010)) {
                if (TOGGLE_POPUP_FLAG) disable_popups();
                TOGGLE_POPUP_FLAG = false;
            }
            // Make sure to read dial here
            the_vehicle.wheel.leftDial = (RxData[1] & 0b11110000) >> 4;
            the_vehicle.wheel.rightDial = (RxData[1] & 0b00001111);
            the_vehicle.wheel.leftPaddle = RxData[2];
            the_vehicle.wheel.rightPaddle = RxData[3];
            break;
    }
}

/* USER CODE END 1 */
