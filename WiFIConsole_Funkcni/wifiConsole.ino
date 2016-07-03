

int wifiPrintf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char temp[1460];
  size_t len = ets_vsnprintf(temp, 1460, format, arg);
  wifiConsolePrintln(temp);
  va_end(arg);
  return len;
}


void WifiConsole_println() 
{
    wifiPrintf("Client running at IP address: %s", WiFi.localIP().toString().c_str());

}


void wifiConsolePrintln(char* text)
{
  wifiBuffer.println("");
  wifiBuffer.print("WiFiConsole: ");
  wifiBuffer.println(text);

  String url = "/sensorData_readConsole.php?" + URLEncode(getUpTime()) + "=" + URLEncode(text);
  sendData(url);
}


char* getUpTime()
{
  char myConcatenation[15];
  //  long milisecondsFromLastSend = (millis() - lastUpdateTime_global);

  long milisecondsFromLastSend = millis();

  int hoursFromLastSend = milisecondsFromLastSend / 1000 / 60 / 60;
  int minutesFromLastSend = milisecondsFromLastSend / 1000 / 60 - hoursFromLastSend * 60;
  int secondsFromLastSend = milisecondsFromLastSend / 1000 - minutesFromLastSend * 60 - hoursFromLastSend * 60 * 60;

  sprintf(myConcatenation, "%02d:%02d:%02d", hoursFromLastSend, minutesFromLastSend, secondsFromLastSend);

  //    ltoa(secondsFromLastSend,myConcatenation,10);

  return myConcatenation;
}

void sendData(String url)
{
  WiFiClient client;
  while (!client.connect("jindrichsirucek.cz", 80)) {
    wifiBuffer.println("connection failed");
      WiFi.begin(g_ssid.c_str(), g_pass.c_str());
  }

  wifiBuffer.print("POST data to URL: ");
  wifiBuffer.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + String("jindrichsirucek.cz") + "\r\n" +
               "Connection: close\r\n" +
               "Content-Length: 0\r\n" +
               "\r\n");

  delay(1000);
  while (client.available()) {
    String line = client.readStringUntil('\r');
    wifiBuffer.print(line);
  }

  wifiBuffer.println();
  wifiBuffer.println("Connection closed");
}



WiFiClient connectToServer()
{
  char host[] = "jindrichsirucek.cz";
  WiFiClient client;
  wifiBuffer.print("Client connected: ");
  wifiBuffer.println(host);
  return client;
}



String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
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

