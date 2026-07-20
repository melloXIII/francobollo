#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("RP2040 Media Player - Avvio Sistema...");
  
  // TODO: Inizializzare lo schermo ST7735
  // TODO: Inizializzare la SD Card
}

void loop() {
  // TODO: Logica di decodifica e riproduzione video
}
