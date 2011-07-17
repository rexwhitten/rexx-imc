                         Installation of REXX/imc

Note: The installation procedure for REXX/imc >= 1.6d is different from that
of previous versions.  It was written in collaboration with Pierre Fortin.
We hope that the new installation procedure is simpler than the old one.

In general, if you just type "Make" (capital M and without the quotes) on
one of the platforms supported by REXX/imc then you should get a working
copy of REXX/imc which you can test (note that if "." isn't on your path
then you will have to type "./Make" instead of "Make").  If you type
"Make install" then REXX/imc will be installed in a system directory.  A
list of tested platforms with some notes about one or two irregularities
discovered can be found in README.platforms.  Please check this to see
whether there is anything you should be aware of.

Of course, things are rarely as simple as they are supposed to be, so a
few notes follow.  You should probably scan through them before starting
(especially the first one).  But before the notes, here are some tests that
you can try to see whether REXX/imc is working properly.

 (a) rexx -v

     The simplest test, which just tells you whether the program was
     successfully compiled.  It should give you the version info.  If
     you have problems at this stage, see the note below about "librexx"
     files.  Note that if "." isn't on your PATH then you will have to say
     "./rexx" instead of just "rexx".

 (b) rexx rexxcps

     This program by Mike Cowlishaw tests most of the main control structures
     of Rexx and in addition tells you how fast it is running.

 (c) rexx rexxtest

     This runs a cursory check on most of the REXX/imc functions to make sure
     all is OK.

 (d) rexx -s 'numeric digits 25;say sqrt(2)'

     This will check that the math library is working OK.  You may need to
     set the environment variable REXXLIB to "." first so that the just-
     compiled library can be found.  If the rxmathfn.rxlib file from the
     source package is not in the current directory, make a copy of it.
     If the math library was not compiled then set REXXLIB to point to
     the source directory and the supplied Rexx program should provide
     the answer.  The expected answer is 1.414213562373095048801689.

====================
Notes on Compilation
====================

Contents:  1. Which compiler and other configuration options
           2. Final locations of object files
           3. The librexx library files
           4. Removing the object files

1. Which compiler and other configuration options

   There are a few variables at the top of the Make program which you can
   configure.  One of these is CC, which gives the name of the compiler.
   In general it is probably best to use the Gnu compiler gcc if you have
   it, except on HP-UX, IRIX and OSF1 platforms, where cc is better.  HP-UX
   also has c89 (apparently).  Note that the program does not always know
   the correct options for the compiler you choose and might have to guess
   (you will get a warning in that case).

   REXX/imc allows you to do the compilation in a different directory from
   where the source is kept.  If you do this then change SRC to point to the
   directory containing the source.  Note that there are some systems on
   which this doesn't work.

   Any general flags that should be passed to the C compiler during the
   compile or link phase can be put in COMPILEFLAG or LINKFLAG respectively
   (this is not usually necessary).

   When REXX/imc starts it usually adds ".rexx" to the filename.  This
   can be changed to a different extension (such as ".exec" or ".cmd")
   at run time with the REXXEXT environment variable, or at compile
   time by defining the filetype macro (e.g., using the compile flag
   -Dfiletype=\\\".cmd\\\" - if you type this on the shell command line you
   will need an extra pair of single quotes around the whole thing).

   If the variable STUFF is left undefined (or blank) then stacked lines
   left on the Rexx queue will be ignored when the Rexx program finishes
   (the stack server will report this on the screen when it closes down).
   If it is set to "-DSTUFF_STACK" then the interpreter will attempt
   to stuff all remaining stacked lines into the terminal buffer, thus
   mimicking the CMS behaviour.  This works on SunOS and Solaris - your
   mileage may vary.

   If SMALL is set to false then normal executables will be created.  If
   it is set to true then the "-n" option will be passed to the linker to
   prevent it from adding padding, resulting in smaller executables.  This
   is obsoleted by ELF-format executables as found in many modern systems.
   It works on SunOS - your milage may vary.

   If you prefer, any of the above variables can be altered without editing
   the program by specifying them immediately after the word Make on the
   command line, for example, "Make CC=gcc SMALL=false".

