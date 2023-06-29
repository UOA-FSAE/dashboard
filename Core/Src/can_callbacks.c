//
// Created by tazukiswift on 28/06/23.
//

#include "main.h"
#include "can_callbacks.h"

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // Probably need a more robust check :skull:
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
        Error_Handler();
    }
    switch (RxHeader.StdId) {
        case CAN_ID_ACCUMULATOR_PACK_DATA:

            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_2:

            break;
        case CAN_ID_ACCUMULATOR_PACK_DATA_3:

            break;

        case CAN_ID_ACCUMULATOR_VOLTAGES:

            break;

        case CAN_ID_ACCUMULATOR_TEMPERATURES:

            break;
        case CAN_ID_GLV:

            break;
        case CAN_ID_FAULT:

            break;
        case CAN_ID_RACE_DATA:

            break;
        case CAN_ID_DRIVER_DATA:

            break;

        case CAN_ID_REAR_LEFT_DRIVE:

            break;

        case CAN_ID_FRONT_LEFT_DRIVE:

            break;

        case CAN_ID_FRONT_RIGHT_DRIVE:

            break;

        case CAN_ID_REAR_RIGHT_DRIVE:

            break;
    }
}
