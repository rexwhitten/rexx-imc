/* The Utility functions of REXX/imc       (C) Ian Collier 1992 */

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<errno.h>
#include<memory.h>
#include<string.h>
#include<setjmp.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/file.h>
#include<sys/param.h>
#include"const.h"
#include"globals.h"
#include"functions.h"
#define INCL_REXXSAA
#include "rexxsaa.h"
#ifdef Solaris
# include<sys/uio.h>
# include<sys/filio.h>
# include<sys/fcntl.h>
#endif

char *words[]= /* Keywords in order of their values */
       {"SAY", "SAYN", "DO", "END", "IF", "ELSE", "SELECT", "WHEN",
        "OPTIONS", "PARSE", "PUSH", "QUEUE", "EXIT", "RETURN", "CALL",
        "SIGNAL", "ITERATE", "LEAVE", "INTERPRET", "TRACE", "OTHERWISE",
        "NOP", "PROCEDURE", "ADDRESS", "NUMERIC", "DROP", "THEN", "PULL",
        "ARG", "SOURCE", "VAR", "VERSION", "LINEIN", "VALUE", "WITH",
        "UPPER", "TO", "BY", "FOR", "FOREVER", "WHILE", "UNTIL", "ON", "OFF",
        "DIGITS", "FUZZ", "FORM", "EXPOSE", "HIDE", "NAME"};

char *message(rc)  /* Return errortext(rc) */
int rc;
{
   char *sys_err;
   if (rc== -3 && fname[0]!=0) perror(fname);
   switch (rc){
      case -3:        return "Error loading program";
      case Einit:     return "Initialisation error";
      case Ehalt:     return "Program interrupted";
      case Emem:      return "Machine storage exhausted";
      case Equote:    return "Unmatched \'/*\' or quote";
      case Enowhen:   return "Expected WHEN/OTHERWISE";
      case Ethen:     return "Unexpected THEN/ELSE";
      case Ewhen:     return "Unexpected WHEN/OTHERWISE";
      case Eend:      return "Unexpected or unmatched END";
      case Echar:     return "Invalid character in program";
      case Enoend:    return "Incomplete DO/SELECT/IF";
      case Ehex:      return "Invalid binary or hexadecimal string";
      case Elabel:    return "Label not found";
      case Eprocedure:return "Unexpected PROCEDURE";
      case Enothen:   return "Expected THEN";
      case Enostring: return "String or symbol expected";
      case Enosymbol: return "Symbol expected";
      case Exend:     return "Invalid data on end of clause";
      case Etrace:    return "Invalid TRACE request";
      case Etrap:     return "Invalid subkeyword found";
      case Erange:    return "Invalid whole number";
      case Exdo:      return "Invalid DO syntax";
      case Eleave:    return "Invalid LEAVE or ITERATE";
      case Elong:     return "Symbol > 250 characters";
      case Ename:     return "Name starts with number or \'.\'";
      case Ebadexpr:  return "Invalid expression";
      case Elpar:     return "Unmatched \'(\'";
      case Ecomma:    return "Unexpected \',\' or \')\'";
      case Eparse:    return "Invalid template";
      case Eopstk:    return "Evaluation stack overflow (> 30 pending operations)";
      case Ecall:     return "Incorrect call to routine";
      case Enum:      return "Bad arithmetic conversion";
      case Eoflow:    return "Arithmetic overflow or underflow";
      case Eundef:    return "Routine not found";
      case Enoresult: return "Function did not return data";
      case Ereturn:   return "No data specified on function RETURN";
      case Exlabel:   return "Unexpected label";
      case Esys:      return "Failure in system service";
      case Elost:     return "Implementation error";
      case Eincalled: return "Error in called routine";
      case Enovalue:  return "No-value error";
      case Eexist:    return "Use of an un-implemented feature!";
      case Esyntax:   return "Syntax error";
      case Elabeldot: return "Label ends with \'.\'";
      case Ercomm:    return "Unexpected \'*/\'";
      case Emanyargs: return "Too many arguments (> 30)";
      case Eerror:    return "ERROR condition occurred";
      case Efailure:  return "FAILURE condition occurred";
      case Eerrno:    return "Unknown error occurred during I/O";
      case Ebounds:   return "File position was out of bounds";
      case Eseek:     return "Reposition attempted on transient stream";
      case Eaccess:   return "Write attempted on a read-only stream";
      case Eread:     return "Read attempted on a write-only stream";
      case Eeof+Eerrno:return"End of file";
   }
   if(rc>Eerrno && (sys_err=strerror(rc-Eerrno))) return sys_err;
   return "";
}

void rcset(rc,type,desc)/* set rc on return from system call */
int rc;                 /* What to set rc to */
int type;               /* What error to die with if the error is trapped */
char *desc;             /* Description for condition(d) */
{
   char rcbuf[20];        /* just turn rc into a string and call the */
   sprintf(rcbuf,"%d",rc);/* next function... */
   rcstringset(rc,rcbuf,strlen(rcbuf),type,desc);
}

void rcstringset(rc,rcval,rclen,type,desc)/* set rc on return from system call */
int rc;                 /* numeric value of rc if appropriate */
char *rcval;            /* Pointer to what to set rc to */
int rclen;              /* Length of what to set rc to */
int type;               /* What error to die with if the error is trapped */
char *desc;             /* Description for condition(d) */
{
   int bit=type==Eerror?Ierror:type==Efailure?Ifailure:Inotready;
   int catch=rc&&(sgstack[interplev].bits&(1<<bit));
   int call=rc&&(sgstack[interplev].callon&(1<<bit));
   if(interact>=0 && interact+1==interplev)
      return;           /* no action for interactive commands */
   if(rc && call==0 && catch==0 && (type==Efailure||type==Enotready&&setrcflag))
      type=Eerror,
      bit=Ierror,
      catch=sgstack[interplev].bits&(1<<bit),
      call=sgstack[interplev].callon&(1<<bit);
   if(type!=Enotready || setrcflag) /* set rc after a command */
      varset("RC",2,rcval,rclen);
   if(rc && type==Enotready)lasterror=rc; /* save an I/O error */
   if(call||catch){
      if(sigdata[bit])free(sigdata[bit]);
      strcpy(sigdata[bit]=allocm(1+strlen(desc)),desc);
   }
   if(call)delayed[bit]=1;
   else if(catch)die(type);
}

void printrc(i) /* Print a trace line showing the return code */
int i;
{
   tracestr("      +++ RC=");
   tracenum(i,1);
   tracestr(" +++\n");
}

/* An exit handling routine */
int exitcall(main,sub,parm)
long main;
long sub;
PEXIT parm;
{   /* very simple. */
   long exrc=exitlist[main](main,sub,parm);
   if(exrc==RXEXIT_RAISE_ERROR)die(Esys);
   return exrc;
}

/* Variable handling routines */
/* The following routines are low-level and serve to abstract from the
   variables' actual representation. As long as the following routines
   are correct, the representation may be changed without affecting
   the rest of the program. */

/* These routines maintain a multiple-level variable table, containing
   names and values of variables. The names of simple symbols and stems
   are kept in a binary tree arrangement, in the format of a varent
   structure followed by a name (padded to a multiple of 4 bytes) and a
   value. Symbols which have been DROPped still exist, but have a value
   length of -1. Symbols which are copies of variables in earlier levels
   have a negative "valalloc" value indicating the level number (starting
   at -1, which means level 0).
   Stems have no trailing dot, but have bit 7 of the first character
   inverted, and the value of a stem is a structure containing a default
   value (an allocated,length,value triple) followed by a binary
   tree of tails associated with values as in the main table
   The binary tree structure should allow access in O(log n) time, except
   when the value pointers need to be updated (when lengthening or
   shortening a value). However no fancy balancing tricks are used, so
   O(n) time is possible in the worst case. A special order is imposed
   which should minimise the possibility of a very unbalanced tree. In
   particular, assigning the letters a-i or the numbers 0-9 in order
   should produce an optimal depth tree (whereas with the usual ordering
   a linear depth tree results).
   The less and grtr fields contain offsets from the start of the level, and
   the next field contains the length of one variable entry. When a variable
   is lengthened or shortened, its own next field is updated, and the less
   and grtr fields of all variables in the same level are updated. All other
   pointers, except for the pointers to each level, remain the same.
*/
int less(s1,s2,n1,n2)/* the ordering - compare s1,len n1 with s2,len n2*/
char *s1,*s2;        /* return -ve (s1<s2), 0 (s1=s2) or +ve (s1>s2). */
int n1,n2;
{
   static char xlate[]={4,7,3,11,1,5,9,13,0,2,6,8,10,12,15,14};
           /* the translation table for ordering */
   char x,y;
   int r;
   if(n1!=n2)return n1-n2;           /* Order on lengths first (it's faster) */
   if(!n1)return 0;                  /* "" == "" */
   while(n1--&&s1++[0]==s2++[0]);    /* find first non-match character */
   x=s1[-1],y=s2[-1];
   r=(x&0xf0)-(y&0xf0);              /* compare last characters */
   if (r) return r;
   return xlate[x&0xf]-xlate[y&0xf]; /* use translation for last 4 bits */
}

char *varsearch(name,len,level,exist)
char *name;
int len;
int *level;
int *exist;
/* search for name `name' of length `len' in the variable table for `level'.
   The answer is the address of the entry which matches, with `exist'
   non-zero, or, if the name does not exist, exist=0 and the answer
   is the address of the slot where the new branch of the tree is to
   be added. If there are no names in the table, 0 is returned.
   On exit, level contains the level number where the variable was actually
   found, which may be different from the given level due to exposure */
{
   char *data=varstk[*level]+vartab;
   char *ans=data;
   int *slot;
   int c;
   *exist=0;
   if(varstk[*level]==varstk[*level+1])return cnull;
   while((c=less(name,ans+sizeof(varent),len,((varent *)ans)->namelen))
     &&  (*(slot= &(((varent *)ans)->less)+(c>0)))>=0)
      ans=data+*slot;   /* Go down the tree */
   if(!c){              /* Equality resulted from the compare */
      *exist=1;
      if((c=((varent *)ans)->valalloc)<0){  /* An exposed variable */
         *level= -(c+1);
         return varsearch(name,len,level,exist);
      }
      else return ans;
   }
   return (char *)slot;
}

char *tailsearch(stem,name,len,exist)/* like varsearch, but searches for the */
char *stem,*name;                    /* tail of a compound variable.         */
int len,*exist;
{
   char *data=stem+sizeof(varent)+align(((varent *)stem)->namelen);
   char *tails=data+2*four+*(int *)data; /* start of tail information */
   char *ans=tails;
   int *slot;
   int c;
   *exist=0;
   if(((varent *)stem)->vallen==tails-data)return cnull;
   while((c=less(name,ans+sizeof(varent),len,((varent *)ans)->namelen))
     &&  (*(slot= &(((varent *)ans)->less)+(c>0)))>=0)
      ans=tails+*slot;
   if(c)return (char* )slot;
   *exist=1;
   return ans;
}

