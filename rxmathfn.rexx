/* Example Rexx program to provide mathematical functions */
trace off

Ecall=40 /* Incorrect call to routine */
Enum=41  /* Bad arithmetic conversion */
Eundef=43   /* Routine not found */

parse upper source . . . what ./* Must have what equalling the function name */
if Arg()<>1+(what="TOPOWER") then do
   say what':'errortext(Ecall)
   return   /* If an error occurs, return with no value causes the caller */
end         /* to be flagged (with "Function did not return result")      */

parse arg in .,in2 .
if \datatype(in,'n') then do
   say what':'errortext(Enum)
   return
end

d=digits()
f=fuzz()

if d<20 then d1=d;else d1=19
lim=10**-d1/20
numeric digits d1+2
pi=3.1415926535897932384626433
halfpi=pi/2
twopi=pi+pi
log10=2.3025850929940456840179910
log2=0.6931471805599453094172321
log10two=0.30102999566398119521373889

if d<20 then d1=d;else d1=19
lim=(1'E-'d1)/20

select
   when what="SIN" then answer = sin(in)
   when what="COS" then answer = sin(in+halfpi)
   when what="TAN" then answer = sin(in)/sin(in+halfpi)
   when what="EXP" then answer = exp(in)
   when what="ATAN" then answer = atn(in)
   when what="LN" then answer = ln(in)
   when what="TOPOWER" then do
         if in2=0 then answer = 1
         else if in=0 & in2>0 then answer = 0
         else answer = exp(ln(in)*in2)
   end
   when what="ASIN" then answer = 2*atn(in/(1+sqr1(1-in*in)))
   when what="ACOS" then answer = halfpi-2*atn(in/(1+sqr1(1-in*in)))
   when what="SQRT" then return sqr(in)
   otherwise say what':'errortext(Eundef) ; return
end
numeric digits d1
answer=format(answer)
return answer

series:     /* Calculates a Chebyshev series with the given coefficients, */
n=arg()-1   /* evaluated at the given argument.  arg(1) is the argument, and */
parse arg z /* all the other arg() are the series coefficients */
z=z+z;m2=0;t=0
do i=n+1 to 2 by -1
   m1=m2
   u=t*z-m2+arg(i)
   m2=t
   t=u
end
return t-m1

sin: parse arg x
/* reduce range of x */
x=(x/halfpi+2)//4-2
if x<-1 then x=-x-2
if x>1 then x=2-x

return series(2*x*x-1,1.27627896240226588021,-0.14263078459051800478,,
   0.00455900800332590124,-0.00006829375677098333,0.00000059248092883084,,
   -0.00000000335139580191,0.00000000001333639299,-0.00000000000003936461,,
   0.00000000000000008961,-0.00000000000000000016),
 *x

exp: parse arg x
x=x/log10
e=trunc(x)
x=x-e
if x<0 then do;x=x+1;e=e-1;end
/* now have answer = 10**x * 10**e where 0<=x<1 and e is an integer */
return series(x+x-1,4.30022915484994695129,2.13908204064962629380,,
     0.58426304848002648400,0.10914429717084543695,0.01545385656994400473,,
     0.00175990305162950212,0.00016753288979627516,0.00001369642180963234,,
     0.00000098103821148018,0.00000006251839260577,0.00000000358806613888,,
     0.00000000018729961298,0.00000000000896585002,0.00000000000039629176,,
     0.00000000000001626892,0.00000000000000062348,0.00000000000000002240,,
     0.00000000000000000075,0.00000000000000000002)'E'e


atn: parse arg x
select
   when x<-1 then do;w=-halfpi;y=-1/x;end
   when x>1 then do;w=halfpi;y=-1/x;end
   otherwise w=0;y=x
