/* multibyte.c */
int enc_canon_props __ARGS((char_u *name));
char_u *mb_init __ARGS((void));
int bomb_size __ARGS((void));
int mb_get_class __ARGS((char_u *p));
int mb_class __ARGS((unsigned lead, unsigned trail));
int latin_char2len __ARGS((int c));
int latin_char2bytes __ARGS((int c, char_u *buf));
int latin_ptr2len_check __ARGS((char_u *p));
int utf_char2cells __ARGS((int c));
int latin_ptr2cells __ARGS((char_u *p));
int utf_ptr2cells __ARGS((char_u *p));
int latin_char2cells __ARGS((int c));
int latin_off2cells __ARGS((unsigned off));
int latin_ptr2char __ARGS((char_u *p));
int utf_ptr2char __ARGS((char_u *p));
int mb_ptr2char_adv __ARGS((char_u **pp));
int utfc_ptr2char __ARGS((char_u *p, int *p1, int *p2));
int utfc_char2bytes __ARGS((int off, char_u *buf));
int utf_ptr2len_check __ARGS((char_u *p));
int utf_byte2len __ARGS((int b));
int utf_ptr2len_check_len __ARGS((char_u *p, int size));
int utfc_ptr2len_check __ARGS((char_u *p));
int utf_char2len __ARGS((int c));
int utf_char2bytes __ARGS((int c, char_u *buf));
int utf_iscomposing __ARGS((int c));
void show_utf8 __ARGS((void));
int latin_head_off __ARGS((char_u *base, char_u *p));
int dbcs_head_off __ARGS((char_u *base, char_u *p));
int utf_head_off __ARGS((char_u *base, char_u *p));
int mb_off_next __ARGS((char_u *base, char_u *p));
int mb_tail_off __ARGS((char_u *base, char_u *p));
void mb_adjust_cursor __ARGS((void));
void mb_adjustpos __ARGS((pos_t *lp));
int mb_charlen __ARGS((char_u *str));
int mb_dec __ARGS((pos_t *lp));
int dbcs_isbyte1 __ARGS((char_u *buf, int x));
int mb_lefthalve __ARGS((int row, int col));
char_u *enc_skip __ARGS((char_u *p));
char_u *enc_canonize __ARGS((char_u *enc));
int enc_default __ARGS((void));
void *my_iconv_open __ARGS((char_u *to, char_u *from));
int iconv_enabled __ARGS((void));
void iconv_end __ARGS((void));
void xim_set_focus __ARGS((int focus));
void xim_set_preedit __ARGS((void));
void xim_set_status_area __ARGS((void));
void xim_init __ARGS((void));
void xim_decide_input_style __ARGS((void));
void xim_reset __ARGS((void));
int xim_queue_key_press_event __ARGS((GdkEvent *ev));
void xim_init __ARGS((void));
int xim_get_status_area_height __ARGS((void));
void convert_setup __ARGS((vimconv_t *vcp, char_u *from, char_u *to));
int convert_input __ARGS((char_u *ptr, int len, int maxlen));
char_u *string_convert __ARGS((vimconv_t *vcp, char_u *ptr, int *lenp));
char_u *keymap_init __ARGS((void));
void ex_loadkeymap __ARGS((exarg_t *eap));
/* vim: set ft=c : */
