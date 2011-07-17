/* The constants and data types of REXX/imc      (C) Ian Collier 1992 */

#ifndef _const_h
#define _const_h

#define VER "beta-1.75"
#define LEVEL "4.00"  /* Language level supported */

/* A paranoid Solaris detector. */
#if (defined(__svr4__)||defined(__SVR4)||defined(__SVR4)) && (defined(sun)||defined(__sun)||defined(__sun__))
#define Solaris
#endif

#define four       (sizeof(int))
#ifdef POINTER64
#define align(x)   (((x)+7)&~7)
#define toalign(x) (((~(x))+1)&7)
#else
#define align(x)   (((x)+3)&~3)
#define toalign(x) (((~(x))+1)&3)
#endif

#define rxmathname "rxmathfn"   /* The basenames of the two REXX auxiliary */
#define rxquename  "rxque"      /* files  */

#ifndef filetype
#define filetype   ".rexx"      /* Default file extension */
#endif

#define maxopstack 30    /* Maximum number of pending operations in an expr */
#define maxvarname 250   /* Maximum length of a variable name or label */
#define maxargs 30       /* Maximum number of arguments in function call */
#define maxextension 32  /* Maximum length of a file extension */
#define maxenviron 31    /* Maximum length of an environment name */
#define maxexp 999999999 /* maximum exponent of a number (must fit into int) */
#define maxdigits 10000  /* maximum allowable setting for NUMERIC DIGITS */
#define maxtracelen 500  /* maximum length of a line of trace */
#define traceindent 1    /* Spaces per indentation level in traceback */
#define STDIN 0          /* = fileno(stdin) */

#define cnull (char *)0  /* Various null pointers ... */
#define inull (int *)0

#define Isyntax   0      /* Integers associated with the SIGNAL ON traps */
#define Ierror    1
#define Ihalt     2
#define Inovalue  3
#define Ifailure  4
#define Inotready 5
#define Imax      6

#define Tclauses    1    /* The bit values for the various TRACE facilities */
#define Tlabels     2
#define Tresults    4
#define Tintermed   8
#define Tcommands  16
#define Terrors    32
#define Tfailures  64
#define Tinteract 128

/* valid flag values for RexxStartProgram */
#define RXMAIN      1   /* Coming from main(). */
#define RXVERSION   2   /* Print version.  Exit unless another flag is set. */
#define RXOPTIONX   4   /* Do not add file extension; ignore first line. */
#define RXEXITS     8   /* Preserve previous exits. */
#define RXDIGITS   16   /* Preserve previous NUMERIC DIGITS. */
#define RXSTUFF    32   /* Stuff the stack just before it is terminated. */

