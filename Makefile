# $Id: Makefile,v 1.1 2003/06/20 05:30:43 lemit Exp $

CC = gcc -pipe
PACKETDEF = -DPACKETVER=5 -DNEW_006b
#PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

PLATFORM = $(shell uname)

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
OS_TYPE = -DCYGWIN
else
OS_TYPE =
endif

CFLAGS = -g -O2 -Wall -I../common $(PACKETDEF) $(OS_TYPE)

MKDEF = CC="$(CC)" CFLAGS="$(CFLAGS)"


all clean: src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile
	cd src ; cd common ; make $(MKDEF) $@ ; cd ..
	cd src ; cd login ; make $(MKDEF) $@ ; cd ..
	cd src ; cd char ; make $(MKDEF) $@ ; cd ..
	cd src ; cd map ; make $(MKDEF) $@ ; cd ..

src/common/GNUmakefile: src/common/Makefile
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile
src/login/GNUmakefile: src/login/Makefile
	sed -e 's/$$>/$$^/' src/login/Makefile > src/login/GNUmakefile
src/char/GNUmakefile: src/char/Makefile
	sed -e 's/$$>/$$^/' src/char/Makefile > src/char/GNUmakefile
src/map/GNUmakefile: src/map/Makefile
	sed -e 's/$$>/$$^/' src/map/Makefile > src/map/GNUmakefile
