#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
//#include "StringStream.h" //https://gist.github.com/cmaglie/5883185


//#define Serial remoteConsole //forwards all Serial outputs to remote console object


const uint16_t aport = 8266;
const char* host = "esp8266"; //esp8266.local in browser

WiFiServer TelnetServer(aport);
WiFiClient Telnet;
WiFiUDP OTA;


void OTA_setup() {
  if(MAIN_DEBUG) Serial.println("F:OTA_setup()");

  Serial.println("Arduino OTA setup..");

  Serial.printf("Sketch size: %ukB\n", ESP.getSketchSize() / 1000);
  Serial.printf("Free size: %ukB\n", ESP.getFreeSketchSpace() / 1000);

  //MDNS.begin(host);
  //MDNS.addService("arduino", "tcp", aport);

  OTA.begin(aport);
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
  Serial.print("Waiting for OTA upload at IP address: ");

  Serial.println(WiFi.localIP());
}


void OTA_loop(int) 
{
  //OTA Sketch
  uint8_t otaLoopsCount = 70;
  while(otaLoopsCount--)
  {
    if(OTA.parsePacket()) {
      resetObjectAskingForResponse(); //turn off waiting for responses - helps upload new firmware smoothly

      showServiceMessage("U_1");
      sendNodeState("Update_Start");
      IPAddress remote = OTA.remoteIP();
      int cmd  = OTA.parseInt();
      int port = OTA.parseInt();
      int size   = OTA.parseInt();

      Serial.print("---> Update Start: ip:");
      Serial.print(remote);
      Serial.printf(", port:%d, size:%d\n", port, size);
      uint32_t startTime = millis();
      showServiceMessage("U_2");

      WiFiUDP::stopAll();

      showServiceMessage("U_3");
      if(!Update.begin(size)) {
        showServiceMessage("U_E_1");
        Serial.println("Update Begin Error: Size of sketch");      
        sendNodeState("UpdateError_Size");
        ESP.restart();
        //      return;
      }

      showServiceMessage("U_4");

      WiFiClient client;
      showServiceMessage("U_5");
      if(client.connect(remote, port))
      {
        showServiceMessage("U_6");
        uint32_t written;
        long data = 0;
        while(!Update.isFinished())
        {
          if(data % 100000 == 0)
            Serial.print(".");

          if(data++ > 1000000)
          {
            sendNodeState("Update_Failed3");
            ESP.restart();
          }

          written = Update.write(client);

          //  sprintf(buffer,"Udata2: %d", data);
          //showServiceMessage(buffer);
          if(written > 0)
            client.print(written, DEC);
        }
        Serial.setDebugOutput(false);
        char buffer[15];
        sprintf(buffer, "Udata: %u", data);
        sendNodeState(buffer);


        showServiceMessage("U_7");

        if(Update.end()) {
          client.println("OK");
          Serial.printf("Update Success in: %useconds\nRebooting...\n", (millis() - startTime) / 1000);
          sendNodeState("Update_Success");
          ESP.restart();
        } else {
          showServiceMessage("U_E_2");
          Update.printError(client);
          //Update.printError(Serial);        
          //ESP.restart();
          sendNodeState("Update_Failed1");
          ESP.restart();
        }
      } else {
        showServiceMessage("U_E_3");
        Serial.printf("Connect Failed after: %useconds\n", (millis() - startTime) / 1000);
        sendNodeState("Update_Failed_NodeCouldNotConnect");
        ESP.restart();
      }
    }
    delay(1);
  }
}

