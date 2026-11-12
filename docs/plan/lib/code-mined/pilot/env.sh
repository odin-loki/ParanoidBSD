# env.sh  source me: puts the pinned Frama-C 33.0 (WP) + Alt-Ergo 2.6.4 + Why3 1.8.2 from the private opam switch first on PATH; z3 4.8.12 and cvc5 1.1.2 come from apt.
export OPAMROOT=$HOME/.opam-pbsd
eval "$(opam env --switch=wp --set-switch 2>/dev/null)"
