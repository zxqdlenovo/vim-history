/* vi:set ts=8 sw=4 sts=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *			Photon GUI support by Julian Kinraid
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 *
 *
 * Clipboard support is in os_qnx.c
 * PhAttach() is called in os_qnx.c:qnx_init()
 */

#include "vim.h"

#if !defined(__QNX__)
/* Used when generating prototypes. */
#define PgColor_t	int
#define PhEvent_t	int
#define PhPoint_t	int
#define PtWidget_t	int
#define Pg_BLACK	0
#define PtCallbackF_t	int
#define PtCallbackInfo_t int
#define PhTile_t	int
#endif

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))
#define RGB(r,g,b) PgRGB(r,g,b)

#define EVENT_BUFFER_SIZE sizeof( PhEvent_t ) + 1000

/* Some defines for gui_mch_mousehide() */
#define MOUSE_HIDE		TRUE
#define MOUSE_SHOW		FALSE

/* Optional support for using a PtPanelGroup widget, needs work */
#undef USE_PANEL_GROUP

#ifdef USE_PANEL_GROUP
static char	*empty_title = "    ";
static char	**panel_titles = NULL;
static ushort_t	num_panels = 0;
static short pg_margin_left, pg_margin_right, pg_margin_top, pg_margin_bottom;
#endif

#define GUI_PH_MARGIN		4	/* Size of the bevel */

#define GUI_PH_MOUSE_TYPE		Ph_CURSOR_INSERT
static PgColor_t gui_ph_mouse_color =	Pg_BLACK;

static PhPoint_t    gui_ph_raw_offset;
static PtWidget_t   *gui_ph_timer_cursor;   /* handle cursor blinking */
static PtWidget_t   *gui_ph_timer_timeout;  /* used in gui_mch_wait_for_chars */
static short	    is_timeout;		    /* Has the timeout occured? */

/*
 * This is set inside the mouse callback for a right mouse
 * button click, and used for the popup menus
 */
static PhPoint_t    abs_mouse;

/* Try and avoid redraws while a resize is in progress */
static int is_ignore_draw = FALSE;

/* Used for converting to/from utf-8 and other charsets */
static struct PxTransCtrl *charset_translate;

/*
 * Cursor blink functions.
 *
 * This is a simple state machine:
 * BLINK_NONE	not blinking at all
 * BLINK_OFF	blinking, cursor is not shown
 * BLINK_ON	blinking, cursor is shown
 */
static enum {
    BLINK_NONE,
    BLINK_OFF,
    BLINK_ON
} blink_state = BLINK_NONE;

static long_u	blink_waittime	= 700;
static long_u	blink_ontime	= 400;
static long_u	blink_offtime	= 250;

static struct
{
    int	    key_sym;
    char_u  vim_code0;
    char_u  vim_code1;
} special_keys[] =
{
    {Pk_Up,	    'k', 'u'},
    {Pk_Down,	    'k', 'd'},
    {Pk_Left,	    'k', 'l'},
    {Pk_Right,	    'k', 'r'},

    {Pk_F1,	    'k', '1'},
    {Pk_F2,	    'k', '2'},
    {Pk_F3,	    'k', '3'},
    {Pk_F4,	    'k', '4'},
    {Pk_F5,	    'k', '5'},
    {Pk_F6,	    'k', '6'},
    {Pk_F7,	    'k', '7'},
    {Pk_F8,	    'k', '8'},
    {Pk_F9,	    'k', '9'},
    {Pk_F10,	    'k', ';'},

    {Pk_F11,	    'F', '1'},
    {Pk_F12,	    'F', '2'},
    {Pk_F13,	    'F', '3'},
    {Pk_F14,	    'F', '4'},
    {Pk_F15,	    'F', '5'},
    {Pk_F16,	    'F', '6'},
    {Pk_F17,	    'F', '7'},
    {Pk_F18,	    'F', '8'},
    {Pk_F19,	    'F', '9'},
    {Pk_F20,	    'F', 'A'},

    {Pk_F21,	    'F', 'B'},
    {Pk_F22,	    'F', 'C'},
    {Pk_F23,	    'F', 'D'},
    {Pk_F24,	    'F', 'E'},
    {Pk_F25,	    'F', 'F'},
    {Pk_F26,	    'F', 'G'},
    {Pk_F27,	    'F', 'H'},
    {Pk_F28,	    'F', 'I'},
    {Pk_F29,	    'F', 'J'},

    {Pk_F30,	    'F', 'K'},
    {Pk_F31,	    'F', 'L'},
    {Pk_F32,	    'F', 'M'},
    {Pk_F33,	    'F', 'O'},
    {Pk_F34,	    'F', 'P'},
    {Pk_F35,	    'F', 'Q'},

    {Pk_Help,	    '%', '1'},
    {Pk_BackSpace,  'k', 'b'},
    {Pk_Insert,	    'k', 'I'},
    {Pk_Delete,	    'k', 'D'},
    {Pk_Home,	    'k', 'h'},
    {Pk_End,	    '@', '7'},
    {Pk_Prior,	    'k', 'P'},
    {Pk_Next,	    'k', 'N'},
    {Pk_Print,	    '%', '9'},

    {Pk_KP_Add,	    'K', '6'},
    {Pk_KP_Subtract,'K', '7'},
    {Pk_KP_Divide,  'K', '8'},
    {Pk_KP_Multiply,'K', '9'},
    {Pk_KP_Enter,   'K', 'A'},

    {Pk_KP_0,	    KS_EXTRA, KE_KINS}, /* Insert    */
    {Pk_KP_Decimal, KS_EXTRA, KE_KDEL}, /* Delete    */

    {Pk_KP_4,	    'k', 'l'}, /* Left	    */
    {Pk_KP_6,	    'k', 'r'}, /* Right	    */
    {Pk_KP_8,	    'k', 'u'}, /* Up	    */
    {Pk_KP_2,	    'k', 'd'}, /* Down	    */

    {Pk_KP_7,	    'K', '1'}, /* Home	    */
    {Pk_KP_1,	    'K', '4'}, /* End	    */

    {Pk_KP_9,	    'K', '3'}, /* Page Up   */
    {Pk_KP_3,	    'K', '5'}, /* Page Down */

    {Pk_KP_5,	    '&', '8'}, /* Undo	    */

    /* Keys that we want to be able to use any modifier with: */
    {Pk_Return,	    CR,  NUL},
    {Pk_space,	    ' ', NUL},
    {Pk_Tab,	    TAB, NUL},
    {Pk_Escape,	    ESC, NUL},
    {NL,	    NL,	 NUL},
    {CR,	    CR,  NUL},

    /* End of list marker: */
    {0,		0, 0}
};


/****************************************************************************/

static PtCallbackF_t gui_ph_handle_timer_cursor;
static PtCallbackF_t gui_ph_handle_timer_timeout;

static PtCallbackF_t gui_ph_handle_window_cb;

static PtCallbackF_t gui_ph_handle_scrollbar;
static PtCallbackF_t gui_ph_handle_keyboard;	/* keyboard input */
static PtCallbackF_t gui_ph_handle_mouse;	/* mouse buttons and movement */
static PtCallbackF_t gui_ph_handle_pulldown_menu;
static PtCallbackF_t gui_ph_handle_menu;
static PtCallbackF_t gui_ph_handle_focus;	/* focus change of text area */

static PtCallbackF_t gui_ph_handle_menu_resize;

/* When a menu is unrealized, give focus back to vimTextArea */
static PtCallbackF_t gui_ph_handle_menu_unrealized;

#ifdef USE_PANEL_GROUP
static void gui_ph_get_panelgroup_margins( short*, short*, short*, short* );
#endif

/* Set the text for the balloon */
static PtWidget_t * gui_ph_show_tooltip( PtWidget_t *window,
                             PtWidget_t *widget,
                             int position,
                             char *text,
                             char *font,
                             PgColor_t fill_color,
                             PgColor_t text_color );

/****************************************************************************/

static PtWidget_t * gui_ph_show_tooltip( PtWidget_t *window,
                             PtWidget_t *widget,
                             int position,
                             char *text,
                             char *font,
                             PgColor_t fill_color,
                             PgColor_t text_color )
{
    PtArg_t arg;
    vimmenu_T *menu;
    char_u  *tooltip;

    PtSetArg( &arg, Pt_ARG_POINTER, &menu, 0 );
    PtGetResources( widget, 1, &arg );

    /* Override the text and position */

    tooltip = text;
    if( menu != NULL )
    {
	int index = MENU_INDEX_TIP;
	if( menu->strings[ index ] != NULL )
	    tooltip = menu->strings[ index ];
    }

    return( PtInflateBalloon(
	    window,
	    widget,
	    Pt_BALLOON_BOTTOM,
	    tooltip,
	    font,
	    fill_color,
	    text_color ) );
}

    static void
gui_ph_resize_container( void )
{
    PhArea_t area;

    PtWidgetArea( gui.vimWindow, &area );
    PtWidgetPos ( gui.vimContainer, &area.pos );

    PtSetResource( gui.vimContainer, Pt_ARG_AREA, &area, 0 );
}

    static int
gui_ph_handle_menu_resize(
	PtWidget_t *widget,
	void *other,
	PtCallbackInfo_t *info )
{
    PtContainerCallback_t *sizes = info->cbdata;
    PtWidget_t		*container;
    PhPoint_t		below_menu;
    int_u		height;
    static short_u	is_first_time = TRUE;

    height = sizes->new_dim.h;

    /*
     * FIXME: Before the main window is realised, and after calling
     * PtExtentWidget, the height is 4 pixels smaller than it should be
     */
    if( is_first_time == TRUE )
    {
	is_first_time = FALSE;
	height += 4;
    }

    /* Because vim treats the toolbar and menubar separatly,
     * and here they're lumped together into a PtToolbarGroup,
     * we only need either menu_height or toolbar_height set at once */
    if( gui.menu_is_active )
    {
	gui.menu_height = height;
	gui.toolbar_height = 0;
    }
#ifdef FEAT_TOOLBAR
    else
	gui.toolbar_height = height;
#endif

    below_menu.x = 0;
    below_menu.y = height;

#ifdef USE_PANEL_GROUP
    container = gui.vimPanelGroup;
#else
    container = gui.vimContainer;
#endif

    PtSetResource( container, Pt_ARG_POS, &below_menu, 0 );

    gui_ph_resize_container();

#ifdef USE_PANEL_GROUP
    gui_ph_get_panelgroup_margins(
	    &pg_margin_top, &pg_margin_bottom,
	    &pg_margin_left, &pg_margin_right );
#endif
    return( Pt_CONTINUE );
}

