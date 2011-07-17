/* Functions of REXX/imc relating to the SAA API     (C) Ian Collier 1994 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include "const.h"
#include "globals.h"
#include "functions.h"
#define INCL_REXXSAA
#include "rexxsaa.h"
#include <sys/socket.h>
#ifdef Solaris
#include <sys/uio.h>
#endif
#ifdef STUFF_STACK
#include<sys/termios.h>
#endif

static void halt_handler();                                                         /* handle halt signals */
static void pipe_handler();                                                         /* handle broken pipe signals */
static void error_handler();                                                        /* handle error signals */
static void sigtrace();                                                             /* Go into trace mode, or exit */

struct status{        /* Saved things from a previous incarnation of REXX */
      int stmt;
      char trcflag,timeflag,trcresult;
      char form;
      int precision,fuzz;
      long tsec,tmic,sec,mic;
      int address0,address1,address2;
      int varlevel;
      char *psource;
      program *prog;
      int stmts;
      char **source;
      int lines;
      char *labels;
      char *cstackptr;
      int ecstackptr,cstacklen;
      char *pstackptr;
      int epstackptr,pstacklen;
      int pstacklev;
      struct sigstruct *sgstack;
      unsigned sigstacklen;
      int interplev;
      char **arg;
      int *arglen;
      jmp_buf *exitbuf;
   };

/* list of environments for environmental functions below */
struct environ *envtable;
static int envtablelen=0;
static int envs;

/* list of registered exits for exitary functions below */
static struct exitentry{
   char name[maxenviron+1];
   RexxExitHandler *handler;
   unsigned char *area;
} *exittable;
static int exitlen=0;
static int exits=0;

char version[80];       /* REXX version string */
char *psource;          /* the string parsed by PARSE SOURCE */

/* Starting REXX */

static int rexxdepth=0; /* nesting level of RexxStart() */
static char rxque[maxvarname]; /* where to find rxque */
static char rxstackholder[128];/* to hold the output of "rxque" */
/* the following structure mirrors struct sockaddr, but has a longer name
   field.  It is to contain the file name of the stack socket. */
static struct {u_short af;char name[maxvarname];} rxsockname={AF_UNIX};
static int rxsocklen;          /* the length of the above structure */
static int rxstackproc=0;      /* the process number of "rxque" */

static void stackinit(rxpathname) /* sets up the stack */
char *rxpathname;                 /* argv[0], if supplied */
{
   char *rxstackname=getenv("RXSTACK");
   char *imcpath=getenv("REXXIMC");
   static char _rxpath[maxvarname];
   int f,l;
   char *basename;
   int pipefd[2];
   char *answer;
/* Construct the REXX auxiliary file path names if necessary */
   if(!rxque[0]){
      strcpy(rxque,imcpath?imcpath:REXXIMC); /* use $REXXIMC if possible, */
      l=strlen(rxque);                       /* otherwise the REXXIMC macro */
      rxque[l++]='/';
      strcpy(rxque+l,rxquename);
      if(access(rxque,X_OK)){                /* rxque does not exist. */
         l=0;
         if(rxpathname && strchr(rxpathname,'/')){/* Try some other directory */
            strcpy(rxque,rxpathname);        /* for instance our path name */
            basename=strrchr(rxque,'/')+1;
            strcpy(basename,rxquename);
            if(!access(rxque,X_OK))l=basename-rxque;
         }
         if(!l){                             /* OK, now try the entire path! */
            if(!which(rxquename,-1,rxque)){
               fprintf(stderr,"Unable to find \'%s\'\n",rxquename);
               die(Einit);
            }
            l=strrchr(rxque,'/')+1-rxque;
         }
      }
      /* libraries: if REXXLIB not set then the default is the compiled-in
         value, if any, otherwise the same place where rxque was found. */
#ifdef REXXLIB
      if (REXXLIB[0]) rxpath=REXXLIB;
      else
#endif
      {
         rxpath=_rxpath;
         memcpy(rxpath,rxque,l);
         rxpath[l-1]=0;
      }
   }
/* open the stack */
   if(!rxstackname || !*rxstackname){ /* it doesn't exist already, so fork off "rxque" */
      if(pipe(pipefd))perror("pipe"),die(Einit);
      if((f=vfork())<0)perror("vfork"),die(Einit);
      if(!f){  /* the child: attach pipe to stdout and exec rxque */
         close(pipefd[0]);
         if(dup2(pipefd[1],1)<0)perror("dup2"),_exit(-1);
         close(pipefd[1]);
         execl(rxque,"rxque",cnull);
         perror(rxque);
         _exit(-1);
      } /* now the parent: read from pipe into rxstackholder. The answer
            should be RXSTACK=(name) RXSTACKPROC=(number).  Split off the
            second token, search for "=", store number in rxstackproc, and
            put RXSTACK into the environment. */
      close(pipefd[1]);
      if(read(pipefd[0],rxstackholder,sizeof rxstackholder)<20
       ||!(answer=strchr(rxstackholder,' '))
       ||!(answer[0]=0,answer=strchr(answer+1,'='))
       ||!(rxstackproc=atoi(answer+1)))
         fputs("Cannot create stack process\n",stderr),die(Einit);
      close(pipefd[0]);
      rxstackname=strchr(rxstackholder,'=')+1;
      putenv(rxstackholder);
      wait((int*)0);     /* delete child from process table */
   }  /* The stack exists. Open a socket to it. */
   strcpy(rxsockname.name,rxstackname),
   rxsocklen=sizeof(u_short)+strlen(rxstackname);
   if((rxstacksock=socket(AF_UNIX,SOCK_STREAM,0))<0)
      perror("REXX: couldn't make socket"),die(Einit);
   if(connect(rxstacksock,(struct sockaddr *)&rxsockname,rxsocklen)<0)
      perror("REXX: couldn't connect socket"),die(Einit);
}

