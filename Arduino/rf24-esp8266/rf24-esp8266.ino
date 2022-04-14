#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(2, 4); // CE, CSN

const byte address[6] = "80808";     // Byte of array representing the address.
                                     // This is the address where we will send the data.
                                     // This should be same on the receiving side.

void setup() {
  Serial.begin(9600);
  if (!radio.begin()) {
    Serial.println(F("radio hardware not responding!"));
    while (1) {} // hold program in infinite loop to prevent subsequent errors
  }
  radio.setDataRate( RF24_250KBPS );
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);  // You can set it as minimum or maximum
                                  // depending on the distance between the transmitter and receiver.
  radio.setPayloadSize(8);
  radio.stopListening();          // This sets the module as transmitter
}

void loop() {
  const char text[9] = "00010030";
  radio.write(&text, 8);
  Serial.print("written: ");
  Serial.println(text);
  delay(10000);
}
