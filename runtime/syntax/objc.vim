" Vim syntax file
" Language:	Objective C
" Maintainer:	Valentino Kyriakides <1kyriaki@informatik.uni-hamburg.de>
" Last Change:	2000 Nov 04

" Read the C syntax to start with
source <sfile>:p:h/c.vim

" Objective C extentions follow below
"
" NOTE: Objective C is abbreviated to ObjC/objc
" and uses *.h, *.m as file extensions!


" ObjC keywords, types, type qualifiers etc.
syn keyword objcStatement	self super _cmd
syn keyword objcType			id Class SEL IMP BOOL nil Nil
syn keyword objcTypeModifier bycopy in out inout oneway

" Match the ObjC #import directive (like C's #include)
syn region objcImported contained start=+"+  skip=+\\\\\|\\"+  end=+"+
syn match  objcImported contained "<[^>]*>"
syn match  objcImport  "^#\s*import\>\s*["<]" contains=objcImported

" Match the important ObjC directives
syn match  objcScopeDecl "@public\|@private\|@protected"
syn match  objcDirective	"@interface\|@implementation"
syn match  objcDirective	"@class\|@end\|@defs"
syn match  objcDirective	"@encode\|@protocol\|@selector"

" Match the ObjC method types
"
" NOTE: here I match only the indicators, this looks
" much nicer and reduces cluttering color highlightings.
" However, if you prefer full method declaration matching
" append .* at the end of the next two patterns!
"
syn match objcInstMethod  "^[\t\s]*-[\s]*"
syn match objcFactMethod  "^[\t\s]*+[\s]*"


" The default highlighting.
hi def link objcImport		Include
hi def link objcImported	cString
hi def link objcType		Type
hi def link objcScopeDecl	Statement
hi def link objcInstMethod	Function
hi def link objcFactMethod	Function
hi def link objcStatement	Statement
hi def link objcDirective	Statement

let b:current_syntax = "objc"

" vim: ts=8