static void rexxterm(old)        /* Destroy the REXX data structures */
struct status *old;
{
   if(cstackptr)free(cstackptr),cstackptr=0;
   if(pstackptr)free(pstackptr),pstackptr=0;
   if(sgstack)free(sgstack),sgstack=0;
   if(source)
      free(source[0]),      /* the file name */
      free(source[1]),      /* the source characters */
      free((char*)source),source=0;
   if(prog)
      free(prog[0].line),   /* the program characters */
      free((char*)prog),prog=0;
   if(labelptr)free(labelptr),labelptr=0;
   if(rexxdepth==0){
      if(varstk)free(varstk),varstk=0;
      if(vartab)free(vartab),vartab=0;
      if(hashlen[2])hashfree();/* This *shouldn't* close stdin, stdout or stderr, but
                                  havoc might ensue anyway if the REXX program changed
                                  them... */
      if(workptr)free(workptr),workptr=0;
      if(pull)free(pull),pull=0;
      if(varnamebuf)free(varnamebuf),varnamebuf=0;
      if(ttyin && ttyin!=stdin)fclose(ttyin),ttyin=0;
      if(ttyout && ttyout!=stdout)fclose(ttyout),ttyout=0;
   /* Neutralise OPTIONs */
      if(traceout!=stderr)fclose(traceout),traceout=stderr;
      setrcflag=0;
      exposeflag=0;
   }
   else {
      ppc=old->stmt;
      trcflag=old->trcflag;
      timeflag=old->timeflag;
      trcresult=old->trcresult;
      numform=old->form;
      precision=old->precision;
      fuzz=old->fuzz;
      timestamp.tv_sec=old->tsec;
      timestamp.tv_usec=old->tmic;
      secs=old->sec;
      microsecs=old->mic;
      address0=old->address0;
      address1=old->address1;
      address2=old->address2;
      varstkptr=old->varlevel;
      psource=old->psource;
      prog=old->prog;
      stmts=old->stmts;
      source=old->source;
      lines=old->lines;
      labelptr=old->labels;
      cstackptr=old->cstackptr;
      ecstackptr=old->ecstackptr;
      cstacklen=old->cstacklen;
      pstackptr=old->pstackptr;
      epstackptr=old->epstackptr;
      pstacklen=old->pstacklen;
      pstacklev=old->pstacklev;
      sgstack=old->sgstack;
      sigstacklen=old->sigstacklen;
      interplev=old->interplev;
      curargs=old->arg;
      curarglen=old->arglen;
      exitbuf=old->exitbuf;
   }
}

long RexxStart(argc,argv,name,instore,envname,calltype,myexits,rc,result)
long argc;
PRXSTRING argv;
char *name;
PRXSTRING instore;
char *envname;
long calltype;
PRXSYSEXIT myexits;
short *rc;
PRXSTRING result;
{
   /* this is just an interface for RexxStartProgram.  The extra arguments
      are given as zeros. */
   return RexxStartProgram((char*)0,argc,argv,name,(char*)0,instore,envname,
          calltype,0,myexits,rc,result);
}

long RexxStartProgram(argv0,argc,argv,name,callname,instore,envname,
                      calltype,flags,myexits,rc,result)
