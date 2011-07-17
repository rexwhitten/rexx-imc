/*bin/true;exec rexx -x "$0" "$@";exit# This is a REXX program */
parse arg args
if args="" then args="/usr/games/fortune"/* default command */
args "| expand | rxstack"            /* stack the command's output */
if rc<>0 then exit rc                /* exit on error */
l=0                                  /* zero maximum length */
s=1e9                                /* minimum space on left */
do i=1 for queued()                  /* pull each line */
   parse pull line.i
   line.i=strip(translate(line.i,,xrange('00'x,'1f'x)||xrange('7f'x)),'T')
                                     /* translate all characters outside the
                                        range 0x20 - 0x7e into spaces.
                                        Remove trailing spaces. */
   l1=length(line.i)
   l2=spc(line.i)
   if l<l1 then l=l1                 /* keep maximum length */
   if s>l2 then s=l2                 /* keep minimum left space */
end
if s=1e9 then s=0                    /* just in case no output */
l=l-s
lines=copies("_",l+4)                /* top and bottom */
cent=33-l%2                          /* calculate the number */
if cent<1 then cent=""               /* of spaces before each */
else cent=copies(" ",cent)           /* line */
say cent lines                       /* output top */
say cent"|"copies(" ",l+4)"|"        /* output a blank line */
do j=1 to i-1
   say cent"|  "substr(line.j,s+1,l+2)"|"/* output each line of text */
end
say cent"|"lines"|"                  /* output bottom */
return
spc: /* return the number of spaces on the left */
parse arg t
if t='' then return 0
else return verify(t,' ')-1
