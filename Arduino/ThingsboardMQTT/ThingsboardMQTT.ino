
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "SSD1306.h"

#define  latitude "29.611328"
#define  longitude "52.546479"

#define WIFI_AP "itpro"
#define WIFI_PASSWORD "12345678"

#define TOKEN "12345678"

// DHT
#define DHTPIN D3
#define DHTTYPE DHT22

int localHum = 0;
int localTemp = 0;
int thingtemp = 0 ;
int thinghum = 0 ;
int analog_sensor=0;

String payload;
char thingsboardServer[] = "192.168.1.20";

WiFiClient wifiClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

SSD1306  display(0x3c, 4, 5); //define Instance for  OLED1306 Library

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
   display.init();
   display.flipScreenVertically();
  Serial.begin(115200);
  dht.begin();
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
   getDHT();
   analog_sensor=analogRead(A0);
   make_json_send();
    lastSend = millis();
  }
     display.clear();
     drawDHT();
     display.display();
  client.loop();
}


 
void getDHT()  
{
    thingtemp = localTemp;
   thinghum = localHum;
  localTemp = dht.readTemperature(); //READ DHT22 Tepmerature
  localHum = dht.readHumidity();   //READ DHT22 Humidity
   if (isnan(localHum) || isnan(localTemp) ) {
     localTemp=thingtemp;
    localHum=thinghum;
   return;
}
if (localHum >99 || localTemp>99){
   localTemp=thingtemp;
    localHum=thinghum;
    return;
}
}
void drawDHT() 
{
  int x=0;
  int y=0;
  
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(43 + x, y, "DHT22");

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0 + x, 5 + y, "Hum");
   
  display.setFont(ArialMT_Plain_24);
  String humdht22 = String(localHum);
  display.drawString(0 + x, 15 + y, humdht22 + "%");
  

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(95 + x, 5 + y, "Temp");

  display.setFont(ArialMT_Plain_24);
  String tempdht22 = String(localTemp) ;
  display.drawString(70 + x, 15 + y, tempdht22+ "°C");
  display.setFont(ArialMT_Plain_10);

  display.setFont(ArialMT_Plain_10);
  display.drawString(34 + x, 37+y, "Analog_Sensor");
  display.setFont(ArialMT_Plain_16);
  display.drawString(45,48,String(analog_sensor) );
}



void make_json_send(){
  DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
  json["temperature"] = localTemp;
  json["humidity"] = localHum;
  json["Analog"] = analog_sensor;
  json["latitude"] = latitude;
  json["longitude"] = longitude;
  json.printTo(payload);
  
  // Send payload
  client.publish( "v1/devices/me/telemetry", payload.c_str());
  Serial.println( payload );
  payload="";

}


void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to Thingsboard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
