# Customize below to fit your system

LIBDIR = ${PREFIX}/lib
LIBNAME = librexx.so

# paths
PREFIX = /usr

# flags

CCFLAGS= -O2 -fpic -Dlinux -DFSTAT_FOR_CHARS -DHAS_GMTOFF
LDL = -ldl
