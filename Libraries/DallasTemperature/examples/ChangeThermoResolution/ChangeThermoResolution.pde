// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This is an example of how to use the DallasTemperature with two temperature
// sensors. For more information see the README.

#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
static const int ONE_WIRE_PIN = 2;

NewOneWire oneWire(ONE_WIRE_PIN);
DallasTemperature tempSensor0(oneWire, 0);
DallasTemperature tempSensor1(oneWire, 1);

void setup(void) {
  // initialize inputs/outputs

  // start serial port
  Serial.begin(9600);

  // initialize the sensors
  tempSensor0.begin();
  tempSensor1.begin();

  Serial.println("Dallas Temperature IC Control Library");
}

void printId(DallasTemperature &sensor) {
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(sensor.getSlaveAddr()[i], HEX);
  }
}

void display(DallasTemperature &sensor) {
  // Ask the library whether the device is valid
  Serial.print("Sensor ");
  Serial.print(sensor.getIndex(), DEC);
  Serial.print(" ");
  printId(sensor);
  Serial.print(": ");
  switch(sensor.isValid())
  {
    case 1:
      Serial.println("Invalid CRC");
      sensor.reset(); // Attempts to redetect IC
      return;
    case 2:
      Serial.println("Not a valid device");
      sensor.reset(); // Attempts to redetect IC
      return;
  }

  // display the device model
  Serial.print("(Model: ");
  switch (sensor.getSlaveAddr()[0])
  {
    case 0x10:
      Serial.print("DS18S20");      // Model ID
      break;
    case 0x28:
      Serial.print("DS18B20");      // Model ID
      break;
    case 0x22:
      Serial.print("DS1822");      // Model ID
      break;
  }
  Serial.print(") ");

  // display the device resolution
  Serial.print("(");
  Serial.print(sensor.getResolution(), DEC);
  Serial.print(" bit) ");

  // display parasite power
  Serial.print("(Power: ");
  if (sensor.parasitePower()) Serial.print("ON) ");
  else Serial.print("OFF) ");

  // and finally display the temperature
  float tempC = sensor.getTempC();
  Serial.print(tempC);
  Serial.print("C\t");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
  Serial.println("F\t");

/* 
  // Alternate method.  Slower, especially in autoRequest mode, as 
  // it reads the scratchpad each time.  If you want to display both 
  // C and F, use the method above.
  Serial.print(sensor.getTempC());
  Serial.print("C\t");
  Serial.print(sensor.getTempF());
  Serial.println("F\t");
*/

}

void loop(void)
{

  Serial.println("AUTO REQUEST ON");
  Serial.println();  

  // this means the library will handle all computation delays. 
  // each temperature request is addressed to ONLY this device 
  // so there will be a conversion delay for each request. (SLOWER)
  tempSensor0.autoRequest = true;
  tempSensor1.autoRequest = true;

  // loop through the resolutions
  for (uint8_t i = 9; i <13; i++)
  {
    // set the resolution of the devices
    tempSensor0.setResolution(i); 
    tempSensor1.setResolution(i); 

    display(tempSensor0);
    display(tempSensor1);
    Serial.println();
  }

  Serial.println();
  Serial.println("++++++++++++++++++++");
  Serial.println();

  Serial.println("AUTO REQUEST OFF");
  Serial.println();

  // this means computation delays are handled outside the library.  
  // use this to send a temperature command to ALL devices on the 
  // bus and then read each device one by one.  this is faster since 
  // there is only one delay necessary and one for each device. (FASTER)
  tempSensor0.autoRequest = false;
  tempSensor1.autoRequest = false;

  // loop through the resolutions
  for (uint8_t j = 9; j <13; j++)
  {
    // set device resolution
    tempSensor0.setResolution(j); 
    tempSensor1.setResolution(j); 

    // sends convert temperature to all devices on the NewOneWire connection.
    // only one device on each bus needs to do this.  ideally, it should be 
    // on your highest resolution device so if a longer delay is necessary, 
    // it will be the long enough.  
    tempSensor0.globalTempRequest();
    tempSensor0.conversionDelay();

    display(tempSensor0);
    display(tempSensor1);
    Serial.println();

  }

  Serial.println();
  Serial.println("++++++++++++++++++++");
  Serial.println();

}