#define Einit      -1   /* Initialisation error */
#define Ehalt       4   /* Program interrupted */
#define Emem        5   /* Machine storage exhausted */
#define Elcomm      6   /* Unmatched comment */
#define Equote      6   /* Unmatched quote */
#define Enowhen     7   /* Expected WHEN */
#define Eselect     7   /* Expected OTHERWISE */
#define Ethen       8   /* Unexpected THEN */
#define Ewhen       9   /* Unexpected WHEN */
#define Eend       10   /* Unexpected or unmatched END */
#define Echar      13   /* Invalid character in program */
#define Enoend     14   /* Incomplete DO/SELECT/IF */
#define Eprogend   14
#define Ebin       15   /* Invalid binary constant */
#define Ehex       15   /* Invalid hexadecimal constant */
#define Elabel     16   /* Label not found */
#define Eprocedure 17   /* Unexpected PROCEDURE */
#define Enothen    18   /* Expected THEN */
#define Enostring  19   /* String or symbol expected */
#define Enosymbol  20   /* Symbol expected */
#define Exend      21   /* Invalid data on end of clause */
#define Etrace     24   /* Invalid TRACE request */
#define Eform      25   /* Invalid subkeyword found */
#define Etrap      25
#define Enonint    26   /* Invalid whole number */
#define Erange     26
#define Exdo       27   /* Invalid DO syntax */
#define Eleave     28   /* Invalid LEAVE or ITERATE */
#define Elong      30   /* Symbol too long */
#define Ename      31   /* Name starts with number or '.' */
#define Ebadexpr   35   /* Invalid expression */
#define Eassign    35
#define Elpar      36   /* Unmatched '(' */
#define Ecomma     37   /* Unexpected comma */
#define Erpar      37   /* Unexpected ')' */
#define Eparse     38   /* Invalid template */
#define Eopstk     39   /* Evaluation stack overflow */
#define Eargs      40
#define Ecall      40   /* Incorrect call to routine */
#define Eformat    40
#define Enoarg     40
#define Enum       41   /* Bad arithmetic conversion */
#define Edivide    42
#define Eoflow     42   /* Arithmetic overflow or underflow */
#define Eundef     43   /* Routine not found */
#define Enoresult  44   /* Function did not return data */
#define Ereturn    45   /* No data specified on function RETURN */
#define Exlabel    47   /* Unexpected label */
#define Esys       48   /* Failure in system service */
#define Elost      49   /* Implementation error */
#define Eincalled  50   /* Error in called routine */
#define Enovalue   80   /* No-value error */
#define Eexist     81   /* Use of an un-implemented feature! */
#define Esyntax    82   /* Syntax error */
#define Elabeldot  83   /* Label ends with '.' */
#define Emanyargs  84   /* Too many arguments (> 30) */
#define Eerror     85   /* command returned error */
#define Efailure   86   /* command returned failure */
#define Enotready  87   /* I/O operation raised "notready" error */
#define Ercomm     88   /* Unexpected '* /' */
#define Esig       89   /* Interpreter stopped because of a signal */
#define Eeof       99   /* End of file */
#define Eerrno    100   /* errortext(rc+Eerrno) gives the IO error for rc */
#define Ebounds   200   /* File position was out of bounds */
#define Eseek     201   /* Reposition attempted on transient stream */
#define Eaccess   202   /* Write attempted on a read-only stream */
#define Eread     203   /* Read attempted on a write-only stream */

#define numwords 50     /* How many tokens are below */
#define Command -102    /* "Command" tokens are strictly less than this */

#define SAY       -128  /* The various tokens */
#define SAYN      -127  /*  - in the same order as in words[] */
#define DO        -126
#define END       -125
#define IF        -124
#define ELSE      -123
#define SELECT    -122
#define WHEN      -121
#define OPTIONS   -120
#define PARSE     -119
#define PUSH      -118
#define QUEUE     -117
#define EXIT      -116
#define RETURN    -115
#define CALL      -114
#define SIGNAL    -113
#define ITERATE   -112
#define LEAVE     -111
#define INTERPRET -110
#define TRACE     -109
#define OTHERWISE -108
#define NOP       -107
#define PROCEDURE -106
#define ADDRESS   -105
#define NUMERIC   -104
#define DROP      -103
#define THEN      -102
#define PULL      -101
#define ARG       -100
#define SOURCE    -99
#define VAR       -98
#define VERSION   -97
#define LINEIN    -96
#define VALUE     -95
#define WITH      -94
#define UPPER     -93
#define TO        -92
#define BY        -91
#define FOR       -90
#define FOREVER   -89
#define WHILE     -88
#define UNTIL     -87
#define ON        -86
#define OFF       -85
#define DIGITS    -84
#define FUZZ      -83
#define FORM      -82
#define EXPOSE    -81
#define HIDE      -80
#define NAME      -79

#define LABEL     -50  /* equals NOP; inserted instead of a label */

#define SYMBOL    -16 /*240*/            /* lowest symbol number */
#define CONCAT    -15 /*241*/ /* ||  */  /* The various symbolic tokens */
#define LXOR      -14 /*242*/ /* &&  */
#define EQU       -13 /*243*/ /* ==  */
#define LEQ       -12 /*244*/ /* <=  */
#define GEQ       -11 /*245*/ /* >=  */
#define NEQ       -10 /*246*/ /* \=  */
#define NNEQ      -9  /*247*/ /* \== */
#define MOD       -8  /*248*/ /* //  */
#define LESS      -7  /*249*/ /* <<  */
#define GRTR      -6  /*250*/ /* >>  */
#define LLEQ      -5  /*251*/ /* <<= */
#define GGEQ      -4  /*252*/ /* >>= */
#define POWER     -3  /*253*/ /* **  */