char *argv0;
long argc;
PRXSTRING argv;
char *name;
char *callname;
PRXSTRING instore;
char *envname;
long calltype;
int flags;
PRXSYSEXIT myexits;
short *rc;
PRXSTRING result;
{
   char *answer;      /* result of executing the program */
   int anslen;        /* length of that result */
   char *input=0;     /* The source code from disk or wherever */
   int ilen;          /* The length of the source code */
   struct fileinfo *info; /* for initialising stdin, stdout, stderr */
   char *basename;    /* basename of the program to execute */
   char *tail;        /* file extension of the program */
   extern char *month[]; /* from rxdate() in rxfn.c */
   char **arglist=0;  /* a copy of the argument addresses */
   int *arglens=0;    /* a copy of the argument lengths */
   int i,j,l;
   long n;
   char *howcall;     /* string to represent calltype */
   char sourcestring[200]; /* string for "parse source" */
   int olddepth=rexxdepth;
   char env[maxenviron+1]; /* a copy of the environment name */
   volatile sighandler sigint,sigterm,sighup,      /* saved signal handlers */
            sigquit,sigsegv,sigbus,sigill,sigpipe;
   struct status old;
   jmp_buf exbuf;     /* buffer for exitbuf */

/* construct version string (should be constant, but it's easier this way) */
   sprintf(version,"REXX/imc-%s %s %d %s %d",VER,LEVEL,DAY,month[MONTH-1],YEAR+1900);
   if(flags&RXVERSION){
      puts(version);
      if(flags==RXVERSION)return 0;
   }

/* Argument checking */
   if(instore && instore[1].strptr)
      return 1;             /* no tokenised program.  May be fixed later... */
   if(instore && !(instore[0].strptr && instore[0].strlength))
      return 1;             /* no macros.  May possibly be fixed later... */
   if(!name)
      if(instore)name="anonymous";
      else return 1;
   if(envname && strlen(envname)>maxenviron) return 1;
   if(calltype!=RXCOMMAND && calltype!=RXFUNCTION && calltype!=RXSUBROUTINE)
      return 1;

   if(!(flags&RXEXITS))
      for(i=0;i<RXEXITNUM;i++)exitlist[i]=0; /* prepare to set exits */
   if(myexits)
      for(i=0;myexits[i].sysexit_code!=RXENDLST; i++){
         if(!exitlen)return RXEXIT_NOTREG;   /* unregistered exit name */
         for(j=0;j<exits && strcmp(exittable[j].name,myexits[i].sysexit_name);j++);
         if(j==exits || !exittable[j].handler)return RXEXIT_NOTREG;
         if(myexits[i].sysexit_code>=RXEXITNUM)
            return RXEXIT_BADTYPE;           /* unrecognised exit code */
         exitlist[myexits[i].sysexit_code]=exittable[j].handler;
      }

   if(rexxdepth){
      old.stmt=ppc;
      old.trcflag=trcflag;
      old.timeflag=timeflag;
      old.trcresult=trcresult;
      old.form=numform;
      old.precision=precision;
      old.fuzz=fuzz;
      old.tsec=timestamp.tv_sec;
      old.tmic=timestamp.tv_usec;
      old.sec=secs;
      old.mic=microsecs;
      old.address0=address0;
      old.address1=address1;
      old.address2=address2;
      old.varlevel=varstkptr;
      newlevel();
      old.psource=psource;
      old.prog=prog;prog=0;
      old.stmts=stmts;
      old.source=source;source=0;
      old.lines=lines;
      old.labels=labelptr;labelptr=0;
      old.cstackptr=cstackptr;cstackptr=0;
      old.ecstackptr=ecstackptr;
      old.cstacklen=cstacklen;
      old.pstackptr=pstackptr;pstackptr=0;
      old.epstackptr=epstackptr;
      old.pstacklen=pstacklen;
      old.pstacklev=pstacklev;
      old.sgstack=sgstack;sgstack=0;
      old.sigstacklen=sigstacklen;
      old.interplev=interplev;interplev=-1;
      old.arg=curargs;
      old.arglen=curarglen;
      old.exitbuf=exitbuf;exitbuf=addressof(exbuf);
   }
   else{
      interplev=-1;
      exitbuf=addressof(exbuf);
   }
   if(!envtablelen)envinit();
   if(!hashlen[2]){
      for(i=0;i<3;i++)hashptr[i]=allocm(hashlen[i]=256),ehashptr[i]=0;
      if(!hashlen[2])return Emem;
   }

   if((i=setjmp(*exitbuf))){    /* catch error during setup */
      rexxterm(&old);
      return i>0 ? i : -i;
   }

/* Initialise all the global variables */
   if (traceout==0) traceout=stderr;
   if(rexxdepth==0){
      stackinit(argv0);
      varstk=(int *)allocm(varstklen=256),
      varstkptr=0,
      varstk[0]=varstk[1]=0,
      vartab=allocm(vartablen=1024);
      worklen=maxvarname+10,
      workptr=allocm(worklen),
      pull=allocm(pulllen=256),
      varnamebuf=allocm(varnamelen=maxvarname);
      if(!(ttyin=fopen("/dev/tty","r")))ttyin=stdin;
      if(!(ttyout=fopen("/dev/tty","w")))ttyout=stderr;
      (info=fileinit("stdin",cnull,stdin))->lastwr=0; /* set up stdin */
      info->rdpos=info->wrpos;    /* wrpos has been set to the current position */
      info->rdline=info->wrline;  /* now rdpos will be there as well */
      fileinit("stdout",cnull,stdout)->wr=-1; /* set up stdout and stderr */
      fileinit("stderr",cnull,stderr)->wr=-1; /* for writing */
   }
   cstackptr=allocm(cstacklen=256),
   ecstackptr=0,
   pstackptr=allocm(pstacklen=512),
   pstacklev=epstackptr=0,
   sgstack=(struct sigstruct *)malloc(sizeof(struct sigstruct)*(sigstacklen=20));
   if(!(flags&RXDIGITS))precision=9;
   fuzz=9;
   numform=0;
   trcresult=0;
   timeflag&=4;
   if(!(flags&RXMAIN))trcflag=Tfailures;
   psource=sourcestring;

   if((i=setjmp(*exitbuf))){
      if(i!=Esig && exitlist[RXTER])exitcall(RXTER,RXTEREXT,(PEXIT)0);
      goto RexxEnd; /* catch execution errors */
   }
#define sigsetup(var,sig,handler) if((var=signal(sig,handler))!=SIG_DFL)\
                                     signal(sig,var);
   sigsetup(sigint,SIGINT,halt_handler);
   sigsetup(sigterm,SIGTERM,halt_handler);
   sigsetup(sighup,SIGHUP,halt_handler);
   sigsetup(sigquit,SIGQUIT,sigtrace);
   /* the following are set even if handlers already exist for them */
   sigpipe=signal(SIGPIPE,pipe_handler);
   sigsegv=signal(SIGSEGV,error_handler);
   sigbus=signal(SIGBUS,error_handler);
   sigill=signal(SIGILL,error_handler);
#undef sigsetup

/* Get the program's details and load it */
   if((basename=strrchr(name,'/')))basename++;
   else basename=name;           /* basename points to the file's name */
   if((tail=strrchr(basename,'.'))&&strlen(tail)<maxextension&&tail[1])
      strcpy(extension,tail);       /* this will be the default extension */
   else strcpy(extension,rexxext()); /* if none, use the system default */
   extlen=strlen(extension);
   if(instore){
      input=allocm(ilen=instore[0].strlength);
      memcpy(input,instore[0].strptr,ilen);
      strcpy(fname,name);
   }
   else{
      if(which(name,(flags&RXOPTIONX) || !(flags&RXMAIN),fname)!=1) /* search for the file */
         errordata=fname,die(-3);      /* error - not found */
      if(!(input=load(fname,&ilen)))
         errordata=fname,die(-3);      /* Error - could not load file */
   }
   tokenise(input,ilen,0,flags&RXOPTIONX);
   source[0]=allocm(strlen(fname)+1);
   strcpy(source[0],fname);
/* construct source string (one per invocation of RexxStart) */
   howcall=(calltype&RXSUBROUTINE)?"SUBROUTINE":
           (calltype&RXFUNCTION)?"FUNCTION":
           "COMMAND";
   if(!envname){
      envname=env;
      if(tail && tail[1] && strlen(tail)<=maxenviron){
         for(i=0;tail[i+1];i++)env[i]=uc(tail[i+1]);
         env[i]=0;
      }
      else strcpy(env,"UNIX");
   }
   address2=address1=address0=envsearch(envname);
   if(address1<0)die(Emem);
   if(callname)basename=callname;
   sprintf(psource,"UNIX %s %s %s %s",howcall,source[0],basename,envname);
/* call the interpreter */
   arglist=(char**)allocm((argc+1)*sizeof(char*));
   arglens=(int*)allocm((argc+1)*four);
   for(i=0;i<argc;i++){
      arglist[i]=argv[i].strptr,
      arglens[i]=argv[i].strlength;
      if(!arglist[i])arglist[i]=(char*)-1,arglens[i]=-1;
   }
   arglist[argc]=0;
   arglens[argc]=0;

   interplev=0;
   rexxdepth++;
   if(exitlist[RXINI])exitcall(RXINI,RXINIEXT,(PEXIT)0);
   answer=interpreter(&anslen,1,basename,calltype,arglist,arglens,0,0);
   if(exitlist[RXTER])exitcall(RXTER,RXTEREXT,(PEXIT)0);
   rexxdepth--;
   if (rc) *rc=1<<15;
   i=answer && anslen && answer[0]=='-';
   if(answer && anslen>i){
      for(n=0;i<anslen;i++){
         if(answer[i]<'0' || answer[i]>'9'){i=0;break;}
         n=n*10+answer[i]-'0';
         if(n<0 || n>=(1<<15)){i=0;break;}
      }
      if (i>0 && rc) *rc = answer[0]=='-' ? -n : n;
      else if(flags&RXMAIN) /* environment raises an error for non-integer */
         interplev=-1,die(Enonint);
   }
   if(result){
      if(!answer)
         result->strptr=0,
         result->strlength=0;
      else {
         if(!result->strptr || result->strlength<anslen){
            if((result->strptr=malloc(anslen)))result->strlength=anslen;
            else result->strlength=0;
         }
         else result->strlength=anslen;
         if(result->strptr)memcpy(result->strptr,answer,anslen);
      }
   }
   i=0;
RexxEnd:
   if(arglist)free(arglist);
   if(arglens)free(arglens);
   if(!(rexxdepth=olddepth)){
      if(rxstackproc){
#ifdef STUFF_STACK
         while(flags&RXMAIN){ /* either nop or infinite loop */
            if(i || write(rxstacksock,"G",1)<1 || /* don't copy if an error */
                    read(rxstacksock,pull,7)<7 || /* has occurred or the    */
                    !memcmp(pull,"FFFFFF",6)) break; /* stack is empty      */
            sscanf(pull,"%x",&l);
            while(l--&&
                  read(rxstacksock,pull,1) &&
                  0==ioctl(fileno(ttyin),TIOCSTI,pull)); /* Stuff one character */
            if(l>=0)break;
            pull[0]='\n';               /* a return at the end of each line */
            if(ioctl(fileno(ttyin),TIOCSTI,pull)) break;
         }
#endif
         kill(rxstackproc,SIGTERM);
         putenv("RXSTACK=");
      }
      close(rxstacksock);
   }
   rexxterm(&old);   /* put everything back as it was */
   /* restore signal handlers to their previous values */
   signal(SIGINT,sigint);
   signal(SIGTERM,sigterm);
   signal(SIGHUP,sighup);
   signal(SIGQUIT,sigquit);
   signal(SIGSEGV,sigsegv);
   signal(SIGBUS,sigbus);
   signal(SIGILL,sigill);
   signal(SIGPIPE,sigpipe);
   return -i;
}

/* Here are the signal handlers. */
/* Each halt signal (SIGINT, SIGHUP, SIGTERM) is handled by recording it.    */
/* SIGHUP and SIGTERM are more forceful signals; too many of them terminates */
/* the interpreter.                                                          */
static void halt_handler(sig)
int sig;
{
   signal(sig,halt_handler);   /* required on SysV */
   on_halt();                  /* Find the line number at which halt occurred */
   delayed[Ihalt]++;
   switch(sig){
      case SIGINT: sigdata[Ihalt]="SIGINT"; putc('\n',ttyout); break;
      case SIGHUP: sigdata[Ihalt]="SIGHUP"; break;
      default:     sigdata[Ihalt]="SIGTERM";
   }
   if(sig!=SIGINT && delayed[Ihalt]>2)
      fputs("Emergency stop\n",ttyout),
      longjmp(*exitbuf,Esig);
}

/* SIGPIPE causes the interpreter to stop immediately unless */
/* OPTIONS SIGPIPE was specified, in which case it is just   */
/* ignored (the write or flush will return an error).        */
static void pipe_handler(sig)/*ARGSUSED*/
int sig;
{
   if (!sigpipeflag) error_handler(sig);
   signal(sig,pipe_handler);    /* required on SysV */
}

/* SIGSEGV, SIGBUS, and SIGILL cause the interpreter to stop */
/* immediately.  This may also be called for SIGPIPE above.  */
static void error_handler(sig)
int sig;
{
   signal(sig,error_handler);   /* required on SysV */
   switch(sig){
      case SIGSEGV: fputs("Segmentation fault",ttyout); break;
      case SIGBUS:  fputs("Bus error",ttyout); break;
      case SIGILL:  fputs("Illegal instruction",ttyout);
   }
   if(sig!=SIGPIPE) fputs(" (cleaning up)\n",ttyout);
   longjmp(*exitbuf,Esig);
}

/* A SIGQUIT is handled by going to interactive trace */
/* mode, or by stopping immediately.  Only stop if we */
/* have already tried to interrupt the program.       */
static void sigtrace(sig)
int sig;
{
   signal(sig,sigtrace);   /* required on SysV */
   fputs("\b\b  \b\b",ttyout);
   fflush(ttyout);
   if(delayed[Ihalt] && (trcflag&Tinteract)){
      fputs("Emergency stop\n",ttyout);
      longjmp(*exitbuf,Esig);
   }
   trcflag=Tinteract|Tclauses|Tlabels|Tresults;
   interactmsg=1;
}

/* Subcommand environment handling routines */

/* Environments will be held in a table of names and addresses (above) */
/* Initially the environments are UNIX, SYSTEM, COMMAND and PATH. */
/* Environment UNIX or SYSTEM gives the command to a Bourne Shell. */
static unsigned long unixhandler(command,flags,returnval)
RXSTRING *command;
unsigned short *flags;
RXSTRING *returnval;
{
   int ret;
   char *cmd=command->strptr;
   *flags=RXSUBCOM_ERROR;
   cmd[command->strlength]=0; /* there should always be room for this kludge */
   ret=(char)(system(cmd)/256);
   if(ret==1 || ret<0)*flags=RXSUBCOM_FAILURE;
   else if(ret==0)*flags=RXSUBCOM_OK;
   sprintf(returnval->strptr,"%d",ret);
   returnval->strlength=strlen(returnval->strptr);
   return 0;
}

/* Environment COMMAND or PATH gives the command to the builtin shell. */
static unsigned long commandhandler(command,flags,returnval)
RXSTRING *command;
unsigned short *flags;
RXSTRING *returnval;
{
   int ret;
   char *cmd=command->strptr;
   *flags=RXSUBCOM_ERROR;
   cmd[command->strlength]=0;
   ret=shell(cmd);
   if(ret<0)*flags=RXSUBCOM_FAILURE;
   else if(ret==0)*flags=RXSUBCOM_OK;
   sprintf(returnval->strptr,"%d",ret);
   returnval->strlength=strlen(returnval->strptr);
   return 0;
}

/* All other environments just return -3 with FAILURE. */
static unsigned long defaulthandler(command,flags,returnval)
RXSTRING *command;
unsigned short *flags;
RXSTRING *returnval;
{
   *flags=RXSUBCOM_FAILURE;
   returnval->strlength=2;
   returnval->strptr[0]='-';
   returnval->strptr[1]='3';
   return 0;
}

/* The initial environments are registered. */
void envinit()
{
   envtable=(struct environ *)allocm((envtablelen=16)*sizeof(struct environ));
   envs=0;
   RexxRegisterSubcomExe("UNIX",unixhandler,NULL);
   RexxRegisterSubcomExe("SYSTEM",unixhandler,NULL);
   RexxRegisterSubcomExe("COMMAND",commandhandler,NULL);
   RexxRegisterSubcomExe("PATH",commandhandler,NULL);
}

/* This function returns a number for each environment name.  The name
   must be null terminated and within the length limits.  A negative
   answer means a memory error. */
int envsearch(name)
char *name;
{
   int i;
   struct environ *tmp;
   for(i=0;i<envs;i++) if(!strcmp(envtable[i].name,name))return i;
   /* if the name is not found, make an undefined environment. */
   if(++envs==envtablelen){
      envtablelen+=16;
      tmp=(struct environ *)realloc(envtable,envtablelen*sizeof(struct environ));
      if(!tmp){
         envtablelen-=16;
         return -1;
      }
      envtable=tmp;
   }
   strcpy(envtable[i].name,name);
   envtable[i].handler=defaulthandler;
   envtable[i].area=0;
   envtable[i].defined=0;
   return i;
}

/* And now the three API calls: */
unsigned long RexxRegisterSubcomExe(name,handler,area)
char *name;
RexxSubcomHandler *handler;
unsigned char *area;
{
   int i;
   if(!envtablelen)envinit();
   if(strlen(name)>maxenviron)return RXSUBCOM_BADTYPE;
   i=envsearch(name);
   if(i<0)return RXSUBCOM_NOEMEM;
   if(envtable[i].defined)return RXSUBCOM_NOTREG;
   envtable[i].handler=handler;
   envtable[i].area=area;
   envtable[i].defined=1;
   return RXSUBCOM_OK;
}

unsigned long RexxDeregisterSubcom(name,mod)
char *name,*mod;
{
   int ans=RXSUBCOM_OK;
   int i;
   if(strlen(name)>maxenviron)return RXSUBCOM_BADTYPE;
   if(!envtablelen)return RXSUBCOM_NOTREG;
   i=envsearch(name);
   if(i<0)return RXSUBCOM_NOTREG;
   if(!envtable[i].defined)ans=RXSUBCOM_NOTREG;
   else{
      envtable[i].handler=defaulthandler;
      envtable[i].area=0;
      envtable[i].defined=0;
   }
   while(envs && !envtable[envs-1].defined)envs--;  /* reclaim unused entries */
   return ans;
}

unsigned long RexxQuerySubcom(name,mod,flag,area)
char *name,*mod;
unsigned short *flag;
unsigned char *area;
{
   int ans=RXSUBCOM_OK;
   int i;
   if(flag)*flag=RXSUBCOM_NOTREG;
   if(strlen(name)>maxenviron)return RXSUBCOM_BADTYPE;
   if(!envtablelen)return RXSUBCOM_NOTREG;
   i=envsearch(name);
   if(i<0)return RXSUBCOM_NOTREG;
   if(!envtable[i].defined)ans=RXSUBCOM_NOTREG;
   if(i==envs-1)envs--;
   else if(area && envtable[i].area)memcpy(area,envtable[i].area,8);
   else if(area)memset(area,0,8);
   if(flag)*flag=ans;
   return ans;
}

/* Call environment number num with command cmd of length len and
   return the result ans of length anslen.  The return value is
   0 for OK, otherwise Eerror or Efailure.  Note: cmd must have a
   writeable byte after it. */
int envcall(num,cmd,len,ans,anslen)
int num,len,*anslen;
char *cmd,**ans;
{
   unsigned short rc;
   static char data[RXRESULTLEN];
   RXSTRING input,output;
   RXCMDHST_PARM rxcmd;
   input.strptr=cmd;
   input.strlength=len;
   cmd[len]=0;
   MAKERXSTRING(output,data,RXRESULTLEN);
   if(exitlist[RXCMD]){
      rxcmd.rxcmd_address=envtable[num].name;
      rxcmd.rxcmd_addressl=strlen(envtable[num].name);
      rxcmd.rxcmd_dll_len=0;
      rxcmd.rxcmd_command=input;
      rxcmd.rxcmd_retc=output;
      if(exitcall(RXCMD,RXCMDHST,&rxcmd)==RXEXIT_HANDLED){
         rc=0;
         if(rxcmd.rxcmd_flags.rxfcfail)rc=Efailure;
         else if(rxcmd.rxcmd_flags.rxfcerr)rc=Eerror;
         if(!output.strptr){
            *ans="0";
            *anslen=1;
         } else {
            *ans=output.strptr;
            *anslen=output.strlength;
            if(output.strptr!=data){
               /* The string is user-allocated.  Let's put it on the
                  calculator stack... */
               stack(*ans,*anslen);
               *ans=delete(anslen);
               free(output.strptr);
            }
         }
         return rc;
      }
   }
   envtable[num].handler(&input,&rc,&output);
   if(!output.strptr){
      *ans="0";
      *anslen=1;
   }
   else{
      *ans=output.strptr;
      *anslen=output.strlength;
      if(output.strptr!=data){
         /* The string is user-allocated.  Let's put it on the
            calculator stack... */
         stack(*ans,*anslen);
         *ans=delete(anslen);
         free(output.strptr);
      }
   }
   if(rc==RXSUBCOM_OK)return 0;
   if(rc==RXSUBCOM_FAILURE)return Efailure;
   return Eerror;
}

/* The RexxVariablePool request interpreter. */
unsigned long RexxVariablePool(request)
SHVBLOCK *request;
{
   extern varent *nextvar;               /* next variable for RXSHV_NEXTV */
   static varent *endvars=0;             /* upper limit of nextvar */
   static varent *nexttail=0;            /* next tail for RXSHV_NEXTV */
   static varent *endtails=0;            /* upper limit of nexttail */
   varent *thisvar;
   unsigned long ret=0;
   char *name;
   int namelen;
   int i;
   int nlen;
   char *nptr;
   int vallen;
   char *valptr;
   int lev;
   if(rexxdepth==0)return RXSHV_NOAVL;
   for(;request;ret|=request->shvret,request=request->shvnext){
      name=request->shvname.strptr;
      namelen=request->shvname.strlength;
      request->shvret=0;
      switch(request->shvcode){ /* variable name massaging */
         case RXSHV_SYFET:
         case RXSHV_SYDRO:      /* turn symbolic into direct */
         case RXSHV_SYSET:
            mtest(workptr,worklen,namelen+1,namelen+1-worklen);
            for(i=0;i<namelen;i++)workptr[i]=uc(name[i]);
            workptr[namelen]=0;
            i=0;
            getvarname(workptr,&i,varnamebuf,&nlen,varnamelen);
            if(nlen==0 || i!=namelen) request->shvret=RXSHV_BADN;
            else name=varnamebuf,namelen=nlen;
            break;
         case RXSHV_DROPV:  /* check variable and set compound/stem bits */
         case RXSHV_FETCH:
         case RXSHV_SET:
            mtest(workptr,worklen,namelen,namelen-worklen);
            memcpy(workptr,name,namelen);
            for(i=0;i<namelen&&name[i]!='.';i++)
               if(!rexxsymbol(name[i])){
                  request->shvret=RXSHV_BADN;
                  break;
               }
            if(rexxsymbol(name[0])<0){
               request->shvret=RXSHV_BADN;
               break;
            }
            if(i<namelen){
               workptr[0]|=128;
               if(i==namelen-1)namelen--;
            }
            name=workptr;
      }
      if(request->shvret)continue;
      switch(request->shvcode){
         /* FIXME: It is impossible for RXSHV_NEWV or RXSHV_MEMFL to
            be returned for a set or drop operation. */
         case RXSHV_DROPV:
         case RXSHV_SYDRO:
            nextvar=0;
            varset(name,namelen,0,-1); 
            break;
         case RXSHV_SET:
         case RXSHV_SYSET:
            nextvar=0;
            varset(name,namelen,request->shvvalue.strptr,
                   request->shvvalue.strlength);
            break;
         case RXSHV_FETCH:
         case RXSHV_SYFET: 
            nextvar=0;
            valptr=varget(name,namelen,&vallen);
            if(!valptr)
               name[0]&=127,
               valptr=name,
               vallen=namelen,
               request->shvret=RXSHV_NEWV;
            if(!request->shvvalue.strptr){
               request->shvvalue.strptr=malloc(vallen);
               if(!request->shvvalue.strptr){
                  request->shvret|=RXSHV_MEMFL;
                  break;
               }
               else request->shvvalue.strlength=request->shvvaluelen=vallen;
            } else {
               if(vallen>request->shvvaluelen)
                  vallen=request->shvvaluelen,request->shvret|=RXSHV_TRUNC;
               request->shvvalue.strlength=vallen;
            }
            memcpy(request->shvvalue.strptr,valptr,vallen);
            break;
         case RXSHV_NEXTV:
         case_RXSHV_NEXTV:
            if(!nextvar){
               nexttail=0;
               nextvar=(varent*)(vartab+varstk[varstkptr]);
               endvars=(varent*)(vartab+varstk[varstkptr+1]);
            }
            if(nexttail && nexttail>=endtails){
               nexttail=0;
               nextvar=(varent*)((char*)nextvar+nextvar->next);
            }
            if(!nexttail && nextvar>=endvars){
               request->shvret=RXSHV_LVAR;
               break;
            }
            nlen=nextvar->namelen;
            mtest(workptr,worklen,nlen,nlen-worklen+256);
            memcpy(workptr,nptr=(char*)(nextvar+1),nlen);
            if(!nexttail){
               if(!(workptr[0]&128)){
                  thisvar=nextvar;
                  nextvar=(varent*)((char*)nextvar+nextvar->next);
                  if((lev=-(thisvar->valalloc))>0)
                     thisvar=(varent*)varsearch(nptr,nlen,&lev,&i);
                  if(thisvar->vallen<0)goto case_RXSHV_NEXTV;
                  vallen=thisvar->vallen;
                  valptr=(char*)(thisvar+1)+align(thisvar->namelen);
                  nptr=workptr;
               }
               else {
                  thisvar=nextvar;
                  if((lev=-(thisvar->valalloc))>0)
                     thisvar=(varent*)varsearch(nptr,nlen,&lev,&i);
                  valptr=(char*)(thisvar+1)+align(thisvar->namelen);
                  vallen=((int*)valptr)[1];
                  nexttail=(varent*)(valptr+((int*)valptr)[0]+2*four);
                  endtails=(varent*)((char*)(thisvar+1)
                          +align(thisvar->namelen)+thisvar->vallen);
                  if(vallen>=0){
                     valptr+=2*four;
                     workptr[nlen++]='.';
                     workptr[0]&=127;
                     nptr=workptr;
                  }
                  else valptr=0;
               }
            }
            else valptr=0;
            if(!valptr){
               workptr[nlen++]='.';
               nptr=(char*)(nexttail+1);
               i=nexttail->namelen;
               mtest(workptr,worklen,i+nlen,i+nlen-worklen);
               memcpy(workptr+nlen,nptr,i);
               nlen+=i;
               thisvar=nexttail;
               nexttail=(varent*)((char *)nexttail+nexttail->next);
               if((lev=-(thisvar->valalloc))>0)
                  thisvar=(varent*)valuesearch(workptr,nlen,&lev,&i,&valptr);
               workptr[0]&=127;
               valptr=(char*)(thisvar+1)+align(thisvar->namelen);
               vallen=thisvar->vallen;
               if(vallen<0)goto case_RXSHV_NEXTV;
               nptr=workptr;
            }
            if(!request->shvname.strptr){
               request->shvname.strptr=malloc(request->shvnamelen=nlen);
               if(!request->shvname.strptr){
                  request->shvret=RXSHV_MEMFL;
                  break;
               }
            }
            if(nlen>request->shvnamelen){
               request->shvret=RXSHV_TRUNC;
               nlen=request->shvnamelen;
            }
            memcpy(request->shvname.strptr,nptr,nlen);
            request->shvname.strlength=nlen;
            if(!request->shvvalue.strptr){
               request->shvvalue.strptr=malloc(request->shvvaluelen=vallen);
               if(!request->shvvalue.strptr){
                  request->shvret|=RXSHV_MEMFL;
                  break;
               }
            }
            if(vallen>request->shvvaluelen){
               request->shvret=RXSHV_TRUNC;
               vallen=request->shvvaluelen;
            }
            memcpy(request->shvvalue.strptr,valptr,vallen);
            request->shvvalue.strlength=vallen;
            break;
         default: request->shvret=RXSHV_BADF;
      }
   }
   return ret;
}

