/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */

/*
 * Code to handle user-settable options. This is all pretty much table-
 * driven. Checklist for adding a new option:
 * - Put it in the options array below (copy an existing entry).
 * - For a global option: Add a variable for it in option.h.
 * - For a buffer or window local option:
 *   - Add a PV_XX entry to the enum below.
 *   - Add a variable to the window or buffer struct in structs.h.
 *   - For a window option, add some code to copy_winopt().
 *   - For a buffer option, add some code to buf_copy_options().
 *   - For a buffer string option, add code to check_buf_options().
 * - If it's a numeric option, add any necessary bounds checks to do_set().
 * - If it's a list of flags, add some code in do_set(), search for WW_ALL.
 * - When adding an option with expansion (P_EXPAND), but with a different
 *   default for Vi and Vim (no P_VI_DEF), add some code at VIMEXP.
 * - Add documentation!  One line in doc/help.txt, full description in
 *   options.txt, and any other related places.
 * - Add an entry in runtime/optwin.vim.
 * When making changes:
 * - Adjust the help for the option in doc/option.txt.
 * - When an entry has the P_VIM flag, or is lacking the P_VI_DEF flag, add a
 *   comment at the help for the 'compatible' option.
 */

#define IN_OPTION_C
#include "vim.h"

/*
 * The options that are local to a window or buffer have "indir" set to one of
 * these values.  Special values:
 * PV_NONE: global option.
 * PV_BOTH is added: global option which also has a local value.
 */
#define PV_BOTH 0x1000
#define OPT_BOTH(x) (idopt_T)(PV_BOTH + (int)(x))

typedef enum
{
    PV_NONE = 0
    , PV_AI
    , PV_AR
    , PV_BH
    , PV_BIN
    , PV_BL
    , PV_BOMB
    , PV_BT
    , PV_CIN
    , PV_CINK
    , PV_CINO
    , PV_CINW
    , PV_CMS
    , PV_COM
    , PV_CPT
    , PV_DEF
    , PV_DICT
    , PV_DIFF
    , PV_EFM
    , PV_EOL
    , PV_EP
    , PV_ET
    , PV_FDC
    , PV_FDE
    , PV_FDI
    , PV_FDL
    , PV_FDM
    , PV_FDN
    , PV_FDT
    , PV_FEN
    , PV_FENC
    , PV_FF
    , PV_FML
    , PV_FMR
    , PV_FO
    , PV_FT
    , PV_GP
    , PV_IMI
    , PV_IMS
    , PV_INC
    , PV_INDE
    , PV_INDK
    , PV_INEX
    , PV_INF
    , PV_ISK
    , PV_KEY
    , PV_KMAP
    , PV_LBR
    , PV_LISP
    , PV_LIST
    , PV_MA
    , PV_ML
    , PV_MOD
    , PV_MP
    , PV_MPS
    , PV_NF
    , PV_NU
    , PV_OFT
    , PV_PATH
    , PV_PVW
    , PV_RL
    , PV_RO
    , PV_SCBIND
    , PV_SCROLL
    , PV_SI
    , PV_SN
    , PV_STS
    , PV_SUA
    , PV_SW
    , PV_SWF
    , PV_SYN
    , PV_TAGS
    , PV_TS
    , PV_TSR
    , PV_TW
    , PV_TX
    , PV_WFH
    , PV_WM
    , PV_WRAP
} idopt_T;

/*
 * Options local to a window have a value local to a buffer and global to all
 * buffers.  Indicate this by setting "var" to VAR_WIN.
 */
#define VAR_WIN ((char_u *)-1)

/*
 * These the global values for options which are also local to a buffer.
 * Only to be used in option.c!
 */
static int	p_ai;
static int	p_bin;
#ifdef FEAT_MBYTE
static int	p_bomb;
#endif
#if defined(FEAT_QUICKFIX)
static char_u	*p_bh;
static char_u	*p_bt;
#endif
static int	p_bl;
#ifdef FEAT_CINDENT
static int	p_cin;
static char_u	*p_cink;
static char_u	*p_cino;
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
static char_u	*p_cinw;
#endif
#ifdef FEAT_COMMENTS
static char_u	*p_com;
#endif
#ifdef FEAT_FOLDING
static char_u	*p_cms;
#endif
#ifdef FEAT_INS_EXPAND
static char_u	*p_cpt;
#endif
static int	p_eol;
static int	p_et;
#ifdef FEAT_MBYTE
static char_u	*p_fenc;
#endif
static char_u	*p_ff;
static char_u	*p_fo;
#ifdef FEAT_AUTOCMD
static char_u	*p_ft;
#endif
static long	p_iminsert;
static long	p_imsearch;
#if defined(FEAT_FIND_ID) && defined(FEAT_EVAL)
static char_u	*p_inex;
#endif
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
static char_u	*p_inde;
static char_u	*p_indk;
#endif
static int	p_inf;
static char_u	*p_isk;
#ifdef FEAT_CRYPT
static char_u	*p_key;
#endif
#ifdef FEAT_LISP
static int	p_lisp;
#endif
static int	p_ml;
static int	p_ma;
static int	p_mod;
static char_u	*p_mps;
static char_u	*p_nf;
#ifdef FEAT_OSFILETYPE
static char_u	*p_oft;
#endif
static int	p_ro;
#ifdef FEAT_SMARTINDENT
static int	p_si;
#endif
#ifndef SHORT_FNAME
static int	p_sn;
#endif
static long	p_sts;
#if defined(FEAT_SEARCHPATH)
static char_u	*p_sua;
#endif
static long	p_sw;
static int	p_swf;
#ifdef FEAT_SYN_HL
static char_u	*p_syn;
#endif
static long	p_ts;
static long	p_tw;
static int	p_tx;
static long	p_wm;
#ifdef FEAT_KEYMAP
static char_u	*p_keymap;
#endif

/* Saved values for when 'bin' is set. */
static int	p_et_nobin;
static int	p_ml_nobin;
static long	p_tw_nobin;
static long	p_wm_nobin;

/* Saved values for when 'paste' is set */
static long	p_tw_nopaste;
static long	p_wm_nopaste;
static long	p_sts_nopaste;
static int	p_ai_nopaste;

struct vimoption
{
    char	*fullname;	/* full option name */
    char	*shortname;	/* permissible abbreviation */
    long_u	flags;		/* see below */
    char_u	*var;		/* global option: pointer to variable;
				 * window-local option: VAR_WIN;
				 * buffer-local option: global value */
    idopt_T	indir;		/* global option: PV_NONE;
				 * local option: indirect option index */
    char_u	*def_val[2];	/* default values for variable (vi and vim) */
#ifdef FEAT_EVAL
    scid_T	scriptID;	/* script in which the option was last set */
#endif
};

#define VI_DEFAULT  0	    /* def_val[VI_DEFAULT] is Vi default value */
#define VIM_DEFAULT 1	    /* def_val[VIM_DEFAULT] is Vim default value */

/*
 * Flags
 */
#define P_BOOL		0x01	/* the option is boolean */
#define P_NUM		0x02	/* the option is numeric */
#define P_STRING	0x04	/* the option is a string */
#define P_ALLOCED	0x08	/* the string option is in allocated memory,
				    must use vim_free() when assigning new
				    value. Not set if default is the same. */
#define P_EXPAND	0x10	/* environment expansion.  NOTE: P_EXPAND can
				   never be used for local or hidden options! */
#define P_NODEFAULT	0x40	/* don't set to default value */
#define P_DEF_ALLOCED	0x80	/* default value is in allocated memory, must
				    use vim_free() when assigning new value */
#define P_WAS_SET	0x100	/* option has been set/reset */
#define P_NO_MKRC	0x200	/* don't include in :mkvimrc output */
#define P_VI_DEF	0x400	/* Use Vi default for Vim */
#define P_VIM		0x800	/* Vim option, reset when 'cp' set */

				/* when option changed, what to display: */
#define P_RSTAT		0x1000	/* redraw status lines */
#define P_RWIN		0x2000	/* redraw current window */
#define P_RBUF		0x4000	/* redraw current buffer */
#define P_RALL		0x6000	/* redraw all windows */
#define P_RCLR		0x7000	/* clear and redraw all */

#define P_COMMA		0x8000	/* comma separated list */
#define P_NODUP		0x10000L/* don't allow duplicate strings */
#define P_FLAGLIST	0x20000L/* list of single-char flags */

#define P_SECURE	0x40000L/* cannot change in modeline or secure mode */

/*
 * options[] is initialized here.
 * The order of the options MUST be alphabetic for ":set all" and findoption().
 * All option names MUST start with a lowercase letter (for findoption()).
 * Exception: "t_" options are at the end.
 * The options with a NULL variable are 'hidden': a set command for them is
 * ignored and they are not printed.
 */
static struct vimoption
#ifdef FEAT_GUI_W16
	_far
#endif
	options[] =
{
    {"aleph",	    "al",   P_NUM|P_VI_DEF,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)&p_aleph, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {
#if (defined(MSDOS) || defined(WIN3264) || defined(OS2)) && !defined(FEAT_GUI_W32)
			    (char_u *)128L,
#else
			    (char_u *)224L,
#endif
					    (char_u *)0L}},
    {"allowrevins", "ari",  P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)&p_ari, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"altkeymap",   "akm",  P_BOOL|P_VI_DEF,
#ifdef FEAT_FKMAP
			    (char_u *)&p_altkeymap, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"autoindent",  "ai",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ai, PV_AI,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"autoprint",   "ap",   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"autoread",    "ar",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ar, OPT_BOTH(PV_AR),
			    {(char_u *)FALSE, (char_u *)0L}},
    {"autowrite",   "aw",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_aw, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"autowriteall","awa",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_awa, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"background",  "bg",   P_STRING|P_VI_DEF|P_RCLR,
			    (char_u *)&p_bg, PV_NONE,
			    {
#if (defined(MSDOS) || defined(OS2) || defined(WIN3264)) && !defined(FEAT_GUI)
			    (char_u *)"dark",
#else
			    (char_u *)"light",
#endif
					    (char_u *)0L}},
    {"backspace",   "bs",   P_STRING|P_VI_DEF|P_VIM|P_COMMA|P_NODUP,
			    (char_u *)&p_bs, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"backup",	    "bk",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_bk, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"backupcopy",  "bkc",  P_STRING|P_VIM,
			    (char_u *)&p_bkc, PV_NONE,
#ifdef UNIX
			    {(char_u *)"yes", (char_u *)"auto"}
#else
			    {(char_u *)"auto", (char_u *)"auto"}
#endif
			    },
    {"backupdir",   "bdir", P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP|P_SECURE,
			    (char_u *)&p_bdir, PV_NONE,
			    {(char_u *)DFLT_BDIR, (char_u *)0L}},
    {"backupext",   "bex",  P_STRING|P_VI_DEF,
			    (char_u *)&p_bex, PV_NONE,
			    {
#ifdef VMS
			    (char_u *)"_",
#else
			    (char_u *)"~",
#endif
					    (char_u *)0L}},
    {"backupskip",  "bsk",  P_STRING|P_VI_DEF|P_COMMA,
#ifdef FEAT_WILDIGN
			    (char_u *)&p_bsk, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
#ifdef FEAT_BEVAL
    {"balloondelay","bdlay",P_NUM|P_VI_DEF,
			    (char_u *)&p_bdlay, PV_NONE,
			    {(char_u *)600L, (char_u *)0L}},
#endif
#ifdef FEAT_SUN_WORKSHOP
    {"ballooneval", "beval",P_BOOL|P_VI_DEF|P_NO_MKRC,
			    (char_u *)&p_beval, PV_NONE,
			    {(char_u*)FALSE, (char_u *)0L}},
#endif
    {"beautify",    "bf",   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"binary",	    "bin",  P_BOOL|P_VI_DEF|P_RSTAT,
			    (char_u *)&p_bin, PV_BIN,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"bioskey",	    "biosk",P_BOOL|P_VI_DEF,
#ifdef MSDOS
			    (char_u *)&p_biosk, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)TRUE, (char_u *)0L}},
    {"bomb",	    NULL,   P_BOOL|P_NO_MKRC|P_VI_DEF|P_RSTAT,
#ifdef FEAT_MBYTE
			    (char_u *)&p_bomb, PV_BOMB,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"breakat",	    "brk",  P_STRING|P_VI_DEF|P_RALL|P_FLAGLIST,
#ifdef FEAT_LINEBREAK
			    (char_u *)&p_breakat, PV_NONE,
			    {(char_u *)" \t!@*-+;:,./?", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"browsedir",   "bsdir",P_STRING|P_VI_DEF,
#ifdef FEAT_BROWSE
			    (char_u *)&p_bsdir, PV_NONE,
			    {(char_u *)"last", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"bufhidden",   "bh",   P_STRING|P_ALLOCED|P_VI_DEF,
#if defined(FEAT_QUICKFIX)
			    (char_u *)&p_bh, PV_BH,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"buflisted",   "bl",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_bl, PV_BL,
			    {(char_u *)1L, (char_u *)0L}
			    },
    {"buftype",	    "bt",   P_STRING|P_ALLOCED|P_VI_DEF,
#if defined(FEAT_QUICKFIX)
			    (char_u *)&p_bt, PV_BT,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"cdpath",	    "cd",   P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_SEARCHPATH
			    (char_u *)&p_cdpath, PV_NONE,
			    {(char_u *)",,", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"cedit",	    NULL,   P_STRING,
#ifdef FEAT_CMDWIN
			    (char_u *)&p_cedit, PV_NONE,
			    {(char_u *)"", (char_u *)CTRL_F_STR}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"charconvert",  "ccv", P_STRING|P_VI_DEF|P_SECURE,
#if defined(FEAT_MBYTE) && defined(FEAT_EVAL)
			    (char_u *)&p_ccv, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"cindent",	    "cin",  P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_CINDENT
			    (char_u *)&p_cin, PV_CIN,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"cinkeys",	    "cink", P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_CINDENT
			    (char_u *)&p_cink, PV_CINK,
			    {(char_u *)"0{,0},0),:,0#,!^F,o,O,e", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"cinoptions",  "cino", P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_CINDENT
			    (char_u *)&p_cino, PV_CINO,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"cinwords",    "cinw", P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
			    (char_u *)&p_cinw, PV_CINW,
			    {(char_u *)"if,else,while,do,for,switch",
				(char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"clipboard",   "cb",   P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_CLIPBOARD
			    (char_u *)&p_cb, PV_NONE,
# ifdef FEAT_XCLIPBOARD
			    {(char_u *)"autoselect,exclude:cons\\|linux",
							       (char_u *)0L}},
# else
			    {(char_u *)"", (char_u *)0L}},
# endif
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
#endif
    {"cmdheight",   "ch",   P_NUM|P_VI_DEF|P_RALL,
			    (char_u *)&p_ch, PV_NONE,
			    {(char_u *)1L, (char_u *)0L}},
    {"cmdwinheight", "cwh", P_NUM|P_VI_DEF,
#ifdef FEAT_CMDWIN
			    (char_u *)&p_cwh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)7L, (char_u *)0L}},
    {"columns",	    "co",   P_NUM|P_NODEFAULT|P_NO_MKRC|P_VI_DEF|P_RCLR,
			    (char_u *)&Columns, PV_NONE,
			    {(char_u *)80L, (char_u *)0L}},
    {"comments",    "com",  P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_COMMENTS
			    (char_u *)&p_com, PV_COM,
			    {(char_u *)"s1:/*,mb:*,ex:*/,://,b:#,:%,:XCOMM,n:>,fb:-",
				(char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"commentstring", "cms", P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_FOLDING
			    (char_u *)&p_cms, PV_CMS,
			    {(char_u *)"/*%s*/", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"compatible",  "cp",   P_BOOL|P_RALL,
			    (char_u *)&p_cp, PV_NONE,
			    {(char_u *)TRUE, (char_u *)FALSE}},
    {"complete",    "cpt",  P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_INS_EXPAND
			    (char_u *)&p_cpt, PV_CPT,
			    {(char_u *)".,w,b,u,t,i", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"confirm",     "cf",   P_BOOL|P_VI_DEF,
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
			    (char_u *)&p_confirm, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"conskey",	    "consk",P_BOOL|P_VI_DEF,
#ifdef MSDOS
			    (char_u *)&p_consk, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"cpoptions",   "cpo",  P_STRING|P_VIM|P_RALL|P_FLAGLIST,
			    (char_u *)&p_cpo, PV_NONE,
			    {(char_u *)CPO_ALL, (char_u *)CPO_DEFAULT}},
    {"cscopepathcomp", "cspc", P_NUM|P_VI_DEF|P_VIM,
#ifdef FEAT_CSCOPE
			    (char_u *)&p_cspc, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)0L, (char_u *)0L}},
    {"cscopeprg",   "csprg", P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_CSCOPE
			    (char_u *)&p_csprg, PV_NONE,
			    {(char_u *)"cscope", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"cscopetag",   "cst",  P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_CSCOPE
			    (char_u *)&p_cst, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)0L, (char_u *)0L}},
    {"cscopetagorder", "csto", P_NUM|P_VI_DEF|P_VIM,
#ifdef FEAT_CSCOPE
			    (char_u *)&p_csto, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)0L, (char_u *)0L}},
    {"cscopeverbose", "csverb", P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_CSCOPE
			    (char_u *)&p_csverbose, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)0L, (char_u *)0L}},
    {"debug",	    NULL,   P_STRING|P_VI_DEF,
			    (char_u *)&p_debug, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"define",	    "def",  P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_FIND_ID
			    (char_u *)&p_def, OPT_BOTH(PV_DEF),
			    {(char_u *)"^#\\s*define", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"delcombine", "deco",  P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_MBYTE
			    (char_u *)&p_deco, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}
			    },
    {"dictionary",  "dict", P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_INS_EXPAND
			    (char_u *)&p_dict, OPT_BOTH(PV_DICT),
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"diff",	    NULL,   P_BOOL|P_VI_DEF|P_RWIN,
#ifdef FEAT_DIFF
			    (char_u *)VAR_WIN, PV_DIFF,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"diffexpr",    "dex",  P_STRING|P_VI_DEF|P_SECURE,
#if defined(FEAT_DIFF) && defined(FEAT_EVAL)
			    (char_u *)&p_dex, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"diffopt",	    "dip",  P_STRING|P_ALLOCED|P_VI_DEF|P_RWIN|P_COMMA|P_NODUP,
#ifdef FEAT_DIFF
			    (char_u *)&p_dip, PV_NONE,
			    {(char_u *)"filler", (char_u *)NULL}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)"", (char_u *)NULL}
