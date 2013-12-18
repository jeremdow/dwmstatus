NAME = dwmstatus
VERSION = 1.2

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
#CPPFLAGS = -DVERSION=\"${VERSION}\" -D_FORTIFY_SOURCE=2
#CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS} -march=native -O2 -pipe -fstack-protector --param=ssp-buffer-size=4
CXXFLAGS = ${CFLAGS}
#LDFLAGS = -g ${LIBS}
LDFLAGS = -g ${LIBS} -Wl,-O1,--sort-common,--as-needed,-z,relro
#LDFLAGS = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc

