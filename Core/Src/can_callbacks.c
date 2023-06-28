//
// Created by tazukiswift on 28/06/23.
//

#include "main.h"


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
    {
        Error_Handler();
    }

    /* Here, you can use the RxHeader and RxData variables */
}
