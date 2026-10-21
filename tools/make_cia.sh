#!/bin/bash
# Usage : bash tools/make_cia.sh heartfight
set -e
NAME="${1:-heartfight}"

bannertool makebanner -i assets/banner.png -a assets/audio.wav -o banner.bnr
bannertool makesmdh -i assets/icon.png -s "Coeur Fragile" -l "Jeu de boss facon bullet hell" -p "Moi" -o icon.icn
makerom -f cia -o "${NAME}.cia" -elf "${NAME}.elf" -rsf app.rsf -target t -exefslogo -icon icon.icn -banner banner.bnr

echo "CIA cree : ${NAME}.cia"