#endif
			    },
    {"digraph",	    "dg",   P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_DIGRAPHS
			    (char_u *)&p_dg, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"directory",   "dir",  P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP|P_SECURE,
			    (char_u *)&p_dir, PV_NONE,
			    {(char_u *)DFLT_DIR, (char_u *)0L}},
    {"display",	    "dy",   P_STRING|P_VI_DEF|P_COMMA|P_RALL|P_NODUP,
			    (char_u *)&p_dy, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"eadirection", "ead",  P_STRING|P_VI_DEF,
#ifdef FEAT_VERTSPLIT
			    (char_u *)&p_ead, PV_NONE,
			    {(char_u *)"both", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"edcompatible","ed",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ed, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"encoding",    "enc",  P_STRING|P_VI_DEF|P_RCLR,
#ifdef FEAT_MBYTE
			    (char_u *)&p_enc, PV_NONE,
			    {(char_u *)ENC_DFLT, (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"endofline",   "eol",  P_BOOL|P_NO_MKRC|P_VI_DEF|P_RSTAT,
			    (char_u *)&p_eol, PV_EOL,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"equalalways", "ea",   P_BOOL|P_VI_DEF|P_RALL,
			    (char_u *)&p_ea, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"equalprg",    "ep",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
			    (char_u *)&p_ep, OPT_BOTH(PV_EP),
			    {(char_u *)"", (char_u *)0L}},
    {"errorbells",  "eb",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_eb, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"errorfile",   "ef",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_ef, PV_NONE,
			    {(char_u *)DFLT_ERRORFILE, (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"errorformat", "efm",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_efm, OPT_BOTH(PV_EFM),
			    {(char_u *)DFLT_EFM, (char_u *)0L},
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"esckeys",	    "ek",   P_BOOL|P_VIM,
			    (char_u *)&p_ek, PV_NONE,
			    {(char_u *)FALSE, (char_u *)TRUE}},
    {"eventignore", "ei",   P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_AUTOCMD
			    (char_u *)&p_ei, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"expandtab",   "et",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_et, PV_ET,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"exrc",	    "ex",   P_BOOL|P_VI_DEF|P_SECURE,
			    (char_u *)&p_exrc, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"fileencoding","fenc", P_STRING|P_ALLOCED|P_VI_DEF|P_RSTAT|P_RBUF|P_NO_MKRC,
#ifdef FEAT_MBYTE
			    (char_u *)&p_fenc, PV_FENC,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"fileencodings","fencs", P_STRING|P_VI_DEF|P_COMMA,
#ifdef FEAT_MBYTE
			    (char_u *)&p_fencs, PV_NONE,
			    {(char_u *)"ucs-bom", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"fileformat",  "ff",   P_STRING|P_ALLOCED|P_VI_DEF|P_RSTAT|P_NO_MKRC,
			    (char_u *)&p_ff, PV_FF,
			    {(char_u *)DFLT_FF, (char_u *)0L}},
    {"fileformats", "ffs",  P_STRING|P_VIM|P_COMMA|P_NODUP,
			    (char_u *)&p_ffs, PV_NONE,
			    {(char_u *)DFLT_FFS_VI, (char_u *)DFLT_FFS_VIM}},
    {"filetype",    "ft",   P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_AUTOCMD
			    (char_u *)&p_ft, PV_FT,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"fillchars",   "fcs",  P_STRING|P_VI_DEF|P_RALL|P_COMMA|P_NODUP,
#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
			    (char_u *)&p_fcs, PV_NONE,
			    {(char_u *)"vert:|,fold:-", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#endif
			    },
    {"fkmap",	    "fk",   P_BOOL|P_VI_DEF,
#ifdef FEAT_FKMAP
			    (char_u *)&p_fkmap, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"flash",	    "fl",   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
#ifdef FEAT_FOLDING
    {"foldclose",   "fcl",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP|P_RWIN,
			    (char_u *)&p_fcl, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"foldcolumn",  "fdc",  P_NUM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FDC,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"foldenable",  "fen",  P_BOOL|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FEN,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"foldexpr",    "fde",  P_STRING|P_ALLOCED|P_VIM|P_VI_DEF|P_RWIN,
# ifdef FEAT_EVAL
			    (char_u *)VAR_WIN, PV_FDE,
			    {(char_u *)"0", (char_u *)NULL}
# else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
# endif
			    },
    {"foldignore",  "fdi",  P_STRING|P_ALLOCED|P_VIM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FDI,
			    {(char_u *)"#", (char_u *)NULL}},
    {"foldlevel",   "fdl",  P_NUM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FDL,
			    {(char_u *)0L, (char_u *)0L}},
    {"foldlevelstart","fdls", P_NUM|P_VI_DEF,
			    (char_u *)&p_fdls, PV_NONE,
			    {(char_u *)-1L, (char_u *)0L}},
    {"foldmarker",  "fmr",  P_STRING|P_ALLOCED|P_VIM|P_VI_DEF|
						       P_RWIN|P_COMMA|P_NODUP,
			    (char_u *)VAR_WIN, PV_FMR,
			    {(char_u *)"{{{,}}}", (char_u *)NULL}},
    {"foldmethod",  "fdm",  P_STRING|P_ALLOCED|P_VIM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FDM,
			    {(char_u *)"manual", (char_u *)NULL}},
    {"foldminlines","fml",  P_NUM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FML,
			    {(char_u *)1L, (char_u *)0L}},
    {"foldnestmax", "fdn",  P_NUM|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_FDN,
			    {(char_u *)20L, (char_u *)0L}},
    {"foldopen",    "fdo",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_fdo, PV_NONE,
		 {(char_u *)"block,hor,mark,percent,quickfix,search,tag,undo",
							       (char_u *)0L}},
    {"foldtext",    "fdt",  P_STRING|P_ALLOCED|P_VIM|P_VI_DEF|P_RWIN,
# ifdef FEAT_EVAL
			    (char_u *)VAR_WIN, PV_FDT,
			    {(char_u *)"foldtext()", (char_u *)NULL}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
#endif
    {"formatoptions","fo",  P_STRING|P_ALLOCED|P_VIM|P_FLAGLIST,
			    (char_u *)&p_fo, PV_FO,
			    {(char_u *)DFLT_FO_VI, (char_u *)DFLT_FO_VIM}},
    {"formatprg",   "fp",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
			    (char_u *)&p_fp, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"gdefault",    "gd",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_gd, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"graphic",	    "gr",   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"grepformat",  "gfm",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_gefm, PV_NONE,
			    {(char_u *)DFLT_GREPFORMAT, (char_u *)0L},
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"grepprg",	    "gp",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_gp, OPT_BOTH(PV_GP),
			    {
# ifdef WIN3264
			    /* may be changed to "grep -n" in os_win32.c */
			    (char_u *)"findstr /n",
# else
#  ifdef UNIX
			    /* Add an extra file name so that grep will always
			     * insert a file name in the match line. */
			    (char_u *)"grep -n $* /dev/null",
#  else
#   ifdef VMS
			    (char_u *)"SEARCH/NUMBERS ",
#   else
			    (char_u *)"grep -n ",
#endif
#endif
# endif
			    (char_u *)0L},
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"guicursor",    "gcr",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef CURSOR_SHAPE
			    (char_u *)&p_guicursor, PV_NONE,
			    {
# ifdef FEAT_GUI
				(char_u *)"n-v-c:block-Cursor/lCursor,ve:ver35-Cursor,o:hor50-Cursor,i-ci:ver25-Cursor/lCursor,r-cr:hor20-Cursor/lCursor,sm:block-Cursor-blinkwait175-blinkoff150-blinkon175",
# else	/* MSDOS or Win32 console */
				(char_u *)"n-v-c:block,o:hor50,i-ci:hor15,r-cr:hor30,sm:block",
# endif
				    (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
				    },
    {"guifont",	    "gfn",  P_STRING|P_VI_DEF|P_RCLR|P_COMMA|P_NODUP,
#ifdef FEAT_GUI
			    (char_u *)&p_guifont, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
				    },
    {"guifontset",  "gfs",  P_STRING|P_VI_DEF|P_RCLR|P_COMMA,
#if defined(FEAT_GUI) && defined(FEAT_XFONTSET)
			    (char_u *)&p_guifontset, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
				    },
    {"guifontwide", "gfw",  P_STRING|P_VI_DEF|P_RCLR|P_COMMA|P_NODUP,
#if defined(FEAT_GUI) && defined(FEAT_MBYTE)
			    (char_u *)&p_guifontwide, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
				    },
    {"guiheadroom", "ghr",  P_NUM|P_VI_DEF,
#if defined(FEAT_GUI_GTK) || defined(FEAT_GUI_X11)
			    (char_u *)&p_ghr, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)50L, (char_u *)0L}},
    {"guioptions",  "go",   P_STRING|P_VI_DEF|P_RALL|P_FLAGLIST,
#if defined(FEAT_GUI)
			    (char_u *)&p_go, PV_NONE,
# if defined(UNIX) && !defined(MACOS)
			    {(char_u *)"agimrLtT", (char_u *)0L}
# else
			    {(char_u *)"gmrLtT", (char_u *)0L}
# endif
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
				    },
    {"guipty",	    NULL,   P_BOOL|P_VI_DEF,
#if defined(FEAT_GUI)
			    (char_u *)&p_guipty, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)TRUE, (char_u *)0L}},
    {"hardtabs",    "ht",   P_NUM|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"helpfile",    "hf",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
			    (char_u *)&p_hf, PV_NONE,
			    {(char_u *)DFLT_HELPFILE, (char_u *)0L}},
    {"helpheight",  "hh",   P_NUM|P_VI_DEF,
#ifdef FEAT_WINDOWS
			    (char_u *)&p_hh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)20L, (char_u *)0L}},
    {"hidden",	    "hid",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_hid, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"highlight",   "hl",   P_STRING|P_VI_DEF|P_RCLR|P_COMMA|P_NODUP,
			    (char_u *)&p_hl, PV_NONE,
			    {(char_u *)"8:SpecialKey,@:NonText,d:Directory,e:ErrorMsg,i:IncSearch,l:Search,m:MoreMsg,M:ModeMsg,n:LineNr,r:Question,s:StatusLine,S:StatusLineNC,c:VertSplit,t:Title,v:Visual,V:VisualNOS,w:WarningMsg,W:WildMenu,f:Folded,F:FoldColumn,A:DiffAdd,C:DiffChange,D:DiffDelete,T:DiffText",
				(char_u *)0L}},
    {"history",	    "hi",   P_NUM|P_VIM,
			    (char_u *)&p_hi, PV_NONE,
			    {(char_u *)0L, (char_u *)20L}},
    {"hkmap",	    "hk",   P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)&p_hkmap, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"hkmapp",	    "hkp",  P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)&p_hkmapp, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"hlsearch",    "hls",  P_BOOL|P_VI_DEF|P_VIM|P_RALL,
			    (char_u *)&p_hls, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"icon",	    NULL,   P_BOOL|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_icon, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"iconstring",  NULL,   P_STRING|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_iconstring, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"ignorecase",  "ic",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ic, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"imactivatekey","imak",P_STRING|P_VI_DEF,
#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
			    (char_u *)&p_imak, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"imcmdline",   "imc",  P_BOOL|P_VI_DEF,
#ifdef USE_IM_CONTROL
			    (char_u *)&p_imcmdline, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"imdisable",   "imd",  P_BOOL|P_VI_DEF,
#ifdef USE_IM_CONTROL
			    (char_u *)&p_imdisable, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
#ifdef __sgi
			    {(char_u *)TRUE, (char_u *)0L}},
#else
			    {(char_u *)FALSE, (char_u *)0L}},
#endif
    {"iminsert",    "imi",  P_NUM|P_VI_DEF,
			    (char_u *)&p_iminsert, PV_IMI,
#ifdef B_IMODE_IM
			    {(char_u *)B_IMODE_IM, (char_u *)0L}
#else
			    {(char_u *)B_IMODE_NONE, (char_u *)0L}
#endif
			    },
    {"imsearch",    "ims",  P_NUM|P_VI_DEF,
			    (char_u *)&p_imsearch, PV_IMS,
#ifdef B_IMODE_IM
			    {(char_u *)B_IMODE_IM, (char_u *)0L}
#else
			    {(char_u *)B_IMODE_NONE, (char_u *)0L}
#endif
			    },
    {"include",	    "inc",  P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_FIND_ID
			    (char_u *)&p_inc, OPT_BOTH(PV_INC),
			    {(char_u *)"^#\\s*include", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"includeexpr", "inex", P_STRING|P_ALLOCED|P_VI_DEF,
#if defined(FEAT_FIND_ID) && defined(FEAT_EVAL)
			    (char_u *)&p_inex, PV_INEX,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"incsearch",   "is",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_is, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"indentexpr", "inde",  P_STRING|P_ALLOCED|P_VI_DEF|P_VIM,
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
			    (char_u *)&p_inde, PV_INDE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"indentkeys", "indk",  P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
			    (char_u *)&p_indk, PV_INDK,
			    {(char_u *)"0{,0},:,0#,!^F,o,O,e", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"infercase",   "inf",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_inf, PV_INF,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"insertmode",  "im",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_im, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"isfname",	    "isf",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_isf, PV_NONE,
			    {
#ifdef BACKSLASH_IN_FILENAME
				/* Excluded are: & and ^ are special in cmd.exe
				 * ( and ) are used in text separating fnames */
			    (char_u *)"@,48-57,/,\\,.,-,_,+,,,#,$,%,{,},[,],:,@-@,!,~,=",
#else
# ifdef AMIGA
			    (char_u *)"@,48-57,/,.,-,_,+,,,$,:",
# else
#  ifdef VMS
			    (char_u *)"@,48-57,/,.,-,_,+,,,#,$,%,<,>,[,],:,;,~",
#  else /* UNIX et al. */
#   ifdef EBCDIC
			    (char_u *)"@,240-249,/,.,-,_,+,,,#,$,%,~,=",
#   else
			    (char_u *)"@,48-57,/,.,-,_,+,,,#,$,%,~,=",
#   endif
#  endif
# endif
#endif
				(char_u *)0L}},
    {"isident",	    "isi",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_isi, PV_NONE,
			    {
#if defined(MSDOS) || defined(MSWIN) || defined(OS2)
			    (char_u *)"@,48-57,_,128-167,224-235",
#else
# ifdef EBCDIC
			    /* TODO: EBCDIC Check this! @ == isalpha()*/
			    (char_u *)"@,240-249,_,66-73,81-89,98-105,"
				    "112-120,128,140-142,156,158,172,"
				    "174,186,191,203-207,219-225,235-239,"
				    "251-254",
# else
			    (char_u *)"@,48-57,_,192-255",
# endif
#endif
				(char_u *)0L}},
    {"iskeyword",   "isk",  P_STRING|P_ALLOCED|P_VIM|P_COMMA|P_NODUP,
			    (char_u *)&p_isk, PV_ISK,
			    {
#ifdef EBCDIC
			     (char_u *)"@,240-249,_",
			     /* TODO: EBCDIC Check this! @ == isalpha()*/
			     (char_u *)"@,240-249,_,66-73,81-89,98-105,"
				    "112-120,128,140-142,156,158,172,"
				    "174,186,191,203-207,219-225,235-239,"
				    "251-254",
#else
				(char_u *)"@,48-57,_",
# if defined(MSDOS) || defined(MSWIN) || defined(OS2)
				(char_u *)"@,48-57,_,128-167,224-235"
# else
				(char_u *)"@,48-57,_,192-255"
# endif
#endif
				}},
    {"isprint",	    "isp",  P_STRING|P_VI_DEF|P_RALL|P_COMMA|P_NODUP,
			    (char_u *)&p_isp, PV_NONE,
			    {
#if defined(MSDOS) || defined(MSWIN) || defined(OS2) || defined(MACOS) \
		|| defined(VMS)
			    (char_u *)"@,~-255",
#else
# ifdef EBCDIC
			    /* all chars above 63 are printable */
			    (char_u *)"63-255",
# else
			    (char_u *)"@,161-255",
# endif
#endif
				(char_u *)0L}},
    {"joinspaces",  "js",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_js, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"key",	    NULL,   P_STRING|P_ALLOCED|P_VI_DEF|P_NO_MKRC,
#ifdef FEAT_CRYPT
			    (char_u *)&p_key, PV_KEY,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"keymap",	    "kmp",  P_STRING|P_ALLOCED|P_VI_DEF|P_RBUF|P_RSTAT,
#ifdef FEAT_KEYMAP
			    (char_u *)&p_keymap, PV_KMAP,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#endif
	},
    {"keymodel",    "km",   P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_VISUAL
			    (char_u *)&p_km, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"keywordprg",  "kp",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
			    (char_u *)&p_kp, PV_NONE,
			    {
#if defined(MSDOS) || defined(MSWIN)
			    (char_u *)"",
#else
#ifdef VMS
			    (char_u *)"help",
#else
# if defined(OS2)
			    (char_u *)"view /",
# else
#  ifdef USEMAN_S
			    (char_u *)"man -s",
#  else
			    (char_u *)"man",
#  endif
# endif
#endif
#endif
				(char_u *)0L}},
    {"langmap",     "lmap", P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_LANGMAP
			    (char_u *)&p_langmap, PV_NONE,
			    {(char_u *)"",	/* unmatched } */
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL,
#endif
				(char_u *)0L}},
    {"langmenu",    "lm",   P_STRING|P_VI_DEF,
#if defined(FEAT_MENU) && defined(FEAT_MULTI_LANG)
			    (char_u *)&p_lm, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"laststatus",  "ls",   P_NUM|P_VI_DEF|P_RALL,
#ifdef FEAT_WINDOWS
			    (char_u *)&p_ls, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)1L, (char_u *)0L}},
    {"lazyredraw",  "lz",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_lz, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"linebreak",   "lbr",  P_BOOL|P_VI_DEF|P_RWIN,
#ifdef FEAT_LINEBREAK
			    (char_u *)VAR_WIN, PV_LBR,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"lines",	    NULL,   P_NUM|P_NODEFAULT|P_NO_MKRC|P_VI_DEF|P_RCLR,
			    (char_u *)&Rows, PV_NONE,
			    {
#if defined(MSDOS) || defined(WIN3264) || defined(OS2)
			    (char_u *)25L,
#else
			    (char_u *)24L,
#endif
					    (char_u *)0L}},
    {"linespace",   "lsp",  P_NUM|P_VI_DEF|P_RCLR,
#ifdef FEAT_GUI
			    (char_u *)&p_linespace, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
#ifdef FEAT_GUI_W32
			    {(char_u *)1L, (char_u *)0L}
#else
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"lisp",	    NULL,   P_BOOL|P_VI_DEF,
#ifdef FEAT_LISP
			    (char_u *)&p_lisp, PV_LISP,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"lispwords",   "lw",   P_STRING|P_VI_DEF,
#ifdef FEAT_LISP
			    (char_u *)&p_lispwords, PV_NONE,
			    {(char_u *)LISPWORD_VALUE, (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#endif
			    },
    {"list",	    NULL,   P_BOOL|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_LIST,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"listchars",   "lcs",  P_STRING|P_VI_DEF|P_RALL|P_COMMA|P_NODUP,
			    (char_u *)&p_lcs, PV_NONE,
			    {(char_u *)"eol:$", (char_u *)0L}},
    {"loadplugins", "lpl",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_lpl, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"magic",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_magic, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"makeef",	    "mef",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_mef, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"makeprg",	    "mp",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_mp, OPT_BOTH(PV_MP),
# ifdef VMS
			    {(char_u *)"MMS", (char_u *)0L}
# else
			    {(char_u *)"make", (char_u *)0L}
# endif
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"matchpairs",  "mps",  P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_mps, PV_MPS,
			    {(char_u *)"(:),{:},[:]", (char_u *)0L}},
    {"matchtime",   "mat",  P_NUM|P_VI_DEF,
			    (char_u *)&p_mat, PV_NONE,
			    {(char_u *)5L, (char_u *)0L}},
    {"maxfuncdepth", "mfd", P_NUM|P_VI_DEF,
#ifdef FEAT_EVAL
			    (char_u *)&p_mfd, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)100L, (char_u *)0L}},
    {"maxmapdepth", "mmd",  P_NUM|P_VI_DEF,
			    (char_u *)&p_mmd, PV_NONE,
			    {(char_u *)1000L, (char_u *)0L}},
    {"maxmem",	    "mm",   P_NUM|P_VI_DEF,
			    (char_u *)&p_mm, PV_NONE,
			    {(char_u *)DFLT_MAXMEM, (char_u *)0L}},
    {"maxmemtot",   "mmt",  P_NUM|P_VI_DEF,
			    (char_u *)&p_mmt, PV_NONE,
			    {(char_u *)DFLT_MAXMEMTOT, (char_u *)0L}},
    {"menuitems",   "mis",  P_NUM|P_VI_DEF,
#ifdef FEAT_MENU
			    (char_u *)&p_mis, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)25L, (char_u *)0L}},
    {"mesg",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"modeline",    "ml",   P_BOOL|P_VIM,
			    (char_u *)&p_ml, PV_ML,
			    {(char_u *)FALSE, (char_u *)TRUE}},
    {"modelines",   "mls",  P_NUM|P_VI_DEF,
			    (char_u *)&p_mls, PV_NONE,
			    {(char_u *)5L, (char_u *)0L}},
    {"modifiable",  "ma",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ma, PV_MA,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"modified",    "mod",  P_BOOL|P_NO_MKRC|P_VI_DEF|P_RSTAT,
			    (char_u *)&p_mod, PV_MOD,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"more",	    NULL,   P_BOOL|P_VIM,
			    (char_u *)&p_more, PV_NONE,
			    {(char_u *)FALSE, (char_u *)TRUE}},
    {"mouse",	    NULL,   P_STRING|P_VI_DEF|P_FLAGLIST,
			    (char_u *)&p_mouse, PV_NONE,
			    {
#if defined(MSDOS) || defined(WIN3264)
				(char_u *)"a",
#else
				(char_u *)"",
#endif
				(char_u *)0L}},
    {"mousefocus",   "mousef", P_BOOL|P_VI_DEF,
#ifdef FEAT_GUI
			    (char_u *)&p_mousef, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"mousehide",   "mh",   P_BOOL|P_VI_DEF,
#ifdef FEAT_GUI
			    (char_u *)&p_mh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)TRUE, (char_u *)0L}},
    {"mousemodel",  "mousem", P_STRING|P_VI_DEF,
			    (char_u *)&p_mousem, PV_NONE,
			    {
#if defined(MSDOS) || defined(MSWIN)
				(char_u *)"popup",
#else
# if defined(MACOS)
				(char_u *)"popup_setpos",
# else
				(char_u *)"extend",
# endif
#endif
				(char_u *)0L}},
    {"mouseshape",  "mouses",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_MOUSESHAPE
			    (char_u *)&p_mouseshape, PV_NONE,
			    {(char_u *)"i-r:beam,s:updown,sd:udsizing,vs:leftright,vd:lrsizing,m:no,ml:up-arrow,v:rightup-arrow", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"mousetime",   "mouset",	P_NUM|P_VI_DEF,
			    (char_u *)&p_mouset, PV_NONE,
			    {(char_u *)500L, (char_u *)0L}},
    {"novice",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"nrformats",   "nf",   P_STRING|P_ALLOCED|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_nf, PV_NF,
			    {(char_u *)"octal,hex", (char_u *)0L}},
    {"number",	    "nu",   P_BOOL|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_NU,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"open",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"optimize",    "opt",  P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"osfiletype",  "oft",  P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_OSFILETYPE
			    (char_u *)&p_oft, PV_OFT,
			    {(char_u *)DFLT_OFT, (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"paragraphs",  "para", P_STRING|P_VI_DEF,
			    (char_u *)&p_para, PV_NONE,
			    {(char_u *)"IPLPPPQPP LIpplpipbp", (char_u *)0L}},
    {"paste",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_paste, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"pastetoggle", "pt",   P_STRING|P_VI_DEF,
			    (char_u *)&p_pt, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"patchexpr",   "pex",  P_STRING|P_VI_DEF|P_SECURE,
#if defined(FEAT_DIFF) && defined(FEAT_EVAL)
			    (char_u *)&p_pex, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"patchmode",   "pm",   P_STRING|P_VI_DEF,
			    (char_u *)&p_pm, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"path",	    "pa",   P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_path, OPT_BOTH(PV_PATH),
			    {
#if defined AMIGA || defined MSDOS || defined MSWIN
			    (char_u *)".,,",
#else
# if defined(__EMX__)
			    (char_u *)".,/emx/include,,",
# else /* Unix, probably */
			    (char_u *)".,/usr/include,,",
# endif
#endif
				(char_u *)0L}},
    {"previewheight", "pvh",P_NUM|P_VI_DEF,
#if defined(FEAT_WINDOWS) && defined(FEAT_QUICKFIX)
			    (char_u *)&p_pvh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)12L, (char_u *)0L}},
    {"previewwindow", "pvw", P_BOOL|P_VI_DEF|P_RSTAT,
#if defined(FEAT_WINDOWS) && defined(FEAT_QUICKFIX)
			    (char_u *)VAR_WIN, PV_PVW,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"printdevice", "pdev", P_STRING|P_VI_DEF,
#ifdef FEAT_PRINTER
			    (char_u *)&p_pdev, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"printexpr", "pexpr",  P_STRING|P_VI_DEF,
#ifdef FEAT_POSTSCRIPT
			    (char_u *)&p_pexpr, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"printfont", "pfn",    P_STRING|P_VI_DEF,
#ifdef FEAT_PRINTER
			    (char_u *)&p_pfn, PV_NONE,
			    {
# ifdef MSWIN
				(char_u *)"Courier_New:h10",
# else
				(char_u *)"courier",
# endif
				(char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"printheader", "pheader",  P_STRING|P_VI_DEF,
#ifdef FEAT_PRINTER
			    (char_u *)&p_header, PV_NONE,
			    {(char_u *)"%<%f%h%m%=Page %N", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"printoptions", "popt", P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_PRINTER
			    (char_u *)&p_popt, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"prompt",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"readonly",    "ro",   P_BOOL|P_VI_DEF|P_RSTAT,
			    (char_u *)&p_ro, PV_RO,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"redraw",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"remap",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_remap, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"report",	    NULL,   P_NUM|P_VI_DEF,
			    (char_u *)&p_report, PV_NONE,
			    {(char_u *)2L, (char_u *)0L}},
    {"restorescreen", "rs", P_BOOL|P_VI_DEF,
#ifdef WIN3264
			    (char_u *)&p_rs, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)TRUE, (char_u *)0L}},
    {"revins",	    "ri",   P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)&p_ri, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"rightleft",   "rl",   P_BOOL|P_VI_DEF|P_RWIN,
#ifdef FEAT_RIGHTLEFT
			    (char_u *)VAR_WIN, PV_RL,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"ruler",	    "ru",   P_BOOL|P_VI_DEF|P_VIM|P_RSTAT,
#ifdef FEAT_CMDL_INFO
			    (char_u *)&p_ru, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"rulerformat", "ruf",  P_STRING|P_VI_DEF|P_ALLOCED|P_RSTAT,
#ifdef FEAT_STL_OPT
			    (char_u *)&p_ruf, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"runtimepath", "rtp",  P_STRING|P_VI_DEF|P_EXPAND|P_COMMA|P_NODUP|P_SECURE,
			    (char_u *)&p_rtp, PV_NONE,
			    {(char_u *)DFLT_RUNTIMEPATH, (char_u *)0L}},
    {"scroll",	    "scr",  P_NUM|P_NO_MKRC|P_VI_DEF,
			    (char_u *)VAR_WIN, PV_SCROLL,
			    {(char_u *)12L, (char_u *)0L}},
    {"scrollbind",  "scb",  P_BOOL|P_VI_DEF,
#ifdef FEAT_SCROLLBIND
			    (char_u *)VAR_WIN, PV_SCBIND,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"scrolljump",  "sj",   P_NUM|P_VI_DEF|P_VIM,
			    (char_u *)&p_sj, PV_NONE,
			    {(char_u *)1L, (char_u *)0L}},
    {"scrolloff",   "so",   P_NUM|P_VI_DEF|P_VIM|P_RALL,
			    (char_u *)&p_so, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"scrollopt",   "sbo",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_SCROLLBIND
			    (char_u *)&p_sbo, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"ver,jump", (char_u *)0L}},
    {"sections",    "sect", P_STRING|P_VI_DEF,
			    (char_u *)&p_sections, PV_NONE,
			    {(char_u *)"SHNHH HUnhsh", (char_u *)0L}},
    {"secure",	    NULL,   P_BOOL|P_VI_DEF|P_SECURE,
			    (char_u *)&p_secure, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"selection",   "sel",  P_STRING|P_VI_DEF,
#ifdef FEAT_VISUAL
			    (char_u *)&p_sel, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"inclusive", (char_u *)0L}},
    {"selectmode",  "slm",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_VISUAL
			    (char_u *)&p_slm, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"sessionoptions", "ssop", P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_SESSION
			    (char_u *)&p_ssop, PV_NONE,
		 {(char_u *)"blank,buffers,curdir,folds,help,options,winsize",
							       (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"shell",	    "sh",   P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
			    (char_u *)&p_sh, PV_NONE,
			    {
#ifdef VMS
			    (char_u *)"-",
#else
# if defined(MSDOS)
			    (char_u *)"command",
# else
#  if defined(WIN16)
			    (char_u *)"command.com",
#  else
#   if defined(WIN3264)
			    (char_u *)"",	/* set in set_init_1() */
#   else
#    if defined(OS2)
			    (char_u *)"cmd.exe",
#    else
#     if defined(ARCHIE)
			    (char_u *)"gos",
#     else
			    (char_u *)"sh",
#     endif
#    endif
#   endif
#  endif
# endif
#endif /* VMS */
				(char_u *)0L}},
    {"shellcmdflag","shcf", P_STRING|P_VI_DEF|P_SECURE,
			    (char_u *)&p_shcf, PV_NONE,
			    {
#if defined(MSDOS) || defined(MSWIN)
			    (char_u *)"/c",
#else
# if defined(OS2)
			    (char_u *)"/c",
# else
			    (char_u *)"-c",
# endif
#endif
				(char_u *)0L}},
    {"shellpipe",   "sp",   P_STRING|P_VI_DEF|P_SECURE,
#ifdef FEAT_QUICKFIX
			    (char_u *)&p_sp, PV_NONE,
			    {
#if defined(UNIX) || defined(OS2)
# ifdef ARCHIE
			    (char_u *)"2>",
# else
			    (char_u *)"| tee",
# endif
#else
			    (char_u *)">",
#endif
				(char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
    },
    {"shellquote",  "shq",  P_STRING|P_VI_DEF|P_SECURE,
			    (char_u *)&p_shq, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"shellredir",  "srr",  P_STRING|P_VI_DEF|P_SECURE,
			    (char_u *)&p_srr, PV_NONE,
			    {(char_u *)">", (char_u *)0L}},
    {"shellslash",  "ssl",   P_BOOL|P_VI_DEF,
#ifdef BACKSLASH_IN_FILENAME
			    (char_u *)&p_ssl, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"shelltype",   "st",   P_NUM|P_VI_DEF,
#ifdef AMIGA
			    (char_u *)&p_st, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)0L, (char_u *)0L}},
    {"shellxquote", "sxq",  P_STRING|P_VI_DEF|P_SECURE,
			    (char_u *)&p_sxq, PV_NONE,
			    {
#if defined(UNIX) && defined(USE_SYSTEM) && !defined(__EMX__)
			    (char_u *)"\"",
#else
			    (char_u *)"",
#endif
				(char_u *)0L}},
    {"shiftround",  "sr",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_sr, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"shiftwidth",  "sw",   P_NUM|P_VI_DEF,
			    (char_u *)&p_sw, PV_SW,
			    {(char_u *)8L, (char_u *)0L}},
    {"shortmess",   "shm",  P_STRING|P_VIM|P_FLAGLIST,
			    (char_u *)&p_shm, PV_NONE,
			    {(char_u *)"", (char_u *)"filnxtToO"}},
    {"shortname",   "sn",   P_BOOL|P_VI_DEF,
#ifdef SHORT_FNAME
			    (char_u *)NULL, PV_NONE,
#else
			    (char_u *)&p_sn, PV_SN,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"showbreak",   "sbr",  P_STRING|P_VI_DEF|P_RALL,
#ifdef FEAT_LINEBREAK
			    (char_u *)&p_sbr, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"showcmd",	    "sc",   P_BOOL|P_VIM,
#ifdef FEAT_CMDL_INFO
			    (char_u *)&p_sc, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE,
#ifdef UNIX
				(char_u *)FALSE
#else
				(char_u *)TRUE
#endif
				}},
    {"showfulltag", "sft",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_sft, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"showmatch",   "sm",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_sm, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"showmode",    "smd",  P_BOOL|P_VIM,
			    (char_u *)&p_smd, PV_NONE,
			    {(char_u *)FALSE, (char_u *)TRUE}},
    {"sidescroll",  "ss",   P_NUM|P_VI_DEF,
			    (char_u *)&p_ss, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"sidescrolloff", "siso", P_NUM|P_VI_DEF|P_VIM|P_RBUF,
			    (char_u *)&p_siso, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"slowopen",    "slow", P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"smartcase",   "scs",  P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_scs, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"smartindent", "si",   P_BOOL|P_VI_DEF|P_VIM,
#ifdef FEAT_SMARTINDENT
			    (char_u *)&p_si, PV_SI,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"smarttab",    "sta",  P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_sta, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"softtabstop", "sts",  P_NUM|P_VI_DEF|P_VIM,
			    (char_u *)&p_sts, PV_STS,
			    {(char_u *)0L, (char_u *)0L}},
    {"sourceany",   NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"splitbelow",  "sb",   P_BOOL|P_VI_DEF,
#ifdef FEAT_WINDOWS
			    (char_u *)&p_sb, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"splitright",  "spr",  P_BOOL|P_VI_DEF,
#ifdef FEAT_VERTSPLIT
			    (char_u *)&p_spr, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"startofline", "sol",  P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_sol, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"statusline"  ,"stl",  P_STRING|P_VI_DEF|P_ALLOCED|P_RSTAT,
#ifdef FEAT_STL_OPT
			    (char_u *)&p_stl, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"suffixes",    "su",   P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_su, PV_NONE,
			    {(char_u *)".bak,~,.o,.h,.info,.swp,.obj",
				(char_u *)0L}},
    {"suffixesadd", "sua",  P_STRING|P_VI_DEF|P_ALLOCED|P_COMMA|P_NODUP,
#if defined(FEAT_SEARCHPATH)
			    (char_u *)&p_sua, PV_SUA,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"swapfile",    "swf",  P_BOOL|P_VI_DEF|P_RSTAT,
			    (char_u *)&p_swf, PV_SWF,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"swapsync",    "sws",  P_STRING|P_VI_DEF,
			    (char_u *)&p_sws, PV_NONE,
			    {(char_u *)"fsync", (char_u *)0L}},
    {"switchbuf",   "swb",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_swb, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"syntax",	    "syn",  P_STRING|P_ALLOCED|P_VI_DEF,
#ifdef FEAT_SYN_HL
			    (char_u *)&p_syn, PV_SYN,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"tabstop",	    "ts",   P_NUM|P_VI_DEF|P_RBUF,
			    (char_u *)&p_ts, PV_TS,
			    {(char_u *)8L, (char_u *)0L}},
    {"tagbsearch",  "tbs",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_tbs, PV_NONE,
#ifdef VMS	/* binary searching doesn't appear to work on VMS */
			    {(char_u *)0L, (char_u *)0L}},
#else
			    {(char_u *)TRUE, (char_u *)0L}},
#endif
    {"taglength",   "tl",   P_NUM|P_VI_DEF,
			    (char_u *)&p_tl, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"tagrelative", "tr",   P_BOOL|P_VIM,
			    (char_u *)&p_tr, PV_NONE,
			    {(char_u *)FALSE, (char_u *)TRUE}},
    {"tags",	    "tag",  P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_tags, OPT_BOTH(PV_TAGS),
			    {
#if defined(FEAT_EMACS_TAGS) && !defined(CASE_INSENSITIVE_FILENAME)
			    (char_u *)"./tags,./TAGS,tags,TAGS",
#else
			    (char_u *)"./tags,tags",
#endif
				(char_u *)0L}},
    {"tagstack",    "tgst", P_BOOL|P_VI_DEF,
			    (char_u *)&p_tgst, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"term",	    NULL,   P_STRING|P_EXPAND|P_NODEFAULT|P_NO_MKRC|P_VI_DEF|P_RALL,
			    (char_u *)&T_NAME, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"termencoding", "tenc", P_STRING|P_VI_DEF|P_RCLR,
#ifdef FEAT_MBYTE
			    (char_u *)&p_tenc, PV_NONE,
			    {(char_u *)"", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"terse",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_terse, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"textauto",    "ta",   P_BOOL|P_VIM,
			    (char_u *)&p_ta, PV_NONE,
			    {(char_u *)DFLT_TEXTAUTO, (char_u *)TRUE}},
    {"textmode",    "tx",   P_BOOL|P_VI_DEF|P_NO_MKRC,
			    (char_u *)&p_tx, PV_TX,
			    {
#ifdef USE_CRNL
			    (char_u *)TRUE,
#else
			    (char_u *)FALSE,
#endif
				(char_u *)0L}},
    {"textwidth",   "tw",   P_NUM|P_VI_DEF|P_VIM,
			    (char_u *)&p_tw, PV_TW,
			    {(char_u *)0L, (char_u *)0L}},
    {"thesaurus",   "tsr",  P_STRING|P_EXPAND|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_INS_EXPAND
			    (char_u *)&p_tsr, OPT_BOTH(PV_TSR),
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"tildeop",	    "top",  P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_to, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"timeout",	    "to",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_timeout, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"timeoutlen",  "tm",   P_NUM|P_VI_DEF,
			    (char_u *)&p_tm, PV_NONE,
			    {(char_u *)1000L, (char_u *)0L}},
    {"title",	    NULL,   P_BOOL|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_title, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"titlelen",    NULL,   P_NUM|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_titlelen, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)85L, (char_u *)0L}},
    {"titleold",    NULL,   P_STRING|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_titleold, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)N_("Thanks for flying Vim"),
							       (char_u *)0L}},
    {"titlestring", NULL,   P_STRING|P_VI_DEF,
#ifdef FEAT_TITLE
			    (char_u *)&p_titlestring, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_W32)
    {"toolbar",     "tb",   P_STRING|P_COMMA|P_VI_DEF|P_NODUP,
			    (char_u *)&p_toolbar, PV_NONE,
			    {(char_u *)"icons,tooltips", (char_u *)0L}},
#endif
    {"ttimeout",    NULL,   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_ttimeout, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"ttimeoutlen", "ttm",  P_NUM|P_VI_DEF,
			    (char_u *)&p_ttm, PV_NONE,
			    {(char_u *)-1L, (char_u *)0L}},
    {"ttybuiltin",  "tbi",  P_BOOL|P_VI_DEF,
			    (char_u *)&p_tbi, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"ttyfast",	    "tf",   P_BOOL|P_NO_MKRC|P_VI_DEF,
			    (char_u *)&p_tf, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"ttymouse",    "ttym", P_STRING|P_NODEFAULT|P_NO_MKRC|P_VI_DEF,
#if defined(FEAT_MOUSE) && (defined(UNIX) || defined(VMS))
			    (char_u *)&p_ttym, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"ttyscroll",   "tsl",  P_NUM|P_VI_DEF,
			    (char_u *)&p_ttyscroll, PV_NONE,
			    {(char_u *)999L, (char_u *)0L}},
    {"ttytype",	    "tty",  P_STRING|P_EXPAND|P_NODEFAULT|P_NO_MKRC|P_VI_DEF|P_RALL,
			    (char_u *)&T_NAME, PV_NONE,
			    {(char_u *)"", (char_u *)0L}},
    {"undolevels",  "ul",   P_NUM|P_VI_DEF,
			    (char_u *)&p_ul, PV_NONE,
			    {
#if defined(UNIX) || defined(WIN3264) || defined(OS2) || defined(VMS)
			    (char_u *)1000L,
#else
			    (char_u *)100L,
#endif
				(char_u *)0L}},
    {"updatecount", "uc",   P_NUM|P_VI_DEF,
			    (char_u *)&p_uc, PV_NONE,
			    {(char_u *)200L, (char_u *)0L}},
    {"updatetime",  "ut",   P_NUM|P_VI_DEF,
			    (char_u *)&p_ut, PV_NONE,
			    {(char_u *)4000L, (char_u *)0L}},
    {"verbose",	    "vbs",  P_NUM|P_VI_DEF,
			    (char_u *)&p_verbose, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"viewdir",     "vdir", P_STRING|P_EXPAND|P_VI_DEF|P_SECURE,
#ifdef FEAT_SESSION
			    (char_u *)&p_vdir, PV_NONE,
			    {(char_u *)DFLT_VDIR, (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"viewoptions", "vop",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_SESSION
			    (char_u *)&p_vop, PV_NONE,
			    {(char_u *)"folds,options,cursor", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"viminfo",	    "vi",   P_STRING|P_COMMA|P_NODUP|P_SECURE,
#ifdef FEAT_VIMINFO
			    (char_u *)&p_viminfo, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
#if defined(MSDOS) || defined(MSWIN) || defined(OS2)
			    {(char_u *)"", (char_u *)"'20,\"50,h,rA:,rB:"}
#else
# ifdef AMIGA
			    {(char_u *)"",
				     (char_u *)"'20,\"50,h,rdf0:,rdf1:,rdf2:"}
# else
			    {(char_u *)"", (char_u *)"'20,\"50,h"}
# endif
#endif
    },
    {"virtualedit", "ve",   P_STRING|P_COMMA|P_NODUP|P_VI_DEF|P_VIM,
#ifdef FEAT_VIRTUALEDIT
			    (char_u *)&p_ve, PV_NONE,
			    {(char_u *)"", (char_u *)""}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}
#endif
			    },
    {"visualbell",  "vb",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_vb, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"w300",	    NULL,   P_NUM|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"w1200",	    NULL,   P_NUM|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"w9600",	    NULL,   P_NUM|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"warn",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_warn, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"weirdinvert", "wiv",  P_BOOL|P_VI_DEF|P_RCLR,
			    (char_u *)&p_wiv, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"whichwrap",   "ww",   P_STRING|P_VIM|P_COMMA|P_FLAGLIST,
			    (char_u *)&p_ww, PV_NONE,
			    {(char_u *)"", (char_u *)"b,s"}},
    {"wildchar",    "wc",   P_NUM|P_VIM,
			    (char_u *)&p_wc, PV_NONE,
			    {(char_u *)(long)Ctrl_E, (char_u *)(long)TAB}},
    {"wildcharm",   "wcm",   P_NUM|P_VI_DEF,
			    (char_u *)&p_wcm, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"wildignore",  "wig",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
#ifdef FEAT_WILDIGN
			    (char_u *)&p_wig, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)"", (char_u *)0L}},
    {"wildmenu",    "wmnu", P_BOOL|P_VI_DEF,
#ifdef FEAT_WILDMENU
			    (char_u *)&p_wmnu, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"wildmode",    "wim",  P_STRING|P_VI_DEF|P_COMMA|P_NODUP,
			    (char_u *)&p_wim, PV_NONE,
			    {(char_u *)"full", (char_u *)0L}},
    {"winaltkeys",  "wak",  P_STRING|P_VI_DEF,
#ifdef FEAT_WAK
			    (char_u *)&p_wak, PV_NONE,
			    {(char_u *)"menu", (char_u *)0L}
#else
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)NULL, (char_u *)0L}
#endif
			    },
    {"window",	    "wi",   P_NUM|P_VI_DEF,
			    (char_u *)NULL, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},
    {"winheight",   "wh",   P_NUM|P_VI_DEF,
#ifdef FEAT_WINDOWS
			    (char_u *)&p_wh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)1L, (char_u *)0L}},
    {"winfixheight", "wfh", P_BOOL|P_VI_DEF|P_RSTAT,
#if defined(FEAT_WINDOWS)
			    (char_u *)VAR_WIN, PV_WFH,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)FALSE, (char_u *)0L}},
    {"winminheight", "wmh", P_NUM|P_VI_DEF,
#ifdef FEAT_WINDOWS
			    (char_u *)&p_wmh, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)1L, (char_u *)0L}},
    {"winminwidth", "wmw", P_NUM|P_VI_DEF,
#ifdef FEAT_VERTSPLIT
			    (char_u *)&p_wmw, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)1L, (char_u *)0L}},
    {"winwidth",   "wiw",   P_NUM|P_VI_DEF,
#ifdef FEAT_VERTSPLIT
			    (char_u *)&p_wiw, PV_NONE,
#else
			    (char_u *)NULL, PV_NONE,
#endif
			    {(char_u *)20L, (char_u *)0L}},
    {"wrap",	    NULL,   P_BOOL|P_VI_DEF|P_RWIN,
			    (char_u *)VAR_WIN, PV_WRAP,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"wrapmargin",  "wm",   P_NUM|P_VI_DEF,
			    (char_u *)&p_wm, PV_WM,
			    {(char_u *)0L, (char_u *)0L}},
    {"wrapscan",    "ws",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_ws, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"write",	    NULL,   P_BOOL|P_VI_DEF,
			    (char_u *)&p_write, PV_NONE,
			    {(char_u *)TRUE, (char_u *)0L}},
    {"writeany",    "wa",   P_BOOL|P_VI_DEF,
			    (char_u *)&p_wa, PV_NONE,
			    {(char_u *)FALSE, (char_u *)0L}},
    {"writebackup", "wb",   P_BOOL|P_VI_DEF|P_VIM,
			    (char_u *)&p_wb, PV_NONE,
			    {
#ifdef FEAT_WRITEBACKUP
			    (char_u *)TRUE,
#else
			    (char_u *)FALSE,
#endif
				(char_u *)0L}},
    {"writedelay",  "wd",   P_NUM|P_VI_DEF,
			    (char_u *)&p_wd, PV_NONE,
			    {(char_u *)0L, (char_u *)0L}},

/* terminal output codes */
#define p_term(sss, vvv)   {sss, NULL, P_STRING|P_VI_DEF|P_RALL, \
			    (char_u *)&vvv, PV_NONE, \
			    {(char_u *)"", (char_u *)0L}},

    p_term("t_AB", T_CAB)
    p_term("t_AF", T_CAF)
    p_term("t_AL", T_CAL)
    p_term("t_al", T_AL)
    p_term("t_bc", T_BC)
    p_term("t_cd", T_CD)
    p_term("t_ce", T_CE)
    p_term("t_cl", T_CL)
    p_term("t_cm", T_CM)
    p_term("t_Co", T_CCO)
    p_term("t_CS", T_CCS)
    p_term("t_cs", T_CS)
