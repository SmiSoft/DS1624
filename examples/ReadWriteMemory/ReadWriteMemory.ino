/*****************************************************************************
Arduino Library for Maxim DS1624 temperature sensor and memory

MIT License

Copyright (c) 2019 Python <smisoft@rambler.ru>

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

// Supported commands:

// Empty command - author's information

// read address length
// reads length (decimal) bytes starting with address (decimal). limited to 256 bytes. Prints data to Serial in human-readable format.
// Example:
// read 0 4
// Response:
// data

// dump address length
// dumps length (decimal) bytes starting with address (decimal), and prints it to Serial using hexadecimal presentation.
// Example:
// dump 5 4
// Response:
// FF 02 FF 01

// write address data-hex
// writes data, presented in hexadecimal format to address (decimal). Has no answer. Command length is limited to 80 chars (up to 20 bytes at once)
// Example:
// write 100 01 FA FB 02
// WARNING! Data overflow is not tested!

#include <DS1624.h>
// https://github.com/SmiSoft/SerialConsole
#include <SerialConsole.hpp>

// Sensor's all address pins connected to Vcc
DS1624 ds1624(0B111);

// create Console object for default serial port
SerialConsole serialConsole(Serial, 80, "#>");

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000); // only my own case - long wires
}

// convert single hexadecimal character into integer value
unsigned char htoi(unsigned char h)
{
  if((h >= '0') && (h <= '9'))
    return h - '0';
  if((h >= 'A') && (h <= 'F'))
    return h - 'A' + 10;
  if((h >= 'a') && (h <= 'f'))
    return h - 'a' + 10;
  return 0;
}

void loop()
{
  if(const char*command = serialConsole.avail())
  {
    byte address, len;
    if(command[0] == 0)
    {
      Serial.println("DS1624 demonstration program by Python <smisoft@rambler.ru>");
    }
    else if(sscanf(command, "read %d %d",&address, &len) == 2)
    {
      bool valid;
      while(len-- > 0){
        unsigned char data = ds1624.readByte(address++, valid);
        Serial.write(data);
      }
      Serial.println();
    }
    else if (sscanf(command, "dump %d %d", &address, &len) == 2)
    {
      bool valid;
      byte cnt = 0;
      while(len-- > 0){
        if(++cnt > 16)
        {
          Serial.println();
          cnt = 1;
        }
        unsigned char data = ds1624.readByte(address++, valid);
        if(data < 16)
        {
          Serial.print('0');
        }
        Serial.print(data, HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
    else if(strncmp(command, "write ", 6) == 0)
    {
      const char*temp = command + 6;
      address = 0;
      byte mode = 0; // 0 - seek address, 1 - in address
      for( ; *temp != 0; ++temp)
      {
        if(isspace(*temp))
        {
          if(mode != 0)
          {
            break;
          }
        }
        else if(isdigit(*temp))
        {
          mode = 1;
          address = 10 * address + htoi(*temp);
        }
        else
        {
          break;
        }
      }
      if(mode!=1)
      {
        Serial.println("Invalid command format");
      }
      else
      {
        mode = 0; // 0 - seek data, 1 - read hex
        unsigned char data = 0;
        for( ; *temp != 0; ++temp)
        {
          if(isspace(*temp))
          {
            if(mode == 1)
            {
              mode = 0;
              ds1624.writeByte(address++, data);
            }
          }
          else if(isxdigit(*temp))
          {
            if(mode == 0)
            {
              data = htoi(*temp);
            }
            else
            {
              data = 16*data+htoi(*temp);
            }
            mode = 1;
          }
          else
          {
            mode=255;
            break;
          }
        }
        if(mode == 255)
        {
          Serial.println("Invalid command format");
        }
        else if(mode == 1)
        {
          ds1624.writeByte(address, data);
        }
      }
    }
    else
    {
      Serial.println("Invalid command");
    }
    serialConsole.done();
  }
  MCUCR=(MCUCR & 0B00001111) | 0B10000000; // idle
  asm("sleep");
}

// translate received character to comConsole for processing
void serialEvent()
{
  serialConsole.nextChar();
}

