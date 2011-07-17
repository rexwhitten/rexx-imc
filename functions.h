/* The functions of REXX/imc            This is a 132 column file            (C) Ian Collier 1992 */

#ifndef _functions_h
#define _functions_h

#ifdef __STDC__     /* define Args macro so that the arguments of each */
#define Args(a) a   /* function can be stated without confusing the */
#else               /* compiler */
#define Args(a) ()
#define volatile    /* Ignore "volatile" for traditional C compiler */
#endif

/* Protection against things not having been included... */
#ifndef putchar
#include<stdio.h>
#endif
#ifndef Einit
#include"const.h"
#endif


#ifdef DEBUG /* Debugging mtest routines */
int mtest_debug Args((char **memptr,unsigned *alloc,unsigned length,unsigned extend,long *diff));
#define mtest(memptr,alloc,length,extend) (mtest_debug(&memptr,&alloc,length,extend,(long *)0))
#define dtest(memptr,alloc,length,extend) (mtest_debug(&memptr,&alloc,length,extend,&mtest_diff))
#else /* not defined DEBUG */
/* Usually the mem test routines use inline code for greater efficiency.
   mtest: reallocate an area only if it is too small
   dtest: the same, but also return 1 if the area moved and set mtest_diff to
          the difference
   Recently fixed to save the old value when realloc fails
*/
#define mtest(memptr,alloc,length,extend) {char *old;         \
        (alloc<(length))&&(                                   \
        old=memptr,                                           \
        (memptr=realloc(memptr,(unsigned)(alloc+=(extend))))||(memptr=old,alloc-=(extend),die(Emem),1)  \
        );}
#define dtest(memptr,alloc,length,extend) (                   \
        (alloc<(length))&&(                                   \
        mtest_old=memptr,                                     \
        (memptr=realloc(memptr,(unsigned)(alloc+=(extend))))||(memptr=mtest_old,alloc-=(extend),die(Emem),1), \
        mtest_diff=memptr-mtest_old ))                      
#endif /* DEBUG */

#define whattype(c)      (types[(unsigned char)(c)])
#define alphanum(c)      (alphs[(unsigned char)(c)])
#define rexxsymboldot(c) (whattype(c)>0)
#define rexxsymbol(c)    (symbs[(unsigned char)(c)])

#ifdef NO_LDL                                                                   /* Declare the dynamic load functions */
void *dlopen Args((char*,int));                                                 /* either in situ or from the system header file */
void *dlsym Args((void *,char*));
char *dlerror Args((void));
int dlclose Args((void*));
#else
#include<dlfcn.h>
#endif /* NO_LDL */

/* In ANSI C, you can take the address of any object and the result is of type pointer to that object.  In K&R C, this
   fails when the object is an array.  The following kludge takes the address of a jmp_buf, which is likely to be an
   array.  On systems where it isn't, this macro has to be changed... */
#ifdef __STDC__
#define addressof(x) (&(x))
#else
#define addressof(x) ((jmp_buf*)(x))
#endif

#ifdef __hpux /* HP-UX doesn't have srandom, random or siginterrupt */
#define srandom srand48
#define random lrand48
#define siginterrupt(arg1,arg2) (0)
#endif

#ifdef sgi /* IRIX doesn't have siginterrupt or vfork */
#define siginterrupt(arg1,arg2) (0)
#define vfork fork
#endif

#ifdef Solaris /* Solaris doesn't have siginterrupt */
#define siginterrupt(arg1,arg2) (0)
#endif

#if (defined(sun) || defined(__sun__)) && !defined(Solaris) /* SunOS4 does not provide strerror */
  extern int errno;
  extern int sys_nerr;
  extern char *sys_errlist[];
# define strerror(x) ((x)>=0 && (x)<sys_nerr ? sys_errlist[x] : (char*)NULL)
  /* SunOS 4 is the only system to recommend getwd above getcwd. */
# define getcwd(name,len) getwd(name)
#endif

#ifdef RENAME_UNDELETE /* FreeBSD has undelete() defined in unistd.h so REXX/imc's will be renamed */
# define undelete rx_undelete
#endif

/* in rexx.c */
int main Args((int argc, char *argv[]));
char *allocm Args((unsigned size));
void die Args((int rc));
char *interpreter Args((int *anslen,int start,                                  /* Interpret program lines */
                        char *callname,long calltype,char *args[],int arglen[],int inherit,int delay));