/*
 * Pt_ARG_TIMER_REPEAT isn't used because the on & off times
 * are different
 */
    static int
gui_ph_handle_timer_cursor(
	PtWidget_t *widget,
	void *data,
	PtCallbackInfo_t *info )
{
    if( blink_state == BLINK_ON )
    {
	gui_undraw_cursor();
	blink_state = BLINK_OFF;
	PtSetResource( gui_ph_timer_cursor, Pt_ARG_TIMER_INITIAL,
		blink_offtime, 0 );
    }
    else
    {
	gui_update_cursor(TRUE, FALSE);
	blink_state = BLINK_ON;
	PtSetResource( gui_ph_timer_cursor, Pt_ARG_TIMER_INITIAL,
		blink_ontime, 0 );
    }
    return( Pt_CONTINUE );
}

    static int
gui_ph_handle_timer_timeout(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
    is_timeout = TRUE;

    return( Pt_CONTINUE );
}

    static int
gui_ph_handle_window_cb( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    PhWindowEvent_t *we = info->cbdata;
    int width, height;

    switch( we->event_f ) {
	case Ph_WM_CLOSE:
	    gui_shell_closed();
	    break;

	case Ph_WM_FOCUS:
	    /* Just in case it's hidden and needs to be shown */
	    gui_mch_mousehide( MOUSE_SHOW );

	    if( we->event_state == Ph_WM_EVSTATE_FOCUS )
	    {
		gui_focus_change(TRUE);
		gui_mch_start_blink();
	    }
	    else
	    {
		gui_focus_change(FALSE);
		gui_mch_stop_blink();
	    }
	    break;

	case Ph_WM_RESIZE:
	    /* FIXME: the +1 is for a bug in photon */
	    width  = we->size.w + 1;
	    height = we->size.h + 1;
#ifdef USE_PANEL_GROUP
	    width  -= (pg_margin_left + pg_margin_right);
	    height -= (pg_margin_top + pg_margin_bottom);
#endif
	    gui_resize_shell( width, height );
	    is_ignore_draw = FALSE;
	    PtEndFlux( gui.vimContainer );
	    PtContainerRelease( gui.vimContainer );
	    break;

	default:
	    break;
    }

    return( Pt_CONTINUE );
}

    static int
gui_ph_handle_scrollbar( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    PtScrollbarCallback_t *scroll;
    scrollbar_T *sb;
    int	    value, dragging = FALSE;

    scroll = info->cbdata;

    sb = (scrollbar_T *) data;
    if( sb != NULL )
    {
	value = scroll->position;
	switch( scroll->action )
	{
	    case Pt_SCROLL_DRAGGED:
		dragging = TRUE;
		break;

	    case Pt_SCROLL_RELEASED:
		/* FIXME: return straight away here? */
		break;
	}

	gui_drag_scrollbar(sb, value, dragging);
    }
    return Pt_CONTINUE;
}

    static int
gui_ph_handle_keyboard( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    PhKeyEvent_t    *key;
    unsigned char   string[6];
    int		    len, i;
    int		    ch, modifiers;

    key = PhGetData( info->event );

    ch = modifiers = len = 0;

    gui_mch_mousehide( MOUSE_HIDE );

    /* We're a good lil photon program, aren't we? yes we are, yeess wee arrr */
    if( key->key_flags & Pk_KF_Compose )
    {
	return( Pt_CONTINUE );
    }

    if( (key->key_flags & Pk_KF_Cap_Valid) &&
	    PkIsKeyDown( key->key_flags ) )
    {
#ifdef FEAT_MENU
	/*
	 * Only show the menu if the Alt key is down, and the Shift & Ctrl
	 * keys aren't down, as well as the other conditions
	 */
	if( ( ( key->key_mods & Pk_KM_Alt ) &&
		    !( key->key_mods & Pk_KM_Shift ) &&
		    !( key->key_mods & Pk_KM_Ctrl ) ) &&
	    gui.menu_is_active &&
	    ( *p_wak == 'y' ||
	      ( *p_wak == 'm' &&
		gui_is_menu_shortcut( key->key_cap ) ) ) )
	{
	    /* Fallthrough and let photon look for the hotkey */
	    return( Pt_CONTINUE );
	}
#endif

	for( i = 0; special_keys[i].key_sym != 0; i++ )
	{
	    if( special_keys[i].key_sym == key->key_cap )
	    {
		len = 0;
		if( special_keys[i].vim_code1 == NUL )
		    ch = special_keys[i].vim_code0;
		else
		{
		    /* Detect if a keypad number key has been pressed
		     * and change the key if Num Lock is on */
		    if( key->key_cap >= Pk_KP_0 && key->key_cap <= Pk_KP_9
			    && ( key->key_mods & Pk_KM_Num_Lock ) )
		    {
			ch = TO_SPECIAL( 'K', key->key_cap - Pk_KP_0 + 'C' );
		    }
		    else
			ch = TO_SPECIAL( special_keys[i].vim_code0,
				special_keys[i].vim_code1 );
		}
		break;
	    }
	}

	if( key->key_mods & Pk_KM_Ctrl )
	    modifiers |= MOD_MASK_CTRL;
	if( key->key_mods & Pk_KM_Alt )
	    modifiers |= MOD_MASK_ALT;
	if( key->key_mods & Pk_KM_Shift )
	    modifiers |= MOD_MASK_SHIFT;

	/* Is this not a special key? */
	if( special_keys[i].key_sym == 0 )
	{
	    ch = PhTo8859_1( key );
	    if( ch == -1
#ifdef FEAT_MBYTE
		|| ( enc_utf8 && ch > 127 )
#endif
		)
	    {
#ifdef FEAT_MBYTE
		len = PhKeyToMb( string, key );
		if( len > 0 )
		{
		    static char buf[6];
		    int src_taken, dst_made;
		    if( enc_utf8 != TRUE )
		    {
			PxTranslateFromUTF(
				charset_translate,
				string,
				len,
				&src_taken,
				buf,
				6,
				&dst_made );

			add_to_input_buf( buf, dst_made );
		    }
		    else
		    {
			add_to_input_buf( string, len );
		    }

		    return( Pt_CONSUME );
		}
		len = 0;
#endif
		ch = key->key_cap;
		if( ch < 0xff )
		{
		    /* FIXME: is this the right thing to do? */
		    if( modifiers & MOD_MASK_CTRL )
		    {
			modifiers &= ~MOD_MASK_CTRL;

			if( ( ch >= 'a'  &&  ch <= 'z' ) ||
				ch == '[' ||
				ch == ']' ||
				ch == '\\' )
			    ch = Ctrl_chr( ch );
			else if( ch == '2' )
			    ch = NUL;
			else if( ch == '6' )
			    ch = 0x1e;
			else if( ch == '-' )
			    ch = 0x1f;
			else
			    modifiers |= MOD_MASK_CTRL;
		    }

		    if( modifiers & MOD_MASK_ALT )
		    {
			ch = Meta( ch );
			modifiers &= ~MOD_MASK_ALT;
		    }
		}
		else
		{
		    return( Pt_CONTINUE );
		}
	    }
	}

	ch = simplify_key( ch, &modifiers );
	if( modifiers )
	{
	    string[ len++ ] = CSI;
	    string[ len++ ] = KS_MODIFIER;
	    string[ len++ ] = modifiers;
	}

	if( IS_SPECIAL( ch ) )
	{
	    string[ len++ ] = CSI;
	    string[ len++ ] = K_SECOND( ch );
	    string[ len++ ] = K_THIRD( ch );
	}
	else
	{
	    string[ len++ ] = ch;
	}

	if (len == 1 && ((ch == Ctrl_C && ctrl_c_interrupts)
							  || ch == intr_char))
	{
	    trash_input_buf();
	    got_int = TRUE;
	}

	if (len == 1 && string[0] == CSI)
	{
	    /* Turn CSI into K_CSI. */
	    string[ len++ ] = KS_EXTRA;
	    string[ len++ ] = KE_CSI;
	}

	if( len > 0 )
	{
	    add_to_input_buf( string, len );
	    return( Pt_CONSUME );
	}
    }

    return( Pt_CONTINUE );
}

    static int
gui_ph_handle_mouse( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    PhPointerEvent_t *pointer;
    PhRect_t	     *pos;
    int		     button = 0, repeated_click, modifiers = 0x0;
    short	     mouse_x, mouse_y;

    pointer = PhGetData( info->event );
    pos = PhGetRects( info->event );

    gui_mch_mousehide( MOUSE_SHOW );

    /*
     * Coordinates need to be relative to the base window,
     * not relative to the vimTextArea widget
     */
    mouse_x = pos->ul.x + gui.border_width;
    mouse_y = pos->ul.y + gui.border_width;

    if( info->event->type == Ph_EV_PTR_MOTION_NOBUTTON )
    {
	gui_mouse_moved( mouse_x, mouse_y );
	return( Pt_CONTINUE );
    }

    if( pointer->key_mods & Pk_KM_Shift )
	modifiers |= MOUSE_SHIFT;
    if( pointer->key_mods & Pk_KM_Ctrl )
	modifiers |= MOUSE_CTRL;
    if( pointer->key_mods & Pk_KM_Alt )
	modifiers |= MOUSE_ALT;

    /*
     * FIXME More than one button may be involved, but for
     * now just deal with one
     */
    if( pointer->buttons & Ph_BUTTON_SELECT )
	button = MOUSE_LEFT;

    if( pointer->buttons & Ph_BUTTON_MENU )
    {
	button = MOUSE_RIGHT;
	/* Need the absolute coordinates for the popup menu */
	abs_mouse.x = pointer->pos.x;
	abs_mouse.y = pointer->pos.y;
    }

    if( pointer->buttons & Ph_BUTTON_ADJUST )
	button = MOUSE_MIDDLE;

    /* Catch a real release (not phantom or other releases */
    if( info->event->type == Ph_EV_BUT_RELEASE )
	button = MOUSE_RELEASE;

    if( info->event->type & Ph_EV_PTR_MOTION_BUTTON )
	button = MOUSE_DRAG;

#if 0
    /* Vim doesn't use button repeats */
    if( info->event->type & Ph_EV_BUT_REPEAT )
	button = MOUSE_DRAG;
#endif

    /* Don't do anything if it is one of the phantom mouse release events */
    if( ( button != MOUSE_RELEASE ) ||
	    ( info->event->subtype == Ph_EV_RELEASE_REAL ) )
    {
	repeated_click = (pointer->click_count >= 2) ? TRUE : FALSE;

	gui_send_mouse_event( button , mouse_x, mouse_y, repeated_click, modifiers );
    }

    return( Pt_CONTINUE );
}

