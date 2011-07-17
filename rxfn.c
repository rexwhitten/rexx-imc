/* The builtin functions of REXX/imc         (C) Ian Collier 1992 */

#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<unistd.h>
#include<stdlib.h>     /* includes bsearch, random */
#include<time.h>
#include<pwd.h>
#include<errno.h>
#include<fcntl.h>
#include<setjmp.h>
#include<sys/types.h>
#include<sys/time.h>
#include<signal.h>
#ifndef Solaris
#include<sys/ioctl.h>
#endif
#include<sys/param.h>
#ifndef FIONREAD
#include<sys/filio.h>
#endif
#include<sys/stat.h>
#ifdef HAS_TTYCOM
#include<sys/ttycom.h>
#else
#include<termios.h>
#endif
#include"const.h"
#include"globals.h"
#include"functions.h"
#define STDIN 0


/* How to find the number of buffered bytes in a FILE *. */
#ifdef NO_CNT
# undef _CNT
# define _CNT(x) (0)
#endif

#ifndef _CNT
# ifdef linux
#  ifdef uclibc
#   define _CNT(fp) ((fp)->__bufpos - (fp)->__bufread)
#  else
#   define _CNT(fp) ((fp)->_IO_read_end - (fp)->_IO_read_ptr)
#  endif
# else
#  ifdef __FreeBSD__
#   define _CNT(fp) ((fp)->_r)
#  else
#   define _CNT(fp) ((fp)->_cnt)
#  endif
# endif
#endif

void rxsource();
void rxerror();
void rxlength();
void rxtime();
void rxdate();
void rxleft();
void rxright();
void rxstrip();
void rxvalue();
void rxdatatype();
void rxcopies();
void rxspace();
void rxrange();
void c2x(),c2d(),b2x(),b2d(),d2c(),d2b(),d2x(),x2c(),x2d(),x2b();
void xbyte();
void rxsystem();
void rxpos();
void rxlastpos();
void rxcentre();
void rxjustify();
void rxsubstr();
void rxarg();
void rxabbrev();
void rxabs();
void rxcompare();
void rxdelstr();
void rxdelword();
void rxinsert();
void rxmax();
void rxmin();
void rxoverlay();
void rxrandom();
void rxreverse();
void rxsign();
void rxsubword();
void rxsymbol();
void rxlate();
void rxtrunc();
void rxverify();
void rxword();
void rxwordindex();
void rxwordlength();
void rxwordpos();
void rxwords();
void rxdigits();
void rxfuzz();
void rxtrace();
void rxform();
void rxformat();
void rxqueued();
void rxlinesize();
void rxbitand();
void rxbitor();
void rxbitxor();
void rxuserid();
void rxgetcwd();
void rxchdir();
void rxgetenv();
void rxputenv();
void rxopen();
void rxlinein();
void rxlineout();
void rxcharin();
void rxcharout();
void rxchars();
void rxlines();
void rxchars2();
void rxclose();
void rxfileno();
void rxfdopen();
void rxpopen();
void rxpclose();
void rxftell();
void rxstream();
void rxaddress();
void rxcondition();
void rxfuncadd();
void rxfuncdrop();
void rxfuncquery();

int compar();

void binrel(); /* The calculator routine which implements binary relations */

struct fnlist {char *name;void (*fn)();};

int rxfn(name,argc)   /* does function if possible; returns 1 if successful */
                      /* Returns -1 if the name was recognised as a math    */
                      /* function, and 0 if the name was unrecognised.      */
char *name;           /* Name of the function to call */
int argc;             /* Number of arguments passed to it */
{
   static struct fnlist names[]={   /* The name and address of ever builtin */
      {"ABBREV",     rxabbrev},       /* function, in alphabetical order      */
      {"ABS",        rxabs},
      {"ADDRESS",    rxaddress},
      {"ARG",        rxarg},
      {"B2D",        b2d},
      {"B2X",        b2x},
      {"BITAND",     rxbitand},
      {"BITOR",      rxbitor},
      {"BITXOR",     rxbitxor},
      {"C2D",        c2d},
      {"C2X",        c2x},
      {"CENTER",     rxcentre},
      {"CENTRE",     rxcentre},
      {"CHARIN",     rxcharin},
      {"CHAROUT",    rxcharout},
      {"CHARS",      rxchars},
      {"CHDIR",      rxchdir},
      {"CLOSE",      rxclose},
      {"COMPARE",    rxcompare},
      {"CONDITION",  rxcondition},
      {"COPIES",     rxcopies},
      {"D2B",        d2b},
      {"D2C",        d2c},
      {"D2X",        d2x},
      {"DATATYPE",   rxdatatype},
      {"DATE",       rxdate},
      {"DELSTR",     rxdelstr},
      {"DELWORD",    rxdelword},
      {"DIGITS",     rxdigits},
      {"ERRORTEXT",  rxerror},
      {"FDOPEN",     rxfdopen},
      {"FILENO",     rxfileno},
      {"FORM",       rxform},
      {"FORMAT",     rxformat},
      {"FTELL",      rxftell},
      {"FUZZ",       rxfuzz},
      {"GETCWD",     rxgetcwd},
      {"GETENV",     rxgetenv},
      {"INSERT",     rxinsert},
      {"JUSTIFY",    rxjustify},
      {"LASTPOS",    rxlastpos},
      {"LEFT",       rxleft},
      {"LENGTH",     rxlength},
      {"LINEIN",     rxlinein},
      {"LINEOUT",    rxlineout},
      {"LINES",      rxlines},
      {"LINESIZE",   rxlinesize},
      {"MAX",        rxmax},
      {"MIN",        rxmin},
      {"OPEN",       rxopen},
      {"OVERLAY",    rxoverlay},
      {"PCLOSE",     rxpclose},
      {"POPEN",      rxpopen},
      {"POS",        rxpos},
      {"PUTENV",     rxputenv},
      {"QUEUED",     rxqueued},
      {"RANDOM",     rxrandom},
      {"REVERSE",    rxreverse},
      {"RIGHT",      rxright},
      {"RXFUNCADD",  rxfuncadd},
      {"RXFUNCDROP", rxfuncdrop},
      {"RXFUNCQUERY",rxfuncquery},
      {"SIGN",       rxsign},
      {"SOURCELINE", rxsource},
      {"SPACE",      rxspace},
      {"STREAM",     rxstream},
      {"STRIP",      rxstrip},
      {"SUBSTR",     rxsubstr},
      {"SUBWORD",    rxsubword},
      {"SYMBOL",     rxsymbol},
      {"SYSTEM",     rxsystem},
      {"TIME",       rxtime},
      {"TRACE",      rxtrace},
      {"TRANSLATE",  rxlate},
      {"TRUNC",      rxtrunc},
      {"USERID",     rxuserid},
      {"VALUE",      rxvalue},
      {"VERIFY",     rxverify},
      {"WORD",       rxword},
      {"WORDINDEX",  rxwordindex},
      {"WORDLENGTH", rxwordlength},
      {"WORDPOS",    rxwordpos},
      {"WORDS",      rxwords},
      {"X2B",        x2b},
      {"X2C",        x2c},
      {"X2D",        x2d},
      {"XRANGE",     rxrange}
      };
#define nofun 0     /* "nofun" means "this function ain't here" */
#define numfun 87   /* The number of builtin functions */

   struct fnlist test;
   struct fnlist *ptr;
   test.name=name; /* Initialise a structure with the candidate name */
   ptr=(struct fnlist *) /* Search for a builtin function */
      bsearch((char*)&test,(char*)names,numfun,sizeof(struct fnlist),compar);
   if(!ptr)return 0;    /* no function recognised */
   (*(ptr->fn))(argc);  /* Call the builtin function */
   return 1;            /* Done. */
}

int compar(s1,s2) /* Compares two items of a function list, */
char *s1,*s2;     /* as required by bsearch()               */
{
   return strcmp(((struct fnlist*)s1)->name,((struct fnlist *)s2)->name);
}

char *undelete(l) /* A utility function like delete(l) except that */
int *l;           /* the value isn't deleted from the stack */
{
   char *ptr=cstackptr+ecstackptr-four;
   (*l)= *(int *)ptr;
   if(*l>=0)ptr-=align(*l);
   else ptr=(char *)-1;/* I don't think this is ever used */
   return ptr;
}

/* The rest of this file contains the builtin functions listed in the
   dictionary above.  In general, each function ABC() is implemented by
   the C routine rxabc().  Each routine takes one parameter - namely
   the number of arguments passed to the builtin function - and gives no
   return value.  The arguments and result of the builtin function are
   passed on the calculator stack.  A null argument (as in abc(x,,y))
   is represented by a stacked value having length -1. */

void rxsource(argc) /* souceline() function */
int argc;
{
   int i;
   char *s;
   if(!argc){
      stackint(lines); /* the number of source lines */
      return;
   }
   if(argc!=1)die(Ecall);
   if((i=getint(1))>lines||i<1)die(Erange);
   s=source[i];
   stack(s,strlen(s)); /* the ith source line */
}

void rxerror(argc)  /* errortext() function */
int argc;
{
   char *msg;
   if(argc!=1)die(Ecall);
   msg=message(getint(1));
   stack(msg,strlen(msg));
}
void rxlength(argc)
int argc;
{
   int l;
   if(argc!=1)die(Ecall);
   delete(&l);
   stackint(l);
}

/* This is used for TIME() with three parameters to collect an input time
   and convert it into a tm structure for output.  Return 0 -> successful */
static int rxgettime(type, time, usec)
char type;
struct tm *time;
long *usec;
{
   int input;
   char *string;
   char ampm[2];
   int len;
   int i;
   char c;
   time->tm_hour = time->tm_min = time->tm_sec = 0;
   *usec=0;
   if (type=='H' || type=='M' || type=='S') {
      input=getint(1);
      if (input<0 || input>86400) return -1;
   } else {
      string=delete(&len);
      for (i=0; i<len; i++) if (!string[i]) return -1;
      string[len]=0;
   }
   switch (type) {
      case 'C':
         if (sscanf(string,"%2d:%2d%2c%c",&time->tm_hour,&time->tm_min,
            ampm,&c) != 3) return -1;
         if (time->tm_hour<1 || time->tm_hour>12) return -1;
         if (ampm[1]!='m') return -1;
         switch (ampm[0]) {
            case 'a':
               if (time->tm_hour==12) time->tm_hour=0;
               break;
            case 'p':
               if (time->tm_hour!=12) time->tm_hour+=12;
               break;
            default: return -1;
         }
         break;
      case 'H': time->tm_hour=input; break;
      case 'L':
         if (sscanf(string,"%2d:%2d:%2d.%c",&time->tm_hour,&time->tm_min,
            &time->tm_sec,&c) !=4) return -1;
         string=strchr(string,'.');
         if (!string) return -1;
         i=100000;
         while((c=*++string)) {
            if (c<'0' || c>'9') return -1;
            *usec+=i*(c-'0');
            i/=10;
         }
         break;
      case 'M':
         time->tm_hour=input/60;
         time->tm_min=input%60;
         break;
      case 'N':
         if (sscanf(string,"%2d:%2d:%2d%c",&time->tm_hour,&time->tm_min,
            &time->tm_sec,&c) !=3) return -1;
         break;
      case 'S':
         time->tm_hour=input/3600;
         input=input%3600;
         time->tm_min=input/60;
         time->tm_sec=input%60;
         break;
      default: return -1;
   }
   if (time->tm_hour<0 || time->tm_hour>23 || time->tm_min<0 ||
      time->tm_min>59 || time->tm_sec<0 || time->tm_sec>59) return -1;
   return 0;
}

void rxtime(argc)
int argc;
{
   struct tm t,*t2;
   struct timezone tz;
   char ans[20];
   char opt='N';
   char type=0;
   char *arg;
   long e1;
   int e2;
   int l;
   int usec;
#ifdef DECLARE_TIMEZONE  /* everything except Sun seems to declare this */
   extern long int timezone;                               /* in time.h */
#endif
   if(!(timeflag&2))
      gettimeofday(&timestamp,&tz);/* Make a timestamp if necessary */
   timeflag|=2;
   if (argc>3) die(Ecall);
   if (argc>1) {
      if (argc==3) {
         arg=delete(&l);
         if(!l)die(Ecall);
         type=arg[0]&0xdf;
         if (isnull()) die(Ecall);
      }
      else type='N';
      if (rxgettime(type,t2=&t,&usec)) die(Ecall);
      argc=1;
      if (isnull()) {
         delete(&l);
         argc--;
      }
   } else {
      t2=localtime(&timestamp.tv_sec);/* t2 now contains all the necessary info */
      usec=timestamp.tv_usec;
   }
   if(argc==1){
      arg=delete(&l);
      if(!l)die(Ecall);
      opt=arg[0]&0xdf;
      if (type) if (opt=='E' || opt=='R' || opt=='O') die(Ecall);
   }
   switch(opt){
      case 'C':l=t2->tm_hour%12;
         if(l==0)l=12;
         sprintf(ans,"%d:%02d%s",l,t2->tm_min,(t2->tm_hour <12)?"am":"pm");
         break;
      case 'N':sprintf(ans,"%02d:%02d:%02d",t2->tm_hour,t2->tm_min,t2->tm_sec);
         break;
      case 'L':sprintf(ans,"%02d:%02d:%02d.%06d",t2->tm_hour,t2->tm_min,
                       t2->tm_sec,usec);
         break;
      case 'H':sprintf(ans,"%d",t2->tm_hour);
         break;
      case 'M':sprintf(ans,"%d",(t2->tm_hour)*60+(t2->tm_min));
         break;
      case 'S':sprintf(ans,"%d",((t2->tm_hour)*60+(t2->tm_min))*60+(t2->tm_sec));
         break;
      case 'O':
#ifdef HAS_GMTOFF
         sprintf(ans,"%ld",(long)(t2->tm_gmtoff));
#else
         sprintf(ans,"%ld",-(long)timezone+3600*(t2->tm_isdst>0));
#endif
         break;
      case 'E':
      case 'R':if(!(timeflag&1))secs=timestamp.tv_sec,
                                microsecs=timestamp.tv_usec;
         timeflag|=1,
         e2=timestamp.tv_usec-microsecs,
         e1=timestamp.tv_sec-secs;
         if(e2<0)e2+=1000000,e1--;
         if(opt=='R')secs=timestamp.tv_sec,microsecs=timestamp.tv_usec;
         if(e1||e2)sprintf(ans,"%ld.%06d",e1,e2);
         else ans[0]='0',ans[1]=0; /* "0", not "0.000000" */
         break;
      default:die(Ecall);
   }
   stack(ans,strlen(ans));
}

