/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved		by Bram Moolenaar
 *				GUI support by Robert Webb
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 * See README.txt for an overview of the Vim source code.
 */
/*
 * Windows GUI.
 *
 * GUI support for Microsoft Windows.  Win32 initially; maybe Win16 later
 *
 * George V. Reilly <george@reilly.org> wrote the original Win32 GUI.
 * Robert Webb reworked it to use the existing GUI stuff and added menu,
 * scrollbars, etc.
 *
 * Note: Clipboard stuff, for cutting and pasting text to other windows, is in
 * os_win32.c.	(It can also be done from the terminal version).
 */

/*
 * Include the common stuff for MS-Windows GUI.
 */
#include "gui_w48.c"


#ifdef __MINGW32__
/*
 * Add a lot of missing defines.
 * They are not always missing, we need the #ifndef's.
 */
# ifndef _cdecl
#  define _cdecl
# endif
# ifndef IsMinimized
#  define     IsMinimized(hwnd)		IsIconic(hwnd)
# endif
# ifndef IsMaximized
#  define     IsMaximized(hwnd)		IsZoomed(hwnd)
# endif
# ifndef SelectFont
#  define     SelectFont(hdc, hfont)  ((HFONT)SelectObject((hdc), (HGDIOBJ)(HFONT)(hfont)))
# endif
# ifndef GetStockBrush
#  define     GetStockBrush(i)     ((HBRUSH)GetStockObject(i))
# endif
# ifndef DeleteBrush
#  define     DeleteBrush(hbr)     DeleteObject((HGDIOBJ)(HBRUSH)(hbr))
# endif

# ifndef HANDLE_WM_RBUTTONDBLCLK
#  define HANDLE_WM_RBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_MBUTTONUP
#  define HANDLE_WM_MBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_MBUTTONDBLCLK
#  define HANDLE_WM_MBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_LBUTTONDBLCLK
#  define HANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_RBUTTONDOWN
#  define HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_MOUSEMOVE
#  define HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_RBUTTONUP
#  define HANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_MBUTTONDOWN
#  define HANDLE_WM_MBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_LBUTTONUP
#  define HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_LBUTTONDOWN
#  define HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_SYSCHAR
#  define HANDLE_WM_SYSCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
# endif
# ifndef HANDLE_WM_ACTIVATEAPP
#  define HANDLE_WM_ACTIVATEAPP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), (DWORD)(lParam)), 0L)
# endif
# ifndef HANDLE_WM_WINDOWPOSCHANGING
#  define HANDLE_WM_WINDOWPOSCHANGING(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)((hwnd), (LPWINDOWPOS)(lParam))
# endif
# ifndef HANDLE_WM_VSCROLL
#  define HANDLE_WM_VSCROLL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(lParam), (UINT)(LOWORD(wParam)),  (int)(short)HIWORD(wParam)), 0L)
# endif
# ifndef HANDLE_WM_SETFOCUS
#  define HANDLE_WM_SETFOCUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_KILLFOCUS
#  define HANDLE_WM_KILLFOCUS(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_HSCROLL
#  define HANDLE_WM_HSCROLL(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(lParam), (UINT)(LOWORD(wParam)), (int)(short)HIWORD(wParam)), 0L)
# endif
# ifndef HANDLE_WM_DROPFILES
#  define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HDROP)(wParam)), 0L)
# endif
# ifndef HANDLE_WM_CHAR
#  define HANDLE_WM_CHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
# endif
# ifndef HANDLE_WM_SYSDEADCHAR
#  define HANDLE_WM_SYSDEADCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
# endif
# ifndef HANDLE_WM_DEADCHAR
#  define HANDLE_WM_DEADCHAR(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
# endif
#endif /* __MINGW32__ */


/*
 * Use the system font for dialogs and tear-off menus.  Remove this line to
 * use DLG_FONT_NAME.
 */
#define USE_SYSMENU_FONT

/* Some parameters for tearoff menus.  All in pixels. */
#define TEAROFF_PADDING_X	2
#define TEAROFF_BUTTON_PAD_X	8
#define TEAROFF_MIN_WIDTH	200
#define TEAROFF_SUBMENU_LABEL	">>"
#define TEAROFF_COLUMN_PADDING	3	// # spaces to pad column with.


/* For the Intellimouse: */
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x20a
#endif


/* Local variables: */

static HBRUSH	s_brush = NULL;
#ifdef FEAT_MENU
static UINT	s_menu_id = 100;
#endif
static int	destroying = FALSE; /* call DestroyWindow() ourselves */


#define VIM_NAME	"vim"
#define VIM_CLASS	"Vim"

static OSVERSIONINFO os_version;    /* like it says.  Init in gui_mch_init() */

/* ron: No need to be stingy on Win32. Make it 16K - s/b big enough for
 * everyone!
 * I have had problems with the original 1000 byte, and with 2 or 5 K.  But
 * 16K should be good for all but the biggest.  Anyway, we free the memory
 * right away.
 */
#define DLG_ALLOC_SIZE 16 * 1024

/*
 * stuff for dialogs, menus, tearoffs etc.
 */
static LRESULT APIENTRY dialog_callback(HWND, UINT, WPARAM, LPARAM);
static LRESULT APIENTRY tearoff_callback(HWND, UINT, WPARAM, LPARAM);
static PWORD
add_dialog_element(
	PWORD p,
	DWORD lStyle,
	WORD x,
	WORD y,
	WORD w,
	WORD h,
	WORD Id,
	WORD clss,
	const char *caption);
static LPWORD lpwAlign(LPWORD);
static int nCopyAnsiToWideChar(LPWORD, LPSTR);
static void gui_mch_tearoff(char_u *title, vimmenu_t *menu, int initX, int initY);
static void rebuild_tearoff(vimmenu_t *menu);
static void get_dialog_font_metrics(void);

static int dialog_default_button = -1;

/* Intellimouse support */
static int mouse_scroll_lines = 0;
static UINT msh_msgmousewheel = 0;

static int	s_usenewlook;	    /* emulate W95/NT4 non-bold dialogs */
static HBITMAP	s_htearbitmap;	    /* bitmap used to indicate tearoff */
#ifdef FEAT_TOOLBAR
static void initialise_toolbar(void);
static int get_toolbar_bitmap(char_u *name);
#endif

/*
 * Return TRUE when running under Windows NT 3.x or Win32s, both of which have
 * less fancy GUI APIs.
 */
    static int
is_winnt_3(void)
{
    return ((os_version.dwPlatformId == VER_PLATFORM_WIN32_NT
		&& os_version.dwMajorVersion == 3)
	    || (os_version.dwPlatformId == VER_PLATFORM_WIN32s));
}

/*
 * Return TRUE when running under Win32s.
 */
    int
gui_is_win32s(void)
{
    return (os_version.dwPlatformId == VER_PLATFORM_WIN32s);
}

#ifdef FEAT_MENU
/*
 * Figure out how high the menu bar is at the moment.
 */
    static int
gui_w32_get_menu_height(
    int	    fix_window)	    /* If TRUE, resize window if menu height changed */
{
    static int	old_menu_height = -1;

    RECT    rc1, rc2;
    int	    num;
    int	    menu_height;

    if (gui.menu_is_active)
	num = GetMenuItemCount(s_menuBar);
    else
	num = 0;

    if (num == 0)
	menu_height = 0;
    else
    {
	if (is_winnt_3())	/* for NT 3.xx */
	{
	    if (gui.starting)
		menu_height = GetSystemMetrics(SM_CYMENU);
	    else
	    {
		RECT r1, r2;
		int frameht = GetSystemMetrics(SM_CYFRAME);
		int capht = GetSystemMetrics(SM_CYCAPTION);

		/* get window rect of s_hwnd
		 * get client rect of s_hwnd
		 * get cap height
		 * subtract from window rect, the sum of client height,
		 * (if not maximized)frame thickness, and caption height.
		 */
		GetWindowRect(s_hwnd, &r1);
		GetClientRect(s_hwnd, &r2);
		menu_height = r1.bottom - r1.top - (r2.bottom - r2.top
				 + 2 * frameht * (!IsZoomed(s_hwnd)) + capht);
	    }
	}
	else			/* win95 and variants (NT 4.0, I guess) */
	{
	    /*
	     * In case 'lines' is set in _vimrc/_gvimrc window width doesn't
	     * seem to have been set yet, so menu wraps in default window
	     * width which is very narrow.  Instead just return height of a
	     * single menu item.  Will still be wrong when the menu really
	     * should wrap over more than one line.
	     */
	    GetMenuItemRect(s_hwnd, s_menuBar, 0, &rc1);
	    if (gui.starting)
		menu_height = rc1.bottom - rc1.top + 1;
	    else
	    {
		GetMenuItemRect(s_hwnd, s_menuBar, num - 1, &rc2);
		menu_height = rc2.bottom - rc1.top + 1;
	    }
	}
    }

    if (fix_window && menu_height != old_menu_height)
	gui_set_shellsize(FALSE);

    old_menu_height = menu_height;
    return menu_height;
}
#endif /*FEAT_MENU*/


/*
 * Get a message when the window is being destroyed.
 */
    static void
_OnDestroy(
    HWND hwnd)
{
    if (!destroying)
	_OnClose(hwnd);
}

/*
 * Got a message when the system will go down.
 */
    static void
_OnEndSession(void)
{
    ml_close_notmod();		    /* close all not-modified buffers */
    ml_sync_all(FALSE, FALSE);	    /* preserve all swap files */
    ml_close_all(FALSE);	    /* close all memfiles, without deleting */
    getout(1);			    /* exit Vim properly */
}


    static void
_OnDropFiles(
    HWND hwnd,
    HDROP hDrop)
{
#ifdef FEAT_WINDOWS
    char    szFile[_MAX_PATH];
    UINT    cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, szFile, _MAX_PATH);
    UINT    i;
    char_u  *fname;
    char_u  **fnames;
    char_u  redo_dirs = FALSE;

    /* TRACE("_OnDropFiles: %d files dropped\n", cFiles); */

#ifdef FEAT_VISUAL
    reset_VIsual();
#endif

    fnames = (char_u **)alloc(cFiles * sizeof(char_u *));

    for (i = 0; i < cFiles; ++i)
    {
	DragQueryFile(hDrop, i, szFile, _MAX_PATH);

	mch_dirname(IObuff, IOSIZE);
	fname = shorten_fname(szFile, IObuff);
	if (fname == NULL)
	    fname = szFile;
	fnames[i] = vim_strsave(fname);
    }
    DragFinish(hDrop);

    /*
     * When the cursor is at the command line, add the file names to the
     * command line, don't edit the files.
     */
    if (State & CMDLINE)
    {
	for (i = 0; i < cFiles; ++i)
	{
	    if (fnames[i] != NULL)
	    {
		if (i > 0)
		    add_to_input_buf(" ", 1);
		add_to_input_buf(fnames[i], STRLEN(fnames[i]));
	    }
	}
    }
    else
    {
	/*
	 * Handle dropping a directory on Vim.
	 * Change to that directory and don't open any file.
	 */
	if (cFiles == 1 && mch_isdir(fnames[0]))
	{
	    if (mch_chdir(fnames[0]) == 0)
	    {
		smsg((char_u *)":cd %s", fnames[0]);
		vim_free(fnames[0]);
		fnames[0] = NULL;
		redo_dirs = TRUE;
	    }
	}

	if (fnames[0] != NULL)
	{
	    /* Shift held down, change to first file's directory */
	    if (GetKeyState(VK_SHIFT) & 0x8000)
		if (vim_chdirfile(fnames[0]) == 0)
		    redo_dirs = TRUE;

	    /* Handle the drop, :edit or :split to get to the file */
	    handle_drop(cFiles, fnames,
				   ((GetKeyState(VK_CONTROL) & 0x8000) != 0));
	}

	if (redo_dirs)
	    shorten_fnames(TRUE);

	/* Update the screen display */
	update_screen(NOT_VALID);
	setcursor();
	out_flush();
    }

    /* SetActiveWindow() doesn't work here... */
    (void)SetForegroundWindow(s_hwnd);
#endif
}


    static void
_OnPaint(
    HWND hwnd)
{
    if (!IsMinimized(hwnd))
    {
	PAINTSTRUCT ps;

	out_flush();	    /* make sure all output has been processed */
	(void)BeginPaint(hwnd, &ps);

#ifdef FEAT_MBYTE
	/* prevent multi-byte characters from misprinting on an invalid
	 * rectangle */
	if (has_mbyte)
	{
	    RECT rect;

	    GetClientRect(hwnd, &rect);
	    ps.rcPaint.left = rect.left;
	    ps.rcPaint.right = rect.right;
	}
#endif

	if (!IsRectEmpty(&ps.rcPaint))
	    gui_redraw(ps.rcPaint.left, ps.rcPaint.top,
		    ps.rcPaint.right - ps.rcPaint.left + 1,
		    ps.rcPaint.bottom - ps.rcPaint.top + 1);
	EndPaint(hwnd, &ps);
    }
}

    static void
_OnSize(
    HWND hwnd,
    UINT state,
    int cx,
    int cy)
{
    if (!IsMinimized(hwnd))
    {
	gui_resize_shell(cx, cy);

#ifdef FEAT_MENU
	/* Menu bar may wrap differently now */
	gui_w32_get_menu_height(TRUE);
#endif
    }
}

    static void
_OnSetFocus(
    HWND hwnd,
    HWND hwndOldFocus)
{
    gui_focus_change(TRUE);
    (void)DefWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hwndOldFocus, 0);
}

    static void
_OnKillFocus(
    HWND hwnd,
    HWND hwndNewFocus)
{
    gui_focus_change(FALSE);
    (void)DefWindowProc(hwnd, WM_KILLFOCUS, (WPARAM)hwndNewFocus, 0);
}


    static int
