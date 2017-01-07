#ifndef TERMINAL_H
#define TERMINAL_H

#ifdef TERMINAL_C
# define TERMINAL_TERMCAP_ERROR 0
# define TERMINAL_TERMCAP_CREATED 1
# define TERMINAL_TERMCAP_CACHED 2
#endif

#define TERMINAL_DEFAULT_WIDTH 80
#define TERMINAL_DEFAULT_HEIGHT 18

#define TERMINAL_NAME_SZ 32

/* 1024 just isn't big enough -- glibc says so */
#define TERMINAL_TERMCAP_SZ (1024 * 2)

#define TERMINAL_OVERRIDE_ANSI_COLOURS_FORE "\033[3%dm"
#define TERMINAL_OVERRIDE_ANSI_COLOURS_BACK "\033[4%dm"

typedef struct terminal_termcap
{
 struct terminal_termcap *next;

 int ref_count;

 int pc; /* not really used */
 int sg;
 int ug;
 
 bitflag am : 1;
 bitflag ms : 1;
 bitflag xn : 1;
 bitflag xs : 1;
 
 const char *up;
 const char *ce;
 const char *cl;
 
 const char *mb;
 const char *md;
 const char *mh;
 const char *mk;
 const char *mr;
 const char *me;
 
 const char *us;
 const char *ue;
 
 const char *Sf;
 const char *Sb;
 
 const char *bl;
 const char *vb;

 char name[TERMINAL_NAME_SZ];
 
 char data[TERMINAL_TERMCAP_SZ];
} terminal_termcap;


#endif
