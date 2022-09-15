#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int total_devices;
float average_temperature =0;
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
      Serial.print(i,DEC);
     
      float temperature_degreeCelsius = sensors.getTempC(sensor_address);
      Serial.print("\t");
      Serial.print(temperature_degreeCelsius);
      Serial.println(" °C");
      average_temperature += temperature_degreeCelsius;
    }
  }
  average_temperature /= total_devices;
  Serial.print(" Average Temperature is :\t");
  Serial.print(average_temperature);
  Serial.println(" °C");
  Serial.println("---------------------------");
  average_temperature = 0;
  delay(1000);  
}


void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
