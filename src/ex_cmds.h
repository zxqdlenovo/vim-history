/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * This file defines the Ex commands.
 * When DO_DECLARE_EXCMD is defined, the table with ex command names and
 * options results.
 * When DO_DECLARE_EXCMD is NOT defined, the enum with all the Ex commands
 * results.
 * This clever trick was invented by Ron Aaron.
 */

/*
 * When adding an Ex command:
 * 1. Add an entry in the table below.  Keep it sorted on the shortest
 *    version of the command name that works.  If it doesn't start with a
 *    lower case letter, add it at the end.
 * 2. Add a "case: CMD_xxx" in the big switch in ex_docmd.c.
 * 3. Add an entry in the index for Ex commands at ":help ex-cmd-index".
 * 4. Add documentation in ../doc/xxx.txt.  Add a tag for both the short and
 *    long name of the command.
 */

#ifdef RANGE
# undef RANGE			/* SASC on Amiga defines it */
#endif

#define RANGE	   0x01		/* allow a linespecs */
#define BANG	   0x02		/* allow a ! after the command name */
#define EXTRA	   0x04		/* allow extra args after command name */
#define XFILE	   0x08		/* expand wildcards in extra part */
#define NOSPC	   0x10		/* no spaces allowed in the extra part */
#define	DFLALL	   0x20		/* default file range is 1,$ */
#define NEEDARG	   0x80		/* argument required */
#define TRLBAR	  0x100		/* check for trailing vertical bar */
#define REGSTR	  0x200		/* allow "x for register designation */
#define COUNT	  0x400		/* allow count in argument, after command */
#define NOTRLCOM  0x800		/* no trailing comment allowed */
#define ZEROR	 0x1000		/* zero line number allowed */
#define USECTRLV 0x2000		/* do not remove CTRL-V from argument */
#define NOTADR	 0x4000		/* number before command is not an address */
#define EDITCMD	 0x8000		/* allow "+command" argument */
#define BUFNAME 0x10000L	/* accepts buffer name */
#define ARGOPT	0x20000L	/* allow "++opt=val" argument */
#define SBOXOK	0x40000L	/* allowed in the sandbox */
#define FILES	(XFILE | EXTRA)	/* multiple extra files allowed */
#define WORD1	(EXTRA | NOSPC)	/* one extra word allowed */
#define FILE1	(FILES | NOSPC)	/* 1 file allowed, defaults to current file */

#ifndef DO_DECLARE_EXCMD
typedef struct exarg exarg_t;
#endif

/*
 * This array maps ex command names to command codes.
 * The order in which command names are listed below is significant --
 * ambiguous abbreviations are always resolved to be the first possible match
 * (e.g. "r" is taken to mean "read", not "rewind", because "read" comes
 * before "rewind").
 * Not supported commands are included to avoid ambiguities.
 */
#ifdef EX
# undef EX	    /* just in case */
#endif
#ifdef DO_DECLARE_EXCMD
# define EX(a, b, c, d)  {(char_u *)b, c, d}

typedef void (*ex_func_t) __ARGS((exarg_t *eap));