_OnScroll(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos)
{
    scrollbar_t *sb, *sb_info;
    long	val;
    int		dragging = FALSE;
    SCROLLINFO	si;
    static UINT	prev_code = 0;   /* code of previous call */

    sb = gui_mswin_find_scrollbar(hwndCtl);
    if (sb == NULL)
	return 0;

    if (sb->wp != NULL)	    /* Left or right scrollbar */
    {
	/*
	 * Careful: need to get scrollbar info out of first (left) scrollbar
	 * for window, but keep real scrollbar too because we must pass it to
	 * gui_drag_scrollbar().
	 */
	sb_info = &sb->wp->w_scrollbars[0];
    }
    else	    /* Bottom scrollbar */
	sb_info = sb;
    val = sb_info->value;

    switch (code)
    {
	case SB_THUMBTRACK:
	    val = pos;
	    dragging = TRUE;
	    if (sb->scroll_shift > 0)
		val <<= sb->scroll_shift;
	    break;
	case SB_LINEDOWN:
	    /* Because of round-off errors we can't move one line when
	     * scroll_shift is non-zero.  Scroll some extra. */
	    if (sb->scroll_shift > 0)
		val += (1 << sb->scroll_shift);
	    else
		val++;
	    break;
	case SB_LINEUP:
	    val--;
	    break;
	case SB_PAGEDOWN:
	    val += (sb_info->size > 2 ? sb_info->size - 2 : 1);
	    break;
	case SB_PAGEUP:
	    val -= (sb_info->size > 2 ? sb_info->size - 2 : 1);
	    break;
	case SB_TOP:
	    val = 0;
	    break;
	case SB_BOTTOM:
	    val = sb_info->max;
	    break;
	case SB_ENDSCROLL:
	    if (prev_code == SB_THUMBTRACK)
	    {
		/*
		 * "pos" only gives us 16-bit data.  In case of large file,
		 * use GetScrollPos() which returns 32-bit.  Unfortunately it
		 * is not valid while the scrollbar is being dragged.
		 */
		val = GetScrollPos(hwndCtl, SB_CTL);
		if (sb->scroll_shift > 0)
		    val <<= sb->scroll_shift;
	    }
	    break;

	default:
	    /* TRACE("Unknown scrollbar event %d\n", code); */
	    return 0;
    }
    prev_code = code;

    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    if (sb->scroll_shift > 0)
	si.nPos = val >> sb->scroll_shift;
    else
	si.nPos = val;
    SetScrollInfo(hwndCtl, SB_CTL, &si, TRUE);

    /*
     * When moving a vertical scrollbar, move the other vertical scrollbar too.
     */
    if (sb->wp != NULL)
    {
	if (sb == &sb->wp->w_scrollbars[SBAR_LEFT])
	    SetScrollInfo(sb->wp->w_scrollbars[SBAR_RIGHT].id,
							   SB_CTL, &si, TRUE);
	else
	    SetScrollInfo(sb->wp->w_scrollbars[SBAR_LEFT].id,
							   SB_CTL, &si, TRUE);
    }

    /* Don't let us be interrupted here by another message. */
    s_busy_processing = TRUE;
    gui_drag_scrollbar(sb, val, dragging);
    s_busy_processing = FALSE;

    return 0;
}

/*
 * Setup for the Intellimouse
 */
    static void
init_mouse_wheel(void)
{

#ifndef SPI_GETWHEELSCROLLLINES
# define SPI_GETWHEELSCROLLLINES    104
#endif

#define VMOUSEZ_CLASSNAME  "MouseZ"		/* hidden wheel window class */
#define VMOUSEZ_TITLE      "Magellan MSWHEEL"	/* hidden wheel window title */
#define VMSH_MOUSEWHEEL    "MSWHEEL_ROLLMSG"
#define VMSH_SCROLL_LINES  "MSH_SCROLL_LINES_MSG"

    HWND hdl_mswheel;
    UINT msh_msgscrolllines;

    msh_msgmousewheel = 0;
    mouse_scroll_lines = 3;	/* reasonable default */

    if ((os_version.dwPlatformId == VER_PLATFORM_WIN32_NT
		&& os_version.dwMajorVersion >= 4)
	    || (os_version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		&& ((os_version.dwMajorVersion == 4
			&& os_version.dwMinorVersion >= 10)
		    || os_version.dwMajorVersion >= 5)))
    {
	/* if NT 4.0+ (or Win98) get scroll lines directly from system */
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0,
		&mouse_scroll_lines, 0);
    }
    else if (os_version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
	    || (os_version.dwPlatformId == VER_PLATFORM_WIN32_NT
		&& os_version.dwMajorVersion < 4))
    {	/*
	 * If Win95 or NT 3.51,
	 * try to find the hidden point32 window.
	 */
	hdl_mswheel = FindWindow(VMOUSEZ_CLASSNAME, VMOUSEZ_TITLE);
	if (hdl_mswheel)
	{
	    msh_msgscrolllines = RegisterWindowMessage(VMSH_SCROLL_LINES);
	    if (msh_msgscrolllines)
	    {
		mouse_scroll_lines = SendMessage(hdl_mswheel,
			msh_msgscrolllines, 0, 0);
		msh_msgmousewheel  = RegisterWindowMessage(VMSH_MOUSEWHEEL);
	    }
	}
    }
}


/* Intellimouse wheel handler */
    static void
_OnMouseWheel(
    HWND hwnd,
    short zDelta)
{
/* Treat a mouse wheel event as if it were a scroll request */
    int i;
    int size;
    HWND hwndCtl;

    if (curwin->w_scrollbars[SBAR_RIGHT].id != 0)
    {
	hwndCtl = curwin->w_scrollbars[SBAR_RIGHT].id;
	size = curwin->w_scrollbars[SBAR_RIGHT].size;
    }
    else if (curwin->w_scrollbars[SBAR_LEFT].id != 0)
    {
	hwndCtl = curwin->w_scrollbars[SBAR_LEFT].id;
	size = curwin->w_scrollbars[SBAR_LEFT].size;
    }
    else
	return;

    size = curwin->w_height;
    if (mouse_scroll_lines == 0)
	init_mouse_wheel();

    if (mouse_scroll_lines > 0
	    && mouse_scroll_lines < (size > 2 ? size - 2 : 1))
    {
	for (i = mouse_scroll_lines; i > 0; --i)
	    _OnScroll(hwnd, hwndCtl, zDelta >= 0 ? SB_LINEUP : SB_LINEDOWN, 0);
    }
    else
	_OnScroll(hwnd, hwndCtl, zDelta >= 0 ? SB_PAGEUP : SB_PAGEDOWN, 0);
}

/*
 * Get current x mouse coordinate in text window.
 * Return -1 when unknown.
 */
    int
gui_mch_get_mouse_x(void)
{
    RECT rct;
    POINT mp;

    if (GetWindowRect(s_textArea, &rct))
    {
	if (GetCursorPos((LPPOINT)&mp))
	    return (int)(mp.x - rct.left);
    }
    return -1;
}

/*
 * Get current y mouse coordinate in text window.
 * Return -1 when unknown.
 */
    int
gui_mch_get_mouse_y(void)
{
    RECT rct;
    POINT mp;

    if (GetWindowRect(s_textArea, &rct))
    {
	if (GetCursorPos((LPPOINT)&mp))
	    return (int)(mp.y - rct.top);
    }
    return -1;
}

/*
 * Move mouse pointer to character at (x, y).
 */
    void
gui_mch_setmouse(int x, int y)
{
    RECT rct;

    if (GetWindowRect(s_textArea, &rct))
	(void)SetCursorPos(x + gui.border_offset + rct.left,
			   y + gui.border_offset + rct.top);
}

    static void
gui_w32_get_valid_dimensions(
    int w,
    int h,
    int *valid_w,
    int *valid_h)
{
    int	    base_width, base_height;

    base_width = gui_get_base_width()
	+ GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    base_height = gui_get_base_height()
	+ GetSystemMetrics(SM_CYSIZEFRAME) * 2
	+ GetSystemMetrics(SM_CYCAPTION)
	+ gui_w32_get_menu_height(FALSE);
    *valid_w = base_width +
		    ((w - base_width) / gui.char_width) * gui.char_width;
    *valid_h = base_height +
		    ((h - base_height) / gui.char_height) * gui.char_height;
}

/*
 * Even though we have _DuringSizing() which makes the rubber band a valid
 * size, we need this for when the user maximises the window.
 * TODO: Doesn't seem to adjust the width though for some reason.
 */
    static BOOL
_OnWindowPosChanging(
    HWND hwnd,
    LPWINDOWPOS lpwpos)
{
    RECT    workarea_rect;

    if (!(lpwpos->flags & SWP_NOSIZE))
    {
	if (IsMaximized(hwnd)
		&& (os_version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		    || (os_version.dwPlatformId == VER_PLATFORM_WIN32_NT
			&& os_version.dwMajorVersion >= 4)))
	{
	    SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea_rect, 0);
	    lpwpos->x = workarea_rect.left;
	    lpwpos->y = workarea_rect.top;
	    lpwpos->cx = workarea_rect.right - workarea_rect.left;
	    lpwpos->cy = workarea_rect.bottom - workarea_rect.top;
	}
	gui_w32_get_valid_dimensions(lpwpos->cx, lpwpos->cy,
				     &lpwpos->cx, &lpwpos->cy);
    }
    return 0;
}

    static int
_DuringSizing(
    HWND hwnd,
    UINT fwSide,
    LPRECT lprc)
{
    int	    w, h;
    int	    valid_w, valid_h;
    int	    w_offset, h_offset;

    w = lprc->right - lprc->left;
    h = lprc->bottom - lprc->top;
    gui_w32_get_valid_dimensions(w, h, &valid_w, &valid_h);
    w_offset = w - valid_w;
    h_offset = h - valid_h;

    if (fwSide == WMSZ_LEFT || fwSide == WMSZ_TOPLEFT
			    || fwSide == WMSZ_BOTTOMLEFT)
	lprc->left += w_offset;
    else if (fwSide == WMSZ_RIGHT || fwSide == WMSZ_TOPRIGHT
			    || fwSide == WMSZ_BOTTOMRIGHT)
	lprc->right -= w_offset;

    if (fwSide == WMSZ_TOP || fwSide == WMSZ_TOPLEFT
			    || fwSide == WMSZ_TOPRIGHT)
	lprc->top += h_offset;
    else if (fwSide == WMSZ_BOTTOM || fwSide == WMSZ_BOTTOMLEFT
			    || fwSide == WMSZ_BOTTOMRIGHT)
	lprc->bottom -= h_offset;
    return TRUE;
}



    static LRESULT CALLBACK
_WndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    /*
    TRACE("WndProc: hwnd = %08x, msg = %x, wParam = %x, lParam = %x\n",
	  hwnd, uMsg, wParam, lParam);
    */

    HandleMouseHide(uMsg, lParam);

    s_uMsg = uMsg;
    s_wParam = wParam;
    s_lParam = lParam;

    switch (uMsg)
    {
	HANDLE_MSG(hwnd, WM_DEADCHAR,	_OnDeadChar);
	HANDLE_MSG(hwnd, WM_SYSDEADCHAR, _OnDeadChar);
	/* HANDLE_MSG(hwnd, WM_ACTIVATE,    _OnActivate); */
	HANDLE_MSG(hwnd, WM_CHAR,	_OnChar);
	HANDLE_MSG(hwnd, WM_CLOSE,	_OnClose);
	/* HANDLE_MSG(hwnd, WM_COMMAND,	_OnCommand); */
	HANDLE_MSG(hwnd, WM_DESTROY,	_OnDestroy);
	HANDLE_MSG(hwnd, WM_DROPFILES,	_OnDropFiles);
	HANDLE_MSG(hwnd, WM_HSCROLL,	_OnScroll);
	HANDLE_MSG(hwnd, WM_KILLFOCUS,	_OnKillFocus);
#ifdef FEAT_MENU
	HANDLE_MSG(hwnd, WM_COMMAND,	_OnMenu);
#endif
	/* HANDLE_MSG(hwnd, WM_MOVE,	    _OnMove); */
	/* HANDLE_MSG(hwnd, WM_NCACTIVATE,  _OnNCActivate); */
	HANDLE_MSG(hwnd, WM_SETFOCUS,	_OnSetFocus);
	HANDLE_MSG(hwnd, WM_SIZE,	_OnSize);
	/* HANDLE_MSG(hwnd, WM_SYSCOMMAND,  _OnSysCommand); */
	/* HANDLE_MSG(hwnd, WM_SYSKEYDOWN,  _OnAltKey); */
	HANDLE_MSG(hwnd, WM_VSCROLL,	_OnScroll);
	HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING,	_OnWindowPosChanging);
	HANDLE_MSG(hwnd, WM_ACTIVATEAPP, _OnActivateApp);

    case WM_QUERYENDSESSION:	/* System wants to go down. */
	gui_shell_closed();	/* Will exit when no changed buffers. */
	return FALSE;		/* Do NOT allow system to go down. */

    case WM_ENDSESSION:
	if (wParam)	/* system only really goes down when wParam is TRUE */
	    _OnEndSession();
	break;

    case WM_SYSCHAR:
	/*
	 * if 'winaltkeys' is "no", or it's "menu" and it's not a menu
	 * shortcut key, handle like a typed ALT key, otherwise call Windows
	 * ALT key handling.
	 */
#ifdef FEAT_MENU
	if (	!gui.menu_is_active
		|| p_wak[0] == 'n'
		|| (p_wak[0] == 'm' && !gui_is_menu_shortcut((int)wParam))
		)
#endif
	    return HANDLE_WM_SYSCHAR((hwnd), (wParam), (lParam), (_OnSysChar));
#ifdef FEAT_MENU
	else
	    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif

    case WM_SYSKEYUP:
#ifdef FEAT_MENU
	/* Only when menu is active, ALT key is used for that. */
	if (gui.menu_is_active)
	{
	    return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	else
#endif
	    return 0;

    case WM_CREATE:	/* HANDLE_MSG doesn't seem to handle this one */
	return _OnCreate (hwnd, (LPCREATESTRUCT)lParam);

    case WM_SIZING:	/* HANDLE_MSG doesn't seem to handle this one */
	return _DuringSizing(hwnd, wParam, (LPRECT)lParam);

    case WM_MOUSEWHEEL:
	_OnMouseWheel(hwnd, HIWORD(wParam));
	break;

#ifdef FEAT_TOOLBAR
    case WM_NOTIFY:
	switch (((LPNMHDR) lParam)->code)
	{
	    case TTN_NEEDTEXT:
		{
		    LPTOOLTIPTEXT	lpttt;
		    UINT		idButton;
		    int			idx;
		    vimmenu_t		*pMenu;

		    lpttt = (LPTOOLTIPTEXT)lParam;
		    idButton = lpttt->hdr.idFrom;
		    pMenu = gui_mswin_find_menu(root_menu, idButton);
		    if (pMenu)
		    {
			idx = MENU_INDEX_TIP;
			if (pMenu->strings[idx])
			{
			    lpttt->hinst = NULL;  /* string, not resource*/
			    lpttt->lpszText = pMenu->strings[idx];
			}
		    }
		}
		break;
	    default:
		break;
	}
	break;
#endif
#if defined(MENUHINTS) && defined(FEAT_MENU)
    case WM_MENUSELECT:
	if (((UINT) HIWORD(wParam)
		    & (0xffff ^ (MF_MOUSESELECT + MF_BITMAP + MF_POPUP)))
		== MF_HILITE
		&& (State & CMDLINE) == 0)
	{
	    UINT	idButton;
	    int		idx;
	    vimmenu_t	*pMenu;

	    idButton = (UINT)LOWORD(wParam);
	    pMenu = gui_mswin_find_menu(root_menu, idButton);
	    if (pMenu)
	    {
		idx = MENU_INDEX_TIP;
		msg_clr_cmdline();
		if (pMenu->strings[idx])
		    msg(pMenu->strings[idx]);
		else
		    msg("");
		setcursor();
		out_flush();
	    }
	}
	break;
#endif
    case WM_NCHITTEST:
	{
	    LRESULT	result;
	    int x, y;
	    int xPos = GET_X_LPARAM(lParam);

	    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	    if (result == HTCLIENT)
	    {
		gui_mch_get_winpos(&x, &y);
		xPos -= x;

		if (xPos < 48) /*<VN> TODO should use system metric?*/
		    return HTBOTTOMLEFT;
		else
		    return HTBOTTOMRIGHT;
		}
	    else
		return result;
	}
	break;

#ifdef FEAT_MBYTE_IME
    case WM_IME_NOTIFY:
	if (!_OnImeNotify(hwnd, (DWORD)wParam, (DWORD)lParam))
	    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif

    default:
	if (uMsg == msh_msgmousewheel && msh_msgmousewheel != 0)
	{   /* handle MSH_MOUSEWHEEL messages for Intellimouse */
	    _OnMouseWheel(hwnd, HIWORD(wParam));
	    break;
	}
#ifdef MSWIN_FIND_REPLACE
	else
	if (uMsg == s_findrep_msg && s_findrep_msg != 0)
	{
	    _OnFindRepl();
	}
#endif
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 1;
}



/*
 * End of call-back routines
 */

    static void
gui_w32_oleerr(char *arg)
{
    EMSG2(_("Argument not supported: \"-%s\"; Use the OLE version."), arg);
}

/*
 * Parse the GUI related command-line arguments.  Any arguments used are
 * deleted from argv, and *argc is decremented accordingly.  This is called
 * when vim is started, whether or not the GUI has been started.
 */
    void
gui_mch_prepare(int *argc, char **argv)
{
    /* Check for special OLE command line parameters */
    if (*argc == 2 && (argv[1][0] == '-' || argv[1][0] == '/'))
    {
	/* Register Vim as an OLE Automation server */
	if (STRICMP(argv[1] + 1, "register") == 0)
	{
#ifdef FEAT_OLE
	    RegisterMe();
	    mch_windexit(0);
#else
	    gui_w32_oleerr("register");
	    mch_windexit(2);
#endif
	}

	/* Unregister Vim as an OLE Automation server */
	if (STRICMP(argv[1] + 1, "unregister") == 0)
	{
#ifdef FEAT_OLE
	    UnregisterMe(TRUE);
	    mch_windexit(0);
#else
	    gui_w32_oleerr("unregister");
	    mch_windexit(2);
#endif
	}

	/* Ignore an -embedding argument. It is only relevant if the
	 * application wants to treat the case when it is started manually
	 * differently from the case where it is started via automation (and
	 * we don't).
	 */
	if (STRICMP(argv[1] + 1, "embedding") == 0)
	{
#ifdef FEAT_OLE
	    *argc = 1;
#else
	    gui_w32_oleerr("embedding");
	    mch_windexit(2);
#endif
	}
    }

#ifdef FEAT_OLE
    {
	int	bDoRestart = FALSE;

	InitOLE(&bDoRestart);
	/* automatically exit after registering */
	if (bDoRestart)
	    mch_windexit(0);
    }
#endif

    /* get the OS version info */
    os_version.dwOSVersionInfoSize = sizeof(os_version);
    GetVersionEx(&os_version); /* this call works on Win32s, Win95 and WinNT */
}

/*
 * Initialise the GUI.	Create all the windows, set up all the call-backs
 * etc.
 */
    int
gui_mch_init(void)
{
    const char szVimWndClass[] = VIM_CLASS;
    const char szTextAreaClass[] = "VimTextArea";
    WNDCLASS wndclass;
#ifdef GLOBAL_IME
    ATOM	atom;
#endif

    /* Display any pending error messages */
    mch_display_error();

    /* Return here if the window was already opened (happens when
     * gui_mch_dialog() is called early). */
    if (s_hwnd != NULL)
	return OK;

    /*
     * Load the tearoff bitmap
     */
    s_htearbitmap = LoadBitmap(s_hinst, "IDB_TEAROFF");

    gui.scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
    gui.scrollbar_height = GetSystemMetrics(SM_CYHSCROLL);
#ifdef FEAT_MENU
    gui.menu_height = 0;	/* Windows takes care of this */
#endif
    gui.border_width = 0;

    s_brush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    if (GetClassInfo(s_hinst, szVimWndClass, &wndclass) == 0) {
    wndclass.style = 0;
    wndclass.lpfnWndProc = _WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = s_hinst;
    wndclass.hIcon = LoadIcon(wndclass.hInstance, "IDR_VIM");
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = s_brush;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szVimWndClass;

    if ((
#ifdef GLOBAL_IME
	atom =
#endif
		RegisterClass(&wndclass)) == 0)
	return FAIL;
    }

    s_hwnd = CreateWindow(
	szVimWndClass, "Vim MSWindows GUI",
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT, CW_USEDEFAULT,
	100,				/* Any value will do */
	100,				/* Any value will do */
	NULL, NULL,
	s_hinst, NULL);

    if (s_hwnd == NULL)
	return FAIL;

#ifdef GLOBAL_IME
    global_ime_init(atom, s_hwnd);
#endif

    /* Create the text area window */
    if (GetClassInfo(s_hinst, szTextAreaClass, &wndclass) == 0) {
    wndclass.style = CS_OWNDC;
    wndclass.lpfnWndProc = _TextAreaWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = s_hinst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szTextAreaClass;

    if (RegisterClass(&wndclass) == 0)
	return FAIL;
	}
    s_textArea = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	szTextAreaClass, "Vim text area",
	WS_CHILD | WS_VISIBLE, 0, 0,
	100,				/* Any value will do for now */
	100,				/* Any value will do for now */
	s_hwnd, NULL,
	s_hinst, NULL);

    if (s_textArea == NULL)
	return FAIL;

#ifdef FEAT_MENU
    s_menuBar = CreateMenu();
#endif
    s_hdc = GetDC(s_textArea);

    DragAcceptFiles(s_hwnd, TRUE);

    /* Do we need to bother with this? */
    /* m_fMouseAvail = GetSystemMetrics(SM_MOUSEPRESENT); */

    /* Get background/foreground colors from system */
    gui.norm_pixel = GetSysColor(COLOR_WINDOWTEXT);
    gui.back_pixel = GetSysColor(COLOR_WINDOW);
    gui.def_norm_pixel = gui.norm_pixel;
    gui.def_back_pixel = gui.back_pixel;

    /* Get the colors from the "Normal" group (set in syntax.c or in a vimrc
     * file) */
    set_normal_colors();

    /*
     * Check that none of the colors are the same as the background color.
     * Then store the current values as the defaults.
     */
    gui_check_colors();
    gui.def_norm_pixel = gui.norm_pixel;
    gui.def_back_pixel = gui.back_pixel;

    /* Get the colors for the highlight groups (gui_check_colors() might have
     * changed them) */
    highlight_gui_started();

    /*
     * Start out by adding the configured border width into the border offset
     */
    gui.border_offset = gui.border_width + 2;	/*CLIENT EDGE*/

    /*
     * Set up for Intellimouse processing
     */
    init_mouse_wheel();

    /*
     * compute a couple of metrics used for the dialogs
     */
    get_dialog_font_metrics();
#ifdef FEAT_TOOLBAR
    /*
     * Create the toolbar
     */
    initialise_toolbar();
#endif
#ifdef MSWIN_FIND_REPLACE
    /*
     * Initialise the dialog box stuff
     */
    s_findrep_msg = RegisterWindowMessage(FINDMSGSTRING);

    /* Initialise the struct */
    s_findrep_struct.lStructSize = sizeof(s_findrep_struct);
    s_findrep_struct.lpstrFindWhat = alloc(MSWIN_FR_BUFSIZE);
    s_findrep_struct.lpstrFindWhat[0] = NUL;
    s_findrep_struct.lpstrReplaceWith = alloc(MSWIN_FR_BUFSIZE);
    s_findrep_struct.lpstrReplaceWith[0] = NUL;
    s_findrep_struct.wFindWhatLen = MSWIN_FR_BUFSIZE;
    s_findrep_struct.wReplaceWithLen = MSWIN_FR_BUFSIZE;
#endif

    return OK;
}


    void
gui_mch_exit(int rc)
{
    ReleaseDC(s_textArea, s_hdc);
    DeleteObject(s_brush);

    /* Unload the tearoff bitmap */
    (void)DeleteObject((HGDIOBJ)s_htearbitmap);

    /* Destroy our window (if we have one). */
    if (s_hwnd != NULL)
    {
	destroying = TRUE;	/* ignore WM_DESTROY message now */
	DestroyWindow(s_hwnd);
    }

#ifdef GLOBAL_IME
    global_ime_end();
#endif
}


/*
 * Set the size of the window to the given width and height in pixels.
 */
    void
gui_mch_set_shellsize(int width, int height, int min_width, int min_height,
		    int base_width, int base_height)
{
    RECT    workarea_rect;
    int     win_width, win_height;
    int	    win_xpos, win_ypos;
    WINDOWPLACEMENT wndpl;

    /* try to keep window completely on screen */
    /* get size of the screen work area (excludes taskbar, appbars) */
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea_rect, 0);

    /* get current posision of our window */
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(s_hwnd, &wndpl);
    if (wndpl.showCmd == SW_SHOWNORMAL)
    {
	win_xpos = wndpl.rcNormalPosition.left;
	win_ypos = wndpl.rcNormalPosition.top;
    }
    else
    {
	win_xpos = workarea_rect.left;
	win_ypos = workarea_rect.top;
    }

    /* compute the size of the outside of the window */
    win_width = width + GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    win_height = height + GetSystemMetrics(SM_CYSIZEFRAME) * 2
			+ GetSystemMetrics(SM_CYCAPTION)
#ifdef FEAT_MENU
			+ gui_w32_get_menu_height(FALSE)
#endif
			;

    /* if the window is going off the screen, move it on to the screen */
    if (win_xpos + win_width > workarea_rect.right)
	win_xpos = workarea_rect.right - win_width;

    if (win_xpos < workarea_rect.left)
	win_xpos = workarea_rect.left;

    if (win_ypos + win_height > workarea_rect.bottom)
	win_ypos = workarea_rect.bottom - win_height;

    if (win_ypos < workarea_rect.top)
	win_ypos = workarea_rect.top;

    /* set window position */
    SetWindowPos(s_hwnd, NULL, win_xpos, win_ypos, win_width, win_height,
		 SWP_NOZORDER | SWP_NOACTIVATE);

#ifdef FEAT_MENU
    /* Menu may wrap differently now */
    gui_w32_get_menu_height(!gui.starting);
#endif
}

    void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
    RECT    workarea_rect;

    /* get size of the screen work area (excludes taskbar, appbars) */
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea_rect, 0);

    *screen_w = workarea_rect.right - workarea_rect.left
	      - GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    *screen_h = workarea_rect.bottom - workarea_rect.top
	      - GetSystemMetrics(SM_CYSIZEFRAME) * 2
	      - GetSystemMetrics(SM_CYCAPTION)
	      - gui_w32_get_menu_height(FALSE);
}

    void
gui_mch_set_scrollbar_thumb(
    scrollbar_t *sb,
    long	val,
    long	size,
    long	max)
{
    SCROLLINFO	info;

    sb->scroll_shift = 0;
    while (max > 32767)
    {
	max = (max + 1) >> 1;
	val  >>= 1;
	size >>= 1;
	++sb->scroll_shift;
    }

    if (sb->scroll_shift > 0)
	++size;

    info.cbSize = sizeof(info);
    info.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
    info.nPos = val;
    info.nMin = 0;
    info.nMax = max;
    info.nPage = size;
    SetScrollInfo(sb->id, SB_CTL, &info, TRUE);
}


/*
 * Set the current text font.
 */
    void
gui_mch_set_font(GuiFont font)
{
    gui.currFont = font;
}




/*
 * Set the current text foreground color.
 */
    void
gui_mch_set_fg_color(guicolor_t color)
{
    gui.currFgColor = color;
}

/*
 * Set the current text background color.
 */
    void
gui_mch_set_bg_color(guicolor_t color)
{
    gui.currBgColor = color;
}

#if (defined(FEAT_MBYTE) && defined(FEAT_MBYTE_IME)) || defined(PROTO)
/*
 * Multi-byte handling, by Sung-Hoon Baek
 */

    static void
HanExtTextOut(HDC hdc, int X, int Y, UINT fuOption, const RECT *lprc,
	LPCTSTR lpString, UINT cbCount, const int *lpDx, BOOL bOpaque)
{
    LPCTSTR	pszTemp;
    int		i;
    HPEN	hpen, old_pen;
    POINT	point;
    int		n;

    if (gui.char_width == sysfixed_width && gui.char_height == sysfixed_height)
    {
	hpen = CreatePen(PS_SOLID, 2, gui.currFgColor);
	old_pen = SelectObject(hdc, hpen);

	pszTemp = lpString;
	i = 0;
	while (cbCount > 0)
	{
	    if (cbCount > 1 && (n = MB_BYTE2LEN(*(char_u *)pszTemp)) > 1)
	    {
		cbCount -= n;
		pszTemp += n;
		i += n;
	    }
	    else if (*pszTemp == '\\')
	    {
		if (i > 0)
		    ExtTextOut(hdc, X+((pszTemp-i)-lpString)*gui.char_width, Y,
			    fuOption, lprc, pszTemp-i, i, lpDx);
		MoveToEx(hdc, (int)(X+(pszTemp-lpString)*gui.char_width
			    + gui.char_width*0.2),
			(int)(Y + gui.char_height*0.2), &point);
		LineTo(hdc, (int)(X+(pszTemp-lpString)*gui.char_width
			    + gui.char_width*0.8),
			(int)(Y + gui.char_height*0.75));
		pszTemp++;
		cbCount--;
		i = 0;
	    }
	    else
	    {
		pszTemp++;
		cbCount--;
		i++;
	    }
	}
	if (i > 0)
	{
	    int OldBkMode = 0;

	    if (bOpaque)
	    {
		OldBkMode = GetBkMode(hdc);
		SetBkMode(hdc, OPAQUE);
	    }
	    ExtTextOut(hdc, X+((pszTemp-i)-lpString)*gui.char_width, Y,
		    fuOption, lprc, pszTemp-i, i, lpDx);
	    if (bOpaque)
		SetBkMode(hdc, OldBkMode);
	}

	DeleteObject(SelectObject(hdc, old_pen));
    }
    else
	ExtTextOut(hdc, X, Y, fuOption, lprc, lpString, cbCount, lpDx);
}

