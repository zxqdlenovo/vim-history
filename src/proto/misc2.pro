/* misc2.c */
extern int virtual_active __ARGS((void));
extern int getviscol __ARGS((void));
extern int getviscol2 __ARGS((colnr_T col, colnr_T coladd));
extern int coladvance_force __ARGS((colnr_T wcol));
extern int coladvance __ARGS((colnr_T wcol));
extern int getvpos __ARGS((pos_T *pos, colnr_T wcol));
extern int inc_cursor __ARGS((void));
extern int inc __ARGS((pos_T *lp));
extern int incl __ARGS((pos_T *lp));
extern int dec_cursor __ARGS((void));
extern int dec __ARGS((pos_T *lp));
extern int decl __ARGS((pos_T *lp));
extern void check_cursor_lnum __ARGS((void));
extern void check_cursor_col __ARGS((void));
extern void check_cursor __ARGS((void));
extern void adjust_cursor_col __ARGS((void));
extern int leftcol_changed __ARGS((void));
extern void vim_mem_profile_dump __ARGS((void));
extern char_u *alloc __ARGS((unsigned size));
extern char_u *alloc_clear __ARGS((unsigned size));
extern char_u *alloc_check __ARGS((unsigned size));
extern char_u *lalloc_clear __ARGS((long_u size, int message));
extern char_u *lalloc __ARGS((long_u size, int message));
extern void *mem_realloc __ARGS((void *ptr, size_t size));
extern void do_outofmem_msg __ARGS((long_u size));
extern char_u *vim_strsave __ARGS((char_u *string));
extern char_u *vim_strnsave __ARGS((char_u *string, int len));
extern char_u *vim_strsave_escaped __ARGS((char_u *string, char_u *esc_chars));
extern char_u *vim_strsave_escaped_ext __ARGS((char_u *string, char_u *esc_chars, int bsl));
extern char_u *vim_strsave_up __ARGS((char_u *string));
extern char_u *vim_strnsave_up __ARGS((char_u *string, int len));
extern void vim_strup __ARGS((char_u *p));
extern void copy_spaces __ARGS((char_u *ptr, size_t count));
extern void copy_chars __ARGS((char_u *ptr, size_t count, int c));
extern void del_trailing_spaces __ARGS((char_u *ptr));
extern void vim_strncpy __ARGS((char_u *to, char_u *from, int len));
extern int copy_option_part __ARGS((char_u **option, char_u *buf, int maxlen, char *sep_chars));
extern void vim_free __ARGS((void *x));
extern int vim_stricmp __ARGS((char *s1, char *s2));
extern int vim_strnicmp __ARGS((char *s1, char *s2, size_t len));
extern char_u *vim_strchr __ARGS((char_u *string, int c));
extern char_u *vim_strrchr __ARGS((char_u *string, int c));
extern int vim_isspace __ARGS((int x));
extern void ga_clear __ARGS((garray_T *gap));
extern void ga_clear_strings __ARGS((garray_T *gap));
extern void ga_init __ARGS((garray_T *gap));
extern void ga_init2 __ARGS((garray_T *gap, int itemsize, int growsize));
extern int ga_grow __ARGS((garray_T *gap, int n));
extern void ga_concat __ARGS((garray_T *gap, char_u *s));
extern void ga_append __ARGS((garray_T *gap, int c));
extern int name_to_mod_mask __ARGS((int c));
extern int simplify_key __ARGS((int key, int *modifiers));
extern char_u *get_special_key_name __ARGS((int c, int modifiers));
extern int trans_special __ARGS((char_u **srcp, char_u *dst, int keycode));
extern int find_special_key __ARGS((char_u **srcp, int *modp, int keycode));
extern int extract_modifiers __ARGS((int key, int *modp));
extern int find_special_key_in_table __ARGS((int c));
extern int get_special_key_code __ARGS((char_u *name));
extern char_u *get_key_name __ARGS((int i));
extern int get_mouse_button __ARGS((int code, int *is_click, int *is_drag));
extern int get_pseudo_mouse_code __ARGS((int button, int is_click, int is_drag));
extern int get_fileformat __ARGS((buf_T *buf));
extern int get_fileformat_force __ARGS((buf_T *buf, exarg_T *eap));
extern void set_fileformat __ARGS((int t, int opt_flags));
extern int default_fileformat __ARGS((void));
extern int call_shell __ARGS((char_u *cmd, int opt));
extern int get_real_state __ARGS((void));
extern int vim_chdirfile __ARGS((char_u *fname));
extern int illegal_slash __ARGS((char *name));
extern char_u *parse_shape_opt __ARGS((int what));
extern int get_shape_idx __ARGS((int mouse));
extern void update_mouseshape __ARGS((int shape_idx));
extern int decrypt_byte __ARGS((void));
extern int update_keys __ARGS((int c));
extern void crypt_init_keys __ARGS((char_u *passwd));
extern char_u *get_crypt_key __ARGS((int store, int twice));
extern void *vim_findfile_init __ARGS((char_u *path, char_u *filename, char_u *stopdirs, int level, int free_visited, int need_dir, void *search_ctx, int tagfile, char_u *rel_fname));
extern char_u *vim_findfile_stopdir __ARGS((char_u *buf));
extern void vim_findfile_cleanup __ARGS((void *ctx));
extern char_u *vim_findfile __ARGS((void *search_ctx));
extern void vim_findfile_free_visited __ARGS((void *search_ctx));
extern char_u *find_file_in_path __ARGS((char_u *ptr, int len, int options, int first, char_u *rel_fname));
extern char_u *find_directory_in_path __ARGS((char_u *ptr, int len, int options, char_u *rel_fname));
extern int vim_chdir __ARGS((char_u *new_dir));
extern int get_user_name __ARGS((char_u *buf, int len));
extern void sort_strings __ARGS((char_u **files, int count));
extern int pathcmp __ARGS((const char *p, const char *q));
extern char_u *parse_list_options __ARGS((char_u *option_str, option_table_T *table, int table_size));
extern void msg_str __ARGS((char_u *s, char_u *arg));
/* vim: set ft=c : */
