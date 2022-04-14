#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(8, 9); // CE, CSN
const byte address[6] = "80808";

const uint8_t PIN = 4;
volatile uint8_t ON_MS = 1;
volatile uint8_t OFF_MS = 30;
char ch[9] = {0};
char ch_on[5] = {0};
char ch_off[5] = {0};

void setup()
{
  Serial.begin(9600);
  if (!radio.begin()) {
    Serial.println(F("radio hardware not responding!"));
    while (1) {} // hold program in infinite loop to prevent subsequent errors
  }
  radio.setDataRate( RF24_250KBPS );
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setPayloadSize(8);
  radio.startListening();
}

void loop()
{
  if (radio.available())
  {
    uint8_t bytes = radio.getPayloadSize();
    if (bytes == 8)
    {
      radio.read(&ch, bytes);
      parse();
    }
  }
  
  analogWrite(PIN, 255);
  delay(ON_MS);
  analogWrite(PIN, 0);
  delay(OFF_MS);
}

void serialEvent()
{
  size_t n_read = Serial.readBytes(ch, 9);
  if(n_read != 9)
  {
    return;
  }

  parse();
}

void parse()
{
  strncpy(ch_on, ch, 4);
  strncpy(ch_off, ch + 4, 4);
  ON_MS = String(ch_on).toInt();
  OFF_MS = String(ch_off).toInt();
}
