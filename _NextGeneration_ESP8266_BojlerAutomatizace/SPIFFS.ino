//SPIFFS.ino


#include "FS.h"

uint8_t cislo = 0;

void SPIFFS_setup()
{
 SPIFFS.begin();
 Serial.begin(115200);
 Serial.println("");
 Serial.println("");
 Serial.println("");
 Serial.println("Serial begin");
 
 File f = SPIFFS.open("/test.txt", "w");


}

void printDebugFile_loop(int)
{
  
debugFile.close();

Serial.println("Opening File");
  debugFile = SPIFFS.open("/test.txt", "r");
  Serial.println(debugFile);

  
debugFile.setTimeout(4000);

  while(debugFile.available())
    Serial.println(debugFile.readStringUntil('\n'));
  
  debugFile.close();

  debugFile = SPIFFS.open("/test.txt", "a");


  
}