#include <ime.h>
#include <imm.h>

static BOOL bImeOpenStatus = FALSE;
static char lpCompStr[100];		// Pointer to composition str.
static BOOL bInComposition=FALSE;
static BOOL bCommandMode=TRUE;
static BOOL bImeNative = FALSE;

/*
 * display composition string(korean)
 */
    static void
DisplayCompStringOpaque(char_u *s, int len)
{
    int OldBkMode = GetBkMode(s_hdc);

    SetBkMode(s_hdc, OPAQUE);
    gui_outstr_nowrap(s, len, GUI_MON_TRS_CURSOR, (guicolor_t)0, (guicolor_t)0, 0);
    SetBkMode(s_hdc, OldBkMode);
}

/*
 * Position IME composition window.
 */
    static void
ImePositionWindow(HIMC hImc)
{
    COMPOSITIONFORM	cfs;

    cfs.dwStyle = CFS_POINT;
    cfs.ptCurrentPos.x = FILL_X(gui.col);
    cfs.ptCurrentPos.y = FILL_Y(gui.row);
    MapWindowPoints(s_textArea, s_hwnd, &cfs.ptCurrentPos, 1);
    ImmSetCompositionWindow(hImc, &cfs);
}

/*
 * handle WM_IME_NOTIFY message
 */
static BOOL bImeStatus = FALSE;

    static LRESULT
_OnImeNotify(HWND hWnd, DWORD dwCommand, DWORD dwData)
{
    LRESULT lResult = 0;
    HIMC hImc;

    if (!(hImc = ImmGetContext(hWnd)))
	return lResult;
    switch (dwCommand)
    {
	case IMN_SETOPENSTATUS:
	    if (bImeStatus = ImmGetOpenStatus(hImc))
	    {
		ImmSetCompositionFont(hImc, &norm_logfont);
		ImePositionWindow(hImc);
	    }
	    gui_update_cursor(TRUE, FALSE);
	    lResult = 1;
	    break;
    }
    ImmReleaseContext(hWnd, hImc);
    return lResult;
}

/*
 * When status IME is 'open' change cursor color to hilight 'CursorIM'
 */
    guicolor_t
ImeGetCursorColor(guicolor_t color)
{
    static int id = 0;
    guicolor_t fg, bg;

    if (bImeStatus)
    {
	if (!id)
	    id = syn_name2id("CursorIM");
	if (id)
	{
	    syn_id2colors(id, &fg, &bg);
	    if (bg)
		color = bg - 1;
	}
    }
    return color;
}

/*
 * Get context and position IME composition window.
 */
    void
ImeSetCompositionWindow(void)
{
    HIMC hImc;

    if ((hImc = ImmGetContext(s_hwnd)))
    {
	ImePositionWindow(hImc);
	ImmReleaseContext(s_hwnd, hImc);
    }
}


/*
 * When entering Insert mode, set IME to previous language mode
 */
    void
ImeSetOriginMode(void)
{
    HIMC    hImc;

    if ((hImc = ImmGetContext(s_hwnd)))
    {
	if (!ImmGetOpenStatus(hImc) && bImeOpenStatus == TRUE)
	    ImmSetOpenStatus(hImc, TRUE);
	else
	    bImeOpenStatus = FALSE;
	ImmReleaseContext(s_hwnd, hImc);
    }
    bCommandMode = FALSE;
}


/* When enter to command mode, set IME to english mode */
    void
ImeSetEnglishMode(void)
{
    HIMC    hImc;
    DWORD   dwConvMode, dwSentMode;

    if ((hImc = ImmGetContext(s_hwnd)))
    {
	ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
	if (ImmGetOpenStatus(hImc))
	{
	    ImmSetOpenStatus(hImc, FALSE);
	    bImeOpenStatus = TRUE;
	}
	else
	    bImeOpenStatus = FALSE;
	ImmReleaseContext(s_hwnd, hImc);
    }
    bCommandMode = TRUE;
}

/* get composition string from WIN_IME */
    static void
GetCompositionStr(HWND hwnd, LPARAM CompFlag)
{
    DWORD	dwBufLen;		// Stogare for len. of composition str
    HIMC	hIMC;			// Input context handle.

    // If fail to get input context handle then do nothing.
    // Applications should call ImmGetContext API to get
    // input context handle.

    if (!(hIMC = ImmGetContext(hwnd)))
	return;

    // Determines how much memory space to store the composition string.
    // Applications should call ImmGetCompositionString with
    // GCS_COMPSTR flag on, buffer length zero, to get the bullfer
    // length.

    if ((dwBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR,
		    (void FAR*)NULL, 0l)) < 0)
	goto exit2;

    if (dwBufLen > 99)
	goto exit2;

    // Reads in the composition string.
    if ( dwBufLen != 0 )
    {
	ImmGetCompositionString(hIMC, GCS_COMPSTR, lpCompStr, dwBufLen);
	lpCompStr[dwBufLen] = 0;
    }
    else
    {
	strcpy(lpCompStr, "  ");
	dwBufLen = 2;
    }

    // Display new composition chars.
    DisplayCompStringOpaque(lpCompStr, dwBufLen);


exit2:
    ImmReleaseContext(hwnd, hIMC);
}


// void GetResultStr()
//
// This handles WM_IME_COMPOSITION with GCS_RESULTSTR flag on.
//
// get complete composition string

    static void
GetResultStr(HWND hwnd)
{
    DWORD	dwBufLen;		// Storage for length of result str.
    HIMC	hIMC;			// Input context handle.

    // If fail to get input context handle then do nothing.
    if (!(hIMC = ImmGetContext(hwnd)))
	return;

    // Determines how much memory space to store the result string.
    // Applications should call ImmGetCompositionString with
    // GCS_RESULTSTR flag on, buffer length zero, to get the bullfer
    // length.
    if ((dwBufLen = ImmGetCompositionString( hIMC, GCS_RESULTSTR,
		    (void FAR *)NULL, (DWORD) 0)) <= 0)
	goto exit2;

    if (dwBufLen > 99)
	goto exit2;

    // Reads in the result string.
    ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpCompStr, dwBufLen);

    // Displays the result string.
    DisplayCompStringOpaque(lpCompStr, dwBufLen);

exit2:
    ImmReleaseContext(hwnd, hIMC);
}

#if 0
/* this handles WM_IME_STARTCOMPOSITION */
    static void
ImeUIStartComposition(HWND hwnd)
{
     bInComposition = TRUE;
     //GetResultStr( hwnd );
}

/* WM_IME_COMPOSITION */
    static void
ImeUIComposition(HWND hwnd, WPARAM wParam, LPARAM CompFlag)
{

    if (CompFlag & GCS_RESULTSTR)
	GetResultStr( hwnd );
    else if (CompFlag & GCS_COMPSTR)
	GetCompositionStr( hwnd, CompFlag );
}

/* WM_IME_COMPOSITION */
    static void
ImeUIEndComposition(HWND hwnd)
{
    bInComposition = FALSE;
    //GetResultStr( hwnd );
}
#endif

    static char *
ImeGetTempComposition(void)
{
    if (bInComposition == TRUE /* && bCommandMode == FALSE */)
    {
	HIMC    hImc;
	DWORD   dwConvMode, dwSentMode;

	if ((hImc = ImmGetContext(s_hwnd)))
	{
	    ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
	    ImmReleaseContext(s_hwnd, hImc);
	    if ((dwConvMode & IME_CMODE_NATIVE))
		return lpCompStr;
	}
    }
    return NULL;
}

#if 0
    static void
ImeNotify(WPARAM w, LPARAM l)
{
    HIMC    hImc;
    DWORD   dwConvMode, dwSentMode;

    if ((hImc = ImmGetContext(s_hwnd)))
    {
	ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
	if (dwConvMode & IME_CMODE_NATIVE)
	{
	    if (w = IMN_SETOPENSTATUS)
	    {
		ImmSetCompositionFont(hImc, &norm_logfont);
		ImePositionWindow(hImc);
	    }
	    bImeNative = TRUE;
	}
	else
	    bImeNative = FALSE;
	ImmReleaseContext(s_hwnd, hImc);
    }
}
#endif

#  if 0 // This is not used !?
    void
ImeOpenClose(HWND hWnd, BOOL fFlag)
{
    HIMC	hIMC;

    //
    // If fFlag is true then open IME; otherwise close it.
    //

    if (!(hIMC = ImmGetContext(hWnd)))
	return;

    ImmSetOpenStatus(hIMC, fFlag);

    ImmReleaseContext(hWnd, hIMC);
}

/*
*   IsDBCSTrailByte - returns TRUE if the given byte is a DBCS trail byte
*
*		The algorithm searchs backward in the string, to some known
*		character boundary, counting consecutive bytes in the lead
*		byte range. An odd number indicates the current byte is part
*		of a two byte character code.
*
*   INPUT: PCHAR  - pointer to a preceding known character boundary.
*	   PCHAR  - pointer to the character to test.
*
*   OUTPUT:BOOL   - indicating truth of p==trailbyte.
*
*/
    int
IsDBCSTrailByte(char *base, char *p)
{
    int lbc = 0;    // lead byte count

    if (base > p)
	return 0;
    if (strlen(base) <= (size_t)(p-base))
	return 0;

    while (p > base)
    {
	if (!IsLeadByte(*(--p)))
	    break;
	lbc++;
    }

    return (lbc & 1);
}
#  endif /* not used */

#endif /* FEAT_MBYTE && FEAT_MBYTE_IME */

/* GIME_TEST */
#if !defined(FEAT_MBYTE_IME) && defined(GLOBAL_IME)
    void
ImeSetEnglishMode(void)
{
    global_ime_status_evacuate();
}

    void
ImeSetOriginMode(void)
{
    global_ime_status_restore();
}
#endif
/* GIME_TEST */

#ifdef FEAT_RIGHTLEFT
/*
 * What is this for?  In the case where you are using Win98 or Win2K or later,
 * and you are using a Hebrew font (or Arabic!), Windows does you a favor and
 * reverses the string sent to the TextOut... family.  This sucks, because we
 * go to a lot of effort to do the right thing, and there doesn't seem to be a
 * way to tell Windblows not to do this!
 *
 * The short of it is that this 'RevOut' only gets called if you are running
 * one of the new, "improved" MS OSes, and only if you are running in
 * 'rightleft' mode.  It makes display take *slightly* longer, but not
 * noticeably so.
 */
    static void
RevOut( HDC s_hdc,
	int col,
	int row,
	UINT foptions,
	CONST RECT *pcliprect,
	LPCTSTR text,
	UINT len,
	CONST INT *padding)
{
    int		ix;
    static int	special = -1;

    if (special == -1)
    {
	/* Check windows version: special treatment is needed if it is NT 5 or
	 * Win98 or higher. */
	if  ((os_version.dwPlatformId == VER_PLATFORM_WIN32_NT
		    && os_version.dwMajorVersion >= 5)
		|| (os_version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		    && (os_version.dwMajorVersion > 4
			|| (os_version.dwMajorVersion == 4
			    && os_version.dwMinorVersion > 0))))
	    special = 1;
	else
	    special = 0;
    }

    if (special)
	for (ix = 0; ix < (int)len; ++ix)
	    ExtTextOut(s_hdc, col + TEXT_X(ix), row, foptions,
					    pcliprect, text + ix, 1, padding);
    else
	ExtTextOut(s_hdc, col, row, foptions, pcliprect, text, len, padding);
}
#endif

    void
