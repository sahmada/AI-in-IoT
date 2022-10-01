#include <SPI.h>
#include <ThingsBoard.h>
#include <ESP8266WiFi.h>
//#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string.h>

#define SERIAL_DEBUG_BAUD 115200

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor_address; 
int total_devices;
float temperature[3];

#define WIFI_SSID1 "saa"
#define WIFI_PWD1 "********************"
#define THINGSBOARD_SERVER  "185.206.231.244"
#define TOKEN "Aa@1234567890"
#define TB_USER "DHTsensor"
#define TB_PWD "DHT01sensor"
#define DEV_ID "DHT22"


WiFiClient espClient;
int status = WL_IDLE_STATUS; // = 0

ThingsBoard tb(espClient);


void setup(){
  Serial.begin(SERIAL_DEBUG_BAUD);
  sensors.begin();
  total_devices = sensors.getDeviceCount();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(total_devices, DEC);
  Serial.println(" devices.");

  for(int i=0;i<total_devices; i++){
    if(sensors.getAddress(sensor_address, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");      
      printAddress(sensor_address);
      Serial.println();
    } else {
      Serial.print("Found device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check circuit connection!");
    }
  }
  Serial.print(F("Connecting to AP : "));
  Serial.println(WiFi.SSID());
  reconnect();
}

void loop(){
   if (!tb.connected())
  {
    // Connect to the ThingsBoard
    Serial.print(F("\nConnecting to: "));
    Serial.println(THINGSBOARD_SERVER);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN))
    {
      Serial.println(F("Failed to connect"));
      return;
    }
  }

  
  sensors.requestTemperatures();  
  for(int i=0;i<total_devices; i++){    
    if(sensors.getAddress(sensor_address, i)){      
      Serial.print("Temperature for device: ");
      Serial.print(i,DEC);
      temperature[i] = sensors.getTempCByIndex(i);
      
      //float temperature_degreeCelsius = sensors.getTempC(sensor_address);
      Serial.print("\t");
      Serial.print(temperature[i]);
      Serial.println(" °C");      
    }
  }

  char buffer[50]={0};
  sprintf(buffer, "{\"temp0\":%.2f,\"temp1\":%.2f,\"temp2\":%.2f}", temperature[0],temperature[1],temperature[2]);
  Serial.println(buffer);
  tb.sendTelemetryJson(buffer);
  tb.loop();

}





void reconnect(){
  // Loop until we're reconnected
  status = WiFi.status();
  if (status != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID1, WIFI_PWD1);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.print("\nAcquired IP : ");
    Serial.println(WiFi.localIP());
  }
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    
  }
}
