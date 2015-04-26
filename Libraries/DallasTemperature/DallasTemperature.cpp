// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "DallasTemperature.h"

extern "C" {
#include <string.h>
#include <stdlib.h>
#include "WConstants.h"
}

#define DS18S20MODEL 0x10       // Model ID
#define DS18B20MODEL 0x28       // Model ID
#define DS1822MODEL 0x22       // Model ID

#define STARTCONVO 0x44         // Tells device to take a temperature reading and put it on the scratchpad
#define COPYSCRATCH 0x48 		// Copy EEPROM
#define READSCRATCH 0xBE 		// Read EEPROM
#define WRITESCRATCH 0x4E 		// Write to EEPROM
#define RECALLSCRATCH 0xB8 		// Reload from last known
#define READPOWERSUPPLY 0xB4		// determin if device needs parasite power

// device resolution
#define TEMP_9_BIT  0x1F //  9 bit
#define TEMP_10_BIT 0x3F // 10 bit
#define TEMP_11_BIT 0x5F // 11 bit
#define TEMP_12_BIT 0x7F // 12 bit

// Slave validation
#define INVALIDCRC 1
#define INVALIDMODEL 2
#define SLAVEGOOD 0

DallasTemperature::DallasTemperature(NewOneWire &oneWire, uint8_t index) :
    pDataWire(oneWire),
    index(index)
{
}

// read length bytes of a device's scratch pad
void DallasTemperature::readScratchPad(uint8_t length)
{
   // sanity check
   if (length > 8) length = 8;

   // send the command
   pDataWire.reset();
   pDataWire.select(arSlaveAddr);
   pDataWire.write(READSCRATCH);

   // read the response   
   for (uint8_t i = 0; i <= length; i++)
   switch (i)
   {
      case 0: 
         // byte 0: temperature LSB
         tempLSB = pDataWire.read();
	 break;
      case 1:
	 // byte 1: temperature MSB
	 tempMSB = pDataWire.read();
	 break;
      case 2:
	 // byte 2: high alarm temp
         highAlarmTemp = pDataWire.read();
	 break;
      case 3:
	 // byte 3: low alarm temp
	 lowAlarmTemp = pDataWire.read();
	 break;
      case 4:
	 // byte 4:
	 // this is really the configuration register, but for 
	 // our purposes, this is how we'll think of it
	 resolution = pDataWire.read();
	 break;
      case 5:
      case 6:
      case 7:
      case 8:
	 break;
   }
   pDataWire.reset();
}

// writes scratch pad for device
void DallasTemperature::writeScratchPad(void)
{
   pDataWire.reset();
   pDataWire.select(arSlaveAddr);
   pDataWire.write(WRITESCRATCH);
   pDataWire.write(highAlarmTemp); // the high alarm temp
   pDataWire.write(lowAlarmTemp); // the low alarm temp
   pDataWire.write(resolution); // configuration which is basically the resolution
   pDataWire.reset();
   // save the newly written values to eeprom
   pDataWire.write(COPYSCRATCH, parasite);
   if (parasite) delay(10); // 10ms delay 
   pDataWire.reset();
}

// reads the device's power requirements
void DallasTemperature::readPowerSupply(void)
{
   pDataWire.reset();
   pDataWire.select(arSlaveAddr);
   pDataWire.write(READPOWERSUPPLY);
   if (pDataWire.read_bit()) parasite = 0; // pulled high so externally powered
   else parasite = 1; // pulled low so it is using parasite power
   pDataWire.reset();
}

// Start the interface with a new instance of NewOneWire
void DallasTemperature::begin()
{
    autoRequest = true;
    reset();
}

// Validate whether the current address is correct
int DallasTemperature::isValid(void)
{
    // Calculate Cycle-Redudancy-Check (ie: check the data is not invalid
    if (NewOneWire::crc8(arSlaveAddr, 7) != arSlaveAddr[7]) {
        return INVALIDCRC;
    }

    if (!(arSlaveAddr[0] ==  DS18B20MODEL
                || arSlaveAddr[0] == DS18S20MODEL
                || arSlaveAddr[0] == DS1822MODEL
        )){
        return INVALIDMODEL;
    }

    // Return all good
    return SLAVEGOOD;

}

// Fires a total reset (Useful for intermittant connections)
void DallasTemperature::reset(void)
{
   // initialize internal variables
   highAlarmTemp = 0x07D0; // +125 C
   lowAlarmTemp = 0xFC90;  // -55 C
   resolution = TEMP_9_BIT; 
   parasite = 1;

   // init array
   for(int i=0;i<8;i++) arSlaveAddr[i] = 0xFF;

   // find the device on the wire
   uint8_t depth = 0;
   pDataWire.reset_search();
   while (depth <= index && pDataWire.search(arSlaveAddr)) {
      if (isValid() == SLAVEGOOD)
         depth++;
   }
    
   // populate the local variables with the device's scratchpad data
   readPowerSupply();
   readScratchPad(4); // we need it all
}

