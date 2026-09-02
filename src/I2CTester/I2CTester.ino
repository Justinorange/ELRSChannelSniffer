#include <Wire.h>

#define MY_I2C_ADDR 0x08

// Stores reconstructed 16-bit channel values (usually 988–2012 us)
volatile uint16_t channel1 = 0;
volatile uint16_t channel2 = 0;
volatile bool newPacketReceived = false;

void receiveEvent(int howMany);

void setup() {
  Serial.begin(115200);

  // Initialize Wire as Slave
  Wire.begin(MY_I2C_ADDR);
  Wire.onReceive(receiveEvent);

  Serial.println("Arduino Uno Ready - Listening for CRSF channel data...");
}

void loop() {
  if (newPacketReceived) {
    // Safely copy volatile variables
    noInterrupts();
    uint16_t currentVal1 = channel1;
    uint16_t currentVal2 = channel2;
    newPacketReceived = false;
    interrupts();

    // Print channel values
    Serial.print("Channel 1: ");
    Serial.print(currentVal1);
    Serial.print(" us | Channel 2: ");
    Serial.print(currentVal2);
    Serial.println(" us");
  }
}

// ISR triggered when ESP32 finishes transmitting packet
void receiveEvent(int howMany) {
  // Check if at least 4 bytes (2 bytes per channel) were sent
  if (howMany >= 4) {
    uint8_t high1 = Wire.read();
    uint8_t low1  = Wire.read();
    uint8_t high2 = Wire.read();
    uint8_t low2  = Wire.read();

    // Reconstruct 16-bit integers from bytes
    channel1 = ((uint16_t)high1 << 8) | low1;
    channel2 = ((uint16_t)high2 << 8) | low2;
    newPacketReceived = true;
  }

  // Clear out any extra remaining bytes if present
  while (Wire.available()) {
    Wire.read();
  }
}