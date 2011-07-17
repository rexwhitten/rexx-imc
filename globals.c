/* The global variables of REXX/imc            (C) Ian Collier 1993 */

#include<stdio.h>
#include<setjmp.h>
#include<sys/types.h>
#include<sys/time.h>
#include"const.h"
#include"globals.h"
#define INCL_RXSYSEXIT
#include"rexxsaa.h"

char fname[maxvarname];        /* the current file name */
char extension[maxextension];  /* the default file extension */
int extlen;                    /* and its length */
int lines;                     /* number of lines in current source */
int stmts;                     /* number of statements in current program*/
char **source=0;               /* the source of the current program */
program *prog=0;               /* the current program, tokenised */
char *labelptr=cnull;          /* start of label table */
int *varstk=inull;             /* offsets to levels in variable table */
unsigned varstklen;            /* number of elements allocated to varstk */
int varstkptr;                 /* current level number */
char *vartab=cnull;            /* start of variable table */
unsigned vartablen;            /* space allocated to vartab */
char *cstackptr=cnull;         /* start of calculator stack */
unsigned ecstackptr;           /* length of calculator stack */
unsigned cstacklen;            /* space allocated to calculator stack */
char *workptr=cnull;           /* start of workspace */
unsigned eworkptr;             /* amount of workspace used */
unsigned worklen;              /* space allocated to workspace */
char *pstackptr;               /* start of program stack */
unsigned epstackptr;           /* length of program stack */
unsigned pstacklen;            /* space allocated to program stack */
int pstacklev=0;               /* program stack level within current func */
char *hashptr[3]={0,0,0};      /* pointers to 3 hash tables */
unsigned ehashptr[3]={0,0,0};  /* lengths of hash tables */
unsigned hashlen[3]={0,0,0};   /* space allocated to hash tables */
struct sigstruct *sgstack;     /* contexts of all calls to interpreter() */
                               /* and which signals to catch */
unsigned sigstacklen=0;        /* number of elements allocated to sgstack */
int delayed[Imax];             /* Which signals are pending action */
char *sigdata[Imax]={0,0,0,0,0,0}; /* Descriptions for pending signals */
jmp_buf interactbuf;           /* context of interactive trace routine */
jmp_buf *exitbuf;              /* where to go on exit */
int interplev=-1;              /* nesting level of interpreter() */
int interact= -1;              /* interplev of interactive trace, or -1 */
int interactmsg=0;             /* Whether to emit message at interactive() */
int ppc=-1;                    /* Statement number being interpreted */
int oldppc;                    /* saved position in case of error */
int newppc;                    /* statement to jump to */
char **curargs;                /* current arguments, for ARG() */
int *curarglen;                /* lengths of current arguments */
char *errordata=cnull;         /* data to print after error message */
int precision=9;               /* NUMERIC DIGITS */
int fuzz=9;                    /* NUMERIC DIGITS minus NUMERIC FUZZ */
char numform=0;                /* 0=SCIENTIFIC, 1=ENGINEERING */
char trcflag=Tfailures;        /* what to trace */
char trcresult=0;              /* nesting level of scanning() */
char otrcflag;                 /* temporary store for trace flag */
char trclp;                    /* whether to get input in interactive mode */
char timeflag=0;               /* bit 0: used elapsed time counter */
                               /* bit 1: timestamp in operation */
                               /* bit 2: used random number generator */
long secs;long microsecs;      /* timestamp when TIME(r) was last called */
struct timeval timestamp;      /* the timestamp, if in operation */
int address0=0;                /* The initial environment */
int address1=0;                /* The current environment */
int address2=0;                /* The alternate environment */
char *rxpath;                  /* where to find libraries if not in REXXLIB */
FILE *ttyin,*ttyout;           /* access to the terminal */
char *pull=cnull;              /* storage used for input */
unsigned pulllen=0;            /* space allocated for pull */
char *returnval;               /* value returned by EXIT */
int returnlen;                 /* length of that value */
char *returnfree;              /* what to free after returnval has been used */
int rxstacksock;               /* fd number of stack socket */
char *varnamebuf;              /* workspace for evaluating variable names */
int varnamelen;                /* amount of memory allocated for varnamebuff */
FILE *traceout=0;              /* Where to send trace output */
int lasterror=0;               /* The last error which occurred on I/O */
int setrcflag=0;               /* flag for "options setrc" */
int exposeflag=0;              /* flag for "options expose" */
int sigpipeflag=0;             /* flag for "options sigpipe" */
varent *nextvar=0;             /* next variable for RXSHV_NEXTV */


long (*exitlist[RXEXITNUM])(); /* list of exit addresses */

char *conditions[]=   /* condition names in the same order as their I-values */
         {"SYNTAX","ERROR","HALT","NOVALUE","FAILURE","NOTREADY"};

/* Character classification tables */

/*                0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F       *
                { ., ., ., ., ., ., ., ., .,\t,\n, ., .,\r, ., .,  * 0 *
                  ., ., ., ., ., ., ., ., ., ., ., ., ., ., ., .,  * 1 *
                   , !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /,  * 2 *
                  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, :, ;, <, =, >, ?,  * 3 *
                  @, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O,  * 4 *
                  P, Q, R, S, T, U, V, W, X, Y, Z, [, \, ], ^, _,  * 5 *
                  `, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o,  * 6 *
                  p, q, r, s, t, u, v, w, x, y, z, {, |, }, ~, .}; * 7 */

/* Table of types: -2 invalid, -1 tokenisable special char, 1 symbol char,
                    2 number, 0 other */
/*                0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F       */
 int types[256]={-2,-2,-2,-2,-2,-2,-2,-2,-2, 0,-2,-2,-2, 0,-2,-2, /* 0 */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* 1 */
                  0, 1, 0, 1, 1, 0,-1, 0, 0, 0,-1, 0, 0, 0, 1,-1, /* 2 */
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0,-1,-1,-1, 1, /* 3 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-2,-1,-2,-1, 1, /* 5 */
                 -2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 6 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-2,-1,-2,-2,-2, /* 7 */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* 8 */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* 9 */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* A */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* B */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* C */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* D */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, /* E */
                 -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2};/* F */

/* Alphanum table: 1 letter, 2 number, 3 dot, 0 other */
/*                0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F       */
 int alphs[256]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0 */
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 1 */
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, /* 2 */
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, /* 3 */
                  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 5 */
                  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 6 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};/* 7 */

/* Symbol table: 0 if invalid in a simple symbol, -1 if valid except at
                 start, 1 otherwise */
/*                0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F       */
 int symbs[256]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0 */
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 1 */
                  0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 2 */
                 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, /* 3 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4 */
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /* 5 */
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 6 */
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};/* 7 */

