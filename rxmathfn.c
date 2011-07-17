/* A sample function package for REXX/imc to provide the math functions */
/*                                                 (C) Ian Collier 1992 */

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>

#if defined(Solaris) && !defined(__STDC__)
#define const        /* Fix broken Solaris headers */
#endif

#include"const.h"
#include"functions.h"

/* Below is the dictionary which informs REXX what functions are available
   and where to find them */

int rxsin(),rxcos(),rxtan(),rxexp(),rxatn(),rxln(),rxpower(),rxasn(),
    rxacs(),rxsqr();

#ifdef sgi
/* The SGI doesn't seem to like the dictionary defined below because
   although the function references get relocated correctly, the
   names do not.  If anyone can fix this I would like to know...
   It probably involves specifying a particular flag on ld.
   Instead, here is an alternative dictionary which does work. */

char names[10][8]={"ACOS","ASIN","ATAN","COS","EXP","LN","SIN",
                   "SQRT","TAN","TOPOWER"};
dictionary rxdictionary[]=
{  names[0],rxacs,
   names[1],rxasn,
   names[2],rxatn,
   names[3],rxcos,
   names[4],rxexp,
   names[5],rxln,
   names[6],rxsin,
   names[7],rxsqr,
   names[8],rxtan,
   names[9],rxpower,
   0,       0
};

#else

dictionary rxdictionary[]=
{  "ACOS"   ,rxacs,
   "ASIN"   ,rxasn,
   "ATAN"   ,rxatn,
   "COS"    ,rxcos,
   "EXP"    ,rxexp,
   "LN"     ,rxln,
   "SIN"    ,rxsin,
   "SQRT"   ,rxsqr,
   "TAN"    ,rxtan,
   "TOPOWER",rxpower,
   0        ,0
};

#endif /* sgi */

/* Two utility functions not provided by the REXX calculator:
   getdouble(argc)   checks that argc==1 and then gets that argument from the
                     stack, returning it as a double.
   stackdouble(argc) stacks the given double on the calculator stack and
                     formats it according to the current settings */

double getdouble(argc)
int argc;
{
   char *num;
   char c;
   int len;
   double ans;
   if(argc!=1)die(Ecall);
   num=delete(&len);
   if(len<0)die(Ecall);
   
   /* now remove all spaces (which are legal in REXX) for sscanf */
   while(len&&num[0]==' ')num++,len--;
   if(num[0]=='+'||num[0]=='-')
      while(len>1&&num[1]==' ')num[1]=num[0],num++,len--;
   while(len&&num[len-1]==' ')len--;

   /* Now the number is converted to double with sscanf */
   num[len]=0;
   if(sscanf(num,"%lf%c",&ans,&c)!=1)die(Enum);
   if(ans==HUGE_VAL)die(Eoflow);
   return ans;
}

void stackdouble(num)
double num;
{  /* the number is simply sprintf-ed, stacked, and formatted. */
   void rxformat();    /* the REXX format() function */
   char result[25];
   sprintf(result,"%.18G",num);
   stack(result,strlen(result));
   rxformat(1);
}

/* The following functions implement the REXX math functions using
   floating-point math. Some functions perform range checking first. */
   
int rxsin(name,argc)
char *name;
int argc;
{
   stackdouble(sin(getdouble(argc)));
   return 1;
}
int rxcos(name,argc)
char *name;
int argc;
{
   stackdouble(cos(getdouble(argc)));
   return 1;
}
int rxtan(name,argc)
char *name;
int argc;
{
   double arg=getdouble(argc);
   double s=sin(arg);
   double c=cos(arg);
   if(c==0.)return -Edivide;
   stackdouble(s/c);
   return 1;
}
int rxasn(name,argc)
char *name;
int argc;
{
   double arg=getdouble(argc);
   if(arg>1.||arg<-1.)return -Ecall;
   stackdouble(asin(arg));
   return 1;
}
int rxacs(name,argc)
char *name;
int argc;
{
   double arg=getdouble(argc);
   if(arg>1.||arg<-1.)return -Ecall;
   stackdouble(acos(arg));
   return 1;
}
int rxatn(name,argc)
char *name;
int argc;
{
   stackdouble(atan(getdouble(argc)));
   return 1;
}
int rxexp(name,argc)
char *name;
int argc;
{
   double val=exp(getdouble(argc));
   if(val==HUGE_VAL)return -Eoflow;
   stackdouble(val);
   return 1;
}
int rxln(name,argc)
char *name;
int argc;
{
   double arg=getdouble(argc);
   if(arg<=0.)return -Ecall;
   arg=log(arg);
   if(arg==HUGE_VAL)return -Eoflow;
   stackdouble(arg);
   return 1;
}
int rxpower(name,argc)
char *name;
int argc;
{
   double arg1,arg2;
   if(argc!=2)return -Ecall;
   arg2=getdouble(1);
   arg1=getdouble(1);
   if(arg2==0.){        /* Firstly, anything to the power 0 is 1. */
      stack("1",1);
      return 1;
   }
   if(arg1==0.&&arg2>0.){  /* Also, 0 to any positive power is 0. */
      stack("0",1);
      return 1;
   }                    /* Otherwise return x**y = exp(y log x) */
   if(arg1<0.)return -Ecall;  
   arg1=exp(arg2*log(arg1));
   if(arg1==HUGE_VAL)return -Eoflow;
   stackdouble(arg1);
   return 1;
}

