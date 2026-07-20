#!/bin/bash
# Convertitore Video per RP2040 Media Player
# Assicurati di avere FFmpeg installato nel sistema

if [ -z "$1" ]; then
  echo "Errore: Specifica il file video da convertire."
  echo "Uso: ./convert_video.sh <file_video_input.mp4>"
  exit 1
fi

INPUT=$1
OUTPUT="${INPUT%.*}.mjpeg"

echo "Conversione di '$INPUT' in formato ottimizzato per RP2040..."
ffmpeg -i "$INPUT" -vf "scale=160:128" -c:v mjpeg -q:v 5 -r 20 -an "$OUTPUT"

echo ""
echo "Finito! Il file generato è: $OUTPUT"
echo "Copia questo file sulla MicroSD."