2. Final locations of object files

   Unless you are performing an install operation, all files created during
   the compilation process will remain in the current directory (they are
   not moved around as in previous versions of the installation procedure).

   If you are installing then the binaries will go into one directory and
   the library files will go into another.  The binaries are: rexx (the
   main program); rxque (the stack server program) and rxstack (the stack
   access program).  The library file will be a file whose name starts
   with "librexx" (see a later section for details).  The Rexx library
   files, which will usually be put with the librexx file, are: either
   rxmathfn.rxfn or rxmathfn.rexx (the math function package - which version
   depends on whether shared objects are supported), and rxmathfn.rxlib (the
   math function dictionary).  The destination of the Rexx library files
   may be specified separately by setting the REXXLIB variable. 

   By default, the binaries will go into $PREFIX/bin and the library will
   go into $PREFIX/lib, where $PREFIX is usually /usr/local/bin.  Manual
   pages will be installed in $PREFIX/man/man/man1.  On Solaris systems
   the $PREFIX will be /opt/REXXimc.  You can change the prefix by setting
   the environment variable PREFIX or by putting a string of the form
   PREFIX=/another/directory in the arguments to Make (see below for Make
   arguments).

   Alternatively, if you set BINDIR, LIBDIR and MANDIR then these will be
   used as the binary, library and manual page directories, respectively.

   The object file interface.o usually contains a record of the binaries
   directory.  This is not essential but can help to speed the startup
   process of REXX/imc.  Therefore it helps if the BINDIR is correct even
   if you are not immediately installing REXX/imc.  It is possible to
   record a directory other than BINDIR in interface.o by setting the
   variable REXXIMC (either in the environment or in the arguments to Make).

   The same object file also contains a default REXXLIB string, which
   will be taken from the REXXLIB variable if you set it, or the library
   directory as detailed above.  REXXLIB could be set differently during
   compilation and during installation to record a path different from the
   one in which the library is installed.  The value during compilation can
   be a colon-separated path, or the special value "" which means the
   libraries are in the binaries directory (as in earlier versions of
   REXX/imc).

3. The librexx library files

   The library file which is produced will be either librexx.a or a file
   whose name starts with "librexx.so".  On some systems there will also
   be a symbolic link whose full name is "librexx.so".  This depends on
   whether or not your system supports dynamic shared libraries (and, more
   importantly, whether I have found out how to create them!).

   The librexx.a file is a static library archive.  It is not necessary in
   order to run REXX/imc and could be deleted.  The librexx.so file is a
   dynamic shared library.  It is necessary in order to run REXX/imc.
   However, even if the library is not required by REXX/imc it may still
   be useful because it is needed in order to compile any application that
   uses the REXX/imc programming interface.  You can even make a copy of
   librexx.a on a system that supports dynamic libraries if you like by
   typing "Make librexx.a".

   NOTE: Some systems are more fussy than others on the location of the
   dynamic shared library.  SunOS 4 appears to record the location of the
   library so that the executable always works as long as you don't move
   the library; the Make program also contains code for OSF1, Linux and
   Solaris (SunOS 5) to make the compiler record the location of the library
   in a similar manner.  However, if you get a message such as "ld.so:
   librexx.so.1: not found" or "librexx.so.1.7: Undefined error: 0" (that
   gem from FreeBSD) then this means that the system doesn't know where the
   library file is.  If you are just testing then setting the environment
   variable LD_LIBRARY_PATH to point to the library directory usually works,
   but this is not particularly good as a permanent solution.  If you can
   run ldconfig (sometimes installed in /sbin - see system manual page)
   then this should fix the problem; otherwise you may have to install the
   library in a system directory.  On FreeBSD you can cure the problem by
   finding the line containing the string "Uncomment above line" in the Make
   program and obeying it before doing the final install.

4. Removing the object files

   Once you have installed REXX/imc, you may type "Make clean" which removes
   any compiled programs or object files.

   If you want to delete REXX/imc from the system directories, you can type
   "Make uninstall" (obviously, the directories have to be set correctly
   as in (2) above).

=============
Fun with Make
=============

The full syntax of Make is:

   Make [VAR=value [...]] [v] [letter] [target [...]].

