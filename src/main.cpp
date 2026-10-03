#include <AlfredoCRSF.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include <Wire.h>


// CRSF UART Pins: valid for Pico UART1 (GPIO 20 TX, 21 RX)
#define CRSF_RX_PIN 21
#define CRSF_TX_PIN 20

// Output UART Pins: valid for Pico UART0 (GPIO 0 TX, 1 RX)
#define OUT_TX_PIN  0
#define OUT_RX_PIN  1
#define OUT_BAUDRATE 115200

// Packet Constants
#define SYNC_BYTE_1   0x37
#define SYNC_BYTE_2   0x01
#define TOTAL_CHANNELS 4 
#define CLASS_ID      0x01 // Custom Class ID
#define SUBCLASS_ID   0x01 // Custom Subclass ID
#define PAYLOAD_LEN   1 + 2*TOTAL_CHANNELS   // 1 byte (status) + (TOTAL_CHANNELS * 2 bytes)

// Set up Pico UART objects. On the RP2040, Serial1/Serial2 are the UART instances
// and need their TX/RX pins assigned before begin().
SerialUART crsfSerial = Serial2;
SerialUART outSerial = Serial1;
AlfredoCRSF crsf;

//Use crsf.getChannel(x) to get us channel values (1-16).
void printChannels()
{
  for (int ChannelNum = 1; ChannelNum <= TOTAL_CHANNELS; ChannelNum++)
  {
    Serial.print(crsf.getChannel(ChannelNum));
    Serial.print(", ");
  }
  Serial.println(" ");
}

// Fletcher-16 Checksum Calculation (Calculated over Class ID to end of Payload)
void calculateFletcher16(const uint8_t *data, size_t length, uint8_t &checkA, uint8_t &checkB) {
  checkA = 0;
  checkB = 0;
  for (size_t i = 0; i < length; i++) {
    checkA += data[i];
    checkB += checkA;
  }
}

void setup()
{
  // setup serial output to the downstream board
  outSerial.setRX(OUT_RX_PIN);
  outSerial.setTX(OUT_TX_PIN);
  outSerial.begin(OUT_BAUDRATE, SERIAL_8N1);

  // setup serial input from CRSF device
  crsfSerial.setRX(CRSF_RX_PIN);
  crsfSerial.setTX(CRSF_TX_PIN);
  crsfSerial.begin(CRSF_BAUDRATE, SERIAL_8N1);

  // loop forever until CRSF is connected
  if (!crsfSerial) while (1) Serial.println("Invalid crsfSerial configuration");

  crsf.begin(crsfSerial);
}

void loop()
{
  crsf.update();

  bool status = crsf.isLinkUp();
  uint16_t chData[TOTAL_CHANNELS];

  for (int ChannelNum = 0; ChannelNum < TOTAL_CHANNELS; ChannelNum++)
  {
    chData[ChannelNum] = crsf.getChannel(ChannelNum + 1);
  }

// Header (4) + Payload (9) + Checksum (2) = 15 bytes total
  uint8_t packet[4 + PAYLOAD_LEN + 2];
  size_t idx = 0;

  // --- HEADER (4 bytes) ---
  packet[idx++] = SYNC_BYTE_1;
  packet[idx++] = SYNC_BYTE_2;
  packet[idx++] = (uint8_t)((PAYLOAD_LEN >> 8) & 0xFF); // Payload Length MSB
  packet[idx++] = (uint8_t)(PAYLOAD_LEN & 0xFF);        // Payload Length LSB

// --- PAYLOAD (9 bytes) ---
  packet[idx++] = (uint8_t)status;

  for (int i = 0; i < TOTAL_CHANNELS; i++) {
    packet[idx++] = (uint8_t)(chData[i] >> 8);   // High byte
    packet[idx++] = (uint8_t)(chData[i] & 0xFF); // Low byte
  }

  // --- CHECKSUM (2 bytes) ---
  // Calculates Fletcher-16 over 2 Length bytes + PAYLOAD_LEN
  uint8_t chkA = 0, chkB = 0;
  calculateFletcher16(&packet[2], 2 + PAYLOAD_LEN, chkA, chkB);

  packet[idx++] = chkA;
  packet[idx++] = chkB;

  // Send exact number of written bytes
  outSerial.write(packet, idx);

  delay(10);
}