/* Call a Unix program as a REXX function */
int unixcall(name,callname,argc)
char *name,*callname;
int argc;
{
   static char *argv[2+maxargs];
   int i;
   int l;
   int pid;
   int fd[2];
   char *ptr;
   for(i=argc;i>0;i--){
      argv[i]=delete(&l);
      if(l<0)argv[i]="";
      else argv[i][l]=0;
   }
   argv[0]=callname;
   argv[argc+1]=0;
   if(pipe(fd))perror("REXX: couldn't make a pipe"),die(Esys);
   if((pid=vfork())<0)perror("REXX: couldn't vfork"),die(Esys);
   if(!pid){  /* child: attach pipe to stdout and exec the function */
      close(fd[0]);
      if(dup2(fd[1],1)<0)perror("REXX: couldn't dup2"),_exit(-1);
      close(fd[1]);
      execv(name,argv);
      perror(name);
      _exit(-1);
   }  /* parent: read the result and stack it */
   close(fd[1]);
   i=0;
   ptr=cstackptr+ecstackptr;
   while(read(fd[0],cstackptr+ecstackptr+i,1)==1){
      i++;
      mtest(cstackptr,cstacklen,ecstackptr+i+2*four,256);
   }
   close(fd[0]);
   waitpid(pid,&l,0);         /* delete child from process table */
   if(i==0 && l==0xff00)die(Eincalled); /* catch one of the above exit(-1)s */
   if(i==0)return 0;
   ptr=cstackptr+ecstackptr;
   if(ptr[i-1]=='\n')i--;     /* knock off a trailing newline */
   l=align(i);
   *(int*)(ptr+l)=i;
   ecstackptr+=l+four;
   return 1;
}

/* API-supplied REXX functions */
int funccall(func,name,argc)  /* call function with SAA calling sequence. */
unsigned long (*func)();      /* funccall() has builtin calling sequence. */
char *name;
int argc;
{
   static RXSTRING argv[maxargs];
   static char data[RXRESULTLEN];
   RXSTRING result;
   unsigned long i;
   int j;
   int l;
   for(j=argc-1;j>=0;j--){
      argv[j].strptr=delete(&l);
      if(l<0)argv[j].strptr=0,argv[j].strlength=0;
      else argv[j].strptr[argv[j].strlength=l]=0;
   }
   MAKERXSTRING(result,data,RXRESULTLEN);
   i=func(name,argc,argv,"SESSION",&result);
   if(i)return -Ecall;
   if(!result.strptr)return 0;
   stack(result.strptr,result.strlength);
   if(result.strptr!=data)free(result.strptr);
   return 1;
}

unsigned long RexxRegisterFunctionDll(name,dllname,entryname)
char *name;
char *dllname;
char *entryname;
{
   funcinfo *info;
   int l,exist;
   void **slot;
   void *handle;
   void *address;
   static char path[MAXPATHLEN];
#ifdef NO_LDL
   return RXFUNC_NOTREG;
#else
   if(!hashlen[2]){
      for(l=0;l<3;l++)hashptr[l]=allocm(hashlen[l]=256),ehashptr[l]=0;
      if(!hashlen[2])return RXFUNC_NOMEM;
   }
   exist=which(dllname,3,path);
   if (!exist) return RXFUNC_NOTREG;
   handle=dlopen(path,1);
   if (!handle) return RXFUNC_NOTREG;
#ifdef _REQUIRED
   if (strlen(entryname)+2>sizeof path) return RXFUNC_NOMEM;
   strcpy(path+1,entryname);
   path[0]='_';
   entryname=path;
#endif
   address=dlsym(handle,entryname);
   if (!address) return RXFUNC_NOTREG;
   slot=hashfind(2,name,&exist);
   if(exist&&*slot){
      if(((funcinfo *)*slot)->dlfunc)return RXFUNC_DEFINED;
      free((char*)*slot); /* it was only a hashed file name */
   }
   info=(funcinfo *)malloc(sizeof(funcinfo));
   if(!info)return RXFUNC_NOMEM;
   *slot=(void *)info;
   info->dlhandle=handle;
   info->dlfunc=(int(*)())address;
   info->saa=1;
   return RXFUNC_OK;
#endif
}

