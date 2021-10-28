#include "app.h"

static constexpr uint32_t welcomeScreenDelay(2000);

static constexpr uint8_t SCREEN_WIDTH(128);
static constexpr uint8_t SCREEN_HEIGHT(64);
static constexpr uint8_t SCREEN_ADDRESS(0x3c);

static constexpr uint64_t DELAY_BETWEEN_CARDS = 1000;


App::App()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1)
  , nfc(23, 5) {
}

void App::Run() {
  InitDisplay();
  InitEncoder();
  InitPN532();
  displayWelcomeScreen(welcomeScreenDelay);
  startListeningToNFC();
  while(true) {
    loop();
  }
}

void App::startListeningToNFC() {
  // Reset our IRQ indicators
  irqPrev = 1;
  irqCurr = 1;

  Serial.println("Waiting for an ISO14443A Card ...");
  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
}

void App::handleCardDetected() {
  uint8_t success = false;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};  // Buffer to store the returned UID
  uint8_t uidLength;  // Length of the UID (4 or 7 bytes depending on ISO14443A
                      // card type)

  // read the NFC tag's info
  success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
  Serial.println(success ? "Read successful" : "Read failed (not a card?)");

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);

    if (uidLength == 4) {
      // We probably have a Mifare Classic card ...
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];
      cardid <<= 8;
      cardid |= uid[3];
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
    }
    Serial.println("");

  }
  timeLastCardRead = millis();

  // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
  readerDisabled = true;
}

void App::loop() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(15, 24);
  display.println(encoder.getCount());
  display.display();

  if (readerDisabled) {
    if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) {
      readerDisabled = false;
      startListeningToNFC();
    }
  } else {
    irqCurr = digitalRead(23);

    // When the IRQ is pulled low - the reader has got something for us.
    if (irqCurr == LOW && irqPrev == HIGH) {
       Serial.println("Got NFC IRQ");  
       handleCardDetected(); 
    }
  
    irqPrev = irqCurr;
  }

}

void App::InitDisplay() {
  Serial.println("Init display");
  Wire1.setPins(18,19);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
}

void App::InitEncoder() {
  Serial.println("Init encoder");
  ESP32Encoder::useInternalWeakPullResistors = UP;
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);
  encoder.attachSingleEdge(26, 25);
  encoder.setCount(0);
}

void App::InitPN532() {
  Serial.println("Init pn532");

  Wire.begin(-1, -1, 100000L);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    // display.clearDisplay();
    // display.setTextSize(2);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(1, 1);
    // display.print("Error init pn532");
    // display.display();
    Serial.println("Error init pn532");
    while (1)
      ;  // halt
  }
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();
}

void App::displayWelcomeScreen(uint32_t delayMs) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(15, 24);
  display.println(F("Quadcode"));
  display.display();
  delay(delayMs);
}