/* Handle a focus change of the PtRaw widget */
    static int
gui_ph_handle_focus( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    if( info->reason == Pt_CB_LOST_FOCUS )
    {
	PtRemoveEventHandler( gui.vimTextArea, Ph_EV_PTR_MOTION_NOBUTTON,
		gui_ph_handle_mouse, NULL );

	gui_mch_mousehide( MOUSE_SHOW );
    }
    else
    {
	PtAddEventHandler( gui.vimTextArea, Ph_EV_PTR_MOTION_NOBUTTON,
		gui_ph_handle_mouse, NULL );
    }
    return( Pt_CONTINUE );
}

    static void
gui_ph_handle_raw_draw( PtWidget_t *widget, PhTile_t *damage )
{
    PhRect_t	*r;
    PhPoint_t	offset;
    PhPoint_t	translation;

    if( is_ignore_draw == TRUE )
	return;

    PtSuperClassDraw( PtBasic, widget, damage );
    PgGetTranslation( &translation );
    PgClearTranslation();

#if 0
    /*
     * This causes some wierd probems, with drawing being done from
     * within this raw drawing function (rather than just simple clearing
     * and text drawing done by gui_redraw)
     *
     * The main problem is when PhBlit is used, and the cursor appearing
     * in places where it shouldn't
     */
    out_flush();
#endif

    PtWidgetOffset( widget, &offset );
    PhTranslatePoint( &offset, PtWidgetPos( gui.vimTextArea, NULL ) );

#if 1
    /* Redraw individual damage regions */
    if( damage->next != NULL )
	damage = damage->next;

    while( damage != NULL )
    {
	r = &damage->rect;
	gui_redraw(
		r->ul.x - offset.x, r->ul.y - offset.y,
		r->lr.x - r->ul.x + 1,
		r->lr.y - r->ul.y + 1 );
	damage = damage->next;
    }
#else
    /* Redraw the rectangle that covers all the damaged regions */
    r = &damage->rect;
    gui_redraw(
	    r->ul.x - offset.x, r->ul.y - offset.y,
	    r->lr.x - r->ul.x + 1,
	    r->lr.y - r->ul.y + 1 );
#endif

    PgSetTranslation( &translation, 0 );
}

    static int
gui_ph_handle_pulldown_menu(
	PtWidget_t *widget,
	void *data,
	PtCallbackInfo_t *info )
{
    if( data != NULL )
    {
	vimmenu_T *menu = (vimmenu_T *) data;

	PtPositionMenu( menu->submenu_id, NULL );
	PtRealizeWidget( menu->submenu_id );
    }

    return( Pt_CONTINUE );
}

/* This is used for pulldown/popup menus and also toolbar buttons */
    static int
gui_ph_handle_menu( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    if( data != NULL )
    {
	vimmenu_T *menu = (vimmenu_T *) data;
	gui_menu_cb( menu );
    }
    return( Pt_CONTINUE );
}

    static int
gui_ph_handle_menu_unrealized(
	PtWidget_t *widget,
	void *data,
	PtCallbackInfo_t *info )
{
    PtGiveFocus( gui.vimTextArea, NULL );
    return( Pt_CONTINUE );
}

/****************************************************************************/

#define DRAW_START  gui_ph_draw_start()
#define DRAW_END    gui_ph_draw_end()

/* TODO: Set a clipping rect? */
    static void
gui_ph_draw_start( void )
{
    PgSetRegion( PtWidgetRid( PtFindDisjoint( gui.vimTextArea ) ) );

    PtWidgetOffset( gui.vimTextArea, &gui_ph_raw_offset );
    PhTranslatePoint( &gui_ph_raw_offset, PtWidgetPos( gui.vimTextArea, NULL ) );

    PgSetTranslation( &gui_ph_raw_offset, Pg_RELATIVE );
}

    static void
gui_ph_draw_end( void )
{
    gui_ph_raw_offset.x = -gui_ph_raw_offset.x;
    gui_ph_raw_offset.y = -gui_ph_raw_offset.y;
    PgSetTranslation( &gui_ph_raw_offset, Pg_RELATIVE );
}

#ifdef USE_PANEL_GROUP
    static vimmenu_T *
gui_ph_find_buffer_item( char_u *name )
{
    vimmenu_T *top_level = root_menu;
    vimmenu_T *items = NULL;

    while( top_level != NULL &&
	    ( strcmp( top_level->dname, "Buffers" ) != 0 ) )
	top_level = top_level->next;

    if( top_level != NULL )
    {
	items = top_level->children;

	while( items != NULL &&
		( strcmp( items->dname, name ) != 0 ) )
	    items = items->next;
    }
    return( items );
}

    static void
gui_ph_pg_set_buffer_num( int_u buf_num )
{
    int i;
    char search[16];
    char *mark;

    if( gui.vimTextArea == NULL || buf_num == 0 )
	return;

    search[0] = '(';
    ultoa( buf_num, &search[1], 10 );
    strcat( search, ")" );

    for( i = 0; i < num_panels; i++ )
    {
	/* find the last "(" in the panel title and see if the buffer
	 * number in the title matches the one we're looking for */
	mark = strrchr( panel_titles[ i ], '(' );
	if( mark != NULL && strcmp( mark, search ) == 0 )
	{
	    PtSetResource( gui.vimPanelGroup, Pt_ARG_PG_CURRENT_INDEX,
		    i, 0 );
	}
    }
}

    static int
gui_ph_handle_pg_change(
	PtWidget_t *widget,
	void *data,
	PtCallbackInfo_t *info )
{
    vimmenu_T *menu;
    PtPanelGroupCallback_t *panel;

    if( info->event != NULL )
    {
	panel = info->cbdata;
	if( panel->new_panel != NULL )
	{
	    menu = gui_ph_find_buffer_item( panel->new_panel );
	    if( menu )
		gui_menu_cb( menu );
	}
    }
    return( Pt_CONTINUE );
}

    static void
gui_ph_get_panelgroup_margins( short *top, short *bottom, short *left, short *right )
{
    unsigned short abs_raw_x, abs_raw_y, abs_panel_x, abs_panel_y;
    const unsigned short *margin_top, *margin_bottom, *margin_left, *margin_right;

    PtGetAbsPosition( gui.vimTextArea, &abs_raw_x, &abs_raw_y );
    PtGetAbsPosition( gui.vimPanelGroup, &abs_panel_x, &abs_panel_y );

    PtGetResource( gui.vimPanelGroup, Pt_ARG_MARGIN_RIGHT, &margin_right, 0 );
    PtGetResource( gui.vimPanelGroup, Pt_ARG_MARGIN_BOTTOM, &margin_bottom, 0 );

    abs_raw_x -= abs_panel_x;
    abs_raw_y -= abs_panel_y;

    *top    = abs_raw_y;
    *bottom = *margin_bottom;

    *left  = abs_raw_x;
    *right = *margin_right;
}

/* Used for the tabs for PtPanelGroup */
    static int
gui_ph_is_buffer_item( vimmenu_T *menu, vimmenu_T *parent )
{
    char *mark;

    if( strcmp( parent->dname, "Buffers" ) == 0 )
    {
	/* Look for '(' digits ')' */
	mark = vim_strchr( menu->dname, '(' );
	if( mark != NULL )
	{
	    mark++;
	    while( isdigit( *mark ) )
		mark++;

	    if( *mark == ')' )
		return( TRUE);
	}
    }
    return( FALSE );
}

    static void
gui_ph_pg_add_buffer(char *name )
{
    char **new_titles = NULL;

    new_titles = (char **) alloc( ( num_panels + 1 ) * sizeof( char ** ) );
    if( new_titles != NULL )
    {
	if( num_panels > 0 )
	    memcpy( new_titles, panel_titles, num_panels * sizeof( char ** ) );

	new_titles[ num_panels++ ] = name;

	PtSetResource( gui.vimPanelGroup, Pt_ARG_PG_PANEL_TITLES, new_titles,
		num_panels );

	vim_free( panel_titles );
	panel_titles = new_titles;
    }
}

    static void
gui_ph_pg_remove_buffer( char *name )
{
    int i;
    char **new_titles = NULL;

    /* If there is only 1 panel, we just use the temporary place holder */
    if( num_panels > 1 )
    {
	new_titles = (char **) alloc( ( num_panels - 1 ) * sizeof( char ** ) );
	if( new_titles != NULL )
	{
	    char **s = new_titles;
	    /* Copy all the titles except the one we're removing */
	    for( i = 0; i < num_panels; i++ )
	    {
		if( strcmp( panel_titles[ i ], name ) != 0 )
		{
		    *s++ = panel_titles[ i ];
		}
	    }
	    num_panels--;

	    PtSetResource( gui.vimPanelGroup, Pt_ARG_PG_PANEL_TITLES, new_titles,
		    num_panels );

	    vim_free( panel_titles );
	    panel_titles = new_titles;
	}
    }
    else
    {
	num_panels--;
	PtSetResource( gui.vimPanelGroup, Pt_ARG_PG_PANEL_TITLES, &empty_title,
		1 );

	vim_free( panel_titles );
	panel_titles = NULL;
    }
}

/* When a buffer item is deleted from the buffer menu */
    static int
gui_ph_handle_buffer_remove(
	PtWidget_t *widget,
	void *data,
	PtCallbackInfo_t *info )
{
    vimmenu_T *menu;

    if( data != NULL )
    {
	menu = (vimmenu_T *) data;
	gui_ph_pg_remove_buffer( menu->dname );
    }

    return( Pt_CONTINUE );
}
#endif

    static int
gui_ph_pane_resize( PtWidget_t *widget, void *data, PtCallbackInfo_t *info )
{
    if( PtWidgetIsRealized( widget ) )
    {
	is_ignore_draw = TRUE;
	PtStartFlux( gui.vimContainer );
	PtContainerHold( gui.vimContainer );
    }

    return( Pt_CONTINUE );
}

/****************************************************************************/

#ifdef FEAT_MBYTE
    void
gui_ph_encoding_changed( int new_encoding )
{
    /* Default encoding is latin1 */
    char *charset = "latin1";
    int i;

    struct {
	int encoding;
	char *name;
    } charsets[] = {
	{ DBCS_JPN, "SHIFT_JIS" },
	{ DBCS_KOR, "csEUCKR" },
	{ DBCS_CHT, "big5" },
	{ DBCS_CHS, "gb" }
    };

    for( i = 0; i < ARRAY_LENGTH( charsets ); i++ )
    {
	if( new_encoding == charsets[ i ].encoding )
	    charset = charsets[ i ].name;
    }

    charset_translate = PxTranslateSet( charset_translate, charset );
}
#endif