static void doaddress Args((char **line,int env));                              /* Address a command to an environment */
static void parse Args((char *list[],int len[],int up,char *line,int *ptr));    /* PARSE strings with a template */
static char uc1 Args((int c,int up));                                           /* Uppercase c, if up */
static void pset1 Args((char *list,int listlen,char *val,int len,int up));      /* Tokenise a string into variables */
static void pset Args((char *varname,int namelen,char *val,int len,int up));    /* Assign a PARSE result to a variable */
static int findsigl Args((int *level));                                         /* Find most recent line number */
static void getcallargs Args((char *args[],int arglen[],int argc));             /* Unstack parameters in a CALL instruc */
int rxcall Args((int stmt,char *name,int argc,int lit,long calltype));          /* Call a procedure */
char *rxinterp Args((char *exp,int len,int *rlen,char *name,long calltype,      /* INTERPRET a string */
                     char *args[],int arglen[]));
static void doconds Args((void));                                               /* Check for delayed conditions and trap them */
void settrace Args((char *));                                                   /* Set trace according to the given option */
int setoption Args((char *option,int len));                                     /* Set an option from the OPTIONS instruction */
static int gettrap Args((char **lineptr,int on,int *stmt));                     /* Get a trap name after "call/signal on" */
static void testvarname Args((char **line,char *var,int len));                  /* Test the symbol against a stored name */
static void skipstmt Args((void));                                              /* Skip the current instruction */
static void stepdo Args((void));                                                /* Step past the current DO */
static void stepselect Args((void));                                            /* Step past the current SELECT */
static void stepif Args((void));                                                /* Step past the current IF */
static void stepwhen Args((void));                                              /* Step past the current WHEN */
static void findend Args((void));                                               /* Find the next END */
void on_halt Args((void));                                                      /* Find the line number at which halt occurred */

/* in calc.c */

char *scanning Args((char *line,int *ptr,int *len));                            /* Evaluate an expression */
void tracelast Args((char *type));                                              /* Trace the last value on the stack */
void traceline Args((char *type,char *exp,int len));                            /* Trace any string */
void stack Args((char *exp,int len));                                           /* Stack a string literally */
void stackq Args((char *exp,int len,int quote));                                /* Stack a string, collapsing quotes */
void stackx Args((char *exp,int len));                                          /* Stack a hex constant */
void stackb Args((char *exp,int len));                                          /* Stack a binary constant */
void stackint Args((int i));                                                    /* Stack an integer */
void stacknull Args((void));                                                    /* Stack a null parameter */
void binplus Args((int op));                                                    /* Add the last two values */
void binmin Args((int op));                                                     /* Subtract the last value from the previous */
void binmul Args((int op));                                                     /* Multiply the last two values */
void bindiv Args((int op));                                                     /* Divide the previous value by the last */
void binexp Args((int op));                                                     /* Raise the previous value to the last-th power */
void rxdup Args((void));                                                        /* Duplicate the last value */
void binrel Args((int op));                                                     /* Compare the last two values */
void binbool Args((int op));                                                    /* Combine the last two logical values */
void bincat Args((int op));                                                     /* Concatenate the last two values */
void unnot Args((int op));                                                      /* Negate logically the last value */
void unmin Args((int op));                                                      /* Negate arithmetically the last value */
void unplus Args((int op));                                                     /* Reformat the last numeric value */
void strip Args((void));                                                        /* Strip surrounding space from the last value */
int num Args((int *minus,int *exp,int *zero,int *len));                         /* Get the last value as a number */
int getint Args((int flg));                                                     /* Get the last value as an integer */
int isint Args((int num,int len,int exp));                                      /* Is the last value an integer? */
char *delete Args((int *len));                                                  /* Delete and return the last value */
int isnull Args((void));                                                        /* Is the last value null? */
void stacknum Args((char *num,int len,int exp,int minus));                      /* Stack a number */
void getvarname Args((char *line,int *ptr,char *varname,int *namelen,int maxlen)); /* Get a symbol from a program line */
void skipvarname Args((char *line,int *ptr));                                   /* Skip a symbol in a program line */
int gettoken Args((char *line,int *ptr,char *varname,int maxlen,int ex));       /* Get a token from a program line */

