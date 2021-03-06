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

#ifndef DS1624_Arduino_Library
#define DS1624_Arduino_Library

#include <Wire.h>

class DS1624
{
private:
  // Stores the actual address
  uint8_t _address;

  // Flag set when temperature measurement is initialized
  bool _initialized;

  // Flag to detect, when Wire library is initialized
  bool _wireInitialized;

  // Flag that stores if last temperature value was valid
  bool _temperatureValueValid;

  // First init of present class
  void Init();

  // Receive from sensor the last converted temperature value
  float ReadConvertedValue();

public:
  // Default constructor. Use address pins a2,a1,a0 connected to ground.
  DS1624();

  // Class constructor, accepting a 3-bit code a2|a1|a0 corresponding to
  // physical address pin connections
  DS1624(uint8_t addressByPins);

  // Constructor, that enables Wire's library initialization
  // by application itself, not by DS1624 library
  DS1624(uint8_t addressByPins, bool customWire);

  // Gets temperature in centigrade degrees
  void GetTemperature(float & readValue, bool & isValid);

  // Write one byte to EEPROM
  bool writeByte(unsigned char addr, unsigned char value);

  // Read one byte from EEPROM
  unsigned char readByte(unsigned char addr, bool & isValid);

  // Read some bytes from EEPROM
  unsigned char readBlock(unsigned char addr, unsigned char len, unsigned char*data);

  // Write some bytes to EEPROM
  unsigned char writeBlock(unsigned char addr, unsigned char len, unsigned char*data);

  // print memory content in human-readable format
  // if startAddress<0 prints all memory content
  void printMem(Stream & _serial,int startAddress, unsigned char len);

  // print memory content in hexadecimal format, splitting into 16-bytes chunks
  // if startAddress<0 prints all memory content
  void dumpMem(Stream & _serial,int startAddress, unsigned char len);
};

#endif //DS1624_Arduino_Library