gui_mch_draw_string(
    int		row,
    int		col,
    char_u	*text,
    int		len,
    int		flags)
{
    static int	*padding = NULL;
    static int	pad_size = 0;
    int		i;
    const RECT	*pcliprect = NULL;
    UINT	foptions = 0;
#ifdef FEAT_MBYTE
    static WCHAR *unicodebuf = NULL;
    int		unibuflen = 0;
    int		n;
#endif
    HPEN	hpen, old_pen;
    int		y;

#if 1
    /*
     * Italic and bold text seems to have an extra row of pixels at the bottom
     * (below where the bottom of the character should be).  If we draw the
     * characters with a solid background, the top row of pixels in the
     * character below will be overwritten.  We can fix this by filling in the
     * background ourselves, to the correct character proportions, and then
     * writing the character in transparent mode.  Still have a problem when
     * the character is "_", which gets written on to the character below.
     * New fix: set gui.char_ascent to -1.  This shifts all characters up one
     * pixel in their slots, which fixes the problem with the bottom row of
     * pixels.	We still need this code because otherwise the top row of pixels
     * becomes a problem. - webb.
     */
    HBRUSH	hbr;
    RECT	rc;
#ifdef FEAT_MBYTE_IME
    char	*szComp;
#endif

    if (!(flags & DRAW_TRANSP))
    {
	/*
	 * Clear background first.
	 * Note: FillRect() excludes right and bottom of rectangle.
	 */
	rc.left = FILL_X(col);
	rc.top = FILL_Y(row);
#ifdef FEAT_MBYTE
	if (has_mbyte)
	{
	    int cell_len = 0;

	    /* Compute the length in display cells. */
	    for (n = 0; n < len; n += MB_BYTE2LEN(text[n]))
		cell_len += (*mb_ptr2cells)(text + n);
	    rc.right = FILL_X(col + cell_len);
	}
	else
#endif
	    rc.right = FILL_X(col + len);
	rc.bottom = FILL_Y(row + 1);
	hbr = CreateSolidBrush(gui.currBgColor);
	FillRect(s_hdc, &rc, hbr);
	DeleteBrush(hbr);

	SetBkMode(s_hdc, TRANSPARENT);

	/*
	 * When drawing block cursor, prevent inverted character spilling
	 * over character cell (can happen with bold/italic)
	 */
	if (flags & DRAW_CURSOR)
	{
	    pcliprect = &rc;
	    foptions = ETO_CLIPPED;
	}
    }
#else
    /*
     * The alternative would be to write the characters in opaque mode, but
     * when the text is not exactly the same proportions as normal text, too
     * big or too little a rectangle gets drawn for the background.
     */
    SetBkMode(s_hdc, OPAQUE);
    SetBkColor(s_hdc, gui.currBgColor);
#endif
    SetTextColor(s_hdc, gui.currFgColor);
    SelectFont(s_hdc, gui.currFont);

    if (pad_size != Columns || padding == NULL || padding[0] != gui.char_width)
    {
	vim_free(padding);
	pad_size = Columns;

	padding = (int *)alloc(pad_size * sizeof(int));
	if (padding != NULL)
	    for (i = 0; i < pad_size; i++)
		padding[i] = gui.char_width;
    }

    /*
     * We have to provide the padding argument because italic and bold versions
     * of fixed-width fonts are often one pixel or so wider than their normal
     * versions.
     * No check for DRAW_BOLD, Windows will have done it already.
     */
#ifdef FEAT_MBYTE_IME
    if (enc_dbcs != 0)
    {
	/* draw an incomplete composition character (korean) */
	if (len == 1 && blink_state == BLINK_ON
		&& (szComp = ImeGetTempComposition()) != NULL) // hangul
	    HanExtTextOut(s_hdc, TEXT_X(col), TEXT_Y(row),
			       foptions, pcliprect, szComp, 2, padding, TRUE);
	else
	    HanExtTextOut(s_hdc, TEXT_X(col), TEXT_Y(row),
		      foptions, pcliprect, (char *)text, len, padding, FALSE);
    }
    else
#endif
    {
#ifdef FEAT_MBYTE
	/* Check if there are any UTF-8 characters.  If not, use normal text
	 * output to speed up output. */
	if (enc_utf8)
	    for (n = 0; n < len; ++n)
		if (text[n] >= 0x80)
		    break;

	/* Check if the Unicode buffer exists and is big enough.  Create it
	 * with the same lengt as the multi-byte string, the number of wide
	 * characters is always equal or smaller. */
	if ((enc_utf8 || is_funky_dbcs)
		&& (unicodebuf == NULL || len > unibuflen))
	{
	    vim_free(unicodebuf);
	    unicodebuf = (WCHAR *)alloc(len * sizeof(WCHAR));
	    unibuflen = len;
	}

	if (enc_utf8 && n < len && unicodebuf != NULL)
	{
	    /* Output UTF-8 characters up to a composing character */
	    /* Composing characters overwrite the preceding char. */
	    int		c1, c2;
	    int		coloff = 0;
	    int		i = 0;
	    int		clen;	/* string length up to composing char */
	    int		cells;	/* cell width of string up to composing char */
	    int		cw;	/* width of last cell */

	    while (i < len)
	    {
		cells = 0;
		for (clen = 0; i < len; ++clen)
		{
		    unicodebuf[clen] = utfc_ptr2char(text + i, &c1, &c2);
		    cw = utf_char2cells(unicodebuf[clen]);
		    cells += cw;
		    i += utfc_ptr2len_check(text + i);
		    if (c1 != 0)
			break;	    /* break at composing char */
		}
		ExtTextOutW(s_hdc, TEXT_X(col + coloff), TEXT_Y(row),
				 foptions, pcliprect, unicodebuf, clen, NULL);
		coloff += cells;
		while (c1 != 0)
		{
		    /* overwrite the last character with composing char(s) */
		    unicodebuf[0] = c1;
		    ExtTextOutW(s_hdc, TEXT_X(col + coloff - cw), TEXT_Y(row),
				    foptions, pcliprect, unicodebuf, 1, NULL);
		    c1 = c2;
		    c2 = 0;
		}
	    }
	    len = coloff;	/* used for underlining */
	}
	else if (is_funky_dbcs)
	{
	    /* If we want to display DBCS, and the current CP is not the DBCS
	     * one, we need to go via Unicode. */
	    if (unicodebuf != NULL)
	    {
		if ((len = MultiByteToWideChar(enc_dbcs,
			    MB_PRECOMPOSED,
			    (char *)text, len,
			    (LPWSTR)unicodebuf, unibuflen)))
		    ExtTextOutW(s_hdc, TEXT_X(col), TEXT_Y(row),
				  foptions, pcliprect, unicodebuf, len, NULL);
	    }
	}
	else
#endif
	{
#ifdef FEAT_RIGHTLEFT
	    /* ron: fixed Hebrew on Win98/Win2000 */
	    if (curwin->w_p_rl)
		RevOut(s_hdc, TEXT_X(col), TEXT_Y(row),
			     foptions, pcliprect, (char *)text, len, padding);
	    else
#endif
		ExtTextOut(s_hdc, TEXT_X(col), TEXT_Y(row),
			     foptions, pcliprect, (char *)text, len, padding);
	}
    }

    if (flags & DRAW_UNDERL)
    {
	hpen = CreatePen(PS_SOLID, 1, gui.currFgColor);
	old_pen = SelectObject(s_hdc, hpen);
	/* When p_linespace is 0, overwrite the bottom row of pixels.
	 * Otherwise put the line just below the character. */
	y = FILL_Y(row + 1) - 1;
	if (p_linespace > 1)
	    y -= p_linespace - 1;
	MoveToEx(s_hdc, FILL_X(col), y, NULL);
	/* Note: LineTo() excludes the last pixel in the line. */
	LineTo(s_hdc, FILL_X(col + len), y);
	DeleteObject(SelectObject(s_hdc, old_pen));
    }
}


    void
gui_mch_flash(int msec)
{
    RECT    rc;

    /*
     * Note: InvertRect() excludes right and bottom of rectangle.
     */
    rc.left = 0;
    rc.top = 0;
    rc.right = gui.num_cols * gui.char_width;
    rc.bottom = gui.num_rows * gui.char_height;
    InvertRect(s_hdc, &rc);
    GdiFlush();			/* make sure it's displayed */

    ui_delay((long)msec, TRUE);	/* wait for a few msec */

    InvertRect(s_hdc, &rc);
}





/*
 * Output routines.
 */

/* Flush any output to the screen */
    void
gui_mch_flush(void)
{
    GdiFlush();
}

    static void
clear_rect(RECT *rcp)
{
    HBRUSH  hbr;

    hbr = CreateSolidBrush(gui.back_pixel);
    FillRect(s_hdc, rcp, hbr);
    DeleteBrush(hbr);
}


/*
 * Delete the given number of lines from the given row, scrolling up any
 * text further down within the scroll region.
 */
    void
gui_mch_delete_lines(
    int	    row,
    int	    num_lines)
{
    RECT	rc;

    rc.left = FILL_X(gui.scroll_region_left);
    rc.right = FILL_X(gui.scroll_region_right + 1);
    rc.top = FILL_Y(row);
    rc.bottom = FILL_Y(gui.scroll_region_bot + 1);
    /* The SW_INVALIDATE is required when part of the window is covered or
     * off-screen.  How do we avoid it when it's not needed? */
    ScrollWindowEx(s_textArea, 0, -num_lines * gui.char_height,
	    &rc, &rc, NULL, NULL, SW_INVALIDATE);

    /* Update gui.cursor_row if the cursor scrolled or copied over */
    if (gui.cursor_row >= row
	    && gui.cursor_col >= gui.scroll_region_left
	    && gui.cursor_col <= gui.scroll_region_right)
    {
	if (gui.cursor_row < row + num_lines)
	    gui.cursor_is_valid = FALSE;
	else if (gui.cursor_row <= gui.scroll_region_bot)
	    gui.cursor_row -= num_lines;
    }
    gui_undraw_cursor();
    UpdateWindow(s_textArea);
    /* This seems to be required to avoid the cursor disappearing when
     * scrolling such that the cursor ends up in the top-left character on
     * the screen...   But why?  (Webb) */
    gui.cursor_is_valid = FALSE;

    gui_clear_block(gui.scroll_region_bot - num_lines + 1,
						       gui.scroll_region_left,
	gui.scroll_region_bot, gui.scroll_region_right);
}

/*
 * Insert the given number of lines before the given row, scrolling down any
 * following text within the scroll region.
 */
    void
gui_mch_insert_lines(
    int		row,
    int		num_lines)
{
    RECT	rc;

    rc.left = FILL_X(gui.scroll_region_left);
    rc.right = FILL_X(gui.scroll_region_right + 1);
    rc.top = FILL_Y(row);
    rc.bottom = FILL_Y(gui.scroll_region_bot + 1);
    /* The SW_INVALIDATE is required when part of the window is covered or
     * off-screen.  How do we avoid it when it's not needed? */
    ScrollWindowEx(s_textArea, 0, num_lines * gui.char_height,
	    &rc, &rc, NULL, NULL, SW_INVALIDATE);

    /* Update gui.cursor_row if the cursor scrolled or copied over */
    if (gui.cursor_row >= gui.row
	    && gui.cursor_col >= gui.scroll_region_left
	    && gui.cursor_col <= gui.scroll_region_right)
    {
	if (gui.cursor_row <= gui.scroll_region_bot - num_lines)
	    gui.cursor_row += num_lines;
	else if (gui.cursor_row <= gui.scroll_region_bot)
	    gui.cursor_is_valid = FALSE;
    }

    gui_undraw_cursor();
    UpdateWindow(s_textArea);

    gui_clear_block(row, gui.scroll_region_left,
				row + num_lines - 1, gui.scroll_region_right);
}



#if defined(FEAT_MENU) || defined(PROTO)
/*
 * Add a sub menu to the menu bar.
 */
    void
gui_mch_add_menu(
    vimmenu_t	*menu,
    int		pos)
{
    vimmenu_t	*parent = menu->parent;

    menu->submenu_id = CreatePopupMenu();
    menu->id = s_menu_id++;

    if (menu_is_menubar(menu->name))
    {
	if (is_winnt_3())
	{
	    InsertMenu((parent == NULL) ? s_menuBar : parent->submenu_id,
		    (UINT)pos, MF_POPUP | MF_STRING | MF_BYPOSITION,
		    (UINT)menu->submenu_id, (LPCTSTR) menu->name);
	}
	else
	{
	    MENUITEMINFO	info;

	    info.cbSize = sizeof(info);
	    info.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
	    info.dwItemData = (DWORD)menu;
	    info.wID = menu->id;
	    info.fType = MFT_STRING;
	    info.dwTypeData = (LPTSTR)menu->name;
	    info.cch = STRLEN(menu->name);
	    info.hSubMenu = menu->submenu_id;
	    InsertMenuItem((parent == NULL) ? s_menuBar : parent->submenu_id,
		    (UINT)pos, TRUE, &info);
	}
    }

    /* Fix window size if menu may have wrapped */
    if (parent == NULL)
	gui_w32_get_menu_height(!gui.starting);
    else if (IsWindow(parent->tearoff_handle))
	rebuild_tearoff(parent);
}

    void
gui_mch_show_popupmenu(vimmenu_t *menu)
{
    POINT mp;

    if (GetCursorPos((LPPOINT)&mp))
    {
	gui_mch_show_popupmenu_at(menu, (int)mp.x, (int)mp.y);
    }
}

    void
gui_make_popup(char_u *path_name)
{
    vimmenu_t	*menu = gui_find_menu(path_name);

    if (menu != NULL)
    {
	POINT	p;

	/* Find the position of the current cursor */
	GetDCOrgEx(s_hdc,&p);
	if (curwin != NULL)
	{
	    p.x+= TEXT_X(W_WINCOL(curwin) + curwin->w_wcol +1);
	    p.y+= TEXT_Y(W_WINROW(curwin) + curwin->w_wrow +1);
	}
	msg_scroll = FALSE;
	gui_mch_show_popupmenu_at(menu, (int)p.x, (int)p.y);
    }
}

/*
 * Given a menu descriptor, e.g. "File.New", find it in the menu hierarchy and
 * create it as a pseudo-"tearoff menu".
 */
    void
gui_make_tearoff(char_u *path_name)
{
    vimmenu_t	*menu = gui_find_menu(path_name);

    /* Found the menu, so tear it off. */
    if (menu != NULL)
	gui_mch_tearoff(menu->dname, menu, 0xffffL, 0xffffL);
}

/*
 * Add a menu item to a menu
 */
    void
gui_mch_add_menu_item(
    vimmenu_t	*menu,
    int		idx)
{
    vimmenu_t	*parent = menu->parent;

    menu->id = s_menu_id++;
    menu->submenu_id = NULL;

    if (STRNCMP(menu->name, TEAR_STRING, TEAR_LEN) == 0)
    {
	InsertMenu(parent->submenu_id, (UINT)idx, MF_BITMAP|MF_BYPOSITION,
		(UINT)menu->id, (LPCTSTR) s_htearbitmap);
    }
    else
#ifdef FEAT_TOOLBAR
    if (menu_is_toolbar(parent->name))
    {
	TBBUTTON newtb;

	vim_memset(&newtb, 0, sizeof(newtb));
	if (menu_is_separator(menu->name))
	{
	    newtb.iBitmap = 0;
	    newtb.fsStyle = TBSTYLE_SEP;
	}
	else
	{
	    newtb.iBitmap = get_toolbar_bitmap(menu->name);
	    newtb.fsStyle = TBSTYLE_BUTTON;
	}
	newtb.idCommand = menu->id;
	newtb.fsState = TBSTATE_ENABLED;
	newtb.iString = 0;
	SendMessage(s_toolbarhwnd, TB_INSERTBUTTON, (WPARAM)idx,
							     (LPARAM)&newtb);
	menu->submenu_id = (HMENU)-1;
    }
    else
#endif
    {
	InsertMenu(parent->submenu_id, (UINT)idx,
		(menu_is_separator(menu->name) ? MF_SEPARATOR : MF_STRING)
							      | MF_BYPOSITION,
		(UINT)menu->id, (LPCTSTR)menu->name);
	if (IsWindow(parent->tearoff_handle))
	    rebuild_tearoff(parent);
    }
}

/*
 * Destroy the machine specific menu widget.
 */
    void
gui_mch_destroy_menu(vimmenu_t *menu)
{
#ifdef FEAT_TOOLBAR
    /*
     * is this a toolbar button?
     */
    if (menu->submenu_id == (HMENU)-1)
    {
	int iButton;

	iButton = SendMessage(s_toolbarhwnd, TB_COMMANDTOINDEX,
							 (WPARAM)menu->id, 0);
	SendMessage(s_toolbarhwnd, TB_DELETEBUTTON, (WPARAM)iButton, 0);
    }
    else
#endif
    {
	if (menu->parent != NULL
		&& menu_is_popup(menu->parent->dname)
		&& menu->parent->submenu_id != NULL)
	    RemoveMenu(menu->parent->submenu_id, menu->id, MF_BYCOMMAND);
	else
	    RemoveMenu(s_menuBar, menu->id, MF_BYCOMMAND);
	if (menu->submenu_id != NULL)
	    DestroyMenu(menu->submenu_id);
	if (IsWindow(menu->tearoff_handle))
	    DestroyWindow(menu->tearoff_handle);
	if (menu->parent != NULL
		&& menu->parent->children != NULL
		&& IsWindow(menu->parent->tearoff_handle))
	    rebuild_tearoff(menu->parent);
    }
}

    static void
