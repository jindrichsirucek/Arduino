//internet_comunication.ino





void sendAllSensorData(String fireEventName)
{
  if(MAIN_DEBUG) Serial.println("F:sendAllSensorData(String fireEventName) :" + fireEventName);
  
  //prepare data to send
  String mainTemp = (lastTemp_global != -127) ? String(lastTemp_global) : "";
  String flowTemp = (flowTemp_global != -127) ? String(flowTemp_global) : "";
  String pipeTemp = (pipeTemp_global != -127) ? String(pipeTemp_global) : "";

  String url = "?temp=" + mainTemp + "&flowTemp=" + flowTemp + "&pipeTemp=" + pipeTemp;


  String waterFlow = "";
  if(waterFlowSensorCount_global != 0)
  {
    waterFlow = String(convertWaterFlowSensorImpulsesToLitres(waterFlowSensorCount_global));
    resetflowCount();
  }

  String current = (lastCurrentMeasurment_global != 0) ? String(lastCurrentMeasurment_global) : "";
  url += "&waterFlow=" + waterFlow + "&event=" + fireEventName + "&current=" + current; // + "&" + "tGlobal" + "=" + t2 + "&" + "tDiff" + "=" + td ;

  String programRelay = (heatingBoilerRelayState_global == ON_RELAY_STATE) ? "ON" : "OFF";
  String manualRelay = (cutOffBoilerElectricityRelayStateFlag_global == ON_RELAY_STATE) ? "CUT_OFF" : "ONLINE";
  
  url += "&manualRelayState=" + manualRelay + "&programaticRelayState=" + programRelay;

  postDataToServer(url);
  lastUpdateTime_global = millis();
}


void postDataToServer(String url)
{
  if(MAIN_DEBUG) Serial.println("F:postDataToServer(String url) : " + url);
  
  //append node identification
  char buffer[50];
  IPAddress ip = WiFi.localIP();
  sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  url = "/sensorData.php" + url + "&chipId=" + String(ESP.getChipId()) + "&ip=" + URLEncode(buffer) + "&nodeName=" + URLEncode(NODE_NAME);   
  
  url += "&response=" + URLEncode(getObjectAskingForResponse().c_str());

  url += (systemStateInfo_global != "") ? "&systemStateInfo=" + URLEncode(systemStateInfo_global.c_str()) : ""; //nodeStatus
  systemStateInfo_global = "";//Resets system state

  if(NOT_SENDING_DATA) { Serial.println("Not sending data over WiFi: NOT_SENDING_DATA code"); return;  }
  if(INTERNET_COMMUNICATION_DEBUG) Serial.println("jindrichsirucek.cz" + url);
  
  showServiceMessage("Sending...");
  
  WiFiClient client = connectToServer();
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String("jindrichsirucek.cz") + "\r\n"
               +  "Connection: close\r\n" +
               "Content-Length: 0\r\n" +
               "\r\n"
              );
  delay(10);

  showServiceMessage("Data sent!");

  if(WAITING_FOR_RESPONSES_MODULE_ENABLED) if(isSomebodyAskingForResponse())
    zpracujOpdpoved(&client);
      
  flowTemp_global++;
}



bool zpracujOpdpoved(WiFiClient *client)
{
  int i = 0;
  while((!client->available()) && (i++ < 1000)) delay(10); //reads response


  if(INTERNET_COMMUNICATION_DEBUG) Serial.println("Client response length: " + (String)client->available());

  bool printLine_Flag = false;
  if(INTERNET_COMMUNICATION_DEBUG) if(client->available() > 500) printLine_Flag = true;
      
  String responseText = "";
  while(client->available())
  {
    String line = client->readStringUntil('\r');
    if(INTERNET_COMMUNICATION_DEBUG) responseText += line;

    if(INTERNET_COMMUNICATION_DEBUG) if(printLine_Flag) 
    {
      line.replace("\r","");
      line.replace("\n","");
      Serial.println("---->" + line);
    }

    int lengthOfLine = line.length();
    if(lengthOfLine > 50)
    {
      int pos = line.indexOf("\"cajaHtml\":");
      if(pos >= 0)
      {

        line = line.substring(pos, line.indexOf(",\"title\":"));
        if(INTERNET_COMMUNICATION_DEBUG) Serial.println("Parsed response: " + line);
        doNecesaryActionsUponResponse(line);
        resetObjectAskingForResponse();
      }
    }
  }
  
  if(isSomebodyAskingForResponse()) //Pokud se nepodařilo zpracovat odpoved
  {
    stressAskingForResponse();

    showServiceMessage("NO Response!");   
    notParsedHttpResponses_errorCount++; totalErrorCount_global++;

    if(SHOW_ERROR_DEBUG_INFORMATION) Serial.println("---->Response not parsed!");
    if(SHOW_ERROR_DEBUG_INFORMATION) Serial.println((String)notParsedHttpResponses_errorCount + " responses not parsed, " + parsedHttpResponses_notErrorCount + " parsed OK!");
    if(INTERNET_COMMUNICATION_DEBUG) Serial.println("CajaHtml_Flag not found, Printing whole response: \n" + responseText);
  }
  else
  {
      parsedHttpResponses_notErrorCount++;
      showServiceMessage("Response OK!");
  }

  return !isSomebodyAskingForResponse(); //if noone asking for anything, thed response was accepted
}

WiFiClient connectToServer()
{
  char host[] = "jindrichsirucek.cz";
  WiFiClient client;
  while(!client.connect(host, 80))
  {
    totalErrorCount_global++;
    showServiceMessage("WiFi down");
  }
  if(INTERNET_COMMUNICATION_DEBUG) Serial.println("Client connected: " + (String)host);

  return client;
}


void sendNodeState(const char* state)
{
  if(MAIN_DEBUG) Serial.println("F:sendNodeState(const char* state) : " + (String)state);
  
  showServiceMessage(state);

  //String url = "?event=" + URLEncode(state); //nodeStatus
  //postDataToServer(url);

  sendAllSensorData(state);
}



String URLEncode(const char* msg)
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
