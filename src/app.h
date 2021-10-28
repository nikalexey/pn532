#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_PN532.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ESP32Encoder.h>

#include "Arduino.h"
// #include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>

class App {
public:
  App();
  void Run();
private:
  void InitDisplay();
  void InitEncoder();
  void InitPN532();
  void displayWelcomeScreen(uint32_t delayMs);
  void loop();
  void startListeningToNFC();
  void handleCardDetected();
private:
  Adafruit_SSD1306 display;
  ESP32Encoder encoder;
  Adafruit_PN532 nfc;
  uint8_t irqCurr{0};
  uint8_t irqPrev{0};
  bool readerDisabled{false};
  uint64_t timeLastCardRead;
};
