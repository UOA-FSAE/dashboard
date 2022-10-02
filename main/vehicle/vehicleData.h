#ifndef __VEHICLEDATA_H
#define __VEHICLEDATA_H

#include <cstdint>

//TS Data
struct TS_DATA {

    uint8_t soc;			// 0 to 100%
    float voltage;			// 0.0 to 600.0V
    float current;			// -15.0A to 100.0A SIGNED

    float minVoltage;		// 0.0000 to 4.2000 V
    float avgVoltage;
    float maxVoltage;
    int numCells;

    uint8_t maxTemp;		// 0 to (hopefully)59 degC
    uint8_t avgTemp;
    uint8_t minTemp;

};

//GLV Data
struct GLV_DATA {
    uint8_t soc;			// 0 to 100%
    float voltage;			// 0.0 to 30.0 V
    float current;			// 0.0A to 15.0A SIGNED
};

struct VEHICLE_STATE {
    bool GLV;
    bool shutdown;
    bool precharging;
    bool precharged;
    bool RTDState;
};

//Driver Data
struct DRIVER_DATA {
    int8_t steeringAngle; 			// -100 to 100 degrees from center SIGNED
    uint8_t brakeBias;				// 0 to 100% forward bias
    uint8_t frontBrakePressure; 	// 0 to 255 kPa
    uint8_t rearBrakePressure;  	// 0 to 255 kPa
    uint8_t throttle;				// 0 to 100%
    uint8_t torque;					// 0 to 210%
    uint16_t rpm;					// 0 to 65,535 rpms
};

//Motor/Inverter/Drive System Data type
struct DRIVE_DATA {
    uint8_t gearboxTemp;			// 0 to 100 degC
    uint8_t motorTemp;				// 0 to 100 degC
    uint8_t inverterTemp;			// 0 to 100 degC
    bool derating;					// Drive is derating for some reason
    bool driveActive;				// Drive is acrive
    uint16_t errorCode;				// AMK error code
};

//Fault and Error information, specifically including the bad ones which stop the car and are required by rules
struct FAULT_DATA {
    //These interrupt the shutdown circuit
    bool IMD;
    bool AMS;
    bool PDOC;
    bool BSPD;

    //These set motor torque to, but do not interrupt the shutdown circuit
    bool throttlePlausability;
    bool brakeThrottleImplausability;

};

//Race Data
struct RACE_DATA {
    uint32_t currentLapTime;	// 0 to 300,000 ms
    uint32_t bestLapTime;		// 0 to 300,000 ms
    int32_t deltaLapTime;		// -300,000 to 300,000 ms
    uint8_t currentSpeed;		// 0 to 255 kph
    uint8_t lapNumber;			// o to 255 laps
    struct VEHICLE_STATE vehicleState;
};

struct Vehicle_Data {
    struct GLV_DATA glv;			// We have one GLV system
    struct TS_DATA ts;				// We have one tractive system
    struct RACE_DATA race;			// We have one set of race data
    struct DRIVER_DATA driver;		// We have one driver
    struct DRIVE_DATA drive[4];		// We have 4 motors and 4 motor controllers
    struct FAULT_DATA errors;		// We have a lot of errors
};

void resetDataStructure(Vehicle_Data &input_data);

#endif