/* mark.c */
extern int setmark __ARGS((int c));
extern void setpcmark __ARGS((void));
extern void checkpcmark __ARGS((void));
extern pos_T *movemark __ARGS((int count));
extern pos_T *movechangelist __ARGS((int count));
extern pos_T *getmark __ARGS((int c, int changefile));
extern pos_T *getnextmark __ARGS((pos_T *startpos, int dir, int begin_line));
extern void fmarks_check_names __ARGS((buf_T *buf));
extern int check_mark __ARGS((pos_T *pos));
extern void clrallmarks __ARGS((buf_T *buf));
extern char_u *fm_getname __ARGS((fmark_T *fmark, int lead_len));
extern void do_marks __ARGS((exarg_T *eap));
extern void ex_jumps __ARGS((exarg_T *eap));
extern void ex_changes __ARGS((exarg_T *eap));
extern void mark_adjust __ARGS((linenr_T line1, linenr_T line2, long amount, long amount_after));
extern void mark_col_adjust __ARGS((linenr_T lnum, colnr_T mincol, long lnum_amount, long col_amount));
extern void copy_jumplist __ARGS((win_T *from, win_T *to));
extern void free_jumplist __ARGS((win_T *wp));
extern void set_last_cursor __ARGS((win_T *win));
extern int read_viminfo_filemark __ARGS((vir_T *virp, int force));
extern void write_viminfo_filemarks __ARGS((FILE *fp));
extern int removable __ARGS((char_u *name));
extern int write_viminfo_marks __ARGS((FILE *fp_out));
extern void copy_viminfo_marks __ARGS((vir_T *virp, FILE *fp_out, int count, int eof));
/* vim: set ft=c : */