#ifdef FEAT_VERTSPLIT
    p_term("t_CV", T_CSV)
#endif
    p_term("t_ut", T_UT)
    p_term("t_da", T_DA)
    p_term("t_db", T_DB)
    p_term("t_DL", T_CDL)
    p_term("t_dl", T_DL)
    p_term("t_fs", T_FS)
    p_term("t_IE", T_CIE)
    p_term("t_IS", T_CIS)
    p_term("t_ke", T_KE)
    p_term("t_ks", T_KS)
    p_term("t_le", T_LE)
    p_term("t_mb", T_MB)
    p_term("t_md", T_MD)
    p_term("t_me", T_ME)
    p_term("t_mr", T_MR)
    p_term("t_ms", T_MS)
    p_term("t_nd", T_ND)
    p_term("t_op", T_OP)
    p_term("t_RI", T_CRI)
    p_term("t_RV", T_CRV)
    p_term("t_Sb", T_CSB)
    p_term("t_Sf", T_CSF)
    p_term("t_se", T_SE)
    p_term("t_so", T_SO)
    p_term("t_sr", T_SR)
    p_term("t_ts", T_TS)
    p_term("t_te", T_TE)
    p_term("t_ti", T_TI)
    p_term("t_ue", T_UE)
    p_term("t_us", T_US)
    p_term("t_vb", T_VB)
    p_term("t_ve", T_VE)
    p_term("t_vi", T_VI)
    p_term("t_vs", T_VS)
    p_term("t_WP", T_CWP)
    p_term("t_WS", T_CWS)
    p_term("t_xs", T_XS)
    p_term("t_ZH", T_CZH)
    p_term("t_ZR", T_CZR)

/* terminal key codes are not in here */

    {NULL, NULL, 0, NULL, PV_NONE, {NULL, NULL}}	/* end marker */
};

#define PARAM_COUNT (sizeof(options) / sizeof(struct vimoption))

static char *(p_bg_values[]) = {"light", "dark", NULL};
static char *(p_bkc_values[]) = {"yes", "auto", "no", NULL};
static char *(p_nf_values[]) = {"octal", "hex", "alpha", NULL};
static char *(p_ff_values[]) = {FF_UNIX, FF_DOS, FF_MAC, NULL};
#ifdef FEAT_WAK
static char *(p_wak_values[]) = {"yes", "menu", "no", NULL};
#endif
static char *(p_mousem_values[]) = {"extend", "popup", "popup_setpos", "mac", NULL};
#ifdef FEAT_VISUAL
static char *(p_sel_values[]) = {"inclusive", "exclusive", "old", NULL};
static char *(p_slm_values[]) = {"mouse", "key", "cmd", NULL};
#endif
#ifdef FEAT_VISUAL
static char *(p_km_values[]) = {"startsel", "stopsel", NULL};
#endif
#ifdef FEAT_BROWSE
static char *(p_bsdir_values[]) = {"current", "last", "buffer", NULL};
#endif
#ifdef FEAT_SCROLLBIND
static char *(p_scbopt_values[]) = {"ver", "hor", "jump", NULL};
#endif
static char *(p_swb_values[]) = {"useopen", "split", NULL};
static char *(p_debug_values[]) = {"msg", NULL};
#ifdef FEAT_VERTSPLIT
static char *(p_ead_values[]) = {"both", "ver", "hor", NULL};
#endif
#if defined(FEAT_QUICKFIX)
static char *(p_buftype_values[]) = {"nofile", "nowrite", "quickfix", "help", NULL};
static char *(p_bufhidden_values[]) = {"hide", "unload", "delete", NULL};
#endif
static char *(p_bs_values[]) = {"indent", "eol", "start", NULL};
#ifdef FEAT_FOLDING
static char *(p_fdm_values[]) = {"manual", "expr", "marker", "indent", "syntax",
#ifdef FEAT_DIFF
				"diff",
#endif
				NULL};
static char *(p_fcl_values[]) = {"all", NULL};
#endif

static void set_option_default __ARGS((int, int opt_flags, int compatible));
static void set_options_default __ARGS((int opt_flags));
static char_u *illegal_char __ARGS((char_u *, int));
static int string_to_key __ARGS((char_u *arg));
#ifdef FEAT_CMDWIN
static char_u *check_cedit __ARGS((void));
#endif
#ifdef FEAT_TITLE
static void did_set_title __ARGS((int icon));
#endif
static char_u *option_expand __ARGS((int opt_idx, char_u *val));
static void didset_options __ARGS((void));
static void check_string_option __ARGS((char_u **pp));
static void set_string_option_global __ARGS((int opt_idx, char_u **varp));
static void set_string_option __ARGS((int opt_idx, char_u *value, int opt_flags));
static char_u *did_set_string_option __ARGS((int opt_idx, char_u **varp, int new_value_alloced, char_u *oldval, char_u *errbuf, int opt_flags));
static char_u *set_chars_option __ARGS((char_u **varp));
#ifdef FEAT_CLIPBOARD
static char_u *check_clipboard_option __ARGS((void));
#endif
static char_u *set_bool_option __ARGS((int opt_idx, char_u *varp, int value, int opt_flags));
static char_u *set_num_option __ARGS((int opt_idx, char_u *varp, long value, char_u *errbuf, int opt_flags));
static void check_redraw __ARGS((long_u flags));
static int findoption __ARGS((char_u *));
static int find_key_option __ARGS((char_u *));
static void showoptions __ARGS((int all, int opt_flags));
static int optval_default __ARGS((struct vimoption *, char_u *varp));
static void showoneopt __ARGS((struct vimoption *, int opt_flags));
static int put_setstring __ARGS((FILE *fd, char *cmd, char *name, char_u **valuep, int expand));
static int put_setnum __ARGS((FILE *fd, char *cmd, char *name, long *valuep));
static int put_setbool __ARGS((FILE *fd, char *cmd, char *name, int value));
static int  istermoption __ARGS((struct vimoption *));
static char_u *get_varp_scope __ARGS((struct vimoption *p, int opt_flags));
static char_u *get_varp __ARGS((struct vimoption *));
static void option_value2string __ARGS((struct vimoption *, int opt_flags));
static int wc_use_keyname __ARGS((char_u *varp, long *wcp));
#ifdef FEAT_LANGMAP
static void langmap_init __ARGS((void));
static void langmap_set __ARGS((void));
#endif
static void paste_option_changed __ARGS((void));
static void compatible_set __ARGS((void));
#ifdef FEAT_LINEBREAK
static void fill_breakat_flags __ARGS((void));
#endif
static int opt_strings_flags __ARGS((char_u *val, char **values, unsigned *flagp, int list));
static int check_opt_strings __ARGS((char_u *val, char **values, int));
static int check_opt_wim __ARGS((void));

/*
 * Initialize the options, first part.
 *
 * Called only once from main(), just after creating the first buffer.
 */
    void
set_init_1()
{
    char_u	*p;
    int		opt_idx;
    long	n;

#ifdef FEAT_LANGMAP
    langmap_init();
#endif

    /* Be Vi compatible by default */
    p_cp = TRUE;

    /*
     * Find default value for 'shell' option.
     */
    if ((p = mch_getenv((char_u *)"SHELL")) != NULL
#if defined(MSDOS) || defined(MSWIN) || defined(OS2)
# ifdef __EMX__
	    || (p = mch_getenv((char_u *)"EMXSHELL")) != NULL
# endif
	    || (p = mch_getenv((char_u *)"COMSPEC")) != NULL
# ifdef WIN3264
	    || (p = default_shell()) != NULL
# endif
#endif
       )
	set_string_default("sh", p);

#ifdef FEAT_WILDIGN
    /*
     * Set the default for 'backupskip' to include environment variables for
     * temp files.
     */
    {
# ifdef UNIX
	static char	*(names[4]) = {"", "TMPDIR", "TEMP", "TMP"};
# else
	static char	*(names[3]) = {"TMPDIR", "TEMP", "TMP"};
# endif
	int	len;
	garray_T ga;

	ga_init2(&ga, 1, 100);
	for (n = 0; n < sizeof(names) / sizeof(char *); ++n)
	{
# ifdef UNIX
	    if (*names[n] == NUL)
		p = (char_u *)"/tmp";
	    else
# endif
		p = mch_getenv((char_u *)names[n]);
	    if (p != NULL && *p != NUL)
	    {
		/* First time count the NUL, otherwise count the ','. */
		len = STRLEN(p) + 3;
		if (ga_grow(&ga, len) == OK)
		{
		    if (ga.ga_len > 0)
			STRCAT(ga.ga_data, ",");
		    STRCAT(ga.ga_data, p);
		    add_pathsep(ga.ga_data);
		    STRCAT(ga.ga_data, "*");
		    ga.ga_room -= len;
		    ga.ga_len += len;
		}
	    }
	}
	if (ga.ga_data != NULL)
	{
	    set_string_default("bsk", ga.ga_data);
	    vim_free(ga.ga_data);
	}
    }
#endif

    /*
     * 'maxmemtot' and 'maxmem' may have to be adjusted for available memory
     */
    opt_idx = findoption((char_u *)"maxmemtot");
#if !defined(HAVE_AVAIL_MEM) && !defined(HAVE_TOTAL_MEM)
    if (options[opt_idx].def_val[VI_DEFAULT] == (char_u *)0L)
#endif
    {
#ifdef HAVE_AVAIL_MEM
	/* Use amount of memory available at this moment. */
	n = (mch_avail_mem(FALSE) >> 11);
#else
# ifdef HAVE_TOTAL_MEM
	/* Use amount of memory available to Vim. */
	n = (mch_total_mem(FALSE) >> 11);
# else
	n = (0x7fffffff >> 11);
# endif
#endif
	options[opt_idx].def_val[VI_DEFAULT] = (char_u *)n;
	opt_idx = findoption((char_u *)"maxmem");
#if !defined(HAVE_AVAIL_MEM) && !defined(HAVE_TOTAL_MEM)
	if ((long)options[opt_idx].def_val[VI_DEFAULT] > n
			  || (long)options[opt_idx].def_val[VI_DEFAULT] == 0L)
#endif
	    options[opt_idx].def_val[VI_DEFAULT] = (char_u *)n;
    }

#ifdef FEAT_GUI_W32
    /* force 'shortname' for Win32s */
    if (gui_is_win32s())
	options[findoption((char_u *)"shortname")].def_val[VI_DEFAULT] =
							       (char_u *)TRUE;
#endif

#ifdef FEAT_SEARCHPATH
    {
	char_u	*cdpath;
	char_u	*buf;
	int	i;
	int	j;

	/* Initialize the 'cdpath' option's default value. */
	cdpath = mch_getenv((char_u *)"CDPATH");
	if (cdpath != NULL)
	{
	    buf = alloc((unsigned)((STRLEN(cdpath) << 1) + 2));
	    if (buf != NULL)
	    {
		buf[0] = ',';	    /* start with ",", current dir first */
		j = 1;
		for (i = 0; cdpath[i] != NUL; ++i)
		{
		    if (vim_ispathlistsep(cdpath[i]))
			buf[j++] = ',';
		    else
		    {
			if (cdpath[i] == ' ' || cdpath[i] == ',')
			    buf[j++] = '\\';
			buf[j++] = cdpath[i];
		    }
		}
		buf[j] = NUL;
		opt_idx = findoption((char_u *)"cdpath");
		options[opt_idx].def_val[VI_DEFAULT] = buf;
		options[opt_idx].flags |= P_DEF_ALLOCED;
	    }
	}
    }
#endif

#ifdef FEAT_POSTSCRIPT
    /* 'printexpr' must be allocated to be able to evaluate it. */
    set_string_default("pexpr",
# ifdef MSWIN
	    (char_u *)"system('copy' . ' ' . v:fname_in . ' \"' . &printdevice . '\"') . delete(v:fname_in)"
# else
#  ifdef VMS
	    (char_u *)"system('print/delete' . (&printdevice == '' ? '' : ' /queue=' . &printdevice) . ' ' . v:fname_in)"

#  else
	    (char_u *)"system('lpr' . (&printdevice == '' ? '' : ' -P' . &printdevice) . ' ' . v:fname_in) . delete(v:fname_in) + v:shell_error"
#  endif
# endif
	    );
#endif

    /*
     * Set all the options (except the terminal options) to their default
     * value.  Also set the global value for local options.
     */
    set_options_default(0);

#ifdef FEAT_GUI
    if (found_reverse_arg)
	set_option_value((char_u *)"bg", 0L, (char_u *)"dark", 0);
#endif

    curbuf->b_p_initialized = TRUE;
    curbuf->b_p_ar = -1;	/* no local 'autoread' value */
    check_buf_options(curbuf);
    check_win_options(curwin);
    check_options();

    /* Must be before option_expand(), because that one needs vim_isIDc() */
    didset_options();

#ifdef FEAT_LINEBREAK
    /*
     * initialize the table for 'breakat'.
     */
    fill_breakat_flags();
#endif

    /*
     * Expand environment variables and things like "~" for the defaults.
     * If option_expand() returns non-NULL the variable is expanded.  This can
     * only happen for non-indirect options.
     * Also set the default to the expanded value, so ":set" does not list
     * them.
     * Don't set the P_ALLOCED flag, because we don't want to free the
     * default.
     */
    for (opt_idx = 0; !istermoption(&options[opt_idx]); opt_idx++)
    {
	p = option_expand(opt_idx, NULL);
	if (p != NULL && (p = vim_strsave(p)) != NULL)
	{
	    *(char_u **)options[opt_idx].var = p;
	    /* VIMEXP
	     * Defaults for all expanded options are currently the same for Vi
	     * and Vim.  When this changes, add some code here!  Also need to
	     * split P_DEF_ALLOCED in two.
	     */
	    if (options[opt_idx].flags & P_DEF_ALLOCED)
		vim_free(options[opt_idx].def_val[VI_DEFAULT]);
	    options[opt_idx].def_val[VI_DEFAULT] = p;
	    options[opt_idx].flags |= P_DEF_ALLOCED;
	}
    }

    /* Initialize the highlight_attr[] table. */
    highlight_changed();

    save_file_ff(curbuf);	/* Buffer is unchanged */

    /* Parse default for 'wildmode'  */
    check_opt_wim();

#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
    /* Parse default for 'fillchars'. */
    (void)set_chars_option(&p_fcs);
#endif

#ifdef FEAT_CLIPBOARD
    /* Parse default for 'clipboard' */
    (void)check_clipboard_option();
#endif

#ifdef FEAT_MBYTE
# if defined(WIN3264) && defined(FEAT_GETTEXT)
    /*
     * If $LANG isn't set, try to get a good value for it.  This makes the
     * right language be used automatically.  Don't do this for English.
     */
    if (mch_getenv("LANG") == NULL)
    {
	char	buf[20];

	/* Could use LOCALE_SISO639LANGNAME, but it's not in Win95.
	 * LOCALE_SABBREVLANGNAME gives us three letters, like "enu", we use
	 * only the first two. */
	n = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME,
							     (LPTSTR)buf, 20);
	if (n >= 2 && STRNICMP(buf, "en", 2) != 0)
	{
	    /* There are a few exceptions (probably more) */
	    if (STRNICMP(buf, "cht", 3) == 0 || STRNICMP(buf, "zht", 3) == 0)
		STRCPY(buf, "zh_TW");
	    else if (STRNICMP(buf, "chs", 3) == 0
					      || STRNICMP(buf, "zhc", 3) == 0)
		STRCPY(buf, "zh_CN");
	    else if (STRNICMP(buf, "jp", 2) == 0)
		STRCPY(buf, "ja");
	    else
		buf[2] = NUL;		/* truncate to two-letter code */
	    vim_setenv("LANG", buf);
	}
    }
# endif

    /* enc_default() will try setting p_enc to a value depending on the
     * current locale */
    if (enc_default() == OK)
    {
	opt_idx = findoption((char_u *)"encoding");
	options[opt_idx].def_val[VI_DEFAULT] = p_enc;
	options[opt_idx].flags |= P_DEF_ALLOCED;
    }
#endif
}

/*
 * Set an option to its default value.
 * This does not take care of side effects!
 */
    static void
set_option_default(opt_idx, opt_flags, compatible)
    int		opt_idx;
    int		opt_flags;	/* OPT_FREE, OPT_LOCAL and/or OPT_GLOBAL */
    int		compatible;	/* use Vi default value */
{
    char_u	*varp;		/* pointer to variable for current option */
    int		dvi;		/* index in def_val[] */
    long_u	flags;
    int		both = (opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0;

    varp = get_varp_scope(&(options[opt_idx]), both ? OPT_LOCAL : opt_flags);
    flags = options[opt_idx].flags;
    if (varp != NULL)	    /* nothing to do for hidden option */
    {
	dvi = ((flags & P_VI_DEF) || compatible) ? VI_DEFAULT : VIM_DEFAULT;
	if (flags & P_STRING)
	{
	    /* Use set_string_option_direct() for local options to handle
	     * freeing and allocating the value. */
	    if (options[opt_idx].indir != PV_NONE)
		set_string_option_direct(NULL, opt_idx,
				    options[opt_idx].def_val[dvi], opt_flags);
	    else
	    {
		if ((opt_flags & OPT_FREE) && (flags & P_ALLOCED))
		    free_string_option(*(char_u **)(varp));
		*(char_u **)varp = options[opt_idx].def_val[dvi];
		options[opt_idx].flags &= ~P_ALLOCED;
	    }
	}
	else if (flags & P_NUM)
	{
	    if (varp == (char_u *)PV_SCROLL)
		win_comp_scroll(curwin);
	    else
	    {
		*(long *)varp = (long)options[opt_idx].def_val[dvi];
		/* May also set global value for local option. */
		if (both)
		    *(long *)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL) =
								*(long *)varp;
	    }
	}
	else	/* P_BOOL */
	{
	    /* the cast to long is required for Manx C */
	    *(int *)varp = (int)(long)options[opt_idx].def_val[dvi];
	    /* May also set global value for local option. */
	    if (both)
		*(int *)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL) =
								*(int *)varp;
	}
    }
}

/*
 * Set all options (except terminal options) to their default value.
 */
    static void
set_options_default(opt_flags)
    int		opt_flags;	/* OPT_FREE, OPT_LOCAL and/or OPT_GLOBAL */
{
    int		i;
#ifdef FEAT_WINDOWS
    win_T	*wp;
#endif

    for (i = 0; !istermoption(&options[i]); i++)
	if (!(options[i].flags & P_NODEFAULT))
	    set_option_default(i, opt_flags, p_cp);

#ifdef FEAT_WINDOWS
    /* The 'scroll' option must be computed for all windows. */
    for (wp = firstwin; wp != NULL; wp = wp->w_next)
	win_comp_scroll(wp);
#else
	win_comp_scroll(curwin);
#endif
}

/*
 * Set the Vi-default value of a string option.
 * Used for 'sh', 'backupskip' and 'term'.
 */
    void
set_string_default(name, val)
    char	*name;
    char_u	*val;
{
    char_u	*p;
    int		opt_idx;

    p = vim_strsave(val);
    if (p != NULL)		/* we don't want a NULL */
    {
	opt_idx = findoption((char_u *)name);
	if (options[opt_idx].flags & P_DEF_ALLOCED)
	    vim_free(options[opt_idx].def_val[VI_DEFAULT]);
	options[opt_idx].def_val[VI_DEFAULT] = p;
	options[opt_idx].flags |= P_DEF_ALLOCED;
    }
}

/*
 * Set the Vi-default value of a number option.
 * Used for 'lines' and 'columns'.
 */
    void
set_number_default(name, val)
    char	*name;
    long	val;
{
    options[findoption((char_u *)name)].def_val[VI_DEFAULT] = (char_u *)val;
}

/*
 * Initialize the options, part two: After getting Rows and Columns and
 * setting 'term'.
 */
    void
set_init_2()
{
    /*
     * 'scroll' defaults to half the window height. Note that this default is
     * wrong when the window height changes.
     */
    options[findoption((char_u *)"scroll")].def_val[VI_DEFAULT]
					      = (char_u *)((long_u)Rows >> 1);
    comp_col();

#if !((defined(MSDOS) || defined(OS2) || defined(WIN3264)) && !defined(FEAT_GUI))
    {
	int	idx4;

	/*
	 * If 'background' wasn't set by the user, try guessing the value,
	 * depending on the terminal name.  Only need to check for terminals
	 * with a dark background, that can handle color.  Only "linux"
	 * console at the moment.
	 */
	idx4 = findoption((char_u *)"bg");
	if (!(options[idx4].flags & P_WAS_SET) && STRCMP(T_NAME, "linux") == 0)
	{
	    set_string_option_direct(NULL, idx4, (char_u *)"dark", OPT_FREE);
	    /* don't mark it as set, when starting the GUI it may be changed
	     * again */
	    options[idx4].flags &= ~P_WAS_SET;
	}
    }
#endif
}

/*
 * Initialize the options, part three: After reading the .vimrc
 */
    void
set_init_3()
{
#if defined(UNIX) || defined(OS2) || defined(WIN3264)
/*
 * Set 'shellpipe' and 'shellredir', depending on the 'shell' option.
 * This is done after other initializations, where 'shell' might have been
 * set, but only if they have not been set before.
 */
    char_u  *p;
    int	    idx_srr;
    int	    do_srr;
#ifdef FEAT_QUICKFIX
    int	    idx_sp;
    int	    do_sp;
#endif

    idx_srr = findoption((char_u *)"srr");
    do_srr = !(options[idx_srr].flags & P_WAS_SET);
#ifdef FEAT_QUICKFIX
    idx_sp = findoption((char_u *)"sp");
    do_sp = !(options[idx_sp].flags & P_WAS_SET);
#endif

    /*
     * Isolate the name of the shell:
     * - Skip beyond any path.  E.g., "/usr/bin/csh -f" -> "csh -f".
     * - Remove any argument.  E.g., "csh -f" -> "csh".
     */
    p = gettail(p_sh);
    p = vim_strnsave(p, (int)(skiptowhite(p) - p));
    if (p != NULL)
    {
	/*
	 * Default for p_sp is "| tee", for p_srr is ">".
	 * For known shells it is changed here to include stderr.
	 */
	if (	   fnamecmp(p, "csh") == 0
		|| fnamecmp(p, "tcsh") == 0
# if defined(OS2) || defined(WIN3264)	/* also check with .exe extension */
		|| fnamecmp(p, "csh.exe") == 0
		|| fnamecmp(p, "tcsh.exe") == 0
# endif
	   )
	{
#if defined(FEAT_QUICKFIX)
	    if (do_sp)
	    {
# ifdef WIN3264
		p_sp = (char_u *)">&";
# else
		p_sp = (char_u *)"|& tee";
# endif
		options[idx_sp].def_val[VI_DEFAULT] = p_sp;
	    }
#endif
	    if (do_srr)
	    {
		p_srr = (char_u *)">&";
		options[idx_srr].def_val[VI_DEFAULT] = p_srr;
	    }
	}
	else
# ifndef OS2	/* Always use bourne shell style redirection if we reach this */
	    if (       fnamecmp(p, "sh") == 0
		    || fnamecmp(p, "ksh") == 0
		    || fnamecmp(p, "zsh") == 0
		    || fnamecmp(p, "bash") == 0
#  ifdef WIN3264
		    || fnamecmp(p, "cmd") == 0
		    || fnamecmp(p, "sh.exe") == 0
		    || fnamecmp(p, "ksh.exe") == 0
		    || fnamecmp(p, "zsh.exe") == 0
		    || fnamecmp(p, "bash.exe") == 0
		    || fnamecmp(p, "cmd.exe") == 0
#  endif
		    )
# endif
	    {
#if defined(FEAT_QUICKFIX)
		if (do_sp)
		{
# ifdef WIN3264
		    p_sp = (char_u *)">%s 2>&1";
# else
		    p_sp = (char_u *)"2>&1| tee";
# endif
		    options[idx_sp].def_val[VI_DEFAULT] = p_sp;
		}
#endif
		if (do_srr)
		{
		    p_srr = (char_u *)">%s 2>&1";
		    options[idx_srr].def_val[VI_DEFAULT] = p_srr;
		}
	    }
	vim_free(p);
    }
#endif

#if defined(MSDOS) || defined(WIN3264) || defined(OS2)
    /*
     * Set 'shellcmdflag and 'shellquote' depending on the 'shell' option.
     * This is done after other initializations, where 'shell' might have been
     * set, but only if they have not been set before.  Default for p_shcf is
     * "/c", for p_shq is "".  For "sh" like  shells it is changed here to
     * "-c" and "\"", but not for DJGPP, because it starts the shell without
     * command.com.  And for Win32 we need to set p_sxq instead.
     */
    if (strstr((char *)p_sh, "sh") != NULL)
    {
	int	idx3;

	idx3 = findoption((char_u *)"shcf");
	if (!(options[idx3].flags & P_WAS_SET))
	{
	    p_shcf = (char_u *)"-c";
	    options[idx3].def_val[VI_DEFAULT] = p_shcf;
	}

# ifndef DJGPP
#  ifdef WIN3264
	/* Somehow Win32 requires the quotes around the redirection too */
	idx3 = findoption((char_u *)"sxq");
	if (!(options[idx3].flags & P_WAS_SET))
	{
	    p_sxq = (char_u *)"\"";
	    options[idx3].def_val[VI_DEFAULT] = p_sxq;
	}
#  else
	idx3 = findoption((char_u *)"shq");
	if (!(options[idx3].flags & P_WAS_SET))
	{
	    p_shq = (char_u *)"\"";
	    options[idx3].def_val[VI_DEFAULT] = p_shq;
	}
#  endif
# endif
    }
#endif

#ifdef FEAT_TITLE
    set_title_defaults();
#endif
}

#ifdef FEAT_GUI
static char_u *gui_bg_default __ARGS((void));

    static char_u *
gui_bg_default()
{
    if (gui_get_lightness(gui.back_pixel) < 127)
	return (char_u *)"dark";
    return (char_u *)"light";
}

/*
 * Option initializations that can only be done after opening the GUI window.
 */
    void
init_gui_options()
{
    /* Set the 'background' option according to the lightness of the
     * background color, unless the user has set it already. */
    if (!option_was_set((char_u *)"bg") && STRCMP(p_bg, gui_bg_default()) != 0)
    {
	set_option_value((char_u *)"bg", 0L, gui_bg_default(), 0);
	highlight_changed();
    }
}
#endif

#ifdef FEAT_TITLE
/*
 * 'title' and 'icon' only default to true if they have not been set or reset
 * in .vimrc and we can read the old value.
 * When 'title' and 'icon' have been reset in .vimrc, we won't even check if
 * they can be reset.  This reduces startup time when using X on a remote
 * machine.
 */
    void
set_title_defaults()
{
    int	    idx1;
    long    val;

    /*
     * If GUI is (going to be) used, we can always set the window title and
     * icon name.  Saves a bit of time, because the X11 display server does
     * not need to be contacted.
     */
    idx1 = findoption((char_u *)"title");
    if (!(options[idx1].flags & P_WAS_SET))
    {
#ifdef FEAT_GUI
	if (gui.starting || gui.in_use)
	    val = TRUE;
	else
#endif
	    val = mch_can_restore_title();
	options[idx1].def_val[VI_DEFAULT] = (char_u *)val;
	p_title = val;
    }
    idx1 = findoption((char_u *)"icon");
    if (!(options[idx1].flags & P_WAS_SET))
    {
#ifdef FEAT_GUI
	if (gui.starting || gui.in_use)
	    val = TRUE;
	else
#endif
	    val = mch_can_restore_icon();
	options[idx1].def_val[VI_DEFAULT] = (char_u *)val;
	p_icon = val;
    }
}
#endif

/*
 * Parse 'arg' for option settings.
 *
 * 'arg' may be IObuff, but only when no errors can be present and option
 * does not need to be expanded with option_expand().
 * "opt_flags":
 * 0 for ":set"
 * OPT_GLOBAL for ":setglobal"
 * OPT_LOCAL for ":setlocal" and a modeline
 * OPT_MODELINE for a modeline
 *
 * returns FAIL if an error is detected, OK otherwise
 */
    int