#define Cconcat   0x7c7c    /* || */     /* ASCII values of these operators */
#define Cxor      0x2626    /* && */
#define Cequ      0x3d3d    /* == */
#define Cleq1     0x3c3d    /* <= */
#define Cleq2     0x5c3e    /* \> */
#define Cgeq1     0x3e3d    /* >= */
#define Cgeq2     0x5c3c    /* \< */
#define Cneq1     0x5c3d    /* \= */
#define Cneq2     0x3c3e    /* <> */
#define Cneq3     0x3e3c    /* >< */
#define Cnneq     0x5c3d3d  /* \== */
#define Cmod      0x2f2f    /* // */
#define Cless     0x3c3c    /* << */
#define Cgrtr     0x3e3e    /* >> */
#define Clleq1    0x3c3c3d  /* <<= */
#define Clleq2    0x5c3e3e  /* \>> */
#define Cggeq1    0x3e3e3d  /* >>= */
#define Cggeq2    0x5c3c3c  /* \<< */
#define Cpower    0x2a2a    /* ** */

#define OPpower   0 /* **        */ /* The various operator numbers (used   */
#define OPmul     1 /* *         */ /* by scanning() and related functions) */
#define OPdiv     2 /* /         */
#define OPadd     3 /* + (binary)*/
#define OPsub     4 /* - (binary)*/
#define OPspc     5 /* space     */
#define OPcat     6 /* ||        */
#define OPequ     7 /* =         */
#define OPeequ    8 /* ==        */
#define OPless    9 /* <         */
#define OPleq    10 /* <=        */
#define OPgrtr   11 /* >         */
#define OPgeq    12 /* >=        */
#define OPneq    13 /* \=        */
#define OPand    14 /* &         */
#define OPxor    15 /* &&        */
#define OPor     16 /* |         */
#define OPidiv   17 /* %         */
#define OPmod    18 /* //        */
#define OPnneq   19 /* \==       */
#define OPlless  20 /* <<        */
#define OPggrtr  21 /* >>        */
#define OPlleq   22 /* <<=       */
#define OPggeq   23 /* >>=       */
#define OPneg    24 /* - (unary) */
#define OPplus   25 /* + (unary) */
#define OPnot    26 /* \         */

typedef void (*sighandler)();

typedef struct _program {   /* Structure for the preprocessed program */
   int num;                 /* Line number in source (for traceback) */
   char *source;            /* Start of this statement within the source */
   char *sourcend;          /* End of same */
   int related;             /* A related statement number (if any) */
   char *line;              /* The program line to be interpreted */
} program;
   
struct procstack {  /* A program stack entry for an external procedure call */
   int stmt;        /* Statement number of call instruction */
   char *csp;       /* Old calculator stack */
   int ecsp;        /* Old calculator stack end */
   int csl;         /* Old calculator stack length */
   char trc;        /* Old trace flag */
   char tim;        /* Old timestamp flag */
   char form;       /* Old NUMERIC FORM */
                    /* Some padding (one byte, probably) */
   int digits;      /* Old NUMERIC DIGITS */
   int fuzz;        /* Old NUMERIC FUZZ */
   long mic;        /* Old timestamp microseconds */
   long sec;        /* Old timestamp seconds */
   int address1;    /* Old current environment */
   int address2;    /* Old alternate environment */
   program *prg;    /* Old tokenised program */
   int stmts;       /* Old number of statements */
   int lines;       /* Old number of program lines */
   char **src;      /* Old source */
   char *lab;       /* Old labels */
#ifdef POINTER64
   int dummy;       /* Artificial padding */
#endif
   int lev;         /* Old pstack level */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 13 for external procedure call */
};

struct procstack2 { /* A program stack entry for a procedure call */
   int stmt;        /* Statement number of call instruction */
   char *csp;       /* Old calculator stack */
   int ecsp;        /* Old calculator stack end */
   int csl;         /* Old calculator stack length */
   char trc;        /* Old trace flag */
   char tim;        /* Old timestamp flag */
   char form;       /* Old NUMERIC FORM */
                    /* Some padding (one byte, probably) */
   int digits;      /* Old NUMERIC DIGITS */
   int fuzz;        /* Old NUMERIC FUZZ */
   long mic;        /* Old timestamp microseconds */
   long sec;        /* Old timestamp seconds */
   int address1;    /* Old current environment */
   int address2;    /* Old alternate environment */
   program *prg;    /* The current interpreted string or the current program */
#ifdef POINTER64
   int dummy;       /* Artificial padding */
#endif
   int stmts;       /* Number of statements in the above */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 11 for internal procedure call;  */
};                  /*        12 after a PROCEDURE instruction */

