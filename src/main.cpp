#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SdFat.h>
#include <TJpg_Decoder.h>

TFT_eSPI tft = TFT_eSPI(); 
SdFs sd;                   
FsFile videoFile;

#define LED_PIN 14 
#define SD_SCK 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_CS 13

// Buffer GIGANTI in RAM per il Dual Core (Ping-Pong Buffer)
#define READ_BUFFER_SIZE 30000 
uint8_t bufferA[READ_BUFFER_SIZE];
uint8_t bufferB[READ_BUFFER_SIZE];

volatile bool useBufferA = true;
volatile bool bufferReadyForCore1 = false;
volatile uint32_t currentFrameSize = 0;

// Doppio buffer per il chip hardware DMA (il decodificatore JPEG sforna quadratini di 16x16 pixel alla volta)
uint16_t dmaBuffer1[256];
uint16_t dmaBuffer2[256];
bool dmaBufferSelect = 0;

// Callback speciale per il DMA (viene eseguita sul Core 1)
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  
  // Scambiamo i buffer: mentre il DMA invia il primo allo schermo, noi calcoliamo il secondo!
  if (dmaBufferSelect) {
    memcpy(dmaBuffer1, bitmap, w * h * 2);
    tft.pushImageDMA(x, y, w, h, dmaBuffer1);
  } else {
    memcpy(dmaBuffer2, bitmap, w * h * 2);
    tft.pushImageDMA(x, y, w, h, dmaBuffer2);
  }
  dmaBufferSelect = !dmaBufferSelect;
  return 1;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); 

  tft.init();
  tft.setRotation(3); // Era 1, il 3 ribalta l'immagine sottosopra (orizzontale inverso)
  tft.initDMA(); // MAGIC TRICK: Attiviamo il chip speciale per la memoria video!
  
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(15, 20);
  tft.println("Francobollo");
  
  SPI1.setSCK(SD_SCK);
  SPI1.setTX(SD_MOSI);
  SPI1.setRX(SD_MISO);
  SPI1.begin();

  if (!sd.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1))) {
    tft.println("Errore SD!");
    while(1) delay(100);
  }

  TJpgDec.setJpgScale(1); 
  TJpgDec.setSwapBytes(true); 
  TJpgDec.setCallback(tft_output);
  
  // Apriamo il video (Ricordati di rinominarlo col nome esatto del tuo file!)
  videoFile = sd.open("/video/tuovideo.mjpeg", O_READ);
  if(!videoFile) {
     tft.println("Video mancante!");
     while(1) delay(100);
  }
}

// ==========================================
// CORE 0: Lavora in miniera. Legge dalla SD senza mai fermarsi e riempie i buffer
// ==========================================
#define CHUNK_SIZE 4096
uint8_t chunk[CHUNK_SIZE];

void loop() {
  if (!videoFile) return;

  uint32_t bufIdx = 0;
  bool inFrame = false;
  uint8_t prevByte = 0;
  
  uint8_t* targetBuffer = useBufferA ? bufferA : bufferB;

  while (videoFile.available()) {
    int bytesRead = videoFile.read(chunk, CHUNK_SIZE);
    
    for (int i = 0; i < bytesRead; i++) {
      uint8_t b = chunk[i];
      
      if (!inFrame && prevByte == 0xFF && b == 0xD8) {
        inFrame = true;
        bufIdx = 0;
        targetBuffer[bufIdx++] = 0xFF; 
        targetBuffer[bufIdx++] = 0xD8; 
      } 
      else if (inFrame) {
        if (bufIdx < READ_BUFFER_SIZE) {
          targetBuffer[bufIdx++] = b;
        }
        
        if (prevByte == 0xFF && b == 0xD9) {
          inFrame = false;
          
          currentFrameSize = bufIdx;
          bufferReadyForCore1 = true; // Dice al Core 1: "Sveglia! Ho la foto pronta!"
          
          // Aspetta che il Core 1 abbia finito di stamparla a schermo
          while (bufferReadyForCore1) {
            delay(1); 
          }
          
          // Scambia il buffer per la lettura successiva
          useBufferA = !useBufferA;
          targetBuffer = useBufferA ? bufferA : bufferB;
        }
      }
      prevByte = b;
    }
  }
  // Se il video finisce, lo facciamo ripartire da zero
  videoFile.seekSet(0);
}

// ==========================================
// CORE 1: È il nostro proiezionista. Decodifica e stampa il video sfruttando il DMA.
// ==========================================
void setup1() {
  // Il Core 1 non ha bisogno di configurare nulla all'avvio
}

void loop1() {
  static uint32_t lastFrameTime = 0;

  // Aspetta finché il Core 0 non gli sventola davanti una foto (frame) pronta
  if (bufferReadyForCore1) {
    
    tft.startWrite(); // Prepariamo la linea SPI per il DMA
    
    if (useBufferA) {
      TJpgDec.drawJpg(0, 0, bufferA, currentFrameSize);
    } else {
      TJpgDec.drawJpg(0, 0, bufferB, currentFrameSize);
    }
    
    tft.endWrite(); // Libera la linea

    // --- LIMITATORE DI VELOCITÀ (Per non farlo sembrare un video comico velocizzato) ---
    // Avendo esportato il video a 24 FPS, ogni frame deve durare circa 41 millisecondi.
    // L'RP2040 col Dual Core è così potente che lo riproduce troppo in fretta!
    while(millis() - lastFrameTime < 41) {
      delay(1);
    }
    lastFrameTime = millis();
    // ---------------------------------------------------------------------------------
    
    // Dice al Core 0: "Ho finito, vai col prossimo frame!"
    bufferReadyForCore1 = false;
  }
}