// returns a float with the temperature in degrees C.
float DallasTemperature::getTemperature()
{
    // If we're ready to rock, begin communication channel
    if (isValid() != SLAVEGOOD) return 0;  // return a value outside our range

   // you can send the convert request to all devices on the 
   // same bus so you can read them all after the conversion 
   // delay
   if (autoRequest) 
   {
      pDataWire.reset();
      pDataWire.select(arSlaveAddr);
      pDataWire.write(STARTCONVO, parasite); // start conversion
      conversionDelay();
   }

    // The temp is the first two bytes so just request those 2
    readScratchPad(2);

    int16_t rawTemperature = (((int16_t) tempMSB) << 8) | tempLSB;

    switch (arSlaveAddr[0]) {
        case DS18B20MODEL:
        case DS1822MODEL:
           switch (resolution)
            {
               case TEMP_12_BIT:
                  return (float)rawTemperature * 0.0625;
                  break;
               case TEMP_11_BIT:
                  return (float)(rawTemperature >> 1) * 0.125;
                  break;
               case TEMP_10_BIT:
                  return (float)(rawTemperature >> 2) * 0.25;
                  break;
               case TEMP_9_BIT:
                  return (float)(rawTemperature >> 3) * 0.5;
                  break;
            }
            break;
        case DS18S20MODEL:
            return (float)rawTemperature * 0.5;
            break;
    }

}

// returns true if the device requires parasite power
bool DallasTemperature::parasitePower(void)
{
   if (parasite) return true;
   return false;
}

// delays an amount based ont he device's resolution to ensure the 
// conversion has had time to take place.
void DallasTemperature::conversionDelay(void)
{
   switch (resolution)
   {
      case TEMP_9_BIT:
         delay(94);
         break;
      case TEMP_10_BIT:
         delay(188);
         break;
      case TEMP_11_BIT:
         delay(375);
         break;
      case TEMP_12_BIT:
      default:
         delay(750);
         break;
   }
}

// returns the current resolution, 9-12
uint8_t DallasTemperature::getResolution(void)
{
   readScratchPad(4); // we need it all 
   switch (resolution)
   {
      case TEMP_9_BIT:
         return 9;
	 break;
      case TEMP_10_BIT:
	 return 10;
	 break;
      case TEMP_11_BIT:
	 return 11;
	 break;
      case TEMP_12_BIT:
	 return 12;
	 break;
   }
}

// set resolution of a device to 9, 10, 11, or 12 bits
void DallasTemperature::setResolution(uint8_t newResolution)
{
   readScratchPad(4); // we need it all
   switch (newResolution)
   {
      case 9:
	 resolution = TEMP_9_BIT;
	 break;
      case 10:
	 resolution = TEMP_10_BIT;
	 break;
      case 11:
	 resolution = TEMP_11_BIT;
	 break;
      case 12:
      default:
	 resolution = TEMP_12_BIT;
	 break;
   }
   writeScratchPad();   
}

// sends convert temperature to all devices on the NewOneWire connection.
// only one device on each bus needs to do this.  ideally, it should be 
// on your highest resolution device so if a delay is necessary, it will 
// be long enough  
void DallasTemperature::globalTempRequest()
{
    pDataWire.reset();
    pDataWire.skip();
    pDataWire.write(STARTCONVO, parasite);
}

// returns temperature in degrees C
float DallasTemperature::getTempC()
{
    return getTemperature();
}

// returns temperature in degrees F
float DallasTemperature::getTempF()
{
    return (getTemperature() * 1.8) + 32;
}

// Convert float celcius to fahrenheit
float DallasTemperature::toFahrenheit(float celcius)
{
    return (celcius*1.8)+32;
}

// MnetCS - Allocates memory for DallasTemperature. Allows us to instance a new object
void* DallasTemperature::operator new(unsigned int size) // Implicit NSS obj size
{
  void * p; // void pointer
  p = malloc(size); // Allocate memory
  memset((DallasTemperature*)p,0,size); // Initalise memory

  //!!! CANT EXPLICITLY CALL CONSTRUCTOR - workaround by using an init() methodR - workaround by using an init() method
  return (DallasTemperature*) p; // Cast blank region to NSS pointer
}

// MnetCS 2009 -  Unallocates the memory used by this instance
void DallasTemperature::operator delete(void* p)
{
  DallasTemperature* pNss =  (DallasTemperature*) p; // Cast to NSS pointer
  pNss->~DallasTemperature(); // Destruct the object

  free(p); // Free the memory
}
