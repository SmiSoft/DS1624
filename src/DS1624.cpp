/*****************************************************************************
Arduino Library for Maxim DS1624 temperature sensor and memory

MIT License

Copyright (c) 2017 Alessio Leoncini

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include "DS1624.h"
#include <Arduino.h>
#include <Wire.h>

DS1624::DS1624()
{ 
  // a2 <- ground; a1 <- ground; a0 <- ground; continuous conversion = true;
  DS1624(false, false, false, true);
}

void DS1624::DS1624(bool a2, bool a1, bool a0, bool continuousConversion)
{
  // Start I2C communication on default SCK, SDA ports for I2C master
  Wire.begin();
  
  // Base address least significant bits will be a2, a1, a0 respectively 
  _address = BASE_ADDRESS | (a2 << 2) | (a1 << 1) | a0;
  
  // Save conversion mode
  _continuousConversion = continuousConversion;
  
  //  Configure sensor
  Wire.beginTransmission(_address);
  Wire.write(ACCESS_CONFIG);
  Wire.write(0x00 | _continuousConversion);
  Wire.endTransmission();
    
  // Minimum time needed to store the configuration is 10ms
  // Here wait for 20ms
  delay(20);
  
  if(_continuousConversion)
  {
    StartConversion();
  }
}

void DS1624::StartConversion()
{
  Wire.beginTransmission(_address);
  Wire.write(START_CONVERSION);
  Wire.endTransmission();
}

void DS1624::StopConversion()
{
  Wire.beginTransmission(_address);
  Wire.write(STOP_CONVERSION);
  Wire.endTransmission();
}

float DS1624::GetTemperature()
{
  uint16_t temperature = 0;
  
  if(!_continuousConversion)
  {
    // Start conversion
    StartConversion();
    
    // Wait for conversion done
    uint8_t configRegister = 0x00;
    do
    {
      // Read configuration register. The most significant bit turns to 1
      // when conversion done
      Wire.beginTransmission(_address);
      Wire.write(ACCESS_CONFIG);
      Wire.requestFrom(_address, 1, true);
      
      // Wait for data sent from sensor
      while(!Wire.available());
      
      // Check DONE bit
      configRegister = Wire.read();
    }while(!(configRegister & 0x80));
  }
  
  // Request to read last converted temperature value
  Wire.beginTransmission(_address);
  Wire.write(READ_TEMPERATURE);
  Wire.requestFrom(_address, 2, true);

  // Wait for data sent from sensor
  while(!Wire.available());
  
  // Read most significant word
  temperature = Wire.read() << 8;
  
  // Wait for data sent from sensor
  while(!Wire.available());
  
  // Read least significant word
  temperature |= Wire.read();

  // Convert to float and handle negative numbers
  temperature >>= 4;
  return (float)((temperature & 0x800 ? (temperature & 0x7ff) - 0x800 : temperature) / 16.0);
}