static struct cmdname
{
    char_u	*cmd_name;	/* name of the command */
    ex_func_t   cmd_func;	/* function for this command */
    long_u	cmd_argt;	/* flags declared above */
}
# if defined(FEAT_GUI_W16)
_far
# endif
cmdnames[] =
#else
# define EX(a, b, c, d)  a
enum CMD_index
#endif
{
EX(CMD_append,		"append",	ex_append,
			BANG|RANGE|ZEROR|TRLBAR),
EX(CMD_abbreviate,	"abbreviate",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_abclear,		"abclear",	ex_abclear,
			EXTRA|TRLBAR),
EX(CMD_all,		"all",		ex_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_amenu,		"amenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_anoremenu,	"anoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_args,		"args",		ex_args,
			BANG|FILES|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_argument,	"argument",	ex_argument,
			BANG|RANGE|NOTADR|COUNT|EXTRA|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_ascii,		"ascii",	do_ascii,
			TRLBAR),
EX(CMD_autocmd,		"autocmd",	ex_autocmd,
			BANG|EXTRA|NOTRLCOM|USECTRLV),
EX(CMD_augroup,		"augroup",	ex_autocmd,
			WORD1|TRLBAR),
EX(CMD_aunmenu,		"aunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_buffer,		"buffer",	ex_buffer,
			BANG|RANGE|NOTADR|BUFNAME|COUNT|EXTRA|TRLBAR),
EX(CMD_bNext,		"bNext",	ex_bprevious,
			BANG|RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_ball,		"ball",		do_buffer_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_badd,		"badd",		ex_edit,
			NEEDARG|FILE1|EDITCMD|TRLBAR),
EX(CMD_bdelete,		"bdelete",	ex_bunload,
			BANG|RANGE|NOTADR|BUFNAME|COUNT|EXTRA|TRLBAR),
EX(CMD_behave,		"behave",	ex_behave,
			NEEDARG|WORD1|TRLBAR),
EX(CMD_bfirst,		"bfirst",	ex_brewind,
			BANG|RANGE|TRLBAR),
EX(CMD_blast,		"blast",	ex_blast,
			BANG|RANGE|TRLBAR),
EX(CMD_bmodified,	"bmodified",	ex_bmodified,
			BANG|RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_bnext,		"bnext",	ex_bnext,
			BANG|RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_botright,	"botright",	ex_botright,
			NEEDARG|EXTRA|NOTRLCOM),
EX(CMD_bprevious,	"bprevious",	ex_bprevious,
			BANG|RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_brewind,		"brewind",	ex_brewind,
			BANG|RANGE|TRLBAR),
EX(CMD_break,		"break",	ex_break,
			TRLBAR|SBOXOK),
EX(CMD_browse,		"browse",	ex_browse,
			NEEDARG|EXTRA|NOTRLCOM),
EX(CMD_buffers,		"buffers",	buflist_list,
			TRLBAR),
EX(CMD_bunload,		"bunload",	ex_bunload,
			BANG|RANGE|NOTADR|BUFNAME|COUNT|EXTRA|TRLBAR),
EX(CMD_change,		"change",	ex_change,
			BANG|RANGE|COUNT|TRLBAR),
EX(CMD_cNext,		"cNext",	ex_cnext,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cabbrev,		"cabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cabclear,	"cabclear",	ex_abclear,
			EXTRA|TRLBAR),
EX(CMD_call,		"call",		ex_call,
			RANGE|NEEDARG|EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_cc,		"cc",		ex_cc,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cd,		"cd",		ex_cd,
			FILE1|TRLBAR),
EX(CMD_center,		"center",	ex_align,
			TRLBAR|RANGE|EXTRA),
EX(CMD_cfile,		"cfile",	ex_cfile,
			TRLBAR|FILE1|BANG),
EX(CMD_cfirst,		"cfirst",	ex_cc,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_chdir,		"chdir",	ex_cd,
			FILE1|TRLBAR),
EX(CMD_checkpath,	"checkpath",	ex_checkpath,
			TRLBAR|BANG),
EX(CMD_clist,		"clist",	qf_list,
			BANG|EXTRA|TRLBAR),
EX(CMD_clast,		"clast",	ex_cc,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_close,		"close",	ex_close,
			BANG|TRLBAR),
EX(CMD_cmap,		"cmap",		ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cmapclear,	"cmapclear",	ex_mapclear,
			EXTRA|TRLBAR),
EX(CMD_cmenu,		"cmenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cnext,		"cnext",	ex_cnext,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cnewer,		"cnewer",	qf_age,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_cnfile,		"cnfile",	ex_cnext,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cnoremap,	"cnoremap",	ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cnoreabbrev,	"cnoreabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cnoremenu,	"cnoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_copy,		"copy",		ex_copymove,
			RANGE|EXTRA|TRLBAR),
EX(CMD_colder,		"colder",	qf_age,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_command,		"command",	ex_command,
			EXTRA|BANG|NOTRLCOM|USECTRLV),
EX(CMD_comclear,	"comclear",	ex_comclear,
			TRLBAR),
EX(CMD_continue,	"continue",	ex_continue,
			TRLBAR|SBOXOK),
EX(CMD_confirm,		"confirm",	ex_confirm,
			NEEDARG|EXTRA|NOTRLCOM),
EX(CMD_cprevious,	"cprevious",	ex_cnext,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cquit,		"cquit",	ex_cquit,
			TRLBAR|BANG),
EX(CMD_crewind,		"crewind",	ex_cc,
			RANGE|NOTADR|COUNT|TRLBAR|BANG),
EX(CMD_cscope,		"cscope",	do_cscope,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_cstag,		"cstag",	do_cstag,
			BANG|TRLBAR|WORD1),
EX(CMD_cunmap,		"cunmap",	ex_unmap,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cunabbrev,	"cunabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cunmenu,		"cunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_cwindow,		"cwindow",	ex_cwindow,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_delete,		"delete",	ex_operators,
			RANGE|REGSTR|COUNT|TRLBAR),
EX(CMD_delcommand,	"delcommand",	ex_delcommand,
			NEEDARG|WORD1|TRLBAR),
EX(CMD_delfunction,	"delfunction",	ex_delfunction,
			NEEDARG|WORD1|TRLBAR),
EX(CMD_display,		"display",	ex_display,
			EXTRA|NOTRLCOM|TRLBAR|SBOXOK),
EX(CMD_digraphs,	"digraphs",	ex_digraphs,
			EXTRA|TRLBAR),
EX(CMD_djump,		"djump",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_dlist,		"dlist",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_doautocmd,	"doautocmd",	ex_doautocmd,
			EXTRA|TRLBAR),
EX(CMD_doautoall,	"doautoall",	ex_doautoall,
			EXTRA|TRLBAR),
EX(CMD_drop,		"drop",		ex_drop,
			FILES|EDITCMD|NEEDARG|ARGOPT|TRLBAR),
EX(CMD_dsearch,		"dsearch",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_dsplit,		"dsplit",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_edit,		"edit",		ex_edit,
			BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_echo,		"echo",		ex_echo,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_echohl,		"echohl",	ex_echohl,
			EXTRA|TRLBAR|SBOXOK),
EX(CMD_echon,		"echon",	ex_echo,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_else,		"else",		ex_else,
			TRLBAR|SBOXOK),
EX(CMD_elseif,		"elseif",	ex_else,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_emenu,		"emenu",	ex_emenu,
			NEEDARG|EXTRA|TRLBAR|NOTRLCOM|RANGE),
EX(CMD_endif,		"endif",	ex_endif,
			TRLBAR|SBOXOK),
EX(CMD_endfunction,	"endfunction",	ex_endfunction,
			TRLBAR),
EX(CMD_endwhile,	"endwhile",	ex_endwhile,
			TRLBAR|SBOXOK),
EX(CMD_enew,		"enew",		ex_edit,
			BANG|TRLBAR),
EX(CMD_ex,		"ex",		ex_edit,
			BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_execute,		"execute",	ex_execute,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_exit,		"exit",		ex_exit,
			RANGE|BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_file,		"file",		ex_file,
			BANG|FILE1|TRLBAR),
EX(CMD_files,		"files",	buflist_list,
			TRLBAR),
EX(CMD_filetype,	"filetype",	ex_filetype,
			EXTRA|TRLBAR),
EX(CMD_find,		"find",		ex_find,
			BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_finish,		"finish",	ex_finish,
			TRLBAR|SBOXOK),
EX(CMD_first,		"first",	ex_rewind,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_fixdel,		"fixdel",	do_fixdel,
			TRLBAR),
EX(CMD_fold,		"fold",		ex_fold,
			RANGE|TRLBAR|SBOXOK),
EX(CMD_foldopen,	"foldopen",	ex_foldopen,
			RANGE|TRLBAR|SBOXOK),
EX(CMD_function,	"function",	ex_function,
			EXTRA|BANG),
EX(CMD_global,		"global",	ex_global,
			RANGE|BANG|EXTRA|DFLALL),
EX(CMD_goto,		"goto",		ex_goto,
			RANGE|NOTADR|COUNT|TRLBAR|SBOXOK),
EX(CMD_grep,		"grep",		ex_make,
			NEEDARG|EXTRA|NOTRLCOM|TRLBAR|XFILE),
EX(CMD_gui,		"gui",		ex_gui,
			BANG|FILES|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_gvim,		"gvim",		ex_gui,
			BANG|FILES|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_help,		"help",		ex_help,
			EXTRA|NOTRLCOM),
EX(CMD_helpfind,	"helpfind",	ex_helpfind,
			EXTRA|NOTRLCOM),
EX(CMD_helptags,	"helptags",	ex_helptags,
			NEEDARG|FILE1|TRLBAR),
EX(CMD_highlight,	"highlight",	ex_highlight,
			BANG|EXTRA|TRLBAR|SBOXOK),
EX(CMD_hide,		"hide",		ex_hide,
			BANG|EXTRA|NOTRLCOM),
EX(CMD_history,		"history",	ex_history,
			EXTRA|TRLBAR),
EX(CMD_insert,		"insert",	ex_append,
			BANG|RANGE|TRLBAR),
EX(CMD_iabbrev,		"iabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_iabclear,	"iabclear",	ex_abclear,
			EXTRA|TRLBAR),
EX(CMD_if,		"if",		ex_if,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_ijump,		"ijump",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_ilist,		"ilist",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_imap,		"imap",		ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_imapclear,	"imapclear",	ex_mapclear,
			EXTRA|TRLBAR),
EX(CMD_imenu,		"imenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_inoremap,	"inoremap",	ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_inoreabbrev,	"inoreabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_inoremenu,	"inoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_intro,		"intro",	ex_intro,
			TRLBAR),
EX(CMD_isearch,		"isearch",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_isplit,		"isplit",	ex_findpat,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_iunmap,		"iunmap",	ex_unmap,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_iunabbrev,	"iunabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_iunmenu,		"iunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_join,		"join",		ex_join,
			BANG|RANGE|COUNT|TRLBAR),
EX(CMD_jumps,		"jumps",	ex_jumps,
			TRLBAR),
EX(CMD_k,		"k",		ex_mark,
			RANGE|WORD1|TRLBAR|SBOXOK),
EX(CMD_list,		"list",		ex_print,
			RANGE|COUNT|TRLBAR),
EX(CMD_last,		"last",		ex_last,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_language,	"language",	ex_language,
			EXTRA|TRLBAR),
EX(CMD_lcd,		"lcd",		ex_cd,
			FILE1|TRLBAR),
EX(CMD_lchdir,		"lchdir",	ex_cd,
			FILE1|TRLBAR),
EX(CMD_left,		"left",		ex_align,
			TRLBAR|RANGE|EXTRA),
EX(CMD_let,		"let",		ex_let,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_ls,		"ls",		buflist_list,
			TRLBAR),
EX(CMD_move,		"move",		ex_copymove,
			RANGE|EXTRA|TRLBAR),
EX(CMD_mark,		"mark",		ex_mark,
			RANGE|WORD1|TRLBAR|SBOXOK),
EX(CMD_make,		"make",		ex_make,
			EXTRA|NOTRLCOM|TRLBAR|XFILE),
EX(CMD_map,		"map",		ex_map,
			BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_mapclear,	"mapclear",	ex_mapclear,
			BANG|TRLBAR),
EX(CMD_marks,		"marks",	do_marks,
			EXTRA|TRLBAR),
EX(CMD_menu,		"menu",		ex_menu,
			RANGE|NOTADR|ZEROR|BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_menutrans,	"menutrans",	ex_menutrans,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_messages,	"messages",	ex_messages,
			TRLBAR),
EX(CMD_mkexrc,		"mkexrc",	ex_mkrc,
			BANG|FILE1|TRLBAR),
EX(CMD_mksession,	"mksession",	ex_mkrc,
			BANG|FILE1|TRLBAR),
EX(CMD_mkvimrc,		"mkvimrc",	ex_mkrc,
			BANG|FILE1|TRLBAR),
EX(CMD_mode,		"mode",		ex_mode,
			WORD1|TRLBAR),
EX(CMD_next,		"next",		ex_next,
			RANGE|NOTADR|BANG|FILES|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_new,		"new",		ex_splitview,
			BANG|FILE1|RANGE|NOTADR|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_nmap,		"nmap",		ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_nmapclear,	"nmapclear",	ex_mapclear,
			EXTRA|TRLBAR),
EX(CMD_nmenu,		"nmenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_nnoremap,	"nnoremap",	ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_nnoremenu,	"nnoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_noremap,		"noremap",	ex_map,
			BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_nohlsearch,	"nohlsearch",	ex_nohlsearch,
			TRLBAR|SBOXOK),
EX(CMD_noreabbrev,	"noreabbrev",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_noremenu,	"noremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_normal,		"normal",	ex_normal,
			RANGE|BANG|EXTRA|NEEDARG|NOTRLCOM|USECTRLV|SBOXOK),
EX(CMD_number,		"number",	ex_print,
			RANGE|COUNT|TRLBAR),
EX(CMD_nunmap,		"nunmap",	ex_unmap,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_nunmenu,		"nunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_open,		"open",		ex_ni,
			TRLBAR),	/* not supported */
EX(CMD_omap,		"omap",		ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_omapclear,	"omapclear",	ex_mapclear,
			EXTRA|TRLBAR),
EX(CMD_omenu,		"omenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_only,		"only",		ex_only,
			BANG|TRLBAR),
EX(CMD_onoremap,	"onoremap",	ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_onoremenu,	"onoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_options,		"options",	ex_options,
			TRLBAR),
EX(CMD_ounmap,		"ounmap",	ex_unmap,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_ounmenu,		"ounmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_print,		"print",	ex_print,
			RANGE|COUNT|TRLBAR),
EX(CMD_pclose,		"pclose",	ex_pclose,
			BANG|TRLBAR),
EX(CMD_perl,		"perl",		ex_perl,
			RANGE|EXTRA|DFLALL|NEEDARG|SBOXOK),
EX(CMD_perldo,		"perldo",	ex_perldo,
			RANGE|EXTRA|DFLALL|NEEDARG|SBOXOK),
EX(CMD_pedit,		"pedit",	ex_pedit,
			BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_pop,		"pop",		ex_tag,
			RANGE|NOTADR|BANG|COUNT|TRLBAR|ZEROR),
EX(CMD_popup,		"popup",	ex_popup,
			NEEDARG|EXTRA|TRLBAR|NOTRLCOM),
EX(CMD_ppop,		"ppop",		ex_ptag,
			RANGE|NOTADR|BANG|COUNT|TRLBAR|ZEROR),
EX(CMD_preserve,	"preserve",	ex_preserve,
			TRLBAR),
EX(CMD_previous,	"previous",	ex_previous,
			EXTRA|RANGE|NOTADR|COUNT|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_promptfind,	"promptfind",	gui_mch_find_dialog,
			EXTRA|NOTRLCOM),
EX(CMD_promptrepl,	"promptrepl",	gui_mch_replace_dialog,
			EXTRA|NOTRLCOM),
EX(CMD_psearch,		"psearch",	ex_psearch,
			BANG|RANGE|DFLALL|EXTRA),
EX(CMD_ptag,		"ptag",		ex_ptag,
			RANGE|NOTADR|BANG|WORD1|TRLBAR|ZEROR),
EX(CMD_ptNext,		"ptNext",	ex_ptag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_ptfirst,		"ptfirst",	ex_ptag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_ptjump,		"ptjump",	ex_ptag,
			BANG|TRLBAR|WORD1),
EX(CMD_ptlast,		"ptlast",	ex_ptag,
			BANG|TRLBAR),
EX(CMD_ptnext,		"ptnext",	ex_ptag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_ptprevious,	"ptprevious",	ex_ptag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_ptrewind,	"ptrewind",	ex_ptag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_ptselect,	"ptselect",	ex_ptag,
			BANG|TRLBAR|WORD1),
EX(CMD_put,		"put",		ex_put,
			RANGE|BANG|REGSTR|TRLBAR|ZEROR),
EX(CMD_pwd,		"pwd",		ex_pwd,
			TRLBAR),
EX(CMD_python,		"python",	ex_python,
			RANGE|EXTRA|NEEDARG|SBOXOK),
EX(CMD_pyfile,		"pyfile",	ex_pyfile,
			RANGE|FILE1|NEEDARG|SBOXOK),
EX(CMD_quit,		"quit",		ex_quit,
			BANG|TRLBAR),
EX(CMD_quitall,		"quitall",	ex_quit_all,
			BANG|TRLBAR),
EX(CMD_qall,		"qall",		ex_quit_all,
			BANG|TRLBAR),
EX(CMD_read,		"read",		ex_read,
			BANG|RANGE|FILE1|TRLBAR|ZEROR),
EX(CMD_recover,		"recover",	ex_recover,
			BANG|FILE1|TRLBAR),
EX(CMD_redo,		"redo",		ex_redo,
			TRLBAR),
EX(CMD_redir,		"redir",	ex_redir,
			BANG|FILES|TRLBAR),
EX(CMD_registers,	"registers",	ex_display,
			EXTRA|NOTRLCOM|TRLBAR),
EX(CMD_resize,		"resize",	ex_resize,
			TRLBAR|WORD1),
EX(CMD_retab,		"retab",	ex_retab,
			TRLBAR|RANGE|DFLALL|BANG|WORD1),
EX(CMD_return,		"return",	ex_return,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_rewind,		"rewind",	ex_rewind,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_right,		"right",	ex_align,
			TRLBAR|RANGE|EXTRA),
EX(CMD_runtime,		"runtime",	ex_runtime,
			BANG|NEEDARG|FILES|TRLBAR|SBOXOK),
EX(CMD_ruby,		"ruby",		ex_ruby,
			RANGE|EXTRA|NEEDARG|SBOXOK),
EX(CMD_rubydo,		"rubydo",	ex_rubydo,
			RANGE|DFLALL|EXTRA|NEEDARG|SBOXOK),
EX(CMD_rubyfile,	"rubyfile",	ex_rubyfile,
			RANGE|FILE1|NEEDARG|SBOXOK),
EX(CMD_rviminfo,	"rviminfo",	ex_viminfo,
			BANG|FILE1|TRLBAR),
EX(CMD_substitute,	"substitute",	do_sub,
			RANGE|EXTRA),
EX(CMD_sNext,		"sNext",	ex_previous,
			EXTRA|RANGE|NOTADR|COUNT|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_sargument,	"sargument",	ex_argument,
			BANG|RANGE|NOTADR|COUNT|EXTRA|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_sall,		"sall",		ex_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sbuffer,		"sbuffer",	ex_buffer,
			BANG|RANGE|NOTADR|BUFNAME|COUNT|EXTRA|TRLBAR),
EX(CMD_sbNext,		"sbNext",	ex_bprevious,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sball,		"sball",	do_buffer_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sbfirst,		"sbfirst",	ex_brewind,
			TRLBAR),
EX(CMD_sblast,		"sblast",	ex_blast,
			TRLBAR),
EX(CMD_sbmodified,	"sbmodified",	ex_bmodified,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sbnext,		"sbnext",	ex_bnext,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sbprevious,	"sbprevious",	ex_bprevious,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_sbrewind,	"sbrewind",	ex_brewind,
			TRLBAR),
EX(CMD_scriptnames,	"scriptnames",	ex_scriptnames,
			TRLBAR),
EX(CMD_set,		"set",		ex_set,
			TRLBAR|EXTRA),
EX(CMD_setfiletype,	"setfiletype",	ex_setfiletype,
			TRLBAR|EXTRA|NEEDARG),
EX(CMD_setglobal,	"setglobal",	ex_set,
			TRLBAR|EXTRA),
EX(CMD_setlocal,	"setlocal",	ex_set,
			TRLBAR|EXTRA),
EX(CMD_sfind,		"sfind",	ex_splitview,
			BANG|FILE1|RANGE|NOTADR|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_sfirst,		"sfirst",	ex_rewind,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_shell,		"shell",	ex_shell,
			TRLBAR),
EX(CMD_simalt,		"simalt",	gui_simulate_alt_key,
			NEEDARG|WORD1|TRLBAR),
EX(CMD_sign,		"sign",		ex_sign,
			RANGE|EXTRA|NOTADR|NOSPC),
EX(CMD_signs,		"signs",	ex_sign,
			TRLBAR),
EX(CMD_silent,		"silent",	ex_silent,
			NEEDARG|EXTRA|BANG|NOTRLCOM|SBOXOK),
EX(CMD_sleep,		"sleep",	ex_sleep,
			RANGE|COUNT|NOTADR|EXTRA|TRLBAR),
EX(CMD_slast,		"slast",	ex_last,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_smagic,		"smagic",	ex_submagic,
			RANGE|EXTRA),
EX(CMD_snext,		"snext",	ex_next,
			RANGE|NOTADR|BANG|FILES|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_sniff,		"sniff",	ex_ni,
			EXTRA|TRLBAR),
EX(CMD_snomagic,	"snomagic",	ex_submagic,
			RANGE|EXTRA),
EX(CMD_source,		"source",	ex_source,
			BANG|FILE1|TRLBAR|SBOXOK),
EX(CMD_split,		"split",	ex_splitview,
			BANG|FILE1|RANGE|NOTADR|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_sprevious,	"sprevious",	ex_previous,
			EXTRA|RANGE|NOTADR|COUNT|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_srewind,		"srewind",	ex_rewind,
			EXTRA|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_stop,		"stop",		ex_stop,
			TRLBAR|BANG),
EX(CMD_stag,		"stag",		ex_stag,
			RANGE|NOTADR|BANG|WORD1|TRLBAR|ZEROR),
EX(CMD_startinsert,	"startinsert",	ex_startinsert,
			BANG|TRLBAR),
EX(CMD_stjump,		"stjump",	ex_stag,
			BANG|TRLBAR|WORD1),
EX(CMD_stselect,	"stselect",	ex_stag,
			BANG|TRLBAR|WORD1),
EX(CMD_sunhide,		"sunhide",	do_buffer_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_suspend,		"suspend",	ex_stop,
			TRLBAR|BANG),
EX(CMD_sview,		"sview",	ex_splitview,
			NEEDARG|RANGE|BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_swapname,	"swapname",	ex_swapname,
			TRLBAR),
EX(CMD_syntax,		"syntax",	ex_syntax,
			EXTRA|NOTRLCOM),
EX(CMD_syncbind,	"syncbind",	ex_syncbind,
			TRLBAR),
EX(CMD_t,		"t",		ex_copymove,
			RANGE|EXTRA|TRLBAR),
EX(CMD_tNext,		"tNext",	ex_tag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_tag,		"tag",		ex_tag,
			RANGE|NOTADR|BANG|WORD1|TRLBAR|ZEROR),
EX(CMD_tags,		"tags",		do_tags,
			TRLBAR),
EX(CMD_tcl,		"tcl",		ex_tcl,
			RANGE|EXTRA|NEEDARG|SBOXOK),
EX(CMD_tcldo,		"tcldo",	ex_tcldo,
			RANGE|DFLALL|EXTRA|NEEDARG|SBOXOK),
EX(CMD_tclfile,		"tclfile",	ex_tclfile,
			RANGE|FILE1|NEEDARG|SBOXOK),
EX(CMD_tearoff,		"tearoff",	ex_tearoff,
			NEEDARG|EXTRA|TRLBAR|NOTRLCOM),
EX(CMD_tfirst,		"tfirst",	ex_tag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_tjump,		"tjump",	ex_tag,
			BANG|TRLBAR|WORD1),
EX(CMD_tlast,		"tlast",	ex_tag,
			BANG|TRLBAR),
EX(CMD_tmenu,		"tmenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_tnext,		"tnext",	ex_tag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_topleft,		"topleft",	ex_topleft,
			NEEDARG|EXTRA|NOTRLCOM),
EX(CMD_tprevious,	"tprevious",	ex_tag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_trewind,		"trewind",	ex_tag,
			RANGE|NOTADR|BANG|TRLBAR|ZEROR),
EX(CMD_tselect,		"tselect",	ex_tag,
			BANG|TRLBAR|WORD1),
EX(CMD_tunmenu,		"tunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_undo,		"undo",		ex_undo,
			TRLBAR),
EX(CMD_unabbreviate,	"unabbreviate",	ex_abbreviate,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_unsign,		"unsign",	ex_unsign,
			RANGE|EXTRA|NOTADR|NOSPC),
EX(CMD_unhide,		"unhide",	do_buffer_all,
			RANGE|NOTADR|COUNT|TRLBAR),
EX(CMD_unlet,		"unlet",	ex_unlet,
			BANG|EXTRA|NEEDARG|TRLBAR|SBOXOK),
EX(CMD_unmap,		"unmap",	ex_unmap,
			BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_unmenu,		"unmenu",	ex_menu,
			BANG|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_update,		"update",	ex_update,
			RANGE|BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_vglobal,		"vglobal",	ex_global,
			RANGE|EXTRA|DFLALL),
EX(CMD_version,		"version",	ex_version,
			EXTRA|TRLBAR),
EX(CMD_vertical,	"vertical",	ex_vertical,
			NEEDARG|EXTRA|NOTRLCOM),
EX(CMD_visual,		"visual",	ex_edit,
			RANGE|BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_view,		"view",		ex_edit,
			RANGE|BANG|FILE1|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_vmap,		"vmap",		ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_vmapclear,	"vmapclear",	ex_mapclear,
			EXTRA|TRLBAR),
EX(CMD_vmenu,		"vmenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_vnoremap,	"vnoremap",	ex_map,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_vnew,		"vnew",		ex_splitview,
			BANG|FILE1|RANGE|NOTADR|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_vnoremenu,	"vnoremenu",	ex_menu,
			RANGE|NOTADR|ZEROR|EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_vsplit,		"vsplit",	ex_splitview,
			BANG|FILE1|RANGE|NOTADR|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_vunmap,		"vunmap",	ex_unmap,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_vunmenu,		"vunmenu",	ex_menu,
			EXTRA|TRLBAR|NOTRLCOM|USECTRLV),
EX(CMD_write,		"write",	ex_write,
			RANGE|BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_wNext,		"wNext",	ex_wnext,
			RANGE|NOTADR|BANG|FILE1|TRLBAR),
EX(CMD_wall,		"wall",		do_wqall,
			BANG|TRLBAR),
EX(CMD_while,		"while",	ex_while,
			EXTRA|NOTRLCOM|SBOXOK),
EX(CMD_winsize,		"winsize",	ex_winsize,
			EXTRA|NEEDARG|TRLBAR),
EX(CMD_winpos,		"winpos",	ex_winpos,
			EXTRA|TRLBAR),
EX(CMD_wnext,		"wnext",	ex_wnext,
			RANGE|NOTADR|BANG|FILE1|TRLBAR),
EX(CMD_wprevious,	"wprevious",	ex_wnext,
			RANGE|NOTADR|BANG|FILE1|TRLBAR),
EX(CMD_wq,		"wq",		ex_exit,
			RANGE|BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_wqall,		"wqall",	do_wqall,
			BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_wsverb,		"wsverb",	ex_wsverb,
			EXTRA|NOTADR|NEEDARG),
EX(CMD_wviminfo,	"wviminfo",	ex_viminfo,
			BANG|FILE1|TRLBAR),
EX(CMD_xit,		"xit",		ex_exit,
			RANGE|BANG|FILE1|DFLALL|TRLBAR),
EX(CMD_xall,		"xall",		do_wqall,
			BANG|TRLBAR),
EX(CMD_yank,		"yank",		ex_operators,
			RANGE|REGSTR|COUNT|TRLBAR),
EX(CMD_z,		"z",		ex_z,
			RANGE|EXTRA|TRLBAR),

/* commands that don't start with a lowercase letter */
EX(CMD_bang,		"!",		ex_bang,
			RANGE|BANG|FILES),
EX(CMD_pound,		"#",		ex_print,
			RANGE|COUNT|TRLBAR),
EX(CMD_and,		"&",		do_sub,
			RANGE|EXTRA),
EX(CMD_star,		"*",		ex_at,
			RANGE|EXTRA|TRLBAR),
EX(CMD_lshift,		"<",		ex_operators,
			RANGE|COUNT|TRLBAR),
EX(CMD_equal,		"=",		ex_equal,
			RANGE|TRLBAR),
EX(CMD_rshift,		">",		ex_operators,
			RANGE|COUNT|TRLBAR),
EX(CMD_at,		"@",		ex_at,
			RANGE|EXTRA|TRLBAR),
EX(CMD_Next,		"Next",		ex_previous,
			EXTRA|RANGE|NOTADR|COUNT|BANG|EDITCMD|ARGOPT|TRLBAR),
EX(CMD_Print,		"Print",	ex_print,
			RANGE|COUNT|TRLBAR),
EX(CMD_X,		"X",		ex_X,
			TRLBAR),
EX(CMD_tilde,		"~",		do_sub,
			RANGE|EXTRA),

#ifndef DO_DECLARE_EXCMD
#ifdef FEAT_USR_CMDS
    CMD_SIZE,		/* MUST be after all real commands! */
    CMD_USER = -1,	/* User-defined command */
    CMD_USER_BUF = -2	/* User-defined command local to buffer */
#else
    CMD_SIZE	/* MUST be the last one! */
#endif
#endif
};

