/*
  Created by Jindrich SIRUCEK
  See www.jindrichsirucek.cz
  Email: jindrich.sirucek@gmail.com


*/


//--------------------------------------------------------------------------------
//Libraries included
#include <ESP8266WiFi.h>
#include <Tasker.h>


#define DEBUG_REZIM false //if true? disable debugging output information
#define DISABLE_SOME_MODULES true // disables some modules

#define NODE_NAME "Doma"

#if DISABLE_SOME_MODULES
#define OTA_MODULE_ENABLED true
#define DISPLAY_MODULE_ENABLED true
#define WATER_FLOW_MODULE_ENABLED true
#define CURRENT_MODULE_ENABLED true
#define TEMP_MODULE_ENABLED true
#define WAITING_FOR_RESPONSES_MODULE_ENABLED true
#define UPLOADING_DATA_MODULE_ENABLED true
#define SENDING_BEACON_MODULE_ENABLED true       //TODO:Node posílá každou hodinu info o tom, že žije


#else
//All Modules enabled
#define OTA_MODULE_ENABLED true
#define DISPLAY_MODULE_ENABLED true
#define WATER_FLOW_MODULE_ENABLED true
#define CURRENT_MODULE_ENABLED true
#define TEMP_MODULE_ENABLED true
#define WAITING_FOR_RESPONSES_MODULE_ENABLED true
#define UPLOADING_DATA_MODULE_ENABLED true
#define SENDING_BEACON_MODULE_ENABLED true       //TODO:Node posílá každou hodinu info o tom, že žije


#endif
//Home mode!
#if !DEBUG_REZIM
#define MAIN_DEBUG true
#define TEMPERATURE_DEBUG false
#define DEBUG_REMOTE_CONSOLE false
#define DISPLAY_DEBUG false
#define RELAY_DEBUG false
#define CURRENT_DEBUG false
#define INTERNET_COMMUNICATION_DEBUG false
#define SHOW_ERROR_DEBUG_INFORMATION true

//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size
//#define Serial remoteConsole //forwards all Serial outputs to remote console object

#else
//Development mode! Sets what to debug
#define NODE_NAME "development"
#define MAIN_DEBUG true
#define TEMPERATURE_DEBUG true
#define DEBUG_REMOTE_CONSOLE true
#define DISPLAY_DEBUG false
#define RELAY_DEBUG true
#define CURRENT_DEBUG false
#define INTERNET_COMMUNICATION_DEBUG false
#define SHOW_ERROR_DEBUG_INFORMATION true



//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size
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
#define AP_SSID "UPC3049010"
#define AP_PASSWORD "RXYDNHRD"

//#define DATA_SERVER_HOST_ADDRESS "script.google.com"
//#define DATA_SERVER_SCRIPT_URL "/macros/s/AKfycbw19MD0EY_ynw22Az6qgWtdC2rdz9dik5dWc-N8CH6VZvbsix6q/exec"

#define DATA_SERVER_HOST_ADDRESS "jindrichsirucek.cz"//ESP8266 cant make HTTPS Request so far
#define DATA_SERVER_SCRIPT_URL "/sensorData.php"

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
#define TASKER_MAX_TASKS 10 // define max number of tasks to save precious Arduino RAM, 10 is default value
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

unsigned long lastWaterFlowSensorCount_global = 0;
volatile unsigned long waterFlowSensorCount_ISR_global;  // Measures flow meter pulses
unsigned long waterFlowDisplay_global = 0;
unsigned long lastWaterFlowResetTime_global = 0; //time from last measured water flow


/// Restart will be triggered on this time
unsigned long espRestartTime_global = 160 * 1000; //this value need to be changed also in function itself

String systemStateInfo_global = "";
String objectAskingForResponse_global = "";
String nowTime_global = "";
String nowDate_global = "";
////////////////////////////////////////////////////////
//DEBUGING VARIABLES and SETTINGS
////////////////////////////////////////////////////////
float t1 = -3;
float t2 = -4;
float td = 5;

uint16_t notParsedHttpResponses_errorCount = 0;
uint16_t parsedHttpResponses_notErrorCount = 0;
uint16_t totalErrorCount_global = 0;

#include "FS.h"

File debugFile = SPIFFS.open("/test.txt", "w");
//#define DEBUG_OUTPUT Serial
#define DEBUG_OUTPUT debugFile

////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------

//Program itself - initialization
void setup()
{
  SPIFFS_setup();
  DEBUG_OUTPUT.println(""); //just to start printing on new line..
  DEBUG_OUTPUT.print(NODE_NAME);
  DEBUG_OUTPUT.println(" node initializing..");

  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:MAIN_setup()");

  if (DISPLAY_MODULE_ENABLED) i2cBus_setup();
  relayBoard_setup();

  wifiConnect();
  yield();

  if (DISPLAY_MODULE_ENABLED) lcd_setup();
  if (DISPLAY_MODULE_ENABLED) oledDisplay_setup();
  if (WATER_FLOW_MODULE_ENABLED) waterFlowSensor_setup();
  if (OTA_MODULE_ENABLED) OTA_setup();
  if (CURRENT_MODULE_ENABLED) current_loop(1); //check state of current at begining, 1 means nothing

  debugFile.close();
  debugFile = SPIFFS.open("/test.txt", "a");
}


