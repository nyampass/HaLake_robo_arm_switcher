#include <Arduino.h>
#include <Servo.h>
#include <vector>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "local_property.h"

// hand: 0-180 | open-close
// arm: 0-100 | up-down
// wrist: 115-180  | down-up

#define HAND_SERVO_PIN 16
#define ARM_SERVO_PIN 5
#define WRIST_SERVO_PIN 4

Servo HandServo; 
Servo WristServo; 
Servo ArmServo; 

ESP8266WebServer server(80); 

std::vector<String> split2vector(String target, char sep){
  String buff = "";
  std::vector<String> result;
  
  for(int i = 0; i < target.length(); i++){
    if(target[i] == sep){
      result.push_back(buff);
      buff = "";
    }else if(i == target.length() - 1){
      buff += target[i];
      result.push_back(buff);
    }
    else buff += target[i];
  }

  return result;
}

uint8_t changeHandServo(uint8_t percentage){ // 0-100 : close-open
  Serial.println(180 - (percentage * 1.8));
  if(percentage <= 100) return 180 - (percentage * 1.8);
}

uint8_t changeWristServo(uint8_t percentage){ // 0-100 : down-up
  Serial.println(map(percentage, 0, 100, 115, 180));
  if(percentage <= 100) return map(percentage, 0, 100, 115, 180);
}

uint8_t changeArmServo(uint8_t percentage){ // 0-100 : down-up
  Serial.println(100 - percentage);
  if(percentage <= 100) return 100 - percentage;
}

int16_t str2uint8(String target){
  long result = target.toInt();

  if(result > 255) return -1;
  if(target == "" || result > 0) return result;
  else -1;
}

void setup(){
  HandServo.attach(HAND_SERVO_PIN);
  WristServo.attach(WRIST_SERVO_PIN);
  ArmServo.attach(ARM_SERVO_PIN);

  HandServo.write(changeHandServo(30));
  WristServo.write(changeWristServo(0));
  ArmServo.write(changeArmServo(80));

  Serial.begin(115200);
  delay(500);

  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(SSID, PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  } 

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("No Connection");
    ESP.restart();
  }
  else Serial.println(WiFi.localIP());

  server.on("/hand", [](){
    if(server.args() == 1){
      if(server.argName(0) == "percentage"){
        int16_t angle = str2uint8(server.arg(0));

        if(angle >= 0){
          HandServo.write(changeHandServo(angle));
          server.send(200, "text/html", "Hand was changed.");
        }
      }
    }

    server.send(200, "text/html", "Failed.");
  }); 

  server.on("/wrist", [](){
    if(server.args() == 1){
      if(server.argName(0) == "percentage"){
        int16_t angle = str2uint8(server.arg(0));

        if(angle >= 0){
          WristServo.write(changeWristServo(angle));
          server.send(200, "text/html", "Wrist was changed.");
        }
      }
    }
    server.send(200, "text/html", "Failed.");
  });

  server.on("/arm", [](){
    if(server.args() == 1){
      if(server.argName(0) == "percentage"){
        int16_t angle = str2uint8(server.arg(0));

        if(angle >= 0){
          ArmServo.write(changeArmServo(angle));
          server.send(200, "text/html", "Arm was changed.");
        }
      }
    }

    server.send(200, "text/html", "Failed.");
  });

  server.begin();
}

void loop(){
  String line = Serial.readStringUntil('\r');

  if(line != ""){
    std::vector<String> splitLine = split2vector(line, ':');

    if(splitLine.size() == 2){
      String which = splitLine[0];
      int16_t angle = str2uint8(splitLine[1]);

      if(angle >= 0 && which != ""){
        if(which == "h") HandServo.write(changeHandServo(angle));
        else if(which == "w") WristServo.write(changeWristServo(angle));
        else if(which == "a") ArmServo.write(changeArmServo(angle));
      }
    }
  }
  server.handleClient();
 }