char *valuesearch(name,namelen,level,exist,stem) /* search for any variable */
char *name;              /* if a simple symbol, the result is like varsearch*/
int namelen;             /* and stem=0. If a compound symbol, level ends up */
int *level,*exist;       /* with the level containing the whole symbol and  */
char **stem;             /* stem points to the stem containing it. exist is */
                         /* non-zero if the whole symbol was found; stem is */
                         /* non-zero if a stem was found, even if it does   */
                         /* not contain the required tail. The return value */
                         /* is the variable entry (if exist), or a slot in  */
                         /* which to put the new tail (if stem && !exist),  */
                         /* or a slot in which to put the new stem (if      */
                         /* !stem). The answer is zero if there are no      */
                         /* entries in the stem (if stem) or if there are no*/
                         /* entries in the vartable (if !stem).             */
                         /* If the variable name is an existing stem, 0 is  */
{                        /* returned with exist=0 and stem pointing to it   */
   char *ans;
   char *tail;
   int stemlen;
   int taillen;
   register int l;
   if(!(name[0]&128))   /* if a simple symbol, the result is like varsearch */
      return *stem=0,varsearch(name,namelen,level,exist);
   stemlen=(tail=memchr(name,'.',namelen))-name;
   if(!tail)stemlen=namelen,taillen=0;
   else tail++,taillen=namelen-stemlen-1;
   while(1){
      if(!(*stem=varsearch(name,stemlen,level,exist))) return 0; /* no vars */
      if(!*exist) return ans= *stem,*stem=0,ans;                 /* no stem */
      if(!tail) return (*exist=0),cnull;                  /* name is a stem */
      if(!(ans=tailsearch(*stem,tail,taillen,exist)))return 0;  /* no tails */
      if(!*exist)return ans;                                    /* no tail  */
      if((l=((varent *)ans)->valalloc)>=0)return ans;          /* it's here */
      *level=-(l+1);                                      /* it's elsewhere */
   }
}

void printtree(lev) /* for testing */
int lev;
{
   varent *v;
   char *c,*d;
   int level=lev;
   int i;
   if(level<0||level>varstkptr)level=varstkptr; /* guard against parameterless
                                                   call */
   v=(varent *)(vartab+varstk[level]),c=vartab+varstk[level+1];
   while((char *)v<c){
      printf("Offset:%d\n",((char *)v)-vartab-varstk[level]),
      printf("   next=%d\n",v->next),
      printf("   less=%d\n",v->less),
      printf("   grtr=%d\n",v->grtr),
      printf("   namelen=%d\n",v->namelen),
      printf("   valalloc=%d\n",v->valalloc),
      printf("   vallen=%d\n",v->vallen),
      printf("   name="),
      i=v->namelen,
      d=sizeof(varent)+(char *)v;
      while(i-->0)putchar(d++[0]&127);
      putchar('\n');
      v=(varent *)(v->next+(char *)v);
   }
}

void printtails(stem) /* for testing */
varent *stem;
{
   varent *v;
   char *c,*d,*e;
   int i;
   c=(char *)(stem+1)+align(stem->namelen);
   printf("Default value alloc %d len %d value %s\n",*(int*)c,*((int *)c+1),
      *((int *)c+1)<0?"":c+2*four);
   d=c+*(int *)c+2*four,
   v=(varent *)d,c+=stem->vallen;
   while((char *)v<c){
      printf("Offset:%d\n",((char *)v)-d),
      printf("   next=%d\n",v->next),
      printf("   less=%d\n",v->less),
      printf("   grtr=%d\n",v->grtr),
      printf("   namelen=%d\n",v->namelen),
      printf("   valalloc=%d\n",v->valalloc),
      printf("   vallen=%d\n",v->vallen),
      printf("   name="),
      i=v->namelen,
      e=sizeof(varent)+(char *)v;
      while(i-->0)putchar(e++[0]&127);
      putchar('\n');
      v=(varent *)(v->next+(char *)v);
   }
}

void update(value,amount,level)
int value,amount,level;
{ /* update all the less/grtr fields of level `level' by `amount' if greater
     than `value'; adjust the level pointers also. This routine is called
     *after* the space has been created or reclaimed. */
   register varent *ptr;
   int l=level;
   while(l++<=varstkptr)varstk[l]+=amount;
   for(  ptr=(varent *)(vartab+varstk[level]);
         (char *)ptr<vartab+varstk[level+1];
         ptr=(varent *)((char *)ptr+ptr->next))
   {
      if(ptr->less>value)ptr->less+=amount;
      if(ptr->grtr>value)ptr->grtr+=amount;
   }
}

long makeroom(var,amount,level) /* var points to a (complete) variable entry */
int var,amount,level;           /* which is to be enlarged by amount. var is */
{                               /* an integer offset from the start of level */
   register char *i;            /* the return is the difference from dtest   */
   register char *j;
   varent *k;
   char *mtest_old;
   long mtest_diff;
   if(!dtest(vartab,vartablen,varstk[varstkptr+1]+amount+2,amount+512))
      mtest_diff=0;
   k=((varent *)(j=vartab+varstk[level]+var));  /* the variable's address */
   j+=(k->next);                                /* the end of the variable */
   for(i=vartab+varstk[varstkptr+1]-1;i>=j;i--)i[amount]=i[0]; /* make room */
   k->next+=amount;
   update(var,amount,level);
   return mtest_diff;
}

void reclaim(var,amount,level)  /* var points to a (complete) variable entry */
int var,amount,level;           /* which is to be reduced by amount. var is  */
{                               /* an integer offset from the start of level */
   register char *i;
   register char *j=vartab+varstk[varstkptr+1]-amount;
   varent *k=(varent *)(vartab+varstk[level]+var);
   for(i=(char *)k+(k->next)-amount;i<j;i++)i[0]=i[amount];
   k->next-=amount;
   update(var,-amount,level);
}

void tailupdate(stem,value,amount)
varent *stem;     /* update all the grtr/less fields of the variable pointed */
int value,amount; /* to by stem by amount if greater than value. Updates the */
{                 /* vallen field of the stem also.                          */
   register varent *ptr;
   int len;
   char *data=(char *)stem+sizeof(varent)+align(stem->namelen);
   len=(stem->vallen+=amount);
   for(  ptr=(varent *)(data+*(int *)data+2*four);
         (char *)ptr<data+len;
         ptr=(varent *)((char *)ptr+ptr->next))
   {
      if(ptr->less>value)ptr->less+=amount;
      if(ptr->grtr>value)ptr->grtr+=amount;
   }
}

long tailroom(stem,var,amount,level) /* make room in the tail of a stem */
varent *stem;       /* var is a tail offset value, or -1 meaning the default */
int var,amount,level;
{
   register char *i;
   register char *j;
   varent *k;
   char *data;
   char *def;
   long diff=0;
   int ext;
   if(stem->vallen+amount>stem->valalloc)  /* Not enough space allocated */
      ext=align(stem->vallen/3+amount*4/3),
      diff=makeroom((char *)stem-vartab-varstk[level],ext,level),
      stem=(varent *)((char *)stem+diff),
      stem->valalloc+=ext;                 /* It is now!                 */
   def=data=(char *)stem+sizeof(varent)+align(stem->namelen);
   data+=*(int *)data+2*four;
   if(var>=0)k=(varent *)(j=data+var),  /* find the tail, and its end */
             j+=(k->next);
   else k=(varent *)(j=data);           /* or use the default value */
   for(i=def+stem->vallen-1;i>=j;i--)i[amount]=i[0];
   if(var>=0)k->next+=amount,
             tailupdate(stem,var,amount);
   else *(int *)def +=amount;
   return diff;
}

void tailreclaim(stem,var,amount) /* Reduce the size of a tail element */
int var,amount;                   /* var is a tail offset value */
varent *stem;
{
   register char *i;
   register char *j;
   varent *k;
   char *data;
   data=(char *)stem+sizeof(varent)+align(stem->namelen);
   j=data+stem->vallen-amount;
   data+=*(int *)data+2*four;
   k=(varent *)(data+var);
   for(i=(char *)k+(k->next)-amount;i<j;i++)i[0]=i[amount];
   k->next-=amount;
   tailupdate(stem,var,-amount);
}

void tailhookup(stem)   /* hook up the tree structure within a stem */
varent *stem;           /* i.e. fill in the grtr & less fields in a list */
{                       /* of tail elements */
   int *slot;
   int exist;
   register char *k;
   char *data=(char *)(stem+1)+align(stem->namelen);/*address of stem's value*/
   char *tails=data+*(int *)data+2*four;            /* address of first tail */
   char *end=data+stem->vallen;                     /* end of last tail */
   for(k=tails;k<end;k+=((varent *)k)->next){
      if(k==tails)continue;
      slot=(int *)tailsearch/* should always tell where to hook the new tail */
         ((char*)stem,k+sizeof(varent),((varent *)k)->namelen,&exist);
      if(!exist) /* should always be true! */ slot[0]=k-tails;
   }
}

void varcreate(varptr,name,value,namelen,len,lev)
char *varptr,*name,*value;         /* create a new variable (used in varset */
int namelen,len,lev;               /* and varcopy) with given value.        */
                                   /* varptr is the result of a failed      */
                                   /* search, i.e. if non-null points to an */
                                   /* integer slot to store the address.    */
                                   /* if lev=0, place in the top level. If  */
                                   /* lev=1, place one level down.          */
{
   int alloc=len/4;
   int ext;
   register char *i;
   register char *v;
   long mtest_diff;
   char *mtest_old;
   if(alloc<20)alloc=20;  /* The extra amount of space to allocate */
   alloc+=len,
   alloc=align(alloc);    /* The total amount of space to allocate */
   if dtest(vartab,
            vartablen,
            varstk[varstkptr+1]+1+(ext=align(alloc+namelen+sizeof(varent))),
            namelen+alloc+256)
      if(varptr)varptr+=mtest_diff;
   v=vartab+varstk[varstkptr+!lev];      /* where to put the new variable */
   if(lev)  /* move up the entire top level to make room */
      for(i=vartab+varstk[varstkptr+1];i>=v;i--)i[ext]=i[0];
   memcpy(v+sizeof(varent),name,namelen),/* copy the variable's name  */
   ((varent *)v)->next=ext,              /* now fill in the fields... */
   ((varent *)v)->less= -1,
   ((varent *)v)->grtr= -1,
   ((varent *)v)->namelen=namelen,
   ((varent *)v)->valalloc=alloc,
   ((varent *)v)->vallen=len;
   if(varptr)             /* make the new variable a part of the tree */
      *(int *)varptr=varstk[varstkptr+!lev]-varstk[varstkptr-lev];
   if(len>0)                       /* copy the new variable's value */
      memcpy(v+sizeof(varent)+align(namelen),value,len);
   varstk[varstkptr+1]+=ext;       /* and finally update the level pointers */
   if(lev)varstk[varstkptr]+=ext;
}