do_set(arg, opt_flags)
    char_u	*arg;		/* option string (may be written to!) */
    int		opt_flags;
{
    int		opt_idx;
    char_u	*errmsg;
    char_u	errbuf[80];
    char_u	*startarg;
    int		prefix;	/* 1: nothing, 0: "no", 2: "inv" in front of name */
    int		nextchar;	    /* next non-white char after option name */
    int		afterchar;	    /* character just after option name */
    int		len;
    int		i;
    long	value;
    int		key;
    long_u	flags;		    /* flags for current option */
    char_u	*varp = NULL;	    /* pointer to variable for current option */
    int		did_show = FALSE;   /* already showed one value */
    int		adding;		    /* "opt+=arg" */
    int		prepending;	    /* "opt^=arg" */
    int		removing;	    /* "opt-=arg" */
    int		cp_val = 0;
    char_u	key_name[2];

    if (*arg == NUL)
    {
	showoptions(0, opt_flags);
	return OK;
    }

    while (*arg != NUL)		/* loop to process all options */
    {
	errmsg = NULL;
	startarg = arg;		/* remember for error message */

	if (STRNCMP(arg, "all", 3) == 0 && !isalpha(arg[3]))
	{
	    /*
	     * ":set all"  show all options.
	     * ":set all&" set all options to their default value.
	     */
	    arg += 3;
	    if (*arg == '&')
	    {
		++arg;
		/* Only for :set command set global value of local options. */
		set_options_default(OPT_FREE | opt_flags);
	    }
	    else
		showoptions(1, opt_flags);
	}
	else if (STRNCMP(arg, "termcap", 7) == 0)
	{
	    showoptions(2, opt_flags);
	    show_termcodes();
	    arg += 7;
	}
	else
	{
	    prefix = 1;
	    if (STRNCMP(arg, "no", 2) == 0)
	    {
		prefix = 0;
		arg += 2;
	    }
	    else if (STRNCMP(arg, "inv", 3) == 0)
	    {
		prefix = 2;
		arg += 3;
	    }

	    /* find end of name */
	    key = 0;
	    if (*arg == '<')
	    {
		nextchar = 0;
		opt_idx = -1;
		/* look out for <t_>;> */
		if (arg[1] == 't' && arg[2] == '_' && arg[3] && arg[4])
		    len = 5;
		else
		{
		    len = 1;
		    while (arg[len] != NUL && arg[len] != '>')
			++len;
		}
		if (arg[len] != '>')
		{
		    errmsg = e_invarg;
		    goto skip;
		}
		arg[len] = NUL;			    /* put NUL after name */
		if (arg[1] == 't' && arg[2] == '_') /* could be term code */
		    opt_idx = findoption(arg + 1);
		arg[len++] = '>';		    /* restore '>' */
		if (opt_idx == -1)
		    key = find_key_option(arg + 1);
	    }
	    else
	    {
		len = 0;
		/*
		 * The two characters after "t_" may not be alphanumeric.
		 */
		if (arg[0] == 't' && arg[1] == '_' && arg[2] && arg[3])
		    len = 4;
		else
		    while (ASCII_ISALNUM(arg[len]) || arg[len] == '_')
			++len;
		nextchar = arg[len];
		arg[len] = NUL;			    /* put NUL after name */
		opt_idx = findoption(arg);
		arg[len] = nextchar;		    /* restore nextchar */
		if (opt_idx == -1)
		    key = find_key_option(arg);
	    }

	    /* remember character after option name */
	    afterchar = arg[len];

	    /* skip white space, allow ":set ai  ?" */
	    while (vim_iswhite(arg[len]))
		++len;

	    adding = FALSE;
	    prepending = FALSE;
	    removing = FALSE;
	    if (arg[len] != NUL && arg[len + 1] == '=')
	    {
		if (arg[len] == '+')
		{
		    adding = TRUE;		/* "+=" */
		    ++len;
		}
		else if (arg[len] == '^')
		{
		    prepending = TRUE;		/* "^=" */
		    ++len;
		}
		else if (arg[len] == '-')
		{
		    removing = TRUE;		/* "-=" */
		    ++len;
		}
	    }
	    nextchar = arg[len];

	    if (opt_idx == -1 && key == 0)	/* found a mismatch: skip */
	    {
		errmsg = (char_u *)N_("Unknown option");
		goto skip;
	    }

	    if (opt_idx >= 0)
	    {
		if (options[opt_idx].var == NULL)   /* hidden option: skip */
		{
		    /* Only give an error message when requesting the value of
		     * a hidden option, ignore setting it. */
		    if (vim_strchr((char_u *)"=:!&<", nextchar) == NULL
			    && (!(options[opt_idx].flags & P_BOOL)
				|| nextchar == '?'))
			errmsg = (char_u *)N_("Option not supported");
		    goto skip;
		}

		flags = options[opt_idx].flags;
		varp = get_varp_scope(&(options[opt_idx]), opt_flags);
	    }
	    else
	    {
		flags = P_STRING;
		if (key < 0)
		{
		    key_name[0] = KEY2TERMCAP0(key);
		    key_name[1] = KEY2TERMCAP1(key);
		}
		else
		{
		    key_name[0] = KS_KEY;
		    key_name[1] = (key & 0xff);
		}
	    }

	    /* Disallow changing some options from modelines */
	    if ((opt_flags & OPT_MODELINE) && (flags & P_SECURE))
	    {
		errmsg = (char_u *)_("Not allowed in a modeline");
		goto skip;
	    }

	    if (vim_strchr((char_u *)"?=:!&<", nextchar) != NULL)
	    {
		arg += len;
		cp_val = p_cp;
		if (nextchar == '&' && arg[1] == 'v' && arg[2] == 'i')
		{
		    if (arg[3] == 'm')	/* "opt&vim": set to Vim default */
		    {
			cp_val = FALSE;
			arg += 3;
		    }
		    else		/* "opt&vi": set to Vi default */
		    {
			cp_val = TRUE;
			arg += 2;
		    }
		}
		if (vim_strchr((char_u *)"?!&<", nextchar) != NULL
			&& arg[1] != NUL && !vim_iswhite(arg[1]))
		{
		    errmsg = e_trailing;
		    goto skip;
		}
	    }

	    /*
	     * allow '=' and ':' as MSDOS command.com allows only one
	     * '=' character per "set" command line. grrr. (jw)
	     */
	    if (nextchar == '?'
		    || (prefix == 1
			&& vim_strchr((char_u *)"=:&<", nextchar) == NULL
			&& !(flags & P_BOOL)))
	    {
		/*
		 * print value
		 */
		if (did_show)
		    msg_putchar('\n');	    /* cursor below last one */
		else
		{
		    gotocmdline(TRUE);	    /* cursor at status line */
		    did_show = TRUE;	    /* remember that we did a line */
		}
		if (opt_idx >= 0)
		{
		    showoneopt(&options[opt_idx], opt_flags);
#ifdef FEAT_EVAL
		    if (p_verbose > 0)
		    {
			if (options[opt_idx].scriptID != 0)
			{
			    MSG_PUTS(_("\n\tLast set from "));
			    MSG_PUTS(get_scriptname(options[opt_idx].scriptID));
			}
		    }
#endif
		}
		else
		{
		    char_u	    *p;

		    p = find_termcode(key_name);
		    if (p == NULL)
		    {
			errmsg = (char_u *)N_("Unknown option");
			goto skip;
		    }
		    else
			(void)show_one_termcode(key_name, p, TRUE);
		}
		if (nextchar != '?'
			&& nextchar != NUL && !vim_iswhite(afterchar))
		    errmsg = e_trailing;
	    }
	    else
	    {
		if (flags & P_BOOL)		    /* boolean */
		{
		    if (nextchar == '=' || nextchar == ':')
		    {
			errmsg = e_invarg;
			goto skip;
		    }

		    /*
		     * ":set opt!": invert
		     * ":set opt&": reset to default value
		     * ":set opt<": reset to global value
		     */
		    if (nextchar == '!')
			value = *(int *)(varp) ^ 1;
		    else if (nextchar == '&')
			value = (int)(long)options[opt_idx].def_val[
						((flags & P_VI_DEF) || cp_val)
						 ?  VI_DEFAULT : VIM_DEFAULT];
		    else if (nextchar == '<')
		    {
			/* For 'autoread' -1 means to use global value. */
			if ((int *)varp == &curbuf->b_p_ar
						    && opt_flags == OPT_LOCAL)
			    value = -1;
			else
			    value = *(int *)get_varp_scope(&(options[opt_idx]),
								  OPT_GLOBAL);
		    }
		    else
		    {
			/*
			 * ":set invopt": invert
			 * ":set opt" or ":set noopt": set or reset
			 */
			if (nextchar != NUL && !vim_iswhite(afterchar))
			{
			    errmsg = e_trailing;
			    goto skip;
			}
			if (prefix == 2)	/* inv */
			    value = *(int *)(varp) ^ 1;
			else
			    value = prefix;
		    }

		    errmsg = set_bool_option(opt_idx, varp, (int)value,
								   opt_flags);
		}
		else				    /* numeric or string */
		{
		    if (vim_strchr((char_u *)"=:&<", nextchar) == NULL
							       || prefix != 1)
		    {
			errmsg = e_invarg;
			goto skip;
		    }

		    if (flags & P_NUM)		    /* numeric */
		    {
			/*
			 * Different ways to set a number option:
			 * &	    set to default value
			 * <	    set to global value
			 * <xx>	    accept special key codes for 'wildchar'
			 * c	    accept any non-digit for 'wildchar'
			 * [-]0-9   set number
			 * other    error
			 */
			++arg;
			if (nextchar == '&')
			    value = (long)options[opt_idx].def_val[
						((flags & P_VI_DEF) || cp_val)
						 ?  VI_DEFAULT : VIM_DEFAULT];
			else if (nextchar == '^')
			    value = *(long *)get_varp_scope(&(options[opt_idx]),
								  OPT_GLOBAL);
			else if (((long *)varp == &p_wc
				    || (long *)varp == &p_wcm)
				&& (*arg == '<'
				    || *arg == '^'
				    || ((!arg[1] || vim_iswhite(arg[1]))
					&& !isdigit(*arg))))
			{
			    value = string_to_key(arg);
			    if (value == 0 && (long *)varp != &p_wcm)
			    {
				errmsg = e_invarg;
				goto skip;
			    }
			}
				/* allow negative numbers (for 'undolevels') */
			else if (*arg == '-' || isdigit(*arg))
			{
			    i = 0;
			    if (*arg == '-')
				i = 1;
#ifdef HAVE_STRTOL
			    value = strtol((char *)arg, NULL, 0);
			    if (arg[i] == '0' && TO_LOWER(arg[i + 1]) == 'x')
				i += 2;
#else
			    value = atol((char *)arg);
#endif
			    while (isdigit(arg[i]))
				++i;
			    if (arg[i] != NUL && !vim_iswhite(arg[i]))
			    {
				errmsg = e_invarg;
				goto skip;
			    }
			}
			else
			{
			    errmsg = (char_u *)N_("Number required after =");
			    goto skip;
			}

			if (adding)
			    value = *(long *)varp + value;
			if (prepending)
			    value = *(long *)varp * value;
			if (removing)
			    value = *(long *)varp - value;
			errmsg = set_num_option(opt_idx, varp, value,
							   errbuf, opt_flags);
		    }
		    else if (opt_idx >= 0)		    /* string */
		    {
			char_u	    *save_arg = NULL;
			char_u	    *s = NULL;
			char_u	    *oldval;	/* previous value if *varp */
			char_u	    *newval;
			char_u	    *origval;
			unsigned    newlen;
			int	    comma;
			int	    bs;
			int	    new_value_alloced;	/* new string option
							   was allocated */

			/* When using ":set opt=val" for a global option
			 * with a local value the local value will be
			 * reset, use the global value here. */
			if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
				&& (int)options[opt_idx].indir >= PV_BOTH)
			    varp = options[opt_idx].var;

			/* The old value is kept until we are sure that the
			 * new value is valid. */
			oldval = *(char_u **)varp;
			if (nextchar == '&')	/* set to default val */
			{
			    newval = options[opt_idx].def_val[
						((flags & P_VI_DEF) || cp_val)
						 ?  VI_DEFAULT : VIM_DEFAULT];
			    if ((char_u **)varp == &p_bg)
			    {
				/* guess the value of 'background' */
#ifdef FEAT_GUI
				if (gui.in_use)
				    newval = gui_bg_default();
				else
#endif
				    if (STRCMP(T_NAME, "linux") == 0)
					newval = (char_u *)"dark";
			    }

			    /* expand environment variables and ~ (since the
			     * default value was already expanded, only
			     * required when an environment variable was set
			     * later */
			    if (newval == NULL)
				newval = empty_option;
			    else
			    {
				s = option_expand(opt_idx, newval);
				if (s == NULL)
				    s = newval;
				newval = vim_strsave(s);
			    }
			    new_value_alloced = TRUE;
			}
			else if (nextchar == '<')	/* set to global val */
			{
			    newval = *(char_u **)get_varp_scope(
					     &(options[opt_idx]), OPT_GLOBAL);
			    new_value_alloced = FALSE;
			}
			else
			{
			    ++arg;	/* jump to after the '=' or ':' */

			    /*
			     * Convert 'whichwrap' number to string, for
			     * backwards compatibility with Vim 3.0.
			     * Misuse errbuf[] for the resulting string.
			     */
			    if (varp == (char_u *)&p_ww && isdigit(*arg))
			    {
				*errbuf = NUL;
				i = getdigits(&arg);
				if (i & 1)
				    STRCAT(errbuf, "b,");
				if (i & 2)
				    STRCAT(errbuf, "s,");
				if (i & 4)
				    STRCAT(errbuf, "h,l,");
				if (i & 8)
				    STRCAT(errbuf, "<,>,");
				if (i & 16)
				    STRCAT(errbuf, "[,],");
				if (*errbuf != NUL)	/* remove trailing , */
				    errbuf[STRLEN(errbuf) - 1] = NUL;
				save_arg = arg;
				arg = errbuf;
			    }
			    /*
			     * Remove '>' before 'dir' and 'bdir', for
			     * backwards compatibility with version 3.0
			     */
			    else if (  *arg == '>'
				    && (varp == (char_u *)&p_dir
					    || varp == (char_u *)&p_bdir))
			    {
				++arg;
			    }

			    /* When setting the local value of a global
			     * option, the old value may be the global value. */
			    if ((int)options[opt_idx].indir >= PV_BOTH
						   && (opt_flags & OPT_LOCAL))
				origval = *(char_u **)get_varp(
							   &options[opt_idx]);
			    else
				origval = oldval;

			    /*
			     * Copy the new string into allocated memory.
			     * Can't use set_string_option_direct(), because
			     * we need to remove the backslashes.
			     */
			    /* get a bit too much */
			    newlen = (unsigned)STRLEN(arg) + 1;
			    if (adding || prepending || removing)
				newlen += (unsigned)STRLEN(origval) + 1;
			    newval = alloc(newlen);
			    if (newval == NULL)  /* out of mem, don't change */
				break;
			    s = newval;

			    /*
			     * Copy the string, skip over escaped chars.
			     * For MS-DOS and WIN32 backslashes before normal
			     * file name characters are not removed, and keep
			     * backslash at start, for "\\machine\path", but
			     * do remove it for "\\\\machine\\path".
			     * The reverse is found in ExpandOldSetting().
			     */
			    while (*arg && !vim_iswhite(*arg))
			    {
				if (*arg == '\\' && arg[1] != NUL
#ifdef BACKSLASH_IN_FILENAME
					&& !((flags & P_EXPAND)
						&& vim_isfilec(arg[1])
						&& (arg[1] != '\\'
						    || (s == newval
							&& arg[2] != '\\')))
#endif
								    )
				    ++arg;	/* remove backslash */
#ifdef FEAT_MBYTE
				if (has_mbyte
					&& (i = (*mb_ptr2len_check)(arg)) > 1)
				{
				    /* copy multibyte char */
				    mch_memmove(s, arg, (size_t)i);
				    arg += i;
				    s += i;
				}
				else
#endif
				    *s++ = *arg++;
			    }
			    *s = NUL;

			    /*
			     * Expand environment variables and ~.
			     * Don't do it when adding without inserting a
			     * comma.
			     */
			    if (!(adding || prepending || removing)
							 || (flags & P_COMMA))
			    {
				s = option_expand(opt_idx, newval);
				if (s != NULL)
				{
				    vim_free(newval);
				    newlen = (unsigned)STRLEN(s) + 1;
				    if (adding || prepending || removing)
					newlen += (unsigned)STRLEN(origval) + 1;
				    newval = alloc(newlen);
				    if (newval == NULL)
					break;
				    STRCPY(newval, s);
				}
			    }

			    /* locate newval[] in origval[] when removing it
			     * and when adding to avoid duplicates */
			    i = 0;	/* init for GCC */
			    if (removing || (flags & P_NODUP))
			    {
				i = (int)STRLEN(newval);
				bs = 0;
				for (s = origval; *s; ++s)
				{
				    if ((!(flags & P_COMMA)
						|| s == origval
						|| (s[-1] == ',' && !(bs & 1)))
					    && STRNCMP(s, newval, i) == 0
					    && (!(flags & P_COMMA)
						|| s[i] == ','
						|| s[i] == NUL))
					break;
				    /* Count backspaces.  Only a comma with an
				     * even number of backspaces before it is
				     * recognized as a separator */
				    if (s > origval && s[-1] == '\\')
					++bs;
				    else
					bs = 0;
				}

				/* do not add if already there */
				if ((adding || prepending) && *s)
				{
				    prepending = FALSE;
				    adding = FALSE;
				    STRCPY(newval, origval);
				}
			    }

			    /* concatenate the two strings; add a ',' if
			     * needed */
			    if (adding || prepending)
			    {
				comma = ((flags & P_COMMA) && *origval);
				if (adding)
				{
				    i = (int)STRLEN(origval);
				    mch_memmove(newval + i + comma, newval,
							  STRLEN(newval) + 1);
				    mch_memmove(newval, origval, (size_t)i);
				}
				else
				{
				    i = (int)STRLEN(newval);
				    mch_memmove(newval + i + comma, origval,
							  STRLEN(origval) + 1);
				}
				if (comma)
				    newval[i] = ',';
			    }

			    /* Remove newval[] from origval[]. (Note: "i" has
			     * been set above and is used here). */
			    if (removing)
			    {
				STRCPY(newval, origval);
				if (*s)
				{
				    /* may need to remove a comma */
				    if (flags & P_COMMA)
				    {
					if (s == origval)
					{
					    /* include comma after string */
					    if (s[i] == ',')
						++i;
					}
					else
					{
					    /* include comma before string */
					    --s;
					    ++i;
					}
				    }
				    mch_memmove(newval + (s - origval), s + i,
							   STRLEN(s + i) + 1);
				}
			    }

			    if (flags & P_FLAGLIST)
			    {
				/* Remove flags that appear twice. */
				for (s = newval; *s; ++s)
				    if ((!(flags & P_COMMA) || *s != ',')
					    && vim_strchr(s + 1, *s) != NULL)
				    {
					STRCPY(s, s + 1);
					--s;
				    }
			    }

			    if (save_arg != NULL)   /* number for 'whichwrap' */
				arg = save_arg;
			    new_value_alloced = TRUE;
			}

			/* Set the new value. */
			*(char_u **)(varp) = newval;

			/* Handle side effects, and set the global value for
			 * ":set" on local options. */
			errmsg = did_set_string_option(opt_idx, (char_u **)varp,
				new_value_alloced, oldval, errbuf, opt_flags);

			/* If error detected, print the error message. */
			if (errmsg != NULL)
			    goto skip;
		    }
		    else	    /* key code option */
		    {
			char_u	    *p;

			if (nextchar == '&')
			{
			    if (add_termcap_entry(key_name, TRUE) == FAIL)
				errmsg = (char_u *)N_("Not found in termcap");
			}
			else
			{
			    ++arg; /* jump to after the '=' or ':' */
			    for (p = arg; *p && !vim_iswhite(*p); ++p)
				if (*p == '\\' && p[1] != NUL)
				    ++p;
			    nextchar = *p;
			    *p = NUL;
			    add_termcode(key_name, arg, FALSE);
			    *p = nextchar;
			}
			if (full_screen)
			    ttest(FALSE);
			redraw_all_later(CLEAR);
		    }
		}
		if (opt_idx >= 0)
		    options[opt_idx].flags |= P_WAS_SET;
	    }

skip:
	    /*
	     * Advance to next argument.
	     * - skip until a blank found, taking care of backslashes
	     * - skip blanks
	     * - skip one "=val" argument (for hidden options ":set gfn =xx")
	     */
	    for (i = 0; i < 2 ; ++i)
	    {
		while (*arg != NUL && !vim_iswhite(*arg))
		    if (*arg++ == '\\' && *arg != NUL)
			++arg;
		arg = skipwhite(arg);
		if (*arg != '=')
		    break;
	    }
	}
	arg = skipwhite(arg);

	if (errmsg != NULL)
	{
	    ++no_wait_return;	/* wait_return done below */
	    EMSG(_(errmsg));	/* show error highlighted */
	    MSG_PUTS(": ");
				/* show argument normal */
	    while (startarg < arg)
		startarg = msg_outtrans_one(startarg, 0);
	    msg_end();		/* check for scrolling */
	    --no_wait_return;

	    return FAIL;
	}
    }

    return OK;
}

    static char_u *
illegal_char(errbuf, c)
    char_u	*errbuf;
    int		c;
{
    if (errbuf == NULL)
	return (char_u *)"";
    sprintf((char *)errbuf, _("Illegal character <%s>"), (char *)transchar(c));
    return errbuf;
}

/*
 * Convert a key name or string into a key value.
 * Used for 'wildchar' and 'cedit' options.
 */
    static int
string_to_key(arg)
    char_u	*arg;
{
    if (*arg == '<')
	return find_key_option(arg + 1);
    if (*arg == '^')
	return Ctrl_chr(arg[1]);
    return *arg;
}

#ifdef FEAT_CMDWIN
/*
 * Check value of 'cedit' and set cedit_key.
 * Returns NULL if value is OK, error message otherwise.
 */
    static char_u *
check_cedit()
{
    int n;

    if (*p_cedit == NUL)
	cedit_key = -1;
    else
    {
	n = string_to_key(p_cedit);
	if (vim_isprintc(n))
	    return e_invarg;
	cedit_key = n;
    }
    return NULL;
}
#endif

#ifdef FEAT_TITLE
/*
 * When changing 'title', 'titlestring', 'icon' or 'iconstring', call
 * maketitle() to create and display it.
 * When switching the title or icon off, call mch_restore_title() to get
 * the old value back.
 */
    static void
did_set_title(icon)
    int	    icon;	    /* Did set icon instead of title */
{
    if (starting != NO_SCREEN
#ifdef FEAT_GUI
	    && !gui.starting
#endif
				)
    {
	maketitle();
	if (icon)
	{
	    if (!p_icon)
		mch_restore_title(2);
	}
	else
	{
	    if (!p_title)
		mch_restore_title(1);
	}
    }
}
#endif

/*
 * set_options_bin -  called when 'bin' changes value.
 */
    void
set_options_bin(oldval, newval, opt_flags)
    int		oldval;
    int		newval;
    int		opt_flags;	/* OPT_LOCAL and/or OPT_GLOBAL */
{
    /*
     * The option values that are changed when 'bin' changes are
     * copied when 'bin is set and restored when 'bin' is reset.
     */
    if (newval)
    {
	if (!oldval)		/* switched on */
	{
	    if (!(opt_flags & OPT_GLOBAL))
	    {
		curbuf->b_p_tw_nobin = curbuf->b_p_tw;
		curbuf->b_p_wm_nobin = curbuf->b_p_wm;
		curbuf->b_p_ml_nobin = curbuf->b_p_ml;
		curbuf->b_p_et_nobin = curbuf->b_p_et;
	    }
	    if (!(opt_flags & OPT_LOCAL))
	    {
		p_tw_nobin = p_tw;
		p_wm_nobin = p_wm;
		p_ml_nobin = p_ml;
		p_et_nobin = p_et;
	    }
	}

	if (!(opt_flags & OPT_GLOBAL))
	{
	    curbuf->b_p_tw = 0;	/* no automatic line wrap */
	    curbuf->b_p_wm = 0;	/* no automatic line wrap */
	    curbuf->b_p_ml = 0;	/* no modelines */
	    curbuf->b_p_et = 0;	/* no expandtab */
	}
	if (!(opt_flags & OPT_LOCAL))
	{
	    p_tw = 0;
	    p_wm = 0;
	    p_ml = FALSE;
	    p_et = FALSE;
	    p_bin = TRUE;	/* needed when called for the "-b" argument */
	}
    }
    else if (oldval)		/* switched off */
    {
	if (!(opt_flags & OPT_GLOBAL))
	{
	    curbuf->b_p_tw = curbuf->b_p_tw_nobin;
	    curbuf->b_p_wm = curbuf->b_p_wm_nobin;
	    curbuf->b_p_ml = curbuf->b_p_ml_nobin;
	    curbuf->b_p_et = curbuf->b_p_et_nobin;
	}
	if (!(opt_flags & OPT_LOCAL))
	{
	    p_tw = p_tw_nobin;
	    p_wm = p_wm_nobin;
	    p_ml = p_ml_nobin;
	    p_et = p_et_nobin;
	}
    }
}

#ifdef FEAT_VIMINFO
/*
 * Find the parameter represented by the given character (eg ', :, ", or /),
 * and return its associated value in the 'viminfo' string.
 * Only works for number parameters, not for 'r' or 'n'.
 * If the parameter is not specified in the string, return -1.
 */
    int
get_viminfo_parameter(type)
    int	    type;
{
    char_u  *p;

    p = find_viminfo_parameter(type);
    if (p != NULL && isdigit(*p))
	return atoi((char *)p);
    return -1;
}

/*
 * Find the parameter represented by the given character (eg ', :, ", or /) in
 * the 'viminfo' option and return a pointer to the string after it.
 * Return NULL if the parameter is not specified in the string.
 */
    char_u *
find_viminfo_parameter(type)
    int	    type;
{
    char_u  *p;

    for (p = p_viminfo; *p; ++p)
    {
	if (*p == type)
	    return p + 1;
	if (*p == 'n')		    /* 'n' is always the last one */
	    break;
	p = vim_strchr(p, ',');	    /* skip until next ',' */
	if (p == NULL)		    /* hit the end without finding parameter */
	    break;
    }
    return NULL;
}
#endif

/*
 * Expand environment variables for some string options.
 * These string options cannot be indirect!
 * If "val" is NULL expand the current value of the option.
 * Return pointer to NameBuff, or NULL when not expanded.
 */
    static char_u *
option_expand(opt_idx, val)
    int		opt_idx;
    char_u	*val;
{
    /* if option doesn't need expansion nothing to do */
    if (!(options[opt_idx].flags & P_EXPAND) || options[opt_idx].var == NULL)
	return NULL;

    /* If val is longer than MAXPATHL no meaningful expansion can be done,
     * expand_env() would truncate the string. */
    if (val != NULL && STRLEN(val) > MAXPATHL)
	return NULL;

    if (val == NULL)
	val = *(char_u **)options[opt_idx].var;

    /*
     * Expanding this with NameBuff, expand_env() must not be passed IObuff.
     */
    expand_env(val, NameBuff, MAXPATHL);
    if (STRCMP(NameBuff, val) == 0)   /* they are the same */
	return NULL;

    return NameBuff;
}

/*
 * After setting various option values: recompute variables that depend on
 * option values.
 */
    static void
didset_options()
{
    /* initialize the table for 'iskeyword' et.al. */
    (void)init_chartab();

#ifdef FEAT_SESSION
    (void)opt_strings_flags(p_ssop, p_ssop_values, &ssop_flags, TRUE);
    (void)opt_strings_flags(p_vop, p_ssop_values, &vop_flags, TRUE);
#endif
#ifdef FEAT_FOLDING
    (void)opt_strings_flags(p_fdo, p_fdo_values, &fdo_flags, TRUE);
#endif
    (void)opt_strings_flags(p_dy, p_dy_values, &dy_flags, TRUE);
#ifdef FEAT_VIRTUALEDIT
    (void)opt_strings_flags(p_ve, p_ve_values, &ve_flags, TRUE);
#endif
#if defined(FEAT_MOUSE) && (defined(UNIX) || defined(VMS))
    (void)opt_strings_flags(p_ttym, p_ttym_values, &ttym_flags, FALSE);
#endif
#ifdef FEAT_CMDWIN
    /* set cedit_key */
    (void)check_cedit();
#endif
}

/*
 * Check for string options that are NULL (normally only termcap options).
 */
    void
check_options()
{
    int		opt_idx;

    for (opt_idx = 0; options[opt_idx].fullname != NULL; opt_idx++)
	if ((options[opt_idx].flags & P_STRING) && options[opt_idx].var != NULL)
	    check_string_option((char_u **)get_varp(&(options[opt_idx])));
}

/*
 * Check string options in a buffer for NULL value.
 */
    void
check_buf_options(buf)
    buf_T	*buf;
{
#if defined(FEAT_QUICKFIX)
    check_string_option(&buf->b_p_bh);
    check_string_option(&buf->b_p_bt);
#endif
#ifdef FEAT_MBYTE
    check_string_option(&buf->b_p_fenc);
#endif
    check_string_option(&buf->b_p_ff);
#ifdef FEAT_FIND_ID
    check_string_option(&buf->b_p_def);
    check_string_option(&buf->b_p_inc);
# ifdef FEAT_EVAL
    check_string_option(&buf->b_p_inex);
# endif
#endif
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
    check_string_option(&buf->b_p_inde);
    check_string_option(&buf->b_p_indk);
#endif
#ifdef FEAT_CRYPT
    check_string_option(&buf->b_p_key);
#endif
    check_string_option(&buf->b_p_mps);
    check_string_option(&buf->b_p_fo);
    check_string_option(&buf->b_p_isk);
#ifdef FEAT_COMMENTS
    check_string_option(&buf->b_p_com);
#endif
#ifdef FEAT_FOLDING
    check_string_option(&buf->b_p_cms);
#endif
    check_string_option(&buf->b_p_nf);
#ifdef FEAT_SYN_HL
    check_string_option(&buf->b_p_syn);
#endif
#ifdef FEAT_SEARCHPATH
    check_string_option(&buf->b_p_sua);
#endif
#ifdef FEAT_CINDENT
    check_string_option(&buf->b_p_cink);
    check_string_option(&buf->b_p_cino);
#endif
#ifdef FEAT_AUTOCMD
    check_string_option(&buf->b_p_ft);
#endif
#ifdef FEAT_OSFILETYPE
    check_string_option(&buf->b_p_oft);
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
    check_string_option(&buf->b_p_cinw);
#endif
#ifdef FEAT_INS_EXPAND
    check_string_option(&buf->b_p_cpt);
#endif
#ifdef FEAT_KEYMAP
    check_string_option(&buf->b_p_keymap);
#endif
#ifdef FEAT_QUICKFIX
    check_string_option(&buf->b_p_gp);
    check_string_option(&buf->b_p_mp);
    check_string_option(&buf->b_p_efm);
#endif
    check_string_option(&buf->b_p_ep);
    check_string_option(&buf->b_p_path);
    check_string_option(&buf->b_p_tags);
#ifdef FEAT_INS_EXPAND
    check_string_option(&buf->b_p_dict);
    check_string_option(&buf->b_p_tsr);
#endif
}

/*
 * Free the string allocated for an option.
 * Checks for the string being empty_option. This may happen if we're out of
 * memory, vim_strsave() returned NULL, which was replaced by empty_option by
 * check_options().
 * Does NOT check for P_ALLOCED flag!
 */
    void
free_string_option(p)
    char_u	*p;
{
    if (p != empty_option)
	vim_free(p);
}

    void
clear_string_option(pp)
    char_u	**pp;
{
    if (*pp != empty_option)
	vim_free(*pp);
    *pp = empty_option;
}

    static void
check_string_option(pp)
    char_u	**pp;
{
    if (*pp == NULL)
	*pp = empty_option;
}

/*
 * Mark a terminal option as allocated, found by a pointer into term_strings[].
 */
    void
set_term_option_alloced(p)
    char_u **p;
{
    int		opt_idx;

    for (opt_idx = 1; options[opt_idx].fullname != NULL; opt_idx++)
	if (options[opt_idx].var == (char_u *)p)
	{
	    options[opt_idx].flags |= P_ALLOCED;
	    return;
	}
    return; /* cannot happen: didn't find it! */
}

/*
 * Set a string option to a new value (without checking the effect).
 * The string is copied into allocated memory.
 * if ("opt_idx" == -1) "name" is used, otherwise "opt_idx" is used.
 */
    void