/* in util.c */
char *message Args((int rc));                                                       /* Return an error message string */
void rcset Args((int i,int type,char *desc));                                       /* Set the variable rc to the given value */
void rcstringset Args((int rc,char *rcval, int rclen,int type,char *desc));         /* Set the variable rc to the given string */
void printrc Args((int i));                                                         /* Print a return code */
int exitcall (/*(long main, long sub, PEXIT parm)*/);                               /* call an exit */
char *varsearch Args((char *name,int len,int *level,int *exist));                   /* Search for a simple symbol or stem */
char *tailsearch Args((char *stem,char *name,int len,int *exist));                  /* Search for a tail within a stem */
char *valuesearch Args((char *name,int namelen,int *level,int *exist,char **stem)); /* Search for any variable */
void printtree Args((int lev));                                                     /* Print the entire variable table */
void update Args((int value,int amount,int level));                                 /* Update the pointers for a variable level */
void varcreate Args((char *varptr,char *name,char *value,int namelen,int len,int lev));/* Create a simple symbol */
void stemcreate Args((char *varptr,char *name,char *value,int namelen,int len,int lev));/* Create a stem */
void tailcreate Args((char *stem,char *tailptr,char *name,char *value,int namelen,int len,int level));/* Create a tail in a stem */
void varset Args((char *name,int varlen,char *value,int len));                      /* Assign any value to any variable */
char *varget Args((char *name,int varlen,int *len));                                /* Get any variable's value */
void newlevel Args((void));                                                         /* Make a new level in the variable table */
void varcopy Args((char *name,int varlen));                                         /* Expose a variable from the previous level */
void vardup Args((void));                                                           /* Duplicate the entire variables level */
void vardel Args((char *name,int len));                                             /* Remove one of the duplicate variables */
char uc Args((int c));                                                              /* Uppercase a character */
void *pstack Args((int type,int len));                                              /* Make a new program stack entry */
int unpstack Args((void));                                                          /* Examine the top program stack entry */
void *delpstack Args((void));                                                       /* Delete and return the top p-stack entry */
int strcmpi Args((char *s1,char *s2));                                              /* Is s1 an initial substring of s2? */
void printstmt Args((int stmt,int after,int error));                                /* Print a source statement */
void freestack Args((void *ptr,int i));                                             /* Clean up after deleting a p-stack entry */
void traceput Args((char *str,int len));                                            /* Print a counted string of trace output */
void tracestr Args((char *str));                                                    /* Print a string of trace output */
void tracechar Args((int ch));                                                      /* Print a character of trace output */
void tracenum Args((int num,int len));                                              /* Print a number to trace output */
void traceprefix Args((int num, char*prefix));                                      /* Print a number and 3-character prefix */
void interactive Args((void));                                                      /* Pause execution, if in interactive trace */
void tokenise Args((char *input,int ilen,int dolabels,int line1));                  /* Preprocess source code */
char *load Args((char *name,int *sourcelen));                                       /* Load a program from disk */
void process Args((int c));                                                         /* Tokenise characters */
void expand Args((int c));                                                          /* Expand a token */
void display Args((int line,int ptr));                                              /* Display a program line */
char *rexxext Args((void));                                                         /* Return the default filetype */
int which Args((char *gn,int opt,char *fn));                                        /* Search for a program or subroutine */
char *hashsearch Args((int hash,char *name,int *exist));                            /* Search for a hash table entry */
void *hashget Args((int hash,char *name,int *exist));                               /* Return the value associated with a key */
void **hashfind Args((int hash,char *name,int *exist));                             /* Return a pointer to a key's value element */
struct fileinfo *fileinit Args((char *name,char *filename,FILE *fp));               /* Store an open file's details */
void funcinit Args((char *name,void *handle,int (*address)(),int saa));             /* Store an external function's details */
void libsearch Args((void));                                                        /* Ssearch for *.rxlib files */
int fileclose Args((char *name));                                                   /* Free a file's hash table entry */

/* in rxfn.c */
int rxfn Args((char *name,int argc));                                               /* Try to call a builtin function */
int rxseterr Args((struct fileinfo *f));                                 /* Set rc to indicate a file's status */
char *undelete Args((int *l));                                                      /* Get last value without deleting it */

/* in shell.c */
int shell Args((char *command));                                                    /* Interpret a shell command */
void hashclear Args((void));                                                        /* Clear the shell's hashed path table */

/* in interface.c */
void envinit Args((void));                                                          /* Initialise the environment table */
int envsearch Args((char *name));                                                   /* Search for an environment name */
int envcall Args((int num,char *cmd, int len, char **ans, int *anslen));            /* call an environment */
int funccall Args((unsigned long (*func)(),char *name,int argc));                   /* call a function by SAA calling sequence */
int unixcall Args((char *name,char *callname,int argc));                            /* call a function by Unix calling sequence */
void hashfree Args((void));                                                         /* Free all memory used by hash tables */
static void halt_handler();                                                         /* handle halt signals */
static void pipe_handler();                                                         /* handle broken pipe signals */
static void error_handler();                                                        /* handle error signals */
static void sigtrace();                                                             /* Go into trace mode, or exit */

#undef Args

#endif /* !_functions_h */
