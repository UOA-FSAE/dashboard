#ifndef __CANFRAMES_H_
#define __CANFRAMES_H_

#define updateCANFreq               500.0

// CAN frame id
#define CAN_ID_ACCUMULATOR_PACK_DATA       0x6B0
#define CAN_ID_ACCUMULATOR_PACK_DATA_2     0x6B1
#define CAN_ID_ACCUMULATOR_PACK_DATA_3     0x6B2
#define CAN_ID_ACCUMULATOR_VOLTAGES        0x6B3
#define CAN_ID_ACCUMULATOR_TEMPERATURES    0x6B4

#define CAN_ID_GLV                         0x602
#define CAN_ID_FAULT                       0x603
#define CAN_ID_RACE_DATA                   0x604
#define CAN_ID_DRIVER_DATA                 0x605

#define CAN_ID_REAR_LEFT_DRIVE             0x384
#define CAN_ID_FRONT_LEFT_DRIVE            0x385
#define CAN_ID_FRONT_RIGHT_DRIVE           0x386
#define CAN_ID_REAR_RIGHT_DRIVE            0x387

//CAN SETUP
#define CAN_TX_PIN GPIO_NUM_4
#define CAN_RX_PIN GPIO_NUM_5

void initCAN();

[[noreturn]] void updateCAN(void *pvParameters);

#endif