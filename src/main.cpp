#include <AlfredoCRSF.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_SLAVE_ADDR 0x08
#define PIN_RX 20
#define PIN_TX 21

// Set up a new Serial object
HardwareSerial crsfSerial(1);
AlfredoCRSF crsf;

//Use crsf.getChannel(x) to get us channel values (1-16).
void printChannels()
{
  for (int ChannelNum = 1; ChannelNum <= 16; ChannelNum++)
  {
    Serial.print(crsf.getChannel(ChannelNum));
    Serial.print(", ");
  }
  Serial.println(" ");
}

void setup()
{

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  crsfSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
  if (!crsfSerial) while (1) Serial.println("Invalid crsfSerial configuration");

  crsf.begin(crsfSerial);
}

void loop()
{
  uint16_t channel1 = crsf.getChannel(1);
  uint16_t channel2 = crsf.getChannel(2);

  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write((uint8_t)(channel1 >> 8));   // High byte Ch1
  Wire.write((uint8_t)(channel1 & 0xFF)); // Low byte Ch1
  Wire.write((uint8_t)(channel2 >> 8));   // High byte Ch2
  Wire.write((uint8_t)(channel2 & 0xFF)); // Low byte Ch2
  Wire.endTransmission();
  
    // Must call crsf.update() in loop() to process data
  crsf.update();
    // printChannels();
}