/****************************************************************************/
/****************************************************************************/
/* all gui_mch_* functions below */

    void
gui_mch_prepare(argc, argv)
    int	    *argc;
    char    **argv;
{
    /* FIXME: Make sure we can run vim on a different node */
    PtInit( NULL );
}

    int
gui_mch_init(void)
{
    PtArg_t args[10];
    int	    n = 0;

    PhDim_t	window_size = {100, 100}; /* Abitrary values */
    PhPoint_t	pos = {0, 0};

    gui.event_buffer = (PhEvent_t *) alloc( EVENT_BUFFER_SIZE );
    if( gui.event_buffer == NULL )
	return( FAIL );

    /* Get a translation so we can convert from ISO Latin-1 to UTF */
    charset_translate = PxTranslateSet( NULL, "latin1" );

    /* The +2 is for the 1 pixel dark line on each side */
    gui.border_offset = gui.border_width = GUI_PH_MARGIN + 2;

    /* Handle close events ourselves */
    PtSetArg( &args[ n++ ], Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_FALSE, Ph_WM_CLOSE );
    PtSetArg( &args[ n++ ], Pt_ARG_WINDOW_NOTIFY_FLAGS, Pt_TRUE,
	    Ph_WM_CLOSE | Ph_WM_RESIZE | Ph_WM_FOCUS );
    PtSetArg( &args[ n++ ], Pt_ARG_DIM, &window_size, 0 );
    gui.vimWindow = PtCreateWidget( PtWindow, NULL, n, args );
    if( gui.vimWindow == NULL )
	return( FAIL );

    PtAddCallback( gui.vimWindow, Pt_CB_WINDOW, gui_ph_handle_window_cb, NULL );

    n = 0;
    PtSetArg( &args[ n++ ], Pt_ARG_ANCHOR_FLAGS,
	    Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT |
	    Pt_BOTTOM_ANCHORED_BOTTOM | Pt_TOP_ANCHORED_TOP,
	    Pt_IS_ANCHORED );
    PtSetArg( &args[ n++ ], Pt_ARG_DIM, &window_size, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_POS, &pos, 0 );

#ifdef USE_PANEL_GROUP
    /* Put in a temprary place holder title */
    PtSetArg( &args[ n++ ], Pt_ARG_PG_PANEL_TITLES, &empty_title, 1 );

    gui.vimPanelGroup = PtCreateWidget( PtPanelGroup, gui.vimWindow, n, args );
    if( gui.vimPanelGroup == NULL )
	return( FAIL );

    PtAddCallback( gui.vimPanelGroup, Pt_CB_PG_PANEL_SWITCHING,
	    gui_ph_handle_pg_change, NULL );
#else
    /* Turn off all edge decorations */
    PtSetArg( &args[ n++ ], Pt_ARG_BASIC_FLAGS, Pt_FALSE, Pt_ALL );
    PtSetArg( &args[ n++ ], Pt_ARG_BEVEL_WIDTH, 0, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_MARGIN_WIDTH, 0, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_MARGIN_HEIGHT, 0, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_CONTAINER_FLAGS, Pt_TRUE, Pt_AUTO_EXTENT );

    gui.vimContainer = PtCreateWidget( PtPane, gui.vimWindow, n, args );
    if( gui.vimContainer == NULL )
	return( FAIL );

    PtAddCallback( gui.vimContainer, Pt_CB_RESIZE, gui_ph_pane_resize, NULL );
#endif

    /* Size for the text area is set in gui_mch_set_text_area_pos */
    n = 0;

    PtSetArg( &args[ n++ ], Pt_ARG_RAW_DRAW_F, gui_ph_handle_raw_draw, 1 );
    PtSetArg( &args[ n++ ], Pt_ARG_BEVEL_WIDTH, GUI_PH_MARGIN, 0 );
    /*
     * Using focus render also causes the whole widget to be redrawn
     * whenever it changes focus, which is very annoying :p
     */
    PtSetArg( &args[ n++ ], Pt_ARG_FLAGS, Pt_TRUE,
	    Pt_GETS_FOCUS | Pt_HIGHLIGHTED );
    PtSetArg( &args[ n++ ], Pt_ARG_CURSOR_TYPE, GUI_PH_MOUSE_TYPE, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_CURSOR_COLOR, gui_ph_mouse_color, 0 );

    gui.vimTextArea = PtCreateWidget( PtRaw, Pt_DFLT_PARENT, n, args );
    if( gui.vimTextArea == NULL)
	return( FAIL );

    /* TODO: use PtAddEventHandlers instead? */
    /* Not using Ph_EV_BUT_REPEAT because vim wouldn't use it anyway */
    PtAddEventHandler( gui.vimTextArea,
	    Ph_EV_BUT_PRESS | Ph_EV_BUT_RELEASE | Ph_EV_PTR_MOTION_BUTTON,
	    gui_ph_handle_mouse, NULL );
    PtAddEventHandler( gui.vimTextArea, Ph_EV_KEY,
	    gui_ph_handle_keyboard, NULL );
    PtAddCallback( gui.vimTextArea, Pt_CB_GOT_FOCUS,
	    gui_ph_handle_focus, NULL );
    PtAddCallback( gui.vimTextArea, Pt_CB_LOST_FOCUS,
	    gui_ph_handle_focus, NULL );

    /*
     * Now that the text area widget has been created, set up the colours,
     * which wil call PtSetResource from gui_mch_new_colors
     */

    /*
     * Create the two timers, not as accurate as using the kernel timer
     * functions, but good enough
     */
    gui_ph_timer_cursor  = PtCreateWidget( PtTimer, gui.vimWindow, 0, NULL );
    if( gui_ph_timer_cursor == NULL )
	return( FAIL );

    gui_ph_timer_timeout = PtCreateWidget( PtTimer, gui.vimWindow, 0, NULL );
    if( gui_ph_timer_timeout == NULL )
	return( FAIL );

    PtAddCallback( gui_ph_timer_cursor,  Pt_CB_TIMER_ACTIVATE,
	    gui_ph_handle_timer_cursor, NULL);
    PtAddCallback( gui_ph_timer_timeout, Pt_CB_TIMER_ACTIVATE,
	    gui_ph_handle_timer_timeout, NULL);

#ifdef FEAT_MENU
    n = 0;
    PtSetArg( &args[ n++ ], Pt_ARG_WIDTH, window_size.w, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_ANCHOR_FLAGS,
	    Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT,
	    Pt_IS_ANCHORED );
    gui.vimToolBarGroup = PtCreateWidget( PtToolbarGroup, gui.vimWindow,
	    n, args );
    if( gui.vimToolBarGroup == NULL )
	return( FAIL );

    PtAddCallback( gui.vimToolBarGroup, Pt_CB_RESIZE,
	    gui_ph_handle_menu_resize, NULL );

    n = 0;
    PtSetArg( &args[ n++ ], Pt_ARG_WIDTH, window_size.w, 0 );

    gui.vimMenuBar = PtCreateWidget( PtMenuBar, gui.vimToolBarGroup, n, args );
    if( gui.vimMenuBar == NULL )
	return( FAIL );

# ifdef FEAT_TOOLBAR
    n = 0;

    PtSetArg( &args[ n++ ], Pt_ARG_ANCHOR_FLAGS,
	    Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT |
	    Pt_TOP_ANCHORED_TOP,
	    Pt_IS_ANCHORED );
    PtSetArg( &args[ n++ ], Pt_ARG_RESIZE_FLAGS, Pt_TRUE,
	    Pt_RESIZE_Y_AS_REQUIRED );
    PtSetArg( &args[ n++ ], Pt_ARG_WIDTH, window_size.w, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_TOOLBAR_FLAGS,
	    Pt_TRUE, Pt_TOOLBAR_ITEM_SEPARATORS );
    PtSetArg( &args[ n++ ], Pt_ARG_FLAGS, Pt_DELAY_REALIZE,
	    Pt_DELAY_REALIZE | Pt_GETS_FOCUS );

    gui.vimToolBar = PtCreateWidget( PtToolbar, gui.vimToolBarGroup, n, args );
    if( gui.vimToolBar == NULL )
	return( FAIL );

    /*
     * Size for the toolbar is fetched in gui_mch_show_toolbar, after
     * the buttons have been added and the toolbar has resized it's height
     * for the buttons to fit
     */
# endif

#endif

    return( OK );
}

    int
gui_mch_init_check(void)
{
    return( (is_photon_available == TRUE) ? OK : FAIL );
}

    int
gui_mch_open(void)
{
    gui.norm_pixel =  Pg_BLACK;
    gui.back_pixel =  Pg_WHITE;

    set_normal_colors();

    gui_check_colors();
    gui.def_norm_pixel = gui.norm_pixel;
    gui.def_back_pixel = gui.back_pixel;

    highlight_gui_started();

    if (gui_win_x != -1 && gui_win_y != -1)
	gui_mch_set_winpos(gui_win_x, gui_win_y);

    return( (PtRealizeWidget( gui.vimWindow ) == 0) ? OK : FAIL );
}

    void
gui_mch_exit(int rc)
{
    PtDestroyWidget( gui.vimWindow );

    PxTranslateSet( charset_translate, NULL );

    vim_free( gui.event_buffer );

#ifdef USE_PANEL_GROUPS
    vim_free( panel_titles );
#endif
}

/****************************************************************************/
/* events */

    void
gui_mch_update(void)
{
    int working = -1;

    while( working && !vim_is_input_buf_full())
    {
	switch( PhEventPeek( gui.event_buffer, EVENT_BUFFER_SIZE ) )
	{
	    case 0: /* No messages */
		working = 0;
		break;

	    case Ph_EVENT_MSG:
		PtEventHandler( gui.event_buffer );
		break;

	    case -1:
		perror( "gui_mch_update: PhEventPeek failed" );
		working = 0;
		break;
	    default:
		perror( "PhEventPeek default reached" );
		working = 0;
		break;
	}
    }
}

    int