char *month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
                 "Aug","Sep","Oct","Nov","Dec"};
/* month names originally for rxdate() but needed for the Rexx version string*/

/* This is used in DATE() with three parameters to convert an input date
   into a Unix date */
static time_t rxgetdate(type,thisyear)
char type;
int thisyear;
{
   long t, t2;
   char *date;
   char mth[3];
   struct tm time;
   int len;
   unsigned long maxtime = (~(unsigned long)0)>>1;
   int i,y;
   char c;
   memset((void*)&time,0,sizeof time);
   if (type=='B' || type=='C' || type=='D') {
      t=getint(1);
      if (t<0) return -1;
   }
   else {
      date=delete(&len);
      for (i=0; i<len; i++) if (!date[i]) return -1;
      date[len]=0;
   }
   time.tm_isdst = 0;
   time.tm_hour = 12; /* stop DST variations from changing the date */
   time.tm_year = thisyear-1900;

   switch(type) {
      case 'C':
         if (t>36524) return -1;
         y=t*100/36524; /* approximate year represented by input value */
         if (y+2000-thisyear <= 50) t+=36524;
         t+=693594L;
         /* fall through */
      case 'B':
         t-=719162L;
         if (t > (long)(maxtime/86400) || t < -(long)(maxtime/86400))
            return -1;
         return 86400*(time_t)t;
      case 'J':
         if (sscanf(date,"%2d%3ld%c",&y,&t,&c) != 2) return -1;
         if (y<0) return -1;
         if (y+2000-thisyear <= 50) y+=100;
         time.tm_year = y;
         /* fall through */
      case 'D':
         t2=mktime(&time);
         if (t2==-1) return -1;
         if (t>366) return -1;
         return t2+t*86400;
      case 'E':
         if (sscanf(date,"%2d/%2d/%2d%c",&time.tm_mday,&time.tm_mon,
            &y,&c) != 3) return -1;
         if (y+2000-thisyear <= 50) y+=100;
         time.tm_year=y;
         break;
      case 'N':
         if (sscanf(date,"%2d %3c %4d%c",&time.tm_mday,mth,
            &y,&c) != 3) return -1;
         time.tm_year=y-1900;
         for (i=0; i<12; i++) if (!memcmp(month[i],mth,3)) break;
         if (i==12) return -1;
         time.tm_mon=i+1;
         break;
      case 'O':
         if (sscanf(date,"%2d/%2d/%2d%c",&y,&time.tm_mon,
            &time.tm_mday,&c) != 3) return -1;
         if (y+2000-thisyear <= 50) y+=100;
         time.tm_year=y;
         break;
      case 'S':
         if (sscanf(date,"%4d%2d%2d%c",&y,&time.tm_mon,
            &time.tm_mday,&c) != 3) return -1;
         time.tm_year=y-1900;
         break;
      case 'U':
         if (sscanf(date,"%2d/%2d/%2d%c",&time.tm_mon,&time.tm_mday,
            &y,&c) != 3) return -1;
         if (y+2000-thisyear <= 50) y+=100;
         time.tm_year=y;
         break;
      default: return -1;
   }
   time.tm_mon--;
   if (time.tm_mday<1 || time.tm_mday>31 || time.tm_mon<0 || time.tm_mon>11
      || time.tm_year<0) return -1;
   return mktime(&time);
}

void rxdate(argc)
int argc;
{
   static char *wkday[7]={"Sunday","Monday","Tuesday","Wednesday",
                          "Thursday","Friday","Saturday"};
   static char *fullmonth[12]={"January","February","March","April","May",
                          "June","July","August","September","October",
                          "November","December"};
   struct tm *t2;
   struct timezone tz;
   char ans[20];
   char opt='N';
   char type='N';
   char *arg;
   int l;
   long t;
   time_t time;
   if(!(timeflag&2))
      gettimeofday(&timestamp,&tz);/* Make a timestamp if necessary */
   timeflag|=2;
   time=timestamp.tv_sec;
   t2=localtime(&timestamp.tv_sec);/* t2 now contains all the necessary info */
   if(argc>3)die(Ecall);
   if (argc>1) { /* get a type and an input date of that type */
      if (argc==3) {
         arg=delete(&l);
         if(!l)die(Ecall);
         type=arg[0]&0xdf;
         if (isnull()) die(Ecall);
      }
      time=rxgetdate(type,t2->tm_year+1900);
      if (time==-1) die(Ecall);
      t2=localtime(&time);
      argc=1;
      if (isnull()) {
         argc--;
         delete(&l);
      }
   }
   if(argc==1){
      arg=delete(&l);
      if(!l)die(Ecall);
      opt=arg[0]&0xdf;
   }
   switch(opt){
      case 'B':
         if (time>=0) t=time/86400;
         else t=-((-time-1)/86400)-1; /* make sure negative numbers round down */
      sprintf(ans,"%ld",t+719162L);
         break;
      case 'C':
         t=time/86400L+25568L;
         if (t>36524) t-=36524;
         sprintf(ans,"%ld",t);
         break;
      case 'D':sprintf(ans,"%d",t2 -> tm_yday+1);
         break;
      case 'E':sprintf(ans,"%02d/%02d/%02d",t2 ->tm_mday,t2->tm_mon+1,t2->tm_year%100);
         break;
      case 'J':sprintf(ans,"%02d%03d",t2->tm_year%100,t2->tm_yday+1);
         break;
      case 'M':strcpy(ans,fullmonth[t2->tm_mon]);
         break;
      case 'N':sprintf(ans,"%d %s %d",t2->tm_mday,month[t2->tm_mon],t2->tm_year+1900);
         break;
      case 'O':sprintf(ans,"%02d/%02d/%02d",t2->tm_year%100,t2->tm_mon+1,t2->tm_mday);
         break;
      case 'S':sprintf(ans,"%04d%02d%02d",t2->tm_year+1900,t2->tm_mon+1,t2->tm_mday);
         break;
      case 'U':sprintf(ans,"%02d/%02d/%02d",t2->tm_mon+1,t2->tm_mday,t2->tm_year%100);
         break;
      case 'W':strcpy(ans,wkday[t2->tm_wday]);
         break;
      default:die(Ecall);
   }
   stack(ans,strlen(ans));
}
void rxstrip(argc)
int argc;
{
   char *arg;
   int len;
   char strip=' ';
   int flg=0;
   if(argc>3||!argc)die(Ecall);
   if(argc==3){
      arg=delete(&len);
      if(len>1||len==0)die(Ecall);
      else if(len==1)strip=arg[0];
   }
   if(argc>1){
      arg=delete(&len);
      if(!len)die(Ecall);
      else if(len>0)switch(arg[0]&0xdf){
      case 'T':flg=1;
         break;
      case 'L':flg= -1;
      case 'B':break;
      default:die(Ecall);
      }
   }
   arg=delete(&len);
   if(len<0)die(Enoarg);
   if(flg<=0)for(;arg[0]==strip&&len;arg++,len--); /* strip leading chars */
   if(flg>=0){while(len--&&arg[len]==strip);len++;}/* strip trailing chars */
   mtest(workptr,worklen,len+5,len+5); /* move to worksp before stacking, */
   memcpy(workptr,arg,len);            /* as stack() will destroy this copy */
   stack(workptr,len);
}
void rxleft(argc)
int argc;
{
   char *arg;
   int len;
   int len1;
   char pad=' ';
   int num;
   if(argc>3||argc<2)die(Ecall);
   if(argc==3){
      arg=delete(&len);
      if(len>=0){
         if(len!=1)die(Ecall);
         pad=arg[0];
      }
   }
   if((num=getint(1))<0)die(Ecall);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   len1=len>num?len:num;
   mtest(workptr,worklen,len1+5,len1+5);
   len1=len<num?len:num;
   memcpy(workptr,arg,len1);
   for(;len1<num;workptr[len1++]=pad);
   stack(workptr,num);
}
void rxright(argc)
int argc;
{
   char *arg;
   int len;
   int len1;
   int i;
   char pad=' ';
   int num;
   if(argc>3||argc<2)die(Ecall);
   if(argc==3){
      arg=delete(&len);
      if(len>0){
         if(len!=1)die(Ecall);
         pad=arg[0];
      }
   }
   if((num=getint(1))<0)die(Ecall);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   len1=len>num?len:num;
   mtest(workptr,worklen,len1+5,len1+5);
   for(i=0;len+i<num;workptr[i++]=pad);
   len1=len<num?len:num;
   memcpy(workptr+i,arg+len-len1,len1);
   stack(workptr,num);
}

char *rxgetname(nl,t) /* get a symbol (if compound symbol, substitute values
                         in tail). Afterwards, t=0 if invalid, otherwise:
                         1 normal symbol, 2 constant symbol, 3 number. */
int *nl,*t;           /* Return value is the name, nl is the length.  The   */
{                     /* result may contain garbage if the symbol was bad.  */
   static char name[maxvarname];
   int len,l,m,e,z;
   char *arg;
   char *val;
   int p;
   int i=0;
   char c;
   int dot=0;
   int constsym;
   if (num(&m,&e,&z,&l)>=0){
      /* Symbol is a number - must not contain spaces or a leading sign.
         Uppercase any 'e' in the exponent. */
      (*t)=0;
      arg=delete(&len);
      if (len>=maxvarname-1) return name;
      if (!rexxsymbol(arg[0])) return name;
      if (!rexxsymbol(arg[len-1])) return name;
      (*t)=3;
      (*nl)=len;
      memcpy(name,arg,len);
      name[len]=0;
      arg=strchr(name,'e');
      if (arg) arg[0]='E';
      return name;
   }
   arg=delete(&len);
   if (len<=0) return *t=0,name;
   constsym=rexxsymbol(uc(arg[0]))<=0; /* is it a constant symbol? */
   (*t)=1+constsym;
   if(len>=maxvarname-1)return *t=0,name;
   while(len&&arg[0]!='.') {        /* Get the stem part */
      name[i++]=c=uc((arg++)[0]),
      len--;
      if(!rexxsymbol(c))return *t=0,name;
   }
   if(len==1&&arg[0]=='.'&&!constsym)
      dot=1,len--;                  /* Delete final dot of a stem */
   while(len&&arg[0]=='.'){         /* Get each element of the tail */
      dot=1;
      name[p= i++]='.',
      ++p,
      ++arg,
      len--;
      while(len&&arg[0]!='.'){      /* copy the element */
         c=name[i++]=uc((arg++)[0]),len--;
         if(!rexxsymbol(c))return *t=0,name;
      }
      if(p!=i&&!constsym){          /* substitute it */
         name[i]=0;
         if(val=varget(name+p,i-p,&l)){
            if(len+l>=maxvarname-1)return *t=0,name;
            memcpy(name+p,val,l),i=p+l;
         }
      }
   }
   (*nl)=i;
   name[i]=0;
   if(dot&&!constsym)name[0]|=128; /* Compound symbols have the MSB set */
   return name;
}
      
void rxvalue(argc)
int argc;
{
   char *arg;
   char *val;
   char *pool=0;
   char **entry;
   int poollen;
   char *new=0;
   int newlen;
   int l,len,t;
   int oldlen;
   int path;
   if(argc==3){
      pool=delete(&poollen);
      argc--;
      pool[poollen]=0;
   }
   if(argc==2){
      new=delete(&newlen);
      argc--;
      if(newlen<0)new=0;
      else{ /* stack will be corrupted, so copy to workspace */
         mtest(workptr,worklen,newlen+1,newlen+1-worklen);
         memcpy(workptr,new,newlen);
         new=workptr;
      }
   }
   if(argc!=1)die(Ecall);
   if(pool)                /* The pool name determines what we do here */
      if(!strcasecmp(pool,"ENVIRONMENT") || !strcmp(pool,"SYSTEM")){
         arg=delete(&len);
         if(len<1 || len>varnamelen-1)die(Ecall);
         /* A valid environment variable contains REXX symbol characters
            but no '$' or '.'.  It is not uppercased. */
         if(whattype(arg[0])==2)die(Ecall);
         for(l=0;l<len;l++)
            if(whattype(arg[l])<1||arg[l]=='.'||arg[l]=='$')die(Ecall);
            else varnamebuf[l]=arg[l];
         arg=varnamebuf;
         arg[len]=0;
         if(val=getenv(arg))stack(val,strlen(val));
         else stack(cnull,0);
         if(!new)return;
         if(memchr(new,0,newlen))die(Ecall);
         path=strcmp(arg,"PATH");
         entry=(char**)hashfind(0,arg,&l);
         arg[len]='=';
         arg[len+1]=0;
         putenv(arg); /* release the previous copy from the environment */
         if(!l)*entry=allocm(len+newlen+2);
         else if(strlen(*entry)<len+newlen+2)
            if(!(*entry=realloc(*entry,len+newlen+2)))die(Emem);
         memcpy(*entry,arg,++len);
         memcpy(*entry+len,new,newlen);
         entry[0][len+newlen]=0;
         putenv(*entry);
         if(!path)hashclear(); /* clear shell's hash table on change of PATH */
         return;
      }
      /* here add more "else if"s */
      else if(strcasecmp(pool,"REXX"))die(Ecall);
   arg=rxgetname(&len,&t); /* Get the symbol name, then try to get its value */
   if (t>1) stack(arg,len); /* for constant symbol stack its name */
   else if(t&&(val=varget(arg,len,&l)))stack(val,l);
   else if(t<1)die(Ecall);/* die if it was bad */
   else { /* stack the variable's name */
      oldlen=len;
      if((l=arg[0]&128)&&!memchr(arg,'.',len))arg[len++]='.';
      arg[0]&=127,stack(arg,len);
      arg[0]|=l;
      len=oldlen;
   }
   if(new)
      if (t>1) die(Ecall); /* can't set a constant symbol */
      else varset(arg,len,new,newlen);
}

