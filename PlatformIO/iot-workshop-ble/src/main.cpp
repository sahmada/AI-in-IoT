#include <Arduino.h>
#include "ESP32BleAdvertise.h"
SimpleBLE ble;

unsigned long long counter = 0;

void setup()
{
  Serial.begin(115200);

  ble.begin("iot-workshop-temp");
}

void loop()
{
  counter++;

  float temp = 27.51;
  uint32_t fault = 10;

  char buffer[20] = {0};
  sprintf(buffer, "xx%.2f,%d", temp, fault);
  ble.advertise((uint8_t *)buffer, 20);

  Serial.println(buffer);
  Serial.print(counter);
  Serial.println(" sent advertising data");

  delay(1000);
}