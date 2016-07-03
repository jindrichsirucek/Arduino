/*
  Created by Jindrich SIRUCEK
  See www.jindrichsirucek.cz
  Email: jindrich.sirucek@gmail.com


*/


//--------------------------------------------------------------------------------
//Libraries included
//#include <RtcDS3231.h>
#include <ESP8266WiFi.h>
#include <Tasker.h>
#include <RemoteConsole.h>


#define DEBUG_REZIM true //if true? disable debugging output information
#define EXTERNI_JEDNOTKA true // disables some modules


//Modules enabling
#define OTA_ENABLE true
#define DISPLAY_ENABLE true
#define WATER_FLOW_ENABLE true
#define CURRENT_ENABLE true
#define WAITING_FOR_RESPONSES_MODULE_ENABLED true
#define EVERY_DAY_RESTART_ENABLE false


#define ONE_WIRE_BUS_PIN 13  // DS18B20 pin


//Home mode!
#if !DEBUG_REZIM
#define NODE_NAME "bojler"
#define MAIN_DEBUG false
#define TEMPERATURE_DEBUG false
#define DEBUG_REMOTE_CONSOLE true
#define DISPLAY_DEBUG false
#define RELAY_DEBUG false
#define CURRENT_DEBUG false
#define INTERNET_COMMUNICATION_DEBUG false
#define NOT_SENDING_DATA false
#define SHOW_ERROR_DEBUG_INFORMATION true

//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size
//#define Serial remoteConsole //forwards all Serial outputs to remote console object

#else
//Development mode!
#define NODE_NAME "development"
#define MAIN_DEBUG true
#define TEMPERATURE_DEBUG false
#define DEBUG_REMOTE_CONSOLE true
#define DISPLAY_DEBUG false
#define RELAY_DEBUG true
#define CURRENT_DEBUG false
#define INTERNET_COMMUNICATION_DEBUG true
#define NOT_SENDING_DATA false
#define SHOW_ERROR_DEBUG_INFORMATION true

#define ONE_WIRE_BUS_PIN 13  // DS18B20 pin


//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size
#endif


#if EXTERNI_JEDNOTKA
  #define NODE_NAME "Daniel Dolni Bojler 160l¨"
  #define OTA_ENABLE true
  #define DISPLAY_ENABLE false
  #define WATER_FLOW_ENABLE false
  #define CURRENT_ENABLE false
  #define WAITING_FOR_RESPONSES_MODULE_ENABLED false
  #define EVERY_DAY_RESTART_ENABLE true

#endif

////////////////////////////////////////////////////////
//BETTER UNDERSTANDING NAMES DEFINITIONS
////////////////////////////////////////////////////////
#define ON_RELAY_STATE true
#define OFF_RELAY_STATE false

////////////////////////////////////////////////////////
//PROJECT SETTINGS
////////////////////////////////////////////////////////
//WIFI AP definitions

#define AP_SSID "Siruckovi"
#define AP_PASSWORD "eAAAdB99DD64fe"

#define AP_SSID "wifi"
#define AP_PASSWORD "66aa6"

#define AP_SSID "UPC3049010"
#define AP_PASSWORD "RXYDNHRD"

#define AP_SSID "DS1"
#define AP_PASSWORD "daniel12"


//#define TASKER_MAX_TASKS 6 // define max number of tasks to save precious Arduino RAM, 10 is default value
////////////////////////////////////////////////////////
//AUTONOMY SETTINGS
////////////////////////////////////////////////////////
#define TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE 2 //in degrees of celsia
#define MIMIMAL_SENDING_TEMP_DIFFERENCE 0.07 // after the difference between two measurments cross this level, data will be uploaded. Lower values bigger acuracy, but values can jump up and down around one value - too many samples with no real value
////////////////////////////////////////////////////////
//SAFETY MAX and MIN SETTINGS
////////////////////////////////////////////////////////
#define CUTTING_OFF_BOILER_ELECTRICITY_SAFE_TEMPERATURE 80
#define MAX_ERROR_COUNT_PER_HOUR 70
////////////////////////////////////////////////////////
//GLOBAL VARIABLES
////////////////////////////////////////////////////////
float lastTemp_global = -127;
float flowTemp_global = 0;
float pipeTemp_global = -127;
unsigned long lastUpdateTime_global = 0;

bool cutOffBoilerElectricityRelayStateFlag_global = OFF_RELAY_STATE;
bool heatingBoilerRelayState_global = OFF_RELAY_STATE;

float bigiestCurrentSample_global = 0;
unsigned int countCurrentSamples_global = 0;
float lastCurrentMeasurment_global = 0;

unsigned long lastwaterFlowSensorCount_global = 0;
volatile unsigned long waterFlowSensorCount_global;  // Measures flow meter pulses
unsigned long waterFlowDisplay_global = 0;
unsigned long flowOffTime_global = 0; //time from last measured water flow