gui_mch_wait_for_chars(int wtime)
{
    is_timeout = FALSE;

    if( wtime > 0 )
	PtSetResource( gui_ph_timer_timeout, Pt_ARG_TIMER_INITIAL, wtime, 0 );

    while( 1 )
    {
	switch( PhEventNext( gui.event_buffer, EVENT_BUFFER_SIZE ) )
	{
	    case Ph_EVENT_MSG:
		PtEventHandler( gui.event_buffer );
		if( !vim_is_input_buf_empty() )
		{
		    PtSetResource( gui_ph_timer_timeout, Pt_ARG_TIMER_INITIAL, 0, 0 );
		    return( OK );
		}
		else if( is_timeout == TRUE )
		    return( FAIL );

		break;

	    case Ph_RESIZE_MSG:
		/* FIXME */
		return( FAIL );
		break;

	    case -1:
		perror( "PhEventNext" );
		return( FAIL );
	}
    }
}

#if defined( FEAT_BROWSE ) || defined( PROTO )
/*
 * Put up a file requester.
 * Returns the selected name in allocated memory, or NULL for Cancel.
 * saving,	    select file to write
 * title	    title for the window
 * default_name	    default name (well duh!)
 * ext		    not used (extension added)
 * initdir	    initial directory, NULL for current dir
 * filter	    not used (file name filter)
 */
    char_u *
gui_mch_browse(
	int saving,
	char_u *title,
	char_u *default_name,
	char_u *ext,
	char_u *initdir,
	char_u *filter)
{
    PtFileSelectionInfo_t file;
    int	    flags;
    char_u  *default_path;
    char_u  *open_text = NULL;

    flags = 0;
    memset( &file, 0, sizeof( file ) );

    default_path = alloc( MAXPATHL + 1 + NAME_MAX + 1 );
    if( default_path != NULL )
    {
	if( saving == TRUE )
	{
	    flags |= Pt_FSR_CONFIRM_EXISTING | Pt_FSR_NO_FCHECK;
	    open_text = "&Save";
	}

	/* combine the directory and filename into a single path */
	if( initdir == NULL || *initdir == NUL )
	{
	    mch_dirname( default_path, MAXPATHL );
	    initdir = default_path;
	}
	else
	{
	    STRCPY( default_path, initdir );
	    initdir = default_path;
	}

	if( default_name != NULL )
	{
	    if( default_path[ STRLEN( default_path ) - 1 ] != '/' )
		STRCAT( default_path, "/" );

	    STRCAT( default_path, default_name );
	}

	/* TODO: add a filter? */
	PtFileSelection(
		gui.vimWindow,
		NULL,
		title,
		default_path,
		NULL,
		open_text,
		NULL,
		NULL,
		&file,
		flags );

	vim_free( default_path );

	if( file.ret == Pt_FSDIALOG_BTN1 )
	    return( vim_strsave( file.path ) );
    }
    return( NULL );
}

#endif

#if defined( FEAT_GUI_DIALOG ) || defined( PROTO )
    int
gui_mch_dialog(
	int	type,
	char_u	*title,
	char_u	*message,
	char_u	*buttons,
	int	default_button,
	char_u	*textfield)
{
    char_u	*str;
    char_u	**button_array;
    char_u	*buttons_copy;

    int		button_count;
    int		i, len;
    int		dialog_result = -1;

    /* FIXME: the vertical option in guioptions is blatantly ignored */
    /* FIXME: so is the type */

    button_count = len = i = 0;

    if( buttons == NULL || *buttons == NUL )
	return( -1 );

    /* There is one less separator than buttons, so bump up the button count */
    button_count = 1;

    /* Count string length and number of seperators */
    for( str = buttons; *str; str++ )
    {
	len++;
	if( *str == DLG_BUTTON_SEP )
	    button_count++;
    }

    buttons_copy = alloc( len + 1 );
    button_array = (char_u **) alloc( button_count * sizeof( char_u * ) );
    if( buttons_copy != NULL && button_array != NULL )
    {
	STRCPY( buttons_copy, buttons );

	/*
	 * Convert DLG_BUTTON_SEP into NUL's and fill in
	 * button_array with the pointer to each NUL terminated string
	 */
	str = buttons_copy;
	for( i = 0; i < button_count; i++ )
	{
	    button_array[ i ] = str;
	    for( ; *str; str++ )
	    {
		if( *str == DLG_BUTTON_SEP )
		{
		    *str++ = NUL;
		    break;
		}
	    }
	}

	dialog_result = PtAlert(
		gui.vimWindow, NULL,
		title,
		NULL,
		message, NULL,
		button_count, (const char **) button_array, NULL,
		default_button, 0, Pt_MODAL );
    }

    vim_free( button_array );
    vim_free( buttons_copy );

    return( dialog_result );
}
#endif
/****************************************************************************/
/* window size/position/state */

    int
gui_mch_get_winpos(int *x, int *y)
{
    PhPoint_t *pos;

    pos = PtWidgetPos( gui.vimWindow, NULL );

    *x = pos->x;
    *y = pos->y;

    return( OK );
}

    void
gui_mch_set_winpos(int x, int y)
{
    PhPoint_t pos = { x, y };

    PtSetResource( gui.vimWindow, Pt_ARG_POS, &pos, 0 );
}

    void
gui_mch_set_shellsize(int width, int height, int min_width, int min_height,
	int base_width, int base_height)
{
    PhDim_t window_size = { width, height };
    PhDim_t min_size = { min_width, min_height };

#ifdef USE_PANEL_GROUP
    window_size.w += pg_margin_left + pg_margin_right;
    window_size.h += pg_margin_top + pg_margin_bottom;
#endif

    PtSetResource( gui.vimWindow, Pt_ARG_MINIMUM_DIM, &min_size, 0 );
    PtSetResource( gui.vimWindow, Pt_ARG_DIM, &window_size, 0 );

    if( ! PtWidgetIsRealized( gui.vimWindow ) )
	gui_ph_resize_container();
}

    void
gui_mch_get_screen_dimensions(int *screen_w, int *screen_h)
{
    /* FIXME */
    *screen_w = 640;
    *screen_h = 480;
}

    void
gui_mch_iconify(void)
{
    PhWindowEvent_t event;

    memset( &event, 0, sizeof (event) );
    event.event_f = Ph_WM_HIDE;
    event.event_state = Ph_WM_EVSTATE_HIDE;
    event.rid = PtWidgetRid( gui.vimWindow );
    PtForwardWindowEvent( &event );
}

    void
gui_mch_settitle(char_u *title,	char_u *icon)
{
#ifdef USE_PANEL_GROUP
    gui_ph_pg_set_buffer_num( curwin->w_buffer->b_fnum );
#endif
    PtSetResource( gui.vimWindow, Pt_ARG_WINDOW_TITLE, title, 0 );
    /* Not sure what to do with the icon text, set balloon text somehow? */
}

/****************************************************************************/
/* Scrollbar */

    void
gui_mch_set_scrollbar_thumb(scrollbar_T *sb, int val, int size, int max)
{
    int	    n = 0;
    PtArg_t args[3];

    PtSetArg( &args[ n++ ], Pt_ARG_MAXIMUM, max, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_SLIDER_SIZE, size, 0 );
    PtSetArg( &args[ n++ ], Pt_ARG_GAUGE_VALUE, val, 0 );
    PtSetResources( sb->id, n, args );
}

    void
gui_mch_set_scrollbar_pos(scrollbar_T *sb, int x, int y, int w, int h)
{
    PhArea_t area = {{ x, y }, { w, h }};

    PtSetResource( sb->id, Pt_ARG_AREA, &area, 0 );
}

    void
gui_mch_create_scrollbar(scrollbar_T *sb, int orient)
{
    int	    n = 0;
    int	    anchor_flags = 0;
    PtArg_t args[4];

    /*
     * Stop the scrollbar from being realized when the parent
     * is realized, so it can be explicitly realized by vim.
     *
     * Also, don't let the scrollbar get focus
     */
    PtSetArg( &args[ n++ ], Pt_ARG_FLAGS, Pt_DELAY_REALIZE,
	    Pt_DELAY_REALIZE | Pt_GETS_FOCUS );
    PtSetArg( &args[ n++ ], Pt_ARG_SCROLLBAR_FLAGS, Pt_SCROLLBAR_SHOW_ARROWS, 0);

    if( orient == SBAR_HORIZ )
    {
	anchor_flags = Pt_BOTTOM_ANCHORED_BOTTOM |
	    Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT;
    }
    else
    {
	anchor_flags = Pt_BOTTOM_ANCHORED_BOTTOM | Pt_TOP_ANCHORED_TOP;
	if( sb->wp != NULL )
	{
	    if( sb == &sb->wp->w_scrollbars[ SBAR_LEFT ] )
		anchor_flags |= Pt_LEFT_ANCHORED_LEFT;
	    else
		anchor_flags |= Pt_RIGHT_ANCHORED_RIGHT;
	}
    }
    PtSetArg( &args[ n++ ], Pt_ARG_ANCHOR_FLAGS, Pt_FALSE, Pt_TRUE );
    PtSetArg( &args[ n++ ], Pt_ARG_ORIENTATION,
	    (orient == SBAR_HORIZ) ? Pt_HORIZONTAL : Pt_VERTICAL, 0 );
#ifdef USE_PANEL_GROUP
    sb->id = PtCreateWidget( PtScrollbar, gui.vimPanelGroup, n, args );
#else
    sb->id = PtCreateWidget( PtScrollbar, gui.vimContainer, n, args );
#endif

    PtAddCallback( sb->id, Pt_CB_SCROLLBAR_MOVE, gui_ph_handle_scrollbar, sb );
}

    void
gui_mch_enable_scrollbar(scrollbar_T *sb, int flag)
{
    if( flag != 0 )
	PtRealizeWidget( sb->id );
    else
	PtUnrealizeWidget( sb->id );
}

    void
gui_mch_destroy_scrollbar(scrollbar_T *sb)
{
    PtDestroyWidget( sb->id );
}

/****************************************************************************/
/* Mouse functions */

    void
gui_mch_mousehide(int hide)
{
    if( p_mh && gui.pointer_hidden != hide )
    {
	PtSetResource( gui.vimTextArea, Pt_ARG_CURSOR_TYPE,
		( hide == MOUSE_SHOW ) ? Ph_CURSOR_INSERT : Ph_CURSOR_NONE,
		0 );
	gui.pointer_hidden = hide;
    }
}

    int
gui_mch_get_mouse_x(void)
{
    PhCursorInfo_t info;
    short x, y;

    /* FIXME: does this return the correct position,
     * with respect to the border? */
    PhQueryCursor( PhInputGroup( NULL ), &info );
    PtGetAbsPosition( gui.vimTextArea , &x, &y );

    return( info.pos.x - x );
}

    int
