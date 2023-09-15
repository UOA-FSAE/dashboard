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

extern volatile Vehicle_Data the_vehicle;
volatile bool RTDS_FLAG = false;
volatile bool CYCLE_SCREEN_FLAG = false;

/* USER CODE END 0 */

CAN_HandleTypeDef hcan2;

/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 9;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

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


      if (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) != HAL_OK) {
          Error_Handler();
      }


      if (HAL_CAN_Start(&hcan2) != HAL_OK) {
          Error_Handler();
      }


      if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
          Error_Handler();
      }

  /* USER CODE END CAN2_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_6);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
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

      HAL_CAN_AddTxMessage(&hcan2, &TxHeader, buffer, &TxMailbox);
//	  if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, buffer, &TxMailbox) != HAL_OK) {
//		  Error_Handler();
//	  }
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // Probably need a more robust check :skull:
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
        Error_Handler();
    }
    int i = 3;
    switch (RxHeader.StdId) {
        case CAN_ID_ACCUMULATOR_PACK_DATA:
        	the_vehicle.ts.current                 = (int16_t)((RxData[0]<<8 & 0xFF00) + RxData[1]);
        	the_vehicle.ts.current                 = ((float)the_vehicle.ts.current) / 10.0f;

        	the_vehicle.ts.voltage                 = (RxData[2]<<8 & 0xFF00) + RxData[3];
        	the_vehicle.ts.voltage                 = ((float)the_vehicle.ts.current) / 10.0f;

        	the_vehicle.ts.soc                     = RxData[4] / 2;

        	set_led(ERROR1,LED_BLACK);
        	if (the_vehicle.ts.soc) {
            	set_led(ACCUMULATOR,LED_GREEN);
        	} else {
            	set_led(ACCUMULATOR,LED_RED);
        	}
        	set_led(INVERTER,LED_BLACK);


            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_2:

            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_3:

            break;

        case CAN_ID_ACCUMULATOR_VOLTAGES:
        	the_vehicle.ts.maxVoltage              = (RxData[1]<<8 & 0xFF00) + RxData[0];
			the_vehicle.ts.maxVoltage              = ((float)the_vehicle.ts.maxVoltage) / 10000.0f;

			the_vehicle.ts.avgVoltage              = (RxData[3]<<8 & 0xFF00) + RxData[2];
			the_vehicle.ts.avgVoltage              = ((float)the_vehicle.ts.avgVoltage) / 10000.0f;

			the_vehicle.ts.minVoltage              = (RxData[5]<<8 & 0xFF00) + RxData[4];
			the_vehicle.ts.minVoltage              = ((float)the_vehicle.ts.minVoltage) / 10000.0f;

			the_vehicle.ts.numCells                = RxData[6];
            break;

        case CAN_ID_ACCUMULATOR_TEMPERATURES:
        	the_vehicle.ts.maxTemp                 = (RxData[1]<<8 & 0xFF00) + RxData[0];
			the_vehicle.ts.maxTemp                 = ((float)the_vehicle.ts.maxTemp) / 10000.0f;

			the_vehicle.ts.avgTemp                 = (RxData[3]<<8 & 0xFF00) + RxData[2];
			the_vehicle.ts.avgTemp                 = ((float)the_vehicle.ts.avgTemp) / 10000.0f;

			the_vehicle.ts.minTemp                 = (RxData[5]<<8 & 0xFF00) + RxData[4];
			the_vehicle.ts.minTemp                 = ((float)the_vehicle.ts.minTemp) / 10000.0f;
            break;
        case CAN_ID_GLV:
        	the_vehicle.glv.soc                  = RxData[0];
			the_vehicle.glv.voltage              = (RxData[2]<<8 & 0xFF00) | (RxData[1] & 0x00FF);
			the_vehicle.glv.voltage              = ((float)the_vehicle.glv.voltage) / 1000.0f;
			the_vehicle.glv.current              = (RxData[4]<<8 & 0xFF00) | (RxData[3] & 0x00FF);
			the_vehicle.glv.current              = ((float)the_vehicle.glv.current) / 1000.0f;
            break;
        case CAN_ID_FAULT:
        	the_vehicle.errors.PDOC              = (RxData[0] & 0x01) >> 0;
			the_vehicle.errors.AMS               = (RxData[0] & 0x02) >> 1;
			the_vehicle.errors.IMD               = (RxData[0] & 0x04) >> 2;
			the_vehicle.errors.BSPD              = (RxData[0] & 0x08) >> 3;

			the_vehicle.errors.throttlePlausability      =  (RxData[1] & 0x01) >> 0;
			the_vehicle.errors.brakeThrottleImplausability =  (RxData[1] & 0x02) >> 1;

			if(the_vehicle.errors.throttlePlausability) {
				set_led(CTRL1,LED_RED);
			} else {
				set_led(CTRL1,LED_BLACK);
			}

			if(the_vehicle.errors.brakeThrottleImplausability) {
				set_led(CTRL2,LED_RED);
			} else {
				set_led(CTRL2,LED_BLACK);
			}

			//Set fault LEDs, these are easy: RED or OFF, and nothing fancy
			if(the_vehicle.errors.IMD) {
				set_led(IMD,LED_RED);
			} else {
				set_led(IMD,LED_BLACK);
			}

			if(the_vehicle.errors.AMS) {
				set_led(AMS,LED_RED);
			}else{
				set_led(AMS,LED_BLACK);
			}

			if(the_vehicle.errors.PDOC) {
				set_led(PDOC,LED_RED);
			} else {
				set_led(PDOC,LED_BLACK);
			}

			if(the_vehicle.errors.BSPD) {
				set_led(BSPD,LED_RED);
			} else {
				set_led(BSPD,LED_BLACK);
			}

			set_led(ERROR2,LED_BLACK);
            break;

        case CAN_ID_RACE_DATA:
        	the_vehicle.race.currentLapTime      = (RxData[3]<<24 & 0xFF000000) + (RxData[2]<<16 & 0x00FF0000) + (RxData[1]<<8 & 0X0000FF00) + (RxData[0] & 0xFF) ;
			the_vehicle.race.currentSpeed        = RxData[4];
			the_vehicle.race.lapNumber           = RxData[5];

			//Update the best lap time if it is actually better, and update the deltaLapTime
			the_vehicle.race.bestLapTime         = (the_vehicle.race.bestLapTime <=the_vehicle.race.currentLapTime) ?the_vehicle.race.currentLapTime :the_vehicle.race.bestLapTime;
			the_vehicle.race.deltaLapTime        = (the_vehicle.race.currentLapTime -the_vehicle.race.bestLapTime);

			the_vehicle.race.vehicleState.GLV            = (RxData[7] & 0x01) >> 0;
			the_vehicle.race.vehicleState.shutdown       = (RxData[7] & 0x02) >> 1;
			the_vehicle.race.vehicleState.precharging    = (RxData[7] & 0x04) >> 2;
			the_vehicle.race.vehicleState.precharged     = (RxData[7] & 0x08) >> 3;
			the_vehicle.race.vehicleState.RTDState       = (RxData[7] & 0x10) >> 4;

			if(the_vehicle.race.vehicleState.precharged) {
				set_led(TS_ACT,LED_GREEN);
			} else if(the_vehicle.race.vehicleState.precharging) {
				set_led(TS_ACT,LED_BLUE);
			} else {
				set_led(TS_ACT,LED_BLACK);
			}

			if (the_vehicle.race.vehicleState.GLV) {
				set_led(GLV,LED_GREEN);
			} else {
				set_led(GLV,LED_RED);
			}
			if (the_vehicle.race.vehicleState.shutdown) {
				set_led(SHUTDOWN,LED_GREEN);
			} else {
				set_led(SHUTDOWN,LED_RED);
			}	// TODO: add shutdown status
			if(the_vehicle.race.vehicleState.RTDState) {
                if (!RTDS_FLAG) {
                    RTDS_FLAG = true;
//                    HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_SET);
                    HAL_TIM_Base_Start_IT(&htim13);
                }
				set_led(READY,LED_GREEN);
			} else {
                RTDS_FLAG = false;
                set_led(READY,LED_BLACK);
			}



            break;
        case CAN_ID_DRIVER_DATA:
        	the_vehicle.driver.steeringAngle       = RxData[0];
			the_vehicle.driver.brakeBias           = RxData[1];
			the_vehicle.driver.frontBrakePressure  = RxData[2];
			the_vehicle.driver.rearBrakePressure   = RxData[3];
			the_vehicle.driver.throttle            = RxData[4];

			//Override displayed brake bias if it's not even sent
			if (the_vehicle.driver.brakeBias == 0) {
				the_vehicle.driver.brakeBias = ((the_vehicle.driver.frontBrakePressure * 100.0) /the_vehicle.driver.rearBrakePressure);
			}
            break;
        case CAN_ID_REAR_LEFT_DRIVE:
        	i--;
        case CAN_ID_FRONT_LEFT_DRIVE:
        	i--;
        case CAN_ID_FRONT_RIGHT_DRIVE:
        	i--;
        case CAN_ID_REAR_RIGHT_DRIVE:
        	the_vehicle.drive[i].gearboxTemp     = RxData[0];
			the_vehicle.drive[i].motorTemp       = RxData[1];
			the_vehicle.drive[i].inverterTemp    = RxData[2];
			the_vehicle.drive[i].derating        = (RxData[5] & 0x01) >> 0;
			the_vehicle.drive[i].driveActive     = (RxData[5] & 0x02) >> 1;
			the_vehicle.drive[i].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];

			if(!the_vehicle.drive[i].driveActive) {	//MoTeC is saying the drive is not active, this is most important
				set_led(RL,LED_RED);
			} else if(the_vehicle.drive[i].derating) {
				set_led(RL,LED_BLUE);
			} else if(the_vehicle.drive[i].inverterTemp > 50 || the_vehicle.drive[i].motorTemp > 60 || the_vehicle.drive[i].gearboxTemp > 40) {		//Something is a little warm
				set_led(RL,LED_YELLOW);
			} else {
				set_led(RL,LED_GREEN);
			}
			break;
        case CAN_ID_VGPIO_MOTEC:
            if ((RxData[0] & 0b00000001)&&(!CYCLE_SCREEN_FLAG)) {
                cycle_screens();
                CYCLE_SCREEN_FLAG = true;
            } else if (!(RxData[0] & 0b00000001)) {
                CYCLE_SCREEN_FLAG = false;
            }
            break;
    }
}

/* USER CODE END 1 */