/// Restart will be triggered on this time
unsigned long espRestartTime_global = 160 * 1000; //this value need to be changed also in function itself

String systemStateInfo_global = "";
String objectAskingForResponse_global = "";
////////////////////////////////////////////////////////
//DEBUGING VARIABLES
////////////////////////////////////////////////////////
float t1 = -3;
float t2 = -4;
float td = 5;

uint16_t notParsedHttpResponses_errorCount = 0;
uint16_t parsedHttpResponses_notErrorCount = 0;
uint16_t totalErrorCount_global = 0;
////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------

//Program itself - initialization
void setup()
{
  if(WAITING_FOR_RESPONSES_MODULE_ENABLED) setObjectAskingForResponse("Begining");
  Serial.begin(115200);
  Serial.println(""); //just to start printing on new line..
  if (MAIN_DEBUG) Serial.println("F:MAIN_setup()");

  if (DISPLAY_ENABLE) i2cBus_setup();
  relayBoard_setup();

  wifiConnect();
  yield();

  if (DISPLAY_ENABLE) lcd_setup();
  if (DISPLAY_ENABLE) oledDisplay_setup();
  yield();

  if (WATER_FLOW_ENABLE) waterFlowSensor_setup();
  if (OTA_ENABLE) OTA_setup();

  if (CURRENT_ENABLE) current_loop(1); //check state at begining 1 means nothing
}


void loop()
{
  if (MAIN_DEBUG) Serial.println("\nF:MAIN_loop()");

  Tasker tasker(false);// false - do not prioritize tasks, do not skip any task in line

  if (OTA_ENABLE)        tasker.setInterval(OTA_loop, 1000);
  if (DISPLAY_ENABLE)    tasker.setInterval(displayData_loop, 1000);
  tasker.setInterval(mainTemperature_loop, 3 * 1000,1);
  tasker.setInterval(mainTemperature_loop, 3 * 1000,0);
  if (WATER_FLOW_ENABLE) tasker.setInterval(waterFlow_loop, 10 * 1000);
  if (CURRENT_ENABLE)    tasker.setInterval(current_loop, 16 * 1000);
  tasker.setInterval(checkSystemState_loop, 60 *  60 *  1000); // 60 * 60 * 1000 = 1 hour
  
  if (EVERY_DAY_RESTART_ENABLE)    tasker.setInterval(everyDayRestartFunction,  24* 60 * 60 * 1000); // 60 * 60 * 1000 = 1 hour
  

  yield();
  tasker.run(); // never returns
}

//--------------------------------------------------------------------------------

void mainTemperature_loop(int sensorIndex)
{
  if (MAIN_DEBUG) Serial.println("F:temperature_loop()");
  float namerenaTeplotaVBojleru = readTemperature(sensorIndex);
  if (MAIN_DEBUG) Serial.println("F:readTemperature() returned: " + (String)namerenaTeplotaVBojleru);

  if(namerenaTeplotaVBojleru == -127 || namerenaTeplotaVBojleru == 85)
  {
    totalErrorCount_global++;
    return;
  }
  
  if(sensorIndex == 1)// boiler temp
    if (abs(namerenaTeplotaVBojleru - lastTemp_global) >= MIMIMAL_SENDING_TEMP_DIFFERENCE)
    {
      if (abs((int)namerenaTeplotaVBojleru - (int)lastTemp_global) >= 1) //pokud měření právě překročilo hranici stupně
        if (((int)namerenaTeplotaVBojleru % TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE) == 0) //každé dva stupně si zažádej o vedení
          setObjectAskingForResponse("Temp_" +  (String)TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE);

      lastTemp_global = namerenaTeplotaVBojleru;
      Serial.print("Temperature has changed.. sending update data: ");
      Serial.println(lastTemp_global);

      sendAllSensorData("bojlerTemp_changed");
    }

  if(sensorIndex == 0)// pipe temp
  {
    if(namerenaTeplotaVBojleru > pipeTemp_global)
    {
      if ((namerenaTeplotaVBojleru - pipeTemp_global) >= 1)
      {      
        pipeTemp_global = namerenaTeplotaVBojleru;
        Serial.print("pipeTemp_global has rised.. sending update data: ");
        Serial.println(pipeTemp_global);

        sendAllSensorData("pipeTemp_risedUp");
      }
    } 
    else
        pipeTemp_global = namerenaTeplotaVBojleru;
  }
}

void setObjectAskingForResponse(String object)
{
  if(WAITING_FOR_RESPONSES_MODULE_ENABLED)
    objectAskingForResponse_global += object;
}

bool isSomebodyAskingForResponse()
{
  return (objectAskingForResponse_global != "") ? true : false;
}

String getObjectAskingForResponse()
{
  return objectAskingForResponse_global;
}

void resetObjectAskingForResponse()
{
  objectAskingForResponse_global = "";
}

void stressAskingForResponse()
{
  objectAskingForResponse_global = "!" + objectAskingForResponse_global;
}