set_string_option_direct(name, opt_idx, val, opt_flags)
    char_u	*name;
    int		opt_idx;
    char_u	*val;
    int		opt_flags;	/* OPT_FREE, OPT_LOCAL and/or OPT_GLOBAL */
{
    char_u	*s;
    char_u	**varp;
    int		both = (opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0;

    if (opt_idx == -1)		/* use name */
    {
	opt_idx = findoption(name);
	if (opt_idx == -1)	/* not found (should not happen) */
	    return;
    }

    if (options[opt_idx].var == NULL)	/* can't set hidden option */
	return;

    s = vim_strsave(val);
    if (s != NULL)
    {
	varp = (char_u **)get_varp_scope(&(options[opt_idx]),
					       both ? OPT_LOCAL : opt_flags);
	if ((opt_flags & OPT_FREE) && (options[opt_idx].flags & P_ALLOCED))
	    free_string_option(*varp);
	*varp = s;

	/* For buffer/window local option may also set the global value. */
	if (both)
	    set_string_option_global(opt_idx, varp);

	options[opt_idx].flags |= P_ALLOCED;

	/* When setting both values of a global option with a local value,
	 * make the local value empty, so that the global value is used. */
	if ((int)options[opt_idx].indir >= PV_BOTH && both)
	{
	    free_string_option(*varp);
	    *varp = empty_option;
	}
    }
}

/*
 * Set global value for string option when it's a local option.
 */
    static void
set_string_option_global(opt_idx, varp)
    int		opt_idx;	/* option index */
    char_u	**varp;		/* pointer to option variable */
{
    char_u	**p, *s;

    /* the global value is always allocated */
    if (options[opt_idx].var == VAR_WIN)
	p = (char_u **)GLOBAL_WO(varp);
    else
	p = (char_u **)options[opt_idx].var;
    if (options[opt_idx].indir != PV_NONE
	    && p != varp
	    && (s = vim_strsave(*varp)) != NULL)
    {
	free_string_option(*p);
	*p = s;
    }
}

/*
 * Set a string option to a new value, and handle the effects.
 */
    static void
set_string_option(opt_idx, value, opt_flags)
    int		opt_idx;
    char_u	*value;
    int		opt_flags;	/* OPT_LOCAL and/or OPT_GLOBAL */
{
    char_u	*s;
    char_u	**varp;
    char_u	*oldval;

    if (options[opt_idx].var == NULL)	/* don't set hidden option */
	return;

    s = vim_strsave(value);
    if (s != NULL)
    {
	varp = (char_u **)get_varp_scope(&(options[opt_idx]),
		(opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
		    ? ((int)options[opt_idx].indir >= PV_BOTH
			? OPT_GLOBAL : OPT_LOCAL)
		    : opt_flags);
	oldval = *varp;
	*varp = s;
	options[opt_idx].flags |= P_WAS_SET;
	(void)did_set_string_option(opt_idx, varp, TRUE, oldval, NULL,
								   opt_flags);
    }
}

/*
 * Handle string options that need some action to perform when changed.
 * Returns NULL for success, or an error message for an error.
 */
    static char_u *
did_set_string_option(opt_idx, varp, new_value_alloced, oldval, errbuf,
								    opt_flags)
    int		opt_idx;		/* index in options[] table */
    char_u	**varp;			/* pointer to the option variable */
    int		new_value_alloced;	/* new value was allocated */
    char_u	*oldval;		/* previous value of the option */
    char_u	*errbuf;		/* buffer for errors, or NULL */
    int		opt_flags;		/* OPT_LOCAL and/or OPT_GLOBAL */
{
    char_u	*errmsg = NULL;
    char_u	*s, *p;
    int		did_chartab = FALSE;
    char_u	**gvarp;

    /* Get the global option to compare with, otherwise we would have to check
     * two values for all local options. */
    gvarp = (char_u **)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL);

    /* Disallow changing some options from secure mode */
    if ((secure
#ifdef HAVE_SANDBOX
		|| sandbox != 0
#endif
		) && (options[opt_idx].flags & P_SECURE))
    {
	errmsg = (char_u *)N_("Not allowed here");
    }

    /* 'term' */
    else if (varp == &T_NAME)
    {
	if (T_NAME[0] == NUL)
	    errmsg = (char_u *)N_("Cannot set 'term' to empty string");
#ifdef FEAT_GUI
	if (gui.in_use)
	    errmsg = (char_u *)N_("Cannot change term in GUI");
	else if (term_is_gui(T_NAME))
	    errmsg = (char_u *)N_("Use \":gui\" to start the GUI");
#endif
	else if (set_termname(T_NAME) == FAIL)
	    errmsg = (char_u *)N_("Not found in termcap");
	else
	    /* Screen colors may have changed. */
	    redraw_later_clear();
    }

    /* 'backupcopy' */
    else if (varp == &p_bkc)
    {
	if (check_opt_strings(p_bkc, p_bkc_values, FALSE) != OK)
	    errmsg = e_invarg;
    }

    /* 'backupext' and 'patchmode' */
    else if (varp == &p_bex || varp == &p_pm)
    {
	if (STRCMP(*p_bex == '.' ? p_bex + 1 : p_bex,
		     *p_pm == '.' ? p_pm + 1 : p_pm) == 0)
	    errmsg = (char_u *)N_("'backupext' and 'patchmode' are equal");
    }

    /*
     * 'isident', 'iskeyword', 'isprint or 'isfname' option: refill chartab[]
     * If the new option is invalid, use old value.  'lisp' option: refill
     * chartab[] for '-' char
     */
    else if (  varp == &p_isi
	    || varp == &(curbuf->b_p_isk)
	    || varp == &p_isp
	    || varp == &p_isf)
    {
	if (init_chartab() == FAIL)
	{
	    did_chartab = TRUE;	    /* need to restore it below */
	    errmsg = e_invarg;	    /* error in value */
	}
    }

    /* 'helpfile' */
    else if (varp == &p_hf)
    {
	/* May compute new values for $VIM and $VIMRUNTIME */
	if (didset_vim)
	{
	    vim_setenv((char_u *)"VIM", (char_u *)"");
	    didset_vim = FALSE;
	}
	if (didset_vimruntime)
	{
	    vim_setenv((char_u *)"VIMRUNTIME", (char_u *)"");
	    didset_vimruntime = FALSE;
	}
    }

    /* 'highlight' */
    else if (varp == &p_hl)
    {
	if (highlight_changed() == FAIL)
	    errmsg = e_invarg;	/* invalid flags */
    }

    /* 'nrformats' */
    else if (gvarp == &p_nf)
    {
	if (check_opt_strings(*varp, p_nf_values, TRUE) != OK)
	    errmsg = e_invarg;
    }

#ifdef FEAT_SESSION
    /* 'sessionoptions' */
    else if (varp == &p_ssop)
    {
	if (opt_strings_flags(p_ssop, p_ssop_values, &ssop_flags, TRUE) != OK)
	    errmsg = e_invarg;
	if ((ssop_flags & SSOP_CURDIR) && (ssop_flags & SSOP_SESDIR))
	{
	    /* Don't allow both "sesdir" and "curdir". */
	    (void)opt_strings_flags(oldval, p_ssop_values, &ssop_flags, TRUE);
	    errmsg = e_invarg;
	}
    }
    /* 'viewoptions' */
    else if (varp == &p_vop)
    {
	if (opt_strings_flags(p_vop, p_ssop_values, &vop_flags, TRUE) != OK)
	    errmsg = e_invarg;
    }
#endif

    /* 'scrollopt' */
#ifdef FEAT_SCROLLBIND
    else if (varp == &p_sbo)
    {
	if (check_opt_strings(p_sbo, p_scbopt_values, TRUE) != OK)
	    errmsg = e_invarg;
    }
#endif

    /* 'background' */
    else if (varp == &p_bg)
    {
	if (check_opt_strings(p_bg, p_bg_values, FALSE) == OK)
	{
#ifdef FEAT_EVAL
	    int dark = (*p_bg == 'd');
#endif

	    init_highlight(FALSE, FALSE);

#ifdef FEAT_EVAL
	    if (dark != (*p_bg == 'd')
			  && get_var_value((char_u *)"g:colors_name") != NULL)
	    {
		/* The color scheme must have set 'background' back to another
		 * value, that's not what we want here.  Disable the color
		 * scheme and set the colors again. */
		do_unlet((char_u *)"g:colors_name");
		free_string_option(p_bg);
		p_bg = vim_strsave((char_u *)(dark ? "dark" : "light"));
		check_string_option(&p_bg);
		init_highlight(FALSE, FALSE);
	    }
#endif
	}
	else
	    errmsg = e_invarg;
    }

    /* 'wildmode' */
    else if (varp == &p_wim)
    {
	if (check_opt_wim() == FAIL)
	    errmsg = e_invarg;
    }

#ifdef FEAT_WAK
    /* 'winaltkeys' */
    else if (varp == &p_wak)
    {
	if (*p_wak == NUL
		|| check_opt_strings(p_wak, p_wak_values, FALSE) != OK)
	    errmsg = e_invarg;
# ifdef FEAT_MENU
#  ifdef FEAT_GUI_MOTIF
	else if (gui.in_use)
	    gui_motif_set_mnemonics(p_wak[0] == 'y' || p_wak[0] == 'm');
#  else
#   ifdef FEAT_GUI_GTK
	else if (gui.in_use)
	    gui_gtk_set_mnemonics(p_wak[0] == 'y' || p_wak[0] == 'm');
#   endif
#  endif
# endif
    }
#endif

#ifdef FEAT_AUTOCMD
    /* 'eventignore' */
    else if (varp == &p_ei)
    {
	if (check_ei() == FAIL)
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_MBYTE
    /* 'encoding' and 'fileencoding' */
    else if (varp == &p_enc || gvarp == &p_fenc || varp == &p_tenc)
    {
	if (varp == &curbuf->b_p_fenc && !curbuf->b_p_ma)
	    errmsg = e_modifiable;
	else
	{
	    /* canonize the value, so that STRCMP() can be used on it */
	    p = enc_canonize(*varp);
	    if (p != NULL)
	    {
		vim_free(*varp);
		*varp = p;
	    }
	    if (varp == &p_enc)
	    {
		errmsg = mb_init();
# ifdef FEAT_TITLE
		need_maketitle = TRUE;
# endif
	    }
	}

	if (errmsg == NULL)
	{
# ifdef FEAT_KEYMAP
	    /* When 'keymap' is used and 'encoding' changes, reload the keymap
	     * (with another encoding). */
	    if (varp == &p_enc && *curbuf->b_p_keymap != NUL)
		(void)keymap_init();
# endif

	    /* When 'termencoding' is not empty and 'encoding' changes or when
	     * 'termencoding' changes, need to setup for keyboard input and
	     * display output conversion. */
	    if (((varp == &p_enc && *p_tenc != NUL) || varp == &p_tenc))
	    {
		convert_setup(&input_conv, p_tenc, p_enc);
		convert_setup(&output_conv, p_enc, p_tenc);
	    }
	}
    }
#endif

#if defined(FEAT_XIM) && defined(FEAT_GUI_GTK)
    else if (varp == &p_imak)
    {
	if (!im_xim_isvalid_imactivate())
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_KEYMAP
    else if (varp == &curbuf->b_p_keymap)
    {
	/* load or unload key mapping tables */
	errmsg = keymap_init();

	/* When successfully installed a new keymap switch on using it. */
	if (*curbuf->b_p_keymap != NUL && errmsg == NULL)
	{
	    curbuf->b_p_iminsert = B_IMODE_LMAP;
	    if (curbuf->b_p_imsearch != B_IMODE_USE_INSERT)
		curbuf->b_p_imsearch = B_IMODE_LMAP;
	    set_iminsert_global();
	    set_imsearch_global();
# ifdef FEAT_WINDOWS
	    status_redraw_curbuf();
# endif
	}
    }
#endif

    /* 'fileformat' */
    else if (gvarp == &p_ff)
    {
	if (!curbuf->b_p_ma && !(opt_flags & OPT_GLOBAL))
	    errmsg = e_modifiable;
	else if (check_opt_strings(*varp, p_ff_values, FALSE) != OK)
	    errmsg = e_invarg;
	else
	{
	    /* may also change 'textmode' */
	    if (get_fileformat(curbuf) == EOL_DOS)
		curbuf->b_p_tx = TRUE;
	    else
		curbuf->b_p_tx = FALSE;
#ifdef FEAT_TITLE
	    need_maketitle = TRUE;
#endif
	}
    }

    /* 'fileformats' */
    else if (varp == &p_ffs)
    {
	if (check_opt_strings(p_ffs, p_ff_values, TRUE) != OK)
	    errmsg = e_invarg;
	else
	{
	    /* also change 'textauto' */
	    if (*p_ffs == NUL)
		p_ta = FALSE;
	    else
		p_ta = TRUE;
	}
    }

#if defined(FEAT_CRYPT) && defined(FEAT_CMDHIST)
    /* 'cryptkey' */
    else if (gvarp == &p_key)
    {
	/* Make sure the ":set" command doesn't show the new value in the
	 * history. */
	remove_key_from_history();
    }
#endif

    /* 'matchpairs' */
    else if (gvarp == &p_mps)
    {
	/* Check for "x:y,x:y" */
	for (p = *varp; *p; p += 4)
	{
	    if (!p[0] || p[1] != ':' || !p[2] || (p[3] && p[3] != ','))
	    {
		errmsg = e_invarg;
		break;
	    }
	    if (!p[3])
		break;
	}
    }

#ifdef FEAT_COMMENTS
    /* 'comments' */
    else if (gvarp == &p_com)
    {
	for (s = *varp; *s; )
	{
	    while (*s && *s != ':')
	    {
		if (vim_strchr((char_u *)COM_ALL, *s) == NULL
						 && !isdigit(*s) && *s != '-')
		{
		    errmsg = illegal_char(errbuf, *s);
		    break;
		}
		++s;
	    }
	    if (*s++ == NUL)
		errmsg = (char_u *)N_("Missing colon");
	    else if (*s == ',' || *s == NUL)
		errmsg = (char_u *)N_("Zero length string");
	    if (errmsg != NULL)
		break;
	    while (*s && *s != ',')
	    {
		if (*s == '\\' && s[1] != NUL)
		    ++s;
		++s;
	    }
	    s = skip_to_option_part(s);
	}
    }
#endif

    /* 'listchars' */
    else if (varp == &p_lcs)
    {
	errmsg = set_chars_option(varp);
    }

#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
    /* 'fillchars' */
    else if (varp == &p_fcs)
    {
	errmsg = set_chars_option(varp);
    }
#endif

#ifdef FEAT_CMDWIN
    /* 'cedit' */
    else if (varp == &p_cedit)
    {
	errmsg = check_cedit();
    }
#endif

#ifdef FEAT_VIMINFO
    /* 'viminfo' */
    else if (varp == &p_viminfo)
    {
	for (s = p_viminfo; *s;)
	{
	    /* Check it's a valid character */
	    if (vim_strchr((char_u *)"!\"%'/:@cfhnr", *s) == NULL)
	    {
		errmsg = illegal_char(errbuf, *s);
		break;
	    }
	    if (*s == 'n')	/* name is always last one */
	    {
		break;
	    }
	    else if (*s == 'r') /* skip until next ',' */
	    {
		while (*++s && *s != ',')
		    ;
	    }
	    else if (*s == '%' || *s == '!' || *s == 'h' || *s == 'c')
		++s;		/* no extra chars */
	    else		/* must have a number */
	    {
		while (isdigit(*++s))
		    ;

		if (!isdigit(*(s - 1)))
		{
		    if (errbuf != NULL)
		    {
			sprintf((char *)errbuf, _("Missing number after <%s>"),
						    transchar_byte(*(s - 1)));
			errmsg = errbuf;
		    }
		    else
			errmsg = (char_u *)"";
		    break;
		}
	    }
	    if (*s == ',')
		++s;
	    else if (*s)
	    {
		if (errbuf != NULL)
		    errmsg = (char_u *)N_("Missing comma");
		else
		    errmsg = (char_u *)"";
		break;
	    }
	}
	if (*p_viminfo && errmsg == NULL && get_viminfo_parameter('\'') < 0)
	    errmsg = (char_u *)N_("Must specify a ' value");
    }
#endif /* FEAT_VIMINFO */

    /* terminal options */
    else if (istermoption(&options[opt_idx]) && full_screen)
    {
	/* ":set t_Co=0" and ":set t_Co=1" do ":set t_Co=" */
	if (varp == &T_CCO)
	{
	    t_colors = atoi((char *)T_CCO);
	    if (t_colors <= 1)
	    {
		if (new_value_alloced)
		    vim_free(T_CCO);
		T_CCO = empty_option;
	    }
	    /* We now have a different color setup, initialize it again. */
	    init_highlight(TRUE, FALSE);
	}
	ttest(FALSE);
	if (varp == &T_ME)
	{
	    out_str(T_ME);
	    redraw_later(CLEAR);
#if defined(MSDOS) || (defined(WIN3264) && !defined(FEAT_GUI_W32))
	    /* Since t_me has been set, this probably means that the user
	     * wants to use this as default colors.  Need to reset default
	     * background/foreground colors. */
	    mch_set_normal_colors();
#endif
	}
    }

#ifdef FEAT_LINEBREAK
    /* 'showbreak' */
    else if (varp == &p_sbr)
    {
	for (s = p_sbr; *s; ++s)
	    if (byte2cells(*s) != 1)
		errmsg = (char_u *)N_("contains unprintable character");
    }
#endif

#ifdef FEAT_GUI
    /* 'guifont' */
    else if (varp == &p_guifont)
    {
	if (gui.in_use)
	{
# ifdef FEAT_GUI_GTK
	    if (STRCMP(p_guifont, "*") == 0)
	    {
		/*
		 * Put up a font dialog and let the user select a new value.
		 * If this is cancelled go back to the old value but don't
		 * give an error message.
		 */
		p = gui_mch_font_dialog(oldval);
		if (p != NULL)
		{
		    free_string_option(p_guifont);
		    p_guifont = p;
		}
		else
		    errmsg = (char_u *)"";
	    }
# endif
	    if (errmsg == NULL && gui_init_font(p_guifont, FALSE) != OK
# if defined(FEAT_GUI_MSWIN) || defined(FEAT_GUI_PHOTON)
		    && STRCMP(p_guifont, "*") != 0
# endif
	       )
		errmsg = (char_u *)N_("Invalid font(s)");
	}
    }
# ifdef FEAT_XFONTSET
    else if (varp == &p_guifontset)
    {
	if (STRCMP(p_guifontset, "*") == 0)
	    errmsg = (char_u *)N_("can't select fontset");
	else if (gui.in_use && gui_init_font(p_guifontset, TRUE) != OK)
	    errmsg = (char_u *)N_("Invalid fontset");
    }
# endif
# ifdef FEAT_MBYTE
    else if (varp == &p_guifontwide)
    {
	if (STRCMP(p_guifontwide, "*") == 0)
	    errmsg = (char_u *)N_("can't select wide font");
	else if (gui_get_wide_font() == FAIL)
	    errmsg = (char_u *)N_("Invalid wide font");
    }
# endif
#endif

#ifdef CURSOR_SHAPE
    /* 'guicursor' */
    else if (varp == &p_guicursor)
	errmsg = parse_shape_opt(SHAPE_CURSOR);
#endif

#ifdef FEAT_MOUSESHAPE
    /* 'mouseshape' */
    else if (varp == &p_mouseshape)
    {
	errmsg = parse_shape_opt(SHAPE_MOUSE);
	update_mouseshape(-1);
    }
#endif

#ifdef FEAT_PRINTER
    else if (varp == &p_popt)
	errmsg = parse_list_options(p_popt, printer_opts, OPT_PRINT_NUM_OPTIONS);
#endif

#ifdef FEAT_LANGMAP
    /* 'langmap' */
    else if (varp == &p_langmap)
	langmap_set();
#endif

#ifdef FEAT_LINEBREAK
    /* 'breakat' */
    else if (varp == &p_breakat)
	fill_breakat_flags();
#endif

#ifdef FEAT_TITLE
    /* 'titlestring' and 'iconstring' */
    else if (varp == &p_titlestring || varp == &p_iconstring)
    {
# ifdef FEAT_STL_OPT
	int	flagval = (varp == &p_titlestring) ? STL_IN_TITLE : STL_IN_ICON;

	/* NULL => statusline syntax */
	if (vim_strchr(*varp, '%') && check_stl_option(*varp) == NULL)
	    stl_syntax |= flagval;
	else
	    stl_syntax &= ~flagval;
# endif
	did_set_title(varp == &p_iconstring);

    }
#endif

#ifdef FEAT_GUI
    /* 'guioptions' */
    else if (varp == &p_go)
	gui_init_which_components(oldval);
#endif

#if defined(FEAT_MOUSE_TTY) && (defined(UNIX) || defined(VMS))
    /* 'ttymouse' */
    else if (varp == &p_ttym)
    {
	if (opt_strings_flags(p_ttym, p_ttym_values, &ttym_flags, FALSE) != OK)
	    errmsg = e_invarg;
	else
	    check_mouse_termcode();
    }
#endif

#ifdef FEAT_VISUAL
    /* 'selection' */
    else if (varp == &p_sel)
    {
	if (*p_sel == NUL
		|| check_opt_strings(p_sel, p_sel_values, FALSE) != OK)
	    errmsg = e_invarg;
    }

    /* 'selectmode' */
    else if (varp == &p_slm)
    {
	if (check_opt_strings(p_slm, p_slm_values, TRUE) != OK)
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_BROWSE
    /* 'browsedir' */
    else if (varp == &p_bsdir)
    {
	if (check_opt_strings(p_bsdir, p_bsdir_values, FALSE) != OK
		&& !mch_isdir(p_bsdir))
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_VISUAL
    /* 'keymodel' */
    else if (varp == &p_km)
    {
	if (check_opt_strings(p_km, p_km_values, TRUE) != OK)
	    errmsg = e_invarg;
	else
	{
	    km_stopsel = (vim_strchr(p_km, 'o') != NULL);
	    km_startsel = (vim_strchr(p_km, 'a') != NULL);
	}
    }
#endif

    /* 'mousemodel' */
    else if (varp == &p_mousem)
    {
	if (check_opt_strings(p_mousem, p_mousem_values, FALSE) != OK)
	    errmsg = e_invarg;
#if defined(FEAT_GUI_MOTIF) && defined(FEAT_MENU) && (XmVersion <= 1002)
	else if (*p_mousem != *oldval)
	    /* Changed from "extend" to "popup" or "popup_setpos" or vv: need
	     * to create or delete the popup menus. */
	    gui_motif_update_mousemodel(root_menu);
#endif
    }

    /* 'switchbuf' */
    else if (varp == &p_swb)
    {
	if (check_opt_strings(p_swb, p_swb_values, TRUE) != OK)
	    errmsg = e_invarg;
    }

    /* 'debug' */
    else if (varp == &p_debug)
    {
	if (check_opt_strings(p_debug, p_debug_values, FALSE) != OK)
	    errmsg = e_invarg;
    }

    /* 'display' */
    else if (varp == &p_dy)
    {
	if (opt_strings_flags(p_dy, p_dy_values, &dy_flags, TRUE) != OK)
	    errmsg = e_invarg;
	else
	    (void)init_chartab();

    }

#ifdef FEAT_VERTSPLIT
    /* 'eadirection' */
    else if (varp == &p_ead)
    {
	if (check_opt_strings(p_ead, p_ead_values, FALSE) != OK)
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_CLIPBOARD
    /* 'clipboard' */
    else if (varp == &p_cb)
	errmsg = check_clipboard_option();
#endif

#ifdef FEAT_AUTOCMD
# ifdef FEAT_SYN_HL
    /* When 'syntax' is set, load the syntax of that name */
    else if (varp == &(curbuf->b_p_syn))
    {
	apply_autocmds(EVENT_SYNTAX, curbuf->b_p_syn,
					     curbuf->b_fname, TRUE, curbuf);
    }
# endif

    /* When 'filetype' is set, trigger the FileType autocommands of that name */
    else if (varp == &(curbuf->b_p_ft))
    {
	did_filetype = TRUE;
	apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft,
					     curbuf->b_fname, TRUE, curbuf);
    }
#endif

#ifdef FEAT_QUICKFIX
    /* When 'bufhidden' is set, check for valid value. */
    else if (gvarp == &p_bh)
    {
	if (check_opt_strings(curbuf->b_p_bh, p_bufhidden_values, FALSE) != OK)
	    errmsg = e_invarg;
    }

    /* When 'buftype' is set, check for valid value. */
    else if (gvarp == &p_bt)
    {
	if (check_opt_strings(curbuf->b_p_bt, p_buftype_values, FALSE) != OK)
	    errmsg = e_invarg;
	else
	{
# ifdef FEAT_WINDOWS
	    if (curwin->w_status_height)
	    {
		curwin->w_redr_status = TRUE;
		redraw_later(VALID);
	    }
# endif
	    curbuf->b_help = (curbuf->b_p_bt[0] == 'h');
	}
    }
#endif

#ifdef FEAT_STL_OPT
    /* 'statusline' or 'rulerformat' */
    else if (varp == &p_stl || varp == &p_ruf)
    {
	int wid;

	if (varp == &p_ruf)	/* reset ru_wid first */
	    ru_wid = 0;
	s = *varp;
	if (varp == &p_ruf && *s == '%')
	{
	    /* set ru_wid if 'ruf' starts with "%99(" */
	    if (*++s == '-')	/* ignore a '-' */
		s++;
	    wid = getdigits(&s);
	    if (wid && *s == '(' && (errmsg = check_stl_option(p_ruf)) == NULL)
		ru_wid = wid;
	    else
		errmsg = check_stl_option(p_ruf);
	}
	else
	    errmsg = check_stl_option(s);
	if (varp == &(p_ruf) && errmsg == NULL)
	    comp_col();
    }
#endif

#ifdef FEAT_INS_EXPAND
    /* check if it is a valid value for 'complete' -- Acevedo */
    else if (gvarp == &p_cpt)
    {
	for (s = *varp; *s;)
	{
	    while(*s == ',' || *s == ' ')
		s++;
	    if (!*s)
		break;
	    if (vim_strchr((char_u *)".wbuksid]tU", *s) == NULL)
	    {
		errmsg = illegal_char(errbuf, *s);
		break;
	    }
	    if (*++s != NUL && *s != ',' && *s != ' ')
	    {
		if (s[-1] == 'k' || s[-1] == 's')
		{
		    /* skip optional filename after 'k' and 's' */
		    while (*s && *s != ',' && *s != ' ')
		    {
			if (*s == '\\')
			    ++s;
			++s;
		    }
		}
		else
		{
		    if (errbuf != NULL)
		    {
			sprintf((char *)errbuf,
				     _("Illegal character after <%c>"), *--s);
			errmsg = errbuf;
		    }
		    else
			errmsg = (char_u *)"";
		    break;
		}
	    }
	}
    }
#endif /* FEAT_INS_EXPAND */


#if defined(FEAT_TOOLBAR) && !defined(FEAT_GUI_W32)
    else if (varp == &p_toolbar)
    {
	if (p_toolbar && (strstr((const char *)p_toolbar, "text")
				 || strstr((const char *)p_toolbar, "icons")))
	    gui_mch_show_toolbar(TRUE);
	else
	    gui_mch_show_toolbar(FALSE);
    }
#endif

    /* 'pastetoggle': translate key codes like in a mapping */
    else if (varp == &p_pt)
    {
	if (*p_pt)
	{
	    (void)replace_termcodes(p_pt, &p, TRUE, TRUE);
	    if (p != NULL)
	    {
		if (new_value_alloced)
		    free_string_option(p_pt);
		p_pt = p;
		new_value_alloced = TRUE;
	    }
	}
    }

    /* 'backspace' */
    else if (varp == &p_bs)
    {
	if (isdigit(*p_bs))
	{
	    if (*p_bs >'2' || p_bs[1] != NUL)
		errmsg = e_invarg;
	}
	else if (check_opt_strings(p_bs, p_bs_values, TRUE) != OK)
	    errmsg = e_invarg;
    }

#ifdef FEAT_DIFF
    /* 'diffopt' */
    else if (varp == &p_dip)
    {
	if (diffopt_changed() == FAIL)
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_FOLDING
    /* 'foldmethod' */
    else if (gvarp == &curwin->w_allbuf_opt.wo_fdm)
    {
	if (check_opt_strings(*varp, p_fdm_values, FALSE) != OK
		|| *curwin->w_p_fdm == NUL)
	    errmsg = e_invarg;
	else
	    foldUpdateAll(curwin);
    }
# ifdef FEAT_EVAL
    /* 'foldexpr' */
    else if (varp == &curwin->w_p_fde)
    {
	if (foldmethodIsExpr(curwin))
	    foldUpdateAll(curwin);
    }
# endif
    /* 'foldmarker' */
    else if (gvarp == &curwin->w_allbuf_opt.wo_fmr)
    {
	if (vim_strchr(*varp, ',') == NULL)
	    errmsg = (char_u *)N_("comma required");
	else if (foldmethodIsMarker(curwin))
	    foldUpdateAll(curwin);
    }
    /* 'commentstring' */
    else if (gvarp == &p_cms)
    {
	if (**varp != NUL && strstr((char *)*varp, "%s") == NULL)
	    errmsg = (char_u *)N_("'commentstring' must be empty or contain %s");
    }
    /* 'foldopen' */
    else if (varp == &p_fdo)
    {
	if (opt_strings_flags(p_fdo, p_fdo_values, &fdo_flags, TRUE) != OK)
	    errmsg = e_invarg;
    }
    /* 'foldclose' */
    else if (varp == &p_fcl)
    {
	if (check_opt_strings(p_fcl, p_fcl_values, TRUE) != OK)
	    errmsg = e_invarg;
    }
#endif

#ifdef FEAT_VIRTUALEDIT
    /* 'virtualedit' */
    else if (varp == &p_ve)
    {
	if (opt_strings_flags(p_ve, p_ve_values, &ve_flags, TRUE) != OK)
	    errmsg = e_invarg;
	else
	    /* Recompute cursor position in case the new ve setting
	     * changes something. */
	    coladvance(curwin->w_virtcol);
    }
#endif

    /* Options that are a list of flags. */
    else
    {
	p = NULL;
	if (varp == &p_ww)
	    p = (char_u *)WW_ALL;
	if (varp == &p_shm)
	    p = (char_u *)SHM_ALL;
	else if (varp == &(p_cpo))
	    p = (char_u *)CPO_ALL;
	else if (varp == &(curbuf->b_p_fo))
	    p = (char_u *)FO_ALL;
	else if (varp == &p_mouse)
	{
#ifdef FEAT_MOUSE
	    p = (char_u *)MOUSE_ALL;
#else
	    if (*p_mouse != NUL)
		errmsg = (char_u *)N_("No mouse support");
#endif
	}
#if defined(FEAT_GUI)
	else if (varp == &p_go)
	    p = (char_u *)GO_ALL;
#endif
	if (p != NULL)
	{
	    for (s = *varp; *s; ++s)
		if (vim_strchr(p, *s) == NULL)
		{
		    errmsg = illegal_char(errbuf, *s);
		    break;
		}
	}
    }

    /*
     * If error detected, restore the previous value.
     */
    if (errmsg != NULL)
    {
	if (new_value_alloced)
	    free_string_option(*varp);
	*varp = oldval;
	/*
	 * When resetting some values, need to act on it.
	 */
	if (did_chartab)
	    (void)init_chartab();
	if (varp == &p_hl)
	    (void)highlight_changed();
    }
    else
    {
#ifdef FEAT_EVAL
	/* Remember where the option was set. */
	options[opt_idx].scriptID = current_SID;
#endif
	/*
	 * Free string options that are in allocated memory.
	 */
	if (options[opt_idx].flags & P_ALLOCED)
	    free_string_option(oldval);
	if (new_value_alloced)
	    options[opt_idx].flags |= P_ALLOCED;
	else
	    options[opt_idx].flags &= ~P_ALLOCED;

	if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
		&& (int)options[opt_idx].indir >= PV_BOTH)
	{
	    /* global option with local value set to use global value; free
	     * the local value and make it empty */
	    p = get_varp_scope(&(options[opt_idx]), OPT_LOCAL);
	    free_string_option(*(char_u **)p);
	    *(char_u **)p = empty_option;
	}

	/* May set global value for local option. */
	else if (!(opt_flags & OPT_LOCAL) && opt_flags != OPT_GLOBAL)
	    set_string_option_global(opt_idx, varp);
    }

#ifdef FEAT_MOUSE
    if (varp == &p_mouse)
    {
# ifdef FEAT_MOUSE_TTY
	if (*p_mouse == NUL)
	    mch_setmouse(FALSE);    /* switch mouse off */
	else
# endif
	    setmouse();		    /* in case 'mouse' changed */
    }
#endif

    if (curwin->w_curswant != MAXCOL)
	curwin->w_set_curswant = TRUE;  /* in case 'showbreak' changed */
    check_redraw(options[opt_idx].flags);

    return errmsg;
}

/*
 * Handle setting 'listchars' or 'fillchars'.
 * Returns error message, NULL if it's OK.
 */
    static char_u *
set_chars_option(varp)
    char_u	**varp;
{
    int		round, i, len, entries;
    char_u	*p;
    struct charstab
    {
	int	*cp;
	char	*name;
    };
#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
    static struct charstab filltab[] =
    {
	{&fill_stl,	"stl"},
	{&fill_stlnc,	"stlnc"},
	{&fill_vert,	"vert"},
	{&fill_fold,	"fold"},
	{&fill_diff,	"diff"},
    };
#endif
    static struct charstab lcstab[] =
    {
	{&lcs_eol,	"eol"},
	{&lcs_ext,	"extends"},
	{&lcs_prec,	"precedes"},
	{&lcs_tab2,	"tab"},
	{&lcs_trail,	"trail"},
    };
    struct charstab *tab;

#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
    if (varp == &p_lcs)
#endif
    {
	tab = lcstab;
	entries = sizeof(lcstab) / sizeof(struct charstab);
    }
#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
    else
    {
	tab = filltab;
	entries = sizeof(filltab) / sizeof(struct charstab);
    }
#endif

    /* first round: check for valid value, second round: assign values */
    for (round = 0; round <= 1; ++round)
    {
	if (round)
	{
	    /* After checking that the value is valid: set defaults: space for
	     * 'fillchars', NUL for 'listchars' */
	    for (i = 0; i < entries; ++i)
		*(tab[i].cp) = (varp == &p_lcs ? NUL : ' ');
	    if (varp == &p_lcs)
		lcs_tab1 = NUL;
#if defined(FEAT_WINDOWS) || defined(FEAT_FOLDING)
	    else
		fill_diff = '-';
#endif
	}
	p = *varp;
	while (*p)
	{
	    for (i = 0; i < entries; ++i)
	    {
		len = (int)STRLEN(tab[i].name);
		if (STRNCMP(p, tab[i].name, len) == 0
			&& p[len] == ':'
			&& p[len + 1] != NUL)
		{
		    if (tab[i].cp == &lcs_tab2)
			++len;
		    if (p[len + 1] != NUL
			    && (p[len + 2] == ',' || p[len + 2] == NUL))
		    {
			if (round)
			{
			    *(tab[i].cp) = p[len + 1];
			    if (tab[i].cp == &lcs_tab2)
				lcs_tab1 = p[len];
			}
			p += len + 2;
			break;
		    }
		}
	    }

	    if (i == entries)
		return e_invarg;
	    if (*p == ',')
		++p;
	}
    }

    return NULL;	/* no error */
}

#ifdef FEAT_STL_OPT
/*
 * Check validity of options with the 'statusline' format.
 * Return error message or NULL.
 */
    char_u *
check_stl_option(s)
    char_u	*s;
{
    int		itemcnt = 0;
    int		groupdepth = 0;
    static char_u   errbuf[80];

    while (*s && itemcnt < STL_MAX_ITEM)
    {
	/* Check for valid keys after % sequences */
	while (*s && *s != '%')
	    s++;
	if (!*s)
	    break;
	s++;
	if (*s != '%' && *s != ')')
	    ++itemcnt;
	if (*s == '%' || *s == STL_TRUNCMARK || *s == STL_MIDDLEMARK)
	{
	    s++;
	    continue;
	}
	if (*s == ')')
	{
	    s++;
	    if (--groupdepth < 0)
		break;
	    continue;
	}
	if (*s == '-')
	    s++;
	while (isdigit(*s))
	    s++;
	if (*s == STL_HIGHLIGHT)
	    continue;
	if (*s == '.')
	{
	    s++;
	    while (*s && isdigit(*s))
		s++;
	}
	if (*s == '(')
	{
	    groupdepth++;
	    continue;
	}
	if (vim_strchr(STL_ALL, *s) == NULL)
	{
	    return illegal_char(errbuf, *s);
	}
	if (*s == '{')
	{
	    s++;
	    while (*s != '}' && *s)
		s++;
	    if (*s != '}')
		return (char_u *)N_("Unclosed expression sequence");
	}
    }
    if (itemcnt >= STL_MAX_ITEM)
	return (char_u *)N_("too many items");
    if (groupdepth != 0)
	return (char_u *)N_("unbalanced groups");
    return NULL;
}
#endif

#ifdef FEAT_CLIPBOARD
/*
 * Extract the items in the 'clipboard' option and set global values.
 */
    static char_u *
check_clipboard_option()
{
    int		new_unnamed = FALSE;
    int		new_autoselect = FALSE;
    int		new_autoselectml = FALSE;
    regprog_T	*new_exclude_prog = NULL;
    char_u	*errmsg = NULL;
    char_u	*p;

    for (p = p_cb; *p != NUL; )
    {
	if (STRNCMP(p, "unnamed", 7) == 0 && (p[7] == ',' || p[7] == NUL))
	{
	    new_unnamed = TRUE;
	    p += 7;
	}
	else if (STRNCMP(p, "autoselect", 10) == 0
					&& (p[10] == ',' || p[10] == NUL))
	{
	    new_autoselect = TRUE;
	    p += 10;
	}
	else if (STRNCMP(p, "autoselectml", 12) == 0
					&& (p[12] == ',' || p[12] == NUL))
	{
	    new_autoselectml = TRUE;
	    p += 12;
	}
	else if (STRNCMP(p, "exclude:", 8) == 0 && new_exclude_prog == NULL)
	{
	    p += 8;
	    new_exclude_prog = vim_regcomp(p, TRUE);
	    if (new_exclude_prog == NULL)
		errmsg = e_invarg;
	    break;
	}
	else
	{
	    errmsg = e_invarg;
	    break;
	}
	if (*p == ',')
	    ++p;
    }
    if (errmsg == NULL)
    {
	clip_unnamed = new_unnamed;
	clip_autoselect = new_autoselect;
	clip_autoselectml = new_autoselectml;
	vim_free(clip_exclude_prog);
	clip_exclude_prog = new_exclude_prog;
    }
    else
	vim_free(new_exclude_prog);

    return errmsg;
}
#endif

/*
 * Set the value of a boolean option, and take care of side effects.
 * Returns NULL for success, or an error message for an error.
 */
    static char_u *
set_bool_option(opt_idx, varp, value, opt_flags)
    int		opt_idx;		/* index in options[] table */
    char_u	*varp;			/* pointer to the option variable */
    int		value;			/* new value */
    int		opt_flags;		/* OPT_LOCAL and/or OPT_GLOBAL */
{
    int		old_value = *(int *)varp;

#ifdef FEAT_GUI
    need_mouse_correct = TRUE;
#endif

    /* Disallow changing some options from secure mode */
    if ((secure
#ifdef HAVE_SANDBOX
		|| sandbox != 0
#endif
		) && (options[opt_idx].flags & P_SECURE))
	return (char_u *)N_("Not allowed here");

    *(int *)varp = value;	    /* set the new value */
#ifdef FEAT_EVAL
    /* Remember where the option was set. */
    options[opt_idx].scriptID = current_SID;
#endif

    /* May set global value for local option. */
    if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0)
	*(int *)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL) = value;

    /*
     * Handle side effects of changing a bool option.
     */

    /* 'compatible' */
    if ((int *)varp == &p_cp)
    {
	compatible_set();
    }

    /* when 'readonly' is reset globally, also reset readonlymode */
    else if ((int *)varp == &curbuf->b_p_ro)
    {
	if (!curbuf->b_p_ro && (opt_flags & OPT_LOCAL) == 0)
	    readonlymode = FALSE;
#ifdef FEAT_TITLE
	need_maketitle = TRUE;
#endif
    }

#ifdef FEAT_TITLE
    /* when 'modifiable' is changed, redraw the window title */
    else if ((int *)varp == &curbuf->b_p_ma)
	need_maketitle = TRUE;
    /* when 'endofline' is changed, redraw the window title */
    else if ((int *)varp == &curbuf->b_p_eol)
	need_maketitle = TRUE;
#endif

    /* when 'bin' is set also set some other options */
    else if ((int *)varp == &curbuf->b_p_bin)
    {
	set_options_bin(old_value, curbuf->b_p_bin, opt_flags);
#ifdef FEAT_TITLE
	need_maketitle = TRUE;
#endif
    }

#ifdef FEAT_AUTOCMD
    /* when 'buflisted' changes, trigger autocommands */
    else if ((int *)varp == &curbuf->b_p_bl && old_value != curbuf->b_p_bl)
    {
	apply_autocmds(curbuf->b_p_bl ? EVENT_BUFADD : EVENT_BUFDELETE,
						    NULL, NULL, TRUE, curbuf);
    }
#endif

    /* when 'swf' is set, create swapfile, when reset remove swapfile */
    else if ((int *)varp == &curbuf->b_p_swf)
    {
	if (curbuf->b_p_swf && p_uc)
	    ml_open_file(curbuf);		/* create the swap file */
	else
	    mf_close_file(curbuf, TRUE);	/* remove the swap file */
    }

    /* when 'terse' is set change 'shortmess' */
    else if ((int *)varp == &p_terse)
    {
	char_u	*p;

	p = vim_strchr(p_shm, SHM_SEARCH);

	/* insert 's' in p_shm */
	if (p_terse && p == NULL)
	{
	    STRCPY(IObuff, p_shm);
	    STRCAT(IObuff, "s");
	    set_string_option_direct((char_u *)"shm", -1, IObuff, OPT_FREE);
	}
	/* remove 's' from p_shm */
	else if (!p_terse && p != NULL)
	    mch_memmove(p, p + 1, STRLEN(p));
    }

    /* when 'paste' is set or reset also change other options */
    else if ((int *)varp == &p_paste)
    {
	paste_option_changed();
    }

    /* when 'ignorecase' is set or reset and 'hlsearch' is set, redraw */
    else if ((int *)varp == &p_ic && p_hls)
    {
	redraw_all_later(NOT_VALID);
    }

#ifdef FEAT_SEARCH_EXTRA
    /* when 'hlsearch' is set or reset: reset no_hlsearch */
    else if ((int *)varp == &p_hls)
    {
	no_hlsearch = FALSE;
    }
#endif

#ifdef FEAT_SCROLLBIND
    /* when 'scrollbind' is set: snapshot the current position to avoid a jump
     * at the end of normal_cmd() */
    else if ((int *)varp == &curwin->w_p_scb)
    {
	if (curwin->w_p_scb)
	    do_check_scrollbind(FALSE);
    }
#endif

#if defined(FEAT_WINDOWS) && defined(FEAT_QUICKFIX)
    /* There can be only one window with 'previewwindow' set. */
    else if ((int *)varp == &curwin->w_p_pvw)
    {
	if (curwin->w_p_pvw)
	{
	    win_T	*win;

	    for (win = firstwin; win != NULL; win = win->w_next)
		if (win->w_p_pvw && win != curwin)
		{
		    curwin->w_p_pvw = FALSE;
		    return (char_u *)N_("A preview window already exists");
		}
	}
    }
#endif

    /* when 'textmode' is set or reset also change 'fileformat' */
    else if ((int *)varp == &curbuf->b_p_tx)
    {
	set_fileformat(curbuf->b_p_tx ? EOL_DOS : EOL_UNIX, opt_flags);
    }

    /* when 'textauto' is set or reset also change 'fileformats' */
    else if ((int *)varp == &p_ta)
    {
	set_string_option_direct((char_u *)"ffs", -1,
				 p_ta ? (char_u *)DFLT_FFS_VIM : (char_u *)"",
							OPT_FREE | opt_flags);
    }

    /*
     * When 'lisp' option changes include/exclude '-' in
     * keyword characters.
     */
#ifdef FEAT_LISP
    else if (varp == (char_u *)&(curbuf->b_p_lisp))
    {
	(void)buf_init_chartab(curbuf, FALSE);	    /* ignore errors */
    }
#endif

#ifdef FEAT_TITLE
    /* when 'title' changed, may need to change the title; same for 'icon' */
    else if ((int *)varp == &p_title)
    {
	did_set_title(FALSE);
    }

    else if ((int *)varp == &p_icon)
    {
	did_set_title(TRUE);
    }
#endif

    else if ((int *)varp == &curbuf->b_changed)
    {
	if (!value)
	    save_file_ff(curbuf);	/* Buffer is unchanged */
#ifdef FEAT_TITLE
	need_maketitle = TRUE;
#endif
#ifdef FEAT_AUTOCMD
	modified_was_set = value;
#endif
    }

#ifdef BACKSLASH_IN_FILENAME
    else if ((int *)varp == &p_ssl)
    {
	if (p_ssl)
	{
	    psepc = '/';
	    psepcN = '\\';
	    pseps[0] = '/';
	    psepsN[0] = '\\';
	}
	else
	{
	    psepc = '\\';
	    psepcN = '/';
	    pseps[0] = '\\';
	    psepsN[0] = '/';
	}

	/* need to adjust the file name arguments and buffer names. */
	buflist_slash_adjust();
	alist_slash_adjust();
    }
#endif

    /* If 'wrap' is set, set w_leftcol to zero. */
    else if ((int *)varp == &curwin->w_p_wrap)
    {
	if (curwin->w_p_wrap)
	    curwin->w_leftcol = 0;
    }

#ifdef FEAT_WINDOWS
    else if ((int *)varp == &p_ea)
    {
	if (p_ea && !old_value)
	    win_equal(curwin, FALSE, 0);
    }
#endif

    else if ((int *)varp == &p_wiv)
    {
	/*
	 * When 'weirdinvert' changed, set/reset 't_xs'.
	 * Then set 'weirdinvert' according to value of 't_xs'.
	 */
	if (p_wiv && !old_value)
	    T_XS = (char_u *)"y";
	else if (!p_wiv && old_value)
	    T_XS = empty_option;
	p_wiv = (*T_XS != NUL);
    }

#ifdef FEAT_SUN_WORKSHOP
    else if ((int *)varp == &p_beval)
    {
	extern BalloonEval	*balloonEval;

	if (p_beval == TRUE)
	    gui_mch_enable_beval_area(balloonEval);
	else
	    gui_mch_disable_beval_area(balloonEval);
    }
#endif

#ifdef FEAT_DIFF
    /* 'diff' */
    else if ((int *)varp == &curwin->w_p_diff)
    {
	win_T	*wp;

	if (!curwin->w_p_diff)
	{
	    /* When there is no window showing a diff for this buffer, remove
	     * it from the diffs. */
	    for (wp = firstwin; wp != NULL; wp = wp->w_next)
		if (wp->w_buffer == curwin->w_buffer && wp->w_p_diff)
		    break;
	    if (wp == NULL)
		diff_buf_delete(curwin->w_buffer);
	}
	else
	    diff_buf_add(curwin->w_buffer);
#ifdef FEAT_FOLDING
	if (foldmethodIsDiff(curwin))
	    foldUpdateAll(curwin);
#endif
    }
#endif

#ifdef USE_IM_CONTROL
    /* 'imdisable' */
    else if ((int *)varp == &p_imdisable)
    {
	/* Only de-activate it here, it will be enabled when changing mode. */
	if (p_imdisable)
	    im_set_active(FALSE);
    }
#endif

#ifdef FEAT_FKMAP
    else if ((int *)varp == &p_altkeymap)
    {
	if (old_value != p_altkeymap)
	{
	    if (!p_altkeymap)
	    {
		p_hkmap = p_fkmap;
		p_fkmap = 0;
	    }
	    else
	    {
		p_fkmap = p_hkmap;
		p_hkmap = 0;
	    }
	    (void)init_chartab();
	}
    }

    /*
     * In case some second language keymapping options have changed, check
     * and correct the setting in a consistent way.
     */
    /*
     * If hkmap set, reset Farsi keymapping.
     */
    if (p_hkmap && p_altkeymap)
    {
	p_altkeymap = 0;
	p_fkmap = 0;
	(void)init_chartab();
    }

    /*
     * If fkmap set, reset Hebrew keymapping.
     */
    if (p_fkmap && !p_altkeymap)
    {
	p_altkeymap = 1;
	p_hkmap = 0;
	(void)init_chartab();
    }
#endif

    /*
     * End of handling side effects for bool options.
     */

    options[opt_idx].flags |= P_WAS_SET;

    comp_col();			    /* in case 'ruler' or 'showcmd' changed */
    if (curwin->w_curswant != MAXCOL)
	curwin->w_set_curswant = TRUE;  /* in case 'list' changed */
    check_redraw(options[opt_idx].flags);

    return NULL;
}

/*
 * Set the value of a number option, and take care of side effects.
 * Returns NULL for success, or an error message for an error.
 */
    static char_u *
set_num_option(opt_idx, varp, value, errbuf, opt_flags)
    int		opt_idx;		/* index in options[] table */
    char_u	*varp;			/* pointer to the option variable */
    long	value;			/* new value */
    char_u	*errbuf;		/* buffer for error messages */
    int		opt_flags;		/* OPT_LOCAL, OPT_GLOBAL and
					   OPT_MODELINE */
{
    char_u	*errmsg = NULL;
    long	old_value = *(long *)varp;
    long	old_Rows = Rows;	/* remember old Rows */
    long	old_Columns = Columns;	/* remember old Columns */
    long	*pp = (long *)varp;

#ifdef FEAT_GUI
    need_mouse_correct = TRUE;
#endif

    *pp = value;
#ifdef FEAT_EVAL
    /* Remember where the option was set. */
    options[opt_idx].scriptID = current_SID;
#endif

    if (curbuf->b_p_sw <= 0)
    {
	errmsg = e_positive;
	curbuf->b_p_sw = curbuf->b_p_ts;
    }

    /*
     * Number options that need some action when changed
     */
#ifdef FEAT_WINDOWS
    if (pp == &p_wh || pp == &p_hh)
    {
	if (p_wh < 1)
	{
	    errmsg = e_positive;
	    p_wh = 1;
	}
	if (p_wmh > p_wh)
	{
	    errmsg = (char_u *)N_("'winheight' cannot be smaller than 'winminheight'");
	    p_wh = p_wmh;
	}
	if (p_hh < 0)
	{
	    errmsg = e_positive;
	    p_hh = 0;
	}

	/* Change window height NOW */
	if (lastwin != firstwin)
	{
	    if (pp == &p_wh && curwin->w_height < p_wh)
		win_setheight((int)p_wh);
	    if (pp == &p_hh && curbuf->b_help && curwin->w_height < p_hh)
		win_setheight((int)p_hh);
	}
    }

    /* 'winminheight' */
    else if (pp == &p_wmh)
    {
	if (p_wmh < 0)
	{
	    errmsg = e_positive;
	    p_wmh = 0;
	}
	if (p_wmh > p_wh)
	{
	    errmsg = (char_u *)N_("'winheight' cannot be smaller than 'winminheight'");
	    p_wmh = p_wh;
	}
	win_setminheight();
    }

# ifdef FEAT_VERTSPLIT
    if (pp == &p_wiw)
    {
	if (p_wiw < 1)
	{
	    errmsg = e_positive;
	    p_wiw = 1;
	}
	if (p_wmw > p_wiw)
	{
	    errmsg = (char_u *)N_("'winwidth' cannot be smaller than 'winminwidth'");
	    p_wiw = p_wmw;
	}

	/* Change window width NOW */
	if (lastwin != firstwin && curwin->w_width < p_wiw)
	    win_setwidth((int)p_wiw);
    }

    /* 'winminwidth' */
    else if (pp == &p_wmw)
    {
	if (p_wmw < 0)
	{
	    errmsg = e_positive;
	    p_wmw = 0;
	}
	if (p_wmw > p_wiw)
	{
	    errmsg = (char_u *)N_("'winwidth' cannot be smaller than 'winminwidth'");
	    p_wmw = p_wiw;
	}
	win_setminheight();
    }
# endif

#endif

#ifdef FEAT_WINDOWS
    /* (re)set last window status line */
    else if (pp == &p_ls)
    {
	last_status(FALSE);
    }
#endif

#ifdef FEAT_GUI
    else if (pp == &p_linespace)
    {
	if (gui.in_use && gui_mch_adjust_charsize() == OK)
	    gui_set_shellsize(FALSE, FALSE);
    }
#endif

#ifdef FEAT_FOLDING
    /* 'foldlevel' */
    else if (pp == &curwin->w_p_fdl)
    {
	if (curwin->w_p_fdl < 0)
	    curwin->w_p_fdl = 0;
	newFoldLevel();
    }

    /* 'foldminlevel' */
    else if (pp == &curwin->w_p_fml)
    {
	foldUpdateAll(curwin);
    }

    /* 'foldnestmax' */
    else if (pp == &curwin->w_p_fdn)
    {
	if (foldmethodIsSyntax(curwin) || foldmethodIsIndent(curwin))
	    foldUpdateAll(curwin);
    }

    /* 'foldcolumn' */
    else if (pp == &curwin->w_p_fdc)
    {
	if (curwin->w_p_fdc < 0)
	{
	    errmsg = e_positive;
	    curwin->w_p_fdc = 0;
	}
	else if (curwin->w_p_fdc > 12)
	{
	    errmsg = e_invarg;
	    curwin->w_p_fdc = 12;
	}
    }

    /* 'shiftwidth' or 'tabstop' */
    else if (pp == &curbuf->b_p_sw || pp == &curbuf->b_p_ts)
    {
	if (foldmethodIsIndent(curwin))
	    foldUpdateAll(curwin);
    }
#endif /* FEAT_FOLDING */

    else if (pp == &curbuf->b_p_iminsert)
    {
	if (curbuf->b_p_iminsert < 0 || curbuf->b_p_iminsert > B_IMODE_LAST)
	{
	    errmsg = e_invarg;
	    curbuf->b_p_iminsert = B_IMODE_NONE;
	}
	p_iminsert = curbuf->b_p_iminsert;
	if (termcap_active)	/* don't do this in the alternate screen */
	    showmode();
#if defined(FEAT_WINDOWS) && defined(FEAT_KEYMAP)
	/* Show/unshow value of 'keymap' in status lines. */
	status_redraw_curbuf();
#endif
    }

    else if (pp == &curbuf->b_p_imsearch)
    {
	if (curbuf->b_p_imsearch < -1 || curbuf->b_p_imsearch > B_IMODE_LAST)
	{
	    errmsg = e_invarg;
	    curbuf->b_p_imsearch = B_IMODE_NONE;
	}
	p_imsearch = curbuf->b_p_imsearch;
    }

#ifdef FEAT_TITLE
    /* if 'titlelen' has changed, redraw the title */
    else if (pp == &p_titlelen)
    {
	if (p_titlelen < 0)
	{
	    errmsg = e_positive;
	    p_titlelen = 85;
	}
	if (starting != NO_SCREEN && old_value != p_titlelen)
	    need_maketitle = TRUE;
    }
#endif

    /* if p_ch changed value, change the command line height */
    else if (pp == &p_ch)
    {
	if (p_ch < 1)
	{
	    errmsg = e_positive;
	    p_ch = 1;
	}
	if (p_ch != old_value)
	    command_height(old_value);
    }

    /* when 'updatecount' changes from zero to non-zero, open swap files */
    else if (pp == &p_uc)
    {
	if (p_uc < 0)
	{
	    errmsg = e_positive;
	    p_uc = 100;
	}
	if (p_uc && !old_value)
	    ml_open_files();
    }

    /*
     * Check the bounds for numeric options here
     */
    if (Rows < min_rows() && full_screen)
    {
	if (errbuf != NULL)
	{
	    sprintf((char *)errbuf, _("Need at least %d lines"), min_rows());
	    errmsg = errbuf;
	}
	Rows = min_rows();
    }
    if (Columns < MIN_COLUMNS && full_screen)
    {
	if (errbuf != NULL)
	{
	    sprintf((char *)errbuf, _("Need at least %d columns"), MIN_COLUMNS);
	    errmsg = errbuf;
	}
	Columns = MIN_COLUMNS;
    }

#ifdef DJGPP
    /* avoid a crash by checking for a too large value of 'columns' */
    if (old_Columns != Columns && full_screen && term_console)
	mch_check_columns();
#endif

    /*
     * If the screen (shell) height has been changed, assume it is the
     * physical screenheight.
     */
    if (old_Rows != Rows || old_Columns != Columns)
    {
	if (full_screen
#ifdef FEAT_GUI
		&& !gui.starting
#endif
	    )
	    set_shellsize((int)Columns, (int)Rows, TRUE);
	else
	{
	    /* Postpone the resizing; check the size and cmdline position for
	     * messages. */
	    check_shellsize();
	    if (cmdline_row > Rows - p_ch)
		cmdline_row = Rows - p_ch;
	}
    }

    if (curbuf->b_p_sts < 0)
    {
	errmsg = e_positive;
	curbuf->b_p_sts = 0;
    }
    if (curbuf->b_p_ts <= 0)
    {
	errmsg = e_positive;
	curbuf->b_p_ts = 8;
    }
    if (curbuf->b_p_tw < 0)
    {
	errmsg = e_positive;
	curbuf->b_p_tw = 0;
    }
    if (p_tm < 0)
    {
	errmsg = e_positive;
	p_tm = 0;
    }
    if ((curwin->w_p_scr <= 0
		|| (curwin->w_p_scr > curwin->w_height
		    && curwin->w_height > 0))
	    && full_screen)
    {
	if (pp == &(curwin->w_p_scr))
	{
	    if (curwin->w_p_scr != 0)
		errmsg = e_scroll;
	    win_comp_scroll(curwin);
	}
	/* If 'scroll' became invalid because of a side effect silently adjust
	 * it. */
	else if (curwin->w_p_scr <= 0)
	    curwin->w_p_scr = 1;
	else /* curwin->w_p_scr > curwin->w_height */
	    curwin->w_p_scr = curwin->w_height;
    }
    if (p_report < 0)
    {
	errmsg = e_positive;
	p_report = 1;
    }
    if ((p_sj < 0 || p_sj >= Rows) && full_screen)
    {
	if (Rows != old_Rows)	/* Rows changed, just adjust p_sj */
	    p_sj = Rows / 2;
	else
	{
	    errmsg = e_scroll;
	    p_sj = 1;
	}
    }
    if (p_so < 0 && full_screen)
    {
	errmsg = e_scroll;
	p_so = 0;
    }
    if (p_siso < 0 && full_screen)
    {
	errmsg = e_positive;
	p_siso = 0;
    }
#ifdef FEAT_CMDWIN
    if (p_cwh < 1)
    {
	errmsg = e_positive;
	p_cwh = 1;
    }
#endif
    if (p_ut < 0)
    {
	errmsg = e_positive;
	p_ut = 2000;
    }
    if (p_ss < 0)
    {
	errmsg = e_positive;
	p_ss = 0;
    }

    /* May set global value for local option. */
    if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0)
	*(long *)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL) = *pp;

    options[opt_idx].flags |= P_WAS_SET;

    comp_col();			    /* in case 'columns' or 'ls' changed */
    if (curwin->w_curswant != MAXCOL)
	curwin->w_set_curswant = TRUE;  /* in case 'tabstop' changed */
    check_redraw(options[opt_idx].flags);

    return errmsg;
}

/*
 * Called after an option changed: check if something needs to be redrawn.
 */
    static void
check_redraw(flags)
    long_u	flags;
{
    /* Careful: P_RCLR and P_RALL are a combination of other P_ flags */
    int		clear = (flags & P_RCLR) == P_RCLR;
    int		all = ((flags & P_RALL) == P_RALL || clear);

#ifdef FEAT_WINDOWS
    if ((flags & P_RSTAT) || all)	/* mark all status lines dirty */
	status_redraw_all();
#endif

    if ((flags & P_RBUF) || (flags & P_RWIN) || all)
	changed_window_setting();
    if (flags & P_RBUF)
	redraw_curbuf_later(NOT_VALID);
    if (clear)
	redraw_all_later(CLEAR);
    else if (all)
	redraw_all_later(NOT_VALID);
}

/*
 * Find index for option 'arg'.
 * Return -1 if not found.
 */
    static int
findoption(arg)
    char_u *arg;
{
    int		    opt_idx;
    char	    *s, *p;
    static short    quick_tab[27] = {0, 0};	/* quick access table */
    int		    is_term_opt;

    /*
     * For first call: Initialize the quick-access table.
     * It contains the index for the first option that starts with a certain
     * letter.  There are 26 letters, plus the first "t_" option.
     */
    if (quick_tab[1] == 0)
    {
	p = options[0].fullname;
	for (opt_idx = 1; (s = options[opt_idx].fullname) != NULL; opt_idx++)
	{
	    if (s[0] != p[0])
	    {
		if (s[0] == 't' && s[1] == '_')
		    quick_tab[26] = opt_idx;
		else
		    quick_tab[CharOrdLow(s[0])] = opt_idx;
	    }
	    p = s;
	}
    }

    /*
     * Check for name starting with an illegal character.
     */
#ifdef EBCDIC
    if (!islower(arg[0]))
#else
    if (arg[0] < 'a' || arg[0] > 'z')
#endif
	return -1;

    is_term_opt = (arg[0] == 't' && arg[1] == '_');
    if (is_term_opt)
	opt_idx = quick_tab[26];
    else
	opt_idx = quick_tab[CharOrdLow(arg[0])];
    for ( ; (s = options[opt_idx].fullname) != NULL; opt_idx++)
    {
	if (STRCMP(arg, s) == 0)		    /* match full name */
	    break;
    }
    if (s == NULL && !is_term_opt)
    {
	opt_idx = quick_tab[CharOrdLow(arg[0])];
	for ( ; options[opt_idx].fullname != NULL; opt_idx++)
	{
	    s = options[opt_idx].shortname;
	    if (s != NULL && STRCMP(arg, s) == 0)   /* match short name */
		break;
	    s = NULL;
	}
    }
    if (s == NULL)
	opt_idx = -1;
    return opt_idx;
}

#if defined(FEAT_EVAL) || defined(FEAT_TCL)
/*
 * Get the value for an option.
 *
 * Returns:
 * Number or Toggle option: 1, *numval gets value.
 *	     String option: 0, *stringval gets allocated string.
 * Hidden Number or Toggle option: -1.
 *	     hidden String option: -2.
 *		   unknown option: -3.
 */
    int
get_option_value(name, numval, stringval, opt_flags)
    char_u	*name;
    long	*numval;
    char_u	**stringval;	    /* NULL when only checking existance */
    int		opt_flags;
{
    int		opt_idx;
    char_u	*varp;

    opt_idx = findoption(name);
    if (opt_idx < 0)		    /* unknown option */
	return -3;

    varp = get_varp_scope(&(options[opt_idx]), opt_flags);

    if (options[opt_idx].flags & P_STRING)
    {
	if (varp == NULL)		    /* hidden option */
	    return -2;
	if (stringval != NULL)
	{
#ifdef FEAT_CRYPT
	    /* never return the value of the crypt key */
	    if ((char_u **)varp == &curbuf->b_p_key)
		*stringval = vim_strsave((char_u *)"*****");
	    else
#endif
		*stringval = vim_strsave(*(char_u **)(varp));
	}
	return 0;
    }

    if (varp == NULL)		    /* hidden option */
	return -1;
    if (options[opt_idx].flags & P_NUM)
	*numval = *(long *)varp;
    else
    {
	/* Special case: 'modified' is b_changed, but we also want to consider
	 * it set when 'ff' or 'fenc' changed. */
	if ((int *)varp == &curbuf->b_changed)
	    *numval = curbufIsChanged();
	else
	    *numval = *(int *)varp;
    }
    return 1;
}
#endif

/*
 * Set the value of option "name".
 * Use "string" for string options, use "number" for other options.
 */
    void
set_option_value(name, number, string, opt_flags)
    char_u	*name;
    long	number;
    char_u	*string;
    int		opt_flags;	/* OPT_LOCAL or 0 (both) */
{
    int		opt_idx;
    char_u	*varp;

    opt_idx = findoption(name);
    if (opt_idx == -1)
	EMSG2(_("E355: Unknown option: %s"), name);
    else if (options[opt_idx].flags & P_STRING)
	set_string_option(opt_idx, string, opt_flags);
    else
    {
	varp = get_varp(&options[opt_idx]);
	if (varp != NULL)	/* hidden option is not changed */
	{
	    if (options[opt_idx].flags & P_NUM)
		(void)set_num_option(opt_idx, varp, number, NULL, opt_flags);
	    else
		(void)set_bool_option(opt_idx, varp, (int)number, opt_flags);
	}
    }
}

/*
 * Get the terminal code for a terminal option.
 * Returns NULL when not found.
 */
    char_u *
get_term_code(tname)
    char_u	*tname;
{
    int	    opt_idx;
    char_u  *varp;

    if (tname[0] != 't' || tname[1] != '_' ||
	    tname[2] == NUL || tname[3] == NUL)
	return NULL;
    if ((opt_idx = findoption(tname)) >= 0)
    {
	varp = get_varp(&(options[opt_idx]));
	if (varp != NULL)
	    varp = *(char_u **)(varp);
	return varp;
    }
    return find_termcode(tname + 2);
}

    char_u *
get_highlight_default()
{
    int i;

    i = findoption((char_u *)"hl");
    if (i >= 0)
	return options[i].def_val[VI_DEFAULT];
    return (char_u *)NULL;
}

/*
 * Translate a string like "t_xx", "<t_xx>" or "<S-Tab>" to a key number.
 */
    static int
find_key_option(arg)
    char_u *arg;
{
    int		key;
    int		modifiers;

    /*
     * Don't use get_special_key_code() for t_xx, we don't want it to call
     * add_termcap_entry().
     */
    if (arg[0] == 't' && arg[1] == '_' && arg[2] && arg[3])
	key = TERMCAP2KEY(arg[2], arg[3]);
    else
    {
	--arg;			    /* put arg at the '<' */
	key = find_special_key(&arg, &modifiers, TRUE);
	if (modifiers)		    /* can't handle modifiers here */
	    key = 0;
    }
    return key;
}

/*
 * if 'all' == 0: show changed options
 * if 'all' == 1: show all normal options
 * if 'all' == 2: show all terminal options
 */
    static void
showoptions(all, opt_flags)
    int		all;
    int		opt_flags;	/* OPT_LOCAL and/or OPT_GLOBAL */
{
    struct vimoption	*p;
    int			col;
    int			isterm;
    char_u		*varp;
    struct vimoption	**items;
    int			item_count;
    int			run;
    int			row, rows;
    int			cols;
    int			i;
    int			len;

#define INC 20
#define GAP 3

    items = (struct vimoption **)alloc((unsigned)(sizeof(struct vimoption *) *
								PARAM_COUNT));
    if (items == NULL)
	return;

    /* Highlight title */
    if (all == 2)
	MSG_PUTS_TITLE(_("\n--- Terminal codes ---"));
    else if (opt_flags & OPT_GLOBAL)
	MSG_PUTS_TITLE(_("\n--- Global option values ---"));
    else if (opt_flags & OPT_LOCAL)
	MSG_PUTS_TITLE(_("\n--- Local option values ---"));
    else
	MSG_PUTS_TITLE(_("\n--- Options ---"));

    /*
     * do the loop two times:
     * 1. display the short items
     * 2. display the long items (only strings and numbers)
     */
    for (run = 1; run <= 2 && !got_int; ++run)
    {
	/*
	 * collect the items in items[]
	 */
	item_count = 0;
	for (p = &options[0]; p->fullname != NULL; p++)
	{
	    varp = NULL;
	    isterm = istermoption(p);
	    if (opt_flags != 0)
	    {
		if (p->indir != PV_NONE && !isterm)
		    varp = get_varp_scope(p, opt_flags);
	    }
	    else
		varp = get_varp(p);
	    if (varp != NULL
		    && ((all == 2 && isterm)
			|| (all == 1 && !isterm)
			|| (all == 0 && !optval_default(p, varp))))
	    {
		if (p->flags & P_BOOL)
		    len = 1;		/* a toggle option fits always */
		else
		{
		    option_value2string(p, opt_flags);
		    len = (int)STRLEN(p->fullname) + vim_strsize(NameBuff) + 1;
		}
		if ((len <= INC - GAP && run == 1) ||
						(len > INC - GAP && run == 2))
		    items[item_count++] = p;
	    }
	}

	/*
	 * display the items
	 */
	if (run == 1)
	{
	    cols = (Columns + GAP - 3) / INC;
	    if (cols == 0)
		cols = 1;
	    rows = (item_count + cols - 1) / cols;
	}
	else	/* run == 2 */
	    rows = item_count;
	for (row = 0; row < rows && !got_int; ++row)
	{
	    msg_putchar('\n');			/* go to next line */
	    if (got_int)			/* 'q' typed in more */
		break;
	    col = 0;
	    for (i = row; i < item_count; i += rows)
	    {
		msg_col = col;			/* make columns */
		showoneopt(items[i], opt_flags);
		col += INC;
	    }
	    out_flush();
	    ui_breakcheck();
	}
    }
    vim_free(items);
}

/*
 * Return TRUE if option "p" has its default value.
 */
    static int
optval_default(p, varp)
    struct vimoption	*p;
    char_u		*varp;
{
    int		dvi;

    if (varp == NULL)
	return TRUE;	    /* hidden option is always at default */
    dvi = ((p->flags & P_VI_DEF) || p_cp) ? VI_DEFAULT : VIM_DEFAULT;
    if (p->flags & P_NUM)
	return (*(long *)varp == (long)p->def_val[dvi]);
    if (p->flags & P_BOOL)
			/* the cast to long is required for Manx C */
	return (*(int *)varp == (int)(long)p->def_val[dvi]);
    /* P_STRING */
    return (STRCMP(*(char_u **)varp, p->def_val[dvi]) == 0);
}

/*
 * showoneopt: show the value of one option
 * must not be called with a hidden option!
 */
    static void
showoneopt(p, opt_flags)
    struct vimoption	*p;
    int			opt_flags;	/* OPT_LOCAL or OPT_GLOBAL */
{
    char_u		*varp;

    varp = get_varp_scope(p, opt_flags);

    /* for 'modified' we also need to check if 'ff' or 'fenc' changed. */
    if ((p->flags & P_BOOL) && ((int *)varp == &curbuf->b_changed
					? !curbufIsChanged() : !*(int *)varp))
	MSG_PUTS("no");
    else if ((p->flags & P_BOOL) && *(int *)varp < 0)
	MSG_PUTS("--");
    else
	MSG_PUTS("  ");
    MSG_PUTS(p->fullname);
    if (!(p->flags & P_BOOL))
    {
	msg_putchar('=');
	/* put value string in NameBuff */
	option_value2string(p, opt_flags);
	msg_outtrans(NameBuff);
    }
}

/*
 * Write modified options as ":set" commands to a file.
 *
 * There are three values for "opt_flags":
 * OPT_GLOBAL:		   Write global option values and fresh values of
 *			   buffer-local options (used for start of a session
 *			   file).
 * OPT_GLOBAL + OPT_LOCAL: Idem, add fresh values of window-local options for
 *			   curwin (used for a vimrc file).
 * OPT_LOCAL:		   Write buffer-local option values for curbuf, fresh
 *			   and local values for window-local options of
 *			   curwin.  Local values are also written when at the
 *			   default value, because a modeline or autocommand
 *			   may have set them when doing ":edit file" and the
 *			   user has set them back at the default or fresh
 *			   value.
 *			   When "local_only" is TRUE, don't write fresh
 *			   values, only local values (for ":mkview").
 * (fresh value = value used for a new buffer or window for a local option).
 *
 * Return FAIL on error, OK otherwise.
 */
    int
makeset(fd, opt_flags, local_only)
    FILE	*fd;
    int		opt_flags;
    int		local_only;
{
    struct vimoption	*p;
    char_u		*varp;			/* currently used value */
    char_u		*varp_fresh;		/* local value */
    char_u		*varp_local = NULL;	/* fresh value */
    char		*cmd;
    int			round;

    /*
     * The options that don't have a default (terminal name, columns, lines)
     * are never written.  Terminal options are also not written.
     */
    for (p = &options[0]; !istermoption(p); p++)
	if (!(p->flags & P_NO_MKRC) && !istermoption(p))
	{
	    /* skip global option when only doing locals */
	    if (p->indir == PV_NONE && !(opt_flags & OPT_GLOBAL))
		continue;

	    /* Global values are only written when not at the default value. */
	    varp = get_varp_scope(p, opt_flags);
	    if ((opt_flags & OPT_GLOBAL) && optval_default(p, varp))
		continue;

	    round = 2;
	    if (p->indir != PV_NONE)
	    {
		if (p->var == VAR_WIN)
		{
		    /* skip window-local option when only doing globals */
		    if (!(opt_flags & OPT_LOCAL))
			continue;
		    /* When fresh value of window-local option is not at the
		     * default, need to write it too. */
		    if (!(opt_flags & OPT_GLOBAL) && !local_only)
		    {
			varp_fresh = get_varp_scope(p, OPT_GLOBAL);
			if (!optval_default(p, varp_fresh))
			{
			    round = 1;
			    varp_local = varp;
			    varp = varp_fresh;
			}
		    }
		}
	    }

	    /* Round 1: fresh value for window-local options.
	     * Round 2: other values */
	    for ( ; round <= 2; varp = varp_local, ++round)
	    {
		if (round == 1 || (opt_flags & OPT_GLOBAL))
		    cmd = "set";
		else
		    cmd = "setlocal";

		if (p->flags & P_BOOL)
		{
		    if (put_setbool(fd, cmd, p->fullname, *(int *)varp) == FAIL)
			return FAIL;
		}
		else if (p->flags & P_NUM)
		{
		    if (put_setnum(fd, cmd, p->fullname, (long *)varp) == FAIL)
			return FAIL;
		}
		else    /* P_STRING */
		{
		    /* Don't set 'syntax' and 'filetype' again if the value is
		     * already right, avoids reloading the syntax file. */
		    if (p->indir == PV_SYN || p->indir == PV_FT)
		    {
			if (fprintf(fd, "if &%s != '%s'", p->fullname,
						       *(char_u **)(varp)) < 0
				|| put_eol(fd) < 0)
			    return FAIL;
		    }
		    if (put_setstring(fd, cmd, p->fullname, (char_u **)varp,
					  (p->flags & P_EXPAND) != 0) == FAIL)
			return FAIL;
		    if (p->indir == PV_SYN || p->indir == PV_FT)
		    {
			if (put_line(fd, "endif") == FAIL)
			    return FAIL;
		    }
		}
	    }
	}
    return OK;
}

#if defined(FEAT_FOLDING) || defined(PROTO)
/*
 * Generate set commands for the local fold options only.  Used when
 * 'sessionoptions' or 'viewoptions' contains "folds" but not "options".
 */
    int
makefoldset(fd)
    FILE	*fd;
{
    if (put_setstring(fd, "setlocal", "fdm", &curwin->w_p_fdm, FALSE) == FAIL
# ifdef FEAT_EVAL
	    || put_setstring(fd, "setlocal", "fde", &curwin->w_p_fde, FALSE)
								       == FAIL
# endif
	    || put_setstring(fd, "setlocal", "fmr", &curwin->w_p_fmr, FALSE)
								       == FAIL
	    || put_setstring(fd, "setlocal", "fdi", &curwin->w_p_fdi, FALSE)
								       == FAIL
	    || put_setnum(fd, "setlocal", "fdl", &curwin->w_p_fdl) == FAIL
	    || put_setnum(fd, "setlocal", "fml", &curwin->w_p_fml) == FAIL
	    || put_setnum(fd, "setlocal", "fdn", &curwin->w_p_fdn) == FAIL
	    || put_setbool(fd, "setlocal", "fen", curwin->w_p_fen) == FAIL
	    )
	return FAIL;

    return OK;
}
#endif

    static int
put_setstring(fd, cmd, name, valuep, expand)
    FILE	*fd;
    char	*cmd;
    char	*name;
    char_u	**valuep;
    int		expand;
{
    char_u	*s;
    char_u	buf[MAXPATHL];

    if (fprintf(fd, "%s %s=", cmd, name) < 0)
	return FAIL;
    if (*valuep != NULL)
    {
	/* Output 'pastetoggle' as key names.  For other
	 * options some characters have to be escaped with
	 * CTRL-V or backslash */
	if (valuep == &p_pt)
	{
	    s = *valuep;
	    while (*s != NUL)
		if (fputs((char *)str2special(&s, FALSE), fd) < 0)
		    return FAIL;
	}
	else if (expand)
	{
	    home_replace(NULL, *valuep, buf, MAXPATHL, FALSE);
	    if (put_escstr(fd, buf, 2) == FAIL)
		return FAIL;
	}
	else if (put_escstr(fd, *valuep, 2) == FAIL)
	    return FAIL;
    }
    if (put_eol(fd) < 0)
	return FAIL;
    return OK;
}

    static int
put_setnum(fd, cmd, name, valuep)
    FILE	*fd;
    char	*cmd;
    char	*name;
    long	*valuep;
{
    long	wc;

    if (fprintf(fd, "%s %s=", cmd, name) < 0)
	return FAIL;
    if (wc_use_keyname((char_u *)valuep, &wc))
    {
	/* print 'wildchar' and 'wildcharm' as a key name */
	if (fputs((char *)get_special_key_name((int)wc, 0), fd) < 0)
	    return FAIL;
    }
    else if (fprintf(fd, "%ld", *valuep) < 0)
	return FAIL;
    if (put_eol(fd) < 0)
	return FAIL;
    return OK;
}

    static int
put_setbool(fd, cmd, name, value)
    FILE	*fd;
    char	*cmd;
    char	*name;
    int		value;
{
    if (fprintf(fd, "%s %s%s", cmd, value ? "" : "no", name) < 0
	    || put_eol(fd) < 0)
	return FAIL;
    return OK;
}

/*
 * Clear all the terminal options.
 * If the option has been allocated, free the memory.
 * Terminal options are never hidden or indirect.
 */
    void
clear_termoptions()
{
    struct vimoption   *p;

    /*
     * Reset a few things before clearing the old options. This may cause
     * outputting a few things that the terminal doesn't understand, but the
     * screen will be cleared later, so this is OK.
     */
#ifdef FEAT_MOUSE_TTY
    mch_setmouse(FALSE);	    /* switch mouse off */
#endif
#ifdef FEAT_TITLE
    mch_restore_title(3);	    /* restore window titles */
#endif
#if defined(FEAT_XCLIPBOARD) && defined(FEAT_GUI)
    /* When starting the GUI close the display opened for the clipboard.
     * After restoring the title, because that will need the display. */
    if (gui.starting)
	clear_xterm_clip();
#endif
#ifdef WIN3264
    /*
     * Check if this is allowed now.
     */
    if (can_end_termcap_mode(FALSE) == TRUE)
#endif
	stoptermcap();			/* stop termcap mode */

    for (p = &options[0]; p->fullname != NULL; p++)
	if (istermoption(p))
	{
	    if (p->flags & P_ALLOCED)
		free_string_option(*(char_u **)(p->var));
	    if (p->flags & P_DEF_ALLOCED)
		free_string_option(p->def_val[VI_DEFAULT]);
	    *(char_u **)(p->var) = empty_option;
	    p->def_val[VI_DEFAULT] = empty_option;
	    p->flags &= ~(P_ALLOCED|P_DEF_ALLOCED);
	}
    clear_termcodes();
}

/*
 * Set the terminal option defaults to the current value.
 * Used after setting the terminal name.
 */
    void
set_term_defaults()
{
    struct vimoption   *p;

    for (p = &options[0]; p->fullname != NULL; p++)
    {
	if (istermoption(p) && p->def_val[VI_DEFAULT] != *(char_u **)(p->var))
	{
	    if (p->flags & P_DEF_ALLOCED)
	    {
		free_string_option(p->def_val[VI_DEFAULT]);
		p->flags &= ~P_DEF_ALLOCED;
	    }
	    p->def_val[VI_DEFAULT] = *(char_u **)(p->var);
	    if (p->flags & P_ALLOCED)
	    {
		p->flags |= P_DEF_ALLOCED;
		p->flags &= ~P_ALLOCED;	 /* don't free the value now */
	    }
	}
    }
}

/*
 * return TRUE if 'p' starts with 't_'
 */
    static int
istermoption(p)
    struct vimoption *p;
{
    return (p->fullname[0] == 't' && p->fullname[1] == '_');
}

/*
 * Compute columns for ruler and shown command. 'sc_col' is also used to
 * decide what the maximum length of a message on the status line can be.
 * If there is a status line for the last window, 'sc_col' is independent
 * of 'ru_col'.
 */

#define COL_RULER 17	    /* columns needed by standard ruler */

    void
comp_col()
{
#if defined(FEAT_CMDL_INFO) && defined(FEAT_WINDOWS)
    int last_has_status = (p_ls == 2 || (p_ls == 1 && firstwin != lastwin));

    sc_col = 0;
    ru_col = 0;
    if (p_ru)
    {
#ifdef FEAT_STL_OPT
	ru_col = (ru_wid ? ru_wid : COL_RULER) + 1;
#else
	ru_col = COL_RULER + 1;
#endif
	/* no last status line, adjust sc_col */
	if (!last_has_status)
	    sc_col = ru_col;
    }
    if (p_sc)
    {
	sc_col += SHOWCMD_COLS;
	if (!p_ru || last_has_status)	    /* no need for separating space */
	    ++sc_col;
    }
    sc_col = Columns - sc_col;
    ru_col = Columns - ru_col;
    if (sc_col <= 0)		/* screen too narrow, will become a mess */
	sc_col = 1;
    if (ru_col <= 0)
	ru_col = 1;
#else
    sc_col = Columns;
    ru_col = Columns;
#endif
}

/*
 * Get pointer to option variable, depending on local or global scope.
 */
    static char_u *
get_varp_scope(p, opt_flags)
    struct vimoption	*p;
    int			opt_flags;
{
    if ((opt_flags & OPT_GLOBAL) && p->indir != PV_NONE)
    {
	if (p->var == VAR_WIN)
	    return (char_u *)GLOBAL_WO(get_varp(p));
	return p->var;
    }
    if ((opt_flags & OPT_LOCAL) && (int)p->indir >= PV_BOTH)
    {
	switch ((int)p->indir)
	{
#ifdef FEAT_QUICKFIX
	    case OPT_BOTH(PV_GP):   return (char_u *)&(curbuf->b_p_gp);
	    case OPT_BOTH(PV_MP):   return (char_u *)&(curbuf->b_p_mp);
	    case OPT_BOTH(PV_EFM):  return (char_u *)&(curbuf->b_p_efm);
#endif
	    case OPT_BOTH(PV_EP):   return (char_u *)&(curbuf->b_p_ep);
	    case OPT_BOTH(PV_PATH): return (char_u *)&(curbuf->b_p_path);
	    case OPT_BOTH(PV_AR):   return (char_u *)&(curbuf->b_p_ar);
	    case OPT_BOTH(PV_TAGS): return (char_u *)&(curbuf->b_p_tags);
#ifdef FEAT_FIND_ID
	    case OPT_BOTH(PV_DEF):  return (char_u *)&(curbuf->b_p_def);
	    case OPT_BOTH(PV_INC):  return (char_u *)&(curbuf->b_p_inc);
#endif
#ifdef FEAT_INS_EXPAND
	    case OPT_BOTH(PV_DICT): return (char_u *)&(curbuf->b_p_dict);
	    case OPT_BOTH(PV_TSR):  return (char_u *)&(curbuf->b_p_tsr);
#endif
	}
	return NULL; /* "cannot happen" */
    }
    return get_varp(p);
}

/*
 * Get pointer to option variable.
 */
    static char_u *
get_varp(p)
    struct vimoption	*p;
{
    /* hidden option, always return NULL */
    if (p->var == NULL)
	return NULL;

    switch ((int)p->indir)
    {
	case PV_NONE:	return p->var;

	/* global option with local value: use local value if it's been set */
	case OPT_BOTH(PV_EP):	return *curbuf->b_p_ep != NUL
				    ? (char_u *)&curbuf->b_p_ep : p->var;
	case OPT_BOTH(PV_PATH):	return *curbuf->b_p_path != NUL
				    ? (char_u *)&(curbuf->b_p_path) : p->var;
	case OPT_BOTH(PV_AR):	return curbuf->b_p_ar >= 0
				    ? (char_u *)&(curbuf->b_p_ar) : p->var;
	case OPT_BOTH(PV_TAGS):	return *curbuf->b_p_tags != NUL
				    ? (char_u *)&(curbuf->b_p_tags) : p->var;
#ifdef FEAT_FIND_ID
	case OPT_BOTH(PV_DEF):	return *curbuf->b_p_def != NUL
				    ? (char_u *)&(curbuf->b_p_def) : p->var;
	case OPT_BOTH(PV_INC):	return *curbuf->b_p_inc != NUL
				    ? (char_u *)&(curbuf->b_p_inc) : p->var;
#endif
#ifdef FEAT_INS_EXPAND
	case OPT_BOTH(PV_DICT):	return *curbuf->b_p_dict != NUL
				    ? (char_u *)&(curbuf->b_p_dict) : p->var;
	case OPT_BOTH(PV_TSR):	return *curbuf->b_p_tsr != NUL
				    ? (char_u *)&(curbuf->b_p_tsr) : p->var;
#endif
#ifdef FEAT_QUICKFIX
	case OPT_BOTH(PV_GP):	return *curbuf->b_p_gp != NUL
				    ? (char_u *)&(curbuf->b_p_gp) : p->var;
	case OPT_BOTH(PV_MP):	return *curbuf->b_p_mp != NUL
				    ? (char_u *)&(curbuf->b_p_mp) : p->var;
	case OPT_BOTH(PV_EFM):	return *curbuf->b_p_efm != NUL
				    ? (char_u *)&(curbuf->b_p_efm) : p->var;
#endif

	case PV_LIST:	return (char_u *)&(curwin->w_p_list);
#ifdef FEAT_DIFF
	case PV_DIFF:	return (char_u *)&(curwin->w_p_diff);
#endif
#ifdef FEAT_FOLDING
	case PV_FDC:	return (char_u *)&(curwin->w_p_fdc);
	case PV_FEN:	return (char_u *)&(curwin->w_p_fen);
	case PV_FDI:	return (char_u *)&(curwin->w_p_fdi);
	case PV_FDL:	return (char_u *)&(curwin->w_p_fdl);
	case PV_FDM:	return (char_u *)&(curwin->w_p_fdm);
	case PV_FML:	return (char_u *)&(curwin->w_p_fml);
	case PV_FDN:	return (char_u *)&(curwin->w_p_fdn);
# ifdef FEAT_EVAL
	case PV_FDE:	return (char_u *)&(curwin->w_p_fde);
	case PV_FDT:	return (char_u *)&(curwin->w_p_fdt);
# endif
	case PV_FMR:	return (char_u *)&(curwin->w_p_fmr);
#endif
	case PV_NU:	return (char_u *)&(curwin->w_p_nu);
#if defined(FEAT_WINDOWS)
	case PV_WFH:	return (char_u *)&(curwin->w_p_wfh);
#endif
#if defined(FEAT_WINDOWS) && defined(FEAT_QUICKFIX)
	case PV_PVW:	return (char_u *)&(curwin->w_p_pvw);
#endif
#ifdef FEAT_RIGHTLEFT
	case PV_RL:	return (char_u *)&(curwin->w_p_rl);
#endif
	case PV_SCROLL:	return (char_u *)&(curwin->w_p_scr);
	case PV_WRAP:	return (char_u *)&(curwin->w_p_wrap);
#ifdef FEAT_LINEBREAK
	case PV_LBR:	return (char_u *)&(curwin->w_p_lbr);
#endif
#ifdef FEAT_SCROLLBIND
	case PV_SCBIND: return (char_u *)&(curwin->w_p_scb);
#endif

	case PV_AI:	return (char_u *)&(curbuf->b_p_ai);
	case PV_BIN:	return (char_u *)&(curbuf->b_p_bin);
#ifdef FEAT_MBYTE
	case PV_BOMB:	return (char_u *)&(curbuf->b_p_bomb);
#endif
#if defined(FEAT_QUICKFIX)
	case PV_BH:	return (char_u *)&(curbuf->b_p_bh);
	case PV_BT:	return (char_u *)&(curbuf->b_p_bt);
#endif
	case PV_BL:	return (char_u *)&(curbuf->b_p_bl);
#ifdef FEAT_CINDENT
	case PV_CIN:	return (char_u *)&(curbuf->b_p_cin);
	case PV_CINK:	return (char_u *)&(curbuf->b_p_cink);
	case PV_CINO:	return (char_u *)&(curbuf->b_p_cino);
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
	case PV_CINW:	return (char_u *)&(curbuf->b_p_cinw);
#endif
#ifdef FEAT_COMMENTS
	case PV_COM:	return (char_u *)&(curbuf->b_p_com);
#endif
#ifdef FEAT_FOLDING
	case PV_CMS:	return (char_u *)&(curbuf->b_p_cms);
#endif
#ifdef FEAT_INS_EXPAND
	case PV_CPT:	return (char_u *)&(curbuf->b_p_cpt);
#endif
	case PV_EOL:	return (char_u *)&(curbuf->b_p_eol);
	case PV_ET:	return (char_u *)&(curbuf->b_p_et);
#ifdef FEAT_MBYTE
	case PV_FENC:	return (char_u *)&(curbuf->b_p_fenc);
#endif
	case PV_FF:	return (char_u *)&(curbuf->b_p_ff);
#ifdef FEAT_AUTOCMD
	case PV_FT:	return (char_u *)&(curbuf->b_p_ft);
#endif
	case PV_FO:	return (char_u *)&(curbuf->b_p_fo);
	case PV_IMI:	return (char_u *)&(curbuf->b_p_iminsert);
	case PV_IMS:	return (char_u *)&(curbuf->b_p_imsearch);
	case PV_INF:	return (char_u *)&(curbuf->b_p_inf);
	case PV_ISK:	return (char_u *)&(curbuf->b_p_isk);
#ifdef FEAT_FIND_ID
# ifdef FEAT_EVAL
	case PV_INEX:	return (char_u *)&(curbuf->b_p_inex);
# endif
#endif
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
	case PV_INDE:	return (char_u *)&(curbuf->b_p_inde);
	case PV_INDK:	return (char_u *)&(curbuf->b_p_indk);
#endif
#ifdef FEAT_CRYPT
	case PV_KEY:	return (char_u *)&(curbuf->b_p_key);
#endif
#ifdef FEAT_LISP
	case PV_LISP:	return (char_u *)&(curbuf->b_p_lisp);
#endif
	case PV_ML:	return (char_u *)&(curbuf->b_p_ml);
	case PV_MPS:	return (char_u *)&(curbuf->b_p_mps);
	case PV_MA:	return (char_u *)&(curbuf->b_p_ma);
	case PV_MOD:	return (char_u *)&(curbuf->b_changed);
	case PV_NF:	return (char_u *)&(curbuf->b_p_nf);
#ifdef FEAT_OSFILETYPE
	case PV_OFT:	return (char_u *)&(curbuf->b_p_oft);
#endif
	case PV_RO:	return (char_u *)&(curbuf->b_p_ro);
#ifdef FEAT_SMARTINDENT
	case PV_SI:	return (char_u *)&(curbuf->b_p_si);
#endif
#ifndef SHORT_FNAME
	case PV_SN:	return (char_u *)&(curbuf->b_p_sn);
#endif
	case PV_STS:	return (char_u *)&(curbuf->b_p_sts);
#ifdef FEAT_SEARCHPATH
	case PV_SUA:	return (char_u *)&(curbuf->b_p_sua);
#endif
	case PV_SWF:	return (char_u *)&(curbuf->b_p_swf);
#ifdef FEAT_SYN_HL
	case PV_SYN:	return (char_u *)&(curbuf->b_p_syn);
#endif
	case PV_SW:	return (char_u *)&(curbuf->b_p_sw);
	case PV_TS:	return (char_u *)&(curbuf->b_p_ts);
	case PV_TW:	return (char_u *)&(curbuf->b_p_tw);
	case PV_TX:	return (char_u *)&(curbuf->b_p_tx);
	case PV_WM:	return (char_u *)&(curbuf->b_p_wm);
#ifdef FEAT_KEYMAP
	case PV_KMAP:	return (char_u *)&(curbuf->b_p_keymap);
#endif
	default:	EMSG(_("E356: get_varp ERROR"));
    }
    /* always return a valid pointer to avoid a crash! */
    return (char_u *)&(curbuf->b_p_wm);
}

/*
 * Get the value of 'equalprg', either the buffer-local one or the global one.
 */
    char_u *
get_equalprg()
{
    if (*curbuf->b_p_ep == NUL)
	return p_ep;
    return curbuf->b_p_ep;
}

#if defined(FEAT_WINDOWS) || defined(PROTO)
/*
 * Copy options from one window to another.
 * Used when splitting a window.
 */
    void
win_copy_options(wp_from, wp_to)
    win_T	*wp_from;
    win_T	*wp_to;
{
    copy_winopt(&wp_from->w_onebuf_opt, &wp_to->w_onebuf_opt);
    copy_winopt(&wp_from->w_allbuf_opt, &wp_to->w_allbuf_opt);
# ifdef FEAT_RIGHTLEFT
#  ifdef FEAT_FKMAP
    /* Is this right? */
    wp_to->w_farsi = wp_from->w_farsi;
#  endif
# endif
}
#endif

/*
 * Copy the options from one winopt_T to another.
 * Doesn't free the old option values in "to", use clear_winopt() for that.
 * The 'scroll' option is not copied, because it depends on the window height.
 * The 'previewwindow' option is reset, there can be only one preview window.
 */
    void
copy_winopt(from, to)
    winopt_T	*from;
    winopt_T	*to;
{
    to->wo_list = from->wo_list;
    to->wo_nu = from->wo_nu;
#ifdef FEAT_RIGHTLEFT
    to->wo_rl = from->wo_rl;
#endif
    to->wo_wrap = from->wo_wrap;
#ifdef FEAT_LINEBREAK
    to->wo_lbr = from->wo_lbr;
#endif
#ifdef FEAT_SCROLLBIND
    to->wo_scb = from->wo_scb;
#endif
#ifdef FEAT_DIFF
    to->wo_diff = from->wo_diff;
#endif
#ifdef FEAT_FOLDING
    to->wo_fdc = from->wo_fdc;
    to->wo_fen = from->wo_fen;
    to->wo_fdi = vim_strsave(from->wo_fdi);
    to->wo_fml = from->wo_fml;
    to->wo_fdl = from->wo_fdl;
    to->wo_fdm = vim_strsave(from->wo_fdm);
    to->wo_fdn = from->wo_fdn;
# ifdef FEAT_EVAL
    to->wo_fde = vim_strsave(from->wo_fde);
    to->wo_fdt = vim_strsave(from->wo_fdt);
# endif
    to->wo_fmr = vim_strsave(from->wo_fmr);
#endif
    check_winopt(to);		/* don't want NULL pointers */
}

/*
 * Check string options in a window for a NULL value.
 */
    void
check_win_options(win)
    win_T	*win;
{
    check_winopt(&win->w_onebuf_opt);
    check_winopt(&win->w_allbuf_opt);
}

/*
 * Check for NULL pointers in a winopt_T and replace them with empty_option.
 */
/*ARGSUSED*/
    void
check_winopt(wop)
    winopt_T	*wop;
{
#ifdef FEAT_FOLDING
    check_string_option(&wop->wo_fdi);
    check_string_option(&wop->wo_fdm);
# ifdef FEAT_EVAL
    check_string_option(&wop->wo_fde);
    check_string_option(&wop->wo_fdt);
# endif
    check_string_option(&wop->wo_fmr);
#endif
}

/*
 * Free the allocated memory inside a winopt_T.
 */
/*ARGSUSED*/
    void
clear_winopt(wop)
    winopt_T	*wop;
{
#ifdef FEAT_FOLDING
    clear_string_option(&wop->wo_fdi);
    clear_string_option(&wop->wo_fdm);
# ifdef FEAT_EVAL
    clear_string_option(&wop->wo_fde);
    clear_string_option(&wop->wo_fdt);
# endif
    clear_string_option(&wop->wo_fmr);
#endif
}

/*
 * Copy global option values to local options for one buffer.
 * Used when creating a new buffer and sometimes when entering a buffer.
 * flags:
 * BCO_ENTER	We will enter the buf buffer.
 * BCO_ALWAYS	Always copy the options, but only set b_p_initialized when
 *		appropriate.
 * BCO_NOHELP	Don't copy the values to a help buffer.
 */
    void
buf_copy_options(buf, flags)
    buf_T	*buf;
    int		flags;
{
    int		should_copy = TRUE;
    char_u	*save_p_isk = NULL;	    /* init for GCC */
    int		dont_do_help;
    int		did_isk = FALSE;

    /*
     * Don't do anything of the buffer is invalid.
     */
    if (buf == NULL || !buf_valid(buf))
	return;

    /*
     * Skip this when the option defaults have not been set yet.  Happens when
     * main() allocates the first buffer.
     */
    if (p_cpo != NULL)
    {
	/*
	 * Always copy when entering and 'cpo' contains 'S'.
	 * Don't copy when already initialized.
	 * Don't copy when 'cpo' contains 's' and not entering.
	 * 'S'	BCO_ENTER  initialized	's'  should_copy
	 * yes	  yes	       X	 X	TRUE
	 * yes	  no	      yes	 X	FALSE
	 * no	   X	      yes	 X	FALSE
	 *  X	  no	      no	yes	FALSE
	 *  X	  no	      no	no	TRUE
	 * no	  yes	      no	 X	TRUE
	 */
	if ((vim_strchr(p_cpo, CPO_BUFOPTGLOB) == NULL || !(flags & BCO_ENTER))
		&& (buf->b_p_initialized
		    || (!(flags & BCO_ENTER)
			&& vim_strchr(p_cpo, CPO_BUFOPT) != NULL)))
	    should_copy = FALSE;

	if (should_copy || (flags & BCO_ALWAYS))
	{
	    /* Don't copy the options specific to a help buffer when
	     * BCO_NOHELP is given or the options were initialized already
	     * (jumping back to a help file with CTRL-T or CTRL-O) */
	    dont_do_help = ((flags & BCO_NOHELP) && buf->b_help)
						       || buf->b_p_initialized;
	    if (dont_do_help)		/* don't free b_p_isk */
	    {
		save_p_isk = buf->b_p_isk;
		buf->b_p_isk = NULL;
	    }
	    /*
	     * Always free the allocated strings.
	     * If not already initialized, set 'readonly' and copy 'fileformat'.
	     */
	    if (!buf->b_p_initialized)
	    {
		free_buf_options(buf, TRUE);
		buf->b_p_ro = FALSE;		/* don't copy readonly */
		buf->b_p_tx = p_tx;
#ifdef FEAT_MBYTE
		buf->b_p_fenc = vim_strsave(p_fenc);
#endif
		buf->b_p_ff = vim_strsave(p_ff);
#if defined(FEAT_QUICKFIX)
		buf->b_p_bh = empty_option;
		buf->b_p_bt = empty_option;
#endif
	    }
	    else
		free_buf_options(buf, FALSE);

	    buf->b_p_ai = p_ai;
	    buf->b_p_ai_nopaste = p_ai_nopaste;
	    buf->b_p_sw = p_sw;
	    buf->b_p_tw = p_tw;
	    buf->b_p_tw_nopaste = p_tw_nopaste;
	    buf->b_p_tw_nobin = p_tw_nobin;
	    buf->b_p_wm = p_wm;
	    buf->b_p_wm_nopaste = p_wm_nopaste;
	    buf->b_p_wm_nobin = p_wm_nobin;
	    buf->b_p_bin = p_bin;
	    buf->b_p_et = p_et;
	    buf->b_p_et_nobin = p_et_nobin;
	    buf->b_p_ml = p_ml;
	    buf->b_p_ml_nobin = p_ml_nobin;
	    buf->b_p_inf = p_inf;
	    buf->b_p_swf = p_swf;
#ifdef FEAT_INS_EXPAND
	    buf->b_p_cpt = vim_strsave(p_cpt);
#endif
	    buf->b_p_sts = p_sts;
	    buf->b_p_sts_nopaste = p_sts_nopaste;
#ifndef SHORT_FNAME
	    buf->b_p_sn = p_sn;
#endif
#ifdef FEAT_COMMENTS
	    buf->b_p_com = vim_strsave(p_com);
#endif
#ifdef FEAT_FOLDING
	    buf->b_p_cms = vim_strsave(p_cms);
#endif
	    buf->b_p_fo = vim_strsave(p_fo);
	    buf->b_p_nf = vim_strsave(p_nf);
	    buf->b_p_mps = vim_strsave(p_mps);
#ifdef FEAT_SMARTINDENT
	    buf->b_p_si = p_si;
#endif
#ifdef FEAT_CINDENT
	    buf->b_p_cin = p_cin;
	    buf->b_p_cink = vim_strsave(p_cink);
	    buf->b_p_cino = vim_strsave(p_cino);
#endif
#ifdef FEAT_AUTOCMD
	    /* Don't copy 'filetype', it must be detected */
	    buf->b_p_ft = empty_option;
#endif
#ifdef FEAT_OSFILETYPE
	    buf->b_p_oft = vim_strsave(p_oft);
#endif
#if defined(FEAT_SMARTINDENT) || defined(FEAT_CINDENT)
	    buf->b_p_cinw = vim_strsave(p_cinw);
#endif
#ifdef FEAT_LISP
	    buf->b_p_lisp = p_lisp;
#endif
#ifdef FEAT_SYN_HL
	    /* Don't copy 'syntax', it must be set */
	    buf->b_p_syn = empty_option;
#endif
#if defined(FEAT_CINDENT) && defined(FEAT_EVAL)
	    buf->b_p_inde = vim_strsave(p_inde);
	    buf->b_p_indk = vim_strsave(p_indk);
#endif
#ifdef FEAT_CRYPT
	    buf->b_p_key = vim_strsave(p_key);
#endif
#ifdef FEAT_SEARCHPATH
	    buf->b_p_sua = vim_strsave(p_sua);
#endif
#ifdef FEAT_KEYMAP
	    buf->b_p_keymap = vim_strsave(p_keymap);
	    buf->b_kmap_state |= KEYMAP_INIT;
#endif
	    /* This isn't really an option, but copying the langmap and IME
	     * state from the current buffer is better than resetting it. */
	    buf->b_p_iminsert = p_iminsert;
	    buf->b_p_imsearch = p_imsearch;

	    /* options that are normally global but also have a local value
	     * are not copied, start using the global value */
	    buf->b_p_ar = -1;
#ifdef FEAT_QUICKFIX
	    buf->b_p_gp = empty_option;
	    buf->b_p_mp = empty_option;
	    buf->b_p_efm = empty_option;
#endif
	    buf->b_p_ep = empty_option;
	    buf->b_p_path = empty_option;
	    buf->b_p_tags = empty_option;
#ifdef FEAT_FIND_ID
	    buf->b_p_def = empty_option;
	    buf->b_p_inc = empty_option;
# ifdef FEAT_EVAL
	    buf->b_p_inex = vim_strsave(p_inex);
# endif
#endif
#ifdef FEAT_INS_EXPAND
	    buf->b_p_dict = empty_option;
	    buf->b_p_tsr = empty_option;
#endif

	    /*
	     * Don't copy the options set by ex_help(), use the saved values,
	     * when going from a help buffer to a non-help buffer.
	     * Don't touch these at all when BCO_NOHELP is used and going from
	     * or to a help buffer.
	     */
	    if (dont_do_help)
		buf->b_p_isk = save_p_isk;
	    else
	    {
		buf->b_p_isk = vim_strsave(p_isk);
		did_isk = TRUE;
		buf->b_p_ts = p_ts;
		buf->b_help = FALSE;
#ifdef FEAT_QUICKFIX
		if (buf->b_p_bt[0] == 'h')
		    clear_string_option(&buf->b_p_bt);
#endif
		buf->b_p_ma = p_ma;
	    }
	}

	/*
	 * When the options should be copied (ignoring BCO_ALWAYS), set the
	 * flag that indicates that the options have been initialized.
	 */
	if (should_copy)
	    buf->b_p_initialized = TRUE;
    }

    check_buf_options(buf);	    /* make sure we don't have NULLs */
    if (did_isk)
	(void)buf_init_chartab(buf, FALSE);
}

/*
 * Reset the 'modifiable' option and its default value.
 */
    void
reset_modifiable()
{
    int		opt_idx;

    curbuf->b_p_ma = FALSE;
    p_ma = FALSE;
    opt_idx = findoption((char_u *)"ma");
    options[opt_idx].def_val[VI_DEFAULT] = FALSE;
}

/*
 * Set the global value for 'iminsert' to the local value.
 */
    void
set_iminsert_global()
{
    p_iminsert = curbuf->b_p_iminsert;
}

/*
 * Set the global value for 'imsearch' to the local value.
 */
    void
set_imsearch_global()
{
    p_imsearch = curbuf->b_p_imsearch;
}

#if defined(FEAT_CMDL_COMPL) || defined(PROTO)
static int expand_option_idx = -1;
static char_u expand_option_name[5] = {'t', '_', NUL, NUL, NUL};
static int expand_option_flags = 0;

    void
set_context_in_set_cmd(xp, arg, opt_flags)
    expand_T	*xp;
    char_u	*arg;
    int		opt_flags;	/* OPT_GLOBAL and/or OPT_LOCAL */
{
    int		nextchar;
    long_u	flags = 0;	/* init for GCC */
    int		opt_idx = 0;	/* init for GCC */
    char_u	*p;
    char_u	*s;
    int		is_term_option = FALSE;
    int		key;

    expand_option_flags = opt_flags;

    xp->xp_context = EXPAND_SETTINGS;
    if (*arg == NUL)
    {
	xp->xp_pattern = arg;
	return;
    }
    p = arg + STRLEN(arg) - 1;
    if (*p == ' ' && *(p - 1) != '\\')
    {
	xp->xp_pattern = p + 1;
	return;
    }
    while (p > arg)
    {
	s = p;
	/* count number of backslashes before ' ' or ',' */
	if (*p == ' ' || *p == ',')
	{
	    while (s > arg && *(s - 1) == '\\')
		--s;
	}
	/* break at a space with an even number of backslashes */
	if (*p == ' ' && ((p - s) & 1) == 0)
	{
	    ++p;
	    break;
	}
	--p;
    }
    if (STRNCMP(p, "no", 2) == 0)
    {
	xp->xp_context = EXPAND_BOOL_SETTINGS;
	p += 2;
    }
    if (STRNCMP(p, "inv", 3) == 0)
    {
	xp->xp_context = EXPAND_BOOL_SETTINGS;
	p += 3;
    }
    xp->xp_pattern = arg = p;
    if (*arg == '<')
    {
	while (*p != '>')
	    if (*p++ == NUL)	    /* expand terminal option name */
		return;
	key = get_special_key_code(arg + 1);
	if (key == 0)		    /* unknown name */
	{
	    xp->xp_context = EXPAND_NOTHING;
	    return;
	}
	nextchar = *++p;
	is_term_option = TRUE;
	expand_option_name[2] = KEY2TERMCAP0(key);
	expand_option_name[3] = KEY2TERMCAP1(key);
    }
    else
    {
	if (p[0] == 't' && p[1] == '_')
	{
	    p += 2;
	    if (*p != NUL)
		++p;
	    if (*p == NUL)
		return;		/* expand option name */
	    nextchar = *++p;
	    is_term_option = TRUE;
	    expand_option_name[2] = p[-2];
	    expand_option_name[3] = p[-1];
	}
	else
	{
		/* Allow * wildcard */
	    while (ASCII_ISALNUM(*p) || *p == '_' || *p == '*')
		p++;
	    if (*p == NUL)
		return;
	    nextchar = *p;
	    *p = NUL;
	    opt_idx = findoption(arg);
	    *p = nextchar;
	    if (opt_idx == -1 || options[opt_idx].var == NULL)
	    {
		xp->xp_context = EXPAND_NOTHING;
		return;
	    }
	    flags = options[opt_idx].flags;
	    if (flags & P_BOOL)
	    {
		xp->xp_context = EXPAND_NOTHING;
		return;
	    }
	}
    }
    /* handle "-=" and "+=" */
    if ((nextchar == '-' || nextchar == '+' || nextchar == '^') && p[1] == '=')
    {
	++p;
	nextchar = '=';
    }
    if ((nextchar != '=' && nextchar != ':')
				    || xp->xp_context == EXPAND_BOOL_SETTINGS)
    {
	xp->xp_context = EXPAND_UNSUCCESSFUL;
	return;
    }
    if (xp->xp_context != EXPAND_BOOL_SETTINGS && p[1] == NUL)
    {
	xp->xp_context = EXPAND_OLD_SETTING;
	if (is_term_option)
	    expand_option_idx = -1;
	else
	    expand_option_idx = opt_idx;
	xp->xp_pattern = p + 1;
	return;
    }
    xp->xp_context = EXPAND_NOTHING;
    if (is_term_option || (flags & P_NUM))
	return;

    xp->xp_pattern = p + 1;

    if (flags & P_EXPAND)
    {
	p = options[opt_idx].var;
	if (p == (char_u *)&p_bdir
		|| p == (char_u *)&p_dir
		|| p == (char_u *)&p_path
		|| p == (char_u *)&p_rtp
#ifdef FEAT_SEARCHPATH
		|| p == (char_u *)&p_cdpath
#endif
#ifdef FEAT_SESSION
		|| p == (char_u *)&p_vdir
#endif
		)
	{
	    xp->xp_context = EXPAND_DIRECTORIES;
	    if (p == (char_u *)&p_path
#ifdef FEAT_SEARCHPATH
		    || p == (char_u *)&p_cdpath
#endif
		   )
		xp->xp_backslash = XP_BS_THREE;
	    else
		xp->xp_backslash = XP_BS_ONE;
	}
	else
	{
	    xp->xp_context = EXPAND_FILES;
	    /* for 'tags' need three backslashes for a space */
	    if (p == (char_u *)&p_tags)
		xp->xp_backslash = XP_BS_THREE;
	    else
		xp->xp_backslash = XP_BS_ONE;
	}
    }

    /* For an option that is a list of file names, find the start of the
     * last file name. */
    for (p = arg + STRLEN(arg) - 1; p > xp->xp_pattern; --p)
    {
	/* count number of backslashes before ' ' or ',' */
	if (*p == ' ' || *p == ',')
	{
	    s = p;
	    while (s > xp->xp_pattern && *(s - 1) == '\\')
		--s;
	    if ((*p == ' ' && (xp->xp_backslash == XP_BS_THREE && (p - s) < 3))
		    || (*p == ',' && (flags & P_COMMA) && ((p - s) & 1) == 0))
	    {
		xp->xp_pattern = p + 1;
		break;
	    }
	}
    }

    return;
}

    int
ExpandSettings(xp, regmatch, num_file, file)
    expand_T	*xp;
    regmatch_T	*regmatch;
    int		*num_file;
    char_u	***file;
{
    int		num_normal = 0;	    /* Nr of matching non-term-code settings */
    int		num_term = 0;	    /* Nr of matching terminal code settings */
    int		opt_idx;
    int		match;
    int		count = 0;
    char_u	*str;
    int		loop;
    int		is_term_opt;
    char_u	name_buf[MAX_KEY_NAME_LEN];
    static char *(names[]) = {"all", "termcap"};
    int		ic = regmatch->rm_ic;	/* remember the ignore-case flag */

    /* do this loop twice:
     * loop == 0: count the number of matching options
     * loop == 1: copy the matching options into allocated memory
     */
    for (loop = 0; loop <= 1; ++loop)
    {
	regmatch->rm_ic = ic;
	if (xp->xp_context != EXPAND_BOOL_SETTINGS)
	{
	    for (match = 0; match < sizeof(names) / sizeof(char *); ++match)
		if (vim_regexec(regmatch, (char_u *)names[match], (colnr_T)0))
		{
		    if (loop == 0)
			num_normal++;
		    else
			(*file)[count++] = vim_strsave((char_u *)names[match]);
		}
	}
	for (opt_idx = 0; (str = (char_u *)options[opt_idx].fullname) != NULL;
								    opt_idx++)
	{
	    if (options[opt_idx].var == NULL)
		continue;
	    if (xp->xp_context == EXPAND_BOOL_SETTINGS
	      && !(options[opt_idx].flags & P_BOOL))
		continue;
	    is_term_opt = istermoption(&options[opt_idx]);
	    if (is_term_opt && num_normal > 0)
		continue;
	    match = FALSE;
	    if (vim_regexec(regmatch, str, (colnr_T)0)
		    || (options[opt_idx].shortname != NULL
			&& vim_regexec(regmatch,
			   (char_u *)options[opt_idx].shortname, (colnr_T)0)))
		match = TRUE;
	    else if (is_term_opt)
	    {
		name_buf[0] = '<';
		name_buf[1] = 't';
		name_buf[2] = '_';
		name_buf[3] = str[2];
		name_buf[4] = str[3];
		name_buf[5] = '>';
		name_buf[6] = NUL;
		if (vim_regexec(regmatch, name_buf, (colnr_T)0))
		{
		    match = TRUE;
		    str = name_buf;
		}
	    }
	    if (match)
	    {
		if (loop == 0)
		{
		    if (is_term_opt)
			num_term++;
		    else
			num_normal++;
		}
		else
		    (*file)[count++] = vim_strsave(str);
	    }
	}
	/*
	 * Check terminal key codes, these are not in the option table
	 */
	if (xp->xp_context != EXPAND_BOOL_SETTINGS  && num_normal == 0)
	{
	    for (opt_idx = 0; (str = get_termcode(opt_idx)) != NULL; opt_idx++)
	    {
		if (!isprint(str[0]) || !isprint(str[1]))
		    continue;

		name_buf[0] = 't';
		name_buf[1] = '_';
		name_buf[2] = str[0];
		name_buf[3] = str[1];
		name_buf[4] = NUL;

		match = FALSE;
		if (vim_regexec(regmatch, name_buf, (colnr_T)0))
		    match = TRUE;
		else
		{
		    name_buf[0] = '<';
		    name_buf[1] = 't';
		    name_buf[2] = '_';
		    name_buf[3] = str[0];
		    name_buf[4] = str[1];
		    name_buf[5] = '>';
		    name_buf[6] = NUL;

		    if (vim_regexec(regmatch, name_buf, (colnr_T)0))
			match = TRUE;
		}
		if (match)
		{
		    if (loop == 0)
			num_term++;
		    else
			(*file)[count++] = vim_strsave(name_buf);
		}
	    }

	    /*
	     * Check special key names.
	     */
	    regmatch->rm_ic = TRUE;		/* ignore case here */
	    for (opt_idx = 0; (str = get_key_name(opt_idx)) != NULL; opt_idx++)
	    {
		name_buf[0] = '<';
		STRCPY(name_buf + 1, str);
		STRCAT(name_buf, ">");

		if (vim_regexec(regmatch, name_buf, (colnr_T)0))
		{
		    if (loop == 0)
			num_term++;
		    else
			(*file)[count++] = vim_strsave(name_buf);
		}
	    }
	}
	if (loop == 0)
	{
	    if (num_normal > 0)
		*num_file = num_normal;
	    else if (num_term > 0)
		*num_file = num_term;
	    else
		return OK;
	    *file = (char_u **)alloc((unsigned)(*num_file * sizeof(char_u *)));
	    if (*file == NULL)
	    {
		*file = (char_u **)"";
		return FAIL;
	    }
	}
    }
    return OK;
}

    int
ExpandOldSetting(num_file, file)
    int	    *num_file;
    char_u  ***file;
{
    char_u  *var = NULL;	/* init for GCC */
    char_u  *buf;

    *num_file = 0;
    *file = (char_u **)alloc((unsigned)sizeof(char_u *));
    if (*file == NULL)
	return FAIL;

    /*
     * For a terminal key code expand_option_idx is < 0.
     */
    if (expand_option_idx < 0)
    {
	var = find_termcode(expand_option_name + 2);
	if (var == NULL)
	    expand_option_idx = findoption(expand_option_name);
    }

    if (expand_option_idx >= 0)
    {
	/* put string of option value in NameBuff */
	option_value2string(&options[expand_option_idx], expand_option_flags);
	var = NameBuff;
    }
    else if (var == NULL)
	var = (char_u *)"";

    /* A backslash is required before some characters.  This is the reverse of
     * what happens in do_set(). */
    buf = vim_strsave_escaped(var, escape_chars);

    if (buf == NULL)
    {
	vim_free(*file);
	*file = NULL;
	return FAIL;
    }

#ifdef BACKSLASH_IN_FILENAME
    /* For MS-Windows et al. we don't double backslashes at the start and
     * before a file name character. */
    for (var = buf; *var != NUL; )
    {
	if (var[0] == '\\' && var[1] == '\\'
		&& expand_option_idx >= 0
		&& (options[expand_option_idx].flags & P_EXPAND)
		&& vim_isfilec(var[2])
		&& (var[2] != '\\' || (var == buf && var[4] != '\\')))
	    mch_memmove(var, var + 1, STRLEN(var));
#ifdef FEAT_MBYTE
	else if (has_mbyte)
	    var += (*mb_ptr2len_check)(var) - 1;
#endif
	++var;
    }
#endif

    *file[0] = buf;
    *num_file = 1;
    return OK;
}
#endif

/*
 * Get the value for the numeric or string option *opp in a nice format into
 * NameBuff[].  Must not be called with a hidden option!
 */
    static void
option_value2string(opp, opt_flags)
    struct vimoption	*opp;
    int			opt_flags;	/* OPT_GLOBAL and/or OPT_LOCAL */
{
    char_u	*varp;

    varp = get_varp_scope(opp, opt_flags);

    if (opp->flags & P_NUM)
    {
	long wc = 0;

	if (wc_use_keyname(varp, &wc))
	    STRCPY(NameBuff, get_special_key_name((int)wc, 0));
	else if (wc != 0)
	    STRCPY(NameBuff, transchar((int)wc));
	else
	    sprintf((char *)NameBuff, "%ld", *(long *)varp);
    }
    else    /* P_STRING */
    {
	varp = *(char_u **)(varp);
	if (varp == NULL)		    /* just in case */
	    NameBuff[0] = NUL;
#ifdef FEAT_CRYPT
	/* don't show the actual value of 'key', only that it's set */
	if (opp->var == (char_u *)&p_key && *varp)
	    STRCPY(NameBuff, "*****");
#endif
	else if (opp->flags & P_EXPAND)
	    home_replace(NULL, varp, NameBuff, MAXPATHL, FALSE);
	/* Translate 'pastetoggle' into special key names */
	else if ((char_u **)opp->var == &p_pt)
	    str2specialbuf(p_pt, NameBuff, MAXPATHL);
	else
	    STRNCPY(NameBuff, varp, MAXPATHL);
    }
}

/*
 * Return TRUE if "varp" points to 'wildchar' or 'wildcharm' and it can be
 * printed as a keyname.
 * "*wcp" is set to the value of the option if it's 'wildchar' or 'wildcharm'.
 */
    static int
wc_use_keyname(varp, wcp)
    char_u	*varp;
    long	*wcp;
{
    if (((long *)varp == &p_wc) || ((long *)varp == &p_wcm))
    {
	*wcp = *(long *)varp;
	if (IS_SPECIAL(*wcp) || find_special_key_in_table((int)*wcp) >= 0)
	    return TRUE;
    }
    return FALSE;
}

#ifdef FEAT_LANGMAP
/*
 * Any character has an equivalent character.  This is used for keyboards that
 * have a special language mode that sends characters above 128 (although
 * other characters can be translated too).
 */

/*
 * char_u langmap_mapchar[256];
 * Normally maps each of the 128 upper chars to an <128 ascii char; used to
 * "translate" native lang chars in normal mode or some cases of
 * insert mode without having to tediously switch lang mode back&forth.
 */

    static void
langmap_init()
{
    int i;

    for (i = 0; i < 256; i++)		/* we init with a-one-to one map */
	langmap_mapchar[i] = i;
}

/*
 * Called when langmap option is set; the language map can be
 * changed at any time!
 */
    static void
langmap_set()
{
    char_u  *p;
    char_u  *p2;
    int	    from, to;

    langmap_init();			    /* back to one-to-one map first */

    for (p = p_langmap; p[0] != NUL; )
    {
	for (p2 = p; p2[0] != NUL && p2[0] != ',' && p2[0] != ';'; ++p2)
	{
	    if (p2[0] == '\\' && p2[1] != NUL)
		++p2;
#ifdef FEAT_MBYTE
	    p2 += (*mb_ptr2len_check)(p2) - 1;
#endif
	}
	if (p2[0] == ';')
	    ++p2;	    /* abcd;ABCD form, p2 points to A */
	else
	    p2 = NULL;	    /* aAbBcCdD form, p2 is NULL */
	while (p[0])
	{
	    if (p[0] == '\\' && p[1] != NUL)
		++p;
#ifdef FEAT_MBYTE
	    from = (*mb_ptr2char)(p);
#else
	    from = p[0];
#endif
	    if (p2 == NULL)
	    {
#ifdef FEAT_MBYTE
		p += (*mb_ptr2len_check)(p);
#else
		++p;
#endif
		if (p[0] == '\\')
		    ++p;
#ifdef FEAT_MBYTE
		to = (*mb_ptr2char)(p);
#else
		to = p[0];
#endif
	    }
	    else
	    {
		if (p2[0] == '\\')
		    ++p2;
#ifdef FEAT_MBYTE
		to = (*mb_ptr2char)(p2);
#else
		to = p2[0];
#endif
	    }
	    if (to == NUL)
	    {
		EMSG2(_("E357: 'langmap': Matching character missing for %s"),
							     transchar(from));
		return;
	    }
	    langmap_mapchar[from & 255] = to;

	    /* Advance to next pair */
#ifdef FEAT_MBYTE
	    p += (*mb_ptr2len_check)(p);
#else
	    ++p;
#endif
	    if (p2 == NULL)
	    {
		if (p[0] == ',')
		{
		    ++p;
		    break;
		}
	    }
	    else
	    {
#ifdef FEAT_MBYTE
		p2 += (*mb_ptr2len_check)(p2);
#else
		++p2;
#endif
		if (*p == ';')
		{
		    p = p2;
		    if (p[0] != NUL)
		    {
			if (p[0] != ',')
			{
			    EMSG2(_("E358: 'langmap': Extra characters after semicolon: %s"), p);
			    return;
			}
			++p;
		    }
		    break;
		}
	    }
	}
    }
}
#endif

/*
 * Return TRUE if format option 'x' is in effect.
 * Take care of no formatting when 'paste' is set.
 */
    int
has_format_option(x)
    int		x;
{
    if (p_paste)
	return FALSE;
    return (vim_strchr(curbuf->b_p_fo, x) != NULL);
}

/*
 * Return TRUE if "x" is present in 'shortmess' option, or
 * 'shortmess' contains 'a' and "x" is present in SHM_A.
 */
    int
shortmess(x)
    int	    x;
{
    return (   vim_strchr(p_shm, x) != NULL
	    || (vim_strchr(p_shm, 'a') != NULL
		&& vim_strchr((char_u *)SHM_A, x) != NULL));
}

/*
 * paste_option_changed() - Called after p_paste was set or reset.
 */
    static void
paste_option_changed()
{
    static int	old_p_paste = FALSE;
    static int	save_sm = 0;
#ifdef FEAT_CMDL_INFO
    static int	save_ru = 0;
#endif
#ifdef FEAT_RIGHTLEFT
    static int	save_ri = 0;
    static int	save_hkmap = 0;
#endif
    buf_T	*buf;

    if (p_paste)
    {
	/*
	 * Paste switched from off to on.
	 * Save the current values, so they can be restored later.
	 */
	if (!old_p_paste)
	{
	    /* save options for each buffer */
	    for (buf = firstbuf; buf != NULL; buf = buf->b_next)
	    {
		buf->b_p_tw_nopaste = buf->b_p_tw;
		buf->b_p_wm_nopaste = buf->b_p_wm;
		buf->b_p_sts_nopaste = buf->b_p_sts;
		buf->b_p_ai_nopaste = buf->b_p_ai;
	    }

	    /* save global options */
	    save_sm = p_sm;
#ifdef FEAT_CMDL_INFO
	    save_ru = p_ru;
#endif
#ifdef FEAT_RIGHTLEFT
	    save_ri = p_ri;
	    save_hkmap = p_hkmap;
#endif
	    /* save global values for local buffer options */
	    p_tw_nopaste = p_tw;
	    p_wm_nopaste = p_wm;
	    p_sts_nopaste = p_sts;
	    p_ai_nopaste = p_ai;
	}

	/*
	 * Always set the option values, also when 'paste' is set when it is
	 * already on.
	 */
	/* set options for each buffer */
	for (buf = firstbuf; buf != NULL; buf = buf->b_next)
	{
	    buf->b_p_tw = 0;	    /* textwidth is 0 */
	    buf->b_p_wm = 0;	    /* wrapmargin is 0 */
	    buf->b_p_sts = 0;	    /* softtabstop is 0 */
	    buf->b_p_ai = 0;	    /* no auto-indent */
	}

	/* set global options */
	p_sm = 0;		    /* no showmatch */
#ifdef FEAT_CMDL_INFO
# ifdef FEAT_WINDOWS
	if (p_ru)
	    status_redraw_all();    /* redraw to remove the ruler */
# endif
	p_ru = 0;		    /* no ruler */
#endif
#ifdef FEAT_RIGHTLEFT
	p_ri = 0;		    /* no reverse insert */
	p_hkmap = 0;		    /* no Hebrew keyboard */
#endif
	/* set global values for local buffer options */
	p_tw = 0;
	p_wm = 0;
	p_sts = 0;
	p_ai = 0;
    }

    /*
     * Paste switched from on to off: Restore saved values.
     */
    else if (old_p_paste)
    {
	/* restore options for each buffer */
	for (buf = firstbuf; buf != NULL; buf = buf->b_next)
	{
	    buf->b_p_tw = buf->b_p_tw_nopaste;
	    buf->b_p_wm = buf->b_p_wm_nopaste;
	    buf->b_p_sts = buf->b_p_sts_nopaste;
	    buf->b_p_ai = buf->b_p_ai_nopaste;
	}

	/* restore global options */
	p_sm = save_sm;
#ifdef FEAT_CMDL_INFO
# ifdef FEAT_WINDOWS
	if (p_ru != save_ru)
	    status_redraw_all();    /* redraw to draw the ruler */
# endif
	p_ru = save_ru;
#endif
#ifdef FEAT_RIGHTLEFT
	p_ri = save_ri;
	p_hkmap = save_hkmap;
#endif
	/* set global values for local buffer options */
	p_tw = p_tw_nopaste;
	p_wm = p_wm_nopaste;
	p_sts = p_sts_nopaste;
	p_ai = p_ai_nopaste;
    }

    old_p_paste = p_paste;
}

/*
 * vimrc_found() - Called when a ".vimrc" or "VIMINIT" has been found.
 *
 * Reset 'compatible' and set the values for options that didn't get set yet
 * to the Vim defaults.
 * Don't do this if the 'compatible' option has been set or reset before.
 */
    void
vimrc_found()
{
    int	    opt_idx;

    if (!option_was_set((char_u *)"cp"))
    {
	p_cp = FALSE;
	for (opt_idx = 0; !istermoption(&options[opt_idx]); opt_idx++)
	    if (!(options[opt_idx].flags & (P_WAS_SET|P_VI_DEF)))
		set_option_default(opt_idx, OPT_FREE, FALSE);
	didset_options();
    }
}

/*
 * Set 'compatible' on or off.  Called for "-C" and "-N" command line arg.
 */
    void
change_compatible(on)
    int	    on;
{
    if (p_cp != on)
    {
	p_cp = on;
	compatible_set();
    }
    options[findoption((char_u *)"cp")].flags |= P_WAS_SET;
}

/*
 * Return TRUE when option "name" has been set.
 */
    int
option_was_set(name)
    char_u	*name;
{
    int idx;

    idx = findoption(name);
    if (idx < 0)	/* unknown option */
	return FALSE;
    if (options[idx].flags & P_WAS_SET)
	return TRUE;
    return FALSE;
}

/*
 * compatible_set() - Called when 'compatible' has been set or unset.
 *
 * When 'compatible' set: Set all relevant options (those that have the P_VIM)
 * flag) to a Vi compatible value.
 * When 'compatible' is unset: Set all options that have a different default
 * for Vim (without the P_VI_DEF flag) to that default.
 */
    static void
compatible_set()
{
    int	    opt_idx;

    for (opt_idx = 0; !istermoption(&options[opt_idx]); opt_idx++)
	if (	   ((options[opt_idx].flags & P_VIM) && p_cp)
		|| (!(options[opt_idx].flags & P_VI_DEF) && !p_cp))
	    set_option_default(opt_idx, OPT_FREE, p_cp);
    didset_options();
}

#ifdef FEAT_LINEBREAK

# if defined(__BORLANDC__) && (__BORLANDC__ < 0x500)
   /* Borland C++ screws up loop optimisation here (negri) */
#  pragma option -O-l
# endif

/*
 * fill_breakat_flags() -- called when 'breakat' changes value.
 */
    static void
fill_breakat_flags()
{
    char_u	*c;
    int		i;

    for (i = 0; i < 256; i++)
	breakat_flags[i] = FALSE;

    if (p_breakat != NULL)
	for (c = p_breakat; *c; c++)
	    breakat_flags[*c] = TRUE;
}

# if defined(__BORLANDC__) && (__BORLANDC__ < 0x500)
#  pragma option -O.l
# endif

#endif

/*
 * Check an option that can be a range of string values.
 *
 * Return OK for correct value, FAIL otherwise.
 * Empty is always OK.
 */
    static int
check_opt_strings(val, values, list)
    char_u	*val;
    char	**values;
    int		list;	    /* when TRUE: accept a list of values */
{
    return opt_strings_flags(val, values, NULL, list);
}

/*
 * Handle an option that can be a range of string values.
 * Set a flag in "*flagp" for each string present.
 *
 * Return OK for correct value, FAIL otherwise.
 * Empty is always OK.
 */
    static int
opt_strings_flags(val, values, flagp, list)
    char_u	*val;		/* new value */
    char	**values;	/* array of valid string values */
    unsigned	*flagp;
    int		list;		/* when TRUE: accept a list of values */
{
    int		i;
    int		len;
    unsigned	new_flags = 0;

    while (*val)
    {
	for (i = 0; ; ++i)
	{
	    if (values[i] == NULL)	/* val not found in values[] */
		return FAIL;

	    len = (int)STRLEN(values[i]);
	    if (STRNCMP(values[i], val, len) == 0
		    && ((list && val[len] == ',') || val[len] == NUL))
	    {
		val += len + (val[len] == ',');
		new_flags |= (1 << i);
		break;		/* check next item in val list */
	    }
	}
    }
    if (flagp != NULL)
	*flagp = new_flags;

    return OK;
}

/*
 * Read the 'wildmode' option, fill wim_flags[].
 */
    static int
check_opt_wim()
{
    char_u	new_wim_flags[4];
    char_u	*p;
    int		i;
    int		idx = 0;

    for (i = 0; i < 4; ++i)
	new_wim_flags[i] = 0;

    for (p = p_wim; *p; ++p)
    {
	for (i = 0; ASCII_ISALPHA(p[i]); ++i)
	    ;
	if (p[i] != NUL && p[i] != ',' && p[i] != ':')
	    return FAIL;
	if (i == 7 && STRNCMP(p, "longest", 7) == 0)
	    new_wim_flags[idx] |= WIM_LONGEST;
	else if (i == 4 && STRNCMP(p, "full", 4) == 0)
	    new_wim_flags[idx] |= WIM_FULL;
	else if (i == 4 && STRNCMP(p, "list", 4) == 0)
	    new_wim_flags[idx] |= WIM_LIST;
	else
	    return FAIL;
	p += i;
	if (*p == NUL)
	    break;
	if (*p == ',')
	{
	    if (idx == 3)
		return FAIL;
	    ++idx;
	}
    }

    /* fill remaining entries with last flag */
    while (idx < 3)
    {
	new_wim_flags[idx + 1] = new_wim_flags[idx];
	++idx;
    }

    /* only when there are no errors, wim_flags[] is changed */
    for (i = 0; i < 4; ++i)
	wim_flags[i] = new_wim_flags[i];
    return OK;
}

/*
 * Check if backspacing over something is allowed.
 */
    int
can_bs(what)
    int		what;	    /* BS_INDENT, BS_EOL or BS_START */
{
    switch (*p_bs)
    {
	case '2':	return TRUE;
	case '1':	return (what != BS_START);
	case '0':	return FALSE;
    }
    return vim_strchr(p_bs, what) != NULL;
}

/*
 * Save the current values of 'fileformat' and 'fileencoding', so that we know
 * the file must be considered changed when the value is different.
 */
    void
save_file_ff(buf)
    buf_T	*buf;
{
    buf->b_start_ffc = *buf->b_p_ff;
    buf->b_start_eol = buf->b_p_eol;
#ifdef FEAT_MBYTE
    vim_free(buf->b_start_fenc);
    buf->b_start_fenc = vim_strsave(buf->b_p_fenc);
#endif
}

/*
 * Return TRUE if 'fileformat' and/or 'fileencoding' has a different value
 * from when editing started (save_file_ff() called).
 * Also when 'endofline' was changed and 'binary' is set.
 */
    int
file_ff_differs(buf)
    buf_T	*buf;
{
    if (buf->b_start_ffc != *buf->b_p_ff)
	return TRUE;
    if (buf->b_p_bin && buf->b_start_eol != buf->b_p_eol)
	return TRUE;
#ifdef FEAT_MBYTE
    if (buf->b_start_fenc == NULL)
	return (*buf->b_p_fenc != NUL);
    return (STRCMP(buf->b_start_fenc, buf->b_p_fenc) != 0);
#else
    return FALSE;
#endif
}

/*
 * return OK if "p" is a valid fileformat name, FAIL otherwise.
 */
    int
check_ff_value(p)
    char_u	*p;
{
    return check_opt_strings(p, p_ff_values, FALSE);
}
