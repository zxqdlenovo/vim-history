" Vim syntax file
" Language:	po (GNU gettext)
" Maintainer:	Sung-Hyun Nam <namsh@kldp.org>
" Last Change:	2000 Nov 04

" remove any old syntax stuff hanging around
syn clear

syn match  poComment	"^#.*$"
syn match  poSources	"^#:.*$"
syn match  poStatement	"^\(msgid\|msgstr\)"
syn match  poSpecial	contained "\\\(x\x\+\|\o\{1,3}\|.\|$\)"
syn match  poFormat	"%\(\d\+\$\)\=[-+' #0*]*\(\d*\|\*\|\*\d\+\$\)\(\.\(\d*\|\*\|\*\d\+\$\)\)\=\([hlL]\|ll\)\=\([diuoxXfeEgGcCsSpn]\|\[\^\=.[^]]*\]\)" contained
syn match  poFormat	"%%" contained
syn region poString	start=+"+ skip=+\\\\\|\\"+ end=+"+
			\ contains=poSpecial,poFormat

" The default highlighting.
hi def link poComment	Comment
hi def link poSources	PreProc
hi def link poStatement	Statement
hi def link poSpecial	Special
hi def link poFormat	poSpecial
hi def link poString	String

let b:current_syntax = "po"

" vim:set ts=8 sts=8 sw=8 noet:
