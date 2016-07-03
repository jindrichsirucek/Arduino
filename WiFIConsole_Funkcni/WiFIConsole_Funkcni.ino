/**
 * @file esp8266-webconf-mDNS-OTA.ino
 *
 * @author Pascal Gollor (http://www.pgollor.de/cms/)
 * @data 2015-08-17

 http://esp8266.local/set?ssid=UPC3049010&pass=RXYDNHRD
 *
 */


 #include <ESP8266WiFi.h>
 #include <ESP8266mDNS.h>
 #include <WiFiUdp.h>
 #include <ESP8266WebServer.h>
 
 int wifiPrintf(const char *format, ...);


 #include <PString.h>
char buffer[256];
PString wifiBuffer(buffer, sizeof(buffer));


/**
 * @brief mDNS and OTA Constants
 * @{
   */
#define HOSTNAME "esp8266" ///< Hostename 
#define APORT 8266 ///< Port for OTA update
/// @}

/**
 * @brief Default WiFi connection information.
 * @{
   */
const char* ap_default_ssid = "UPC3049010"; ///< Default SSID.
const char* ap_default_psk = "RXYDNHRD"; ///< Default PSK.
/// @}

/// HTML answer on restart request.
#define RESTART_HTML_ANSWER "<html><head><meta http-equiv=\"refresh\" content=\"15; URL=http://" HOSTNAME ".local/\"></head><body>Restarting in 15 seconds.<br/><img src=\"/loading.gif\"></body></html>"

/// OTA Update UDP server handle.
WiFiUDP OTA;

/// Webserver handle on port 80.
ESP8266WebServer g_server(80);

/// global WiFi SSID.
String g_ssid = "";

/// global WiFi PSK.
String g_pass = "";

/// Restart will be triggert on this time
unsigned long g_restartTime = 0;



/**
 * @brief Handle OTA update stuff.
 *
 * This function comes from ESP8266 Arduino example:
 * https://github.com/esp8266/Arduino/blob/esp8266/hardware/esp8266com/esp8266/libraries/ESP8266mDNS/examples/DNS_SD_Arduino_OTA/DNS_SD_Arduino_OTA.ino
 *
 */
 static inline void ota_handle(void)
 {
  if (! OTA.parsePacket())
  {
    return;
  }

  IPAddress remote = OTA.remoteIP();
  int cmd  = OTA.parseInt();
  int port = OTA.parseInt();
  int size   = OTA.parseInt();

  wifiBuffer.print("Update Start: ip:");
  wifiBuffer.print(remote);
  wifiBuffer.printf(", port:%d, size:%d\n", port, size);
  uint32_t startTime = millis();

  WiFiUDP::stopAll();
  delay(1000);

  if (!Update.begin(size))
  {
    wifiBuffer.println("Update Begin Error");
    return;
  }

  WiFiClient client;
  if (client.connect(remote, port))
  {
    uint32_t written;
    while (!Update.isFinished())
    {
      written = Update.write(client);
      if (written > 0) client.print(written, DEC);
    }
    wifiBuffer.setDebugOutput(false);

    if (Update.end())
    {
      client.println("OK");
      wifiBuffer.printf("Update Success: %u\nRebooting...\n", millis() - startTime);
      ESP.restart();
    }
    else
    {      
      //Update.printError(wifiBuffer);
      //ESP.restart();
      OTA.begin(APORT);
    }
  }
  else
  {
    wifiBuffer.printf("Connect Failed: %u\n", millis() - startTime);
  }
} // ota_handle


/**
 * @brief Handle http root request.
 */
 void handleRoot()
 {
  String indexHTML;
  char buff[10];
  uint16_t s = millis() / 1000;
  uint16_t m = s / 60;
  uint8_t h = m / 60;

  
  if (false)
  {  }
  else
  {
    indexHTML = "<html><head><title>File not found</title></head><body><h1>File not found.</h1></body></html>";
  }

  g_server.send (200, "text/html", indexHTML);
} // handleRoot


/**
 * @brief Handle set request from http server.
 *
 * URI: /set?ssid=[WiFi SSID]&pass=[WiFi Pass]
 * http://esp8266.local/set?ssid=UPC3049010&pass=RXYDNHRD
 */
 void handleSet()
 {
  String response = "<html><head><meta http-equiv=\"refresh\" content=\"2; URL=http://";
  response += HOSTNAME;
  response += ".local\"></head><body>";

  // Some debug output
  wifiBuffer.print("uri: ");
  wifiBuffer.println(g_server.uri());

  wifiBuffer.print("method: ");
  wifiBuffer.println(g_server.method());

  wifiBuffer.print("args: ");
  wifiBuffer.println(g_server.args());

  // Check arguments
  if (g_server.args() < 2)
  {
    g_server.send (200, "text/plain", "Arguments fail.");
    return;
  }

  String ssid = "";
  String pass = "";

  // read ssid and psk
  for (uint8_t i = 0; i < g_server.args(); i++)
  {
    if (g_server.argName(i) == "ssid")
    {
      ssid = g_server.arg(i);
    }
    else if (g_server.argName(i) == "pass")
    {
      pass = g_server.arg(i);
    }
  }

  // check ssid and psk
  if (ssid != "" && pass != "")
  {
    {
      response += "<h1>Fail save to config file.</h1>";
    }
  }
  else
  {
    response += "<h1>Wrong arguments.</h1>";
  }

  response += "</body></html>";
  g_server.send (200, "text/html", response);
} // handleSet








void setup()
{
  g_ssid = "";
  g_pass = "";

  wifiBuffer.begin(115200);

  delay(100);

  wifiBuffer.println("\r\n");
  wifiBuffer.print("Chip ID: ");
  wifiBuffer.println(ESP.getChipId());

  wifiBuffer.println("Wait for WiFi connection.");

  // Try to connect to WiFi AP.
  WiFi.mode(WIFI_STA);
  delay(10);
  WiFi.begin(g_ssid.c_str(), g_pass.c_str());

  // check connection
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    
    wifiBuffer.println("connect");
    wifiBuffer.print("Client running at IP address: ");
    wifiBuffer.println(WiFi.localIP());

  }
  else
  {
    // Go into station mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    wifiBuffer.print("IP address: ");
    wifiBuffer.println(WiFi.softAPIP());
  }

  // Initialize mDNS service.
  MDNS.begin(HOSTNAME);

  // ... Add OTA service.
  MDNS.addService("arduino", "tcp", APORT);

  // ... Add http service.
  MDNS.addService("http", "tcp", 80);

  // Open OTA Server.
  OTA.begin(APORT);

  // Initialize web server.
  // ... Add requests.
  g_server.on("/", handleRoot);
  g_server.on("/set", HTTP_GET, handleSet);
  g_server.on("/restart", []() {
    g_server.send(200, "text/html", RESTART_HTML_ANSWER);
    g_restartTime = millis() + 100;
    } );

  // ... Start server.
  g_server.begin();

  //saveIndexFile();

  wifiBuffer.print("a1234567890");

  wifiBuffer.println("b1234567890");
  wifiBuffer.print("c1234567890");
  wifiBuffer.println("d1234567890");
  wifiBuffer.println("abcdefghijklmnopqrstuvwxyz");
  wifiBuffer.println("e1234567890");
}


/**
 * @brief Arduino loop function.
 */
 void loop()
 {
  // Handle OTA update.
  ota_handle();

  // Handle Webserver requests.
  g_server.handleClient();

  // trigger restart?
  if (g_restartTime > 0 && millis() >= g_restartTime)
  {
    g_restartTime = 0;
    ESP.restart();
  }



  delay(100);

}