void stemcreate(varptr,name,value,namelen,len,lev)
char *varptr,*name,*value;         /* similar to varcreate(), but a whole   */
int namelen,len,lev;               /* stem is created with the given default*/
                                   /* name does not include the dot */
{
   int alloc=align(len*5/4+256);
   int ext;
   register char *i,*v;
   long mtest_diff;
   char *mtest_old;
   if dtest(vartab,
        vartablen,
        varstk[varstkptr+1]+1+(ext=align(alloc+namelen+sizeof(varent)+2*four)),
        namelen+alloc+256)
      if(varptr)varptr+=mtest_diff;
   v=vartab+varstk[varstkptr+!lev];
   if(lev)for(i=vartab+varstk[varstkptr+1];i>=v;i--)i[ext]=i[0];
   memcpy(v+sizeof(varent),name,namelen);
   if(varptr) *(int *)varptr=varstk[varstkptr+!lev]-varstk[varstkptr-lev];
   ((varent *)v)->next=ext,
   ((varent *)v)->less= -1,
   ((varent *)v)->grtr= -1,
   ((varent *)v)->namelen=namelen,
   ((varent *)v)->valalloc=alloc,
   ((varent *)v)->vallen=(alloc=align(len))+2*four;
   v+=sizeof(varent)+align(namelen),
   ((int *)v)[0]=alloc,
   ((int *)v)[1]=len;
   if(len>0)memcpy(v+2*four,value,len);
   varstk[varstkptr+1]+=ext;
   if(lev)varstk[varstkptr]+=ext;
}

void tailcreate(stem,tailptr,name,value,namelen,len,level)
char *stem,*tailptr,*name,*value;  /* create new tail within a stem with */
int namelen,len,level;             /* a given value. Stem is the address */
                                   /* of the stem structure, tailptr is  */
                                   /* the equivalent of varptr in earlier*/
                                   /* functions. Level is the actual     */
                                   /* level number. */
{
   long diff;
   int alloc=len/4;
   int ext;
   char *v=stem+sizeof(varent)+align(((varent *)stem)->namelen);
   char *e=v+((varent *)stem)->vallen;  /* end of last tail */
   v+=*(int *)v+2*four;                 /* start of first tail */
   if(len<0)alloc=0;
   else {
      if(alloc<20)alloc=20;
      alloc=align(alloc+len);
   }
   if(   (ext=alloc+align(namelen)+sizeof(varent))
       + ((varent *)stem)->vallen
     >   ((varent *)stem)->valalloc){
      if(diff=makeroom(stem-vartab-varstk[level],ext+256,level)){
         if(tailptr)tailptr+=diff;
         stem+=diff,e+=diff,v+=diff;
      }
      ((varent *)stem)->valalloc+=ext+256;
   }
   if(tailptr)*(int *)(tailptr)=e-v; /* Save the offset in the parent's slot */
   memcpy(e+sizeof(varent),name,namelen), /* Make the new tail at e */
   ((varent *)e)->next=ext,
   ((varent *)e)->less= -1,
   ((varent *)e)->grtr= -1,
   ((varent *)e)->namelen=namelen,
   ((varent *)e)->valalloc=alloc,
   ((varent *)e)->vallen=len;
   if(len>0)memcpy(e+sizeof(varent)+align(namelen),value,len);
   ((varent *)stem)->vallen+=ext;
}

void varset(name,varlen,value,len) /* set variable `name' of namelength   */
char *name,*value;                 /* `varlen' equal to the value `value' */
int len,varlen;                    /* which has length `len'              */
{
   int varalloc,varoff,ext,newlen,exist;
   register char *i;
   register varent *v1,*v2;
   int level=varstkptr;
   char *valptr;
   char *varptr;
   char *oldptr;
   char *stemptr;
   long diff;
   int compound=name[0]&128;
   int isstem=compound&&!memchr(name,'.',varlen);/* stems do not contain dots*/
   char varname[maxvarname];
   if(isstem){ /* Set the default value of a whole stem. */
      varptr=varsearch(name,varlen,&level,&exist);
      if(exist){ /* stem exists. Set default and clear all non-exposed tails */
         valptr=varptr+sizeof(varent)+align(((varent *)varptr)->namelen);
            /* valptr points to the default value */
         if((ext=align(len-*(int *)valptr))>0)/* extra mem needed for default*/
            if(diff=tailroom((varent *)varptr,-1,ext,level))
               varptr+=diff,
               valptr+=diff;
         ((int *)valptr)[1]=len;  /* now copy the default value */
         if(len>0)memcpy(valptr+2*four,value,len);
         ext= *(int *)valptr;
         i=((varent *)varptr)->vallen+valptr; /* the end of the last tail */
         v2=(varent *)(valptr+2*four+ext);    /* the start of the first tail */
         oldptr=valptr;
         valptr+= (*(int *)valptr=align(len))+2*four; /* new start of tails */
         v1=(varent *)valptr;              /* pointer to "current" new tail */
         /* now copy all exposed tails from v2 to v1. upper bound of v2 = i */
         while((char *)v2<i){
            if(v2->valalloc<0)  /* It is exposed */
               memcpy((char*)v1,(char*)v2,v2->next),
               v1->grtr= -1,
               v1->less= -1,
               v1=(varent *)((char *)v1+v1->next);
            v2=(varent *)((char *)v2+v2->next);
         }
         ((varent *)varptr)->vallen=ext=((char *)v1)-oldptr; /* new length */
         ext=align(ext);      /* The amount of space to leave in this stem */
         if(len>=0)ext+=256;  /* Leave some extra space for future tails   */
         if((ext-=((varent *)varptr)->valalloc)<0)      /* Shrink the stem */
            reclaim(varptr-varstk[level]-vartab,-ext,level),
            ((varent *)varptr)->valalloc+=ext;
         /* hook up the tree of tails */
         tailhookup((varent*)varptr);
         /* assign the given string to each remaining tail */
         memcpy(varname,name,varlen); /* varname holds each compund symbol */
         varname[varlen]='.';
         for(v2=(varent *)valptr;v2<v1;v2=(varent *)((char *)v2+v2->next))
            memcpy(varname+varlen+1,(char*)(v2+1),v2->namelen),
            varset(varname,1+varlen+v2->namelen,value,len);
         return;
      }
      /* a stem which does not exist is being initialised */
      if(len>=0)stemcreate(varptr,name,value,varlen,len,0);
      return;
   }
   if(compound){  /* A compound symbol is being assigned to */
      varptr=valuesearch(name,varlen,&level,&exist,&stemptr);
      if(exist){ /* change an existing compound variable */
         valptr=stemptr+sizeof(varent)+align(((varent *)stemptr)->namelen),
         valptr+=*(int *)valptr+2*four;
         varoff=varptr-valptr, /* now varoff contains the offset within stem */
         varalloc= ((varent *)varptr)->valalloc;
         if(len>varalloc){     /* need some more memory */
            ext=len/4;
            if(ext<20)ext=20;
            newlen=align(len+ext), /* the total amount of memory */
            ext=newlen-varalloc;   /* the extra amount */
            varptr+=tailroom((varent*)stemptr,varoff,ext,level);
            ((varent *)varptr)->valalloc=newlen;
         }
         else if(len<0&&varalloc>10)  /* variable is being dropped - reclaim */
            tailreclaim((varent*)stemptr,varoff,varalloc),
            ((varent *)varptr)->valalloc=0;
         if(len>0) /* Copy the value */
            memcpy(varptr+sizeof(varent)+align(((varent *)varptr)->namelen),
                   value,len);
         ((varent *)varptr)->vallen=len; /* and copy the length */
         return;
      }
      if(!stemptr){/* the stem does not exist. Create then continue */
         if(len<0)return; /* Do not bother to DROP from a nonexistent stem */
         stemcreate(varptr,name,cnull,strchr(name,'.')-name,-1,0),
            /* create stem with no default (the above line) */
         level=varstkptr,
         varptr=valuesearch(name,varlen,&level,&exist,&stemptr);
            /* the search is guaranteed to find a stem with no tail */
      }
      /* the stem exists but the tail does not */
      /* Even if the variable is being dropped, it is necessary to create it
         in case of e.g. "a.=5; drop a.1; say a.1" (should say "A.1") */
      oldptr=name,
      varlen-=((name=1+strchr(name,'.'))-oldptr);
      tailcreate(stemptr,varptr,name,value,varlen,len,level);
      return;
   } /* So now it is a simple symbol. */
   varptr=varsearch(name,varlen,&level,&exist);
   if(exist){ /* variable exists, so reset */
      varoff= varptr-vartab-varstk[level],
      varalloc= ((varent *)varptr)->valalloc;
      if(len>varalloc){
         ext=len/4;
         if(ext<20)ext=20;
         newlen=align(len+ext),
         ext=newlen-varalloc;
         varptr+=makeroom(varoff,ext,level);
         ((varent *)varptr)->valalloc=newlen;
      }
      else if(len<0&&varalloc>10)  /* variable is being dropped - reclaim */
         reclaim(varoff,varalloc,level),
         ((varent *)varptr)->valalloc=0;
      if(len>0)
         memcpy(varptr+sizeof(varent)+align(((varent *)varptr)->namelen),
         value,len);
      ((varent *)varptr)->vallen=len;
   }
   else if(len>=0) /* variable does not exist, so create */
      varcreate(varptr,name,value,varlen,len,0);
}

char *varget(name,varlen,len)/* get value and length of variable `name'.     */
char *name;                  /* Value is returned, length is placed in `len' */
int varlen;
int *len;
{
   int level=varstkptr;
   char *stem;
   char *varptr=valuesearch(name,varlen,&level,len,&stem);
   if(!(*len||stem))return 0;    /* does not exist at all */
   if(*len&&stem&&((varent *)varptr)->vallen<0)
      return (*len=0),cnull;     /* compound symbol has "null" value */
   if(!*len){
      /* compound variable doesn't exist; try default value */
      stem+=sizeof(varent)+align(((varent *)stem)->namelen);
      if((*len= *((int *)stem+1))>=0) return stem+2*four;
      else return (*len=0),cnull;
   }
   if((*len= ((varent *)varptr)->vallen)>=0) /* exists */
      return varptr+align(((varent *)varptr)->namelen)+sizeof(varent);
   else return (*len=0),cnull;
}

void newlevel()    /* increment variable level, making a clean environment  */
{
   char *charvarstk=(char *)varstk;
   mtest(charvarstk,varstklen,four*(++varstkptr+2),four*25);
   varstk=(int *)charvarstk;
   varstk[varstkptr+1]=varstk[varstkptr];
}