unsigned long RexxRegisterFunctionExe(name,address)
char *name;
RexxFunctionHandler *address;
{
   funcinfo *info;
   int l,exist;
   void **slot;
   if(!hashlen[2]){
      for(l=0;l<3;l++)hashptr[l]=allocm(hashlen[l]=256),ehashptr[l]=0;
      if(!hashlen[2])return RXFUNC_NOMEM;
   }
   slot=hashfind(2,name,&exist);
   if(exist&&*slot){
      if(((funcinfo *)*slot)->dlfunc)return RXFUNC_DEFINED;
      free((char*)*slot); /* it was only a hashed file name */
   }
   info=(funcinfo *)malloc(sizeof(funcinfo));
   if(!info)return RXFUNC_NOMEM;
   *slot=(void *)info;
   info->dlhandle=0;
   info->dlfunc=(int(*)())address;
   info->saa=1;
   return RXFUNC_OK;
}

unsigned long RexxDeregisterFunction(name)
char *name;
{
   int exist;
   hashent *ptr;
   if(!hashlen[2])return RXFUNC_NOTREG;
   ptr=(hashent *)hashsearch(2,name,&exist);
   if(!(exist && ptr->value))return RXFUNC_NOTREG;
   if(!(((funcinfo*)ptr->value)->dlfunc))
      return RXFUNC_NOTREG;  /* it was only a hashed file name */
   free(ptr->value);
   ptr->value=0;
   return RXFUNC_OK;
}

unsigned long RexxQueryFunction(name)
char *name;
{
   int exist;
   hashent *ptr;
   if(!hashlen[2])return RXFUNC_NOTREG;
   ptr=(hashent *)hashsearch(2,name,&exist);
   if(!(exist && ptr->value))return RXFUNC_NOTREG;
   if(!(((funcinfo*)ptr->value)->dlfunc))
      return RXFUNC_NOTREG;  /* it was only a hashed file name */
   return RXFUNC_OK;
}

void hashfree() /* minimise memory used by hash table 1. */
{               /* Tables 0 (environment variables) and 2 (functions) */
   int hash;    /* might be needed as long as the process lives. */
   int len;
   hashent *ptr;
   FILE *fp;
   hash=1; /* used to be a for loop */
   if((ptr=(hashent *)hashptr[hash])){
      for(len=ehashptr[hash];len;
          len-=ptr->next,ptr=(hashent*)((char *)ptr+ptr->next))
         /* for hash table 1 */
         if(ptr->value){
            if((fp=((struct fileinfo *)(ptr->value))->fp))
               if(fp!=stdin && fp!=stdout && fp!=stderr)
                  fclose(fp);
            free((char*)ptr->value);
         }
      free(hashptr[hash]);
      hashptr[hash]=allocm(hashlen[hash]=256);
   }
   ehashptr[hash]=0;
}

/* Exit API calls */

/* Exit names are stored in a list (above), like environment names.  They are
   only ever needed by RexxStart(). */

unsigned long RexxRegisterExitExe(name,address,area)
char *name;
RexxExitHandler *address;
unsigned char *area;
{
   int i;
   char *tmp;
   if(!exitlen){
      exittable=(struct exitentry *)
                malloc((exitlen=16)*sizeof(struct exitentry));
      if(!exittable){
         exitlen=0;
         return RXEXIT_NOEMEM;
      }
   }
   if(strlen(name)>maxenviron)return RXEXIT_BADTYPE;
   for(i=0;i<exits && strcmp(exittable[i].name,name);i++);
   if(i<exits && exittable[i].handler)return RXEXIT_NOTREG;
   if(i==exits && exits++==exitlen){
      tmp=realloc(exittable,(exitlen+16)*sizeof(struct exitentry));
      if(!tmp)return RXEXIT_NOEMEM;
      exittable=(struct exitentry *)tmp;
      exitlen+=16;
   }
   strcpy(exittable[i].name,name);
   exittable[i].handler=address;
   exittable[i].area=area;
   return RXEXIT_OK;
}

unsigned long RexxDeregisterExit(name,mod)
char *name,*mod;
{
   int i;
   if(strlen(name)>maxenviron)return RXEXIT_BADTYPE;
   if(!exitlen)return RXEXIT_NOTREG;
   for(i=0;i<exits && strcmp(exittable[i].name,name);i++);
   if(i<exits && exittable[i].handler){
      exittable[i].handler=0;
      while(exits && !exittable[exits-1].handler)exits--; /* reclaim unused entries */
      return RXEXIT_OK;
   }
   return RXEXIT_NOTREG;
}

unsigned long RexxQueryExit(name,mod,flag,area)
char *name,*mod;
unsigned short *flag;
unsigned char *area;
{
   int i;
   if(flag)*flag=RXEXIT_NOTREG;
   if(strlen(name)>maxenviron)return RXEXIT_BADTYPE;
   if(!exitlen)return RXEXIT_NOTREG;
   for(i=0;i<exits && strcmp(exittable[i].name,name);i++);
   if(i<exits && exittable[i].handler){
      if(area && exittable[i].area)memcpy(area,exittable[i].area,8);
      else if(area)memset(area,0,8);
      if(flag)*flag=RXEXIT_OK;
      return RXEXIT_OK;
   }
   return RXEXIT_NOTREG;
}