void current_loop(int)
{
  if (MAIN_DEBUG) Serial.println("F:current_loop()");

  int currentNow = getMaxCurrentValue();
  if (abs(lastCurrentMeasurment_global - currentNow) > 100)
  {
    if (lastCurrentMeasurment_global > 800 && currentNow < 200) //Pokud se právě vypnul ohřev vody v bojleru
    {
      waterFlowDisplay_global = 0; //Vynuluj měření spotřeby teplé vody - bojler je po vypnutí ohřevu celý nahřátý
      //TODO : převést na funkci a funkci dát do display souboru kam patří
      sendNodeState("WaterFlowDisplay_reset");
    }

    lastCurrentMeasurment_global = currentNow;
    sendAllSensorData((lastCurrentMeasurment_global > currentNow) ? "Current_dropped" : "Current_rised");
  }
  else
    lastCurrentMeasurment_global = currentNow;
}


void waterFlow_loop(int)
{
  if (MAIN_DEBUG) Serial.println("F:waterFlow_loop()");

  if (readFlowInLitres() == 0 && lastwaterFlowSensorCount_global > 10)
  {
    Serial.println("Flow state has droped to zero.. sending update data..");
    sendAllSensorData("WaterFlow_Stopped");
    //if((millis() - flowOffTime_global) > 1000 * 60 * 20) //pokud od posledního puštění vody uplynulo více než 20minut
    //waterFlowDisplay_global = 0; //zresetuj počítadlo spotřeoavné vody na displayi

    flowOffTime_global = millis();
  }
}



void everyDayRestartFunction(int)
{
  sendAllSensorData("EveryDayRestart");
  if (MAIN_DEBUG) Serial.println("F:EveryDayRestartFunction()");
  
  ESP.restart();
}
void checkSystemState_loop(int)
{
  setObjectAskingForResponse("checkSystemState_loop");
  String systemStateInfo = "";

  if (MAIN_DEBUG) Serial.println("F:checkSystemState()");
  if ((totalErrorCount_global / millis() / 1000 / 60 / 60) >= MAX_ERROR_COUNT_PER_HOUR)
    ESP.restart();

  systemStateInfo += "getFreeHeap(); " + (String)ESP.getFreeHeap() + "\n";
  systemStateInfo += "Uptime: " + getStringUpTime() + "\n";
  systemStateInfo += "TimeFromLastUpdate: " + getTimeFromLastUpdate() + "\n";
  systemStateInfo += "totalErrorCount_global: " + (String)totalErrorCount_global + "\n";
  systemStateInfo += "ERROR_COUNT_PER_HOUR: " + (String)(totalErrorCount_global / millis() / 1000 / 60 / 60) + "\n";

  systemStateInfo += "notParsedHttpResponses_errorCount" + (String)notParsedHttpResponses_errorCount + "\n";
  systemStateInfo += "parsedHttpResponses_notErrorCount" + (String)parsedHttpResponses_notErrorCount + "\n";

  //TODO nastavit pouze proměnou a po té ji při dalším odeslání přiložit
  systemStateInfo_global = systemStateInfo;
  //postDataToServer(systemStateInfo_global);



  if (SHOW_ERROR_DEBUG_INFORMATION) Serial.println(systemStateInfo);
  if (SHOW_ERROR_DEBUG_INFORMATION) WiFi.printDiag(Serial);

  if (lastTemp_global >= CUTTING_OFF_BOILER_ELECTRICITY_SAFE_TEMPERATURE)
  {
    cutOffBoilerElectricity();
    sendNodeState(String("BOILER_TEMP_OverHeating_" + CUTTING_OFF_BOILER_ELECTRICITY_SAFE_TEMPERATURE).c_str());
  }
}


//--------------------------------------------------------------------------------

void wifiConnect()
{
  if (MAIN_DEBUG) Serial.println("F:wifiConnect()");
  showServiceMessage("WiFi");

  WiFi.begin(AP_SSID, AP_PASSWORD);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    displayPrint(".");
  }
  Serial.println("");

  sendNodeState("WiFi_connected");
}


void doNecesaryActionsUponResponse(String parsedText)
{
  if (MAIN_DEBUG) Serial.println("F:doNecesaryActionsUponResponse(String parsedText)" + (String)parsedText);

  //client.parseInt(); //TODO?

  if (parsedText.indexOf("boilerHeating_NoChange") == -1)
    if (parsedText.indexOf("boilerHeating_On") != -1)
      turnOnBoilerHeating();
    else if (parsedText.indexOf("boilerHeating_Off") != -1)
      turnOffBoilerHeating();


  if (parsedText.indexOf("boilerElectricity_OFF") != -1)
    cutOffBoilerElectricity();
  else if (parsedText.indexOf("boilerElectricity_ON") != -1)
    turnOnBoilerElectricity();

  if (parsedText.indexOf("nodeRestart_True") != -1)
    ESP.restart();

}