void varcopy(name,varlen) /* copy a variable (as in procedure expose)       */
int varlen;
char *name;        /* when this procedure is called, varstkptr has already  */
{                  /* been incremented to point to the level in which the new
                      copy of the variable is required. The old copy of the
                      variable will be in level varstkptr-1.                */
   int ext,l;
   register char *i;
   char *oldptr;
   int level=varstkptr-1;
   int compound=name[0]&128;
   int isstem=compound&&!memchr(name,'.',varlen);
   char *varptr;
   char *stemptr;
   char *endvar;
   char *mtest_old;
   long mtest_diff;
   if(compound&&!isstem){ /* An individual compound symbol */
      varptr=valuesearch(name,varlen,&level,&l,&stemptr);
      if(!l){ /* compound variable does not exist, so create before exposing */
         if(!stemptr) /* stem does not exist, so create with no default */
            stemcreate(varptr,name,cnull,strchr(name,'.')-name,-1,1),
            level=varstkptr-1,
            varptr=valuesearch(name,varlen,&level,&l,&stemptr);
         oldptr=1+strchr(name,'.'),
         tailcreate(stemptr,varptr,oldptr,cnull,varlen-(oldptr-name),-1,level);
      }
      /* now copy the variable, which is in level `level' */
      ext=varstkptr;
      varptr=valuesearch(name,varlen,&ext,&l,&stemptr);
      if(!l){/* not already exposed, so go ahead */
         /* make sure there is a stem to hold the new variable */
         if(!stemptr)
            stemcreate(varptr,name,cnull,strchr(name,'.')-name,-1,0),
               /* create stem with no default */
            ext=varstkptr,
            varptr=valuesearch(name,varlen,&ext,&l,&stemptr);
         if(ext==varstkptr&&((varent *)stemptr)->valalloc>=0){
            /* stem is not already exposed, so go ahead */
            oldptr=name,name=1+strchr(name,'.'),varlen-=name-oldptr,
            ext=sizeof(varent)+align(varlen),
            oldptr=vartab;
            if(((varent *)stemptr)->valalloc<((varent *)stemptr)->vallen+ext){
               if(mtest_diff
               =makeroom(stemptr-vartab-varstk[varstkptr],ext+256,varstkptr)){
                  if(varptr)varptr+=mtest_diff;
                  stemptr+=mtest_diff;
               }
               ((varent *)stemptr)->valalloc+=ext+256;
            } /* There is now enough room to place the new tail at the end
              of the stem. */
            i=stemptr+sizeof(varent)+align(((varent *)stemptr)->namelen),
            endvar=i+((varent *)stemptr)->vallen,
            i+= *(int*)i+2*four,
            ((varent *)stemptr)->vallen+=ext;
            if(varptr)*(int *)varptr=endvar-i;
            memcpy(endvar+sizeof(varent),name,varlen),
            ((varent *)endvar)->next=ext,
            ((varent *)endvar)->less= -1,
            ((varent *)endvar)->grtr= -1,
            ((varent *)endvar)->namelen=varlen,
            ((varent *)endvar)->valalloc= -(level+1),
            ((varent *)endvar)->vallen=0;
         }
      }
      return;
   }
   /* stems are like ordinary symbols; both are treated here. */
   varptr=varsearch(name,varlen,&level,&l);
   if(!l) /* create in old level before exposing to new level */
      if(isstem) stemcreate(varptr,name,cnull,varlen,-1,1);
      else        varcreate(varptr,name,cnull,varlen,-1,1);
   ext=varstkptr;
   varptr=varsearch(name,varlen,&ext,&l);
   if(!l){ /* not already exposed, so go ahead */
      if dtest(vartab,vartablen,varstk[varstkptr+1]+1+(ext=sizeof(varent)+align(varlen)),varlen+256)
         if(varptr)varptr+=mtest_diff;
      ((varent *)(i=vartab+varstk[varstkptr+1]))->less= -1,
      ((varent *)i)->grtr= -1,
      ((varent *)i)->next=ext,
      ((varent *)i)->namelen=varlen,
      ((varent *)i)->valalloc= -(level+1),
      ((varent *)i)->vallen=0;
      if(varptr)*(int *)varptr=varstk[varstkptr+1]-varstk[varstkptr];
      varstk[varstkptr+1]+=ext;
      memcpy(i+sizeof(varent),name,varlen);
   }
}

void vardup() /* make an exact copy of the variables to pass into the
                 next procedure */
{
   int ext=varstk[varstkptr]-varstk[varstkptr-1];
   int exist;
   int *slot;
   register char *i,*j,*k;
   /* test for memory. The new level requires no more memory than the
      previous one */
   mtest(vartab,vartablen,varstk[varstkptr+1]+ext+1,ext+10);
   /* Compress the old variables into the new level */
   i=vartab+varstk[varstkptr-1],
   j=k=vartab+varstk[varstkptr];
   while(i<j){
      memcpy(k,i,ext=sizeof(varent)+align(((varent *)i)->namelen));
      if(((varent *)k)->valalloc>=0)((varent *)k)->valalloc= -varstkptr;
      ((varent *)k)->next= ext,
      ((varent *)k)->less= -1,
      ((varent *)k)->grtr= -1,
      ((varent *)k)->vallen= 0,
      k+=ext;
      i+=((varent *)i)->next;
   }
   varstk[varstkptr+1]=k-vartab;
   /* hook up the tree structure */
   for(i=k,k=j;k<i;k+=((varent *)k)->next){
      if(k==j)continue;
      ext=varstkptr;
      slot=(int *)varsearch(k+sizeof(varent),((varent *)k)->namelen,&ext,&exist);
      if(!exist) /* should always be true! */ slot[0]=k-j;
   }
}

void vardel(name,len) /* delete name (as in procedure hide) */
int len;
char *name;       /* the name is not deleted, rather given a new */
{                 /* undefined value (to avoid massive restructuring)*/
   int compound=name[0]&128;
   int isstem=compound&&!memchr(name,'.',len);
   int *slot;
   int c;
   char *ans=vartab+varstk[varstkptr];
   if(compound&&!isstem)die(Ebadexpr);
   while((c=less(name,ans+sizeof(varent),len,((varent *)ans)->namelen))&&(*(slot=(int *)ans+1+(c>0)))>=0)ans=vartab+varstk[varstkptr]+*slot;
   if(!c){
      ((varent *)ans)->valalloc=0;
      if(isstem)
         ans+=tailroom((varent*)ans,-1,2*four,varstkptr),
         slot=(int *)(ans+sizeof(varent)+align(((varent *)ans)->namelen)),
         slot++[0]=0,
         slot[0]=-1,
         ((varent *)ans)->vallen=2*four;
      else ((varent *)ans)->vallen= -1;
   }
}

char uc(c)       /* return the upper case of c */
char c;
{
   if(c<'a'||c>'z')return c;
   return c&0xdf;
}

void *pstack(type,len) /* stack current position on the program stack,*/
int type,len;          /* returning the address of a stack item to be */
{                      /* filled in */
   register int *answer,*ptr;
   mtest(pstackptr,pstacklen,epstackptr+len+16,256+len);
   *(ptr=answer=(int *)(pstackptr+epstackptr))=ppc, /* Store the first elmnt */
   *(ptr=(int *)(pstackptr+(epstackptr+=len))-1)=type,/* Store the type, and */
   *--ptr=len,                                      /* the length before it  */
   pstacklev++;                                     /* Record the extra entry*/
   return (void *)answer;
}

int unpstack()      /* examine an entry from the program stack */
                    /* without deleting it.  The type is returned.  */
{
   int type;
   register char *ptr=pstackptr+epstackptr;
   type= *((int *)ptr-1);
   ptr-= *((int *)ptr-2);  /* ptr points to the start of the entry */
   newppc=((struct minstack *)ptr)->stmt;
   return type;
}

void *delpstack() /* Delete the top program stack entry; return its address */
{
   if(!pstacklev)return (void *)(pstackptr+(epstackptr=0));
   pstacklev--;
   return (void *)(pstackptr+(epstackptr-=*((int *)(pstackptr+epstackptr)-2)));
}

int strcmpi(s1,s2)  /* compare s1 & s2 with case independence       */
char *s1,*s2;       /* return 1 if s2 is an initial substring of s2 */
{
   int i;
   for(i=0;s2[i]&&!((s1[i]^s2[i])&0xDF);i++);
   return !s2[i];
}
#if 0
void printstmt(line,st,error)   /* print the source statement indicated */
int line,st,error;              /* if error=1 then precede with +++     */
{
   int i=line; /* temporary */
   char c;
   int spc=0;
   char quote=0;
   char *st1=stmts(&line,st);   /* Find the start and end of the statemtent */
   char *st2=stmts(&i,st+1);    /* in the source code */
   char *st3;
   static char *symwords[]=     /* the symbolic tokens */
      {"||","&&","==","<=",">=","<>","\\==","//","<<",">>","<<=",">>=","**"};
   char *what=error?"+++":"*-*"; /* The trace prefix */
   if(!st)st++;
   if(!line){ /* interpreted ... un-parse the line */
      printf("  --- %s ",what);
      for(i=0;i<traceindent*pstacklev;i++)putchar(' '); /* indent */
      for(st1=interp;--st;){                            /* find statement */
         while((c=st1[0])&&c!=THEN&&c!=-1)st1++;        /* (easy!) */
         if(c&&st1[1]==THEN)st1++;
         if(c)st1++;
      }
      if(!st1[0]){puts("<EOL>");return;}  /* statement doesn't exist */
      while((c=st1[0])&&c!=-1&&c!=THEN){  /* Print up to next terminator */
         if(c<SYMBOL){                    /* Print a word */
            if(spc)putchar(' ');
            for(st2=words[c+128];st2[0];st2++)putchar(st2[0]|0x20);
            putchar(' ');
            spc=0;
         }
         else if(c<0){  /* Print a symbolic token */
            if(spc)putchar(' ');
            printf("%s",symwords[c-(SYMBOL+1)]);
            putchar(' ');
            spc=0;
         }
         else {  /* Print a character; lowercase it if outside quotes */
            if(quote&&c==quote)quote=0;
            else if((c=='\''||c=='\"')&&!quote)quote=c;
            if((c>='A'&&c<='Z')&&!quote)c|=0x20;
            putchar(c);
            spc=(c!=' ');
         }
         st1++;
      }
      if(c==THEN){  /* Print a terminating THEN */
         if(spc)putchar(' ');
         puts("then");
      }
      else putchar('\n');
      return;
   } /* Print a regular source line (or lines) */
   if(st2)if(st2[-1]==';')st2--;      /* Remove a final semicolon */
   if(st1&&st2) /* calculate column at which the stmt starts */
      for(spc=0,st3=(*source)[line];
            st3<st2&&(c=st3[0],st3<st1||c==' '||c=='\t');st3++)
         if(c=='\t')spc=8+(spc&~7);
         else spc++;
   do{
      printf("%5d %s ",line,what);
      for(i=0;i<traceindent*pstacklev;i++)putchar(' '); /* indent */
      if(st1&&st2){           /* Both ends of the statement found, so print */
         for(i=0;i<spc&&st1<st2&&((c=st1[0])==' '||c=='\t');st1++)
            if(c=='\t')i=8+(i&~7);     /* Remove leading spaces */
            else i++;
         while(i>spc)putchar(' '),i--; /* Print part of a tab if necessary */
         for(;st1<st2&&st1[0];st1++)
            printf("%c",st1[0]);  /* Print the statement, up to EOL */
         if(st1<st2&&line<lines)st1=(*source)[++line]; /* Go to next line */
      }
      else if(line>lines)fputs("<EOF>",stdout);  /* Line wasn't found */
      else fputs("<EOL>",stdout);                /* statement wasn't found */
      putchar('\n');
   } while(st1&&st2&&st1<st2&&line<=lines);
}
#endif

