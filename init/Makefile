#	$OpenBSD: Makefile,v 1.10 2018/01/06 16:26:12 millert Exp $

PROG=	init
MAN=	init.8
DPADD=	${LIBUTIL}
LDADD=	-lutil
CFLAGS+=-DDEBUGSHELL -DSECURE

# Don't enable this unless you know what you're doing!
#CFLAGS+=-DDANGEROUS

# Set this flag to enable regress tests.
#CFLAGS+=-DJUSTKIDDING

# Enable debug messages
#CFLAGS+=-DDEBUG

# Some /sbin make flags
LDSTATIC=${STATIC}
BINDIR=	/sbin

CFLAGS+=-Wall
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=-Wmissing-declarations
CFLAGS+=-Wshadow -Wpointer-arith
CFLAGS+=-Wsign-compare -Wcast-qual

SRCS=	init.c
SRCS+=	systemd.c
SRCS+=	systemd-file.c
SRCS+=	systemd-dir.c
SRCS+=	systemd-reboot.c
SRCS+=	systemd-move.c
SRCS+=	systemd-rename.c

.include <bsd.prog.mk>
