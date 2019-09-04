#include "settings.h"
#include <WiFiManager.h>
#include <ArduinoOTA.h>

WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String relayState = "off";

// Assign output variables to GPIO pins
const int relayPin = 2;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  //entered config mode
}

void setup() {
  delay(1000);
  // Initialize the output variables as outputs
  pinMode(relayPin, OUTPUT);

  // Set outputs to LOW
  digitalWrite(relayPin, LOW);
  
  //Serial.begin(115200);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  //wm.resetSettings();
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

  ArduinoOTA.setHostname(WIFI_HOSTNAME);
  ArduinoOTA.onStart([]() {
    //Serial.println("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    //if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed"); }
    //else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed"); }
    //else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed"); }
    //else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed"); }
    //else if (error == OTA_END_ERROR) { Serial.println("End Failed"); }
  });
  // Start the server
  server.begin();
  ArduinoOTA.begin();
}


void loop() {
  ArduinoOTA.handle();
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /relay/on") >= 0) {
              Serial.println("Relay on");
              relayState = "on";
              digitalWrite(relayPin, HIGH);
            } else if (header.indexOf("GET /relay/off") >= 0) {
              Serial.println("Relay off");
              relayState = "off";
              digitalWrite(relayPin, LOW);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Relay - State " + relayState + "</p>");
            // If the relayState is off, it displays the ON button       
            if (relayState=="off") {
              client.println("<p><a href=\"/relay/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/relay/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}