void freestack(ptr,i)    /* free areas indicated by program stack type i */
void *ptr;               /* stack entry starts at ptr */
int i;
{
   extern int address1,address2;
   register struct procstack2 *sptr=(struct procstack2 *)ptr;
   if(i==11||i==12) /* internal call */
      interplev--,
      free(cstackptr),
      cstackptr=sptr->csp,
      cstacklen=sptr->csl,
      ecstackptr=sptr->ecsp,
      prog=sptr->prg,
      stmts=sptr->stmts,
      timeflag=(timeflag&4)|(sptr->tim &1),
      trcflag=sptr->trc,
      microsecs=sptr->mic,
      secs=sptr->sec,
      address1=sptr->address1,
      address2=sptr->address2,
      numform=sptr->form,
      precision=sptr->digits,
      fuzz=sptr->fuzz;
   else if(i==14) /* interpret */
      interplev--,
      free(prog[0].source),  /* the interpreted string */
      free(prog[0].line),    /* the tokenised string */
      free((char*)prog),     /* the statement table */
      stmts=((struct interpstack *)sptr)->stmts,
      prog=((struct interpstack *)sptr)->prg;
   else if(i==16) /* interactive() stored calculator stack */
      free(cstackptr),
      cstackptr=sptr->csp,
      cstacklen=sptr->csl,
      ecstackptr=sptr->ecsp,
      interact=-1;
   else if(i==20) /* saved traceback line */
      prog=((struct errorstack *)sptr)->prg,
      stmts=((struct errorstack *)sptr)->stmts;
   if(i==12)      /* reclaim procedural variables */
      varstkptr--;
   if(i>=11&&i<=14 && sgstack[interplev+1].data) /* reclaim condition data */
      free(sgstack[interplev+1].data);
}

static char tracebuff[maxtracelen+1];
static int tracepos=0;

void tracestr(str)     /* like traceput but length parameter is not needed */
char *str;
{
   traceput(str,strlen(str));
}

void traceput(str,len) /* like fputs to the trace output stream. */
char *str;             /* The line is output if the last char is \n. */
int len;
{
   char c;
   static RXSIOTRC_PARM sio;
   int cr;
   if(!len)return;
   if((cr=str[len-1]=='\n'))len--;
   while(len--)
      if(tracepos<maxtracelen)
         tracebuff[tracepos++]=(((c=str++[0])&127)<' '||c==127)?'?':c;
   if(!cr)return;
   if(tracepos==maxtracelen)
      tracebuff[maxtracelen-1]='.',
      tracebuff[maxtracelen-2]='.',
      tracebuff[maxtracelen-3]='.';
   tracebuff[tracepos]=0;
   sio.rxsio_string.strptr=tracebuff;
   sio.rxsio_string.strlength=tracepos;
   tracepos=0;
   if(exitlist[RXSIO] && exitcall(RXSIO,RXSIOTRC,&sio)==RXEXIT_HANDLED)
      return;
   fputs(tracebuff,traceout);
   putc('\n',traceout);
}

void tracechar(ch)
char ch;
{
   if(ch=='\n')traceput("\n",1);
   else if(tracepos<maxtracelen)
      tracebuff[tracepos++]=(ch&=127)<' '||ch==127?'?':ch;
}

void tracenum(num,len) /* print a number to the trace output stream. */
int num,len;
{
   static char buff[20];
   sprintf(buff,"%*d",len,num);
   traceput(buff,strlen(buff));
}

void traceprefix(num,prefix) /* print a trace prefix */
int num;
char *prefix;
{
   static char buff[20];
   if(num)sprintf(buff,"%5d %s ",num,prefix);
   else sprintf(buff,"      %s ",prefix);
   traceput(buff,strlen(buff));
}

char *traceget(len)  /* get input for interactive trace */
int *len;            /* space must be freed by caller */
{
   char *inbuf=allocm(RXRESULTLEN);
   int inlen;
   RXSIODTR_PARM rxio;
   if(exitlist[RXSIO]){
      MAKERXSTRING(rxio.rxsiodtr_retc,inbuf,RXRESULTLEN);
      if(exitcall(RXSIO,RXSIODTR,&rxio)==RXEXIT_HANDLED){
         if(rxio.rxsiodtr_retc.strptr!=inbuf)free(inbuf);
         *len=rxio.rxsiodtr_retc.strlength;
         return rxio.rxsiodtr_retc.strptr;
      }
   }
   fputs(">trace>",ttyout),fflush(ttyout);
   clearerr(ttyin);
   if(!(fgets(inbuf,RXRESULTLEN,ttyin)))inlen=0;
   else inlen=strlen(inbuf)-1;
   *len=inlen;
   return inbuf;
}

void interactive() /* interactive tracing - called whenever the tracer might */
{                  /* want to stop for input */
   char *inbuf;
   int inlen;
   char **ocurargs=curargs;   /* Save the arguments to the current procedure */
   int *ocurarglen=curarglen; /* in case of a trap from a lower procedure */
   int oppc=ppc;              /* save also the current position */
   int i;
   struct interactstack *entry;
   if((!(trcflag&0x80)) || interact>=0)
      return;                 /* Continue only in interactive mode */
   if(interactmsg)
      interactmsg=0,
      fputs("      +++ Interactive trace.  TRACE OFF to end debug, ENTER to continue. +++",ttyout),
      putc('\n',ttyout);
   entry=(struct interactstack *)pstack(16,sizeof(struct interactstack));
   entry->csp=cstackptr,      /* Now fill in a program stack entry for the */
   entry->csl=cstacklen,      /* commands typed in */
   entry->ecs=ecstackptr;
   otrcflag=trcflag;
   cstackptr=allocm(cstacklen=200); /* Make a new calculator stack. */
   ecstackptr=0;
   trclp=1;                   /* signal "do wait for more input" */
   while(trclp){              /* Until the user restarts the program ...*/
      returnlen=-1;           /* signal that a RETURN was not executed */
      inbuf=traceget(&inlen); /* input a line */
      returnval=0;
      if(!inlen)break;        /* No input -> continue with program */
      interact=interplev;     /* signal "interactive mode" */
      trcflag=Terrors;        /* turn tracing "off" while interpreting input */
      if(setjmp(interactbuf)) /* Save the context in case of an error */
         curargs=ocurargs,    /* error! restore the correct context */
         curarglen=ocurarglen,
         ppc=oppc,
         returnlen=-1;
      else returnval=rxinterp(inbuf,inlen,&returnlen, /* Interpret */
                              "TRACE",RXSUBROUTINE,curargs,curarglen);
      free(inbuf);
      if(trclp==1)trcflag=otrcflag; /* Unless the input contained a trace
                                       command, restore the old trace flag. */
      if(returnlen>=0)break;  /* Continue with program if a RETURN occurred */
   }
   interact= -1;              /* signal "not interactive mode" */
   if(returnval)returnfree=cstackptr; /* The result's user will free it */
   else free(cstackptr);      /* Nothing of value was on the stack */
   while(i=*((int *)(pstackptr+epstackptr)-1)!=16)/* Clear the program stack */
      freestack(delpstack(),i);
   entry=(struct interactstack *)delpstack();/* delete interactive()'s entry */
   cstackptr=entry->csp,                     /* and restore the old stack */
   ecstackptr=entry->ecs,
   cstacklen=entry->csl;
   if(returnlen>=0)      /* if a RETURN occurred, jump back to do the return */
      longjmp(sgstack[interplev].jmp,-1);
}

/* The following function loads a source file from disk and returns the
   block of memory allocated to hold it.  The return value is null if 
   an error occurred. */
char *load(name,sourcelen)
char *name;          /* The path name of the program */
int *sourcelen;      /* The length of the source (to be returned) */
{
   struct stat buf;  /* For finding the size of the program */
   int f= -1;        /* A file descriptor */
   unsigned size;    /* The size of the program */
   char *store;      /* The memory allocated to hold the source */

/* find size of file */
   if (stat(name,&buf)==-1)return 0;
   size=buf.st_size,
/* get mem for the file */
   store=allocm(size+2);
/* read file */
   if((f=open(name,O_RDONLY))==-1){
      free(store);
      return 0;
   }
   if(read(f,store,size)!=size){
      free(store);
      return 0;
   }
   close(f);
   if(store[size-1]!='\n')store[size++]='\n'; /* terminate last line */
   store[size]=0;
   *sourcelen=size;  /* Ahem! */
   return store;
}

/* The following function preprocesses a block of source passed to it.
   Space for the preprocessed program and the label tabel is allocated
   and assigned to global variables.  Also, the source is broken into
   lines and a source line table is allocated.  The 0th line of source
   is usually its file name.  However this will be inserted by the caller. */
void tokenise(input,ilen,interpret,line1)
char *input;         /* the source code */
int ilen;            /* length of the source code */
int interpret;       /* if nonzero, ignore labels and do not make a source
                        line table */
