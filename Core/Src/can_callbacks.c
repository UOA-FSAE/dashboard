//
// Created by tazukiswift on 28/06/23.
//

#include "main.h"
#include "can_callbacks.h"
#include "vehicle.h"

extern Vehicle_Data the_vehicle;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // Probably need a more robust check :skull:
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
        Error_Handler();
    }
    switch (RxHeader.StdId) {
        case CAN_ID_ACCUMULATOR_PACK_DATA:
        	the_vehicle.ts.current                 = (int16_t)((RxData[0]<<8 & 0xFF00) + RxData[1]);
        	the_vehicle.ts.current                 = ((float)the_vehicle.ts.current) / 10.0f;

        	the_vehicle.ts.voltage                 = (RxData[2]<<8 & 0xFF00) + RxData[3];
        	the_vehicle.ts.voltage                 = ((float)the_vehicle.ts.current) / 10.0f;

        	the_vehicle.ts.soc                     = RxData[4] / 2;

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
        	the_vehicle.drive[0].gearboxTemp     = RxData[0];
			the_vehicle.drive[0].motorTemp       = RxData[1];
			the_vehicle.drive[0].inverterTemp    = RxData[2];
			the_vehicle.drive[0].derating        = (RxData[5] & 0x01) >> 0;
			the_vehicle.drive[0].driveActive     = (RxData[5] & 0x02) >> 1;
			the_vehicle.drive[0].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            break;

        case CAN_ID_FRONT_LEFT_DRIVE:
        	the_vehicle.drive[1].gearboxTemp     = RxData[0];
			the_vehicle.drive[1].motorTemp       = RxData[1];
			the_vehicle.drive[1].inverterTemp    = RxData[2];
			the_vehicle.drive[1].derating        = (RxData[5] & 0x01) >> 0;
			the_vehicle.drive[1].driveActive     = (RxData[5] & 0x02) >> 1;
			the_vehicle.drive[1].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
			break;

        case CAN_ID_FRONT_RIGHT_DRIVE:
			the_vehicle.drive[2].gearboxTemp     = RxData[0];
			the_vehicle.drive[2].motorTemp       = RxData[1];
			the_vehicle.drive[2].inverterTemp    = RxData[2];
			the_vehicle.drive[2].derating        = (RxData[5] & 0x01) >> 0;
			the_vehicle.drive[2].driveActive     = (RxData[5] & 0x02) >> 1;
			the_vehicle.drive[2].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            break;

        case CAN_ID_REAR_RIGHT_DRIVE:
        	the_vehicle.drive[3].gearboxTemp     = RxData[0];
			the_vehicle.drive[3].motorTemp       = RxData[1];
			the_vehicle.drive[3].inverterTemp    = RxData[2];
			the_vehicle.drive[3].derating        = (RxData[5] & 0x01) >> 0;
			the_vehicle.drive[3].driveActive     = (RxData[5] & 0x02) >> 1;
			the_vehicle.drive[3].errorCode       = (RxData[7]<<8 & 0xFF00) | RxData[6];
            break;
    }
}
