/* test program for interpreter */

signal on syntax
parse source sys how me nick env
parse version spec ver date
parse arg args

say "This is REXX test program "me", called"
say "from "sys" by "how" with name '"nick"'."
say "Commands are addressed to '"env"' by default."
say
say "Running "spec" language level "ver", last updated "date
say
say "Running loop tests, 10 times each"
do 10;sayn"*";end;say
do i=.1 to 1 by .1;sayn"*";end;say
do i=0 while i<10;sayn i;end;say
call time 'r'
say "Running loop speed test"
do 20000;end
say "Completed in time" time('r')
say "Running variable access speed test"
do i=1 to 1000;a=a||i;end
say "Completed in time" time('r')
say "Running compound variable speed test"
do i=1 to 500;a.i=a.(i-1)||i;end
say "Completed in time" time('r')
if right(a.500,9)^=="498499500"|right(a,10)^=='9989991000' then
   say "Error in variable tests"
say
say "Running statement tests"
nop
drop a;if a^=='A' then say "Error in drop test"
numeric digits 12
numeric fuzz 1
numeric form engineering
if 1/3^=="0.333333333333" then say "Error in numeric digits test"
if 1/3^=0.333333333331 then say "Error in numeric fuzz test"
numeric digits 2
if 100+1^=='100'|610*23^=='14E+3' then say "Error in numeric form test"
if 1 then do
   a=5
   if 0 then do
      a=1
   end
end
else a=6
if a^==5 then say "Error in if/do test"
a=''
do i=1
   if i=5 then iterate i
   a=a||i
   do j=i
      if j=10 then leave i
      else iterate i
   end j
end i
if a^=="1234678910" then say "Error in iterate/leave test"
a=0
select 5
   when 0 then a=1
   when 5 then nop
   otherwise a=1
end select
if a then say "Error in select test"
parse value "abcdefg hijklmn o","123 456 789" with a b,c '4' x +1 d 8 e
parse upper var a a 'F'
a= a^=="ABCDE"|b^=="hijklmn o"|c^=="123 "|x^=="4"|d^=="56"|e^==" 789"
parse value "1.hello there. 234  567" with 2 b +1 c d e (b) f g 1 (b) (b) h
b= b^=='.'|c^=='hello'|d^=='there'|e^==''|f^=='234'|g^==' 567'|h^==' 234  567'
if a|b then say "Error in parse test"

a=0
b=1
call test1
if result^=="result" then a=1
call test2
if result^=="RESULT" then a=1
if a|b then say "Error in call tests"
interpret "a=1";if ^a then say "Error in interpret test"
signal next
say "Error in signal test"
Next:
signal on novalue
j=novalue
say "Error in novalue test"
Novalue:
q=queued()
"echo 'echo test worked OK'|rxstack -lifo"
if queued()^==q+1 then say "Echo/stack test failed"
else do;parse pull q;say q;end

say "Running function tests"
numeric digits 9
numeric form scientific
if (0|0)(1|0)(0|1)(1|1)^==0111 then say "Error in OR test"
if 1+2*3**4^==163|'abc'^=' abc '|'abc'==' abc '|1<0|'a'<<'A'|,
   10//3^==1|10%3^==3
   then say "Arithmetic test failed"

if test1()^=='result' then say "Function call test failed"
if ^abbrev('information','info')|abbrev('INFORMATION','info') then
   say "Error in abbrev test"
if test3(1,2)^=='21' then say "error in arg test"
if c2x('A')c2d('A')b2d(01000001)d2c(65)d2x(65)x2c(41)x2d(41)^=="416565A41A65"
   then say "Error in base conversion test"
if centre(1,3,0)^=='010' then say "Error in centre test"
if compare(123,1234,4)|compare(1234,1235)^==4 then say "Error in compare test"
if copies("test",3)^=="testtesttest" then say "Error in copies test"
if ^datatype("abcABC123",'a')|datatype(".",'a')|,
   ^datatype("010101010",'b')|datatype("2","b")|,
   ^datatype("abcdefghi",'l')|datatype("A",'l')|,
   ^datatype("abcDEFGhi",'m')|datatype("2","m")|,
   ^datatype("1456.4E+4","n")|datatype("e","n")|,
   ^datatype("sf45_retf","s")|datatype("*","s")|,
   ^datatype("ABCDEFGHI","u")|datatype("a","u")|,
   ^datatype("123456554","w")|datatype("1.2","w")|,
   ^datatype("1 4e cd21","x")|datatype("r","x")
   then say "Error in datatype test"
say "Date test: "date('w') date()
if delstr("abcdefg",3,1)^=="abdefg" then say "Error in delstr test"
if delword("this is a test",2,1)^=="this a test" then say "Error in delword test"
if digits()^==9 then say "Error in digits test"
if errortext(41)^=="Bad arithmetic conversion" then say "Error in errortext test"
if form()^=="SCIENTIFIC" then say "Error in form test"
if format('1234567e5',,3,0)^=='123456700000.000'|,
   format('1.2345',,3,2,0)^== '1.235    '       |,
   format('12345.73',,,2,2)^=='1.234573E+04' then say "Error in format test"