int line1;           /* if nonzero, the first line is a comment */
{
   static char msg[20];/* For reporting invalid chars */
   int type;         /* Type of a character */
   int comment=0;    /* Comment nesting level */
   int commentstart; /* Start stmt number of a comment */
   int comma=0;      /* Continuation character is in force */
   int start=1;      /* the start of a statement */
   char first=0;     /* the first word in this statement */
   char last=0;      /* the most recent word in this statement */
   char token=0;     /* candidate token number */
   int spc=0;        /* a space just occurred */
   int wordlen=0;    /* length of a stored word */
#define word varnamebuf /* "word" seems a better name just now */
   int spcbefore=0;  /* Put a space before the word */
   int gobble=1;     /* whether a character gobbles spaces */
   int sourcelen=100;/* lines allocated in source line table */
   int proglen=100;  /* statements allocated in program line table */
   int plen=ilen+2;  /* length allocated for program */
   char*srcptr=input;/* pointer into the source */
   char *prgptr;     /* pointer into the program */
   char *prevptr;    /* source address for the stored word */
   int lablen;       /* Length allocated to labels */
   int elabptr;      /* Length of labels so far */
   char c;
   char *ptr;
   int i;
   int ch;

   if(!interpret)source=(char**)allocm(sourcelen*sizeof(char*));
   prog=(program*)allocm(proglen*sizeof(program));
   prgptr=prog[0].line=allocm(plen);/* plen=ilen+2 is a guaranteed upper 
          bound (the 2 extra are a line terminator and program terminator) */
   prog[0].source=input;
   prog[0].num=!interpret;
   if(!interpret)
      source[0]=cnull,
      labelptr=allocm(lablen=200),
      elabptr=0;
   stmts=0;
   if(!interpret)lines=0;
   if(!interpret && (line1 || ilen>2&&srcptr[0]=='#'&&srcptr[1]=='!')){
      source[++lines]=srcptr;
      while(ilen--&&srcptr++[0]!='\n');
      if(ilen<0)ilen++;
      else srcptr[-1]=0;
   }
   prog[0].sourcend=srcptr;
   if(ilen){
      if(!interpret)source[++lines]=srcptr;
      prog[++stmts].line=prgptr,
      prog[stmts].num=(interpret?0:lines),
      prog[stmts].source=srcptr,
      prog[stmts].sourcend=0,
      prog[stmts].related=0;
   }
   ppc=0;                 /* this must be a signal that no ppc is available */
   while(ilen-- || !interpret&&srcptr>source[lines] || wordlen || !start){
      if(ilen<0){         /* we repeat the loop to finish off the source */
         ilen++;          /* This happens when the last line is unterminated */
                          /* The last byte of source will be overwritten with
                          \0.  This only fails if input was an empty string. */
         c='\n';
         if(comment)die(Elcomm);
      }
      else c=srcptr++[0];
      if(c=='\n'){
         srcptr[-1]=0;
         if(!interpret){
            if(sourcelen-1<=++lines)
               if(ptr=(char*)realloc((char*)source,(sourcelen+=50)*sizeof(char*)))
                  source=(char**)ptr;
               else die(Emem);
            source[lines]=srcptr;
            if(comma){
               if(!ilen)die(Ecomma); /* Last line ended with comma */
               prgptr--,
               gobble--,     /* restore gobble to previous val */
               comma=0,
               c=' ';
            }
            else c=';';      /* line ends terminate statements.  Note:
                                this is ineffective within comments */
         }
         else
            if(!ilen)
               if(comma)die(Ecomma); /* interpreted line ends with comma */
               else c=';';           /* terminate the interpreted line */
            else /* do nothing.  \n will be rejected later. */ ;
      }
      if(c=='^')c='\\';   /* Translate "^" into the real "not" character */
      if(c=='*'&&ilen&&srcptr[0]=='/'){
                /* if(--comment<0)die(Ercomm);  Not an error really. */
         if(--comment<0)comment=0;
         else srcptr++,ilen--,
              c=' ';      /* Comment equals space.  This should be changed. */
      }
      if(c=='/'&&ilen&&srcptr[0]=='*'){
         if(comment++==0)commentstart=stmts;
         srcptr++,ilen--;
      }
      if(comment)continue;/* all characters within comments are ignored. */
      if((type=whattype(c))==-2){           /* Invalid character */
         if(c<127&&c>' ')sprintf(errordata=msg,": \'%c\'",c);
         else sprintf(errordata=msg,": \'%02x\'x",(int)(unsigned char)c);
         die(Echar);
      }
      if(c==' '||c=='\t'||c=='\r'){
         spc=1;
         continue;
      }
      /* A non-blank source character has been found within a line */
      /* Time to emit the stored word (if any) */
      comma=0;
      if(c==':'&&start&&wordlen){             /* the stored word is a label */
         if(word[wordlen-1]=='.')die(Elabeldot); /* Ends with dot */
         if(interpret)die(Exlabel);
         /* Add the label to the label table */
         mtest(labelptr,lablen,elabptr+wordlen+4*four,256+wordlen);
         *((int *)(labelptr+elabptr))=wordlen,
         *((int *)(labelptr+elabptr)+1)=stmts,
         memcpy(labelptr+(elabptr+=2*four),word,wordlen),
         *(labelptr+elabptr+wordlen)=0,
         elabptr+=align(wordlen+1);
         /* Add a LABEL clause to the program */
         if(stmts+2>proglen)
            if(ptr=(char*)realloc((char*)prog,(proglen+=50)*sizeof(program)))
               prog=(program*)ptr;
            else die(Emem);
         prgptr++[0]=LABEL;
         prgptr++[0]=0;
         prog[stmts].source=prevptr;
         prog[stmts].sourcend=srcptr;
         prog[stmts].num=lines;
         prog[++stmts].line=prgptr;
         prog[stmts].num=lines;
         prog[stmts].source=srcptr;
         prog[stmts].sourcend=0;
         prog[stmts].related=0;
         wordlen=spcbefore=spc=0;
         gobble=1;
         continue;
      }
      /* as it is not a label, the word is uppercased */
      for(i=wordlen,ptr=word;i--;ptr++)ptr[0]=uc(ptr[0]);
      if(c=='='&&wordlen&&(start||last==DO)){ /* the stored word is a symbol */
         if(rexxsymbol(word[0])<1)die(Ename); /* Starts with number or dot */
         memcpy(prgptr,word,wordlen),
         prgptr+=wordlen,
         prgptr++[0]=c,
         wordlen=spcbefore=spc=0;
         gobble=1;
         start=0;
         last=0;
         continue;
      }
      /* the word may now be a token. */
      if(wordlen){
         for(i=0;i<numwords&&strcmp(word,words[i]);i++); 
         if(i<numwords)token=(i-128);
         else token=0;
         if(token<Command&&!start){ /* "Commands" must be at the start, */
            if(token==NUMERIC&&last==PARSE);  /* except NUMERIC & SELECT */
            else if(token==SELECT&&first==last&&last==END);
            else token=0;
         }
         else if(token>=Command&&start){ /* at the start must be a "command" */
            if(token==THEN);             /* except THEN, PULL and ARG */
            else if(token==ARG||token==PULL)
               prgptr++[0]=PARSE,
               prgptr++[0]=UPPER,
               first=last=PARSE,
               start=0;
            else token=0;
         } /* Now some special case checking... */
         if(!token); /* no need to check if there is no token */
         else if(token==VALUE)if(last==ADDRESS||last==FORM||last==TRACE
                               ||last==PARSE||last==SIGNAL);else token=0;
         else if(token==UPPER)if(last==PARSE);else token=0;
         else if(token>=PULL&&token<=LINEIN)if(last==PARSE);else token=0;
         else if(token==WITH)if(first==VALUE);else token=0;
         else if(token==ON||token==OFF)if(last==SIGNAL||last==CALL)
                        first=token;/* allow NAME */ else token=0;
         else if(token==NAME)if(first==ON)first=token;else token=0;
         else if(token>=TO&&token<=FOR)if(first==DO);else token=0;
         else if(token==FOREVER)if(last==DO);else token=0;
         else if(token==WHILE||token==UNTIL)if(first==DO||first==WHILE)
                        first=WHILE; /* disable TO, BY, FOR */ else token=0;
         else if(token==EXPOSE||token==HIDE)if(last==PROCEDURE);else token=0;
         else if(token>=DIGITS&&token<=FORM)if(first==last&&last==NUMERIC);
                        else token=0;
         else if(token==THEN)if(start||first==IF||first==WHEN);else token=0;
         if(start)first=token;       /* Save first token in each line */
         if(token!=UPPER)last=token; /* Save the previous token */
         if(token==VALUE&&first==PARSE)first=token; /* allow WITH */
         if(token==WITH)first=token;                /* disallow WITH */
         if(token)wordlen=0;
      }
      else token=0;
      if(wordlen){   /* If there is still a word, it is a symbol */
         if(spcbefore)prgptr++[0]=' ';
         memcpy(prgptr,word,wordlen),
         prgptr+=wordlen,
         wordlen=0,
         start=0,
         gobble=0;
      }
      /* Check for space in case we add a new statement or two */
      if(token==THEN || token==ELSE || token==OTHERWISE || c== ';')
         if(stmts+3>=proglen)
           if(ptr=(char*)realloc((char*)prog,(proglen+=50)*sizeof(program)))
              prog=(program*)ptr;
           else die(Emem);
      if(token==THEN || token==ELSE || token==OTHERWISE){
         /* these tokens start new statements */
         if(!start){
            prgptr++[0]=0;
            prog[stmts].sourcend=prevptr,
            prog[++stmts].line=prgptr,
            prog[stmts].source=prevptr,
            prog[stmts].num=(interpret?0:lines),
            prog[stmts].related=0;
         }
         prgptr++[0]=token,
         prgptr++[0]=0;
         prog[stmts].sourcend=srcptr-1;
         prog[++stmts].line=prgptr,
         prog[stmts].num=(interpret?0:lines),
         prog[stmts].source=srcptr-1,
         prog[stmts].sourcend=0,
         prog[stmts].related=0;
         token=0;
         start=gobble=1;
         first=last=0;
      }
      else if(token){
         prgptr++[0]=token;
         gobble=1;
         start=0;
      }
      if(c==';'){
         if(start){
            prog[stmts].source=srcptr,        /* delete the source of the */
            prog[stmts].num=(interpret?0:lines);   /* null statement, but */
            continue;                         /* don't make an extra line */
         }
         prgptr++[0]=0;
         prog[stmts].sourcend=srcptr-1,
         prog[++stmts].line=prgptr,
         prog[stmts].source=srcptr,
         prog[stmts].sourcend=0,
         prog[stmts].num=(interpret?0:lines),
         prog[stmts].related=0;
         start=gobble=1;
         first=last=0;
         continue;
      }
      if(c==','){
         comma=1,
         gobble++,        /* this saves the previous value of gobble */
         spc=0,           /* and also makes gobble true */
         prgptr++[0]=c;
         continue;
      }
      /* Proceed to insert some non-blank characters.  Gobble any previous
         spaces if necessary. */
      if(gobble)gobble=spc=0;
      if(type<=0 && c!='\'' && c!='\"'){ /* non-alpha and non-quote char */
         if(c!='(')spc=0;                /* all except "(" gobble on left */
         if(c!=')')gobble=1;             /* all except ")" gobble on right */
      }
      if(c=='\"'||c=='\''){
         if(spc)prgptr++[0]=' ',spc=0;
         prgptr++[0]=c;
         while(ilen--&&srcptr[0]!=c&&srcptr[0]!='\n')prgptr++[0]=srcptr++[0];
         if(srcptr++[0]!=c)die(Equote);
      }
      if(!type){                         /* Can't be a token. Just insert it */
         if(spc)prgptr++[0]=' ',spc=0;
         prgptr++[0]=c;
         start=last=0;
         continue;
      }
      if(type<0){                        /* might be a multi-char operator */
         ptr=srcptr;
         i=ilen;
         wordlen=0;
         ch=c;
         while(wordlen<3){
            while(i&&(ptr[0]==' '||ptr[0]=='\t'))i--,ptr++;
            if(whattype(ptr[0])!=-1)break;
            ch=(ch<<8)+ptr[0];
            ptr++,i--,wordlen++;
         }
         token=0;
         while(!token&&wordlen)
            switch(ch){
               case Cconcat: token=CONCAT; break; /* || */
               case Cxor:    token=LXOR;   break; /* && */
               case Cequ:    token=EQU;    break; /* == */
               case Cleq1:                        /* <= */
               case Cleq2:   token=LEQ;    break; /* \> */
               case Cgeq1:                        /* >= */
               case Cgeq2:   token=GEQ;    break; /* \> */
               case Cneq1:                        /* \= */
               case Cneq2:                        /* <> */
               case Cneq3:   token=NEQ;    break; /* >< */
               case Cnneq:   token=NNEQ;   break; /* \== */
               case Cmod:    token=MOD;    break; /* // */
               case Cless:   token=LESS;   break; /* << */
               case Cgrtr:   token=GRTR;   break; /* >> */
               case Clleq1:                       /* <<= */
               case Clleq2:  token=LLEQ;   break; /* \>> */
               case Cggeq1:                       /* >>= */
               case Cggeq2:  token=GGEQ;   break; /* \<< */
               case Cpower:  token=POWER;  break; /* ** */
               default: ch>>=8,wordlen--;
            }
         if(token)ch=token;
         prgptr++[0]=ch;
         while(wordlen){
            while(ptr[0]==' '||ptr[0]=='\t')ilen--,srcptr++;
            ilen--,srcptr++,wordlen--;
         }
         gobble=1;
         start=0;
         continue;
      }
      /* We have an alphanumeric character.  Store a word. */
      prevptr=srcptr-1;
      spcbefore=spc;
      spc=gobble=0;
      ptr=srcptr-1;
      while(ilen--&&rexxsymboldot(srcptr++[0]));
      if(++ilen>0)srcptr--;
      wordlen=srcptr-ptr;
      mtest(word,varnamelen,wordlen+1,wordlen+1-varnamelen);
      memcpy(word,ptr,wordlen),
      word[wordlen]=0;
   }
   /* All characters considered; ilen was zero and the source was terminated */
   prgptr++[0]=0;
   prog[stmts].sourcend=srcptr-1;
   if(!interpret)lines--;  /* Discount the new line started at the last '\n' */
                           /* It will remain in the line table, however. */
   /* Now shrink all areas to their correct sizes */
   if(ptr=realloc((char*)prog,(1+stmts)*sizeof(program)))
      prog=(program*)ptr;
   if(!interpret && (ptr=realloc((char*)source,(2+lines)*sizeof(char*))))
      source=(char**)ptr;
   if(ptr=realloc(prog[0].line,prgptr-prog[0].line))
      if(ptr!=prog[0].line)
         /* Oops, the program moved! */
         for(i=stmts;i--;prog[i].line+=ptr-prog[0].line);
   if(!interpret){
      if(ptr=realloc(labelptr,elabptr+four))
         labelptr=ptr;
      (*(int *)(labelptr+elabptr))=0;
   }
   if(comment)stmts=commentstart,die(Elcomm);
}
#undef word

