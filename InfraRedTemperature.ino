/* Sensitive temperature sensor to use with domoticz
 *  on DOIT ESP32 DEVKIT V1 80Mhz, 921600 https://www.banggood.com/ESP32-Development-Board-WiFiBluetooth-Ultra-Low-Power-Consumption-Dual-Cores-ESP-32-ESP-32S-Board-p-1109512.html?p=VQ141018240205201801
 *  and MLX 90614   https://www.banggood.com/nl/MLX90614ESF-AAA-Non-contact-Human-Body-Infrared-IR-Temperature-Sensor-Module-For-Arduino-p-1100990.html?p=VQ141018240205201801
 *  MLX 90614   ESP32
 *  SDA      ->   D21
 *  SCL      ->   D23
 *  Vin      ->   3v3
 *  Gnd      ->   Gnd
 */
// Needed libraries
#include <Wire.h>  
#include <SparkFunMLX90614.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
// setup NTP timeserver
#define NTP_OFFSET 2 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "ntp2.xs4all.nl"

//variables or temperatures
float tempSensor;
float tempObject;
float lastTemp = 0;
// WiFi network settings
const char* ssid     = "SSID OF YOUR WIFI";
const char* password = "PASSWORD FOR YOUR WIFI";
/** username and password for Domoticz
 *  use http://codebeautify.org/base64-encode to encode both
 */
String user ="ENCODED_USERNAME";
String pw="ENCODED_PASSWORD";
/** IP and Port to Domoticz */
String domoticzIP="IP_ADDRESS";
String domoticzPort="PORT";
// The Idx of the domoticz device used string to easy adding to getstring
String domoticzIdx ="IDX_SEE_DEVICES_IN_DOMOTICZ";

//object for Sensor
IRTherm sensor;
// object for timeserver update
WiFiUDP ntpUDP;
// object for timeserver client
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
// object for server
WiFiServer server(80);

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP32 has started");
  //start sensor 
  sensor.begin(0x00);
  //Select to use temperature in Celsius Change to TEMP_F for Fahrenheit
  sensor.setUnit(TEMP_C);
  // start WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Enter this address in your Internet browser.");
 
  // start Server
  server.begin();
  // start Time client
  timeClient.begin();
}


void loop() {
  if (sensor.read()){
      timeClient.update();
      String formattedTime = timeClient.getFormattedTime();
      //retrieve the temperature of the sensor (ambient)
      tempSensor = sensor.ambient();
      //retrieve the temperature of the object (Field of View)
      tempObject = sensor.object();
      // Only send the new temperature changed by 1 degrees of on the 20th seconds in time once a minute
      if (formattedTime.substring(6,8)=="20" || abs(lastTemp - tempObject) > .99 ){
         HTTPClient http;
         http.begin("http://" + domoticzIP + ":" + domoticzPort + "/json.htm?username=" + user + "&password=" + pw + "&type=command&param=udevice&idx=" + domoticzIdx + "&nvalue=0&svalue="+(String)+tempObject);
         int httpCode = http.GET();        //Make the request
         if (httpCode > 0) {               //Check for the returning code
           String payload = http.getString();
           Serial.println(httpCode);
           Serial.println(payload);
         }  else {
           Serial.println("Error on HTTP request");
         }
         http.end();
      }      
      lastTemp = tempObject;
      Serial.print(formattedTime + " temperature Sensor: "+(String)tempSensor+"°C");
      Serial.println(" Object: "+(String)tempObject+"°C");
  } else {
      Serial.println("Geen data");
  }
  /* ONLY NEEDED IF YOU LIKE A WEBINTERFACE */
  WiFiClient client = server.available();        // luisteren naar inkomende gebruiker
  if (client) {                             
    while (client.connected()) {            
      if (client.available()) {                 // als de gebruiker iets stuurt
        timeClient.update();
        String formattedTime = timeClient.getFormattedTime();
        client.flush();
 String s = "<!DOCTYPE html><html><head><meta name='viewport' content='initial-scale=1.0'><meta charset='utf-8'><style>#map {height: 100%;}html, body {height: 100%;margin: 25px;padding: 10px;font-family: Sans-Serif;} p{font-family:'Courier New', Sans-Serif;}</style>";
        s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\">";
        s +="<script defer src=\"https://use.fontawesome.com/releases/v5.0.9/js/all.js\"></script></head>";
        s += "<body><h1>Temperature with IR sensor</h1>";
        s += "<p>Sensor temperature: "+(String)tempSensor+"°C</p>";
        s += "<p>Object temperature: "+(String)tempObject+"°C</p>";
        s += "</body></html>";
        client.print(s);
        delay(1);
      }
      break;
    }
  }    
  /* END OF WEBINTERFACE */ 
   delay(1000);
}
