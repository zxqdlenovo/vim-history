/* os_mswin.c */
void mch_windexit __ARGS((int r));
void mch_init __ARGS((void));
int mch_input_isatty __ARGS((void));
void mch_settitle __ARGS((char_u *title, char_u *icon));
void mch_restore_title __ARGS((int which));
int mch_can_restore_title __ARGS((void));
int mch_can_restore_icon __ARGS((void));
int mch_FullName __ARGS((char_u *fname, char_u *buf, int len, int force));
int mch_isFullName __ARGS((char_u *fname));
void slash_adjust __ARGS((char_u *p));
int vim_stat __ARGS((const char *name, struct stat *stp));
void mch_settmode __ARGS((int tmode));
int mch_get_shellsize __ARGS((void));
void mch_set_shellsize __ARGS((void));
void mch_new_shellsize __ARGS((void));
void mch_suspend __ARGS((void));
void display_errors __ARGS((void));
int mch_has_wildcard __ARGS((char_u *s));
int mch_chdir __ARGS((char *path));
int can_end_termcap_mode __ARGS((int give_msg));
int mch_screenmode __ARGS((char_u *arg));
int mch_libcall __ARGS((char_u *libname, char_u *funcname, char_u *argstring, int argint, char_u **string_result, int *number_result));
int clip_mch_own_selection __ARGS((VimClipboard *cbd));
void clip_mch_lose_selection __ARGS((VimClipboard *cbd));
void clip_mch_request_selection __ARGS((VimClipboard *cbd));
void clip_mch_set_selection __ARGS((VimClipboard *cbd));
void DumpPutS __ARGS((const char *psz));
/* vim: set ft=c : */
