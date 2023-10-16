#include <Arduino.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>


#define RXD2 18 // uart1 pins for locker control
#define TXD2 17

void connectmqtt();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void make_message(byte* payload);

const char* mqtt_server = "172.16.27.83"; //mqtt server
const char* ssid = "LIIS";
const char* password = "qw8J*883";
const char* control_topic = "locker/control"; // to subscribe
const char* feedback_topic = "locker/locker_status"; // to publish

byte message [] = {0x8A, 0x01, 0x18, 0x11, 0x9B};
byte recieved_data [5];
byte check_sum;
WiFiClient espClient;
PubSubClient client(espClient); //lib required for mqtt

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  WiFi.begin(ssid, password);
  Serial.println("connected");
  client.setServer(mqtt_server, 1883);//connecting to mqtt server
  client.setCallback(callback);
  delay(3000);
  connectmqtt();
}

void callback(char* topic, byte* payload, unsigned int length) {   //callback includes topic and payload ( from which (topic) the payload is comming)
  /*  // DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print(0x0 + char(payload[i]));
    //Serial.print(' ');
  }
  Serial.println();
  */  // DEBUG

  make_message(payload);
  Serial1.write(message, sizeof(message));
  Serial1.read(recieved_data, 5);
  if(recieved_data[3] == 0x11)  // check is successful unlocking
  {  
    client.publish(feedback_topic, "open");
  }
  else 
  {
    client.publish(feedback_topic, "closed");
  }
  
}
  

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32_clientID")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Nodemcu connected to MQTT");
      // ... and resubscribe
       client.subscribe(control_topic);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}

void connectmqtt()
{
  client.connect("ESP32_clientID");  // ESP will connect to mqtt broker with clientID
  {
    Serial.println("connected to MQTT");
    // Once connected, publish an announcement...

    // ... and resubscribe
    client.subscribe(control_topic); 
    client.publish("outTopic",  "connected to MQTT");
    client.publish("locker/locker_status", "check");

    if (!client.connected())
    {
      reconnect();
    }
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
}


void make_message(byte* payload){
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

  message[1] = plate_addr;
  message[2] = lock_addr;
  message[4] = message[0] ^ message[1] ^ message[2] ^ message[3];
}

/*
byte message_unlock [] = {0x8A, 0x01, 0x01, 0x11, 0x9B};
byte recieved_data [5];


void loop() {                     
  Serial1.write(message_unlock, sizeof(message_unlock));
  Serial1.read(recieved_data, 5);


  if(recieved_data[3] == 0x11){  // successful unlocking
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
  
  delay(1000); 
               
}


void callback(char* topic, byte* payload, unsigned int length) {   //callback includes topic and payload ( from which (topic) the payload is comming)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == 'O' && (char)payload[1] == 'N') //on
  {
    digitalWrite(LED, HIGH);
    Serial.println("on");
    client.publish("outTopic", "LED turned ON");
  }
  else if ((char)payload[0] == 'O' && (char)payload[1] == 'F' && (char)payload[2] == 'F') //off
  {
    digitalWrite(LED, LOW);
    Serial.println(" off");
    client.publish("outTopic", "LED turned OFF");
  }
  Serial.println();
}

*/