if fuzz()^==0 then say "Error in fuzz test"
if insert("a","1234",2,2,"z")^=="12az34" then say "Error in insert test"
if justify("this is a test",16)^=="this  is a  test" then say "Error in justify test"
if lastpos(4,"98765432123456789")^==12 then say "Error in lastpos test"
if left("hi",4)^=='hi  '|left("there",2)^=="th" then say "Error in left test"
if length("hello")^==5 then say "Error in length test"
if max(4,2,6,5)^==6 then say "Error in max test"
if min(4,2,6,5)^==2 then say "Error in min test"
if overlay("1","abcdefg",3,2,0)^=="ab10efg" then say "Error in overlay test"
if pos("23","21234")^==3 then say "Error in pos test"
say "Random test:" random() random() random()
if reverse("1abcde2")^=="2edcba1" then say "Error in reverse test"
if right(1,2,0)^=="01"|right(123,1)^=="3" then say "Error in right test"
signal srctest
SrcTest:
if sourceline(sigl)^=="signal srctest" then say "Error in sourceline test"
if space("  hi    there",2,0)^=="hi00there" then say "Error in space test"
if strip("  hi  there  ","l")^=="hi  there  " then say "Error in strip test"
if substr("abc",2,4,"0")^=="bc00"|substr("abc",-1)^=="  abc"|,
   substr("12345",2,2)^=="23" then say "Error in substr test"
if subword(" this is  a   test ",2,2)^=="is  a" then say "Error in subword test"
if symbol("123f")^=="LIT"|symbol(123)^=="LIT"|,
   symbol("a")^=="VAR"|symbol("lit")^=="LIT" then say "Error in symbol test"
say "Time test:" time('c')
if trace()^=="F" then say "Error in trace test"
if translate('abc123DEF')^=='ABC123DEF'|,
   translate('abbc','&','b')^=='a&&c'|,
   translate('abcdef','12','ec')^=='ab2d1f' then say "Error in translate test"
if trunc(12.6)^==12|trunc(345e-2,1)^==3.4|trunc(26,5)^==26.00000 then
   say "Error in trunc test"
a=1;b.a=2
if value("b.a")^==2 then say "Error in value test"
if verify('123','1234567890')^==0|,
   verify('1Z3','1234567890')^==2|,
   verify('AB4T','1234567890','M')^==3|,
   verify('1P3Q4','1234567890',,3)^==4 then say "Error in verify test"
if word(" this  is a  test",2)^=="is" then say "Error in word test"
if wordindex("This is a test",2)^==6 then say "Error in wordindex test"
if wordlength("This  is a test",2)^==2 then say "Error in wordlength test"
if wordpos("is a","  this  is   a test")^==2 then say "Error in wordpos test"
if words(" this  is a  test")^==4 then say "Error in words test"
if xrange("a","g")^=="abcdefg" then say "Error in xrange test"
say "Function tests completed."
say "Testing I/O"
f="/tmp/testfile"
rc=stream(f,'c','open w')
if rc then say "Open of test file failed with "errortext(100+rc)
if lineout(f,"Testing, testing, 1, 2, 3") then
   say "lineout failed with" stream(f,'d')
rc=stream(f,'c','close')
if rc then say "Close of test file failed with" errortext(100+rc)
if linein(f)^="Testing, testing, 1, 2, 3" then
   say "linein failed with" stream(f,'d')
do i=1 to 10
   do j=1 to 20
      if charout(f,j) then say "charout filed with" stream(f,'d')
   end
   call lineout f,"<-"i
end
if linein(f)^="1234567891011121314151617181920<-1" then say "linein failed"
say "Line 5 is:" linein(f,6)
if lineout(f,"blah-blah",2) then say "lineout failed with" stream(f,'d')
call lineout f
call lineout f,"another test"
if charin(f,27,9)^="blah-blah" | linein(f,13)^="another test" then
   say "charin/linein failed"
call stream f,'c','close'

c="ls -alg" f
rc=stream('ls','c','popen r,'c)
if rc then say "popen failed with" errortext(100+rc)
parse value linein('ls') with . . . s1 s . . n1 n .
rc=stream('ls','c','pclose')
if rc then say "pclose gave rc="rc
if s1=390 then parse value s1 n1 with s n /* we are on HP-UX */
if s^=390|n^=f then say "pipe read failed"
call stream f,'c','open'
fd=stream(f,'c','fileno')
rc=stream(fd,'c','fdopen')
if rc then say "fdopen failed with" errortext(100+rc)
if linein(fd,3)^="011121314151617181920<-1" then
   say "linein from fd" fd "failed"
call stream fd,'c','close'
call stream f,'c','close'
say "I/O test completed"

exit

Test1:return 'result'
TEST2:procedure expose b
b=0
a=1
return
Test3:return arg()arg(1)

Syntax:
say left(sigl,5) "+++" sourceline(sigl)
say "Error" rc "running rexxtest.rexx, line" sigl":" errortext(rc)
trace ?r
say "Trace mode"