The parameters must appear in this order, though there is no set order for
individual variables or targets.  Explanations of these parameters follow.

   VAR=value
       sets the variable VAR to the given value.  Variables which can be
       set in this manner include SRC, CC, COMPILEFLAG, LINKFLAG, STUFF,
       SMALL, PREFIX, BINDIR, LIBDIR, MANDIR, REXXLIB and REXXIMC, all of
       which are described in previous sections; MAKE and LD, which name the
       programs make and ld respectively, and MAKEFILE which names the file
       Makefile.REXXimc.

   v (also -v)
       Sets "verbose" mode.  This just prints out the name of each rule
       which is followed in the Makefile as compilation progresses.

   letter
       One of the following letters or "pg" which modifies the
       "optimisation" flags for compilation.  These are mostly used
       for debugging.  By default the programs will be optimised and
       stripped.

       a  uses cc with the "-a" flag to compile a version of REXX/imc which
          will output profiling statistics when executed.
       d  uses the "-g" compilation flag and also defines the DEBUG
          preprocessor macro in order to make a debug version of REXX/imc
          with memory tracing.
       g  uses the "-g" compilation flag to produce an ordinary debug
          version of REXX/imc.
       n  uses no compiler flags at all, which reduces compilation time at
          the cost of making REXX/imc slightly slower.  Does not strip the
          executable.
       o  uses optimisation flags to produce faster, smaller executables.
       p  uses the "-p" compiler flag to compile a version of REXX/imc which
          will output profiling statistics for "prof" when executed.
       pg uses the "-pg" compiler flag to compile a version of REXX/
          imc which will output profiling statistics for "gprof" when
          executed.

   target
       One of the following things to make.  You can specify several
       targets, but any (un)installation option should be specified by
       itself.  The "test" target also only works by itself.

       all (same as leaving out the target): compile all the binaries (in
              the current directory), namely rexx, rxque, rxstack and, if it
              is supported, the math library.  Compiling rexx also involves
              compiling an appropriate librexx file.
       test:  a dummy run - the same as "Make all" but the actions are
              printed out instead of executed.
       check: print out the values of important parameters.
       install: Compile and install all the binaries and manual pages.
       install-pgm: Compile and install the main programs and librexx file.
       install-fn: Compile and install the math function library.
       install-man: Install the manual pages.
       uninstall: Remove all Rexx-related files from system directories.
       uninstall-pgm: Remove the programs and librexx file from system
              directories.
       uninstall-fn: Remove the math function library from the system
              directory.
       uninstall-man: Remove the manual pages from the system directory.
       clean: Remove all compiled objects from the current directory.
       rexx:  Compile just the library "librexx" and the program "rexx".
       rxque: Compile just the program "rxque".
       rxstack: Compile just the program "rxstack".
       rexx.1: Make the necessary substitution in the rexx manual page.
       librexx.a: Compile the static libary archive.
       librexx.so: Compile the dynamic shared library (the target is
              called librexx.so even if the file is actually called
              librexx.so.1.7).  This only works if shared libraries
              are supported on your platform.
       math:  Compile the math function library (only works if it is
              supported on your platform).
       lint:  Run the REXX/imc source through lint (only on systems which
              have lint).

       The name of any "*.o" file may also be supplied as a target.

================================
Configuring Make for your system
================================

All the variables that need changing are set in the Make program for
flexibility - this allows the program to decide on the compiler flags
depending on the operating system.  In order to add or change the values
for your system, follow the format of the second half of the Make program,
which is as follows.

   SYSTEM)
       the various settings that apply to all
       compilers on that system
       case $CC in
          COMPILER)
              the various settings that
              apply to this compiler
              ;;
          COMPILER)
              some more settings
              for another compiler
              ;;
          *)  what happens if the compiler
              is unknown
              ;;
       esac
       ;;

The string you use for SYSTEM will be the output of the command "uname -s"
followed by a colon and a version number (the output of "uname -r").  A star
usually suffices for the version number, meaning "all versions".  Examples
from the Make program are "SunOS:5.*" and "OSF1:*".

The important variables that you can examine or set are as follows.

   CCFLAG
       Contains all the necessary flags and defines for compiling.  If
       your machine does not have signed characters by default then you
       will need to insert a flag for this ("-fsigned-char" works on gcc).
       The various preprocessor symbols you can define are described later
       on.
   DEBUG
       If debugging was turned on by the "letter" option "d" then this
       contains the names of any special object files which may be linked
       with the executables.
   DLLFLAG
       If your machine supports dynamic shared libraries then this variable
       contains flags which must be passed to ld in order to make the
       library.
   LDL This will usually contain the string "-ldl", but if function
       libraries have been disabled (because of certain debug options)
       it will be blank.  This just exists to help you write the list of
       required libraries.  The -ldl library is required on SunOS but not
       on some other systems.
   LIBRARIES
       This contains the list of libraries required when linking the
       programs.  Possible libraries that may be required are -ldl (for
       loading shared objects - see also LDL), -lbsd (for BSD-compatible
       functions on systems such as AIX), -lsocket and so on.
   LREXX
       Usually contains the string "-lrexx" for linking with the librexx
       file, but if it helps you could change this to the absolute path
       name of the library, namely "$(LIBDIR)/$(SONAME)" (SONAME is set
       in the Makefile to the name of the dynamic library).
   MATH
       If your machine supports shared objects which can be loaded
       by a running program (using the dlopen(3) function) then this
       variable should contain the name of the math function library,
       namely "rxmathfn.rxfn".  Otherwise it should remain blank, and
       the Rexx program rxmathfn.rexx will be installed instead.
   OPTFLAG
       This contains the flags which were chosen by the "letter" option of
       Make, which will be passed to the compiler during both compilation
       and linking stages.
   PIC If you will be using a dynamic shared library then it may have to
       be compiled using position-independent code.  If that is so then
       PIC should contain a flag which makes the compiler produce such
       code.
   RANLIB
       Contains "ranlib" if you need to run ranlib on a library archive
       after it is created, and "@true" otherwise.
   REXXLIB
       If you will be using a dynamic shared library (and the system
       supports it) then this should contain "librexx.so".  Otherwise it
       should contain "librexx.a".
   RUNLIBS
       The contents of this variable are appended to the command line when
       linking programs.  This is used on Solaris for adding a library
       directory.
   SOFLAG
       If your machine supports shared objects which can be loaded by a
       running program (using the dlopen(3) function) then this variable
       contains flags which must be passed to ld in order to make the
       shared object.
   SOLINK
       If you will be using a dynamic shared library then it will be
       called something like librexx.so.1.6.4 (even though REXXLIB
       equals "librexx.so").  Some systems require a symbolic link called
       librexx.so to point to this.  If it is required on your system then
       let SOLINK="librexx.so", otherwise leave it blank.
   STRIPFLAG
       This usually contains either nothing or "-s" to strip the executables.
       It will have been set by the "letter" option of Make.
   USEDLLFLAG
       Usually blank, but sometimes contains some flags which are given to
       the compiler when the rexx executable is being linked in order to
       make sure the dynamic library is linked correctly.