gui_mch_get_mouse_y(void)
{
    PhCursorInfo_t info;
    short x, y;

    PhQueryCursor( PhInputGroup( NULL ), &info );
    PtGetAbsPosition( gui.vimTextArea , &x, &y );
    /* TODO: Add border offset? */
    return( info.pos.y - y );
}

    void
gui_mch_setmouse(int x, int y)
{
    short abs_x, abs_y;

    PtGetAbsPosition( gui.vimTextArea, &abs_x, &abs_y );
    /* Add the border offset? */
    PhMoveCursorAbs( PhInputGroup( NULL ), abs_x + x, abs_y + y );
}

/****************************************************************************/
/* Colours */

#if (defined(FEAT_SYN_HL) && defined(FEAT_EVAL)) || defined(PROTO)
/*
 * Return the RGB value of a pixel as "#RRGGBB".
 */
    char_u *
gui_mch_get_rgb(
	guicolor_T    pixel)
{
    static char_u retval[10];

    sprintf((char *)retval, "#%02x%02x%02x",
	    PgRedValue(pixel), PgGreenValue(pixel), PgBlueValue(pixel));
    return( retval );
}
#endif

    int
gui_mch_get_lightness(guicolor_T pixel)
{
    return( (int) PgGreyValue( pixel ) );
}

    void
gui_mch_new_colors(void)
{
    short color_diff;

    /*
     * If there isn't enough difference between the background colour and
     * the mouse pointer colour then change the mouse pointer colour
     */
    color_diff = gui_mch_get_lightness( gui_ph_mouse_color ) -
	gui_mch_get_lightness( gui.back_pixel );

    if( abs( color_diff ) < 64 )
    {
	short r, g, b;
	/* not a great algorithm... */
	r = PgRedValue( gui_ph_mouse_color ) ^ 255;
	g = PgGreenValue( gui_ph_mouse_color ) ^ 255;
	b = PgBlueValue( gui_ph_mouse_color ) ^ 255;

	gui_ph_mouse_color = PgRGB( r, g, b );
	PtSetResource( gui.vimTextArea, Pt_ARG_CURSOR_COLOR,
		gui_ph_mouse_color, 0 );
    }

    PtSetResource( gui.vimTextArea, Pt_ARG_FILL_COLOR, gui.back_pixel, 0 );
}

    static int
hex_digit(int c)
{
    if (vim_isdigit(c))
	return( c - '0' );
    c = TO_LOWER(c);
    if (c >= 'a' && c <= 'f')
	return( c - 'a' + 10 );
    return( -1000 );
}


/*
 * This should be split out into a seperate file,
 * every port does basically the same thing.
 *
 * This is the gui_w32.c version (i think..)
 */

    guicolor_T
gui_mch_get_color(char_u *name)
{
    int i;
    int r, g, b;


    typedef struct GuiColourTable
    {
	char	    *name;
	guicolor_T     colour;
    } GuiColourTable;

    static GuiColourTable table[] =
    {
	{"Black",	    RGB(0x00, 0x00, 0x00)},
	{"DarkGray",	    RGB(0x80, 0x80, 0x80)},
	{"DarkGrey",	    RGB(0x80, 0x80, 0x80)},
	{"Gray",	    RGB(0xC0, 0xC0, 0xC0)},
	{"Grey",	    RGB(0xC0, 0xC0, 0xC0)},
	{"LightGray",	    RGB(0xD3, 0xD3, 0xD3)},
	{"LightGrey",	    RGB(0xD3, 0xD3, 0xD3)},
	{"White",	    RGB(0xFF, 0xFF, 0xFF)},
	{"DarkRed",	    RGB(0x80, 0x00, 0x00)},
	{"Red",		    RGB(0xFF, 0x00, 0x00)},
	{"LightRed",	    RGB(0xFF, 0xA0, 0xA0)},
	{"DarkBlue",	    RGB(0x00, 0x00, 0x80)},
	{"Blue",	    RGB(0x00, 0x00, 0xFF)},
	{"LightBlue",	    RGB(0xA0, 0xA0, 0xFF)},
	{"DarkGreen",	    RGB(0x00, 0x80, 0x00)},
	{"Green",	    RGB(0x00, 0xFF, 0x00)},
	{"LightGreen",	    RGB(0xA0, 0xFF, 0xA0)},
	{"DarkCyan",	    RGB(0x00, 0x80, 0x80)},
	{"Cyan",	    RGB(0x00, 0xFF, 0xFF)},
	{"LightCyan",	    RGB(0xA0, 0xFF, 0xFF)},
	{"DarkMagenta",	    RGB(0x80, 0x00, 0x80)},
	{"Magenta",	    RGB(0xFF, 0x00, 0xFF)},
	{"LightMagenta",    RGB(0xFF, 0xA0, 0xFF)},
	{"Brown",	    RGB(0x80, 0x40, 0x40)},
	{"Yellow",	    RGB(0xFF, 0xFF, 0x00)},
	{"LightYellow",	    RGB(0xFF, 0xFF, 0xA0)},
	{"SeaGreen",	    RGB(0x2E, 0x8B, 0x57)},
	{"Orange",	    RGB(0xFF, 0xA5, 0x00)},
	{"Purple",	    RGB(0xA0, 0x20, 0xF0)},
	{"SlateBlue",	    RGB(0x6A, 0x5A, 0xCD)},
	{"Violet",          RGB(0xEE, 0x82, 0xEE)},
    };

    /* is name #rrggbb format? */
    if( name[0] == '#' && STRLEN( name ) == 7 )
    {
	r = hex_digit( name[1] ) * 16 + hex_digit( name[2] );
	g = hex_digit( name[3] ) * 16 + hex_digit( name[4] );
	b = hex_digit( name[5] ) * 16 + hex_digit( name[6] );
	if( r < 0 || g < 0 || b < 0 )
	    return( (guicolor_T) -1 );
	return( RGB( r, g, b ) );
    }

    for( i = 0; i < ARRAY_LENGTH( table ); i++ )
    {
	if(STRICMP(name, table[i].name) == 0)
	    return (table[i].colour);
    }

    /*
     * Last attempt. Look in the file "$VIMRUNTIME/rgb.txt".
     */
    {
#define LINE_LEN 100
	FILE	*fd;
	char	line[LINE_LEN];
	char_u	*fname;

	fname = expand_env_save((char_u *)"$VIMRUNTIME/rgb.txt");
	if (fname == NULL)
	    return( (guicolor_T) -1 );

	fd = fopen((char *)fname, "rt");
	vim_free(fname);
	if (fd == NULL)
	    return( (guicolor_T) -1 );

	while (!feof(fd))
	{
	    int	    len;
	    int	    pos;
	    char    *color;

	    fgets(line, LINE_LEN, fd);
	    len = strlen(line);

	    if (len <= 1 || line[len-1] != '\n')
		continue;

	    line[len-1] = '\0';

	    i = sscanf(line, "%d %d %d %n", &r, &g, &b, &pos);
	    if (i != 3)
		continue;

	    color = line + pos;

	    if (STRICMP(color, name) == 0)
	    {
		fclose(fd);
		return( (guicolor_T) RGB(r,g,b) );
	    }
	}

	fclose(fd);
    }


    return( (guicolor_T) -1 );
}

    void
gui_mch_set_fg_color(guicolor_T color)
{
    PgSetTextColor( color );
}

    void
gui_mch_set_bg_color(guicolor_T color)
{
    PgSetFillColor( color );
}

    void
gui_mch_invert_rectangle(int row, int col, int nr, int nc)
{
    PhRect_t rect;

    rect.ul.x = FILL_X( col );
    rect.ul.y = FILL_Y( row );

    /* FIXME: This has an off by one pixel problem */
    rect.lr.x = rect.ul.x + nc * gui.char_width;
    rect.lr.y = rect.ul.y + nr * gui.char_height;
    if( nc > 0 )
	rect.lr.x -= 1;
    if( nr > 0 )
	rect.lr.y -= 1;

    DRAW_START;
    PgSetDrawMode( Pg_DrawModeDSTINVERT );
    PgDrawRect( &rect, Pg_DRAW_FILL );
    PgSetDrawMode( Pg_DrawModeSRCCOPY );
    DRAW_END;
}

    void
gui_mch_clear_block(int row1, int col1, int row2, int col2)
{
    PhRect_t block = {
	{ FILL_X( col1 ), FILL_Y( row1 ) },
	{ FILL_X( col2 + 1 ) - 1, FILL_Y( row2 + 1 ) - 1}
    };

    DRAW_START;
    gui_mch_set_bg_color( gui.back_pixel );
    PgDrawRect( &block, Pg_DRAW_FILL );
    DRAW_END;
}

    void
gui_mch_clear_all()
{
    PhRect_t text_rect = {
	{ gui.border_width, gui.border_width },
	{ Columns * gui.char_width + gui.border_width - 1 ,
	    Rows * gui.char_height + gui.border_width - 1 }
    };

    if( is_ignore_draw == TRUE )
	return;

    DRAW_START;
    gui_mch_set_bg_color( gui.back_pixel );
    PgDrawRect( &text_rect, Pg_DRAW_FILL );
    DRAW_END;
}

    void
gui_mch_delete_lines(int row, int num_lines)
{
    PhRect_t    rect;
    PhPoint_t   delta;

    rect.ul.x = FILL_X( gui.scroll_region_left );
    rect.ul.y = FILL_Y( row + num_lines );

    rect.lr.x = FILL_X( gui.scroll_region_right + 1 ) - 1;
    rect.lr.y = FILL_Y( gui.scroll_region_bot + 1) - 1;

    PtWidgetOffset( gui.vimTextArea, &gui_ph_raw_offset );
    PhTranslatePoint( &gui_ph_raw_offset, PtWidgetPos(gui.vimTextArea, NULL));
    PhTranslateRect( &rect, &gui_ph_raw_offset );

    delta.x = 0;
    delta.y = -num_lines * gui.char_height;

    PgFlush();

    PhBlit( PtWidgetRid( PtFindDisjoint( gui.vimTextArea ) ), &rect, &delta );

    gui_clear_block(
	gui.scroll_region_bot - num_lines + 1,
	gui.scroll_region_left,
	gui.scroll_region_bot,
	gui.scroll_region_right );
}

    void
