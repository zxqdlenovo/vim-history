" Vim syntax file
" Language:	B (A Formal Method with refinement and mathematical proof)
" Maintainer:	Mathieu Clabaut <mathieu.clabaut@free.fr>
" Last change:	28 Jan 1999  

" Remove any old syntax stuff hanging around
syn clear

" A bunch of useful B keywords
syn keyword bStatement	MACHINE SEES OPERATIONS INCLUDES DEFINITIONS CONSTRAINTS CONSTANTS VARIABLES CONCRETE_CONSTANTS CONCRETE_VARIABLES ABSTRACT_CONSTANTS ABSTRACT_VARIABLES HIDDEN_CONSTANTS HIDDEN_VARIABLES ASSERT ASSERTIONS  EXTENDS IMPLEMENTATION REFINEMENT IMPORTS USES INITIALISATION INVARIANT PROMOTES PROPERTIES REFINES SETS VALUES VARIANT VISIBLE_CONSTANTS VISIBLE_VARIABLES THEORY
syn keyword bLabel		CASE IN EITHER OR CHOICE DO OF 
syn keyword bConditional	IF ELSE SELECT ELSIF THEN WHEN
syn keyword bRepeat		WHILE FOR
syn keyword bOps		bool card conc closure closure1 dom first fnc front not or id inter iseq iseq1 iterate last max min mod perm pred prj1 prj2 ran rel rev seq seq1 size skip succ tail union
syn keyword bKeywords		LET VAR BE IN BEGIN END  POW POW1 FIN FIN1  PRE  SIGMA STRING UNION IS ANY WHERE
syn match bKeywords	"||"

syn keyword bBoolean	TRUE FALSE bfalse btrue
syn keyword bConstant	PI MAXINT MININT User_Pass PatchProver PatchProverH0 PatchProverB0 FLAT ARI DED SUB RES
syn keyword bGuard binhyp band bnot bguard bsearch bflat bfresh bguardi bget bgethyp barith bgetresult bresult bgoal bmatch bmodr bnewv  bnum btest bpattern bprintf bwritef bsubfrm  bvrb blvar bcall bappend bclose

syn keyword bLogic	or not
syn match bLogic	"\&\|=>\|<=>"

syn keyword cTodo contained	TODO FIXME XXX

" String and Character constants
" Highlight special characters (those which have a backslash) differently
syn match bSpecial contained	"\\[0-7][0-7][0-7]\=\|\\."
syn region bString		start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=bSpecial
syn match bCharacter		"'[^\\]'"
syn match bSpecialCharacter	"'\\.'"
syn match bSpecialCharacter	"'\\[0-7][0-7]'"
syn match bSpecialCharacter	"'\\[0-7][0-7][0-7]'"

"catch errors caused by wrong parenthesis
syn region bParen		transparent start='(' end=')' contains=ALLBUT,bParenError,bIncluded,bSpecial,bTodo,bUserLabel,bBitField
syn match bParenError		")"
syn match bInParen contained	"[{}]"

"integer number, or floating point number without a dot and with "f".
syn case ignore
syn match bNumber		"\<[0-9]\+\>"
"syn match bIdentifier	"\<[a-z_][a-z0-9_]*\>"
syn case match

if exists("b_comment_strings")
  " A comment can contain bString, bCharacter and bNumber.
  " But a "*/" inside a bString in a bComment DOES end the comment!  So we
  " need to use a special type of bString: bCommentString, which also ends on
  " "*/", and sees a "*" at the start of the line as comment again.
  " Unfortunately this doesn't very well work for // type of comments :-(
  syntax match bCommentSkip	contained "^\s*\*\($\|\s\+\)"
  syntax region bCommentString	contained start=+"+ skip=+\\\\\|\\"+ end=+"+ end=+\*/+me=s-1 contains=bSpecial,bCommentSkip
  syntax region bComment2String	contained start=+"+ skip=+\\\\\|\\"+ end=+"+ end="$" contains=bSpecial
  syntax region bComment	start="/\*" end="\*/" contains=bTodo,bCommentString,bCharacter,bNumber,bFloat
  syntax region bComment	start="/\?\*" end="\*\?/" contains=bTodo,bCommentString,bCharacter,bNumber,bFloat
  syntax match  bComment	"//.*" contains=bTodo,bComment2String,bCharacter,bNumber
else
  syn region bComment		start="/\*" end="\*/" contains=bTodo
  syn region bComment		start="/\?\*" end="\*\?/" contains=bTodo
  syn match bComment		"//.*" contains=bTodo
endif
syntax match bCommentError	"\*/"

syn keyword bType		INT INTEGER BOOL NAT NATURAL NAT1 NATURAL1 

syn region bPreCondit	start="^\s*#\s*\(if\>\|ifdef\>\|ifndef\>\|elif\>\|else\>\|endif\>\)" skip="\\$" end="$" contains=bComment,bString,bCharacter,bNumber,bCommentError
syn region bIncluded contained start=+"+ skip=+\\\\\|\\"+ end=+"+
syn match bIncluded contained "<[^>]*>"
syn match bInclude		"^\s*#\s*include\>\s*["<]" contains=bIncluded

syn region bDefine		start="^\s*#\s*\(define\>\|undef\>\)" skip="\\$" end="$" contains=ALLBUT,bPreCondit,bIncluded,bInclude,bDefine,bInParen
syn region bPreProc		start="^\s*#\s*\(pragma\>\|line\>\|warning\>\|warn\>\|error\>\)" skip="\\$" end="$" contains=ALLBUT,bPreCondit,bIncluded,bInclude,bDefine,bInParen


syn sync ccomment bComment minlines=10

if !exists("did_b_syntax_inits")
  let did_b_syntax_inits = 1
  " The default methods for highlighting.  Can be overridden later
  hi link bLabel	Label
  hi link bUserLabel	Label
  hi link bConditional	Conditional
  hi link bRepeat	Repeat
  hi link bLogic	Special
  hi link bCharacter	Character
  hi link bSpecialCharacter bSpecial
  hi link bNumber	Number
  hi link bFloat	Float
  hi link bOctalError	bError
  hi link bParenError	bError
"  hi link bInParen	bError
  hi link bCommentError	bError
  hi link bBoolean	Identifier
  hi link bConstant	Identifier
  hi link bGuard	Identifier
  hi link bOperator	Operator
  hi link bKeywords	Operator
  hi link bOps		Identifier
  hi link bStructure	Structure
  hi link bStorageClass	StorageClass
  hi link bInclude	Include
  hi link bPreProc	PreProc
  hi link bDefine	Macro
  hi link bIncluded	bString
  hi link bError	Error
  hi link bStatement	Statement
  hi link bPreCondit	PreCondit
  hi link bType		Type
  hi link bCommentError	bError
  hi link bCommentString bString
  hi link bComment2String bString
  hi link bCommentSkip	bComment
  hi link bString	String
  hi link bComment	Comment
  hi link bSpecial	SpecialChar
  hi link bTodo		Todo
  "hi link bIdentifier	Identifier
endif

let current_syntax = "b"

" vim: ts=8