The definitions of some preprocessor symbols that you can put in the CCFLAG
variable are as follows.

   _REQUIRED
       The symbol name that is passed to the dlsym(3) function requires
       a "_" before it (see rexx.c).  If this is incorrectly defined then
       trying to call a math function results in a message such as "Routine
       not found: SQRT in file rxmathfn.rxfn".
   DECLARE_RANDOM
       Used on Sun systems: the random() function is not declared in
       system headers so the Rexx source files should define it.
   DECLARE_TIMEZONE
       Used on Sun systems: the timezone external variable is not
       declared in system headers so the Rexx source files should
       define it.
   FSTAT_FOR_CHARS
       Do not trust the FIONREAD ioctl for finding the number of unread
       characters in a regular file, but instead call fstat to find the
       file's size.  Note that FIONREAD is still used for finding the
       number of unread characters in a tty or other special file.
   HAS_GMTOFF
       Used on SunOS 4 and Digital Unix: the tm structure has a tm_gmtoff
       element which contains the offset from GMT.
   HAS_MALLOPT
       The malloc library has the function mallopt() as in SunOS.  This is
       by no means essential.
   HAS_TTYCOM
       The system requires <sys/ttycom.h> to be included before the
       TIOCGWINSZ ioctl can be used, as in SunOS.  If this is not defined
       then <termios.h> will be included instead.
   MALLOC_DEBUG
       If DEBUG is set and the function "malloc_debug()" is provided on your
       system then defining this will allow the program to use it.
   NO_CNT 
       Do not use the hack of inspecting fp->_cnt (where fp is a FILE
       pointer) to find the number of characters read but not returned
       to the program.  If this symbol is defined then the chars() call
       may not work correctly on special files (for regular files you can
       define FSTAT_FOR_CHARS instead).  However, most systems do have a
       way of calculating this number.  If the method exists but requires
       something other than inspecting fp->_cnt, then change the _CNT
       macro in rxfn.c accordingly.  Suitable definitions have already
       been included for Linux and FreeBSD.
   NO_LDL       
       The system does not have the functions dlopen(), dlsym(), dlclose(),
       dlerror() for dynamic loading.  The effect of defining NO_LDL will
       be to remove support for external compiled functions.  If you define
       this then leave MATH blank (see above) so that the math function
       library will not be compiled.
   POINTER64   
       Insert extra padding into various structures in order to align for
       a system with 64-bit pointers (it does no harm to define this on a
       machine with short pointers; it just makes some of the structures
       use a few bytes more memory).

If you define NO_LDL, you will not be able to use compiled function
packages, such as rxmathfn.rxfn (the math library).  However, it will
still be possible to use external functions written in Rexx.  You will be
able to use the Rexx math library by copying the Rexx file rxmathfn.rexx
into the binaries directory together with rxmathfn.rxlib.  This will be
done automatically by "Make install".

It appears that a dlfcn package containing dlopen(), dlsym() and dlclose()
functions has been written for AIX.  One of the locations where it may be
found is at <ftp://ftp.rzg.mpg.de/pub/software/the/Regina/>.  I have not
tested this so cannot say whether it will allow shared objects to be used
with REXX/imc on that system.

------------------------

REXX/imc is copyright, but free.  Permission is granted to use, copy
and redistribute this code, provided that the same permission is
granted to all recipients, and that due acknowledgement is given to
the author.

