#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


void OTA_setup() 
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("F:OTA_setup()");

  DEBUG_OUTPUT.println("Arduino OTA setup..");

  DEBUG_OUTPUT.printf("Sketch size: %ukB\n", ESP.getSketchSize() / 1000);
  DEBUG_OUTPUT.printf("Free size: %ukB\n", ESP.getFreeSketchSpace() / 1000);

  ArduinoOTA.onStart([]() {
    DEBUG_OUTPUT.println("Start");
    resetObjectAskingForResponse(); //turn off waiting for responses - helps upload new firmware smoothly
    showServiceMessage("Update_Start");
    sendNodeState("Update_Start");  
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_OUTPUT.println("\nEnd");
    showServiceMessage("Update_End");
    sendNodeState("Update_End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_OUTPUT.printf("Progress: %u%%\r", (progress / (total / 100)));
    //showServiceMessage(progress / (total / 100));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_OUTPUT.printf("Error[%u]: ", error);
    //sendNodeState(error);
    
    if (error == OTA_AUTH_ERROR) DEBUG_OUTPUT.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_OUTPUT.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_OUTPUT.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_OUTPUT.println("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_OUTPUT.println("End Failed");
  });
  ArduinoOTA.begin();
  DEBUG_OUTPUT.println("OTA Ready..");
  DEBUG_OUTPUT.print("IP address: ");
  DEBUG_OUTPUT.println(WiFi.localIP());
  
}


void OTA_loop(int) 
{
  ArduinoOTA.handle();
}

