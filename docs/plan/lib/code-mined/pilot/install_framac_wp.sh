#!/bin/bash
# install_framac_wp.sh  Build Frama-C with the WP plugin and Alt-Ergo in a private opam root (~/.opam-pbsd). Logs to $LOG. Resumable (each step is skipped if done).
# Needs (as root, done once):  apt-get install -y opam m4 libgmp-dev pkg-config graphviz bubblewrap autoconf
set -u
LOG=${LOG:-/mnt/c/ParanoidBSD/mined-data/pilot/install_framac.log}
export OPAMROOT=$HOME/.opam-pbsd OPAMYES=1 OPAMCONFIRMLEVEL=unsafe-yes
exec >>"$LOG" 2>&1
echo "== $(date -u +%FT%TZ) start"
[ -d "$OPAMROOT" ] || opam init --bare --disable-sandboxing --no-setup -y || exit 1
opam switch list 2>/dev/null | grep -q wp || opam switch create wp ocaml-base-compiler.4.14.2 || exit 1
eval "$(opam env --switch=wp)"
opam install -y --switch=wp alt-ergo || echo "alt-ergo failed"
opam install -y --switch=wp frama-c || exit 1
frama-c -version; frama-c -wp-help >/dev/null 2>&1 && echo WP_OK
opam list --switch=wp --installed 2>/dev/null | grep -E "^(frama-c|why3|alt-ergo|ocaml|zarith) "
echo "== $(date -u +%FT%TZ) done"
touch /mnt/c/ParanoidBSD/mined-data/pilot/install_framac.DONE
