#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int total_devices;
DeviceAddress sensor_address; 

void setup(){
  Serial.begin(115200);
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
}

void loop(){ 
  sensors.requestTemperatures(); 
  
  for(int i=0;i<total_devices; i++){
    
    if(sensors.getAddress(sensor_address, i)){
      
      Serial.print("Temperature for device: ");
      Serial.println(i,DEC);
     
      float temperature_degreeCelsius = sensors.getTempC(sensor_address);
      Serial.print("Temp (degree celsius): ");
      Serial.println(temperature_degreeCelsius);
  
    }
  }
  delay(1000);
}


void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
