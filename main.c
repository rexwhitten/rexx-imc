/* The main() function of REXX/imc             (C) Ian Collier 1994 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<memory.h>
#include<string.h>
#include<signal.h>
#include<setjmp.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/file.h>
#if defined(HAS_MALLOPT) && !defined(M_MXFAST) || defined(MALLOC_DEBUG)
#include<malloc.h>
#endif
#include "const.h"
#include "globals.h"
#include "functions.h"
#include "rexxsaa.h"

static RXSTRING arglist;       /* argument list to main program */
static RXSTRING instore[2];    /* "instore" argument for RexxStart */
static RXSTRING answer;

main(argc,argv)  /* This function gives an interface between the command */
int argc;        /* line and the API function RexxStart(). */
char *argv[];
{
   int c,l;
   int optionx=0;     /* set if "-x" option present */
   int minus=0;       /* set if "-" present (take from stdin) */
   int opt;           /* argument counter */
   unsigned arglen;   /* amount of space allocated to arg string */
   unsigned argtot=0; /* length of arg string so far */
   char *input=0;     /* The source code from disk or wherever */
   int ilen;          /* The length of the source code */
   int callflags=RXMAIN; /* Flags to pass to RexxStartProgram */
   char *args;
   char *name=0;
   short rc;
   RXSTRING *progrm;
   jmp_buf exbuf;

#ifdef MALLOC_DEBUG
   malloc_debug(2);
#endif
#if defined(HAS_MALLOPT) && defined(M_MXFAST)
   mallopt(M_MXFAST,1024);
#endif

#ifdef STUFF_STACK
   callflags|=RXSTUFF;
#endif

   exitbuf=addressof(exbuf);
   if(setjmp(*exitbuf))exit(1); /* catch error during arg interpretation */

/* Argument processing */
   /* Flags are all arguments starting with "-" until a "-x" or "-" found */
   traceout=stderr;
   for(opt=1;!optionx && !minus && opt<argc && argv[opt][0]=='-';opt++)
      if(!setoption(argv[opt]+1,strlen(argv[opt]+1)))
         switch(argv[opt][1]&0xdf){
         case 'X':
            optionx=1; break;
         case 0:
            minus=1; break;
         case 'C':
         case 'S':
            if(++opt==argc)
               errordata=": no program supplied",die(Einit);
            input=allocm(ilen=1+strlen(argv[opt]));
            memcpy(input,argv[opt],ilen);
            input[ilen-1]='\n';
            break;
         case 'T':
            if(!argv[opt][2])
               if(++opt==argc)die(Etrace);
               else settrace(argv[opt]);
            else settrace(argv[opt]+2);
            break;
         case 'I':
            input=allocm(32);
            strcpy(input,"do forever;nop;end\n");
            ilen=strlen(input);
            name="<trace>";
            settrace("?a");
            break;
         case 'V':              /* "rexx -v" prints version and exits */
            callflags|=RXVERSION;/* but "rexx -v something" prints version */
            if(argc==2)         /* and behaves like "rexx something". */
               callflags=RXVERSION;
            break;
         default:
            workptr=allocm(worklen=32+strlen(argv[opt]));
            sprintf(workptr,": invalid option '%s'",argv[opt]);
            errordata=workptr;
            die(Einit);
      }
   if(opt==argc || input)
      minus=1; /* minus==0 if and only if there is a program name */
   /* get argument list in string form */
   /* estimate length and get mem for arg list */
   for(arglen=0,c=opt+!minus;c<argc;++c)arglen+=strlen(argv[c])+1;
   if(arglen)args=allocm(arglen);
   else args=0;     /* this makes arg()=0. */
   /* form list by concatenating all the arguments separated by spaces */
   for(c=opt+!minus;c<argc;++c){
      l=strlen(argv[c]);
      memcpy(args+argtot,argv[c],l);
      argtot+=l;
      if(c<argc-1)args[argtot++]=' ';
   }
   if(input){
      optionx=0;
      if(!name)name="string";
   }
   else if(minus && callflags!=RXVERSION){
      name="<stdin>";
      input=allocm(ilen=256);
      l=0;
      while(1){
         l+=fread(input+l,1,256,stdin);
         if(feof(stdin))break;
         else mtest(input,ilen,l+256,256);
      }
      if(!l || input[l-1]!='\n')input[l++]='\n';
      input=realloc(input,ilen=l);
      if(ttyout=fopen("/dev/tty","w"))
         fputs("  \b\b",ttyout),
         fclose(ttyout);
   }
   else name=argv[opt];
   if(optionx)callflags|=RXOPTIONX;
/* call the interpreter */
   arglist.strptr=args;             /* there is one argument - "args" */
   arglist.strlength=argtot;
   if(input){
      progrm=instore;
      instore[0].strptr=input;
      instore[0].strlength=ilen;
      instore[1].strptr=0;
      instore[1].strlength=0;
   }
   else progrm=0;
   l=RexxStartProgram(argv[0],args!=0,&arglist,name,(char*)0,
          progrm,"UNIX",RXCOMMAND,callflags,(PRXSYSEXIT)0,&rc,&answer);
   if(l)exit(-l);
   if(rc!=(short)(1<<15))exit(rc);
   if(!answer.strptr)exit(0);
/* Error has already been raised.  This code is not reached.
   fputs("REXX: exit value of '",stderr);
   for(l=0;l<answer.strlength;l++)putc(answer.strptr[l],stderr);
   fputs("' is an invalid return code.\n",stderr);
*/
   exit(1); /*NOTREACHED*/
}
