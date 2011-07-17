# rexx/imc - Implementation of IBMs REXX scripting language
# See LICENSE file for copyright and license details.

include config.mk

SRC = rexx.c rxfn.c calc.c util.c shell.c interface.c globals.c
OBJ = ${SRC:.c=.o}

REXXLIBDIR = ${PREFIX}/lib
REXXIMC = ${PREFIX}/lib

RXDAY = 25
RXMONTH = 2
RXYEAR = 102

DATE = -DDAY=$(RXDAY) -DMONTH=$(RXMONTH) -DYEAR=$(RXYEAR)

FILEDEFS = -DREXXIMC=\"$(REXXIMC)\" -DREXXLIB=\"$(REXXLIBDIR)\"

all: ${LIBNAME} rexx rxque rxstack rxmathfn.rxfn

${LIBNAME}: ${OBJ}
	@echo CC -o ${LIBNAME}
	@${CC} -o ${LIBNAME} -shared ${OBJ}

rexx: ${LIBNAME} main.o
	@echo CC -o rexx
	@${CC} main.o ${LIBNAME} ${LDL} -L${LIBDIR} -o rexx

rxque: ${LIBNAME} rxque.o
	@echo CC -o rexque
	@${CC} rxque.o ${LIBNAME}  ${LDL} -L${LIBDIR} -o rxque

rxstack: ${LIBNAME} rxstack.o
	@echo CC -o rxstack
	@${CC} rxstack.o ${LIBNAME} ${LDL} -L${LIBDIR} -o rxstack

rxmathfn.rxfn: rxmathfn.c const.h functions.h
	@echo CC -c rxmathfn.c
	@$(CC) $(CCFLAGS) ${DATE} ${FILEDEFS} -I. -c rxmathfn.c
	@echo CC -o rxmathfn.rxfn
	@${CC} -o rxmathfn.rxfn -shared rxmathfn.o -lm

.c.o:
	@echo CC -c $<
	@$(CC) $(CCFLAGS) ${DATE} ${FILEDEFS} -c $<

clean:
	@echo cleaning
	@rm -f librexx.so rexx rxque rxstack rxmathfn.rxfn *.o rexx.1

install:
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@for f in rexx rxque rxstack; do \
		install -m 775 $$f ${DESTDIR}${PREFIX}/bin; \
	done
	@echo installing libraries to ${DESTDIR}${PREFIX}/lib
	@mkdir -p ${DESTDIR}${PREFIX}/lib
	@for f in librexx.so rxmathfn.rxfn rxmathfn.rxlib; do \
		install -m 644 $$f ${DESTDIR}${PREFIX}/lib; \
	done
	@echo installing headers to ${DESTDIR}${PREFIX}/include
	@mkdir -p ${DESTDIR}${PREFIX}/include
	@install -m 644 rexxsaa.h ${DESTDIR}${PREFIX}/include
	@echo installing manual page to ${DESTDIR}${PREFIX}/share/man/man1
	@sed -e 's|@REXXLIBDIR@|${REXXLIBDIR}|g' rexx.1.in > rexx.1
	@mkdir -p ${DESTDIR}${PREFIX}/share/man/man1
	@for f in rxque.1 rxstack.1 rexx.1.in; do \
	install -m 644 $$f ${DESTDIR}${PREFIX}/share/man/man1; \
	done
	@echo installing docs and examples to ${DESTDIR}${PREFIX}/share/doc/rexx-imc
	@mkdir -p ${DESTDIR}${PREFIX}/share/doc/rexx-imc/examples
	@for f in rexx.info rexx.ref rexx.summary rexx.tech; do \
		install -m 644 $$f ${DESTDIR}${PREFIX}/share/doc/rexx-imc; \
	done
	@for f in box rexxcps.rexx rexxtest.rexx rxmathfn.rexx shell.rexx; do \
		install -m 644 $$f ${DESTDIR}${PREFIX}/share/doc/rexx-imc/examples; \
	done
.PHONY: all
