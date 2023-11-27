//
// Created by tazukiswift on 29/06/23.
//

#ifndef DASHBOARD_CAN_CALLBACKS_H
#define DASHBOARD_CAN_CALLBACKS_H

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
#define CAN_ID_LAP_DATA                    0x605
#define CAN_ID_LAP_DATA_2                  0x606
#define CAN_ID_DRIVER_DATA                 0x607
#define CAN_ID_DRIVER_DATA_2               0x608

#define CAN_ID_VGPIO_MOTEC                 0x620    // Needs 2, 3, 4 0b
#define CAN_ID_VGPIO_DASHBOARD             0x621

#define CAN_ID_REAR_LEFT_DRIVE             0x610
#define CAN_ID_FRONT_LEFT_DRIVE            0x611
#define CAN_ID_FRONT_RIGHT_DRIVE           0x612
#define CAN_ID_REAR_RIGHT_DRIVE            0x613

#define CAN_ID_COOLANT_TEMPS               0x614    // Implement!


#endif //DASHBOARD_CAN_CALLBACKS_H
