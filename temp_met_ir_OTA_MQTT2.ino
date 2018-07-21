// set the dependencies

#include "credentials.h"
#include <ArduinoJson.h>
#include <SparkFunMLX90614.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#define NTP_OFFSET 2 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "ntp2.xs4all.nl"

//object for Sensor
IRTherm sensor;
 
//variables for temperatures
float tempAmbient;
float tempObject;
float lastTemp = 0;
const char* ssid     = my_SSID;
const char* password = my_PASSWORD;
const char* mqttServer = ip_MQTT;
const int mqttPort = port_MQTT;
const char* mqttUser = my_MQTT_USER;
const char* mqttPassword = my_MQTT_PW;
float idx = 219;    // idx for temperature widget domoticz

bool justOnce = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
WiFiServer server(80);

int counter = 5;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial started");
  
  sensor.begin(0x00);
  sensor.setUnit(TEMP_C);       //Set temperature for Celsius
  
  // Start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // Set up Over The Air updates
  ArduinoOTA.setHostname("IR_Temperatuur");
  ArduinoOTA.setPassword(my_OTA_PW);
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  // Show WiFi is connected
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Enter this address in your Internet browser.");
 
  server.begin();
  timeClient.begin();
}


void loop() {
  
  ArduinoOTA.handle();
  // update the time
  timeClient.update();
  
  WiFiClient espClient = server.available();        // luisteren naar inkomende gebruiker
  PubSubClient client(espClient);
  
  // at 7 minutes before the hour restart the ESP  
  String formattedTime = timeClient.getFormattedTime();
  if (formattedTime.substring(3,5).toInt() % 53 == 0 && 
      formattedTime.substring(6,8).toInt()<3 ){
      ESP.restart();
  }
  // here the temperature proccesing and publishing starts
  if (sensor.read()){
      //if sensor has been read get the ambient temperature (the sensor itself)
      tempAmbient = sensor.ambient();
      //get the temperature of the object in view of sensor
      tempObject = sensor.object();
      if ((formattedTime.substring(3,5).toInt() % 10 == 0 && justOnce == false && 
              formattedTime.substring(6,8).toInt()< 3 ) ||
              abs(lastTemp - tempObject) > .2 ){
          justOnce = true;
       
          // MQTT only connect client when temperature has changed or 10 minutes
          client.setServer(mqttServer, mqttPort);
          while (!client.connected()) {
             Serial.println("Connecting to MQTT...");
             if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
                Serial.println("connected");
             } else {
               Serial.print("failed with state ");
               Serial.print(client.state());
               delay(2000);
             }
          }
          // make a json to send
          StaticJsonBuffer<300> JSONbuffer;
          JsonObject& JSONencoder = JSONbuffer.createObject();  
          // three parameters are needed for domoticz
          JSONencoder["command"] = "udevice";
          JSONencoder["idx"] = idx;
          JSONencoder["svalue"] = (String)+tempObject;
          
          char JSONmessageBuffer[100];
          JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
          if (client.publish("domoticz/in", JSONmessageBuffer) == true) {
             Serial.println("Success sending message by MQTT temperature set to: " + (String)+tempObject);
          } else {
             // it failed
             Serial.println("Error sending message through MQTT");
          }
          client.disconnect();
      } else {
         justOnce = false;     
      }
      lastTemp = tempObject;
  } else {
      Serial.println("Geen data");
  }
  // if the user uses browser (add a nice thermometer with fontawesome)
  if (espClient) {                             
    while (espClient.connected()) {            
      if (espClient.available()) {                 
        espClient.flush();
 String s = "<!DOCTYPE html><html><head><meta name='viewport' content='initial-scale=1.0'><meta charset='utf-8'><style>#map {height: 100%;}html, body {height: 100%;margin: 25px;padding: 10px;font-family: Sans-Serif;} p{font-family:'Courier New', Sans-Serif;}</style>";
        s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\">";
        s +="<script defer src=\"https://use.fontawesome.com/releases/v5.0.9/js/all.js\"></script></head>";
        s += "<body><h1>Temperature with IR sensor</h1>";
        s += "<p>Sensor temperature: "+(String)tempAmbient+"°C <i class=\"fas fa-thermometer-half\"></i><br>";
        s += "Object&nbsp;temperature: "+(String)tempObject+"°C <i class=\"fas fa-thermometer-half\"></i></p>";
        s += "</body></html>";
        espClient.print(s);
        delay(1);
        
      }
      break;
    }
  }    
   delay(1000);
}
