/* small shell program. */
trace off
signal on halt
signal on syntax
address command
home=value("HOME",,"ENVIRONMENT")
startup=home'/.shellrc'
address command 'test -f' startup
if rc=0 then address unix 'rxstack < 'startup
alias.=""
noargs.=0
hostname='/bin/hostname'()
rc=0
n=1
t=0
hist=20
restart:
do i=n
   qu=queued()
   if qu=0 then do
      r="Ready"
      if rc then r=r "("rc")"
      if t>0 then r=r"; T="format(t,,2)
      say r
      call charout ,hostname":"i">"
   end
   t=0
   parse pull command 1 word $@ 1 $0 $1 $2 $3 $4 $5 $6 $7 $8 $9
   echo=left(word,1)\=='@'
   if \echo then parse var command 2 command 2 word $@ 2,
      $0 $1 $2 $3 $4 $5 $6 $7 $8 $9
   if qu>0&echo then say hostname":"i">"command
   if alias.word\=="" then
      if noargs.word then command=alias.word
      else command=alias.word $@
   if command="??" then do
      do l=max(1,i-hist) to i-1
         say right(l,4)':'line.(l//hist)
      end
      i=i-1
      iterate i
   end
   if left(command,1)="!" then do
      what=substr(command,2)
      if \datatype(what,'W') then do
         do l=i-1 to max(1,i-hist) by -1
            if abbrev(line.(l//hist),what,1) then leave l
         end
         what=l
      end
      if what<i-hist|what<1|what>=i then do
         say "Event not found"
         i=i-1
         iterate i
      end
      command=line.(what//hist)
      say command
   end  /* line 58!! */
   line.(i//hist)=command
   parse var command c t1 t2 1 . tt
   c=translate(c)
   select
      when c='EXIT' then do
         say 'Exiting rexx shell'
         if t1="" then exit
         else exit t1
      end
      when c='SETENV' then rc=value(t1,t2,"ENVIRONMENT")
      when c='SAY' then interpret 'say' tt
      when c=';' then do
         call time 'R'
	 interpret tt
	 t=time('E')
      end 
      when c='CD' then do
         rc=chdir(tt)
	 if rc\=0 then say tt':' ioerr(rc)
      end
      when c='ALIAS' then do
         if t2="" then say alias.t1
	 else do
            alias.t1=t2
	    if left($2,1)==';' then noargs.t1=1
	 end
      end
      otherwise
         call time 'R'
         command
	 t=time('E')
   end
end

halt:
signal on halt
say "Interrupted!"
n=i
if sigl>58 then n=n+1
signal restart

syntax:
signal on syntax
say right(sigl,5) "+++" errortext(rc)
n=i
if sigl>58 then n=n+1
signal restart