rebuild_tearoff(vimmenu_t *menu)
{
    /*hackish*/
    char_u	tbuf[128];
    RECT	trect;
    RECT	rct;
    RECT	roct;
    int		x, y;

    HWND thwnd = menu->tearoff_handle;

    GetWindowText(thwnd, tbuf, 127);
    if (GetWindowRect(thwnd, &trect)
	    && GetWindowRect(s_hwnd, &rct)
	    && GetClientRect(s_hwnd, &roct))
    {
	x = trect.left - rct.left;
	y = (trect.top -  rct.bottom  + roct.bottom);
    }
    else
    {
	x = y = 0xffffL;
    }
    DestroyWindow(thwnd);
    if (menu->children != NULL)
    {
	gui_mch_tearoff(tbuf, menu, x, y);
	if (IsWindow(menu->tearoff_handle))
	    (void) SetWindowPos(menu->tearoff_handle,
				NULL,
				(int)trect.left,
				(int)trect.top,
				0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

/*
 * Make a menu either grey or not grey.
 */
    void
gui_mch_menu_grey(
    vimmenu_t	*menu,
    int		grey)
{
# ifdef FEAT_TOOLBAR
    /*
     * is this a toolbar button?
     */
    if (menu->submenu_id == (HMENU)-1)
    {
	SendMessage(s_toolbarhwnd, TB_ENABLEBUTTON,
	    (WPARAM)menu->id, (LPARAM) MAKELONG((grey ? FALSE : TRUE), 0) );
    }
    else
# endif
    if (grey)
	EnableMenuItem(s_menuBar, menu->id, MF_BYCOMMAND | MF_GRAYED);
    else
	EnableMenuItem(s_menuBar, menu->id, MF_BYCOMMAND | MF_ENABLED);

    if ((menu->parent != NULL) && (IsWindow(menu->parent->tearoff_handle)))
    {
	WORD menuID;
	HWND menuHandle;

	/*
	 * A tearoff button has changed state.
	 */
	if (menu->children == NULL)
	    menuID = (WORD)(menu->id);
	else
	    menuID = (WORD)((WORD)(menu->submenu_id) | (WORD)0x8000);
	menuHandle = GetDlgItem(menu->parent->tearoff_handle, menuID);
	if (menuHandle)
	    EnableWindow(menuHandle, !grey);

    }
}

#endif /* FEAT_MENU */


#ifdef FEAT_BROWSE

/*
 * Pop open a file browser and return the file selected, in allocated memory,
 * or NULL if Cancel is hit.
 *  saving  - TRUE if the file will be saved to, FALSE if it will be opened.
 *  title   - Title message for the file browser dialog.
 *  dflt    - Default name of file.
 *  ext     - Default extension to be added to files without extensions.
 *  initdir - directory in which to open the browser (NULL = current dir)
 *  filter  - Filter for matched files to choose from.
 *	Has a format like this:
 *	"C Files (*.c)\0*.c\0"
 *	"All Files\0*.*\0\0"
 *	If these two strings were concatenated, then a choice of two file
 *	filters will be selectable to the user.  Then only matching files will
 *	be shown in the browser.  If NULL, the default allows all files.
 *
 *	*NOTE* - the filter string must be terminated with TWO nulls.
 */
    char_u *
gui_mch_browse(
	int saving,
	char_u *title,
	char_u *dflt,
	char_u *ext,
	char_u *initdir,
	char_u *filter)
{
    OPENFILENAME    fileStruct;
    char_u	    fileBuf[MAX_PATH], *p;

    if (dflt == NULL)
	fileBuf[0] = '\0';
    else
    {
	STRNCPY(fileBuf, dflt, MAX_PATH - 1);
	fileBuf[MAX_PATH - 1] = NUL;
    }

    /*
     * The default filter. NOTE: should we perhaps put this in
     * feature.h?
     */
    if (filter == NULL)
	filter =
	    "All Files (*.*)\0*.*\0"
	    "C source (*.c, *.h)\0*.c;*.h\0"
	    "C++ source (*.cpp, *.hpp)\0*.cpp;*.hpp\0"
	    "VB code (*.bas, *.frm)\0*.bas;*.frm\0"
	    "Vim files (*.vim, _vimrc, _gvimrc)\0*.vim;_vimrc;_gvimrc\0\0";

    memset(&fileStruct, 0, sizeof(OPENFILENAME));
    fileStruct.lStructSize = sizeof(OPENFILENAME);
    fileStruct.lpstrFilter = filter;
    fileStruct.lpstrFile = fileBuf;
    fileStruct.nMaxFile = MAX_PATH;
    fileStruct.lpstrTitle = title;
    fileStruct.lpstrDefExt = ext;
    fileStruct.hwndOwner = s_hwnd;		/* main Vim window is owner*/
    /* has an initial dir been specified? */
    if (initdir != NULL && *initdir != NUL)
	fileStruct.lpstrInitialDir = initdir;

    /*
     * TODO: Allow selection of multiple files.  Needs another arg to this
     * function to ask for it, and need to use OFN_ALLOWMULTISELECT below.
     * Also, should we use OFN_FILEMUSTEXIST when opening?  Vim can edit on
     * files that don't exist yet, so I haven't put it in.  What about
     * OFN_PATHMUSTEXIST?
     * Don't use OFN_OVERWRITEPROMPT, Vim has its own ":confirm" dialog.
     */
    fileStruct.Flags = (OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY);
    if (saving)
    {
	if (!GetSaveFileName(&fileStruct))
	    return NULL;
    }
    else
    {
	if (!GetOpenFileName(&fileStruct))
	    return NULL;
    }

    /* Shorten the file name if possible */
    mch_dirname(IObuff, IOSIZE);
    p = shorten_fname(fileBuf, IObuff);
    if (p == NULL)
	p = fileBuf;
    return vim_strsave(p);
}

#endif /* FEAT_BROWSE */

#if defined(FEAT_GUI_DIALOG) || defined(PROTO)
/*
 * stuff for dialogs
 */

/*
 * The callback routine used by all the dialogs.  Very simple.  First,
 * acknowledges the INITDIALOG message so that Windows knows to do standard
 * dialog stuff (Return = default, Esc = cancel....) Second, if a button is
 * pressed, return that button's ID - IDCANCEL (2), which is the button's
 * number.
 */
    static LRESULT CALLBACK
dialog_callback(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_INITDIALOG)
    {
	CenterWindow(hwnd, GetWindow(hwnd, GW_OWNER));
	/* Set focus to the dialog.  Set the default button, if specified. */
	(void)SetFocus(hwnd);
	if (dialog_default_button > 0)
	    (void)SetFocus(GetDlgItem(hwnd, dialog_default_button + IDCANCEL));
	return FALSE;
    }

    if (message == WM_COMMAND)
    {
	int	button = LOWORD(wParam);

	/*
	 * Need to check for IDOK because if the user just hits Return to
	 * accept the default value, some reason this is what we get.
	 */
	if (button == IDOK)
	    EndDialog(hwnd, dialog_default_button);
	else
	    EndDialog(hwnd, button - IDCANCEL);
	return TRUE;
    }

    if ((message == WM_SYSCOMMAND) && (wParam == SC_CLOSE))
    {
	EndDialog(hwnd, 0);
	return TRUE;
    }
    return FALSE;
}

/*
 * Create a dialog dynamically from the parameter strings.
 * type		= type of dialog (question, alert, etc.)
 * title	= dialog title. may be NULL for default title.
 * message	= text to display. Dialog sizes to accommodate it.
 * buttons	= '\n' separated list of button captions, default first.
 * dfltbutton	= number of default button.
 *
 * This routine returns 1 if the first button is pressed,
 *			2 for the second, etc.
 *
 *			0 indicates Esc was pressed.
 *			-1 for unexpected error
 *
 * If stubbing out this fn, return 1.
 */

static const char_u *dlg_icons[] = /* must match names in resource file */
{
    "IDR_VIM",
    "IDR_VIM_ERROR",
    "IDR_VIM_ALERT",
    "IDR_VIM_INFO",
    "IDR_VIM_QUESTION"
};

#ifdef USE_SYSMENU_FONT
/*
 * Get Menu Font.
 * Return OK or FAIL.
 */
    static int
gui_w32_get_menu_font(LOGFONT *lf)
{
    NONCLIENTMETRICS nm;

    nm.cbSize = sizeof(NONCLIENTMETRICS);
    if (!SystemParametersInfo(
	    SPI_GETNONCLIENTMETRICS,
	    sizeof(NONCLIENTMETRICS),
	    &nm,
	    0))
	return FAIL;
    *lf = nm.lfMenuFont;
    return OK;
}
#endif

    int
gui_mch_dialog(
    int		 type,
    char_u	*title,
    char_u	*message,
    char_u	*buttons,
    int		 dfltbutton)
{
    WORD	*p, *pdlgtemplate, *pnumitems;
    int		numButtons;
    int		*buttonWidths, *buttonPositions;
    int		nchar, i;
    DWORD	lStyle;
    int		dlgwidth = 0;
    int		horizWidth;
    int		msgheight;
    char_u	*pstart;
    char_u	*pend;
    char_u	*tbuffer;
    RECT	rect;
    HWND	hwnd;
    HDC		hdc;
    HFONT	font, oldFont;
    TEXTMETRIC	fontInfo;
    int		fontHeight;
    int		textWidth, minButtonWidth, messageWidth;
    int		maxDialogWidth;
    int		vertical;
    int		dlgPaddingX;
    int		dlgPaddingY;
#ifdef USE_SYSMENU_FONT
    LOGFONT	lfSysmenu;
    int		use_lfSysmenu = FALSE;
#endif

#ifndef NO_CONSOLE
    /* Don't output anything in silent mode ("ex -s") */
    if (silent_mode)
	return dfltbutton;   /* return default option */
#endif

    /* If there is no window yet, open it. */
    if (s_hwnd == NULL && gui_mch_init() == FAIL)
	return dfltbutton;

    if ((type < 0) || (type > VIM_LAST_TYPE))
	type = 0;

    /* allocate some memory for dialog template */
    /* TODO should compute this really*/
    pdlgtemplate = p = (PWORD) LocalAlloc(LPTR, DLG_ALLOC_SIZE);

    if (p == NULL)
	return -1;

    /*
     * make a copy of 'buttons' to fiddle with it.  complier grizzles because
     * vim_strsave() doesn't take a const arg (why not?), so cast away the
     * const.
     */
    tbuffer = vim_strsave(buttons);
    if (tbuffer == NULL)
	return -1;

    --dfltbutton;   /* Change from one-based to zero-based */

    /* Count buttons */
    numButtons = 1;
    for (i = 0; tbuffer[i] != '\0'; i++)
    {
	if (tbuffer[i] == DLG_BUTTON_SEP)
	    numButtons++;
    }
    if (dfltbutton >= numButtons)
	dfltbutton = 0;

    /* Allocate array to hold the width of each button */
    buttonWidths = (int *) lalloc(numButtons * sizeof(int), TRUE);
    if (buttonWidths == NULL)
	return -1;

    /* Allocate array to hold the X position of each button */
    buttonPositions = (int *) lalloc(numButtons * sizeof(int), TRUE);
    if (buttonPositions == NULL)
	return -1;

    /*
     * Calculate how big the dialog must be.
     */
    hwnd = GetDesktopWindow();
    hdc = GetWindowDC(hwnd);
#ifdef USE_SYSMENU_FONT
    if (gui_w32_get_menu_font(&lfSysmenu) == OK)
    {
	font = CreateFontIndirect(&lfSysmenu);
	use_lfSysmenu = TRUE;
    }
    else
#endif
    font = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		      VARIABLE_PITCH , DLG_FONT_NAME);
    if (s_usenewlook)
    {
	oldFont = SelectFont(hdc, font);
	dlgPaddingX = DLG_PADDING_X;
	dlgPaddingY = DLG_PADDING_Y;
    }
    else
    {
	oldFont = SelectFont(hdc, GetStockObject(SYSTEM_FONT));
	dlgPaddingX = DLG_OLD_STYLE_PADDING_X;
	dlgPaddingY = DLG_OLD_STYLE_PADDING_Y;
    }
    GetTextMetrics(hdc, &fontInfo);
    fontHeight = fontInfo.tmHeight;

    /* Minimum width for horizontal button */
    minButtonWidth = GetTextWidth(hdc, "Cancel", 6);

    /* Maximum width of a dialog, if possible */
    GetWindowRect(s_hwnd, &rect);
    maxDialogWidth = rect.right - rect.left
		     - GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    if (maxDialogWidth < DLG_MIN_MAX_WIDTH)
	maxDialogWidth = DLG_MIN_MAX_WIDTH;

    /* Set dlgwidth to width of message */
    pstart = message;
    messageWidth = 0;
    msgheight = 0;
    do
    {
	pend = vim_strchr(pstart, DLG_BUTTON_SEP);
	if (pend == NULL)
	    pend = pstart + STRLEN(pstart);	/* Last line of message. */
	msgheight += fontHeight;
	textWidth = GetTextWidth(hdc, pstart, pend - pstart);
	if (textWidth > messageWidth)
	    messageWidth = textWidth;
	pstart = pend + 1;
    } while (*pend != NUL);
    dlgwidth = messageWidth;

    /* Add width of icon to dlgwidth, and some space */
    dlgwidth += DLG_ICON_WIDTH + 3 * dlgPaddingX;

    if (msgheight < DLG_ICON_HEIGHT)
	msgheight = DLG_ICON_HEIGHT;

    /*
     * Check button names.  A long one will make the dialog wider.
     */
    vertical = (vim_strchr(p_go, GO_VERTICAL) != NULL);
    if (!vertical)
    {
	// Place buttons horizontally if they fit.
	horizWidth = dlgPaddingX;
	pstart = tbuffer;
	i = 0;
	do
	{
	    pend = vim_strchr(pstart, DLG_BUTTON_SEP);
	    if (pend == NULL)
		pend = pstart + STRLEN(pstart);	// Last button name.
	    textWidth = GetTextWidth(hdc, pstart, pend - pstart);
	    if (textWidth < minButtonWidth)
		textWidth = minButtonWidth;
	    textWidth += dlgPaddingX;	    /* Padding within button */
	    buttonWidths[i] = textWidth;
	    buttonPositions[i++] = horizWidth;
	    horizWidth += textWidth + dlgPaddingX; /* Pad between buttons */
	    pstart = pend + 1;
	} while (*pend != NUL);

	if (horizWidth > maxDialogWidth)
	    vertical = TRUE;	// Too wide to fit on the screen.
	else if (horizWidth > dlgwidth)
	    dlgwidth = horizWidth;
    }

    if (vertical)
    {
	// Stack buttons vertically.
	pstart = tbuffer;
	do
	{
	    pend = vim_strchr(pstart, DLG_BUTTON_SEP);
	    if (pend == NULL)
		pend = pstart + STRLEN(pstart);	// Last button name.
	    textWidth = GetTextWidth(hdc, pstart, pend - pstart);
	    textWidth += dlgPaddingX;		/* Padding within button */
	    textWidth += DLG_VERT_PADDING_X * 2; /* Padding around button */
	    if (textWidth > dlgwidth)
		dlgwidth = textWidth;
	    pstart = pend + 1;
	} while (*pend != NUL);
    }

    if (dlgwidth < DLG_MIN_WIDTH)
	dlgwidth = DLG_MIN_WIDTH;	/* Don't allow a really thin dialog!*/

    /* start to fill in the dlgtemplate information.  addressing by WORDs */
    if (s_usenewlook)
	lStyle = DS_MODALFRAME | WS_CAPTION |DS_3DLOOK| WS_VISIBLE |DS_SETFONT;
    else
	lStyle = DS_MODALFRAME | WS_CAPTION |DS_3DLOOK| WS_VISIBLE;

    *p++ = LOWORD(lStyle);
    *p++ = HIWORD(lStyle);
    *p++ = 0;		// LOWORD (lExtendedStyle)
    *p++ = 0;		// HIWORD (lExtendedStyle)
    pnumitems = p;	/*save where the number of items must be stored*/
    *p++ = 0;		// NumberOfItems(will change later)
    *p++ = 10;		// x
    *p++ = 10;		// y
    *p++ = PixelToDialogX(dlgwidth);	// cx

    // Dialog height.
    if (vertical)
	*p++ = PixelToDialogY(msgheight + 2 * dlgPaddingY +
			      DLG_VERT_PADDING_Y + 2 * fontHeight * numButtons);
    else
	*p++ = PixelToDialogY(msgheight + 3 * dlgPaddingY + 2 * fontHeight);

    *p++ = 0;		// Menu
    *p++ = 0;		// Class

    /* copy the title of the dialog */
    nchar = nCopyAnsiToWideChar(p, (title ?
				    (LPSTR)title :
				    (LPSTR)("Vim "VIM_VERSION_MEDIUM)));
    p += nchar;

    if (s_usenewlook)
    {
	/* do the font, since DS_3DLOOK doesn't work properly */
#ifdef USE_SYSMENU_FONT
	if (use_lfSysmenu)
	{
	    /* point size */
	    *p++ = -MulDiv(lfSysmenu.lfHeight, 72,
		    GetDeviceCaps(hdc, LOGPIXELSY));
	    nchar = nCopyAnsiToWideChar(p, TEXT(lfSysmenu.lfFaceName));
	}
	else
#endif
	{
	    *p++ = DLG_FONT_POINT_SIZE;		// point size
	    nchar = nCopyAnsiToWideChar (p, TEXT(DLG_FONT_NAME));
	}
	p += nchar;
    }

    pstart = tbuffer;
    horizWidth = (dlgwidth - horizWidth) / 2;	/* Now it's X offset */
    for (i = 0; i < numButtons; i++)
    {
	/* get end of this button. */
	for (	pend = pstart;
		*pend && (*pend != DLG_BUTTON_SEP);
		pend++)
	    ;

	if (*pend)
	    *pend = '\0';

	/*
	 * NOTE:
	 * setting the BS_DEFPUSHBUTTON style doesn't work because Windows sets
	 * the focus to the first tab-able button and in so doing makes that
	 * the default!! Grrr.  Workaround: Make the default button the only
	 * one with WS_TABSTOP style. Means user can't tab between buttons, but
	 * he/she can use arrow keys.
	 */
	if (vertical)
	{
	    p = add_dialog_element(p,
		    BS_PUSHBUTTON | WS_TABSTOP,
		    PixelToDialogX(DLG_VERT_PADDING_X),
		    PixelToDialogY(msgheight + 2 * dlgPaddingY
				   + 2 * fontHeight * i),
		    PixelToDialogX(dlgwidth - 2 * DLG_VERT_PADDING_X),
		    (WORD)(PixelToDialogY(2 * fontHeight) - 1),
		    (WORD)(IDCANCEL + 1 + i), (WORD)0x0080, pstart);
	}
	else
	{
	    p = add_dialog_element(p,
		    BS_PUSHBUTTON | WS_TABSTOP,
		    PixelToDialogX(horizWidth + buttonPositions[i]),
		    PixelToDialogY(msgheight + 2 * dlgPaddingY),
		    PixelToDialogX(buttonWidths[i]),
		    (WORD)(PixelToDialogY(2 * fontHeight) - 1),
		    (WORD)(IDCANCEL + 1 + i), (WORD)0x0080, pstart);
	}
	pstart = pend + 1;	/*next button*/
    }
    *pnumitems += numButtons;

    /* Vim icon */
    p = add_dialog_element(p, SS_ICON,
	    PixelToDialogX(dlgPaddingX),
	    PixelToDialogY(dlgPaddingY),
	    PixelToDialogX(DLG_ICON_WIDTH),
	    PixelToDialogY(DLG_ICON_HEIGHT),
	    0, (WORD)0x0082,
	    dlg_icons[type]);

    /* Dialog message */
    p = add_dialog_element(p, SS_LEFT,
	    PixelToDialogX(2 * dlgPaddingX + DLG_ICON_WIDTH),
	    PixelToDialogY(dlgPaddingY),
	    (WORD)(PixelToDialogX(messageWidth) + 1),
	    PixelToDialogY(msgheight),
	    1, (WORD)0x0082, message);

    *pnumitems += 2;

    SelectFont(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(hwnd, hdc);

    /* Let the dialog_callback() function know which button to make default */
    dialog_default_button = dfltbutton + 1;	/* Back to 1-based for this */

    /* show the dialog box modally and get a return value */
    nchar = DialogBoxIndirect(
	    s_hinst,
	    (LPDLGTEMPLATE)pdlgtemplate,
	    s_hwnd,
	    (DLGPROC)dialog_callback);

    LocalFree(LocalHandle(pdlgtemplate));
    vim_free(tbuffer);
    vim_free(buttonWidths);
    vim_free(buttonPositions);

    return nchar;
}

#endif /* FEAT_GUI_DIALOG */
/*
 * Put a simple element (basic class) onto a dialog template in memory.
 * return a pointer to where the next item shoudl be added.
 *
 * parameters:
 *  lStyle = additional style flags
 *		(be careful, NT3.51 & Win32s will ignore the new ones)
 *  x,y = x & y positions IN DIALOG UNITS
 *  w,h = width and height IN DIALOG UNITS
 *  Id  = ID used in messages
 *  clss  = class ID, e.g 0x0080 for a button, 0x0082 for a static
 *  caption = usually text or resource name
 *
 *  TODO: use the length information noted here to enable the dialog creation
 *  routines to work out more exactly how much memory they need to alloc.
 */
    static PWORD
add_dialog_element(
    PWORD p,
    DWORD lStyle,
    WORD x,
    WORD y,
    WORD w,
    WORD h,
    WORD Id,
    WORD clss,
    const char *caption)
{
    int nchar;

    p = lpwAlign(p);	/* Align to dword boundary*/
    lStyle = lStyle | WS_VISIBLE | WS_CHILD;
    *p++ = LOWORD(lStyle);
    *p++ = HIWORD(lStyle);
    *p++ = 0;		// LOWORD (lExtendedStyle)
    *p++ = 0;		// HIWORD (lExtendedStyle)
    *p++ = x;
    *p++ = y;
    *p++ = w;
    *p++ = h;
    *p++ = Id;		//9 or 10 words in all

    *p++ = (WORD)0xffff;
    *p++ = clss;			//2 more here

    nchar = nCopyAnsiToWideChar(p, (LPSTR)caption); //strlen(caption)+1
    p += nchar;

    *p++ = 0;  // advance pointer over nExtraStuff WORD   - 2 more

    return p;	//total = 15+ (strlen(caption)) words
		//	   = 30 + 2(strlen(caption) bytes reqd
}


/*
 * Helper routine.  Take an input pointer, return closest pointer that is
 * aligned on a DWORD (4 byte) boundary.  Taken from the Win32SDK samples.
 */
    static LPWORD
lpwAlign(
    LPWORD lpIn)
{
    ULONG ul;

    ul = (ULONG)lpIn;
    ul += 3;
    ul >>= 2;
    ul <<= 2;
    return (LPWORD)ul;
}

/*
 * Helper routine.  Takes second parameter as Ansi string, copies it to first
 * parameter as wide character (16-bits / char) string, and returns integer
 * number of wide characters (words) in string (including the trailing wide
 * char NULL).  Partly taken from the Win32SDK samples.
 */
    static int
nCopyAnsiToWideChar(
    LPWORD lpWCStr,
    LPSTR lpAnsiIn)
{
    int nChar = 0;

#ifdef FEAT_MBYTE
    int len = lstrlen(lpAnsiIn) + 1;	/* include NUL character */
    int i;

    nChar = MultiByteToWideChar(
	    CP_OEMCP,
	    MB_PRECOMPOSED,
	    lpAnsiIn, len,
	    lpWCStr, len);
    for (i = 0; i < nChar; ++i)
	if (lpWCStr[i] == (WORD)'\t')	/* replace tabs with spaces */
	    lpWCStr[i] = (WORD)' ';
#else
    do
    {
	if (*lpAnsiIn == '\t')
	    *lpWCStr++ = (WORD)' ';
	else
	    *lpWCStr++ = (WORD)*lpAnsiIn;
	nChar++;
    } while (*lpAnsiIn++);
#endif

    return nChar;
}


/*
 * The callback function for all the modeless dialogs that make up the
 * "tearoff menus" Very simple - forward button presses (to fool Vim into
 * thinking its menus have been clicked), and go away when closed.
 */
    static LRESULT CALLBACK
tearoff_callback(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_INITDIALOG)
	return (TRUE);

    if (message == WM_COMMAND)
    {
	if ((WORD)(LOWORD(wParam)) & 0x8000)
	{
	    POINT   mp;
	    RECT    rect;
	    if (GetCursorPos(&mp) && GetWindowRect(hwnd, &rect));
	    {
		(void)TrackPopupMenu(
			 (HMENU)(LOWORD(wParam) ^ 0x8000),
			 TPM_LEFTALIGN | TPM_LEFTBUTTON,
			 (int)rect.right - 8,
			 (int)mp.y,
			 (int)0,	    /*reserved param*/
			 s_hwnd,
			 NULL);
		/*
		 * NOTE: The pop-up menu can eat the mouse up event.
		 * We deal with this in normal.c.
		 */
	    }
	}
	else
	    /* Pass on messages to the main Vim window */
	    PostMessage (s_hwnd, WM_COMMAND, LOWORD(wParam), 0);
	/*
	 * Give main window the focus back: this is so after
	 * choosing a tearoff button you can start typing again
	 * straight away.
	 */
	(void)SetFocus(s_hwnd);
	return TRUE;
    }
    if ((message == WM_SYSCOMMAND) && (wParam == SC_CLOSE))
    {
	DestroyWindow(hwnd);
	return TRUE;
    }

    /* When moved around, give main window the focus back. */
    if (message == WM_EXITSIZEMOVE)
	(void)SetActiveWindow(s_hwnd);

    return FALSE;
}



/*
 * Decide whether to use the "new look" (small, non-bold font) or the "old
 * look" (big, clanky font) for dialogs, and work out a few values for use
 * later accordingly.
 */
    static void
get_dialog_font_metrics(void)
{
    HDC		    hdc;
    HFONT	    hfontTools = 0;
    DWORD	    dlgFontSize;
    SIZE	    size;
#ifdef USE_SYSMENU_FONT
    LOGFONT	    lfSysmenu;
#endif

    s_usenewlook = FALSE;

    /*
     * For NT3.51 and Win32s, we stick with the old look
     * because it matches everything else.
     */
    if (!is_winnt_3())
    {
#ifdef USE_SYSMENU_FONT
	if (gui_w32_get_menu_font(&lfSysmenu) == OK)
	    hfontTools = CreateFontIndirect(&lfSysmenu);
	else
#endif
	hfontTools = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, VARIABLE_PITCH , DLG_FONT_NAME);

	if (hfontTools)
	{
	    hdc = GetDC (s_hwnd);
	    SelectObject (hdc, hfontTools);
	    /*
	     * GetTextMetrics() doesn't return the right value in
	     * tmAveCharWidth, so we have to figure out the dialog base units
	     * ourselves.
	     */
	    GetTextExtentPoint(hdc,
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
		    52, &size);
	    ReleaseDC (s_hwnd, hdc);

	    s_dlgfntwidth = (WORD)((size.cx / 26 + 1) / 2);
	    s_dlgfntheight = (WORD)size.cy;
	    s_usenewlook = TRUE;
	}
    }

    if (!s_usenewlook)
    {
	dlgFontSize = GetDialogBaseUnits();	/* fall back to big old system*/
	s_dlgfntwidth = LOWORD(dlgFontSize);
	s_dlgfntheight = HIWORD(dlgFontSize);
    }
}

