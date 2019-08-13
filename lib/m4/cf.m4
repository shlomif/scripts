divert(-1)
define(`camino', `esyscmd(`which $1')')
define(`asegurar', `ifelse(`len($1)', `0', `m4exit(1)', `$1')')
define(`sin_nl', `regexp($1, `([^
]+)', `\1')')
define(`asociar', `define($1, `sin_nl(`asegurar(`esyscmd(`$2')')')')')
define(`comando', `sin_nl(`asegurar(`camino(`$1')')')' `$2')
divert(0)dnl