void rxdatatype(argc)
int argc;
{
   char *arg;
   int len;
   int i,numb=1,fst=1;
   int m,e,z,l;
   char c;
   if(argc>2||!argc)die(Ecall);
   if(argc==2&&isnull())delete(&len),argc--;
   if(argc==1){
      if(num(&m,&e,&z,&l)>=0)  /* numeric if true */
         delete(&l),
         stack("NUM",3);
      else delete(&l),stack("CHAR",4);
   }
   else{
      arg=delete(&len);
      if(isnull())die(Enoarg);
      if(len<1)die(Ecall);
      switch(arg[0]&0xdf){ /* Depending on type, set i to the answer */
      case 'A':arg=delete(&len);
         if(!len){i=0;break;}
         i=1;
         while(len--)if((m=alphanum((arg++)[0]))<1||m==3)i=0;
         break;
      case 'B':arg=delete(&len);
         if(!len){i=0;break;}
         i=1;
         while(len--)if((c=(arg++)[0])!='0'&&c!='1')i=0;
         break;
      case 'L':arg=delete(&len);
         if(!len){i=0;break;}
         i=1;
         while(len--)if((c=(arg++)[0])<'a'||c>'z')i=0;
         break;
      case 'M':arg=delete(&len);
         if(!len){i=0;break;}
         i=1;
         while(len--)if((c=(arg++)[0]|0x20)<'a'||c>'z')i=0;
         break;
      case 'N':i=(num(&m,&e,&z,&l)>=0),
         delete(&len);
         break;
      case 'S':arg=rxgetname(&len,&l);
         i = l>0;
         break;
      case 'U':arg=delete(&len);
         if(!len){i=0;break;}
         i=1;
         while(len--)if((c=(arg++)[0])<'A'||c>'Z')i=0;
         break;
      case 'W':numb=num(&m,&e,&z,&l),
         i=numb>=0&&(z||isint(numb,l,e)),
         delete(&len);
         break;
      case 'X':arg=delete(&len);
         i=1,l=0;
    /*   while(len&&arg[0]==' ')arg++,len--; */
         if(len && (arg[0]==' '||arg[len-1]==' ')){i=0; break;}
         while(len){
            if(arg[0]==' '){
               if(fst)fst=0;
               else if(l%2)i=0;
               l=0;
               while(len&&arg[0]==' ')arg++,len--;
            }
            if(len==0)break;
            c=(arg++)[0],len--;
            if((c-='0')<0)i=0;
            else if(c>9){
               if((c-=7)<10)i=0;
               if(c>15)if((c-=32)<10)i=0;
               if(c>15)i=0;
            }
            l++;
         }
         if(!fst&&(l%2))i=0;
         break;
      default:die(Ecall);
      }
      stack((c=i+'0',&c),1);
   }
}
void rxcopies(argc)
int argc;
{
   int copies;
   char *arg,*p;
   char *mtest_old;
   long mtest_diff;
   int len;
   int a;
   if(argc!=2)die(Ecall);
   if((copies=getint(1))<0)die(Ecall);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   if(!(len&&copies)){stack(cnull,0);return;}
   if dtest(cstackptr,cstacklen,ecstackptr+len*copies+16,len*copies+16)
      arg+=mtest_diff; /* Make room for the copies, then stack them directly */
   for(a=len*(copies-1),p=arg+len;a--;p++[0]=arg++[0]);
   ecstackptr+=align(len*=copies),
   *(int *)(cstackptr+ecstackptr)=len,
   ecstackptr+=four;
}
void rxspace(argc)
int argc;
{
   char *arg;
   int len;
   int len1,len2;
   char pad=' ';
   int num=1;
   int i;
   if(argc<1||argc>3)die(Ecall);
   if(argc==3){  /* First we find the character to pad with */
      argc--;
      arg=delete(&len);
      if(len>=0){
         if(len!=1)die(Ecall);
         pad=arg[0];
      }
   }
   if(argc==2){ /* Then the number of spaces between each word */
      argc--;
      if(isnull())delete(&len);
      else if((num=getint(1))<0)die(Ecall);
   }
   arg=delete(&len); /* and finally the phrase to operate on */
   if(len<0)die(Enoarg);
   while(len--&&arg[0]==' ')arg++;
   len++;
   while(len--&&arg[len]==' ');
   len++;
   mtest(workptr,worklen,len*(num+1),len*(num+2));
   for(len1=len2=0;len2<len;){ /* Make the result string in the workspace */
      while((workptr[len1++]=arg[len2++])!=' '&&len2<=len);
      while(len2<len&&arg[len2]==' ')len2++;
      for(i=0,len1--;i<num;workptr[len1++]=pad)i++;
   }
   if(len)len1-=num;  /* Remove the padding from after the last word */
   stack(workptr,len1);
}
void rxrange(argc)
int argc;
{
   unsigned int c2=255;
   unsigned int c1=0;
   unsigned char *arg;
   int len;
   if(argc>2)die(Ecall);
   if(argc>1){
      arg=(unsigned char *)delete(&len);
      if(len>=0)
         if(len!=1)die(Ecall);
         else c2=arg[0];
   }
   if(argc){
      arg=(unsigned char *)delete(&len);
      if(len>=0)
         if(len!=1)die(Ecall);
         else c1=arg[0];
   }
   if(c1>c2)c2+=256;
   len=c2-c1+1;
   mtest(cstackptr,cstacklen,ecstackptr+len+16,len+16);
   for(arg=(unsigned char *)(cstackptr+ecstackptr);c1<=c2;(*(arg++))=(c1++)&255);
   *(int *)(cstackptr+(ecstackptr+=align(len)))=len,
   ecstackptr+=four;
}
void c2x(argc)
int argc;
{
   char *arg;
   int len;
   int i;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   mtest(workptr,worklen,len+len,len+len-worklen);
   for(i=0;i<len;i++)xbyte(workptr+i+i,arg[i]);
   stack(workptr,len+len);
}
void xbyte(where,what) /* Place two hex digits representing "what", "where" */
char *where;
unsigned char what;
{
   unsigned char c1=what>>4;
   what&=15;
   if(what>9)what+=7;
   if(c1>9)c1+=7;
   where[0]=c1+'0',where[1]=what+'0';
}
void c2d(argc)
int argc;
{
   unsigned char *arg;
   int len;
   int n=-1;
   unsigned int num=0;
   unsigned char sign;
   int s=0;
   if(argc==2){
      argc--;
      if((n=getint(1))<0)die(Ecall);
   }
   if(argc!=1)die(Ecall);
   arg=(unsigned char *)delete(&len);
   if(n<0)n=len+1;
   while(n-->0)
      if(len>0){
         num|=(sign=arg[--len])<<s;
         if(sign&&s>=8*four||(int)num<0)die(Ecall);
         s+=8;
      }
      else sign=0;
   sign= -(sign>127);
   while(s<8*four)num|=sign<<s,s+=8;
   stackint((int)num);
}
void b2x(argc)
int argc;
{
   char *arg;
   char *ans;
   int len;
   int anslen=0;
   int n;
   int d;
   char c;
   if(argc!=1)die(Ecall);
   ans=arg=delete(&len);
   for(n=0;n<len && arg[n]!=' ' && arg[n]!='\t';n++);
                                    /* count up to first space */
   if(len && !n)die(Ebin);          /* leading spaces not allowed */
   if(!(n%=4))n=4;                  /* how many digits in first nybble */
   while(len){                      /* for each nybble */
      d=0;
      while(n--){                   /* for each digit */
         if(!len)die(Ebin);
         c=arg++[0];
         len--;
         if(c!='0' && c!='1')die(Ebin);
         d=(d<<1)+(c=='1');         /* add digit to d */
      }
      n=4;                          /* next nybble has 4 digits */
      if((d+='0')>'9')d+='A'-'9'-1; /* convert digit to hex */
      ans++[0]=d;
      anslen++;
      while(len && (*arg==' '||*arg=='\t')){
         arg++;                     /* spaces allowed between nybbles */
         if(!--len)die(Ebin);       /* trailing spaces not allowed */
      }
   }
   ecstackptr+=align(anslen);       /* finish the calculator stack */
   *(int*)(cstackptr+ecstackptr)=anslen;
   ecstackptr+=four;
}
void b2d(argc)
int argc;
{
   char *arg;
   int len;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   /* hack: do b2c then c2d */
   mtest(workptr,worklen,len,len-worklen);
   memcpy(workptr,arg,len);
   stackb(workptr,len);
   c2d(1);
}
void d2c(argc)
int argc;
{
   unsigned int num,minus;
   int n=-1;
   int l;
   unsigned char sign;
   char *ans;
   if(argc==2){
      argc--;
      if((n=getint(1))<0)die(Ecall);
   }
   if(argc!=1)die(Ecall);
   num=(unsigned)getint(1);
   minus=-num;
   sign=-((int)num<0);
   mtest(workptr,worklen,n<four?four:n,n+1+four);
   if(n<0){
      if(!num){
         stack("",1); /* stack d2c(0) - the null char from "" */
         return;
      }
      for(n=0,ans=workptr+four-1;num&&minus;n++,num>>=8,minus>>=8)
         *ans--=(char)num;
      stack(++ans,n);
      return;
   }
   for(l=n,ans=workptr+n-1;n--;num>>=8)*ans--=num?(char)num:sign;
   stack(workptr,l);
}
void d2b(argc)
int argc;
{
   int num;
   char c[8*four];
   int i;
   if(argc!=1)die(Ecall);
   if((num=getint(1))<0)die(Ecall);
   if(!num)stack("00000000",8);
   else{
      for(i=8*four;num||(i&7);c[--i]=(num&1)+'0',num>>=1);
      stack(c+i,8*four-i);
   }
}
void d2x(argc)
int argc;
{
   unsigned int num,minus;
   unsigned char sign;
   int l;
   int n=-1;
   char *ans;
   if(argc==2){
      argc--;
      if((n=getint(1))<0)die(Ecall);
   }
   if(argc!=1)die(Ecall);
   num=getint(1);
   minus=-num;
   sign=-((int)num<0);
   if(n<0){
      if(!num){stack("0",1);return;}
      mtest(workptr,worklen,2*four,2*four);
      for(n=0,ans=workptr+2*four-2;num&&minus;n+=2,num>>=8,minus>>=8)
         xbyte(ans,(char)num),ans-=2;
      if((ans+=2)[0]==(sign?'F':'0')&&(!sign||ans[1]>'7'))ans++,n--;
      stack(ans,n);
   }
   else{
      mtest(workptr,worklen,n+1,n+1-worklen);
      for(l=n,ans=workptr+n;n>0;n-=2,ans-=2,num>>=8)
         xbyte(ans,num?(char)num:sign);
      if(n<0)ans++;
      stack(ans+2,l);
   }
}
void x2c(argc)
int argc;
{
   char *arg;
   int len;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   mtest(workptr,worklen,len+1,len+1-worklen);
   memcpy(workptr,arg,len),
   stackx(workptr,len);
}
void x2d(argc)
int argc;
{
   char *arg;
   int len;
   int i;
   int num=0;
   int n=-1;
   char c;
   int k;
   int minus=0;
   if(argc==2){
      if((n=getint(1))<0)die(Ecall);
      argc--;
   }
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   if(n<0)n=len+1;
   if(n==0){stack("0",1);return;}
   if(n<=len){
      k=n;
      arg+=len-k;
      if(arg[0]>='8')minus=(~(unsigned)0)<<(4*k);
   }
   else k=len;
   for(i=0;i<k;i++){
      if((c=arg[i]-'0')<0)die(Ehex);
      if(c>9){
         if((c-=7)<0)die(Ehex);
         if(c>15)if((c-=32)<0||c>15)die(Ehex);
      }
      if((num=num*16+c)<0)die(Erange);
   }
   stackint(num|minus);
}
void x2b(argc)
int argc;
{
   char *arg,*ans;
   int len,anslen=0;
   int n;
   int i;
   int c;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   mtest(workptr,worklen,len,len-worklen);
   memcpy(workptr,arg,len);        /* copy the shorter string */
   arg=workptr;
   mtest(cstackptr,cstacklen,len*4+10,len*4+10-cstacklen);
                        /* prepare to stack the longer string */
   ans=cstackptr+ecstackptr;
   for(n=0;n<len && arg[n]!=' ' && arg[n]!='\t';n++);
                                    /* count up to first space */
   if(len && !n)die(Ebin);          /* leading spaces not allowed */
   n%=2;                            /* how many digits in first nybble */
   while(len){                      /* for each digit */
      c=arg++[0];
      len--;
      if((c<'0'||c>'9') && (c<'A'||c>'F') && (c<'a'||c>'f'))die(Ehex);
      if(c>='a')c-='a'-'A';         /* convert from hex */
      if((c-='0')>9)c-='A'-'9'-1;
      for(i=4;i--;anslen++,c=(c<<1)&15) /* convert to binary */
         ans++[0]=(c>=8)+'0';
      if(n)                         /* spaces allowed between nybbles */
         while(len && (*arg==' '||*arg=='\t')){
            arg++;
            if(!--len)die(Ebin);       /* trailing spaces not allowed */
         }
      n=!n;
   }
   if(n)die(Ehex);
   ecstackptr+=align(anslen);       /* finish the calculator stack */
   *(int*)(cstackptr+ecstackptr)=anslen;
   ecstackptr+=four;
}
   
void rxsystem(argc)
int argc;
{
   char *arg;
   int len;
   FILE *p;
   char c;
   int rc;
   int type;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   arg[len]=0;
   len=0;
   if(p=popen(arg,"r")){ /* Open a pipe, read the output, close the pipe */
      while(1){
         c=getc(p);
         if(feof(p)||ferror(p))break;
         mtest(workptr,worklen,len+1,50);
         workptr[len++]=c;
      }
      rc=pclose(p)/256;
   }
   else rc= -1;
   stack(workptr,len);
   if(rc<0||rc==1)type=Efailure;
   else type=Eerror;
   rcset(rc,type,arg);
}

int rxseterr(info)        /* Set info->errnum to indicate the I/O error */
struct fileinfo *info;    /* which just occurred on info->fp. */
{
   info->errnum=Eerrno;
   if(feof(info->fp))info->errnum=Eerrno+Eeof;
   if(ferror(info->fp))info->errnum=errno+Eerrno;
   return 0;
}

