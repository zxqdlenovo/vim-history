/* os_win32.c */
void PlatformId __ARGS((void));
int mch_windows95 __ARGS((void));
void mch_setmouse __ARGS((int on));
void mch_update_cursor __ARGS((void));
int mch_char_avail __ARGS((void));
int mch_inchar __ARGS((char_u *buf, int maxlen, long time));
void mch_shellinit __ARGS((void));
void mch_windexit __ARGS((int r));
int mch_check_win __ARGS((int argc, char **argv));
void fname_case __ARGS((char_u *name));
int mch_get_user_name __ARGS((char_u *s, int len));
void mch_get_host_name __ARGS((char_u *s, int len));
long mch_get_pid __ARGS((void));
int mch_dirname __ARGS((char_u *buf, int len));
long mch_getperm __ARGS((char_u *name));
int mch_setperm __ARGS((char_u *name, long perm));
void mch_hide __ARGS((char_u *name));
int mch_isdir __ARGS((char_u *name));
int mch_nodetype __ARGS((char_u *name));
vim_acl_t mch_get_acl __ARGS((char_u *fname));
void mch_set_acl __ARGS((char_u *fname, vim_acl_t acl));
void mch_free_acl __ARGS((vim_acl_t acl));
void mch_settmode __ARGS((int tmode));
int mch_get_shellsize __ARGS((void));
void mch_set_shellsize __ARGS((void));
void mch_new_shellsize __ARGS((void));
void mch_set_winsize_now __ARGS((void));
int mch_call_shell __ARGS((char_u *cmd, int options));
int mch_expandpath __ARGS((garray_t *gap, char_u *path, int flags));
void mch_set_normal_colors __ARGS((void));
void mch_write __ARGS((char_u *s, int len));
void mch_delay __ARGS((long msec, int ignoreinput));
int mch_remove __ARGS((char_u *name));
void mch_breakcheck __ARGS((void));
long_u mch_avail_mem __ARGS((int special));
int mch_libcall __ARGS((char_u *libname, char_u *funcname, char_u *argstring, int argint, char_u **string_result, int *number_result));
int mch_rename __ARGS((const char *pszOldFile, const char *pszNewFile));
char *default_shell __ARGS((void));
void clip_mch_request_selection __ARGS((void));
void clip_mch_set_selection __ARGS((void));
