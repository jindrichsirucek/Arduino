/*
  RemoteConsole.h - Lightweight printable string class
  Copyright (c) 2009-2012 Mikal Hart.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef RemoteConsole_h
#define RemoteConsole_h

#include "Print.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <ESP8266WiFi.h>

#include <HardwareSerial.h> //for posibility to print data also to serial output
#include <ESP_SSD1306.h>    //for posibility to print data also to display output
#include <FS.h>             //for posibility to print data also into file output


class RemoteConsole : public Print
{
private:
  char *_buf, *_cur;
  size_t _size;
  unsigned int _position;
  size_t _sendingBufferSize;

  char const* _serverUrl;
  char const* _scriptUrl;
  
  
  // Take a pointer to Serial and display instance
  HardwareSerial* _SerialOutput;
  ESP_SSD1306* _DisplayOutput;

  //File output variables
  File _FileOutput;
  String _fileName = "";

  uint8_t _startRowDisplay, _startCellDisplay, _bufferSizeDisplay, _currentCursorPositionDisplay;

public:
	virtual size_t write(uint8_t);
  
public:

  // Basic constructor requires size of a preallocated buffer
  RemoteConsole(size_t bufferSize) 
  {
    _sendingBufferSize = bufferSize;  
    char _sendingBuffer[bufferSize]; 
    _buf = _sendingBuffer;
    _resetBuffer(); 
  }


  void assignWifiOutput(char const*, char const*);
  int sendDataOverWiFi(const char*);

  //for posibility to print data also to other outputs
  void assignSerialOutput(HardwareSerial*);
  void printToSerialOutput(const char*);

  void assignDisplayOutput(ESP_SSD1306*, uint8_t, uint8_t, uint8_t);
  void printToDisplayOutput(const char*);
  void clearDisplay();

  void assignFileOutput(File*);
  void printToFileOutput(const char*);
  String getFileOutput();
  void clearFileOutput();

  // returns the length of the current string, not counting the 0 terminator
  inline const size_t length() 
  { return _cur - _buf; }

  // returns the capacity of the string
  inline const size_t capacity() 
  { return _size; }

  // gives access to the internal string
  inline operator const char *() 
  { return _buf; }

  

  // Safe access to sprintf-like formatting, e.g. str.format("Hi, my name is %s and I'm %d years old", name, age);
  int printf(char *str, ...);
  int sendDataOverWiFi(const char* text);
  

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////Object Serial - for find/replace compatibility code
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//Do not use this methods for anything!

  inline void setDEBUGOutput(bool val){};
  inline void begin(int val){};
  inline size_t available(){};
  inline void readBytes(uint8_t *, size_t){};

//////////////////////////////////////////////////////////////////

  private:

  void _resetBuffer(); 
  String _URLEncode(const char* msg);
  String _getUptime();

};

#endif