#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char *ssid = "Drone";
const char *password = "password";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Create WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Print the IP
  Serial.println("Access Point started");
  Serial.println("IP Address: " + WiFi.softAPIP().toString());

  // Setup OTA
  AsyncElegantOTA.begin(&server);

  // Start the server
  server.begin();
}

void processCommand(char command) {
  switch (command) {
    case 'w':
      Serial.println("Received command forwards");
      break;
    case 'a':
      Serial.println("Received command left");
      break;
    case 's':
      Serial.println("Received command backwards");
      break;
    case 'd':
      Serial.println("Received command right");
      break;
    default:
      Serial.println("Unknown command");
  }
}

void loop() {
  // OTA updates
  AsyncElegantOTA.loop();

  // Check for serial input
  while (Serial.available() > 0) {
    char command = Serial.read();
    processCommand(command);
  }
}