end
/* now -1<=y<=1 and atn x=w+atn y */
return series(2*y*y-1,0.88137358701954302523,-0.05294646227335292762,,
   0.00556792102970276495,-0.00069059750180019885,0.00009287148663927100,,
   -0.00001310759805636663,0.00000191051829724472,-0.00000028495930832883,,
   0.00000004324438932225,-0.00000000665169198781,0.00000000103425288179,,
   -0.00000000016224319604,0.00000000002563983127,-0.00000000000407739378,,
   0.00000000000065190359,-0.00000000000010471489,0.00000000000001688910,,
   -0.00000000000000273382,0.00000000000000044394,-0.00000000000000007229,,
   0.00000000000000001180,-0.00000000000000000193,0.00000000000000000031,,
   -0.00000000000000000005)*y+w

ln: parse arg x
if x<=0 then do
   say 'ln('||x'):'errortext(Ecall)
   return
end
if x=1 then return 0
if x<1 then do;s='-';x=1/x;end
else s=''
numeric form scientific
parse value format(x,1,,,0) with . 'E' exp
if exp='' then exp=0
e=trunc((1+exp)/log10two-3)
/* x/(2**e) is now between 1 and 16 */
x=x/2**e
do while x>1.6
   x=x/2
   e=e+1
end
ans=series(2.5*x-3,0.93022922133637741858,-0.08184145665572451963,
   ,0.00947661158839661302,-0.00122828373982062934,0.00016939529033498945,
   ,-0.00002430127203947124,0.00000358276167843137,-0.00000053890849196729,
   ,0.00000008231521903733,-0.00000001272671619709,0.00000000198712378075,
   ,-0.00000000031280003537,0.00000000004957693767,-0.00000000000790361608,
   ,0.00000000000126636133,-0.00000000000020379548,0.00000000000003292356,
   ,-0.00000000000000533708,0.00000000000000086781,-0.00000000000000014149,
   ,0.00000000000000002312,-0.00000000000000000378,0.00000000000000000062,
   ,-0.00000000000000000010,0.00000000000000000001)*(x-1)+e*log2
return s||ans

sqr1: /* just return exp(ln(x)/2) unless x=0 */
parse arg z
if z=0 then return 0
return exp(ln(z)/2)

sqr2: /* use digit-by-digit algorithm to find sqr(x) to required number of
         places.  This routine is no longer in use. */
parse arg a
numeric digits d+1
if a=0 then b=0
else if a<0 then do
   say "sqrt("a"):"errortext(Ecall)
   return
end
else do
   numeric form scientific
   parse value format(a,1,,,0) with before '.' after 'E' e
   if pos('E',before)>0 then parse var before before 'E' e
   a=before||after  /* a is now pointless */
   if e='' then e=0 /* e is the exponent  */
   if e//2 \= 0 then e=e-1
   else a='0'a      /* exponent is odd, and decimal comes after 2 digits */
   b='';b1=0;c=0
   numeric digits d%2+2 /* avoid unnecessary work - we only need this precision*/
   do i=1 by 2 until length(b)=d+1
      b1=trunc(b1)||substr(a,i,2,0)
      /* find least x s.t. (c||x)*x>b1. Find (c||x-1)*(x-1). */
      d2=c||1
      d1=0
      x=1
      do while d2<=b1
         d1=d2
         d2=d2+(c||x)
         x=x+1
         d2=d2+x
      end
      x=x-1
      b=b||x
      b1=b1-d1
      c=trunc((c||x)+x) /* never in exponential form */
   end
   b=left(b,1)'.'substr(b,2)'E'e/2
end
numeric digits d
return format(b)

sqr: /* Use Newton's method to get square root to required number of places */
parse arg x
if x=0 then return 0
if x<0 then do
   say "sqrt("||x"):"errortext(Ecall)
   return
end
numeric form scientific
parse value format(x,1,1,,0) with n +1 'E' exp /* error if x<0 */
if exp='' then exp=0
r=n/2||'E'||(exp+1)%2 /* divide both mantissa and exponent by 2 for initial
                         estimate */
d1=5
numeric digits d1+2
numeric fuzz 1
do until r = r1       /* Get estimate accurate up to d1 digits */
   parse value r (x/r+r)/2  with r1 r
end
do while d1<d
   d1=min(d1+d1,d)    /* Double up until required accuracy is reached */
   numeric digits d1+2
   parse value r (x/r+r)/2  with r1 r
end
numeric digits d
return format(r)