gui_mch_insert_lines(int row, int num_lines)
{
    PhRect_t    rect;
    PhPoint_t   delta;

    rect.ul.x = FILL_X( gui.scroll_region_left );
    rect.ul.y = FILL_Y( row );

    rect.lr.x = FILL_X( gui.scroll_region_right + 1 ) - 1;
    rect.lr.y = FILL_Y( gui.scroll_region_bot - num_lines + 1 ) - 1;

    PtWidgetOffset( gui.vimTextArea, &gui_ph_raw_offset );
    PhTranslatePoint( &gui_ph_raw_offset, PtWidgetPos( gui.vimTextArea, NULL ) );
    PhTranslateRect( &rect, &gui_ph_raw_offset );

    delta.x = 0;
    delta.y = num_lines * gui.char_height;

    PgFlush();

    PhBlit( PtWidgetRid( PtFindDisjoint( gui.vimTextArea ) ) , &rect, &delta );

    gui_clear_block( row, gui.scroll_region_left,
	    row + num_lines - 1, gui.scroll_region_right );
}

    void
gui_mch_draw_string(int row, int col, char_u *s, int len, int flags)
{
    static char *utf8_buffer = NULL;
    static int	utf8_len = 0;

    PhPoint_t	pos = { TEXT_X( col ), TEXT_Y( row ) };
    PhRect_t	rect;

    if( is_ignore_draw == TRUE )
	return;

    DRAW_START;

    if( !( flags & DRAW_TRANSP ) )
    {
	PgDrawIRect(
		FILL_X( col ), FILL_Y( row ),
		FILL_X( col + len ) - 1, FILL_Y( row + 1 ) - 1,
		Pg_DRAW_FILL );
    }

    if( flags & DRAW_UNDERL )
	PgSetUnderline( gui.norm_pixel, Pg_TRANSPARENT, 0 );

    if( charset_translate != NULL
#ifdef FEAT_MBYTE
	    && enc_utf8 == 0
#endif
	    )
    {
	int src_taken, dst_made;

	/* Use a static buffer to avoid large amounts of de/allocations */
        if( utf8_len < len )
	{
	    utf8_buffer = realloc( utf8_buffer, len * MB_LEN_MAX );
	    utf8_len = len;
	}

	PxTranslateToUTF(
		charset_translate,
		s,
		len,
		&src_taken,
		utf8_buffer,
		utf8_len,
		&dst_made );
	s = utf8_buffer;
	len = dst_made;
    }

    PgDrawText( s, len, &pos, 0 );

    if( flags & DRAW_BOLD )
    {
	/* FIXME: try and only calculate these values once... */
	rect.ul.x = FILL_X( col ) + 1;
	rect.ul.y = FILL_Y( row );
	rect.lr.x = FILL_X( col + len ) - 1;
	rect.lr.y = FILL_Y( row + 1) - 1;
	/* PgSetUserClip( NULL ) causes the scrollbar to not redraw... */
#if 0
	pos.x++;

	PgSetUserClip( &rect );
	PgDrawText( s, len, &pos, 0 );
	PgSetUserClip( NULL );
#else
	rect.lr.y -= ( p_linespace + 1 ) / 2;
	PgDrawTextArea( s, len, &rect, Pg_TEXT_BOTTOM );
#endif
    }

    if( flags & DRAW_UNDERL )
	PgSetUnderline( Pg_TRANSPARENT, Pg_TRANSPARENT, 0 );

    DRAW_END;
}

/****************************************************************************/
/* Cursor */

    void
gui_mch_draw_hollow_cursor(guicolor_T color)
{
    PhRect_t r;

    /* FIXME: Double width characters */

    r.ul.x = FILL_X( gui.col );
    r.ul.y = FILL_Y( gui.row );
    r.lr.x = r.ul.x + gui.char_width - 1;
    r.lr.y = r.ul.y + gui.char_height - 1;

    DRAW_START;
    PgSetStrokeColor( color );
    PgDrawRect( &r, Pg_DRAW_STROKE );
    DRAW_END;
}

    void
gui_mch_draw_part_cursor(int w, int h, guicolor_T color)
{
    PhRect_t r;

    r.ul.x = FILL_X( gui.col );
    r.ul.y = FILL_Y( gui.row ) + gui.char_height - h;
    r.lr.x = r.ul.x + w - 1;
    r.lr.y = r.ul.y + h - 1;

    DRAW_START;
    gui_mch_set_bg_color( color );
    PgDrawRect( &r, Pg_DRAW_FILL );
    DRAW_END;
}

    void
gui_mch_set_blinking(long wait, long on, long off)
{
    blink_waittime = wait;
    blink_ontime = on;
    blink_offtime = off;
}

    void
gui_mch_start_blink(void)
{
    /* Only turn on the timer on if none of the times are zero */
    if( blink_waittime && blink_ontime && blink_offtime && gui.in_focus)
    {
	PtSetResource( gui_ph_timer_cursor, Pt_ARG_TIMER_INITIAL, blink_waittime, 0 );
	blink_state = BLINK_ON;
	gui_update_cursor(TRUE, FALSE);
    }
}

    void
gui_mch_stop_blink(void)
{
    PtSetResource( gui_ph_timer_cursor, Pt_ARG_TIMER_INITIAL, 0, 0 );

    if( blink_state == BLINK_OFF )
	gui_update_cursor(TRUE, FALSE);

    blink_state = BLINK_NONE;
}

/****************************************************************************/
/* miscellaneous functions */

    void
gui_mch_beep(void)
{
    PtBeep();
}

    void
gui_mch_flash(int msec)
{
    PgSetFillXORColor( Pg_BLACK, Pg_WHITE );
    PgSetDrawMode( Pg_DRAWMODE_XOR );
    gui_mch_clear_all();
    gui_mch_flush();

    ui_delay( (long) msec, TRUE );

    gui_mch_clear_all();
    PgSetDrawMode( Pg_DRAWMODE_OPAQUE );
    gui_mch_flush();
}

    void
gui_mch_flush(void)
{
    PgFlush();
}

    void
gui_mch_set_text_area_pos(int x, int y, int w, int h)
{
    PhArea_t area = {{x, y}, {w, h}};

    PtSetResource( gui.vimTextArea, Pt_ARG_AREA, &area, 0 );
}

    int
gui_mch_haskey(char_u *name)
{
    int i;

    for (i = 0; special_keys[i].key_sym != 0; i++)
	if (name[0] == special_keys[i].vim_code0 &&
		 name[1] == special_keys[i].vim_code1)
	    return( OK );
    return( FAIL );
}

/****************************************************************************/
/* Menu */

#if defined( FEAT_MENU ) || defined( PROTO )
    void
gui_mch_enable_menu(int flag)
{
    if( flag != 0 )
    {
	PtRealizeWidget( gui.vimMenuBar );
	PtExtentWidgetFamily( gui.vimToolBarGroup );
    }
    else
	PtUnrealizeWidget( gui.vimMenuBar );
}

    void
gui_mch_set_menu_pos(int x, int y, int w, int h)
{
    /* Nothing */
}

/* Change the position of a menu button in the parent */
    static void
gui_ph_position_menu( PtWidget_t *widget, int priority )
{
    PtWidget_t	*traverse;
    vimmenu_T	*menu;

    traverse = PtWidgetChildBack( PtWidgetParent( widget ) );

    /* Iterate through the list of widgets in traverse, until
     * we find the position we want to insert our widget into */
    /* TODO: traverse from front to back, possible speedup? */
    while( traverse != NULL )
    {
	PtGetResource( traverse, Pt_ARG_POINTER, &menu, 0 );

	if( widget != traverse &&
		menu != NULL &&
		priority < menu->priority )
	{
	    /* Insert the widget before the current traverse widget */
	    PtWidgetInsert( widget, traverse, 1 );
	    return;
	}

	traverse = PtWidgetBrotherInFront( traverse );
    }
}

/* the index is ignored because it's not useful for our purposes */
    void
gui_mch_add_menu(vimmenu_T *menu, int index)
{
    vimmenu_T	*parent = menu->parent;
    int	    n;
    PtArg_t args[5];

    menu->submenu_id = menu->id = NULL;

    if( menu_is_menubar( menu->name ) )
    {
	char_u	*accel_key;

	/* FIXME: This hack doesn't seem to work if the menu has
	 * the Accelerator Key text */
	accel_key = vim_strchr( menu->name, '&' );
	if( accel_key != NULL )
	    accel_key++;

	/* Create the menu button */
	n = 0;
	PtSetArg( &args[ n++ ], Pt_ARG_TEXT_STRING, menu->dname, 0 );
	PtSetArg( &args[ n++ ], Pt_ARG_ACCEL_TEXT, menu->actext, 0 );
	PtSetArg( &args[ n++ ], Pt_ARG_ACCEL_KEY, accel_key, 0 );
	PtSetArg( &args[ n++ ], Pt_ARG_POINTER, menu, 0 );

	if( parent != NULL )
	{
	    PtSetArg( &args[ n++ ], Pt_ARG_BUTTON_TYPE,
		    Pt_MENU_RIGHT, 0 );
	}

	menu->id = PtCreateWidget( PtMenuButton,
		(parent == NULL) ? gui.vimMenuBar : parent->submenu_id,
		n, args );

	PtAddCallback( menu->id, Pt_CB_ARM, gui_ph_handle_pulldown_menu, menu );

	/* Create the actual menu */
	n = 0;
	if( parent != NULL )
	{
	    PtSetArg( &args[ n++ ], Pt_ARG_MENU_FLAGS, Pt_TRUE, Pt_MENU_CHILD );
	}
	menu->submenu_id = PtCreateWidget( PtMenu, menu->id, n, args );

	if( parent == NULL )
	{
	    PtAddCallback( menu->submenu_id, Pt_CB_UNREALIZED,
		    gui_ph_handle_menu_unrealized, menu );
	}

	if( menu->mnemonic != 0 )
	{
	    PtAddHotkeyHandler( gui.vimWindow, tolower( menu->mnemonic ), Pk_KM_Alt,
		    0 ,
		    menu, gui_ph_handle_pulldown_menu );
	}

	gui_ph_position_menu( menu->id, menu->priority );

	/* Redraw menubar here instead of gui_mch_draw_menubar */
	if( gui.menu_is_active )
	    PtRealizeWidget( menu->id );

    }
    else if( menu_is_popup( menu->name ) )
    {
	menu->submenu_id = PtCreateWidget( PtMenu, gui.vimWindow, 0, NULL );
	PtAddCallback( menu->submenu_id, Pt_CB_UNREALIZED,
		gui_ph_handle_menu_unrealized, menu );
    }
}

    void
