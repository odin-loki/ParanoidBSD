#!/bin/bash
# build_bsd_index.sh   One-time index of every .m file in the FreeBSD tree (used by tu_bsd.sh instead of a find per unit).
S=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src
IDX=$HOME/kb/bsd_m_index.txt
find "$S/sys" "$S" -maxdepth 6 -name '*.m' 2>/dev/null > "$IDX.part.$$" && mv "$IDX.part.$$" "$IDX"
echo "$(date '+%F %T') index built: $(wc -l < "$IDX") entries"
