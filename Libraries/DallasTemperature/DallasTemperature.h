#ifndef DallasTemperature_h
#define DallasTemperature_h

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include <inttypes.h>

#include "NewOneWire.h"

class DallasTemperature
{
    private:

    // One Wire Instance
    NewOneWire &pDataWire;

    // Which temp sensor on the bus this instance talks to.
    uint8_t index;

    // 64bit address of device
    uint8_t arSlaveAddr[8];

    // temperature LSB
    uint8_t tempLSB;

    // temperature MSB
    uint8_t tempMSB;

    // T(high)
    uint8_t highAlarmTemp;

    // T(low)
    uint8_t lowAlarmTemp;

    // device configuration
    uint8_t resolution;

    // parasite power on or off
    uint8_t parasite;

    // read device's scratchpad
    void readScratchPad(uint8_t);

    // write device's scratchpad
    void writeScratchPad();

    // read device's power requirements
    void readPowerSupply();

    public:

    DallasTemperature(NewOneWire &oneWire, uint8_t index = 0);

    // auto request: when TRUE, each temperature request will send a 
    // conversion request to the specific device (SLOWER).  when FALSE, 
    // you must call globalTempRequest() followed buy conversionDelay() 
    // to send a conversion command to all devices on the bus.  
    // getTemperature then just reads the scratchpad (FASTER when there
    // is more than one device on the bus)
    bool autoRequest;

    // Initalise.
    void begin();
    
    // returns devices index
    uint8_t getIndex() const { return index; }

    // is the slave valid
    int isValid(); // strangely, 0 is "GOOD"

    // reset everything. Useful if you knock the power cable etc
    void reset();
    
    // Process temperature in given format (default is C*)
    float getTemperature();

    // true if ON | false if OFF
    bool parasitePower();
    
    // delays the proper amount of time for a conversion to take place
    // based on the device's resolution
    void conversionDelay();

    // get resolution for the device
    uint8_t getResolution();

    // set device resolution
    void setResolution(uint8_t);

    // sends convert temperature to all devices on the NewOneWire connection.
    // only one device on each bus needs to do this.  ideally, it should be 
    // on your highest resolution device so if a delay is necessary, it will 
    // be long enough  
    void globalTempRequest();

    // returns temperature in degrees C
    float getTempC();

    // returns temperature in degrees F
    float getTempF();

    // Convert from celcius to farenheit if required
    static float toFahrenheit(float);

    void* operator new (unsigned int size); // Initalise memory area
    void operator delete(void* p); // delete memory reference

    // returns device's 64 bit address
    const uint8_t *getSlaveAddr() const { return arSlaveAddr; }
};
#endif
