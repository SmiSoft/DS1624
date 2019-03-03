/*****************************************************************************
Arduino Library for Maxim DS1624 temperature sensor and memory

MIT License

Copyright (c) 2017 Alessio Leoncini, https://www.technologytourist.com

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

DS1624::DS1624()
{
  // a2 <- ground; a1 <- ground; a0 <- ground
  DS1624(0x00, false);
}

DS1624::DS1624(uint8_t addressByPins)
{
  // by default, Wire library should be started from DS1624 library
  DS1624(addressByPins, false);
}

DS1624::DS1624(uint8_t addressByPins, bool customWire)
{
  // Class instance needs to be initialized
  _initialized = customWire;
  _wireInitialized = customWire;

  // Default true
  _temperatureValueValid = true;

  // Base address least significant bits will be a2, a1, a0 respectively
  _address = 0x48 + (addressByPins & 0x7);
}

void DS1624::Init()
{
  // Start I2C communication on default SCK, SDA ports for I2C master
  if(!_wireInitialized)
  {
    Wire.begin();
  }

  // Configure sensor
  Wire.beginTransmission(_address);
  // Command "Access config"
  Wire.write(0xAC);
  // lsb of configuration register is ONESHOT bit. Set it to zero to enable continuous conversion
  Wire.write(0x00);
  Wire.endTransmission();

  // Minimum time needed to store the configuration is 10ms
  // So wait for 100ms
  delay(100);

  // Set initialization flag
  _initialized = true;
  _wireInitialized = true;

  // Start conversion
  Wire.beginTransmission(_address);
  Wire.write(0xEE);
  Wire.endTransmission();
}

void DS1624::GetTemperature(float & readValue, bool & isValid)
{
  // Init instance if necessary
  if(!_initialized)
  {
    Init();
  }

  readValue = ReadConvertedValue();
  isValid = _temperatureValueValid;
}

float DS1624::ReadConvertedValue()
{
  uint8_t msw = 0x00;
  uint8_t lsw = 0x00;
  _temperatureValueValid = true;

  // Request to read last converted temperature value
  Wire.beginTransmission(_address);
  Wire.write(0xAA);
  Wire.requestFrom(_address, (uint8_t)2);

  // Read most significant word
  if(Wire.available())
  {
    msw = Wire.read();
  }
  else
  {
    _temperatureValueValid = false;
  }

  // Read least significant word
  if(Wire.available())
  {
    lsw = Wire.read();
  }
  else
  {
    _temperatureValueValid = false;
  }

  // Read possible other data
  while(Wire.available()) Wire.read();

  // End transmission
  Wire.endTransmission();

  // If negative temperature, apply two's complement
  if(msw & 0x80)
  {
    msw = ~msw + 0x01;
    lsw = ~lsw + 0x01;
  }

  // Decimal part of the temperature is stored on 4 most significant bits
  // of the lsw value, so shift right 4 bits
  lsw >>= 4;

  // Convert to float and handle negative numbers
  float temperature = (float)msw;
  temperature += ((float)lsw) / 16.0;
  return temperature;
}

bool DS1624::writeByte(unsigned char addr, unsigned char value)
{
  if(!_wireInitialized)
  {
    Wire.begin();
    _wireInitialized = true;
  }
  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  Wire.write(value);
  bool result = Wire.endTransmission() == 0;
//Serial.print("write[");Serial.print(addr);Serial.print("]=");Serial.println(value,HEX);
  _delay_ms(50);
  return result;
}

unsigned char DS1624::writeBlock(unsigned char addr, unsigned char len, unsigned char*data)
{
  if(!_wireInitialized)
  {
    Wire.begin();
    _wireInitialized = true;
  }
  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  for(unsigned char w = 0; w < len; ++w)
  {
    Wire.write(data[w]);
  }
  bool result = Wire.endTransmission() == 0;

  // From datasheet:
  // During the programming cycle the DS1624 does not acknowledge
  // any further accesses to the device until the programming
  // cycle is complete (no longer than 50ms).
  _delay_ms(50);

  return result ? len : 0;

}

unsigned char DS1624::readByte(unsigned char addr, bool & isValid)
{
  if(!_wireInitialized)
  {
    Wire.begin();
    _wireInitialized = true;
  }

  // this is a workaround, request is "cached" somewhere and I need to request each byte twice
  // TODO: find the reason on such strange behaviour
  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  Wire.requestFrom(_address,(uint8_t)1,(uint8_t)1);
  while(Wire.available()) Wire.read();
  Wire.endTransmission();

  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  Wire.requestFrom(_address,(uint8_t)1,(uint8_t)1);
  isValid = Wire.available();
  unsigned char result = isValid ? Wire.read() : '?';

  // Read possible other data
  while(Wire.available()) Wire.read();

  if(Wire.endTransmission() != 0)
    isValid = false;
  return result;
}

unsigned char DS1624::readBlock(unsigned char addr, unsigned char len, unsigned char*data)
{
  if(!_wireInitialized)
  {
    Wire.begin();
    _wireInitialized = true;
  }

  // this is a workaround, request is "cached" somewhere and I need to request each byte twice
  // TODO: find the reason on such strange behaviour
  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  Wire.requestFrom(_address,(uint8_t)1,(uint8_t)1);
  while(Wire.available()) Wire.read();
  Wire.endTransmission();

  unsigned char r = 0;
  Wire.beginTransmission(_address);
  Wire.write(0x17);
  Wire.write(addr);
  Wire.requestFrom(_address,(uint8_t)len,(uint8_t)1);
  while (Wire.available())
  {
    data[r++] = Wire.read();
  }
  return Wire.endTransmission() == 0 ? r : 0;
}

void DS1624::printMem(Stream & _serial,int startAddress, unsigned char len)
{
  bool valid;
  if(startAddress < 0)
  {
    // int required! Otherwice, this loop will never stop!
    for(int addr = 0; addr <= 255; ++addr)
    {
      // TODO: try to create bulk transmission (by 8 bytes simultaneously)
      unsigned char data = readByte((unsigned char)addr, valid);
      if(!isPrintable(data))
      {
        data = '?';
      }
      _serial.write(data);
      // this delay is not required for 9600 bods, but with higher speeds, DS1624 sometimes don't answer.
      // maybe, this is a defect of my chip? Try to remove.
      delayMicroseconds(10);
    }
  } else {
    while( (len > 0) && (startAddress <= 255))
    {
      unsigned char data = readByte((unsigned char)startAddress, valid);
      if(!isPrintable(data))
      {
        data = '?';
      }
      _serial.write(data);
      --len;
      ++startAddress;
    }
  }
}

void DS1624::dumpMem(Stream & _serial,int startAddress, unsigned char len)
{
  bool valid;
  unsigned char cnt = 0;
  if(startAddress < 0)
  {
    // int required! Otherwice, this loop will never stop!
    for(int addr = 0; addr <= 255; ++addr)
    {
      // TODO: try to create bulk transmission (by 8 bytes simultaneously)
      unsigned char data = readByte((unsigned char)addr, valid);
      if(data < 16)
        _serial.print("0");
      _serial.print(data, HEX);
      if(++cnt >= 16)
      {
        _serial.println();
        cnt = 0;
      }
    }
  } else {
    while( (len > 0) && (startAddress <= 255))
    {
      unsigned char data = readByte((unsigned char)startAddress, valid);
      if(data < 16)
        _serial.print("0");
      _serial.print(data, HEX);
      if(++cnt >= 16)
      {
        _serial.println();
        cnt = 0;
      }
      --len;
      ++startAddress;
    }
  }
}