void rxpos(argc)
int argc;
{
   char *s1,*s2,*p;
   int l1,l2,start;
   if(argc!=2&&argc!=3)die(Ecall);
   if(argc==3&&isnull())argc--,delete(&l1);
   if(argc==3)start=getint(1);
   else start=1;
   if(--start<0)die(Erange);
   p=(s1=delete(&l1))+start;
   if(l1<0)die(Enoarg);
   l1-=start,
   s2=delete(&l2);
   if(l2<0)die(Enoarg);
   if(l2==0){stack("0",1);return;}
   while(l1>=l2&&memcmp(p,s2,l2))p++,l1--;
   if(l1<l2)stack("0",1);
   else stackint(p-s1+1);
}
void rxlastpos(argc)
int argc;
{
   char *s1,*s2,*p;
   int l1,l2,start;
   if(argc!=2&&argc!=3)die(Ecall);
   if(argc==3&&isnull())argc--,delete(&l1);
   if(argc==3){
      start=getint(1);
      if(start<1)die(Erange);
   }
   else start=0;
   s1=delete(&l1),
   s2=delete(&l2);
   if(l1<0||l2<0)die(Enoarg);
   if(!l2){stack("0",1);return;}
   if(start&&start<l1)l1=start;
   p=s1+l1-l2;
   while(p>=s1&&memcmp(p,s2,l2))p--;
   if(p<s1)stack("0",1);
   else stackint(p-s1+1);
}
void rxsubstr(argc)
int argc;
{
   char *arg;
   int len;
   int len1,len2;
   int i;
   char pad=' ';
   int num;
   int strlen= -1;
   if(argc>4||argc<2)die(Ecall);
   if(argc==4){
      arg=delete(&len);
      if(len>=0)
         if(len!=1)die(Ecall);
         else pad=arg[0];
   }
   if(argc>2&&isnull())delete(&len1),argc=2;
   if(argc>2)if((strlen=getint(1))<0)die(Ecall);
   num=getint(1);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   strlen=len1=strlen<0?len-num+1:strlen; /* fix up the default length */
   if(strlen<=0){          /* e.g. in substr("xyz",73) */
      stack("",0);
      return;
   }
   mtest(workptr,worklen,len1+5,len1+5);
   for(i=0;num<1&&len1;workptr[i++]=pad)num++,len1--; /* The initial padding */
   len2=len-num+1<len1?len-num+1:len1;
   if(len2<=0)len2=0;
   memcpy(workptr+i,arg+num-1,len2);  /* The substring */
   i+=len2;
   len1-=len2;
   for(;len1--;workptr[i++]=pad);    /* The final padding */
   stack(workptr,strlen);
}
void rxcentre(argc)
int argc;
{
   char *arg;
   int len;
   int num;
   int i;
   int spleft;
   char pad=' ';
   if(argc==3){
      arg=delete(&len);
      if(len>=0)
         if(len!=1)die(Ecall);
         else pad=arg[0];
      argc--;
   }
   if(argc!=2)die(Ecall);
   if((num=getint(1))<=0)die(Ecall);
   arg=delete(&len);
   if(len<0)die(Enoarg);
   mtest(workptr,worklen,num+5,num+5);
   if(len>=num)memcpy(workptr,arg+(len-num)/2,num); /* centre window on text */
   else {                                           /* centre text in window */
      spleft=(num-len)/2;
      for(i=0;i<spleft;workptr[i++]=pad);
      memcpy(workptr+i,arg,len);
      for(i+=len;i<num;workptr[i++]=pad);
   }
   stack(workptr,num);
}
void rxjustify(argc)
int argc;
{
   char *arg,*ptr;
   int len;
   int num;
   int i,j;
   int sp;
   int n=0;
   int a;
   char pad=' ';
   if(argc==3){
      arg=delete(&len);
      if(len>=0)
         if(len!=1)die(Ecall);
         else pad=arg[0];
      argc--;
   }
   if(argc!=2)die(Ecall);
   if((num=getint(1))<0)die(Ecall);
   rxspace(1);
   arg=delete(&len);
   if((sp=num-len)<=0){
      for(len=num,ptr=arg;len--;ptr++)if(ptr[0]==' ')ptr[0]=pad;
      stack(arg,num);
      return;
   }
   mtest(workptr,worklen,num+5,num+5);
   for(i=0;i<len;i++)if(arg[i]==' ')n++;
   if(!n){
      memcpy(workptr,arg,len);
      for(i=len;i<num;workptr[i++]=pad);
   }
   else{
      a=n/2;
      for(i=j=0;i<len;workptr[j++]=arg[i++])
         if(arg[i]==' '){
            arg[i]=pad;
            for(a+=sp;a>=n;a-=n)workptr[j++]=pad;
         }
   }
   stack(workptr,num);
}

void rxarg(argc)
int argc;
{
   int n;
   int i;
   int ex;
   char opt='A';
   char *arg;
   for(n=0;curargs[n];n++); /* count arguments to current procedure */
   if(argc>2)die(Ecall);
   if(argc>0&&isnull()){
      delete(&i);
      argc--;
      if(argc>0&&isnull()){
         delete(&i);
         argc--;
      }
   }
   if(argc==0){stackint(n);return;}
   if(argc==2){
      arg=delete(&i);
      if(i<1)die(Ecall);
      if((opt=arg[0]&0xdf)!='E'&&opt!='O')die(Ecall);
   }
   i=getint(1);
   if(i-- <=0)die(Ecall);
   ex=(i<n &&curarglen[i]>=0);
   switch(opt){
      case 'A':if(ex)stack(curargs[i],curarglen[i]);
         else stack(cnull,0);
         break;
      case 'O':ex=!ex;
      case 'E':stack((opt='0'+ex,&opt),1);
   }
}
void rxabbrev(argc)
int argc;
{
   int al= -1;
   char *longs,*shorts;
   int longl,shortl;
   char c;
   if(argc==3&&isnull())argc--,delete(&longl);
   if(argc==3)if((argc--,al=getint(1))<0)die(Ecall);
   if(argc!=2)die(Ecall);
   shorts=delete(&shortl);
   longs=delete(&longl);
   if(shortl<0||longl<0)die(Enoarg);
   if(al<0)al=shortl;
   c= '1'-(al>shortl||shortl>longl||memcmp(longs,shorts,shortl)),
   stack(&c,1);
}

void rxabs(argc)
int argc;
{
   int m,e,z,l,n;
   if(argc!=1)die(Ecall);
   if((n=num(&m,&e,&z,&l))<0)die(Enum);
   delete(&m);
   stacknum(workptr+n,l,e,0);
}

void rxcompare(argc)
int argc;
{
   char pad=' ';
   char *s1,*s2;
   int l1,l2,l3;
   int i;
   if(argc==3){
      s1=delete(&l1);
      if(l1>=0)
         if(l1!=1)die(Ecall);
         else pad=s1[0];
      argc--;
   }
   if(argc!=2)die(Ecall);
   s2=delete(&l2),
   s1=delete(&l1);
   if(l1<0||l2<0)die(Enoarg);
   l3=((l1<l2)?l2:l1);  /* the length of the larger string */
   for(i=0;i<l3&&(i<l2?s2[i]:pad)==(i<l1?s1[i]:pad);i++);
   if(i++==l3)i=0;
   stackint(i);
}

void rxdelstr(argc)
int argc;
{
   int n,l,d= -1;
   int osp;
   char *s;
   if(argc==3){
      argc--;
      if(isnull())delete(&l);
      else if((d=getint(1))<0)die(Ecall);
   }
   if(argc!=2)die(Ecall);
   if((n=getint(1))<1)die(Ecall);
   osp=ecstackptr;
   s=delete(&l);
   if(l<0)die(Enoarg);
   if(n>l||!d){ecstackptr=osp;return;}/* delete nothing:return the old string*/
   mtest(workptr,worklen,l,l);
   n--;
   if(d<0||n+d>l)d=l-n;
   memcpy(workptr,s,n),
   memcpy(workptr+n,s+n+d,l-n-d);
   stack(workptr,l-d);
}

void rxdelword(argc)
int argc;
{
   int n,l,d= -1,n1,d1,l1,i;
   int osp;
   char *s;
   if(argc==3){
      argc--;
      if(isnull())delete(&l);
      else if((d=getint(1))<0)die(Ecall);
   }
   if(argc!=2)die(Ecall);
   if((n=getint(1))<1)die(Ecall);
   osp=ecstackptr;
   s=delete(&l1);
   if(l1<0)die(Enoarg);
   for(i=0;i<l1&&s[i]==' ';i++);
   if(i==l1||!d){ecstackptr=osp;return;}
   n--;
   for(l=0;i<l1;l++){
      if(l==n)n1=i;
      if(l==n+d&&d>0)d1=i-n1;
      while(i<l1&&s[i]!=' ')i++;
      while(i<l1&&s[i]==' ')i++;
   }
   if(n>l-1){ecstackptr=osp;return;}
   mtest(workptr,worklen,l1,l1);
   if(d<0||n+d>l-1)d1=l1-n1;
   memcpy(workptr,s,n1),
   memcpy(workptr+n1,s+n1+d1,l1-n1-d1);
   stack(workptr,l1-d1);
}

void rxinsert(argc)
int argc;
{
   char *new,*target;
   int nl,tl;
   int n=0,length= -1;
   int i;
   char pad=' ';
   if(argc==5){
      argc--;
      new=delete(&nl);
      if(nl>=0)
         if(nl==1)pad=new[0];
         else die(Ecall);
   }
   if(argc==4){
      argc--;
      if(isnull())delete(&nl);
      else if((length=getint(1))<0)die(Ecall);
   }
   if(argc==3){
      argc--;
      if(isnull())delete(&nl);
      else if((n=getint(1))<0)die(Ecall);
   }
   if(argc!=2)die(Ecall);
   target=delete(&tl);
   new=delete(&nl);
   if(tl<0||nl<0)die(Enoarg);
   if(length<0)length=nl;
   mtest(workptr,worklen,length+n+tl,length+n+tl);
   memcpy(workptr,target,n<tl?n:tl);
   if(n>tl)for(i=tl;i<n;workptr[i++]=pad);
   memcpy(workptr+n,new,length<nl?length:nl);
   if(length>nl)for(i=nl;i<length;workptr[i++ +n]=pad);
   if(n<tl)memcpy(workptr+n+length,target+n,tl-n);
   else tl=n;
   stack(workptr,tl+length);
}

void rxminmax(argc,op) /* Calculate the minimum/maximum of a list of numbers */
int argc;   /* How many numbers are supplied */
int op;     /* What comparison operator to use */
{
   int m1,z1,e1,l1,n1,m2,z2,e2,l2,n2,d,owp;
   if(!argc)die(Enoarg);
   if((n1=num(&m1,&e1,&z1,&l1))<0)die(Enum);
   delete(&d);
   owp=eworkptr;
   while(--argc){
      eworkptr=owp;
      if((n2=num(&m2,&e2,&z2,&l2))<0)die(Enum);
      stacknum(workptr+n1,l1,e1,m1);
      binrel(op);
      if((delete(&d))[0]=='1')n1=n2,m1=m2,e1=e2,l1=l2,owp=eworkptr;
   }
   stacknum(workptr+n1,l1,e1,m1);
}

void rxmax(argc)
int argc;
{
   rxminmax(argc,OPgeq);
}

void rxmin(argc)
int argc;
{
   rxminmax(argc,OPleq);
}

void rxoverlay(argc)
int argc;
{
   char *new,*target;
   int nl,tl;
   int n=1,length= -1;
   int i;
   char pad=' ';
   if(argc==5){
      argc--;
      new=delete(&nl);
      if(nl>=0)
         if(nl==1)pad=new[0];
         else die(Ecall);
   }
   if(argc==4){
      argc--;
      if(isnull())delete(&nl);
      else if((length=getint(1))<0)die(Ecall);
   }
   if(argc==3){
      argc--;
      if(isnull())delete(&nl);
      else if((n=getint(1))<=0)die(Ecall);
   }
   n--;
   if(argc!=2)die(Ecall);
   target=delete(&tl);
   new=delete(&nl);
   if(tl<0||nl<0)die(Enoarg);
   if(length<0)length=nl;
   mtest(workptr,worklen,length+n+tl,length+n+tl);
   memcpy(workptr,target,n<tl?n:tl);
   if(n>tl)for(i=tl;i<n;workptr[i++]=pad);
   memcpy(workptr+n,new,length<nl?length:nl);
   if(length>nl)for(i=nl;i<length;workptr[i++ +n]=pad);
   if(n+length<tl)memcpy(workptr+n+length,target+n+length,tl-n-length);
   else tl=n+length;
   stack(workptr,tl);
}

void rxrandom(argc)
int argc;
{
   struct timeval t1;
   struct timezone tz;
   int min=0,max=999;
   int dummy;
#ifdef DECLARE_RANDOM
   long random();   /* everything except Sun defines this in stdlib.h */
#endif
   unsigned long r;
   if(argc==3){
      argc--;
      srandom(getint(1)),timeflag|=4;
   }
   if(!(timeflag&4)){
      timeflag|=4;
      gettimeofday(&t1,&tz);
      srandom(t1.tv_sec*50+(t1.tv_usec/19999));
   }
   if(argc>2)die(Ecall);
   if(argc&&isnull())argc--,delete(&dummy);
   if(argc&&isnull())argc--,delete(&dummy);
   if(argc)argc--,max=getint(1);
   if(argc)
      if(isnull())delete(&dummy);
      else min=getint(1);
   if(min>max||max-min>100000)die(Ecall);
   if(min==max)r=0;
   else max=max-min+1,
        r=(unsigned long)random()%max;
   stackint((int)r+min);
}

void rxreverse(argc)
int argc;
{
   char *s;
   int i,l,l2;
   char c;
   if(argc!=1)die(Ecall);
   s=undelete(&l);
   l2=l--/2;
   for(i=0;i<l2;i++)c=s[i],s[i]=s[l-i],s[l-i]=c;
}

void rxsign(argc)
int argc;
{
   int m,z,e,l;
   char c;
   if(argc!=1)die(Ecall);
   if(num(&m,&e,&z,&l)<0)die(Enum);
   delete(&l);
   if(m)stack("-1",2);
   else c='1'-z,stack(&c,1);
}

