/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * Common MS-DOS and Win32 (Windows NT and Windows 95) defines.
 *
 * Names for the EXRC, HELP and temporary files.
 * Some of these may have been defined in the makefile or feature.h.
 */

#ifndef USR_VIMRC_FILE
# define USR_VIMRC_FILE		"$HOME\\_vimrc"
#endif
#ifndef USR_VIMRC_FILE2
# define USR_VIMRC_FILE2	"$VIM\\_vimrc"
#endif

#ifndef USR_EXRC_FILE
# define USR_EXRC_FILE		"$HOME\\_exrc"
#endif
#ifndef USR_EXRC_FILE2
# define USR_EXRC_FILE2		"$VIM\\_exrc"
#endif

#ifdef FEAT_GUI
# ifndef USR_GVIMRC_FILE
#  define USR_GVIMRC_FILE	"$HOME\\_gvimrc"
# endif
# ifndef USR_GVIMRC_FILE2
#  define USR_GVIMRC_FILE2	"$VIM\\_gvimrc"
# endif
# ifndef SYS_MENU_FILE
#  define SYS_MENU_FILE		"$VIMRUNTIME\\menu.vim"
# endif
#endif

#ifndef SYS_OPTWIN_FILE
# define SYS_OPTWIN_FILE	"$VIMRUNTIME\\optwin.vim"
#endif

#ifdef FEAT_VIMINFO
# ifndef VIMINFO_FILE
#  define VIMINFO_FILE		"$HOME\\_viminfo"
# endif
# ifndef VIMINFO_FILE2
#  define VIMINFO_FILE2		"$VIM\\_viminfo"
# endif
#endif

#ifndef VIMRC_FILE
# define VIMRC_FILE	"_vimrc"
#endif

#ifndef EXRC_FILE
# define EXRC_FILE	"_exrc"
#endif

#ifdef FEAT_GUI
# ifndef GVIMRC_FILE
#  define GVIMRC_FILE	"_gvimrc"
# endif
#endif

#ifndef DFLT_HELPFILE
# define DFLT_HELPFILE	"$VIMRUNTIME\\doc\\help.txt"
#endif

#ifndef FILETYPE_FILE
# define FILETYPE_FILE	"filetype.vim"
#endif
#ifndef SETTINGS_FILE
# define SETTINGS_FILE	"settings.vim"
#endif
#ifndef FTOFF_FILE
# define FTOFF_FILE	"ftoff.vim"
#endif
#ifndef SETSOFF_FILE
# define SETSOFF_FILE	"setsoff.vim"
#endif

#ifndef SYNTAX_FNAME
# define SYNTAX_FNAME	"$VIMRUNTIME\\syntax\\%s.vim"
#endif

#ifndef DFLT_BDIR
# define DFLT_BDIR	".,c:\\tmp,c:\\temp"	/* default for 'backupdir' */
#endif

#ifndef DFLT_DIR
# define DFLT_DIR	".,c:\\tmp,c:\\temp"	/* default for 'directory' */
#endif

#define DFLT_ERRORFILE		"errors.err"
#define DFLT_MAKEEF		"vim##.err"
#define DFLT_RUNTIMEPATH	"$VIMRUNTIME"

#define CASE_INSENSITIVE_FILENAME   /* ignore case when comparing file names */
#define SPACE_IN_FILENAME
#define BACKSLASH_IN_FILENAME
#define USE_CRNL		/* lines end in CR-NL instead of NL */
