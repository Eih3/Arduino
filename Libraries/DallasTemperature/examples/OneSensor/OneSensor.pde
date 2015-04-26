// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This is an example of how to use the DallasTemperature with a single
// temperature sensor. For more information see the README.

#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
static const int ONE_WIRE_PIN = 2;

NewOneWire oneWire(ONE_WIRE_PIN);
DallasTemperature tempSensor(oneWire);

void setup(void) {
    // initialize inputs/outputs

    // start serial port
    Serial.begin(9600);

    tempSensor.begin();
    Serial.println("Dallas Temperature IC Control Library");
}

void printId(DallasTemperature &sensor) {
    for (uint8_t i = 0; i < 8; i++) {
        Serial.print(sensor.getSlaveAddr()[i], HEX);
    }
}

void display(DallasTemperature &sensor) {
    // Ask the library whether the device is valid
    Serial.print("sensor ");
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

    // getTemperature returns a float. 
    float temperature = sensor.getTemperature();     
    Serial.print(temperature);
    Serial.print("C\t");
    Serial.print(DallasTemperature::toFahrenheit(temperature)); // Use the inbuild Celcius to Fahrenheit conversion. Returns a float
    Serial.print("F\t");
}

void loop(void) {
    display(tempSensor);
    Serial.println();
}
