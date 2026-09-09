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
  bool status = crsf.isLinkUp();
  uint16_t chData[16];
  

for (int ChannelNum = 0; ChannelNum < 16; ChannelNum++)
  {
    chData[ChannelNum] = crsf.getChannel(ChannelNum + 1); // load data

  }


  Wire.beginTransmission(I2C_SLAVE_ADDR); // begin transmission
  Wire.write((uint8_t) status); // send out the status of connection
  // Data channel send loop
  for (int ChannelNum = 0; ChannelNum < 16; ChannelNum++)
  {
    Wire.write((uint8_t)(chData[ChannelNum] >> 8));   // High byte Ch
    Wire.write((uint8_t)(chData[ChannelNum] & 0xFF)); // Low byte Ch
  }
  
  Wire.endTransmission(); // end I2C transmission
  
  // Must call crsf.update() in loop() to process data
  crsf.update();
}