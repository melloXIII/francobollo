#!/bin/bash
# Convertitore Video per RP2040 Media Player
# Assicurati di avere FFmpeg installato nel sistema

# Prendi tutti gli argomenti e uniscili (utile se dimentichi le virgolette su nomi con spazi)
INPUT="$*"

if [ -z "$INPUT" ]; then
  echo "Errore: Specifica il file video da convertire."
  echo "Uso: bash scripts/convert_video.sh <file_video_input.mp4>"
  exit 1
fi

# Controllo se il file esiste davvero
if [ ! -f "$INPUT" ]; then
  echo "Errore: Il file '$INPUT' non esiste!"
  echo "Assicurati di aver scritto bene il percorso."
  exit 1
fi

# Forziamo l'output ad essere sempre "odissea.mjpeg" per la compatibilità con il firmware
OUTPUT="odissea.mjpeg"

echo "Conversione di '$INPUT' in formato ottimizzato per RP2040..."
# Filtro video:
# 1. scale+pad: mantiene le proporzioni e aggiunge le bande nere
# 2. eq: aumenta il contrasto (1.2) e la saturazione (1.2) per compensare i colori sbiaditi dello schermo TFT
ffmpeg -y -i "$INPUT" -vf "scale=160:128:force_original_aspect_ratio=decrease,pad=160:128:(ow-iw)/2:(oh-ih)/2,eq=contrast=1.2:saturation=1.3:brightness=0.02" -c:v mjpeg -q:v 5 -r 24 -an "$OUTPUT"

# Controllo dell'esito di FFmpeg
if [ $? -eq 0 ]; then
  echo ""
  echo "Finito! Il file generato è: $OUTPUT"
  echo "Copia questo file nella cartella /video/ della tua MicroSD."
else
  echo ""
  echo "ERRORE CRITICO: La conversione è fallita!"
  rm -f "$OUTPUT" # Elimina l'eventuale file corrotto a metà
  exit 1
fi