gui_mch_add_menu_item(vimmenu_T *menu, int index)
{
    vimmenu_T	*parent = menu->parent;
    int	    n;
    PtArg_t args[5];

    n = 0;
    PtSetArg( &args[ n++ ], Pt_ARG_POINTER, menu, 0 );

#ifdef FEAT_TOOLBAR
    if( menu_is_toolbar( parent->name ) )
    {
	if( menu_is_separator( menu->name ) )
	{
	    PtSetArg( &args[ n++ ], Pt_ARG_SEP_FLAGS,
		    Pt_SEP_ORIENTATION, Pt_SEP_VERTICAL );
	    PtSetArg( &args[ n++ ], Pt_ARG_SEP_TYPE, Pt_ETCHED_IN, 0 );
	    menu->id = PtCreateWidget( PtSeparator, gui.vimToolBar, n, args );
	}
	else
	{
	    PtSetArg( &args[ n++ ], Pt_ARG_TEXT_STRING, menu->dname, 0 );
	    PtSetArg( &args[ n++ ], Pt_ARG_FLAGS, Pt_FALSE, Pt_HIGHLIGHTED );
	    PtSetArg( &args[ n++ ], Pt_ARG_LABEL_BALLOON,
		    gui_ph_show_tooltip, 0 );
	    PtSetArg( &args[ n++ ], Pt_ARG_LABEL_FLAGS,
		    Pt_TRUE, Pt_SHOW_BALLOON );
	    menu->id = PtCreateWidget( PtButton, gui.vimToolBar, n, args );

	    PtAddCallback( menu->id, Pt_CB_ACTIVATE, gui_ph_handle_menu, menu );
	}
    }

    else
#endif
	if( menu_is_separator( menu->name ) )
    {
	menu->id = PtCreateWidget( PtSeparator, parent->submenu_id, n, args );
    }
    else
    {
	char_u *accel_key;

	accel_key = vim_strchr( menu->name, '&' );
	if( accel_key != NULL )
	    accel_key++;

	PtSetArg( &args[ n++ ], Pt_ARG_TEXT_STRING, menu->dname, 0 );
	PtSetArg( &args[ n++ ], Pt_ARG_ACCEL_KEY, accel_key, 0 );
	PtSetArg( &args[ n++ ], Pt_ARG_ACCEL_TEXT, menu->actext, 0 );

	menu->id = PtCreateWidget( PtMenuButton, parent->submenu_id, n, args );

	PtAddCallback( menu->id, Pt_CB_ACTIVATE, gui_ph_handle_menu, menu );

#ifdef USE_PANEL_GROUP
	if( gui_ph_is_buffer_item( menu, parent ) == TRUE )
	{
	    PtAddCallback( menu->id, Pt_CB_DESTROYED,
		    gui_ph_handle_buffer_remove, menu );
	    gui_ph_pg_add_buffer( menu->dname );
	}
#endif
    }

    gui_ph_position_menu( menu->id, menu->priority );
}

    void
gui_mch_destroy_menu(vimmenu_T *menu)
{
    if( menu->submenu_id != NULL )
	PtDestroyWidget( menu->submenu_id );
    if( menu->id != NULL )
	PtDestroyWidget( menu->id );

    menu->submenu_id = NULL;
    menu->id = NULL;
}

    void
gui_mch_menu_grey(vimmenu_T *menu, int grey)
{
    long    flags, mask, fields;

    if( menu->id == NULL )
	return;

    flags = PtWidgetFlags( menu->id );
    if( PtWidgetIsClass( menu->id, PtMenuButton ) )
    {
	fields = Pt_FALSE;
	mask = Pt_SELECTABLE;
    }
    else
    {
	fields = Pt_TRUE;
	mask = Pt_BLOCKED | Pt_GHOST;
    }

    if( ! grey )
	fields = ~fields;

    PtSetResource( menu->id, Pt_ARG_FLAGS, fields,
	    mask );
}

    void
gui_mch_menu_hidden(vimmenu_T *menu, int hidden)
{
    /* TODO: [un]realize the widget? */
}

    void
gui_mch_draw_menubar(void)
{
    /* The only time a redraw is needed is when a menu button
     * is added to the menubar, and that is detected and the bar
     * redrawn in gui_mch_add_menu_item
     */
}

    void
gui_mch_show_popupmenu(vimmenu_T *menu)
{
    PtSetResource( menu->submenu_id, Pt_ARG_POS, &abs_mouse, 0 );
    PtRealizeWidget( menu->submenu_id );
}

    void
gui_mch_toggle_tearoffs(int enable)
{
    /* No tearoffs yet */
}

#endif

#if defined( FEAT_TOOLBAR ) || defined( PROTO )
    void
gui_mch_show_toolbar(int showit)
{
    if( showit )
	PtRealizeWidget( gui.vimToolBar );
    else
	PtUnrealizeWidget( gui.vimToolBar );
}
#endif

/****************************************************************************/
/* Fonts */

    static GuiFont
gui_ph_get_font(
	char_u	*font_name,
	int_u	font_flags,
	int_u	font_size,
	/* Check whether the resulting font has the font flags and size that
	 * was asked for */
	int_u	enforce
	)
{
    char_u	    *font_tag;
    FontQueryInfo   info;
    int_u	    style;

    font_tag = alloc( MAX_FONT_TAG );
    if( font_tag != NULL )
    {
	if( PfGenerateFontName( font_name, font_flags, font_size,
		    font_tag ) != NULL )
	{
	    /* Enforce some limits on the font used */
	    style = PHFONT_INFO_FIXED;

	    if( enforce & PF_STYLE_BOLD )
		style |= PHFONT_INFO_BOLD;
	    if( enforce & PF_STYLE_ANTIALIAS )
		style |= PHFONT_INFO_ALIAS;
	    if( enforce & PF_STYLE_ITALIC )
		style |= PHFONT_INFO_ITALIC;

	    PfQueryFontInfo( font_tag, &info );

	    if( info.size == 0 )
		font_size = 0;

	    /* Make sure font size matches, and that the font style
	     * at least has the bits we're checking for */
	    if( font_size == info.size &&
		    style == (info.style & style) )
		return( (GuiFont) font_tag );
	}
	vim_free( font_tag );
    }
    return( NULL );
}

/*
 * Split up the vim font name
 *
 * vim_font is in the form of
 * <name>:s<height>:a:b:i
 *
 * a = antialias
 * b = bold
 * i = italic
 *
 */

    static int
gui_ph_parse_font_name(
	char_u *vim_font,
	char_u **font_name,
	int_u *font_flags,
	int_u *font_size )
{
    char_u  *mark;
    int_u   name_len, size;

    mark = vim_strchr( vim_font, ':' );
    if( mark == NULL )
	name_len = STRLEN( vim_font );
    else
	name_len = (int_u) ( mark - vim_font );

    *font_name = vim_strnsave( vim_font, name_len );
    if( *font_name != NULL )
    {
	if( mark != NULL )
	{
	    while( *mark != NUL && *mark++ == ':')
	    {
		switch( tolower( *mark++ ) )
		{
		    case 'a': *font_flags |= PF_STYLE_ANTIALIAS; break;
		    case 'b': *font_flags |= PF_STYLE_BOLD; break;
		    case 'i': *font_flags |= PF_STYLE_ITALIC; break;

		    case 's':
			size = getdigits( &mark );
			/* Restrict the size to some vague limits */
			if( size < 1 || size > 100 )
			    size = 8;

			*font_size = size;
			break;

		    default:
			break;
		}
	    }
	}
	return( TRUE );
    }
    return( FALSE );
}

    int
gui_mch_init_font(char_u *vim_font_name, int fontset)
{
    char_u  *font_tag;
    char_u  *font_name = NULL;
    int_u   font_flags = 0;
    int_u   font_size  = 12;

    FontQueryInfo info;
    PhRect_t extent;

    if( vim_font_name == NULL )
    {
	/* Default font */
	vim_font_name = "PC Term";
    }

    if( STRCMP( vim_font_name, "*" ) == 0 )
    {
	font_tag = PtFontSelection( gui.vimWindow, NULL, NULL,
		"pcterm12", -1, PHFONT_FIXED, NULL );

	if( font_tag == NULL )
	    return( FAIL );

	gui_mch_free_font( gui.norm_font );
	gui.norm_font = font_tag;

	PfQueryFontInfo( font_tag, &info );
	font_name = vim_strsave( info.font );
    }
    else
    {
	if( gui_ph_parse_font_name( vim_font_name, &font_name, &font_flags,
		    &font_size ) == FALSE )
	    return( FAIL );

	font_tag = gui_ph_get_font( font_name, font_flags, font_size, 0 );
	if( font_tag == NULL )
	{
	    vim_free( font_name );
	    return( FAIL );
	}
	gui_mch_free_font( gui.norm_font );
	gui.norm_font = font_tag;
    }

    gui_mch_free_font( gui.bold_font );
    gui.bold_font = gui_ph_get_font( font_name, font_flags | PF_STYLE_BOLD,
	    font_size, PF_STYLE_BOLD );

    gui_mch_free_font( gui.ital_font );
    gui.ital_font = gui_ph_get_font( font_name, font_flags | PF_STYLE_ITALIC,
	    font_size, PF_STYLE_ITALIC );

    /* This extent was brought to you by the letter 'g' */
    PfExtentText( &extent, NULL, font_tag, "g", 1 );

    gui.char_width = extent.lr.x - extent.ul.x + 1;
    gui.char_height = (- extent.ul.y) + extent.lr.y + 1;
    gui.char_ascent = - extent.ul.y;

    vim_free( font_name );
    return( OK );
}

    int
gui_mch_adjust_charsize(void)
{
    FontQueryInfo info;

    PfQueryFontInfo( gui.norm_font, &info );

    gui.char_height = - info.ascender + info.descender + p_linespace;
    gui.char_ascent = - info.ascender + p_linespace / 2;

    return( OK );
}


    GuiFont
gui_mch_get_font(char_u *vim_font_name, int report_error)
{
    char_u  *font_name;
    char_u  *font_tag;
    int_u   font_size = 12;
    int_u   font_flags = 0;

    if( gui_ph_parse_font_name( vim_font_name, &font_name, &font_flags,
		&font_size ) != FALSE )
    {
	font_tag = gui_ph_get_font( font_name, font_flags, font_size, -1 );
	vim_free( font_name );

	if( font_tag != NULL )
	    return( (GuiFont) font_tag );
    }

    if( report_error )
	EMSG2("E235: Unknown font: %s", vim_font_name );

    return( FAIL );
}

    void
gui_mch_set_font(GuiFont font)
{
    PgSetFont( font );
}

    void
gui_mch_free_font(GuiFont font)
{
    vim_free( font );
}

