#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SdFat.h>
#include <TJpg_Decoder.h>

TFT_eSPI tft = TFT_eSPI(); // Istanza dello schermo
SdFs sd;                   // Istanza della scheda SD

#define LED_PIN 14 // Il pin che hai scelto per la retroilluminazione

// Pin per la scheda SD (SPI1)
#define SD_SCK 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_CS 13

// Buffer gigante in RAM per contenere un singolo frame video JPEG (max 30KB)
#define READ_BUFFER_SIZE 30000 
uint8_t jpegBuffer[READ_BUFFER_SIZE];

// Buffer per leggere velocemente dalla SD a blocchi
#define CHUNK_SIZE 4096
uint8_t chunk[CHUNK_SIZE];

// Funzione chiamata dal decodificatore JPEG per stampare i pixel sullo schermo
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void playVideo(const char* filename) {
  FsFile file = sd.open(filename, O_READ);
  if (!file) {
    Serial.println("Video non trovato!");
    return;
  }

  uint32_t bufIdx = 0;
  bool inFrame = false;
  uint8_t prevByte = 0;

  // Leggiamo la SD a blocchi grandi per andare velocissimi
  while (file.available()) {
    int bytesRead = file.read(chunk, CHUNK_SIZE);
    
    for (int i = 0; i < bytesRead; i++) {
      uint8_t b = chunk[i];
      
      // Cerchiamo l'inizio di una foto JPEG (0xFF 0xD8)
      if (!inFrame && prevByte == 0xFF && b == 0xD8) {
        inFrame = true;
        bufIdx = 0;
        jpegBuffer[bufIdx++] = 0xFF; // Salva il primo pezzo
        jpegBuffer[bufIdx++] = 0xD8; // Salva il secondo pezzo
      } 
      // Se siamo dentro la foto, salviamo tutti i pixel in RAM
      else if (inFrame) {
        if (bufIdx < READ_BUFFER_SIZE) {
          jpegBuffer[bufIdx++] = b;
        }
        
        // Cerchiamo la fine della foto JPEG (0xFF 0xD9)
        if (prevByte == 0xFF && b == 0xD9) {
          inFrame = false;
          // Mostriamo il frame sul display a velocità folle!
          TJpgDec.drawJpg(0, 0, jpegBuffer, bufIdx);
        }
      }
      prevByte = b;
    }
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  Serial.println("francobollo - Avvio Sistema...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  tft.init();
  tft.setRotation(1); 
  
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(15, 20);
  tft.println("Francobollo");
  tft.setTextSize(1);
  tft.setCursor(15, 50);
  tft.println("Inizializzazione...");

  SPI1.setSCK(SD_SCK);
  SPI1.setTX(SD_MOSI);
  SPI1.setRX(SD_MISO);
  SPI1.begin();

  if (!sd.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1))) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Errore SD!");
  } else {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("SD Trovata!");
  }

  // Setup del decodificatore JPEG
  TJpgDec.setJpgScale(1); 
  TJpgDec.setSwapBytes(true); // Serve per lo schermo ST7735
  TJpgDec.setCallback(tft_output);
  
  delay(1000); // Pausa per far leggere le info all'utente
}

void loop() {
  // Avvia il player video! Ricordati di mettere il nome esatto del tuo file
  // Siccome l'hai messo in "video/", il percorso è "/video/nomefile.mjpeg"
  playVideo("/video/tuovideo.mjpeg"); 
}
