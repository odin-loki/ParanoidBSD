# lc.sh   Source me.  lc <path>  prints the path of a WSL-local (ext4) mirror of a big, rarely-changing file that lives on the slow /mnt/c (9p) share,
# refreshed whenever the original's size or mtime changes (flock-guarded, atomic mv). Scans of 400 MB rows.tsv / 275 MB tags.jsonl from /mnt/c took minutes under load; on ext4 seconds.
# LC=<dir> overrides the cache directory (default ~/kb/scratch/pbsd-cache). Falls back to the original path when the mirror cannot be made.
LC=${LC:-$HOME/kb/scratch/pbsd-cache}; mkdir -p "$LC" 2>/dev/null
lc() {
  local p=$1 n sig
  n=$LC/$(printf '%s' "$p" | tr '/' '_'); sig=$(stat -c '%s %Y' "$p" 2>/dev/null) || { echo "$p"; return; }
  if [ "$(cat "$n.sig" 2>/dev/null)" != "$sig" ]; then
    ( flock 9
      if [ "$(cat "$n.sig" 2>/dev/null)" != "$sig" ]; then
        cp "$p" "$n.part" && mv "$n.part" "$n" && printf '%s' "$sig" > "$n.sig" || rm -f "$n.part"
      fi ) 9> "$n.lock"
  fi
  [ "$(cat "$n.sig" 2>/dev/null)" = "$sig" ] && echo "$n" || echo "$p"
}
# state_tsv   prints the WSL-local copy of final/state.tsv that state.sh left behind, when it is exactly what is on /mnt/c (same content hash and untouched since the publish), else the /mnt/c path.
state_tsv() {
  local t=${F:-/mnt/c/ParanoidBSD/mined-data/final}/state.tsv sc=${SC:-$HOME/kb/scratch/pbsd-state} pf
  pf=$sc/pub.$(printf '%s' "$t" | tr '/' '_')
  if [ -s "$sc/state.tsv" ] && [ "$(cat "$pf" 2>/dev/null)" = "$(md5sum < "$sc/state.tsv" | cut -d' ' -f1) $(stat -c '%s %Y' "$t" 2>/dev/null)" ]; then echo "$sc/state.tsv"; else echo "$t"; fi
}
# publish <local-file> <target>   copy to /mnt/c via a temp file and an atomic mv (cp uses big blocks; awk/cmp on 9p use tiny ones and are 10-50x slower). Skips the copy when this very content was published
# before and the target is untouched since (checked by stat, without reading the 9p file). $PUBD = directory for the bookkeeping (default $SC).
publish() {
  local pd=${PUBD:-${SC:-$HOME/kb/scratch/pbsd-state}} pf want; mkdir -p "$pd"; pf=$pd/pub.$(printf '%s' "$2" | tr '/' '_')
  want="$(md5sum < "$1" | cut -d' ' -f1) $(stat -c '%s %Y' "$2" 2>/dev/null)"
  [ "$(cat "$pf" 2>/dev/null)" = "$want" ] && return 0
  cp "$1" "$2.part" && mv "$2.part" "$2" && printf '%s' "$(md5sum < "$1" | cut -d' ' -f1) $(stat -c '%s %Y' "$2")" > "$pf"
}
