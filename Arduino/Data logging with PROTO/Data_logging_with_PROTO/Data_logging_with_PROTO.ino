/*
   Industruino PROTO sketch for MAYA's chemistry experiments

   Hardware:
   > PROTO with D21G topboard
   > DS18B20 temperature sensors (2) both on pin 9 with one 10K pull-up resistor, powered from 5V
   > pH sensor on analog pin A8 https://www.dfrobot.com/product-1782.html output 0-3V
     https://wiki.dfrobot.com/Gravity__Analog_pH_Sensor_Meter_Kit_V2_SKU_SEN0161-V2
   > WIFI module

   IoT platform:
   > Thingsboard CE running on a cloud server, see https://thingsboard.io/
   
   Fuctionality:
   > retrieve pH calibration values from EEPROM
   > connect to wifi network and mqtt server
   > at interval, read sensors and display on LCD
   > at interval, send data over mqtt
   > ENTER button resets the device
   > DOWN button enters interactive pH sensor calibration

   Tom Tobback, March 2022
   
 *******************************************************************************************************************************************************
    MIT LICENSE
    Copyright (c) 2019-2022 Cassiopeia Ltd
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
 *******************************************************************************************************************************************************   
*/

#define VERSION "v0.3"
#define SEND_INTERVAL_MIN 1     // for mqtt
#define SENSOR_INTERVAL_SEC 1   // it takes around 700ms to read the 2 temp sensors
#define time_zone 8             // to display local time
#define ANALOG_SAMPLES 5        // for pH sensor averaging

// Industruino LCD
#include <UC1701.h>
static UC1701 lcd;
#define LCD_PIN 26
// A custom "degrees" symbol...
static const byte DEGREES_CHAR = 1;
static const byte degrees_glyph[] = { 0x00, 0x07, 0x05, 0x07, 0x00 };
#define ENTER_PIN 24
#define DOWN_PIN 23
#define UP_PIN 25

// EEPROM for PH sensor calibration at PH 4 and 7
#include <I2C_eeprom.h>
#define EEPROM_SIZE 255
I2C_eeprom eeprom50(0x50, EEPROM_SIZE);   // use only 1 address with 255 bytes; more available at 0x51, 0x52, 0x53 if needed
const byte EEPROM_PH4 = 4;        // float for analog mV PH 4
const byte EEPROM_PH7 = 8;        // float for analog mV PH 7
float calib_ph4_mV, calib_ph7_mV; // read from EEPROM

// Temperature sensors
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(9);
DallasTemperature sensors(&oneWire);
int deviceCount = 0;

// Industruino WIFI module
#include <SPI.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>   // for module RED LED
#define ESP32_RGB_RED    26
#define SPIWIFI_SS       10
#define SPIWIFI_ACK       7
#define ESP32_RESETN      5
#define ESP32_GPIO0       -1  // was 6 but D6 needed for FRAM CS
#define SPIWIFI          SPI
const char ssid[] = "xxx";        // your network SSID (name)
const char pass[] = "xxx";    // your network password (use for WPA, or use as key for WEP)
WiFiSSLClient client;     // for client that always uses SSL

// MQTT
#include <PubSubClient.h>
PubSubClient mqtt_client(client);
const char server[] = "xxx";   // example test server, available both SSL and not
const int port = 8883;
const String mqtt_user = "xxx";  // this is Maya's device in solarmonitor.hk thingsboard
const String topic_attributes = "v1/devices/me/attributes";  // for thingsboard
const String topic_telemetry = "v1/devices/me/telemetry";  // for thingsboard

// WDT
#include <WDTZero.h>
WDTZero myWDT; // Define WDT

// Time
#include <TimeLib.h>