#define USER_CMDIDX(idx) ((int)(idx) < 0)

#ifndef DO_DECLARE_EXCMD
typedef enum CMD_index cmdidx_t;

/*
 * Arguments used for Ex commands.
 */
struct exarg
{
    char_u	*arg;		/* argument of the command */
    char_u	*nextcmd;	/* next command (NULL if none) */
    char_u	*cmd;		/* the name of the command (except for :make) */
    char_u	**cmdlinep;	/* pointer to pointer of allocated cmdline */
    cmdidx_t	cmdidx;		/* the index for the command */
    long	argt;		/* flags for the command */
    int		skip;		/* don't execute the command, only parse it */
    int		forceit;	/* TRUE if ! present */
    int		addr_count;	/* the number of addresses given */
    linenr_t	line1;		/* the first line number */
    linenr_t	line2;		/* the second line number or count */
    char_u	*do_ecmd_cmd;	/* +command arg to be used in edited file */
    linenr_t	do_ecmd_lnum;	/* the line number in an edited file */
    int		append;		/* TRUE with ":w >>file" command */
    int		usefilter;	/* TRUE with ":w !command" and ":r!command" */
    int		amount;		/* number of '>' or '<' for shift command */
    int		regname;	/* register name (NUL if none) */
    char_u	*force_ff;	/* forced 'fileformat' */
#ifdef FEAT_MBYTE
    char_u	*force_fcc;	/* forced 'filecharcode' */
#endif
#ifdef FEAT_USR_CMDS
    int		useridx;	/* user command index */
#endif
    char_u	*errmsg;	/* returned error message */
    char_u	*(*getline) __ARGS((int, void *, int));
    void	*cookie;	/* argument for getline() */
#ifdef FEAT_EVAL
    struct condstack *cstack;	/* condition stack for ":if" etc. */
#endif
};
#endif