void rxsubword(argc)
int argc;
{
   char *s;
   int l,n,k= -1,i,n1,k1,l1;
   if(argc==3){
      if((k=getint(1))<0)die(Ecall);
      argc--;
   }
   if(argc!=2)die(Ecall);
   if((n=getint(1))<=0)die(Ecall);
   s=delete(&l1);
   if(l1<0)die(Enoarg);
   for(i=0;i<l1&&s[i]==' ';i++);
   n--;
   for(l=0;i<l1;l++){
      if(n==l)n1=i;
      if(k>=0&&k+n==l)k1=i-n1;
      while(i<l1&&s[i]!=' ')i++;
      while(i<l1&&s[i]==' ')i++;
   }
   if(n>=l||k==0){stack(cnull,0);return;}
   if(k<0||k+n>=l)k1=l1-n1;
   while(k1>0&&s[n1+k1-1]==' ')k1--;
   stack(s+n1,k1);
}

void rxsymbol(argc)
int argc;
{
   char *arg;
   int len,good;
   int l;
   if(argc!=1)die(Ecall);
   arg=rxgetname(&len,&good);
   if(good==1 && varget(arg,len,&l)) stack("VAR",3);
   else if(!good)stack("BAD",3);
   else stack("LIT",3);
}

void rxlate(argc)
int argc;
{
   char *s,*ti,*to;
   int sl,til= -1,tol=-1;
   int j;
   char pad=' ';
   if(argc==4){
      s=delete(&sl);
      if(sl==1)pad=s[0];
      else die(Ecall);
      argc--;
   }
   if(argc==3)argc--,ti=delete(&til);
   if(argc==2)argc--,to=delete(&tol);
   if(argc!=1)die(Ecall);
   s=undelete(&sl);
   if(sl<0)die(Enoarg);
   if(tol==-1&&til== -1)for(;sl--;s++)s[0]=uc(s[0]);
   else for(;sl--;s++){
      if(til== -1)j=s[0];
      else{
         for(j=0;j<til&&s[0]!=ti[j];j++);
         if(j==til)continue;
      }
      if(j>=tol)s[0]=pad;
      else s[0]=to[j];
   }
}

void rxtrunc(argc)
int argc;
{
   int d=0,n,m,e,z,l,i;
   char *p;
   if(argc==2){
      if(isnull())delete(&l);
      else if((d=getint(1))<0||d>5000)die(Ecall);
      argc--;
   }
   if(argc!=1)die(Ecall);
   eworkptr=2; /* Save room for a carry digits */
   if((n=num(&m,&e,&z,&l))<0)die(Enum); /* Get the number to truncate */
   delete(&i);
   if(e>0)i=l+d+e+5;
   else i=l+d+5;
   mtest(workptr,worklen,i,i);
   p=workptr+n;
   if(l>precision)  /* round it to precision before truncating */
   if(p[l=precision]>='5'){
      for(i=l-1;i>=0;i--){
         p[i]++;
         if(p[i]<='9')break;
         p[i]='0';
      }
      if(i<0)(--p)[0]='1',e++;
   }
   for(i=l;i<=e;p[i++]='0'); /* Extend the number to the decimal point */
   if(d==0&&e<0){p[0]='0';stack(p,1);return;}  /* 0 for trunc(x) where |x|<1 */
   if(d>0){
      if(e<0){
         if(e<-d)e= -d-1;
         for(i=l;i--;)p[i-e]=p[i];
         for(i=0;i<-e;p[i++]='0');
         l-=e;
         e=0;
      }
      if(l>e+1)for(i=l;i>e;i--)p[i+1]=p[i];
      p[e+1]='.';
      if(l<e+2)l=e+2;
      else l++;
      for(i=l;i<e+d+2;p[i++]='0');
      d++;
   }
   if(m)(--p)[0]='-',d++;
   stack(p,d+e+1);
}

void rxverify(argc)
int argc;
{
   char *s,*r;
   int sl,rl,st=1,opt=0;
   int i,j;
   if(argc==4){
      argc--;
      if(isnull())delete(&sl);
      else if((st=getint(1))<1)die(Ecall);
   }
   if(argc==3){
      argc--;
      s=delete(&sl);
      if(sl>=0){
         if(sl==0)die(Ecall);
         switch(s[0]&0xdf){
            case 'M':opt=1;
            case 'N':break;
            default:die(Ecall);
         }
      }
   }
   if(argc!=2)die(Ecall);
   r=delete(&rl),
   s=delete(&sl);
   if(rl<0||sl<0)die(Enoarg);
   if(st>sl)i=0;
   else{
      s+=(--st);
      for(i=st;i<sl;i++,s++){
         for(j=0;j<rl&&s[0]!=r[j];j++);
         if((j==rl)^opt)break;
      }
      if(i==sl)i=0;
      else i++;
   }
   stackint(i); 
}

void rxword(argc)
int argc;
{
   if(argc!=2)die(Ecall);
   stack("1",1);
   rxsubword(3);
}

void rxwordindex(argc)
int argc;
{
   char *s;
   int sl,n,i,l;
   if(argc!=2)die(Ecall);
   if((n=getint(1))<1)die(Ecall);
   s=delete(&sl);
   if(sl<0)die(Enoarg);
   for(i=0;i<sl&&s[0]==' ';s++,i++);
   n--;
   for(l=0;i<sl;l++){
      if(n==l)break;
      while(i<sl&&s[0]!=' ')i++,s++;
      while(i<sl&&s[0]==' ')i++,s++;
   }
   if(i==sl)i=0;
   else i++;
   stackint(i);
}

void rxwordlength(argc)
int argc;
{
   rxword(argc);
   rxlength(1);
}

void rxwordpos(argc)
int argc;
{
   char *p,*s;
   int pl,sl,st=1;
   int i,l,j,k;
   if(argc==3){
      if((st=getint(1))<1)die(Ecall);
      argc--;
   }
   if(argc!=2)die(Ecall);
   s=delete(&sl),
   p=delete(&pl);
   if(sl<0||pl<0)die(Enoarg);
   for(i=0;i<sl&&s[0]==' ';s++,i++);
   while(pl&&p[0]==' ')p++,pl--;
   while(pl--&&p[pl]==' ');
   if(!++pl){stack("0",1);return;}
   st--;
   for(l=0;i<sl;l++){
      if(l>=st){
         for(j=k=0;j<pl&&k<sl-i;j++,k++){
            if(s[k]!=p[j])break;
            if(s[k]!=' ')continue;
            while(++k<sl-i&&s[k]==' ');
            while(++j<pl&&p[j]==' ');
            j--,k--;
         }
         if(j==pl && (k==sl-i || s[k]==' '))break;
         if(k==sl-i){l= -1;break;}
      }
      while(i<sl&&s[0]!=' ')i++,s++;
      while(i<sl&&s[0]==' ')i++,s++;
   }
   if(i==sl)l=0;
   else l++;
   stackint(l);
}

void rxwords(argc)
int argc;
{
   char *s;
   int l1,l;
   if(argc!=1)die(Ecall);
   s=delete(&l1);
   while(l1&&s[0]==' ')s++,l1--;
   for(l=0;l1;l++){
      while(l1&&s[0]!=' ')s++,l1--;
      while(l1&&s[0]==' ')s++,l1--;
   }
   stackint(l);
}

void rxdigits(argc)
int argc;
{
   if(argc)die(Ecall);
   stackint(precision);
}

void rxfuzz(argc)
int argc;
{
   if(argc)die(Ecall);
   stackint(precision-fuzz);
}

void rxaddress(argc)
int argc;
{
   extern int address1;  /* from rexx.c */
   char *address=envtable[address1].name;
   if(argc)die(Ecall);
   stack(address,strlen(address));
}

void rxtrace(argc)
int argc;
{
   char *arg;
   int len;
   char ans[2];
   int q=0;
   if(argc>1)die(Ecall);
   if(trcflag&Tinteract)ans[q++]='?';
   switch(trcflag&~Tinteract&0xff){
      case Tclauses:             ans[q]='A';break;
      case Tcommands|Terrors:    ans[q]='C';break;
      case Terrors:              ans[q]='E';break;
      case Tfailures:            ans[q]='F';break;
      case Tclauses|Tintermed:   ans[q]='I';break;
      case Tlabels:              ans[q]='L';break;
      case 0:                    ans[q]='O';break;
      case Tresults|Tclauses:    ans[q]='R';
   }
   if(argc){
      arg=delete(&len);
      if(!(trcflag&Tinteract)&&interact<0 ||
          (interact==interplev-1 && interact>=0)){
               /* if interactive trace, only interpret
                  trace in the actual command, also use old trace flag
                  as the starting value */
         if (interact>=0)trclp=2,trcflag=otrcflag;
         arg[len]=0;
         settrace(arg);
      }
   }
   stack(ans,++q);
}

void rxform(argc)
int argc;
{
   if(argc)die(Ecall);
   if(numform)stack("ENGINEERING",11);
         else stack("SCIENTIFIC",10);
}

void rxformat(argc)
int argc;
{
   int n,l,e,m,z;
   int before=0,after= -1, expp= -1,expt= precision;
   char *ptr1;
   int len1=0;
   int i;
   int p;
   int c=argc;
   char *num1;
   int exp;
   if(argc==5){  /* Get the value of expt */
      argc--;
      if(!isnull()){if((expt=getint(1))<0)die(Ecall);}
      else delete(&i);
   }
   if(argc==4){  /* Get the value of expp */
      argc--;
      if(!isnull()){if((expp=getint(1))<0)die(Ecall);}
      else delete(&i);
   }
   if(argc==3){  /* Get the value of after */
      argc--;
      if(!isnull()){if((after=getint(1))<0)die(Ecall);}
      else delete(&i);
   }
   if(argc==2){  /* Get the value of before */
      argc--;
      if(!isnull()){if((before=getint(1))<=0)die(Ecall);}
      else delete(&i);
   }
   if(argc!=1)die(Ecall); /* The number to be formatted must be supplied */
   eworkptr=1;            /* allow for overflow one place to the left */
   if((n=num(&m,&e,&z,&l))<0)die(Enum);
   delete(&i);
   num1=n+workptr;
   if(c==1){ /* A simple format(number) command, in which case */
      stacknum(num1,l,e,m);                 /* format normally */
      return;
   }
   if(l>precision) /* Before processing, the number is rounded to digits() */
      if(num1[l=precision]>='5'){
         for(i=l-1;i>=0;i--){
            if(++num1[i]<='9')break;
            num1[i]='0';
         }
         if(i<0)*--num1='1';
      }
   i=l+before+after+expp+30;
   mtest(cstackptr,cstacklen,i+ecstackptr,i);
   ptr1=cstackptr+ecstackptr;
   if(z)num1[0]='0',m=e=0,l=1;              /* adjust zero to be just "0" */
   if(exp=((e<expt&&!(e<0&&l-e-1>2*expt))||!expp)) {/* no exponent */
      if(e<0)n=1+m;  /* calculate number of places before . */
      else n=e+1+m;
      p=1+e;
   }
   else{
      if(numform)n=1+m+e%3; /* number of places before . in expon. notation */
      else n=1+m;
      p=n-m;
   }
   if((p+=after)>precision||after<0)p=precision; /* what precision? */
   if (p<0 || (p==0&&num1[0]<'5')) { /* number is too small so make it "0" */
      num1[0]='0'; m=e=0; l=1;
   }
   if(l>p&&p>=0)  /* if l>p, round the number; if p<0 it needs rounding down */
      if(num1[l=p]>='5'){              /* anyway, so we don't need to bother */
         for(i=l-1;i>=0;i--){
            if(++num1[i]<='9')break;
            num1[i]='0';
         }
         if(i<0){
            (--num1)[0]='1';
            if(!l)l++; /* if that's the only '1' in the whole number, */
                       /* count it. */
            if(++e==expt&&expt&&expp)
               exp=0; /* just nudged into exponential form */
            if(exp){if(e>0)n++;}
            else
               if(numform)n=1+m+e%3;
               else n=1+m;
         }
      }
   /* should now have number rounded to fit into format, and n
      is the number of characters required for the integer part */
   if(before<n&&before)die(Eformat);
   for(n=before-n;n>0;n--)ptr1[len1++]=' ';
   if(m)ptr1[len1++]='-';
   if(exp){/* stack floating point number; no exponent */
      if(e<0){
         ptr1[len1++]='0';
         if(after){
            ptr1[len1++]='.';
            for(i= -1;i>e&&after;i--)ptr1[len1++]='0',after--;
         }
      }
      while(l&&(e>=0||after)){
         ptr1[len1++]=num1[0],
         num1++,
         l--,
         e--;
         if(l&&e==-1&&after)ptr1[len1++]='.';
         if(e<-1)after--;
      }
      while(e>-1)ptr1[len1++]='0',e--;
      if(after>0){
         if(e==-1)ptr1[len1++]='.';
         while(after--)ptr1[len1++]='0';
      }
   }
   else{/*stack floating point in appropriate form with exponent */
      ptr1[len1++]=num1[0];
      if(numform)while(e%3)
            e--,
            ptr1[len1++]=((--l)>0 ? (++num1)[0] : '0');
      else --l;
      if((l>0 && after<0)||after>0){
         ptr1[len1++]='.';
         while(l--&&after)ptr1[len1++]=(++num1)[0],after--;
         while(after-- >0)ptr1[len1++]='0';
      }
      if(!e){
         if(expp>0)for(i=expp+2;i--;ptr1[len1++]=' ');
      }
      else{
         ptr1[len1++]='E',
         ptr1[len1++]= e<0 ? '-' : '+',
         e=abs(e);
         for(p=0,i=1;i<=e;i*=10,p++);
         if(expp<0)expp=p;
         if(expp<p)die(Eformat);
         for(p=expp-p;p--;ptr1[len1++]='0');
         for(i/=10;i>=1;i/=10)
            ptr1[len1++]=e/i+'0',
            e%=i;
      }
   }
   *(int *)(ptr1+align(len1))=len1;
   ecstackptr+=align(len1)+four;
}

void rxqueued(argc)
int argc;
{
   int l;
   static char buff[8];
   if(argc)die(Ecall);
   if(write(rxstacksock,"N",1)<1)die(Esys);
   if(read(rxstacksock,buff,7)<7)die(Esys);
   sscanf(buff,"%x",&l);
   stackint(l);
}

void rxlinesize(argc)
int argc;
{
   int ans;
   struct winsize sz;
   if(argc)die(Ecall);
   if(!ioctl(fileno(ttyout),TIOCGWINSZ,&sz))ans=sz.ws_col;
   else ans=0;
   stackint(ans);
}

