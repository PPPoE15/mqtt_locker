#ifndef MQTT_PROTO_H
#define MQTT_PROTO_H

#include <PubSubClient.h>
#include "Config.h"
WiFiClient LockerClient;
PubSubClient mqttClient(LockerClient);

const char* control_topic = "locker/control"; // to subscribe
const char* feedback_topic = "locker/locker_status"; // to publish

void reconnect() {
  Serial.println("Enter reconnect");
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect("LockerController")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("locker/outTopic", "Nodemcu connected to MQTT");
      // ... and resubscribe
      mqttClient.subscribe(control_topic);

    } else {
      //Serial.print("failed, rc=");
      //Serial.println(" try again in 1 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}

void connectmqtt()
{
  mqttClient.connect("LockerController");  // ESP will connect to mqtt broker with clientID
  {
    Serial.println("connected to MQTT");
    // Once connected, publish an announcement...

    // ... and resubscribe
    mqttClient.subscribe(control_topic); 
    mqttClient.publish("locker/outTopic",  "connected to MQTT");
    mqttClient.publish("locker/locker_status", "check");

    if (!mqttClient.connected())
    {
      Serial.println("Reconecting...");
      reconnect();
    }
  }
}

void messageDecoder(const byte* payload, byte* feedback){
  byte plate_addr;
  byte lock_addr;

  if (payload[1] == ';')  // determinating plate addres
  {  
    plate_addr = payload[0] - 0x30;
    if (payload[3] == ';')  // determinating locker addres
    { 
    lock_addr = payload[2] - 0x30;
    } else{
      lock_addr = payload[3] - 0x30 + (payload[2] - 0x30) * 10;
    }
  } 
  else 
  {
    plate_addr = payload[1] - 0x30 + (payload[0] - 0x30) * 10;
    if (payload[4] == ';')  // determinating locker addres
    { 
    lock_addr = payload[3] - 0x30;
    } 
    else
    {
     lock_addr = payload[4] - 0x30 + (payload[3] - 0x30) * 10;
    }
  }

  Message mqttMessage("open", plate_addr, lock_addr, feedback);
}

void callback(char* topic, byte* payload, unsigned int lenght) {   //callback includes topic and payload ( from which (topic) the payload is comming)
  byte feedback[5];
  messageDecoder(payload, feedback);

  if(feedback[3] == 0x11)  // check is successful unlocking
  {  
    mqttClient.publish(feedback_topic, "open");
  }
  else 
  {
    mqttClient.publish(feedback_topic, "closed");
  } 
}

void initMQTT(char* serverIP, uint16_t serverPort){
  Serial.println("trying to connect to MQTT");
  Serial.println(serverIP);
  Serial.println(serverPort);
  mqttClient.setServer(serverIP, serverPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connect("LockerController")) {
    Serial.print(".");
    delay(500);
  }
  delay(500);
  connectmqtt();
}

#endif // MQTT_PROTO_h