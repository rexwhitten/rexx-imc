/* The external variables of REXX/imc          (C) Ian Collier 1992 */

#ifndef _globals_h
#define _globals_h

/* Protection against things not having been included... */
#ifndef putchar
#include<stdio.h>
#endif
#ifndef Einit
#include"const.h"
#endif
#include<setjmp.h>

extern int types[];                /* Three character type tables */
extern int alphs[];
extern int symbs[];
extern char fname[];               /* the current file name */
extern char extension[];           /* the default file extension */
extern int extlen;                 /* and its length */
extern int lines;                  /* number of lines in current source */
extern int stmts;                  /* number of statements in current program*/
extern char **source;              /* the source of the current program */
extern program *prog;              /* the current program, tokenised */
extern char *labelptr;             /* start of label table */
extern char *vartab;               /* start of variable table */
extern int *varstk;                /* offsets to levels in variable table */
extern unsigned varstklen;         /* number of elements allocated to varstk */
extern unsigned vartablen;         /* space allocated to vartab */
extern int varstkptr;              /* current level number */
extern char *cstackptr;            /* start of calculator stack */
extern unsigned ecstackptr;        /* length of calculator stack */
extern unsigned cstacklen;         /* space allocated to calculator stack */
extern char *workptr;              /* start of workspace */
extern unsigned eworkptr;          /* amount of workspace used */
extern unsigned worklen;           /* space allocated to workspace */
extern char *pstackptr;            /* start of program stack */
extern unsigned epstackptr;        /* length of program stack */
extern unsigned pstacklen;         /* space allocated to program stack */
extern int pstacklev;              /* prog stack level within current func */
extern char* hashptr[];            /* pointers to 3 hash tables */
extern unsigned ehashptr[];        /* lengths of hash tables */
extern unsigned hashlen[];         /* space allocated to hash tables */
extern struct sigstruct {          /* contexts of all calls to interpreter() */
       short bits;                 /* and which signals to catch by default */
       short bitson;               /* which signals are caught here */
       short callon;               /* which signals cause a call */
       short delay;                /* which signals are set to "delay" */
       char type;                  /* condition(i) 0=none, 1=signal, 2=call */
       char which;                 /* condition(c) */
       char *data;                 /* condition(d) */
       int ppc[Imax];              /* number of the SIGNAL ON instruction */
         jmp_buf jmp;              /* for each signal */
      } *sgstack;
extern unsigned sigstacklen;       /* number of elements allocated to sgstack */
extern int delayed[Imax];          /* Which signals are pending action */
extern char *sigdata[Imax];        /* Descriptions for pending signals */
extern jmp_buf interactbuf;        /* context of interactive trace routine */
extern jmp_buf *exitbuf;           /* where to go on exit */
extern int interplev;              /* nesting level of interpreter() */
extern int interact;               /* interplev of interactive trace, or -1 */
extern int interactmsg;            /* Whether to emit message at interactive */
extern int ppc;                    /* statement number being interpreted */
extern int oldppc;                 /* saved position in case of error */
extern int newppc;                 /* statement to jump to */
extern char **curargs;             /* current arguments, for ARG() */
extern int *curarglen;             /* lengths of current arguments */
extern char *errordata;            /* data to print after error message */
extern int precision;              /* NUMERIC DIGITS */
extern int fuzz;                   /* NUMERIC DIGITS minus NUMERIC FUZZ */
extern char numform;               /* 0=SCIENTIFIC, 1=ENGINEERING */
extern char trcflag;               /* what to trace */
extern char trcresult;             /* nesting level of scanning() */
extern char otrcflag;              /* temporary store for trace flag */
extern char trclp;                 /* whether to input in interactive mode */
extern char timeflag;              /* bit 0: used elapsed time counter */
                                   /* bit 1: timestamp in operation */
			           /* bit 2: used random number generator */
extern long secs;extern long microsecs; /* time when TIME(r) last called */
extern struct timeval timestamp;   /* the timestamp, if in operation */
extern int address0;               /* The initial environment */
extern int address1;               /* The current environment */
extern int address2;               /* The alternate environment */
extern char *rxpath;               /* where to find libraries if not in REXXLIB */
extern FILE *ttyin,*ttyout;        /* access to the terminal */
extern char *pull;                 /* storage used for input */
extern unsigned pulllen;           /* space allocated for pull */
extern char *returnval;            /* value returned by EXIT */
extern int returnlen;              /* length of that value */
extern char *returnfree;           /* what to free after returnval is used */
extern int rxstacksock;            /* fd number of stack socket */
extern char *varnamebuf;           /* workspace for evaluating variable names*/
extern int varnamelen;             /* amount of memory allocated for this */
extern FILE *traceout;             /* Where to send trace output */
extern int lasterror;              /* The last error which occurred on I/O */
extern int setrcflag;              /* flag for "options setrc" */
extern int exposeflag;             /* flag for "options expose" */
extern int sigpipeflag;            /* flag for "options sigpipe" */
extern struct _varent *nextvar;    /* next variable for RXSHV_NEXTV */


extern char *conditions[];         /* the condition names */

extern struct environ {            /* the list of environments */
   char name[maxenviron+1];
   unsigned long (*handler)();
   unsigned char *area;
   int defined;
} *envtable;

extern long (*exitlist[])(); /* list of exit addresses */

#endif /* !_globals_h */