void rxbitand(argc)
int argc;
{
   char *arg1,*arg2,*argt;
   int len1,len2,lent;
   unsigned char pad=255;
   if(argc==3){
      argt=delete(&lent);
      if(lent!=1)die(Ecall);
      pad=argt[0];
      argc--;
   }
   if(argc==2){
      arg2=delete(&len2);
      if(len2==-1)len2=0;
   }
   else{
      if(argc!=1)die(Ecall);
      len2=0;
   }
   arg1=delete(&len1);
   if(len1<0)die(Ecall);
   if(len1<len2)argt=arg1,arg1=arg2,arg2=argt,lent=len1,len1=len2,len2=lent;
   argt=cstackptr+ecstackptr;
   for(lent=0;lent<len1;lent++)
      argt[lent]=arg1[lent]&(lent<len2?arg2[lent]:pad);
   argt+=lent=align(len1);
   *(int *)argt=len1;
   ecstackptr+=lent+four;
}
void rxbitor(argc)
int argc;
{
   char *arg1,*arg2,*argt;
   int len1,len2,lent;
   char pad=0;
   if(argc==3){
      argt=delete(&lent);
      if(lent!=1)die(Ecall);
      pad=argt[0];
      argc--;
   }
   if(argc==2){
      arg2=delete(&len2);
      if(len2==-1)len2=0;
   }
   else{
      if(argc!=1)die(Ecall);
      len2=0;
   }
   arg1=delete(&len1);
   if(len1<0)die(Ecall);
   if(len1<len2)argt=arg1,arg1=arg2,arg2=argt,lent=len1,len1=len2,len2=lent;
   argt=cstackptr+ecstackptr;
   for(lent=0;lent<len1;lent++)
      argt[lent]=arg1[lent]|(lent<len2?arg2[lent]:pad);
   argt+=lent=align(len1);
   *(int *)argt=len1;
   ecstackptr+=lent+four;
}
void rxbitxor(argc)
int argc;
{
   char *arg1,*arg2,*argt;
   int len1,len2,lent;
   char pad=0;
   if(argc==3){
      argt=delete(&lent);
      if(lent!=1)die(Ecall);
      pad=argt[0];
      argc--;
   }
   if(argc==2){
      arg2=delete(&len2);
      if(len2==-1)len2=0;
   }
   else{
      if(argc!=1)die(Ecall);
      len2=0;
   }
   arg1=delete(&len1);
   if(len1<0)die(Ecall);
   if(len1<len2)argt=arg1,arg1=arg2,arg2=argt,lent=len1,len1=len2,len2=lent;
   argt=cstackptr+ecstackptr;
   for(lent=0;lent<len1;lent++)
      argt[lent]=arg1[lent]^(lent<len2?arg2[lent]:pad);
   argt+=lent=align(len1);
   *(int *)argt=len1;
   ecstackptr+=lent+four;
}

void rxuserid(argc)
int argc;
{
   void endpwent();
   static int uid=-1;
   int cuid;
   static struct passwd *pw=0;
   if(argc)die(Ecall);
   if((cuid=getuid())!=uid)
      uid=cuid,
      pw=getpwuid(cuid),
      endpwent();
   if(!pw)stack(cnull,0);
   else stack(pw->pw_name,strlen(pw->pw_name));
}

void rxgetcwd(argc)
int argc;
{
   static char name[MAXPATHLEN];
   if(argc)die(Ecall);
   if (!getcwd(name,MAXPATHLEN)) {
      char *err=strerror(errno);
      if (!err) err="Unknown error occurred";
      if (strlen(err) < MAXPATHLEN) strcpy(name,err);
      else {
         memcpy(name,err,MAXPATHLEN-1);
         name[MAXPATHLEN-1]=0;
      }
   }
   stack(name,strlen(name));
}

void rxchdir(argc)
int argc;
{
   char *arg;
   int len;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   arg[len]=0; /* that location must exist since the length used to be
                  after the string */
   if(chdir(arg))stackint(errno);
   else stack("0",1);
}

void rxgetenv(argc)
int argc;
{
   char *arg;
   int len;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   arg[len]=0;
   if(arg=getenv(arg))stack(arg,strlen(arg));
   else stack(cnull,0);
}

void rxputenv(argc)
int argc;
{
   char *arg;
   char *eptr;
   int len;
   int exist;
   char **value;
   int path;
   if(argc!=1)die(Ecall);
   arg=delete(&len);
   arg[len++]=0;
   if(!(eptr=strchr(arg,'=')))die(Ecall);
   eptr[0]=0;
   value=(char**)hashfind(0,arg,&exist);
   path=strcmp(arg,"PATH");
   eptr[0]='=';
   putenv(arg); /* release the previous copy from the environment */
   if(!exist)*value=allocm(len);
   else if(strlen(*value)<len)
      if(!(*value=realloc(*value,len)))die(Emem);
   strcpy(*value,arg);
   if(putenv(*value))stack("1",1);
   else stack("0",1);
   if(!path)hashclear(); /* clear shell's hash table on change of PATH */
}

void rxopen2(stream,mode,mlen,path,plen)
char *stream,*mode,*path;   /* implement open(stream,mode,path) */
int mlen,plen;
{
   char modeletter[3];
   struct fileinfo *info;
   FILE *fp;
   int rc;
   modeletter[0]='r';
   modeletter[1]=modeletter[2]=0;
   if(plen<=0)path=stream,plen=strlen(stream);
   if(memchr(path,0,plen))die(Ecall);
   path[plen]=0;
   if(mlen>0)switch(mode[0]&0xdf){
      case 'R': break;
      case 'W': modeletter[0]='w';
                modeletter[1]='+';
                break;
      case 'A': rc=access(path,F_OK);
                modeletter[0]=rc?'w':'r';
                modeletter[1]='+';
                break;
      default:  die(Ecall);
   }
   if(info=(struct fileinfo *)hashget(1,stream,&rc)){
      fp=info->fp;          /* if "stream" already exists, perform freopen */
      free((char *)info);
      *(struct fileinfo **)hashfind(1,stream,&rc)=0;
      fp=freopen(path,modeletter,info->fp);
   }
   else fp=fopen(path,modeletter);
   if(!fp){
      stackint(errno);
      return;
   }
   if(modeletter[0]=='r'&&modeletter[1]=='+') /* for append, go to eof */
      fseek(fp,0L,2);
   info=fileinit(stream,path,fp);
   info->wr=modeletter[1]=='+';
   stack("0",1);
}

void rxopen(argc)
int argc;
{
   char *stream,*mode,*path;
   int len=0,mlen=0,plen;
   if(argc==3){
      argc--;
      stream=delete(&len);
      if(len<0)stream=0;
      else
         if(memchr(stream,0,len))die(Ecall);
         else stream[len]=0;
      if(!len)die(Ecall);
   }
   if(argc==2){
      argc--;
      mode=delete(&mlen);
   }
   if(argc!=1)die(Ecall);
   path=delete(&plen);
   if(plen<=0)die(Ecall);
   path[plen]=0;
   if(len<=0)stream=path,len=plen;
   rxopen2(stream,mode,mlen,path,plen);
}

void rxfdopen2(stream,mode,modelen,n,nlen) /* implement fdopen(stream,mode,n)*/
char *stream;
char *n;
int nlen;
char *mode;
int modelen;
{
   int fd;
   char fmode[3];
   FILE *fp;
   int streamlen=strlen(stream);
   fmode[0]='r';
   fmode[1]=fmode[2]=0;
   if(nlen<=0)n=stream,nlen=streamlen; /* default number is same as name */
   mtest(workptr,worklen,nlen+streamlen+2,nlen+streamlen+2-worklen);
   memcpy(workptr,n,nlen);
   workptr[nlen]=0;
   memcpy(workptr+nlen+1,stream,streamlen+1);
   eworkptr=nlen+streamlen+2;
   stack(workptr,nlen);
   fd=getint(1);       /* convert the fd to an integer */
   if(modelen>0)switch(mode[0]&0xdf){
      case 'R': break;
      case 'W': fmode[0]='w';
                fmode[1]='+';
                break;
      case 'A': fmode[0]='r';
                fmode[1]='+';
                break;
      default:  die(Ecall);
   }
   if(fp=fdopen(fd,fmode)){
      fileinit(workptr+nlen+1,cnull,fp)->wr=fmode[1]=='+';
      errno=0;
   }
   stackint(errno);
}

void rxfdopen(argc)
int argc;
{
   char *stream,*n,*mode;
   int len=0,nlen=0,modelen=0;
   if(argc==3){
      argc--;
      stream=delete(&len);
      if(len>0)
         if(memchr(stream,0,len))die(Ecall);
         else stream[len]=0;
      if(len==0)die(Ecall);
      stream[len]=0;
   }
   if(argc==2){
      argc--;
      mode=delete(&modelen);
      if(modelen==0)die(Ecall);
   }
   if(argc!=1)die(Ecall);
   n=delete(&nlen);
   n[nlen]=0;
   if(nlen<=0)die(Ecall);
   if(len<=0)stream=n,len=nlen;
   rxfdopen2(stream,mode,modelen,n,nlen);
}

void rxpopen2(stream,mode,mlen,command,comlen)
char *stream,*mode,*command;      /* implement popen(stream,mode,command) */
int mlen,comlen;
{
   char fmode[2];
   int rc;
   FILE *fp;
   struct fileinfo *info;
   fmode[0]='r';
   fmode[1]=0;
   if(mlen>0)fmode[0]=mode[0]|0x20;
   if(fmode[0]!='r'&&fmode[0]!='w')die(Ecall);
   if(comlen<=0)command=stream,comlen=strlen(stream);
   else command[comlen]=0;
   if(memchr(command,0,comlen))die(Ecall);
   if(fp=popen(command,fmode)){
      info=fileinit(stream,cnull,fp);
      info->wr=-(fmode[0]=='w'),
      info->lastwr=-(info->wr);
      rc=0;
   }
   else rc=errno;
   stackint(rc);
}

void rxpopen(argc)
int argc;
{
   char *stream,*mode,*command;
   int len=0,mlen=0,comlen;
   if(argc==3){
      argc--;
      stream=delete(&len);
      if(len<0)stream=0;
      else
         if(memchr(stream,0,len))die(Ecall);
         else stream[len]=0;
      if(!len)die(Ecall);
   }
   if(argc==2){
      argc--;
      mode=delete(&mlen);
   }
   if(argc!=1)die(Ecall);
   command=delete(&comlen);
   if(comlen<=0)die(Ecall);
   command[comlen]=0;
   if(len<=0)stream=command,len=comlen;
   rxpopen2(stream,mode,mlen,command,comlen);
}

