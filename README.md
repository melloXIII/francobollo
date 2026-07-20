## francobollo
# one of the smallest cinema in the world. 
a minimalistic MJPEG video player built with Raspberry Pi Pico (RP2040) and an ST7735 display.

## funzionalità principali
- architettura Dual-core: il carico lavorativo è diviso ai due core del RP2040 - Zero. il core 0 gestisce esclusivamente la lettura della SD (tramite Ping-Pong buffer); il core 1 si occupa della decodifica JPEG e del rendering a schermo;
- DMA (Direct Memory Access): trasferimento dei pixel al display evitando i colli di bottiglia sul bus SPI;
- overclock a 250MHz;
- SPI separate: lo schermo (SPI0) e la scheda SD (SPI1) viaggiano su linee separate;
- limitatore del frame rate: sincronizzazione a 24 FPS.

## hardware 
- microcontrollore: Raspberry Pi RP2040-Zero;
- display: schermo tft ST7735 da 1.8" pollici con lettore SD integrato;
memoria: scheda SD.

## collegamenti

### Display (Linea SPI0)
| Pin ST7735 | Pin RP2040-Zero | Funzione |
|:---|:---|:---|
| `VCC` | **3.3V** | Alimentazione |
| `GND` | **GND** | Massa |
| `SDA / MOSI` | **GP3** | Dati SPI0 |
| `SCL / SCK` | **GP2** | Clock SPI0 |
| `CS / TFT_CS` | **GP1** | Chip Select |
| `DC / RS` | **GP4** | Data / Command |
| `RES / RST` | **GP5** | Reset hardware |
| `BLK / LED` | **GP14** | Retroilluminazione fissa |

### SD Card (Linea SPI1)
| Pin Lettore SD | Pin RP2040-Zero | Funzione |
|:---|:---|:---|
| `SD_MOSI` | **GP11** | Dati in ingresso SPI1 |
| `SD_MISO` | **GP12** | Dati in uscita SPI1 |
| `SD_SCK` | **GP10** | Clock SPI1 |
| `SD_CS` | **GP13** | Chip Select SD |

## video
l'RP2040 non può leggere i file .mp4, quindi in questo caso ho deciso di usare i .mjpeg.
nel progetto è incluso lo script script/convert_video.sh che permette la conversione e aggiunge bande nere in modo intelligente.

il comando per Linux/Mac con FFmpeg installato: 
    bash scripts/convert_video.sh /percorso/video.mp4

in questo modo otterrai un file .mjpeg dal nome odissea, prendilo e copialo nella tua SD!

## licenza
questo progetto è distribuito sotto licenza MIT.
