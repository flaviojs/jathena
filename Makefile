# $Id: Makefile,v 1.1 2003/06/20 05:30:43 lemit Exp $

CC = gcc -pipe
PACKETDEF = -DPACKETVER=6 -DNEW_006b
#PACKETDEF = -DPACKETVER=5 -DNEW_006b
#PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

PLATFORM = $(shell uname)

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
OS_TYPE = -DCYGWIN -DFD_SETSIZE=4096

else
OS_TYPE =
endif

ifeq ($(findstring FreeBSD,$(PLATFORM)), FreeBSD)
MAKE = gmake
else
MAKE = make
endif

CFLAGS = -Wall -I../common $(PACKETDEF) $(OS_TYPE)

#debug(recommended)
CFLAGS += -g

#optimize(recommended)
CFLAGS += -O2

#change authfifo
#CFLAGS += -DCMP_AUTHFIFO_IP -DCMP_AUTHFIFO_LOGIN2

#optimize for Athlon-4(mobile Athlon)
#CFLAGS += -march=athlon -mcpu=athlon-4 -mfpmath=sse

#optimize for Athlon-mp
#CFLAGS += -march=athlon -mcpu=athlon-mp -mfpmath=sse

#optimize for Athlon-xp
#CFLAGS += -march=athlon -mcpu=athlon-xp -mfpmath=sse

#optimize for pentium3
#CFLAGS += -march=i686 -mcpu=pentium3 -mfpmath=sse -mmmx -msse2

#optimize for pentium4
#CFLAGS += -march=pentium4 -mfpmath=sse -msse -msse2

MKDEF = CC="$(CC)" CFLAGS="$(CFLAGS)"


all clean: src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd char ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MKDEF) $@ ; cd ..

src/common/GNUmakefile: src/common/Makefile
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile
src/login/GNUmakefile: src/login/Makefile
	sed -e 's/$$>/$$^/' src/login/Makefile > src/login/GNUmakefile
src/char/GNUmakefile: src/char/Makefile
	sed -e 's/$$>/$$^/' src/char/Makefile > src/char/GNUmakefile
src/map/GNUmakefile: src/map/Makefile
	sed -e 's/$$>/$$^/' src/map/Makefile > src/map/GNUmakefile
