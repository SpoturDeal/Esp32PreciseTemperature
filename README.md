# Esp32PreciseTemperatue
A precise temperature sensor setup for Domoticz

## Built a Precise thermometer to use with Domoticz
Most temperature sensors are based on an DS18B20 sensors, the disadvantage of this sensor is that is heats up itself. This reults in a higher temperature.
By using an InfraRed sensor you are able the measure the temperature in the field of view of the sensor. (you sensor could be 40C (104F) in the sun, but in field of view the temperature is on 21.2C (70C)
If you are using the sensor for controlling something else the precise temperute could benefit

## Materials to use
Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3 <a href="https://www.banggood.com/ESP32-Development-Board-WiFiBluetooth-Ultra-Low-Power-Consumption-Dual-Cores-ESP-32-ESP-32S-Board-p-1109512.html?p=VQ141018240205201801">Available at Banggood</a> (about EUR 5.50)

Sensor: MLX90614ESF Infra red non-touch sensor <a href="https://www.banggood.com/nl/MLX90614ESF-AAA-Non-contact-Human-Body-Infrared-IR-Temperature-Sensor-Module-For-Arduino-p-1100990.html?p=VQ141018240205201801">Available at Banggood</a> (about EUR 10.00)

## Wired connections

|From|-|To|
|---|--|---|
|**MLX 90614**|--|**ESP32**|
|SDA|->|D21|
|SCL|->|D23|
|Vin|->|3v3|
|GND|->|GND|

## Set up Domoticz
### Add Hardware (if needed)
Click Setup -> Hardware

set Name ="Virtual Sensor"

set Type ="Dummy (Does nothing, for virtual device only)"

set Data Time ="Disabled"

Click Add


Click Create Virtual Sensor

Type a name "whatever you like"

Set Sensortype to "Temperature"

### Find Idx of device in Devices
Click Setup -> Devices

Click Idx(the top row of the label so it sort the high to lowest)

write down the Idx number 

## Needed settings
To use the Domoticz API you must **base64 encode** your username and password see the link below

```
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
```
## Project Photo
#### Sensor: Actual temperature of the sensor, object is the temperature the sensor faces in this case my room
![Monitor](/Monitor.png?raw=true "Serial Monitor output") 


