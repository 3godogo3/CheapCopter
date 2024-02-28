#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <MPU6050_light.h>
#include "Wire.h"

//Wi-Fi AP credentials
const char *ssid = "Drone";
const char *password = "password";

AsyncWebServer server(80);
WiFiServer telnet(23);

WiFiClient client;

//motor pins
int motorHigh = 12;
int motorLow = 13;
int motor1Enable = 14;
int motor2Enable = 15;
int motor3Enable = 16;
int motor4Enable = 17;

// Declare mpu
MPU6050 mpu(Wire);

// PID parameters. (NEEDS TO BE ADJUSTED ON WORKING DRONE)
float kp = 1.0;
float ki = 0.1;
float kd = 1.0;

// Declare PID errors
float lastErrorPitch, integralPitch;
float lastErrorRoll, integralRoll;

// Declare motor speeds
int motorSpeed1, motorSpeed2, motorSpeed3, motorSpeed4;

// Declare directional speeds
int forward, backward, left, right, up, down;
int movementSpeed = 50; //NEEDS TO BE ADJUSTED ON A WORKING DRONE

void setup() {
  Serial.begin(115200);

  // Create WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Print the IP
  //Serial.println("Access Point started");
  //Serial.println("IP Address: " + WiFi.softAPIP().toString());

  // Setup OTA
  AsyncElegantOTA.begin(&server);

  // Setup MPU6050
  Wire.begin();
  mpu.begin();
  mpu.calcOffsets();

  // Start the servers
  server.begin();
  telnet.begin();


  // Set pins as output
  pinMode(motorHigh, OUTPUT);
  pinMode(motorLow, OUTPUT);
  pinMode(motor1Enable, OUTPUT);
  pinMode(motor2Enable, OUTPUT);
  pinMode(motor3Enable, OUTPUT);
  pinMode(motor4Enable, OUTPUT);

  // Set PWM pins
  ledcAttachPin(motor1Enable, 0);
  ledcAttachPin(motor2Enable, 1);
  ledcAttachPin(motor3Enable, 2);
  ledcAttachPin(motor4Enable, 3);
  ledcSetup(1, 1000, 8);
  ledcSetup(2, 1000, 8);
  ledcSetup(3, 1000, 8);
  ledcSetup(4, 1000, 8);
}

void processCommand(char command) {
  switch (command) {
    case 'w':
      forward += movementSpeed;
      delay(250);
      forward -= movementSpeed;
      client.println("Received command forwards");
      break;
    case 'a':
      left += movementSpeed;
      delay(250);
      left -= movementSpeed;
      client.println("Received command left");
      break;
    case 's':
      backward += movementSpeed;
      delay(250);
      backward -= movementSpeed;
      client.println("Received command backwards");
      break;
    case 'd':
      right += movementSpeed;
      delay(250);
      right -= movementSpeed;
      client.println("Received command right");
      break;
    case 'r':
      up += movementSpeed;
      delay(250);
      up -= movementSpeed;
      client.println("Received command up");
      break;
    case 'f':
      down += movementSpeed;
      delay(250);
      down -= movementSpeed;
      client.println("Received command down");
      break;
    default:
      break;
  }
}


void loop() {
  // OTA updates
  AsyncElegantOTA.loop();

  client = telnet.available();

  // Check for serial input
  if (client) {
    // Handle client commands
    while (client.connected()) {
      if (client.available()) {
        char command = client.read();
        // Handle the received command
        processCommand(command);
      }
    }
  }

  // Calculate motor speed

  mpu.update();
  float roll = mpu.getAngleY();
  float pitch = mpu.getAngleX();

  float pidPitch = kp * pitch + ki * integralPitch + kd * (pitch - lastErrorPitch);
  float pidRoll = kp * roll + ki * integralRoll + kd  * (roll - lastErrorRoll);

  integralPitch += pitch;
  integralRoll += roll;

  lastErrorPitch = pitch;
  lastErrorRoll = roll;

  motorSpeed1 = 100 + (int)pidPitch - (int)pidRoll + backward + right + up - down;
  motorSpeed2 = 100 - (int)pidPitch - (int)pidRoll + backward + left + up - down;
  motorSpeed3 = 100 - (int)pidPitch + (int)pidRoll + forward + right + up - down;
  motorSpeed4 = 100 + (int)pidPitch + (int)pidRoll + forward + left + up - down;

  motorSpeed1 = constrain(motorSpeed1, 0, 255);
  motorSpeed2 = constrain(motorSpeed2, 0, 255);
  motorSpeed3 = constrain(motorSpeed3, 0, 255);
  motorSpeed4 = constrain(motorSpeed4, 0, 255);
 
  // Set motor speeds
  ledcWrite(1, motorSpeed1);
  ledcWrite(2, motorSpeed2);
  ledcWrite(3, motorSpeed3);
  ledcWrite(4, motorSpeed4);
  digitalWrite(motorHigh, HIGH);
  digitalWrite(motorLow, LOW);
}