// global variables
unsigned long last_sensor_ts;
unsigned long last_send_ts;
float temp_in, temp_out, ph;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void setup() {

  pinMode(LCD_PIN, OUTPUT);
  digitalWrite(LCD_PIN, HIGH);
  lcd.begin();
  lcd.print("MAYA's sensors ");
  lcd.print(VERSION);
  lcd.createChar(DEGREES_CHAR, degrees_glyph); // degrees char for LCD

  SerialUSB.begin(115200);
  delay(2000); // give Serial Monitor time to catch up

  SerialUSB.println();
  SerialUSB.println("===============================");
  SerialUSB.print("===== MAYA's sensors ");
  SerialUSB.print(VERSION);
  SerialUSB.println(" =====");
  SerialUSB.println("===============================");

  // enable WDT
  myWDT.attachShutdown(WDTshutdown);
  myWDT.setup(WDT_SOFTCYCLE2M);  // initialize WDT-softcounter refesh cycle on 32sec interval WDT_SOFTCYCLE32S
  SerialUSB.println("[WDT] watchdog timer started, max 2 minutes");
  myWDT.clear();

  // intervals
  SerialUSB.print("sensor interval (sec): ");
  SerialUSB.println(SENSOR_INTERVAL_SEC);
  SerialUSB.print("send interval (min): ");
  SerialUSB.println(SEND_INTERVAL_MIN);

  // temperature sensors
  initDS18B20();

  // retrieve PH calibration values
  eeprom50.begin();
  //saveEEPROM(EEPROM_PH4, 2032.44);  // default
  //saveEEPROM(EEPROM_PH7, 1500.00);  // default
  calib_ph4_mV = readEEPROM(EEPROM_PH4);
  calib_ph7_mV = readEEPROM(EEPROM_PH7);
  SerialUSB.print("[EEPROM] calibration values: pH4 = ");
  SerialUSB.print(calib_ph4_mV);
  SerialUSB.print("mV and pH7 = ");
  SerialUSB.print(calib_ph7_mV);
  SerialUSB.println("mV");

  // connect to wifi
  initWifi();
  myWDT.clear();

  // connect mqtt
  mqttConnection();
  myWDT.clear();
  delay(2000);

  lcdMain();  // display fixed elements on LCD

  SerialUSB.println();

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void loop() {

  myWDT.clear();

  mqtt_client.loop();

  // shorter interval for sensor reading and LCD display
  if (millis() - last_sensor_ts > SENSOR_INTERVAL_SEC * 1000) {
    sensors.requestTemperatures();
    temp_in = sensors.getTempCByIndex(0);
    temp_out = sensors.getTempCByIndex(1);
    ph = getPH();
    SerialUSB.print("[SENSORS] temp in: ");
    SerialUSB.print(temp_in, 2);
    SerialUSB.print("degC\t temp out: ");
    SerialUSB.print(temp_out, 2);
    SerialUSB.print("degC\t");
    SerialUSB.print("pH: ");
    SerialUSB.println(ph, 1);
    lcd.setCursor(70, 2);
    lcd.print(temp_in);
    lcd.print("\001C   ");
    lcd.setCursor(70, 3);
    lcd.print(temp_out);
    lcd.print("\001C   ");
    lcd.setCursor(70, 5);
    lcd.print(ph, 1);
    lcd.print("    ");  // to clear any long numbers
    last_sensor_ts = millis();
  }

  // longer interval for data sending
  if (millis() - last_send_ts >  SEND_INTERVAL_MIN * 60 * 1000) {
    if (!mqtt_client.connected()) mqttConnection();
    // construct a data JSON 
    String payload = "{temp_in:";
    payload += String(temp_in, 2);
    payload += ",temp_out:";
    payload += String(temp_out, 2);
    payload += ",ph:";
    payload += String(ph, 1);
    payload += "}";
    lcd.setCursor(70, 7);
    if (mqtt_client.publish(topic_telemetry.c_str(), payload.c_str(), 0)) {
      SerialUSB.println("[MQTT] data sent OK");
      lcd.print("send OK");
    } else {
      SerialUSB.println("[MQTT] data send failed");
      lcd.print("send FAIL!");
    }
    last_send_ts = millis();
  }

  // press ENTER button to restart the device
  if (!digitalRead(ENTER_PIN)) {
    SerialUSB.println("ENTER PRESSED, reset..");
    delay(100);
    NVIC_SystemReset();      // processor software reset
  }

  // press DOWN button to start interactive pH sensor calibration
  if (!digitalRead(DOWN_PIN)) {
    SerialUSB.println("DOWN PRESSED, wait for long press..");
    delay(1000);
    if (!digitalRead(DOWN_PIN)) {
      SerialUSB.println("DOWN PRESSED long, enter calibration mode..");
      doCalibration();
      lcdMain();  // display fixed elements on LCD
    }
  }

  lcd.setCursor(0, 7);
  lcdprintLocalTime(now());

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////

void initWifi() {

  lcd.setCursor(0, 1);
  lcd.print("initWifi");

  // configure WIFI pins
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);   // specific to Industruino WIFI module
  // find wifi module, with timeout 5sec
  SerialUSB.print("[WIFI] connecting to wifi module..");
  unsigned long start_ts = millis();
  while (WiFi.status() == WL_NO_MODULE && millis() - start_ts < 5000) {
    SerialUSB.print(".");
    delay(500);
  }

  // check WIFI module status
  if (WiFi.status() != WL_NO_MODULE) {
    SerialUSB.println("found");
  } else {
    SerialUSB.println("NOT FOUND, stop here");
    lcd.setCursor(0, 2);
    lcd.print("no wifi module found");
    while (true); // stay here forever
  }

  // initiate the RED LED on WIFI module and blink it
  WiFiDrv::pinMode(ESP32_RGB_RED, OUTPUT);
  // blink RED LED
  for (int i = 0; i < 5; i++) {
    WiFiDrv::digitalWrite(ESP32_RGB_RED, HIGH);  // on
    delay(100);
    WiFiDrv::digitalWrite(ESP32_RGB_RED, LOW);  // off
    delay(100);
  }

  // check WIFI module firmware
  String fv = WiFi.firmwareVersion();
  SerialUSB.print("[WIFI] module firmware: ");
  SerialUSB.println(fv);
  lcd.setCursor(0, 2);
  lcd.print("firmware: ");
  lcd.print(fv);

  // attempt to connect to wifi network as defined above:
  lcd.setCursor(0, 3);
  lcd.print("SSID: ");
  lcd.print(ssid);
  SerialUSB.print("[WIFI] connecting to SSID: ");
  SerialUSB.println(ssid);
  lcd.setCursor(0, 4);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  int status = WL_IDLE_STATUS;
  bool led_status = false;
  do {
    WiFiDrv::digitalWrite(ESP32_RGB_RED, led_status);  // blink RED LED
    status = WiFi.begin(ssid, pass);
    lcd.print(".");
    SerialUSB.print(".");
    delay(500);
    led_status = !led_status;
  } while (status != WL_CONNECTED);
  SerialUSB.println();

  SerialUSB.print("[WIFI] connected to wifi network: ");
  SerialUSB.println(WiFi.SSID());  // just to double check it is the correct SSID
  lcd.print("OK");
  WiFiDrv::digitalWrite(ESP32_RGB_RED, LOW);   // RED LED off

  IPAddress ip = WiFi.localIP();
  SerialUSB.print("[WIFI] IP Address: ");
  SerialUSB.println(ip);
  lcd.setCursor(0, 5);
  lcd.print("IP:");
  lcd.print(ip);

  long rssi = WiFi.RSSI();
  SerialUSB.print("[WIFI] signal strength (RSSI): ");
  SerialUSB.print(rssi);
  SerialUSB.println("dBm");
  lcd.setCursor(0, 6);
  lcd.print("RSSI: ");
  lcd.print(rssi);
  lcd.print("dBm");

  // use NTP
  SerialUSB.println("[NTP] Use Wifi NTP");
  //SerialUSB.println(WiFi.getTime());
  //setTime(getWifiNtpTime());
  delay(3000);  // sometimes not enough
  setSyncProvider(getWifiNtpTime);
  setSyncInterval(5 * 60); // 5min between sync

}

//////////////////////////////////////////////////////////////////////////////////////////

void initDS18B20() {
  sensors.begin();
  SerialUSB.print("[TEMP] found ");
  deviceCount = sensors.getDeviceCount();
  SerialUSB.print(deviceCount);
  SerialUSB.print(" devices, ");
  if (deviceCount == 2) SerialUSB.println("OK");
  else {
    SerialUSB.println("but we expected 2 (in/out) - continue anyway");
  }
}
//////////////////////////////////////////////////////////////////////////////////////////

void WDTshutdown() {
  SerialUSB.println();
  SerialUSB.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  SerialUSB.println("+++  WDT shutdown! 2 minutes stuck somewhere.. +++++++++++++++++++++++");
  SerialUSB.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  SerialUSB.println();
  delay(100);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void mqttConnection() {

  SerialUSB.print("[MQTT] connecting to server.. ");
  lcd.setCursor(0, 7);
  lcd.print("connect MQTT.. ");
  mqtt_client.setServer(server, port);
  if (mqtt_client.connect("INDUSTRUINO", mqtt_user.c_str(), "")) {
    SerialUSB.println("connected");
    lcd.print("OK");
  } else {
    SerialUSB.print("MQTT failed, rc=");
    int mqtt_status = mqtt_client.state();
    SerialUSB.print(mqtt_status);
    if (mqtt_status == -2) SerialUSB.println(" connect fail");
    if (mqtt_status == -4) SerialUSB.println(" connect timeout");
    lcd.print("FAIL!");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void lcdprintLocalTime(time_t t) {
  // system time is GMT
  t += time_zone * 3600; // change to local time
  if (year(t) == 1970) {
    lcd.print("time n/a");
  } else {
    if (hour(t) < 10) lcd.print("0");
    lcd.print(hour(t));
    lcd.print(":");
    if (minute(t) < 10) lcd.print("0");
    lcd.print(minute(t));
    lcd.print(":");
    if (second(t) < 10) lcd.print("0");
    lcd.print(second(t));
  }
  /*  // time only, not date
    lcd.print(" ");
    lcd.print(day(t));
    lcd.print(" / ");
    lcd.print(month(t));
    lcd.print(" / ");
    lcd.print(year(t));
    lcd.print();
  */
}

///////////////////////////////////////////////////////////////////////////////////////////

time_t getWifiNtpTime() {
  myWDT.clear();
  return WiFi.getTime(); // return 0 if unable to get the time
}

///////////////////////////////////////////////////////////////////////////////////////////

void saveEEPROM(byte this_address, float this_value) {
  // use a union to convert float to 4 bytes
  union f_4b {
    float number;
    byte bytes[4];
  } this_union;
  this_union.number = this_value;

  eeprom50.writeByte(this_address, this_union.bytes[0]);
  delay(5);
  eeprom50.writeByte(this_address + 1, this_union.bytes[1]);
  delay(5);
  eeprom50.writeByte(this_address + 2, this_union.bytes[2]);
  delay(5);
  eeprom50.writeByte(this_address + 3, this_union.bytes[3]);
  delay(5);
  SerialUSB.print("[EEPROM] saved number at address ");
  SerialUSB.println(this_address);
}

///////////////////////////////////////////////////////////////////////////////////////////

float readEEPROM(byte this_address) {
  // use a union to convert float to 4 bytes
  union f_4b {
    float number;
    byte bytes[4];
  } this_union;

  this_union.bytes[0] = eeprom50.readByte(this_address);
  this_union.bytes[1] = eeprom50.readByte(this_address + 1);
  this_union.bytes[2] = eeprom50.readByte(this_address + 2);
  this_union.bytes[3] = eeprom50.readByte(this_address + 3);

  SerialUSB.print("[EEPROM] retrieved number from address ");
  SerialUSB.println(this_address);

  return this_union.number;
}

///////////////////////////////////////////////////////////////////////////////////////////

float getPH() {
  float analog_mV_avg = getAnalogVoltage();
  float this_ph = (analog_mV_avg - calib_ph4_mV) * (7.0 - 4.0) / (calib_ph7_mV - calib_ph4_mV) + 4.0;
  if (this_ph > 14 || this_ph < 0) return -999;
  else return this_ph;
}

///////////////////////////////////////////////////////////////////////////////////////////

float getAnalogVoltage() {
  float analog_mV_cum = 0;
  for (int i = 0; i < ANALOG_SAMPLES; i++) {
    float this_reading = analogRead(A8);
    //SerialUSB.println(this_reading, 1);
    analog_mV_cum += this_reading * 3300.0 / 1024.0;
    delay(10);
  }
  float analog_mV_avg = analog_mV_cum / ANALOG_SAMPLES - 40.0;  // it seems there is an offset of 40mV
  SerialUSB.print("avg mV: ");
  SerialUSB.println(analog_mV_avg);
  return analog_mV_avg;
}

///////////////////////////////////////////////////////////////////////////////////////////

void lcdMain() {
  lcd.clear();
  lcd.print("MAYA's sensors ");
  lcd.print(VERSION);
  lcd.setCursor(0, 2);
  lcd.print("temp in:");
  lcd.setCursor(0, 3);
  lcd.print("temp out:");
  lcd.setCursor(0, 5);
  lcd.print("pH:");
}

///////////////////////////////////////////////////////////////////////////////////////////

void doCalibration() {
  float this_mV;
  SerialUSB.println("[CAL] start calibration");
  lcd.clear();
  lcd.print("PH calibration");
  SerialUSB.println("[CAL] start with pH 4.0");
  lcd.setCursor(0, 2);
  lcd.print("sensor in ph 4.0");
  lcd.setCursor(0, 7);
  lcd.print("press ENTER to save");
  lcd.setCursor(0, 4);
  lcd.print("mV:");
  while (digitalRead(ENTER_PIN)) { // as long as no ENTER press
    myWDT.clear();
    this_mV = getAnalogVoltage();
    lcd.setCursor(70, 4);
    lcd.print(this_mV, 1);
    lcd.print("    ");  // to clear any long numbers
    //SerialUSB.print("[CAL] mV: ");
    //SerialUSB.println(this_mV);
    delay(200);
  }
  // blink LCD backlight
  digitalWrite(LCD_PIN, LOW);
  delay(100);
  digitalWrite(LCD_PIN, HIGH);
  SerialUSB.print("[CAL] saving calibration value for pH 4.0: ");
  SerialUSB.println(this_mV);
  saveEEPROM(EEPROM_PH4, this_mV);

  while (!digitalRead(ENTER_PIN));  // wait for ENTER press release

  SerialUSB.println("[CAL] change to pH 7.0");
  lcd.setCursor(0, 2);
  lcd.print("sensor in ph 7.0");
  lcd.setCursor(0, 7);
  lcd.print("press ENTER to save");
  lcd.setCursor(0, 4);
  lcd.print("mV:");
  while (digitalRead(ENTER_PIN)) { // as long as no ENTER press
    myWDT.clear();
    this_mV = getAnalogVoltage();
    lcd.setCursor(70, 4);
    lcd.print(this_mV, 1);
    lcd.print("    ");  // to clear any long numbers
    //SerialUSB.print("[CAL] mV: ");
    //SerialUSB.println(this_mV);
    delay(200);
  }
  // blink LCD backlight
  digitalWrite(LCD_PIN, LOW);
  delay(100);
  digitalWrite(LCD_PIN, HIGH);
  SerialUSB.print("[CAL] saving calibration value for pH 7.0: ");
  SerialUSB.println(this_mV);
  saveEEPROM(EEPROM_PH7, this_mV); 

  while (!digitalRead(ENTER_PIN));  // wait for ENTER press release

  // retrieve new calibration values
  calib_ph4_mV = readEEPROM(EEPROM_PH4); 
  calib_ph7_mV = readEEPROM(EEPROM_PH7);

}
///////////////////////////////////////////////////////////////////////////////////////////
