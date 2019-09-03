#include "settings.h"
#include <WiFiManager.h>
#include <ArduinoOTA.h>

WiFiServer server(80);

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  //entered config mode
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  // wm.resetSettings();
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect()) {
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }
  WiFi.hostname(WIFI_HOSTNAME);
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed"); }
    else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed"); }
    else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed"); }
    else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed"); }
    else if (error == OTA_END_ERROR) { Serial.println("End Failed"); }
  });
  // Start the server
  server.begin();
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  WiFiClient client = server.available();

  if (!client) 
  {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available())
  {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  // Match the request
  int value = LOW;
  if (request.indexOf("/RELAY=ON") != -1)  
  {
    Serial.println("RELAY=ON");
    digitalWrite(RELAY,LOW);
    value = LOW;
  }
  if (request.indexOf("/RELAY=OFF") != -1)  
  {
    Serial.println("RELAY=OFF");
    digitalWrite(RELAY,HIGH);
    value = HIGH;
  }
  
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  this is a must
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>ESP8266 RELAY Control</title></head>");
  client.print("Relay is now: ");
 
  if(value == HIGH) 
  {
    client.print("OFF");
  } 
  else 
  {
    client.print("ON");
  }
  client.println("<br><br>");
  client.println("Turn <a href=\"/RELAY=OFF\">OFF</a> RELAY<br>");
  client.println("Turn <a href=\"/RELAY=ON\">ON</a> RELAY<br>");
    client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");

}