struct minstack {   /* A minimum program stack entry */
   int stmt;        /* Statement where block started */
#ifdef POINTER64
   int dummy;       /* Artificial padding */
#endif
   char *pos;       /* Pointer to statement (used to find the WHILE or UNTIL)*/
   int len;         /* Length of this stack entry */
   int type;        /* Type =  0 for DO;  */
};                  /*         8 for DO WHILE/FOREVER */
                    /*        10 for DO with control variable */
                    /*         2 for SELECT */

struct forstack {   /* A program stack entry for the "DO count" instruction */
   int stmt;        /* Statement number of DO instruction */
#ifdef POINTER64
   int dummy1;      /* Artificial padding */
#endif
   char *pos;       /* Pointer to statement (used to find the WHILE or UNTIL)*/
#ifdef POINTER64
   int dummy;       /* More artificial padding */
#endif
   int fornum;      /* Loop counter */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 15 for a "DO count" instruction */
};

struct interpstack {/* A program stack entry for the INTERPRET instruction */
   int stmt;        /* Statement number of INTERPRET instruction */
   program *prg;    /* Old tokenised program */
#ifdef POINTER64
   int dummy;       /* Artificial padding */
#endif
   int stmts;       /* Old number of statements */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 14 for an INTERPRET instruction */
};

struct interactstack {/* A program stack entry for interactive instructions */
                      /* NB always followed by an interpstack entry */
   int stmt;        /* Statement number of the interruption */
   char *csp;       /* Old calculator stack */
   int ecs;         /* Old calculator stack end pointer */
   int csl;         /* Old calculator stack length */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 16 for interactive instructions */
};

struct errorstack { /* A program stack entry for saving a traceback line */
   int stmt;        /* Statement number of the error */
   program *prg;    /* Program where the error occurred */
#ifdef POINTER64
   int dummy;       /* Artificial padding */
#endif
   int stmts;       /* Number of statements in this program */
   int len;         /* Length of this stack entry */
   int type;        /* Type = 20 for saved traceback line */
};

struct fileinfo {   /* Structure containing information about an open file */
   FILE *fp;        /* The stream pointer */
   char wr;         /* Whether writing is allowed; -1=write-only */
   char lastwr;     /* Whether the last operation was a write */
   char persist;    /* Whether the file is persistent (i.e. a regular file) */
   long rdpos;      /* The read character position (0-based) */
   int rdline;      /* The read line position, if known (1-based) */
   int rdchars;     /* Whether an incomplete line has been read */
   long wrpos;      /* The write character position (0-based) */
   int wrline;      /* The write line position, if known (1-based) */
   int wrchars;     /* Whether an incomplete line has been written */
   int errnum;      /* Error status for stream('d') */
};
   
typedef struct _hashent { /* Structure containing hash table entry */
   int next;        /* Length of the whole item */
   int less;        /* Pointer to left child */
   int grtr;        /* Pointer to right child */
   void *value;     /* Pointer to value */
} hashent;

typedef struct _varent {       /* a variable table entry */
   int next;                   /* length of structure */
   int less;                   /* position of left child within tree */
   int grtr;                   /* position of right child within tree */
   int namelen;                /* length of variable's name */
   int valalloc;               /* length allocated to variable's value */
   int vallen;                 /* actual length of variable's value */
} varent;

typedef struct _funcinfo { /* Structure containing info about a function */
   void *dlhandle;         /* Handle from dlopen(), if this is the "main" fn */
   int (*dlfunc)();        /* Address of the function */
   char *name;             /* file name of a previously found function*/
   int saa;                /* calling sequence */
} funcinfo;

typedef struct _dictionary {/* Structure containing a list of REXX functions */
   char *name;              /* REXX name of function */
   int (*function)();       /* Address of function */
} dictionary;

#endif /* !_const_h */
