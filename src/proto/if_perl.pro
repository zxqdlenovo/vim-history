/* auto/if_perl.c */
int perl_enabled __ARGS((void));
void perl_end __ARGS((void));
void msg_split __ARGS((char_u *s, int attr));
void perl_win_free __ARGS((win_t *wp));
void perl_buf_free __ARGS((buf_t *bp));
void ex_perl __ARGS((exarg_t *eap));
void ex_perldo __ARGS((exarg_t *eap));
