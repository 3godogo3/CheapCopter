#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <MPU6050_light.h>
#include "Wire.h"

//Wi-Fi AP credentials
const char *ssid = "Drone";
const char *password = "password";

AsyncWebServer server(80);

WiFiClient client;

//motor pins
bool status = false;
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

void handleCommand(AsyncWebServerRequest *request) {
  if (request->hasParam("command")) {
    String commandStr = request->getParam("command")->value();
    
    char command = commandStr[0];

    // Perform actions based on the received command
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
      case '1':
        status = true;
        client.println("Recieved command turn on");
        break;
      case '0':
        status = false;
        client.print("Recieved command turn off");
        break;
      default:
        break;
    }

    request->send(200, "text/plain", "Command received: " + command);
  } else {
    request->send(400, "text/plain", "Missing command parameter");
  }
}

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

  const char* htmlContent = R"html(
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controller</title>
    <link href="https://fonts.googleapis.com/css2?family=Mitr:wght@500&display=swap" rel="stylesheet">
    <style>
        body {
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            background-color: #484f4f;
            height: 100vh;
        }

        .main{
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            width: 400px;
            height: 700px;
            background-color: #1a1a1a;
            border: 3px solid #618685;
            border-radius: 10px;
        }

        h1 {
            margin: 20px 0 70px;
            color: #efe49f;
            font-family: 'Mitr', sans-serif;
        }

        .button-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 7px;
        }

        .button {
            display: flex;
            background-color: #007bff;
            cursor: pointer;
            height: 100px;
            width: 100px;
            justify-content: center;
            align-items: center;
            border-radius: 10px;
            color: #fff;
            font-family: 'Mitr', sans-serif;
            font-size: 15px;
        }

        /* Set position for the last button */
        .button:nth-child(8) {
            grid-row: 3;
            grid-column: 3;
        }

        .upload-button {
            margin: 100px 0 5px;
            background-color: #007bff;
            color: #fff;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            text-decoration: none;
            font-family: 'Mitr', sans-serif;
        }
    </style>
</head>
<body>
    <div class="main">
        <h1>Drone Controller</h1>
        <div class="button-grid">
            <div class="button" onclick="sendCommand('1')">ON</div>
            <div class="button" onclick="sendCommand('w')">FORWARDS</div>
            <div class="button" onclick="sendCommand('0')">OFF</div>
            <div class="button" onclick="sendCommand('a')">LEFT</div>
            <div class="button" onclick="sendCommand('s')">BACKWARDS</div>
            <div class="button" onclick="sendCommand('d')">RIGHT</div>
            <div class="button" onclick="sendCommand('r')">UP</div>
            <div class="button" onclick="sendCommand('f')">DOWN</div>
        </div>
        <a href="http://192.168.4.1/update" class="upload-button">Upload Firmware</a>
        <script>
            function sendCommand(command) {
                var request = new XMLHttpRequest();
                request.open("GET", "/send_command?command=" + command, true);
                request.onreadystatechange = function () {
                    if (request.readyState == 4 && request.status == 200) {
                        console.log("Command sent successfully.");
                    }
                };
                request.send();
            }
        </script>
    </div>
</body>
</html>

  )html";

  server.on("/", HTTP_GET, [htmlContent](AsyncWebServerRequest *request){
    request->send(200, "text/html", htmlContent);
  });

  server.on("/send_command", HTTP_GET, handleCommand);

  // Start the servers

  server.begin();

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

void loop() {
  // OTA updates
  AsyncElegantOTA.loop();

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
  if(status){
    ledcWrite(1, motorSpeed1);
    ledcWrite(2, motorSpeed2);
    ledcWrite(3, motorSpeed3);
    ledcWrite(4, motorSpeed4);
    digitalWrite(motorHigh, HIGH);
    digitalWrite(motorLow, LOW);
  }
}