/* This function prints the source associated with a particular statement.
   If "after" is non-zero, it prints the source (if any) occurring between
   this statement and the next.  It prefixes the source with "*-*" unless
   "error" is non-zero, in which case the prefix is "+++". */
void printstmt(stmt,after,error)
int stmt,after,error;
{
   int line=prog[stmt].num;      /* source line number */
   char *start,*end;             /* start and end of the source */
   char *what=error?"+++":"*-*"; /* The trace prefix */
   int spc;                      /* How much indentation there is */
   char *ptr;
   int i;
   if(stmt>stmts){               /* This never happens, I hope... */
      traceprefix(lines+1,what);
      tracestr("<EOF>\n");
      return;
   }
   else if(after){
      for(start=prog[stmt].source;start<prog[stmt].sourcend;start++)
         if(line&&start+1==source[line+1])
            ++line;     /* find the line number of the source end */
      end=prog[stmt+1].source;
   }
   else start=prog[stmt].source,end=prog[stmt].sourcend;
   if(!end){                     /* This never happens, I hope... */
      traceprefix(line,what);
      tracestr("<EOL>\n");
      return;
   }
   while(start<end&&
        (start[0]==0||start[0]==';'||start[0]==' '|start[0]=='\t')){
      if(line&&start+1==source[line+1])
         ++line;
      start++;                   /* step past uninteresting chars */
   }
   while(start<end&&
        (end[-1]==0||end[-1]==';'||end[-1]==' '|end[-1]=='\t'))
      end--;                     /* delete uninteresting trailing chars */
   if(start>=end)return;         /* Nothing to print. */
   if(line)
      for(spc=0,ptr=source[line];ptr<start;ptr++)
         if(ptr[0]=='\t')spc=8+(spc&~7);/* This calculates the column within */
         else spc++;             /* the line in which the statement starts   */
   else spc=0;
   do{
      traceprefix(line,what);
      for(i=0;i<traceindent*pstacklev;i++)tracechar(' ');  /* indent */
      for(i=0;i<spc&&start<end&&(start[0]==' '||start[0]=='\t');start++)
         if(start[0]=='\t')i=8+(i&~7);            /* Remove leading spaces */
         else i++;
      while(i>spc)tracechar(' '),i--;/* Print part of a tab if necessary */
      for(;start<end&&(!line||start<source[line+1]-1);start++)
         if((i=start[0]&127)<' '||i==127)tracechar('?');
         else tracechar(i);                             /* Print statement */
      if(start<end&&line<lines)start=source[++line];    /* Go to next line */
      tracechar('\n');
      if(!error)what="*,*";           /* new ANSI prefix for continuations */
   } while(start<end&&line<=lines);
}
#if 0
void expand(c)   /* this is an old test routine. */
char c;
{
   static char *symwords[]={"||","&&","==","<=",">=","<>","\\==","//","<<",">>","<<=",">>=","**"};
   static char invvideo[]={27,'[','1','m',0};
   static char truevideo[]={27,'[','m',0};
   if(c==-1){printf("%s;%s",invvideo,truevideo);return;}
   printf("%s ",invvideo);
   if(c>SYMBOL)printf("%s",symwords[c-(SYMBOL+1)]);
   if(c<numwords-128)printf(words[c+128]);
   printf(" %s",truevideo);
}

void display(line,ptr) /* so is this */
int line,ptr;
{
   char *s=((*prog)[line]);
   char c;
   int i=0;
   printf("      +++ %d +++ ",ppc);
   if(s==cnull)puts("(null)");
   while(c=s[i++]){
      if(c<0)expand(c);
      else putchar(c);
      if(i==ptr)printf("[*]");
   }
   putchar('\n');
}
#endif /* end of the old tokenisation routines which are commented out */

/* Return the default file extension (e.g., ".rexx") by checking the
   environment and then returning the system default. */
char *rexxext() {
   static char answer[maxextension];
   char *getenv();
   char *env=getenv("REXXEXT");
   if (env) {
      if (env[0]=='.' && env[1]) return env;
      if (!env[0] || strlen(env)>sizeof answer-2) return filetype;
      answer[0]='.';
      strcpy(answer+1,env);
      return answer;
   }
   else return filetype;
}

int which(gn,opt,fn)/* finds a file given name `gn'; puts path name in `fn'.
                     opt<0 indicates that we are looking for a Unix
                           program (namely, rxque).
                     opt=0 indicates that the default extension should be
                           appended, unless it is already at the end of gn.
                           If not found then try without extension.
                     opt=1 indicates that it is not to be appended.
                     opt=2 means do a full search for a REXX function.
                     opt=3 means search for a dll (.rxfn or exact name).*/
char *fn,*gn;
int opt;            /* returns 0 if not found, 1 if rexx found, */
{                   /*         2 if .rxfn found, 3 if Unix program found.  */
   char *getenv();
   char *path;      /* an element of the path */
   char *pathend;   /* end of this element */
   char *basename;  /* basename of the file to search for */
   int baselen;     /* length of basename */
   int pathlen;     /* length of path */
   int gpathlen;    /* length of path component in given name */
   int tmplen;      /* length of temporary filename buffer */
   char *defaultext=0; /* default extension */
   static char tmp[MAXPATHLEN];  /* temporary filename buffer */
   DIR *dp;
   struct dirent *dir;
   int found=0;     /* 0->nothing found, 1->unix prog found, 2->.rexx found
                       3->default filetype found, 4->.rxfn found  */
   int copy;
   int go=1;
   int doexec=1;    /* 0 -> don't search for default extension */
   int dot=0;

   if (opt!=1 && opt!=3) {
      /* get system default extension and see if it is the same as the
         current default */
      defaultext=rexxext();  
      doexec=strcmp(extension,defaultext);
   }

/* split name into pathname and basename */
   if((basename=strrchr(gn,'/')))gpathlen=basename++-gn;
   else gpathlen=0,basename=gn;
   baselen=strlen(basename);
   if(opt==0 && baselen>extlen && !strcmp(basename+baselen-extlen,extension))
      opt=1;
/* find out where to look. */
   if(gn[0]=='.' && gn[1]=='/'){             /* special case for files in ./ */
      path=".";                              /* make path="." and remove the */
      gn+=2;                                 /* "." from the name.  This     */
      if((gpathlen-=2)<0)gpathlen=0;         /* causes "." to be expanded.   */
   }
   else if(gn[0]=='.' && gn[1]=='.' && gn[2]=='/') /* for files in ../ */
      path=".";                              /* prepend current dir name */
   else if(gn[0]=='/'){
      path="";                               /* path given; prepend nothing */
      dot=1;                                 /* don't search "." either */
      if(opt==1){                            /* whole name given - no search*/
         strcpy(fn,gn);
         return !access(fn,0);
      }
   }
   else {
      path=0;
      if(opt==3)path=getenv("REXXLIB");      /* DLLs in REXXLIB */
      if(opt>=2&&!(path && path[0]))
         path=getenv("REXXFUNC");            /* functions in REXXFUNC */
      if(opt==3&&!(path && path[0]))
         path=rxpath;                        /* default for DLLs is my libpath */
      if(opt>=0&&!(path && path[0]))
         path=getenv("REXXPATH");            /* REXX programs in REXXPATH */
      if(!(path && path[0]))path=getenv("PATH"); /* or in PATH */
      if(!(path && path[0]))path=".";            /* or in "." */
   }
   if(opt<0)opt=1;
/* scan each directory in the path */
   while(go && path){
      if((pathend=strchr(path,':')))pathlen=pathend++-path;
      else pathlen=strlen(path);
      if(pathlen==1 && path[0]=='.'){
         dot=1;
         if(!getcwd(tmp,sizeof tmp) || tmp[0]!='/') strcpy(tmp,".");
         tmplen=strlen(tmp);
      }
      else memcpy(tmp,path,tmplen=pathlen);
      if(gpathlen && gn[0]!='/' && pathlen)tmp[tmplen++]='/';
      memcpy(tmp+tmplen,gn,gpathlen);
      tmp[tmplen+=gpathlen]=0;
      if((dp=opendir(tmp))){
         while(go && (dir=readdir(dp))){/* for each file in the directory */
            if(memcmp(dir->d_name,basename,baselen))
               continue;       /* check that it starts with basename */
            copy=0;            /* if "copy" gets set then the current */
                               /* name will be saved. */
            switch(opt){       /* validate the name according to opt */
               case 0: if (!strcmp(dir->d_name+baselen,extension))
                     copy=found=1,go=0;
                  else if (!found && !dir->d_name[baselen])copy=found=1;
                  break;
               case 1: go=dir->d_name[baselen];
                  if(!go)copy=found=1;
                  break;
               case 2:
                  if(!strcmp(dir->d_name+baselen,".rxfn"))copy=found=4,go=0;
                  else if(found<3 && !strcmp(dir->d_name+baselen,extension))
                     copy=found=3;
                  else if(doexec && 
                        found<2 && !strcmp(dir->d_name+baselen,defaultext))
                     copy=found=2;
                  else if(found<1 && !dir->d_name[baselen])copy=found=1;
                  break;
               case 3:
                  if(!dir->d_name[baselen])copy=found=1,go=0;
                  else if(!strcmp(dir->d_name+baselen,".rxfn"))copy=found=4;
            }
            if(copy)
               strcpy(fn,tmp),
               fn[tmplen]='/',
               strcpy(fn+tmplen+1,dir->d_name);
         }
         closedir(dp);
      }
      else if (!access(tmp,X_OK)) {
         /* opendir failed - probably an unreadable directory. Try access().*/
         tmp[tmplen]='/';
         memcpy(tmp+tmplen+1,basename,baselen);
         tmp[tmplen+=baselen+1]=0;
         copy=0;
         switch(opt) {
            case 0:
               strcpy(tmp+tmplen,extension);
               if (!access(tmp,0)) {copy=found=1;go=0;break;}
               tmp[tmplen]=0;
               if (!found && !access(tmp,0)) copy=found=1;
               break;
            case 1:
               if (!access(tmp,0)) {copy=found=1;go=0;}
               break;
            case 2:
               strcpy(tmp+tmplen,".rxfn");
               if (!access(tmp,0)) {copy=found=4;go=0;break;}
               if (found==3) break;
               strcpy(tmp+tmplen,extension);
               if (!access(tmp,0)) {copy=found=3;break;}
               if (doexec && found<2) {
                  strcpy(tmp+tmplen,defaultext);
                  if (!access(tmp,0)) {copy=found=2;break;}
               }
               if (found>0) break;
               tmp[tmplen]=0;
               if (!access(tmp,0)) copy=found=1;
               break;
            case 3:
               if (!access(tmp,0)) {copy=found=1;go=0;break;}
               if (found) break;
               strcpy(tmp+tmplen,".rxfn");
               if (!access(tmp,0)) copy=found=1;
         }
         if (copy) strcpy(fn,tmp);
      }
      path=pathend;
      if(!path && !dot)path=".";
   }
   if(!found){
      strcpy(fn,gn);
      if(opt!=1)strcat(fn,extension);
      errno=ENOENT;
      return 0;
   }
   if(opt<2)return 1;
   if(found==1)return 3;
   if(found==4)return 2;
   return 1;
}