void loop()
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("\nF:MAIN_loop()");

  Tasker tasker(false);// false - do not prioritize tasks, do not skip any task in line

  if (OTA_MODULE_ENABLED)        tasker.setInterval(OTA_loop, 50);
  if (DISPLAY_MODULE_ENABLED)    tasker.setInterval(displayData_loop, 1000);
  if (TEMP_MODULE_ENABLED)       tasker.setInterval(temperature_loop, 3 * 1000);
  if (WATER_FLOW_MODULE_ENABLED) tasker.setInterval(waterFlow_loop, 10 * 1000);
  if (CURRENT_MODULE_ENABLED)    tasker.setInterval(current_loop, 16 * 1000);
  tasker.setInterval(checkSystemState_loop, 60 *  60 *  1000); // 60 * 60 * 1000 = 1 hour


  //tasker.setInterval(printDebugFile_loop, 20 * 1000); // 60 * 60 * 1000 = 1 hour

  yield();
  tasker.run(); // never returns
}

//--------------------------------------------------------------------------------

void temperature_loop(int)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:temperature_loop()");
  float namerenaTeplotaVBojleru = readTemperature(1);
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:readTemperature() returned: " + (String)namerenaTeplotaVBojleru);

  if (namerenaTeplotaVBojleru == -127 || namerenaTeplotaVBojleru == 85)
  {
    totalErrorCount_global++;
    return;
  }

  if (abs(namerenaTeplotaVBojleru - lastTemp_global) >= MIMIMAL_SENDING_TEMP_DIFFERENCE)
  {
    if (abs((int)namerenaTeplotaVBojleru - (int)lastTemp_global) >= 1) //pokud měření právě překročilo hranici stupně
      if (((int)namerenaTeplotaVBojleru % TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE) == 0) //každé dva stupně si zažádej o vedení
        setObjectAskingForResponse("Temp_" +  (String)TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE);

    lastTemp_global = namerenaTeplotaVBojleru;
    DEBUG_OUTPUT.print("Temperature has changed.. sending update data: ");
    DEBUG_OUTPUT.println(lastTemp_global);

    sendAllSensorData("Temp_changed");
  }
}

void setObjectAskingForResponse(String object)
{
  if (WAITING_FOR_RESPONSES_MODULE_ENABLED)
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
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:current_loop()");
  
  postDataByPost();
  
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
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:waterFlow_loop()");

  if (readFlowInLitres() == 0 && lastWaterFlowSensorCount_global > 10)//watter stopped flowing
  {
    DEBUG_OUTPUT.println("Flow state has droped to zero.. sending update data..");
    sendAllSensorData("WaterFlow_Stopped");

    //waterFlowDisplay_global = 0; //zresetuj počítadlo spotřeoavné vody na displayi
  }
  if (readFlowInLitres() > 0 && lastWaterFlowSensorCount_global <= 10) //watter started to flow
    lastWaterFlowResetTime_global = millis();
}


void checkSystemState_loop(int)
{
  setObjectAskingForResponse("checkSystemState_loop");
  String systemStateInfo = "";

  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:checkSystemState()");
  //if ((totalErrorCount_global / millis() / 1000 / 60 / 60) >= MAX_ERROR_COUNT_PER_HOUR)
    //ESP.restart();

  systemStateInfo += "getFreeHeap(); " + (String)ESP.getFreeHeap() + "\n";
  systemStateInfo += "Uptime: " + getStringUpTime() + "\n";
  systemStateInfo += "TimeFromLastUpdate: " + getTimeFromLastUpdate() + "\n";
  systemStateInfo += "totalErrorCount_global: " + (String)totalErrorCount_global + "\n";
  systemStateInfo += "ERROR_COUNT_PER_HOUR: " + (String)(totalErrorCount_global / millis() / 1000 / 60 / 60) + "\n";

  systemStateInfo += "notParsedHttpResponses_errorCount" + (String)notParsedHttpResponses_errorCount + "\n";
  systemStateInfo += "parsedHttpResponses_notErrorCount" + (String)parsedHttpResponses_notErrorCount + "\n";

  systemStateInfo_global = systemStateInfo;
  //postDataToServer(systemStateInfo_global);
  sendNodeState("Beacon_Alive");//zároveň slouží jako beacon alive


  if (SHOW_ERROR_DEBUG_INFORMATION) DEBUG_OUTPUT.println(systemStateInfo);
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
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:wifiConnect()");
  showServiceMessage("WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    DEBUG_OUTPUT.print(".");
    displayPrint(".");
  }
  DEBUG_OUTPUT.println("");

  sendNodeState("WiFi_connected");
}


void doNecesaryActionsUponResponse(String parsedText)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("F:doNecesaryActionsUponResponse(String parsedText)" + (String)parsedText);

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

  nowTime_global = parsedText.substring(parsedText.indexOf("nowTime_") + 8, parsedText.indexOf("nowTime_") + 8 + 8);
  nowDate_global = parsedText.substring(parsedText.indexOf("nowDate_") + 8, parsedText.indexOf("nowDate_") + 8 + 10);

}





