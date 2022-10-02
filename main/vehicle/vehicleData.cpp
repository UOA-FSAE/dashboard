#include "vehicleData.h"

void resetDataStructure(Vehicle_Data &input_data) {

    input_data.ts.soc                     = 0;
    input_data.ts.minVoltage              = 0;
    input_data.ts.minVoltage              = 0;
    input_data.ts.maxTemp                 = 0;
    input_data.ts.current                 = 0;
    input_data.ts.current                 = 0;

    input_data.driver.steeringAngle       = 0;
    input_data.driver.brakeBias           = 0;
    input_data.driver.frontBrakePressure  = 0;
    input_data.driver.rearBrakePressure   = 0;
    input_data.driver.throttle            = 0;

    input_data.race.currentLapTime        = 0;
    input_data.race.currentSpeed          = 0;
    input_data.race.lapNumber             = 0;

    input_data.race.bestLapTime           = 0;
    input_data.race.deltaLapTime          = 0;

    input_data.race.vehicleState.GLV            = false;
    input_data.race.vehicleState.shutdown       = false;
    input_data.race.vehicleState.precharging    = false;
    input_data.race.vehicleState.precharged     = false;
    input_data.race.vehicleState.RTDState       = false;

    input_data.glv.soc                  = 0;
    input_data.glv.voltage              = 0;
    input_data.glv.voltage              = 0;
    input_data.glv.current              = 0;
    input_data.glv.current              = 0;

    for(auto & i : input_data.drive) {
        i.gearboxTemp     = 0;
        i.motorTemp       = 0;
        i.inverterTemp    = 0;
        i.derating        = false;
        i.driveActive     = false;
        i.errorCode       = 0;
    }

    //Errors default to on
    input_data.errors.PDOC              = 1;
    input_data.errors.AMS               = 1;
    input_data.errors.IMD               = 1;
    input_data.errors.BSPD              = 1;
}
