#include "app.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  App app;
  app.Run();
  Serial.println("Halt");
}

void loop() {
}
 