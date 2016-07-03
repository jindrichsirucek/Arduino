/*
  RemoteConsole.cpp - Lightweight printable string class
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

#include "RemoteConsole.h"
#include <ESP8266WiFi.h>
//#include <FS.h>             //for posibility to print data also into file output  


////////////////////////////////////////////////////////
//for posibility to print data also to Serial output
////////////////////////////////////////////////////////
void RemoteConsole::assignSerialOutput(HardwareSerial* serial)
{
  _SerialOutput = serial;
}

void RemoteConsole::printToSerialOutput(const char* text)
{
  _SerialOutput->print(text);
}


////////////////////////////////////////////////////////
//for posibility to print data also to display output
////////////////////////////////////////////////////////
void RemoteConsole::assignDisplayOutput(ESP_SSD1306* display, uint8_t startCell, uint8_t startRow, uint8_t bufferSize)
{
  _DisplayOutput = display;
  _startRowDisplay = startRow;
  _startCellDisplay = startCell;
  _bufferSizeDisplay = bufferSize;
  _currentCursorPositionDisplay = 0;
}

void RemoteConsole::printToDisplayOutput(const char* text)
{
  uint8_t currentRow =  ((_currentCursorPositionDisplay / 21) + _startRowDisplay) * 8 ;
  uint8_t currentCell = (_currentCursorPositionDisplay % 21)  * 6;

  _DisplayOutput->setCursor(currentCell, currentRow);
  _DisplayOutput->print(text);
  _DisplayOutput->display();

  _currentCursorPositionDisplay += 8;
  if(_currentCursorPositionDisplay >= _bufferSizeDisplay)
  {
    clearDisplay();
  }
}

void RemoteConsole::clearDisplay()
{
  _currentCursorPositionDisplay = 0;
  //clear console display area
  _DisplayOutput->setCursor(_startCellDisplay * 6, _startRowDisplay * 8);
  char clearingText[_bufferSizeDisplay];
  for (int i = 0; i < _bufferSizeDisplay; ++i)
  {
    clearingText[i] = ' ';
  }
  _DisplayOutput->print(clearingText);
  _DisplayOutput->display();
}


////////////////////////////////////////////////////////
//for posibility to print data also into file output
////////////////////////////////////////////////////////

void assignFileOutput(String fileName)
{
  _fileName = fileName;
}


String getFileOutput()
{
  _FileOutput.setTimeout(4000);
  String fileOutput = _FileOutput.readString();  
  _FileOutput.close();
  return fileOutput;
}


void clearFileOutput()
{
  _FileOutput = SPIFFS.open("/" + _fileName, "w");
  _FileOutput.close();
}


void RemoteConsole::printToFileOutput(const char* line)
{
  _FileOutput = SPIFFS.open("/" + _fileName, "a");
  _FileOutput.print(line);
  _FileOutput.close();
}

////////////////////////////////////////////////////////
//for posibility to print data also over Wifi
////////////////////////////////////////////////////////

void RemoteConsole::assignWifiOutput(char const* serverUrl, char const* scriptUrl) 
{
  _scriptUrl = ((String)scriptUrl + '?').c_str();
  _serverUrl = serverUrl;
}


int RemoteConsole::sendDataOverWiFi(const char* text)
{
  String url = _scriptUrl + _URLEncode(_getUptime().c_str())+"=" + _URLEncode(text);
  
  if(WiFi.status() != WL_CONNECTED)
    return -1;

  WiFiClient client;
  if(!client.connect(_serverUrl, 80)) {
    WiFi.begin("", "");
  }
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(_serverUrl) + "\r\n" +
               "Connection: close\r\n" +
               "Content-Length: 0\r\n" +
               "\r\n");
  delay(10);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RemoteConsole::_resetBuffer()
{
  _cur = _buf;
  if(_size > 0)
    _buf[0] = '\0';

  _position = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t RemoteConsole::write(uint8_t b)
{
  if(_cur + 1 < _buf + _size)
  {
    *_cur++ = (char)b;
    *_cur = '\0';

    _position++;

    if(_position >= _sendingBufferSize || (char)b == '\n')
    {
      if(_SerialOutput)
        printToSerialOutput(_buf);
    
      if(_DisplayOutput)
        printToDisplayOutput(_buf);

      if(_serverUrl)
        sendDataOverWiFi(_buf);

      if(_fileName)
        printToFileOutput(_buf);

      _resetBuffer();
    }

    if((char)b == '\n')
      _currentCursorPositionDisplay += 21 - (_currentCursorPositionDisplay % 21);

		return 1;
	}
	return 0;
}

int RemoteConsole::printf(char *str, ...) 
{ 
  va_list argptr;  
  va_start(argptr, str); 
  int ret = vsnprintf(_cur, _size - (_cur - _buf), str, argptr);
  if(_size)
     while(*_cur) 
        ++_cur;
  return ret;
}


String RemoteConsole::_URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while(*msg != '\0') {
    if( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

String RemoteConsole::_getUptime()
{
  long upTimeInMilliseconds = millis();

  int hoursFromLastSend = upTimeInMilliseconds / 1000 / 60 / 60;
  int minutesFromLastSend = upTimeInMilliseconds / 1000 / 60 - hoursFromLastSend * 60;
  int secondsFromLastSend = upTimeInMilliseconds / 1000 - minutesFromLastSend * 60 - hoursFromLastSend * 60 * 60;

  return (String)hoursFromLastSend + ":" + (String)minutesFromLastSend + ":" + (String)secondsFromLastSend;
}