void rxlinein(argc)
int argc;
{
   char *name=0;
   int lines=1;
   int pos= 0;
   int len;
   int call;
   int ch=0;
   long filepos;
   struct fileinfo *info;
   FILE *fp;
   if(argc==3){
      argc--;
      if(isnull())delete(&len);
      else if((lines=getint(1))!=0&&lines!=1)die(Ecall);
   }
   if(argc==2){
      argc--;
      if(isnull())delete(&len);
      else if((pos=getint(1))<1)die(Ecall);
   }
   if(argc==1){
      argc--;
      name=delete(&len);
      if(len<=0)name=0;
      else
         if(memchr(name,0,len))die(Ecall);
         else name[len]=0;
   }
   if(argc)die(Ecall);
   if(!name)name="stdin";
   if(!(info=(struct fileinfo *)hashget(1,name,&len))){/* If not found, then */
      fp=fopen(name,"r");                             /* open it for reading */
      info=fileinit(name,name,fp);
      if(!fp){
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
         stack(cnull,0);
         return;
      }
      info->lastwr=0;
   }
   else fp=info->fp;
   if(!fp){
      rcset(info->errnum-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(info->wr<0){
      info->errnum=Eread;
      rcset(Eread-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(info->persist && info->lastwr==0 &&
         (filepos=ftell(info->fp))>=0 && filepos!=info->rdpos)
      info->rdpos=filepos,
      info->rdline=0; /* position has been disturbed by external prog */
   clearerr(fp);      /* Ignore errors and try from scratch */
   info->errnum=0;
   if(info->lastwr || pos>0)len=fseek(fp,info->rdpos,0);
   else len=0;
   info->lastwr=0;
   if(pos>0 && (len<0 || !info->persist)){
      info->errnum=Eseek;        /* Seek not allowed on transient stream */
      rcset(Eseek-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(pos>0){                   /* Search for given line number (ugh!) */
      if(info->rdline==0 || info->rdline+info->rdchars>pos)
         fseek(fp,0L,0),
         info->rdline=1;
      info->rdchars=0;
      for(;ch!=EOF&&info->rdline<pos;info->rdline++)
         while((ch=getc(fp))!='\n'&&ch!=EOF);
      if(ch==EOF){
         info->rdline--;
         info->errnum=Ebounds;
         rcset(Ebounds-Eerrno,Enotready,name);
         stack(cnull,0);
         return;
      }
   }
   len=0;
   if(lines){
      call=sgstack[interplev].callon&(1<<Ihalt) |
           sgstack[interplev].delay &(1<<Ihalt);
      if(!call)siginterrupt(2,1); /* Allow ^C during read */
      while((ch=getc(fp))!='\n'&&ch!=EOF){
         mtest(pull,pulllen,len+1,256);
         pull[len++]=ch;
      }
      siginterrupt(2,0);
      if(delayed[Ihalt] && !call)
         delayed[Ihalt]=0,
         fseek(fp,info->rdpos,0), /* reset to start of line, if possible */
         die(Ehalt);
      if(info->rdline)info->rdline++;
      info->rdchars=0;
   }
   if(ch==EOF&&!len)rxseterr(info);
   if(info->persist && (info->rdpos=ftell(fp))<0)info->rdpos=0;
   if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,name);
   stack(pull,len);
}
      
void rxlineout(argc)
int argc;
{
   char *name=0;
   char *file;
   int pos= 0;
   int charlen=0;
   int len;
   int acc;
   int ch=0;
   char *chars=0;
   long filepos;
   struct fileinfo *info;
   FILE *fp;
   if(argc==3){
      argc--;
      if(isnull())delete(&len);
      else if((pos=getint(1))<1)die(Ecall);
   }
   if(argc==2){
      argc--;
      chars=delete(&charlen);
      if(charlen<0)chars=0;
      else if(memchr(chars,'\n',charlen))die(Ecall);
   }
   if(argc==1){
      argc--;
      name=delete(&len);
      if(len<=0)name=0;
      else
         if(memchr(name,0,len))die(Ecall);
         else name[len]=0;
   }
   if(argc)die(Ecall);
   if(!name)name="stdout";
   if(!(info=(struct fileinfo *)hashget(1,name,&len))){
      acc=access(name,F_OK);  /* If not found in table, then open for append */
      fp=fopen(name,acc?"w+":"r+");
      if(fp)fseek(fp,0L,2);
      info=fileinit(name,name,fp);
      if(!fp){
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
         stack(chars?"1":"0",1);
         return;
      }
      info->wr=1;
   }
   else fp=info->fp;
   if(!fp){
      rcset(info->errnum-Eerrno,Enotready,name);
      stack(chars?"1":"0",1);
      return;
   }
   if(!info->wr){  /* If it is open for reading, try to reopen for writing */
      file=(char*)(info+1);
      if(!file[0]){ /* reopen not allowed, since file name not given */
         info->errnum=Eaccess;
         rcset(Eaccess-Eerrno,Enotready,name);
         stack(chars?"1":"0",1);
         return;
      }
      if(!(fp=freopen(file,"r+",fp))){
         info->errnum=errno+Eerrno;
         fp=fopen(file,"r");/* try to regain read access */
         info->fp=fp;
         if(fp)fseek(fp,info->rdpos,0);
         rcset(info->errnum-Eerrno,Enotready,name);
         stack(chars?"1":"0",1);
         file[0]=0;         /* Prevent this whole thing from happening again */
         return;
      }
      info->wr=1;
      fseek(fp,0L,2);
      info->wrline=0;
      info->lastwr=1;
      if((info->wrpos=ftell(fp))<0)info->wrpos=0;
   }
   if(info->persist && info->lastwr &&
         (filepos=ftell(fp))>=0 && filepos!=info->wrpos)
      info->wrpos=filepos,
      info->wrline=0;  /* position has been disturbed by external prog */
   clearerr(fp);       /* Ignore errors and try from scratch */
   info->errnum=0;
   if(info->lastwr==0 || pos>0)len=fseek(fp,info->wrpos,0);
   else len=0;
   info->lastwr=1;
   if(pos>0 && (len<0 || !info->persist)){
      info->errnum=Eseek;       /* Seek not allowed on transient stream */
      rcset(Eseek-Eerrno,Enotready,name);
      stack(chars?"1":"0",1);
      return;
   }
   if(pos>0){                   /* Search for required line number (Ugh!) */
      if(info->wrline==0 || info->wrline+info->wrchars>pos)
         fseek(fp,0L,0),
         info->wrline=1;
      info->wrchars=0;
      for(;ch!=EOF&&info->wrline<pos;info->wrline++)
         while((ch=getc(fp))!='\n'&&ch!=EOF);
      fseek(fp,0L,1);          /* seek between read and write */
      if(ch==EOF){
         info->wrline--;
         info->errnum=Ebounds;
         rcset(Ebounds-Eerrno,Enotready,name);
         stack(chars?"1":"0",1);
         return;
      }
   }
   if(!chars){
      if(!pos){   /* No data and no position given so flush and go to EOF */
         if (fflush(fp)) rxseterr(info);
         fseek(fp,0L,2);
         info->wrline=0;
      }
      if((info->wrpos=ftell(fp))<0)info->wrpos=0; /* just pos given */
      if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,name);
      stack("0",1);
      return;
   }
   chars[charlen++]='\n';
   if(fwrite(chars,charlen,1,fp)){
      stack("0",1);
      if(info->wrline)info->wrline++;
      info->wrchars=0;
      if(info->persist && (info->wrpos=ftell(fp))<0) info->wrpos=0;
      if(setrcflag)rcset(0,Enotready,name);
   }else{
      stack("1",1);
      rxseterr(info);
      fseek(fp,info->wrpos,0);
      rcset(info->errnum-Eerrno,Enotready,name);
   }
}

void rxcharin(argc)
int argc;
{
   char *name=0;
   int chars=1;
   int pos= 0;
   int len;
   int l;
   int call;
   long filepos;
   struct fileinfo *info;
   FILE *fp;
   if(argc==3){
      argc--;
      if(isnull())delete(&len);
      else if((chars=getint(1))<0)die(Ecall);
   }
   if(argc==2){
      argc--;
      if(isnull())delete(&len);
      else if((pos=getint(1))<1)die(Ecall);
   }
   if(argc==1){
      argc--;
      name=delete(&len);
      if(len<=0)name=0;
      else
         if(memchr(name,0,len))die(Ecall);
         else name[len]=0;
   }
   if(argc)die(Ecall);
   if(!name)name="stdin";
   if(!(info=(struct fileinfo *)hashget(1,name,&len))){
      fp=fopen(name,"r"); /* not found in table so try to open */
      info=fileinit(name,name,fp);
      if(!fp){
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
         stack(cnull,0);
         return;
      }
      info->lastwr=0;
   }
   else fp=info->fp;
   if(!fp){
      rcset(info->errnum-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(info->wr<0){
      info->errnum=Eread;
      rcset(Eread-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(info->persist && info->lastwr==0 &&
         (filepos=ftell(info->fp))>=0 && filepos!=info->rdpos)
      info->rdpos=filepos,
      info->rdline=0; /* position has been disturbed by external prog */
   clearerr(fp);
   info->errnum=0;
   if(pos>0 && (!info->persist || fseek(fp,0L,2)<0)){
      info->errnum=Eseek;       /* Seek not allowed on transient stream */
      rcset(Eseek-Eerrno,Enotready,name);
      stack(cnull,0);
      return;
   }
   if(pos){
      filepos=ftell(fp);      
      if(fseek(fp,(long)pos-1,0)>=0)info->rdpos=pos-1;
      info->rdline=0;
      if(filepos<pos){          /* Seek was out of bounds */
         info->errnum=Ebounds;
         rcset(Ebounds-Eerrno,Enotready,name);
         stack(cnull,0);
         return;
      }
   }
   else if(info->lastwr)fseek(fp,info->rdpos,0);
   info->lastwr=0;
   call=sgstack[interplev].callon&(1<<Ihalt) |
        sgstack[interplev].delay &(1<<Ihalt);
   if(!call)siginterrupt(2,1); /* allow ^C to interrupt */
   mtest(workptr,worklen,chars,chars-worklen);
   len=fread(workptr,1,chars,fp);
   siginterrupt(2,0);
   if(delayed[Ihalt] && !call)
      delayed[Ihalt]=0,
      fseek(fp,info->rdpos,0),
      die(Ehalt);
   if(len&&info->rdline){ /* Try to keep the line counter up to date */
      for(l=0;l<len;)if(workptr[l++]=='\n')info->rdline++;
      if(workptr[len-1]!='\n')info->rdchars=1;
   }
   if(len<chars)rxseterr(info);
   if(info->persist && (info->rdpos=ftell(fp))<0)info->rdpos=0;
   if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,name);
   stack(workptr,len);
}

void rxcharout(argc)
int argc;
{
   char *name=0;
   char *file;
   int pos= 0;
   int charlen;
   int len;
   int acc;
   int l;
   char *chars=0;
   long filepos;
   struct fileinfo *info;
   FILE *fp;
   if(argc==3){
      argc--;
      if(isnull())delete(&len);
      else if((pos=getint(1))<1)die(Ecall);
   }
   if(argc==2){
      argc--;
      chars=delete(&charlen);
      if(charlen<0)chars=0,charlen=0;
   }
   else charlen=0;
   if(argc==1){
      argc--;
      name=delete(&len);
      if(len<=0)name=0;
      else
         if(memchr(name,0,len))die(Ecall);
         else name[len]=0;
   }
   if(argc)die(Ecall);
   if(!name)name="stdout";
   if(!(info=(struct fileinfo *)hashget(1,name,&len))){
      acc=access(name,F_OK); /* If not found in table, open for append */
      fp=fopen(name,acc?"w+":"r+");
      if(fp)fseek(fp,0L,2);
      info=fileinit(name,name,fp);
      if(!fp){
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
         stackint(charlen);
         return;
      }
      info->wr=1;
   }
   else fp=info->fp;
   if(!fp){
      rcset(info->errnum-Eerrno,Enotready,name);
      stackint(charlen);
      return;
   }
   if(!info->wr){ /* If not open for write, try to gain write access */
      file=(char*)(info+1);
      if(!file[0]){
         info->errnum=Eaccess;
         rcset(Eaccess-Eerrno,Enotready,name);
         stackint(charlen);
         return;
      }
      if(!(fp=freopen(file,"r+",fp))){
         info->errnum=errno+Eerrno;
         fp=fopen(file,"r");/* try to regain read access */
         info->fp=fp;
         if(fp)fseek(fp,info->rdpos,0);
         rcset(info->errnum-Eerrno,Enotready,name);
         stackint(charlen);
         file[0]=0;         /* Prevent this whole thing from happening again */
         return;
      }
      info->wr=1;
      fseek(fp,0L,2);
      info->wrline=0;
      info->lastwr=1;
      if((info->wrpos=ftell(fp))<0)info->wrpos=0;
   }
   if(info->persist && info->lastwr &&
         (filepos=ftell(fp))>=0 && filepos!=info->wrpos)
      info->wrpos=filepos,
      info->wrline=0;  /* position has been disturbed */
   clearerr(fp);
   info->errnum=0;
   if(pos>0 && (!info->persist || fseek(fp,0L,2)<0)){
      info->errnum=Eseek;        /* Seek not allowed on transient stream */
      rcset(Eseek-Eerrno,Enotready,name);
      stackint(charlen);
      return;
   }
   if(pos){
      filepos=ftell(fp);
      if(fseek(fp,(long)pos-1,0)>=0)info->wrpos=pos-1;
      info->wrline=0;
      if(filepos+1<pos){        /* Seek was out of bounds */
         info->errnum=Ebounds;
         rcset(Ebounds-Eerrno,Enotready,name);
         stack(cnull,0);
         return;
      }
   }
   else if(info->lastwr==0)fseek(fp,info->wrpos,0);
   info->lastwr=1;
   if(!chars){
      if(!pos){   /* No data, no pos, so flush and seek to EOF */
         if (fflush(fp)) rxseterr(info);
         fseek(fp,0L,2);
         info->wrline=0;
      }
      if((info->wrpos=ftell(fp))<0)info->wrpos=0; /* no data, so OK */
      if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,name);
      stack("0",1);
      return;
   }
   len=fwrite(chars,1,charlen,fp);
   info->wrpos+=len;
   if(len&&info->wrline){
      for(l=0;l<len;)if(chars[l++]=='\n')info->wrline++;
      if(chars[len-1]!='\n')info->wrchars=1;
   }
   if(len<charlen)rxseterr(info);
   if(info->persist && (info->wrpos=ftell(fp))<0) info->wrpos=0;
   if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,name);
   stackint(charlen-len);
}

void rxchars(argc)
int argc;
{
   rxchars2(argc,0);
}
void rxlines(argc)
int argc;
{
   rxchars2(argc,1);
}

void rxchars2(argc,line) /* = rxchars(argc) if line==0, or rxlines(argc) o/w */
int argc,line;
{
   long chars;
   long(filepos);
   int lines;
   char *name=0;
   int len;
   struct fileinfo *info;
   struct stat buf;
   int ch,c2;
   FILE *fp;
   if(argc==1){
      name=delete(&len);
      if(len<=0)name=0;
      else
         if(memchr(name,0,len))die(Ecall);
         else name[len]=0;
   }
   else if(argc)die(Ecall);
   if(!name)name="stdin";
   info=(struct fileinfo *)hashget(1,name,&len);
   if(info && !info->fp){
      rcset(info->errnum-Eerrno,Enotready,name);
      stack("0",1);
      return;
   }
   if(info && info->wr<0){
      info->errnum=Eread;
      rcset(Eread-Eerrno,Enotready,name);
      stack("0",1);
      return;
   }
   if(info){
#ifdef FSTAT_FOR_CHARS  /* fstat appears to be quicker (and more
                           correct) than seeking to EOF and back. */
      if(   info->persist &&
            !fstat(fileno(info->fp),&buf) &&
            S_ISREG(buf.st_mode)){
         if(info->lastwr || (filepos=ftell(info->fp))<0)
            filepos=info->rdpos;
         chars=buf.st_size-filepos;
         if(chars<0)chars=0;
      } else
#endif
      {
         if(info->lastwr)fseek(info->fp,info->rdpos,0);
         if(ioctl(fileno(info->fp),FIONREAD,&chars))chars=0;
         chars+=_CNT(info->fp); /* add the number of buffered chars */
      }
      if(line && info->persist && (filepos=ftell(info->fp))>=0){
         lines=0;
         c2='\n';
         while((ch=getc(info->fp))!=EOF){ /* count lines */
            if(ch=='\n')lines++;
            c2=ch;
         }
         if(c2!='\n')lines++;
         fseek(info->fp,filepos,0);
      }
      else lines=(chars>0);
   }
   else { /* Not open.  Try to open it (to see whether we have access) */
          /* Funny thing is, we only make a fileinfo structure for it if
             there is an error (to hold the error number). */
      chars=lines=0;
      if(!(fp=fopen(name,"r"))){
         info=fileinit(name,name,fp);
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
      }
      else if(fstat(fileno(fp),&buf)){
         info=fileinit(name,name,fp);
         info->errnum=errno+Eerrno;
         rcset(errno,Enotready,name);
         /* file is still open, but that's OK since its info is stored */
      }
      else if(!S_ISREG(buf.st_mode)){
         /* Not a regular file.  Sometimes we are allowed to fopen a directory,
            in which case EISDIR should be reported.  Otherwise, since we
            were allowed to open the file, assume it is a readable file with
            no characters (e.g. a tty) and do not report an error. */
         if(S_ISDIR(buf.st_mode)){
            fclose(fp);
            info=fileinit(name,cnull,(FILE *)0);
            info->errnum=EISDIR+Eerrno;
            rcset(EISDIR,Enotready,name);
         }
         else fclose(fp);
      }
      else{
         chars=buf.st_size;
         if(line){    /* Count lines */
            c2='\n';
            while((ch=getc(fp))!=EOF){
               if(ch=='\n')lines++;
               c2=ch;
            }
            if(c2!='\n')lines++;
         }
         else lines=(chars>0);
         fclose(fp);
      }
   }
   if(line)stackint(lines);
   else stackint((int)chars); /* Ahem! */
}

void rxclose(argc)
int argc;
{
   char *name;
   int len;
   if(argc!=1)die(Ecall);
   name=delete(&len);
   if(memchr(name,0,len))die(Ecall);
   else name[len]=0;
   if(!len)die(Ecall);
   stackint(fileclose(name));
}

void rxpclose(argc)
int argc;
{
   char *name;
   int len;
   int rc;
   char *ptr;
   struct fileinfo *info;
   if(argc!=1)die(Ecall);
   name=delete(&len);
   if(memchr(name,0,len))die(Ecall);
   else name[len]=0;
   if(!len)die(Ecall);
   ptr=hashsearch(1,name,&len);
   if(len&&(info=(struct fileinfo *)(((hashent *)ptr)->value))){
      if(info->fp)rc=pclose(info->fp);
      else rc=-1;
      if(info->fp && rc<0)fclose(info->fp); /* if error, close anyway */
      free((char*)info);
      ((hashent *)ptr)->value=0;
   }
   else rc=0;
   if(rc==-1)stack("-1",2);
   else stackint((char)(rc/256));
}
   
void rxfileno(argc)
int argc;
{
   char *name;
   int len;
   struct fileinfo *info;
   if(argc!=1)die(Ecall);
   name=delete(&len);
   if(memchr(name,0,len))die(Ecall);
   else name[len]=0;
   if(!len)die(Ecall);
   if(!(info=(struct fileinfo *)hashget(1,name,&len)) || !(info->fp))
      stack("-1",2);
   else stackint(fileno(info->fp));
}

void rxftell(argc)
int argc;
{
   char *name;
   int len;
   struct fileinfo *info;
   if(argc!=1)die(Ecall);
   name=delete(&len);
   if(memchr(name,0,len))die(Ecall);
   else name[len]=0;
   if(!len)die(Ecall);
   if(!(info=(struct fileinfo *)hashget(1,name,&len)) || !(info->fp))len=-1;
   else len=ftell(info->fp); /* Ahem! */
   if(len>=0)len++;
   stackint(len);
}

void rxquery2(stream,info,param,len) /* used for stream(file,"c","query ...") */
char *stream;
struct fileinfo *info;
char *param;
int len;
{
   struct stat st;
   struct tm *tp;
   char *name;
   char *cp;
   char *dir;
   static char tmp[MAXPATHLEN];
   static char curdir[MAXPATHLEN];
   int statrc;
   int fd=-1;
   /* if the stream is open, fstat it, otherwise stat the named file */
   if (info && info->fp) {
      fd=fileno(info->fp);
      statrc=fstat(fd,&st);
   }
   else statrc=stat(stream,&st);
   if (statrc) { /* answer is "" if the file does not exist */
      stack(cnull,0);
      return;
   }
   tp=localtime(&st.st_mtime);
   param[len]=0;
   if (!strcasecmp(param,"datetime")) {
      sprintf(tmp,"%02d-%02d-%02d %02d:%02d:%02d",
         tp->tm_mon+1,tp->tm_mday,tp->tm_year%100,
         tp->tm_hour,tp->tm_min,tp->tm_sec);
      stack(tmp,strlen(tmp));
      return;
   }
   if (!strcasecmp(param,"exists")) {
      if (fd>=0) { /* stream is open; fetch the associated file name */
         name=(char*)(info+1);
         if (!name[0]) { /* no name known so return the stream name */
            stack(stream,strlen(stream));
            return;
         }
         if (stat(name,&st)) { 
            /* name was known but the file does not seem to exist */
            stack(stream,strlen(stream));
            return;
         }
      }
      else name=stream; /* use the supplied name */
      /* since the stat worked the file exists so qualify and return it */
      /* (files of form "/foo" don't need qualification) */
      if (getcwd(curdir,sizeof curdir) && curdir[0]=='/' && 
          (cp=strrchr(name,'/')) != name) {
         dir=curdir;
         if (cp && cp-name<sizeof tmp) {
            memcpy(tmp,name,cp-name);
            tmp[cp-name]=0;
            if (!chdir(tmp) && getcwd(tmp,sizeof tmp) && tmp[0]=='/') {
               name=cp+1;
               dir=tmp;
            }
            chdir(curdir);
         }
         /* the answer is now dir concatenated to name */
         /* In case dir was not found or name is just '.', remove leading '.' */
         if (name[0]=='.') {
            if (name[1]=='/') name+=2;
            else if (!name[1]) name++;
         }
         if (strlen(name)+strlen(dir)+1 < sizeof tmp) {
            strcat(dir,"/");
            strcat(dir,name);
            name=dir;
         }
      }
      stack(name,strlen(name));
      return;
   }
   if (!strcasecmp(param,"handle")) {
      if (fd<0) stack(cnull,0);
      else stackint(fd);
      return;
   }
   if (!strcasecmp(param,"size")) {
      if (S_ISREG(st.st_mode)) stackint(st.st_size);
      else stack("0",1);
      return;
   }
   if (!strcasecmp(param,"streamtype")) {
      if (fd<0) stack("UNKNOWN",7);
      else if (info->persist) stack("PERSISTENT",10);
      else stack("TRANSIENT",9);
      return;
   }
   if (!strcasecmp(param,"timestamp")) {
      sprintf(tmp,"%04d-%02d-%02d %02d:%02d:%02d",
         tp->tm_year+1900,tp->tm_mon+1,tp->tm_mday,
         tp->tm_hour,tp->tm_min,tp->tm_sec);
      stack(tmp,strlen(tmp));
      return;
   }
   die(Ecall);
}

void rxstream(argc)
int argc;
{
   char *stream;
   char option='S';
   char *command=0;
   char *param;
   int comlen;
   int len;
   int isnull=0;
   int exist;
   char *answer;
   struct fileinfo *info;
   if(argc==3){
      command=delete(&comlen);
      argc--;
      if(comlen<=0)die(Ecall);
   }
   if(argc==2){
      stream=delete(&len);
      argc--;
      if(len==0)die(Ecall);
      if(len>0)option=stream[0]&0xdf;
   }
   if(argc!=1)die(Ecall);
   stream=delete(&len);
   if(len<0)die(Ecall);
   if(len==0){stream="stdin";isnull=1;}
   else {
      if(memchr(stream,0,len))die(Ecall);
      stream[len]=0;
   }
   info=(struct fileinfo *)hashget(1,stream,&exist);
   switch(option){
      case 'D': if(command)die(Ecall);
         if(!info)answer="Stream is not open";
         else if(!info->errnum)answer="Ready";
         else answer=message(info->errnum);
         stack(answer,strlen(answer));
         return;
      case 'S': if(command)die(Ecall);
         if(!info)stack("UNKNOWN",7);
         else if(!info->errnum)stack("READY",5);
         else if(info->errnum==Eeof+Eerrno || info->errnum<Eerrno)
            stack("NOTREADY",8);
         else stack("ERROR",5);
         return;
      case 'C': break; /* out of the switch to do the work */
      default: die(Ecall);
   }
   if(!command)die(Ecall);
   param=command;
   while(comlen--&& *param++!=' ');    /* Find the command end */
   if(comlen>=0){
      param[-1]=0;                     /* terminate the command */
      while(comlen--&& *param++==' '); /* Find the parameter */
      comlen++,param--;
   }
   else param[0]=comlen=0;
   /***/if(!strcasecmp(command,"close")){ /* syntax: "close" */
      if(comlen)die(Ecall);
      stackint(fileclose(stream));
   }
   else if(!strcasecmp(command,"fdopen")){/* syntax: "fdopen [mode][,n]" */
      char *n;
      if (isnull) die(Ecall);
      for(len=0;len<comlen&&param[len]!=','&&param[len]!=' ';len++);
      comlen-=len+1;
      for(n=param+len+1;comlen>0&&n[0]==' ';n++,comlen--);
      if(comlen<0)comlen=0;
      rxfdopen2(stream,param,len,n,comlen);
   }
   else if(!strcasecmp(command,"fileno")){/* syntax: "fileno" */
      if(info && info->fp)stackint(fileno(info->fp));
      else stack("-1",2);
   }
   else if(!strcasecmp(command,"flush")){ /* syntax: "flush" */
      if (isnull) die(Ecall);
      if(info && info->fp) {
         int answer=fflush(info->fp);
         if (answer<0) rxseterr(info);
         if(info->errnum || setrcflag)rcset(info->errnum-Eerrno,Enotready,stream);
         stackint(answer);
      }
      else stack("-1",2);
   }
   else if(!strcasecmp(command,"ftell")){ /* syntax: "ftell" */
      if(info && info->fp)stackint(ftell(info->fp));
      else stack("-1",2);
   }
   else if(!strcasecmp(command,"open")){  /* syntax: "open [mode][,path]" */
      char *path=0;
      if (isnull) die(Ecall);
      /* for compatibility, accept "open both *", "open write append" and */
      /* "open write replace" before parsing the usual parameters. */
      if (comlen==12 && !strncasecmp(param,"write append",comlen)) {
         param="a";
         len=1;
         comlen=0;
      }
      else if (comlen==13 && !strncasecmp(param,"write replace",comlen)) {
         param="w";
         len=1;
         comlen=0;
      }
      else if (comlen>4 && !strncasecmp(param,"both",5)){
         if (comlen==4) {
            param="a";
         }
         else if (comlen==11 && !strncasecmp(param+4," append",7)) {
            param="a";
         }
         else if (comlen==12 && !strncasecmp(param+4," replace",8)) {
            param="w";
         }
         else die(Ecall);
         len=1;
         comlen=0;
      }
      else {
         for(len=0;len<comlen&&param[len]!=','&&param[len]!=' ';len++);
         comlen-=len+1;
         for(path=param+len+1;comlen>0&&path[0]==' ';path++,comlen--);
         if(comlen<0)comlen=0;
      }
      rxopen2(stream,param,len,path,comlen);
   }
   else if(!strcasecmp(command,"pclose")){/* syntax: "pclose" */
      char *ptr=hashsearch(1,stream,&exist);
      int rc;
      if(exist&&(info=(struct fileinfo *)(((hashent *)ptr)->value))){
         if(info->fp)rc=pclose(info->fp);
         else rc=-1;
         if(info->fp && rc<0)fclose(info->fp); /* if error, close anyway */
         free((char*)info);
         ((hashent *)ptr)->value=0;
      }
      else rc=0;
      if(rc==-1)stack("-1",2);
      else stackint((char)(rc/256));
   }
   else if(!strcasecmp(command,"popen")){ /* syntax: "popen [mode][,command]"*/
      char *cmd;
      if (isnull) die(Ecall);
      for(len=0;len<comlen&&param[len]!=','&&param[len]!=' ';len++);
      comlen-=len+1;
      for(cmd=param+len+1;comlen>0&&cmd[0]==' ';cmd++,comlen--);
      if(comlen<0)comlen=0;
      rxpopen2(stream,param,len,cmd,comlen);
   }
   else if(!strcasecmp(command,"query")){ /* syntax: "query <info>" */
      rxquery2(stream,info,param,comlen);
   }
   else if (!strcasecmp(command,"persistent")){ /* syntax: persistent */
      if (info) {
         info->persist=1;
         stack("0",1);
      }
      else stack("-1",2);
   }
   else if (!strcasecmp(command,"transient")) {/* syntax: transient */
      if (info) {
         info->persist=0;
         stack("0",1);
      }
      else stack("-1",2);
   }
   else die(Ecall);
}

void rxcondition(argc)
int argc;
{
   char option='I';
   char *arg;
   int len;
   int which=sgstack[interplev].which;
   if(argc>1)die(Ecall);
   if(argc){
      arg=delete(&len);
      if(len<=0)die(Ecall);
      option=arg[0]&0xdf;
   }
   switch(option){
      case 'I': arg=sgstack[interplev].type==1?"SIGNAL":"CALL";  break;
      case 'C': arg=conditions[which];                           break;
      case 'D': for(len=interplev;len>=0 && !(arg=sgstack[len].data);len--);
                                                                 break;
      case 'S': arg=sgstack[interplev].delay  &(1<<which)? "DELAY":
                    sgstack[interplev].callon &(1<<which)? "ON":
                    sgstack[interplev].bits   &(1<<which)? "ON":
                    "OFF";                                       break;
      default: die(Ecall);
   }
   if(!sgstack[interplev].type)arg=0;
   if(!arg)stack("",0);
   else stack(arg,strlen(arg));
}

static char *getstring() { /* unstack a string, check and nul-terminate it */
   char *ans;
   int len;
   ans=delete(&len);
   if (len<=1) die(Ecall);
   ans[len]=0;
   while (len--) if (!ans[len]) die(Ecall);
   return ans;
}

#define INCL_RXFUNC
#include "rexxsaa.h"
void rxfuncadd(argc)
int argc;
{
   char *entry;
   char *dll;
   char *func;
   int ans;
   int i;
   char C,c;
   if (argc!=3) die(Ecall);
   entry=getstring();
   dll=getstring();
   func=getstring();
   ans=RexxRegisterFunctionDll(func,dll,entry);
   if (ans) {
      stackint(ans);
      return;
   }
   /* Also register the uppercase of the function */
   for(i=0;(c=func[i]);i++) {
      C=uc(c);
      if (c!=C) {ans=1; func[i]=C;}
   }
   if (ans) ans=RexxRegisterFunctionDll(func,dll,entry);
   stackint(ans);
}
      
void rxfuncdrop(argc)
int argc;
{
   char *func;
   int i;
   int ans=0;
   char c,C;
   int doupper=0;
   if (argc!=1) die(Ecall);
   func=getstring();
   ans=RexxDeregisterFunction(func);
   /* also drop the uppercase of the function */
   for(i=0;(c=func[i]);i++) {
      C=uc(c);
      if (c!=C) {doupper=1; func[i]=C;}
   }
   if (doupper) ans=ans && RexxDeregisterFunction(func);
   if (ans) stack("1",1);
   else stack("0",1);
}

void rxfuncquery(argc)
int argc;
{
   char *func;
   int i;
   int ans=0;
   char c,C;
   if (argc!=1) die(Ecall);
   func=getstring();
   if (RexxQueryFunction(func)) {
      /* Also query the uppercase of the function */
      for(i=0;(c=func[i]);i++) {
         C=uc(c);
         if (c!=C) {ans=1; func[i]=C;}
      }
      if (ans) ans=RexxQueryFunction(func);
      else ans=1;
   }
   if (ans) stack("1",1);
   else stack("0",1);
}