/* Two utility functions for the square root program below.  They provide
   add and subtract functions which are quicker than going via REXX's
   calculator. */
   
static int add(num1,len1,num2,len2)  /* add num 2 to num 1. */
char *num1,*num2;                    /* Return the carry */
int len1,len2;
{
   int i;
   char c=0;
   num1+=len1;
   num2+=len2;
   for(i=len2;i--;){
      *--num1+=*--num2+c-'0';
      if(c=(*num1>'9'))*num1-=10;
   }
   for(i=len1-len2;c&&i--;)
      if(c=((++*--num1)>'9'))*num1-=10;
   if(c)*--num1='1';
   return c;
}

static void sub(num1,len1,num2,len2)  /* subtract num2 from num1. */
char *num1,*num2;
int len1,len2;
{
   int i;
   char c=0;
   num1+=len1;
   num2+=len2;
   for(i=len2;i--;){
      *--num1-=*--num2+c-'0';
      if(c=(*num1<'0'))*num1+=10;
   }
   for(i=len1-len2;c&&i--;)
     if(c=((--*--num1)<'0'))*num1+=10;
}

int rxsqr2(name,argc)  /* Quick square root evaluator, for "low" precision */
char *name;
int argc;
{
   double arg=getdouble(argc);
   if(arg<0.)return -Ecall;
   stackdouble(sqrt(arg));
   return 1;
}

/* The following function implements the REXX square root function.
   If the number of digits required is low enough, the floating-point
   method (above) is used.  Otherwise, a digit-by-digit method is used. */
   
int rxsqr(name,argc)
char *name;
int argc;
{
   int n,m,e,l,z;
   char x;
   char *arg,*ans,*residue,*string,*tmp1,*tmp2;
   int rlen,t1len,t2len;
   extern int precision;  /* Extract the precision and the workspace from */
   extern char *workptr;  /* the REXX calculator */
   extern int eworkptr,worklen;
   if(precision<=16)return rxsqr2(name,argc);
   /* now the digit by digit algorithm for greater precision than FP math */
   eworkptr=1;  /* Clear the work space, but allow for one digit of left-
                   overflow (to be put at workptr[0]) */
   if((n=num(&m,&e,&z,&l))<0)return -Enum;  
   if(m)return -Ecall;
   delete(&m);
   if(z){       /* the square root of zero is zero. */
      stack("0",1);
      return 1;
   }
   /* workptr[n] is now the first in a sequence of digits of a positive
      number.  The decimal point belongs after the first digit.  The
      following code moves the decimal point to after the second digit,
      and at the same time makes the exponent even. */
   if(e%2)e--;
   else workptr[--n]='0', /* (add leading 0) */
        l++;
   /* Expand the workspace to hold all the temporary material */
   mtest(workptr,worklen,n+l+6*precision+20,n+l+6*precision+20-worklen);
   arg=workptr+n;
   if(l%2)arg[l++]='0'; /* make the length even by adding a trailing 0 */
   ans=arg+l;
   residue=ans+precision+1;
   rlen=0;
   string=residue+precision+precision+2;
   tmp1=string+precision+2;
   tmp2=tmp1+precision+3;
   string[0]='0';
   /* Now we have five numbers: ans, residue, string, tmp1, tmp2, all of
      ample length, in the workspace as well as the argument. */
      
   for(n=0;n<=precision;n++){
      /* invariants: length of arg used so far = 2n
                     length of ans = n
		     length of string = n+1
		     length of residue = rlen <= 2n       */
		     
      if(n+n<l)residue[rlen++]=arg[n+n],  /* Bring down two more digits */
               residue[rlen++]=arg[n+n+1];/* from the argument to residue */
      else residue[rlen]=residue[rlen+1]='0',
           rlen+=2;
	   
      /* find least x s.t. (string||x)*x>residue. Find (string||x-1)*(x-1). */
      /* Invariant wil be: tmp1=(string||x-1)*(x-1) and tmp2=(string||x)*x  */
      memcpy(tmp2,string,n+1);
      tmp2[n+1]='1';
      memset(tmp1,'0',t1len=t2len=n+2);
      x='1';
      /* Remove some leading zeros from the residue */
      while(rlen>t2len&&residue[0]=='0')residue++,rlen--;
      /* Below says "while(tmp2<=residue)" */
      while(t2len<rlen||(t2len==rlen&&memcmp(tmp2,residue,t2len)<=0)){
         memcpy(tmp1,tmp2,t1len=t2len);
	 string[n+1]=x;
	 if(add(tmp2,t2len,string,n+2))tmp2--,t2len++;
	 x++;
	 if(add(tmp2,t2len,&x,1))tmp2--,t2len++;
      }
      if(t2len>n+2)tmp2++; /* Restore tmp2 pointer to its original value */
      
      /* Append x-1 to the answer; subtract (string||x-1)*(x-1) from residue;
         append 2(x-1) to the string. */
      ans[n]=--x;
      sub(residue,rlen,tmp1,t1len);
      string[n+1]=x+x-'0';
      if(x>='5')string[n+1]-=10,string[n]++;
   }
   stacknum(ans,n,e/2,0); /* We have the answer! */
   return 1;
}