#ifdef FEAT_MENU
/*
 * Create a pseudo-"tearoff menu" based on the child
 * items of a given menu pointer.
 */
    static void
gui_mch_tearoff(
    char_u	*title,
    vimmenu_t	*menu,
    int		initX,
    int		initY)
{
    WORD	*p, *pdlgtemplate, *pnumitems, *ptrueheight;
    int		nchar, textWidth, submenuWidth;
    DWORD	lStyle;
    DWORD	lExtendedStyle;
    WORD	dlgwidth;
    WORD	menuID;
    vimmenu_t	*pmenu;
    vimmenu_t	*the_menu = menu;
    HWND	hwnd;
    HDC		hdc;
    HFONT	font, oldFont;
    int		col, spaceWidth, len;
    int		columnWidths[2];
    char_u	*label, *text, *end, *acEnd = NULL;
    int		padding0, padding1, padding2 = 0;
    int		sepPadding=0;
#ifdef USE_SYSMENU_FONT
    LOGFONT	lfSysmenu;
    int		use_lfSysmenu = FALSE;
#endif

    /*
     * If this menu is already torn off, then don't
     * tear it off again, but move the existing tearoff
     * to the mouse position.
     */

    if (IsWindow(menu->tearoff_handle))
    {
	POINT mp;
	if (GetCursorPos((LPPOINT)&mp))
	{
	    SetWindowPos(menu->tearoff_handle, NULL, mp.x, mp.y, 0, 0,
		    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	}
	return;
    }

    /*
     * Otherwise, create a new tearoff
     */

    if (*title == MNU_HIDDEN_CHAR)
	title++;

    /* allocate some memory to play with  */
    /* TODO should compute this really    */
    pdlgtemplate = p = (PWORD)LocalAlloc(LPTR,  DLG_ALLOC_SIZE);
    if (p == NULL)
	return;

    hwnd = GetDesktopWindow();
    hdc = GetWindowDC(hwnd);
#ifdef USE_SYSMENU_FONT
    if (gui_w32_get_menu_font(&lfSysmenu) == OK)
    {
	font = CreateFontIndirect(&lfSysmenu);
	use_lfSysmenu = TRUE;
    }
    else
#endif
    font = CreateFont(-DLG_FONT_POINT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		      VARIABLE_PITCH , DLG_FONT_NAME);
    if (s_usenewlook)
	oldFont = SelectFont(hdc, font);
    else
	oldFont = SelectFont(hdc, GetStockObject(SYSTEM_FONT));
    /*
     * Calculate width of a single space.  Used for padding columns to the
     * right width.
     */
    spaceWidth = GetTextWidth(hdc, " ", 1);

    submenuWidth = 0;
    /* Figure out widths for each column. */
    for (col = 0; col < 2; col++)
    {
	columnWidths[col] = 0;
	for (pmenu = menu->children; pmenu != NULL; pmenu = pmenu->next)
	{
	    text = (col == 0) ? pmenu->dname : pmenu->actext;
	    if (pmenu->children != NULL)
		submenuWidth = TEAROFF_COLUMN_PADDING * spaceWidth;
	    if (text != NULL && *text != NUL)
	    {
		end = text + strlen(text);
		textWidth = GetTextWidth(hdc, text, end - text);
		if (textWidth > columnWidths[col])
		    columnWidths[col] = textWidth;
	    }
	}
    }
    if (columnWidths[1] == 0)
    {
	if (submenuWidth != 0)
	    columnWidths[0] += submenuWidth;
	else
	    columnWidths[0] += spaceWidth;
    }
    else
    {
	columnWidths[0] += TEAROFF_COLUMN_PADDING * spaceWidth;
	columnWidths[1] += submenuWidth;
    }
    /*
     * Now find the width of our 'menu'.
     */
    textWidth = 0;
    for (col = 0; col < 2; col++)
	textWidth += columnWidths[col];
    if (submenuWidth != 0)
    {
	submenuWidth = GetTextWidth(hdc, TEAROFF_SUBMENU_LABEL,
			      STRLEN(TEAROFF_SUBMENU_LABEL));
	textWidth += submenuWidth;
    }
    dlgwidth = GetTextWidth(hdc, title, strlen(title));
    if (textWidth > dlgwidth)
	dlgwidth = textWidth;
    dlgwidth += 2 * TEAROFF_PADDING_X + TEAROFF_BUTTON_PAD_X;

    /* W95 can't do thin dialogs, they look v. weird! */
    if (mch_windows95() && dlgwidth < TEAROFF_MIN_WIDTH)
	dlgwidth = TEAROFF_MIN_WIDTH;

    /* start to fill in the dlgtemplate information.  addressing by WORDs */
    if (s_usenewlook)
	lStyle = DS_MODALFRAME | WS_CAPTION| WS_SYSMENU |DS_SETFONT| WS_VISIBLE;
    else
	lStyle = DS_MODALFRAME | WS_CAPTION| WS_SYSMENU | WS_VISIBLE;

    lExtendedStyle = WS_EX_TOOLWINDOW|WS_EX_STATICEDGE;
    *p++ = LOWORD(lStyle);
    *p++ = HIWORD(lStyle);
    *p++ = LOWORD (lExtendedStyle);
    *p++ = HIWORD (lExtendedStyle);
    pnumitems = p;	/* save where the number of items must be stored */
    *p++ = 0;		// NumberOfItems(will change later)
    if (initX == 0xffffL)
	*p++ = PixelToDialogX(gui_mch_get_mouse_x()); // x
    else
	*p++ = PixelToDialogX(initX); // x
    if (initY == 0xffffL)
	*p++ = PixelToDialogY(gui_mch_get_mouse_y()); // y
    else
	*p++ = PixelToDialogY(initY); // y
    *p++ = PixelToDialogX(dlgwidth);    // cx
    ptrueheight =p;
    *p++ = 0;		// dialog height: changed later anyway
    *p++ = 0;		// Menu
    *p++ = 0;		// Class

    /* copy the title of the dialog */
    nchar = nCopyAnsiToWideChar(p, ((*title) ?
				    (LPSTR)title :
				    (LPSTR)("Vim "VIM_VERSION_MEDIUM)));
    p += nchar;

    if (s_usenewlook)
    {
	/* do the font, since DS_3DLOOK doesn't work properly */
#ifdef USE_SYSMENU_FONT
	if (use_lfSysmenu)
	{
	    /* point size */
	    *p++ = -MulDiv(lfSysmenu.lfHeight, 72,
		    GetDeviceCaps(hdc, LOGPIXELSY));
	    nchar = nCopyAnsiToWideChar(p, TEXT(lfSysmenu.lfFaceName));
	}
	else
#endif
	{
	    *p++ = DLG_FONT_POINT_SIZE;		// point size
	    nchar = nCopyAnsiToWideChar (p, TEXT(DLG_FONT_NAME));
	}
	p += nchar;
    }

    /* Don't include tearbar in tearoff menu */
    if (STRCMP(menu->children->name, TEAR_STRING) == 0)
	menu = menu->children->next;
    else
	menu = menu->children;

    for ( ; menu != NULL; menu = menu->next)
    {
	if (menu_is_separator(menu->dname))
	{
	    sepPadding += 3;
	    continue;
	}

	/* Figure out length of this menu label */
	len = STRLEN(menu->dname);
	end = menu->dname + STRLEN(menu->dname);
	padding0 = (columnWidths[0] - GetTextWidth(hdc, menu->dname,
		    end - menu->dname)) / spaceWidth;
	len += padding0;
	if (menu->actext != NULL)
	{
	    len += STRLEN(menu->actext);
	    acEnd = menu->actext + STRLEN(menu->actext);
	    textWidth = GetTextWidth(hdc, menu->actext, acEnd - menu->actext);
	}
	else
	    textWidth = 0;

	padding1 = (columnWidths[1] - textWidth) / spaceWidth;
	len += padding1;
	if (menu->children == NULL)
	{
	    padding2 = submenuWidth / spaceWidth;
	    len += padding2;
	    menuID = (WORD)(menu->id);
	}
	else
	{
	    len += STRLEN(TEAROFF_SUBMENU_LABEL);
	    menuID = (WORD)((WORD)(menu->submenu_id) | (WORD)0x8000);
	}

	/* Allocate menu label and fill it in */
	text = label = alloc((unsigned)len + 1);
	if (label == NULL)
	    break;
	STRNCPY(text, menu->dname, end - menu->dname);
	text += end - menu->dname;
	while (padding0-- > 0)
	    *text++ = ' ';
	if (menu->actext != NULL)
	{
	    STRNCPY(text, menu->actext, acEnd - menu->actext);
	    text += acEnd - menu->actext;
	}
	while (padding1-- > 0)
	    *text++ = ' ';
	if (menu->children != NULL)
	{
	    STRCPY(text, TEAROFF_SUBMENU_LABEL);
	    text += STRLEN(TEAROFF_SUBMENU_LABEL);
	}
	else
	{
	    while (padding2-- > 0)
		*text++ = ' ';
	}
	*text = NUL;
	*end = NUL;

	/*
	 * BS_LEFT will just be ignored on Win32s/NT3.5x - on
	 * W95/NT4 it makes the tear-off look more like a menu.
	 */
	    p = add_dialog_element(p,
		    BS_PUSHBUTTON|BS_LEFT,
		(WORD)PixelToDialogX(TEAROFF_PADDING_X),
		    (WORD)(sepPadding + 1 + 13 * (*pnumitems)),
		(WORD)PixelToDialogX(dlgwidth - 2 * TEAROFF_PADDING_X),
		(WORD)12,
		menuID, (WORD)0x0080, label);
	vim_free(label);
	    (*pnumitems)++;
    }

    *ptrueheight = (WORD)(sepPadding + 1 + 13 * (*pnumitems));


    /* show modelessly */
    the_menu->tearoff_handle = CreateDialogIndirect(
	    s_hinst,
	    (LPDLGTEMPLATE)pdlgtemplate,
	    s_hwnd,
	    (DLGPROC)tearoff_callback);

    LocalFree(LocalHandle(pdlgtemplate));
    SelectFont(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(hwnd, hdc);

    /*
     * Reassert ourselves as the active window.  This is so that after creating
     * a tearoff, the user doesn't have to click with the mouse just to start
     * typing agin!
     */
    (void)SetActiveWindow(s_hwnd);

    /* make sure the right buttons are enabled */
    force_menu_update = TRUE;
}
#endif
#if defined(FEAT_TOOLBAR) || defined(PROTO)
#include "gui_w32_rc.h"

/* This not defined in older SDKs */
# ifndef TBSTYLE_FLAT
#  define TBSTYLE_FLAT		0x0800
# endif

/*
 * Create the toolbar, initially unpopulated.
 *  (just like the menu, there are no defaults, it's all
 *  set up through menu.vim)
 */
    static void
initialise_toolbar(void)
{
    InitCommonControls();
    s_toolbarhwnd = CreateToolbarEx(
		    s_hwnd,
		    WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT,
		    4000,		//any old big number
		    31,			//number of images in inital bitmap
		    s_hinst,
		    IDR_TOOLBAR1,	// id of initial bitmap
		    NULL,
		    0,			// initial number of buttons
		    TOOLBAR_BUTTON_WIDTH, //api guide is wrong!
		    TOOLBAR_BUTTON_HEIGHT,
		    TOOLBAR_BUTTON_WIDTH,
		    TOOLBAR_BUTTON_HEIGHT,
		    sizeof(TBBUTTON)
		    );

    gui_mch_show_toolbar(vim_strchr(p_go, GO_TOOLBAR) != NULL);
}

    static int
get_toolbar_bitmap(char_u *name)
{
    int i = -1;

    if (STRNCMP(name, "BuiltIn", 7) == 0)
    {
	char *dummy;
	/*
	 * reference by index
	 */
	i = strtol(name + 7, &dummy, 0);
	return i;
    }
    /*
     * Check user bitmaps next
     */
    if (!is_winnt_3())
    {
	char_u *fname;
	char_u *ffname;
	HANDLE hbitmap;

	fname = alloc(_MAX_PATH);
	STRCPY(fname, "$VIM\\bitmaps\\");
	strcat(fname, name);
	strcat(fname, ".bmp");
	ffname = expand_env_save(fname);

	hbitmap = LoadImage(
		    NULL,
		    ffname,
		    IMAGE_BITMAP,
		    TOOLBAR_BUTTON_WIDTH,
		    TOOLBAR_BUTTON_HEIGHT,
		    LR_LOADFROMFILE |
		    LR_LOADMAP3DCOLORS
		);
	if (hbitmap != NULL)
	{
	    TBADDBITMAP tbAddBitmap;

	    tbAddBitmap.hInst = NULL;
	    tbAddBitmap.nID = (UINT)hbitmap;

	    i = SendMessage(s_toolbarhwnd, TB_ADDBITMAP,
			    (WPARAM)1, (LPARAM)&tbAddBitmap);
	    /* i will be set to -1 if it fails */

	}
	vim_free(fname);
	vim_free(ffname);
    }
    if (i != -1)
	return i;

    for (i = 0; BuiltInBitmaps[i]; i++)
    {
	if (STRCMP(name, BuiltInBitmaps[i]) == 0)
	    return i;
    }
    return i;
}
#endif


#if defined(FEAT_OLE) || defined(PROTO)
/*
 * Make the GUI window come to the foreground.
 */
    void
gui_mch_set_foreground(void)
{
    if (IsIconic(s_hwnd))
	 SendMessage(s_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    SetForegroundWindow(s_hwnd);
}
#endif

#if 0	    /* DISABLED - has been reported not to work */
#ifdef FEAT_MBYTE
/*
 * locale-aware TranslateMessage replacement
 *
 * lpMsg should be WM_KEYDOWN message (not checked)
 */
    static BOOL
LCTranslateMessage(CONST MSG *lpMsg)
{
    MSG		charMsg;
    BYTE	lpKeyState[256];	/* keyboard state array */
    WCHAR	pwszBuff[4];		/* translated key buffer.  usually
					   doesn't hold more than 2 chars */
    int		i, ccount;
    int		len;
    char_u	string[MB_MAXBYTES];

    /* This only works on NT. */
    if (os_version.dwPlatformId != VER_PLATFORM_WIN32_NT)
	return TranslateMessage(lpMsg);

    GetKeyboardState(lpKeyState);

    /*
     * If Ctrl or Left Alt key is depressed, call base processing.
     * NB: Right Alt key is used in some keyboard layouts
     * (it also sets the Ctrl key flag), so it is disregarded here.
     */
    if (!(lpKeyState[VK_RMENU] & 128)
	&& ((lpKeyState[VK_CONTROL] & 128) || (lpKeyState[VK_MENU] & 128)))
	return TranslateMessage(lpMsg);

    ccount = ToUnicode(lpMsg->wParam, (lpMsg->lParam >> 16) & 0xFF,
						  lpKeyState, pwszBuff, 4, 0);
    switch (ccount)
    {
	case -1:	/* dead key */
	    charMsg.hwnd = lpMsg->hwnd;
	    charMsg.lParam = lpMsg->lParam;
	    charMsg.message = WM_DEADCHAR;
	    charMsg.wParam = pwszBuff[0];
	    DispatchMessage(&charMsg);
	    break;

	case 0:		/* key does not have translation in current keymap */
	    return TranslateMessage(lpMsg);

	default:	/* one or more unicode chars returned */
	    for (i = 0; i < ccount; i++)
	    {
		len = (*mb_char2bytes)(pwszBuff[i], string);
		add_to_input_buf(string, len);
	    }
	    break;
    }
    return TRUE;
}
#endif
#endif