/* Hash table routines */
/* These routines maintain several tables (not actually hash tables, but
   never mind) in the style of the above variable handling routines, except
   that each table is single-level.
   Each table entry contains a hashent structure containing the following
   fields: next (length), grtr, less (tree pointer fields), value (the void*
   value associated with the name), and name. The name is a NUL-
   terminated sequence of characters followed by pad bytes to make up a
   multiple of 4 bytes.
   The hash tables maintained are:
   0. environment variable names => address of storage for their values
   1. file names => address of a structure containing their details
   2. function names => address of structure containing their details

   Each hash table is characterised by three values: hashptr[x] is the
   address of hash table x, hashlen[x] is the amount of storage allocated,
   and ehashptr[x] is the actual length of the table.
*/

char *hashsearch(hash,name,exist)
int hash;
char *name;
int *exist;
/* search for name `name' of length `len' in hash table `hash'.
   The answer is the address of the entry which matches, with `exist'
   non-zero, or, if the name does not exist, exist=0 and the answer
   is the address of the slot where the new branch of the tree is to
   be added. If there are no names in the table, 0 is returned. */
{
   char *data=hashptr[hash];
   char *ans=data;
   int *slot;
   int c;
   *exist=0;
   if(!ehashptr[hash])return cnull;
   while((c=strcmp(name,ans+sizeof(hashent)))
     &&  (*(slot= &(((hashent *)ans)->less)+(c>0)))>=0)
   ans=data+*slot;
   if(!c)return *exist=1,ans;
   return(char*)slot;
}

void *hashget(hash,name,exist) /* like hashsearch, but the value is returned */
int hash;                      /* (if any) */
char *name;
int *exist;
{  
   char *ptr=hashsearch(hash,name,exist);
   if(*exist)return((hashent *)ptr)->value;
   else return 0;
}

void **hashfind(hash,name,exist)
int hash;
char *name;
int *exist;
{  /* like hashsearch, but the address of the value is returned. If no
      value is present, one is created. */
   char *ptr=hashsearch(hash,name,exist);
   int len;
   if(*exist)return &(((hashent *)ptr)->value);
   if(ptr)*(int *)ptr=ehashptr[hash];
   len=align(strlen(name)+1)+sizeof(hashent);
   mtest(hashptr[hash],hashlen[hash],ehashptr[hash]+len,len+256);
   ptr=hashptr[hash]+ehashptr[hash],
   ehashptr[hash]+=len,
   ((hashent *)ptr)->next=len,
   ((hashent *)ptr)->less=-1,
   ((hashent *)ptr)->grtr=-1,
   strcpy(ptr+sizeof(hashent),name);
   return &(((hashent *)ptr)->value);
}

struct fileinfo *fileinit(name,filename,fp) 
char *name,*filename;          /* associate "name" with the file "filename" */
FILE *fp;                      /* which has just been opened on fp          */
{                              /* return the fileinfo structure created     */
   int exist;
   struct stat buf;            /* For finding the file's details */
   void **entry=hashfind(1,name,&exist);
   unsigned len=align(filename?strlen(filename)+1:1);
   struct fileinfo *info=
      (struct fileinfo *)allocm(sizeof(struct fileinfo)+len);
   if(exist&&*entry)           /* What if the name is already used? */
      fclose(((struct fileinfo *)(*entry))->fp),
      free((char*)(*entry));
   *entry=(void *)info;
   if(filename)strcpy((char*)(info+1),filename);
   else *(char*)(info+1)=0;
   if(fp && fstat(fileno(fp),&buf)==0)    /* Make the file persistent if and */
      info->persist=S_ISREG(buf.st_mode); /* only if it can be determined    */
   else info->persist=0;                  /* that it is a regular file       */
   info->fp=fp,                /* fill in the structure with suitable */
   info->wr=0,                 /* defaults */
   info->lastwr=1,             /* lastwr=1 so that the first read does seek */
   info->rdpos=0,              /* usually read from beginning of file */
   info->rdline=1,
   info->rdchars=0,
   info->wrpos=fp?ftell(fp):0, /* Usually write to end of file */
   info->wrline=!info->wrpos,
   info->wrchars=0,
   info->errnum=0;
   if(info->wrpos<0)info->wrpos=0; /* In case ftell failed */
   return info;
}

void funcinit(name,handle,address,saa) /* Associate "name" with a function */
char *name;      /* The REXX name of the function */
void *handle;    /* The handle from dlopen(), if this is the "main" function */
int (*address)();/* The address of the function's implementation */
int saa;         /* calling sequence of the function */
{
   funcinfo *info;
   int exist;
   void **slot=hashfind(2,name,&exist);
   if(!(exist&&*slot)) /* if it exists, a dl handle might be lost. */
      info=(funcinfo *)allocm(sizeof(funcinfo)),
      *slot=(void *)info;
   else info=(funcinfo*)*slot;
   info->dlhandle=handle;
   info->dlfunc=address;
   info->saa=saa;
   if(!address){   /* if the func has no address, just register its name. */
      info->dlhandle=0;
      info->name=allocm(1+strlen((char*)handle));
      strcpy(info->name,(char*)handle);
   }
}

void libsearch(){    /* search for *.rxlib files */
   char *getenv();   /* and hash the functions they contain. */
   char *path=getenv("REXXLIB");
   char *pathend;
   char *file;
   int l;
   int namelen;
   int ch;
   DIR *dp;
   FILE *fp;
   struct dirent *dir;
   int type;
   if(!(path&&path[0]))path=rxpath;
   while(path){
      if((pathend=strchr(path,':'))) /* temporarily change the next ':' */
         pathend[0]=0;               /* into a 0 */
      if((dp=opendir(path))){
         while((dir=readdir(dp))){   /* for each file in the directory */
                                     /* matching *.rxlib ... */
#if defined(sgi) || defined(Solaris) || defined(linux)
            namelen=strlen(dir->d_name);
#else
            namelen=dir->d_namlen;
#endif
            if(namelen>6 &&
            !memcmp(dir->d_name+namelen-6,".rxlib",6)){
               l=strlen(path);
               file=allocm(l+namelen+2);
               strcpy(file,path);
               file[l++]='/';
               strcpy(file+l,dir->d_name);
               l+=namelen;
               if((fp=fopen(file,"r"))){ /* read the file */
                  file[l-6]=0;           /* knock off the ".rxlib" */
                  type=0;
                  while((ch=getc(fp))!=EOF){
                     if(ch==' ' || ch=='\t' || ch=='\r' || ch=='\n')
                        continue;
                     pull[0]=ch;
                     l=1;
                     while((ch=getc(fp))!=EOF &&
                     !(ch==' ' || ch=='\t' || ch=='\r' || ch=='\n')){
                        mtest(pull,pulllen,l+2,256);
                        pull[l++]=ch;
                     }
                     pull[l]=0;
                     if(!strcmp(pull,"rxmathfn:"))
                        type=RXDIGITS;  /* kludge for math functions */
                     else if(!strcmp(pull,"rxsaa:"))
                        type=1;         /* kludge for SAA functions */
                     else funcinit(pull,(void*)file,(int(*)())0,type);
                  }
                  fclose(fp);
               }
               free(file);
            }
         }
         closedir(dp);
      }
      if(pathend)pathend++[0]=':';
      path=pathend;
   }
}

int fileclose(name)  /* close and free the file associated with "name" */
char *name;          /* return the code from close */
{
   int exist;
   int ans=0;
   char *ptr=hashsearch(1,name,&exist);
   struct fileinfo *info;
   if(!exist)return 0;
   info=(struct fileinfo *)(((hashent *)ptr)->value);
   if(info){
      if(info->fp)ans=fclose(info->fp),
      free((char*)info);
   }
   ((hashent *)ptr)->value=0;
   return ans;
}

#ifdef NO_LDL /* Define dummy versions of the dynamic load functions */
void *dlopen(path, mode)
char *path; int mode;
{die(Eexist);/*NOTREACHED*/}

void *dlsym(handle,sym)
void *handle; char *sym;
{die(Eexist);/*NOTREACHED*/}

char *dlerror()
{die(Eexist);/*NOTREACHED*/}

int dlclose(handle)
void *handle;
{die(Eexist);/*NOTREACHED*/}
                        
#endif
