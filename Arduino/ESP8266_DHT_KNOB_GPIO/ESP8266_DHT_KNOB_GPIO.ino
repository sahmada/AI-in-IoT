#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
//#include "SSD1306.h"
#include <DHT.h>

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SERIAL_DEBUG_BAUD 115200

#define WIFI_AP "saa"
#define WIFI_PASSWORD "********************"

#define TOKEN "DHT11_DEMO_TOKEN"
#define THINGSBOARD_SERVER  "185.206.231.244"
#define TB_USER "DHTsensor"
#define TB_PWD "DHT01sensor"
#define CLIENT_ID "DHT22"

#define GPIO16 16
#define GPIO2 2


String termostat="50.00";

//char thingsboardServer[] = "192.168.1.20";

//SSD1306  display(0x3c, 4, 5); //define Instance for  OLED1306 Library

WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;

void setup() {
   //display.init();
   //display.flipScreenVertically();
  Serial.begin(SERIAL_DEBUG_BAUD);
  // Set output mode for all GPIO pins
  pinMode(GPIO16, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  dht.begin();
  delay(10);
  InitWiFi();
  client.setServer( THINGSBOARD_SERVER, 1883 );
  client.setCallback(on_message);
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }

  client.loop();
}

// The callback for when a PUBLISH message is received from the server.
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("On message");
////////////////////////////////
  char data[length+1];
  strncpy (data, (char*)payload, length);  //convert byte to char array
  data[length]='\0';
  //////////////////////////////
  
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(data);

  // Decode JSON request
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  

  if (!json.success())
  {
    Serial.println("parseObject() failed");
    return;
  }

/////////////////////////////////////////////// convert request topic to Response topic  v1/devices/me/rpc/request/ID  to   v1/devices/me/rpc/response/ID
 String responseTopic = String(topic);
    responseTopic.replace("request", "response"); 

     ////////////////////////////////////////////// // Check request method
  String methodName = json["method"].as<String>();
  
  if (methodName=="checkStatus") {
  
  client.publish(responseTopic.c_str(),get_gpio_status_and_Send().c_str()); ///important to publiash all GPIOs Status
    
  }else if(methodName=="SETGPIO16"){ 
                                              
    set_gpio_status(GPIO16, json["params"]);   // Update GPIO status 
  
   
    
  }else if(methodName=="GETGPIO16"){ /////send GPIO16 Status
    
    String status = digitalRead(16) ? "true":"false";
    client.publish(responseTopic.c_str(),status.c_str());

    
  }else if(methodName=="SETGPIO2"){   
  
    set_gpio_status(GPIO2, json["params"]);  // Update GPIO status 
    
    
    
  }else if(methodName=="GETGPIO2"){   ///send GPIO2 status
    
    String status = digitalRead(2) ? "true":"false";
    client.publish(responseTopic.c_str(),status.c_str());


    
  }else if(methodName=="SETtermostat"){
 
  termostat=json["params"].as<String>();
    //display.clear();
    //display.setFont(ArialMT_Plain_16);
  //display.drawString(24 , 0, "Termostat");
  //display.setFont(ArialMT_Plain_24);
  //display.drawString(33,30,termostat );
    //display.display();
    Serial.print("Thermostat knob value :");
    Serial.println(termostat);
  }else if(methodName=="GETtermostat"){
   
    client.publish(responseTopic.c_str(), termostat.c_str());
  }
}
///////////////////////////////////////////////////////////////////SEND All GPIO status
String get_gpio_status_and_Send() {
  // Prepare gpios JSON payload string
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  //--------------------------------GPIO16
  json[String(GPIO16)] = digitalRead(GPIO16) ? true:false ;
  //-----------------------------------GPIO2
 json[String(GPIO2)] = digitalRead(GPIO2) ? true:false ;
  
  String payload_str;
  json.printTo(payload_str);
  
  Serial.print("gpio status: ");
  Serial.println(payload_str);
  
  client.publish("v1/devices/me/attributes",payload_str.c_str());   //to set client side attributes for monitoring LED widgets 
  return  payload_str;
}
/////////////////////////////////////////////////////////////set GPIO status
void set_gpio_status(int pin, boolean enabled) {
  
    digitalWrite(pin, enabled ? LOW : HIGH);

  get_gpio_status_and_Send(); //sending status to show on leds
}

void InitWiFi() {
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
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(CLIENT_ID, TB_USER, TB_PWD) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");  /////////////////////Telling server we are ready to receive RPC requests
    get_gpio_status_and_Send(); //sending GPIO Last Status
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

void getDHT()  
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float temperatureFahrenheit = dht.readTemperature(true);
  if (isnan(humidity) || isnan(temperature) || isnan(temperatureFahrenheit))
  {
    Serial.println(F(" Failed to read from DHT"));
    return;
  }
}
