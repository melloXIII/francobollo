# RP2040 Media Player

Un lettore multimediale sperimentale (Video MJPEG) basato su Raspberry Pi Pico (RP2040) e un display ST7735 con lettore di schede SD integrato.

## Caratteristiche
- Riproduzione di file Video `.mjpeg` a 20 FPS (160x128 pixel).
- Lettura streaming diretta da MicroSD Card.
- [Work In Progress] Supporto Audio.

## Struttura del Progetto
Questo repository è progettato per essere compilato tramite **PlatformIO**.
- `src/` : Contiene il codice sorgente C++ modulare.
- `include/` : Header files.
- `scripts/` : Script utili, come il convertitore video FFmpeg.
- `docs/` : Documentazione e schemi elettrici (wiring).

## Come convertire i video
Utilizza lo script in bash fornito per preparare i video per l'RP2040:
```bash
./scripts/convert_video.sh il_tuo_film.mp4
```

## Licenza
Questo progetto è distribuito sotto licenza **GNU GPLv3**. Sei libero di utilizzarlo e modificarlo, ma sei obbligato a rilasciare a tua volta il codice sorgente delle tue versioni modificate.
