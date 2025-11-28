# ABCKEYS Makefile for atari
#
MAKE=/usr/local/gnu/make

ABC=abckeys.tos

CC=/ufs/frank/cross-gcc-atari/bin/cgcc

LIBS=-lpml -lgem

PARENT=../../..

CPATH=$(PARENT):$(PARENT)/b:$(PARENT)/bed:$(PARENT)/bint1:$(PARENT)/bint2:$(PARENT)/bint3:$(PARENT)/btr:$(PARENT)/stc:$(PARENT)/bio:$(PARENT)/atari:$(PARENT)/keys
vpath %.c $(CPATH)

HPATH=$(PARENT)
vpath %.h $(HPATH)

INCL=-I$(PARENT)/bhdrs -I$(PARENT)/ehdrs -I$(PARENT)/btr -I$(PARENT)/ihdrs -I$(PARENT)/atari -I$(PARENT)/keys

CFLAGS= -O -DNDEBUG -DKEYS $(INCL)

OBJECTS=keydef.o keyhlp.o e1getc.o b1file.o b1memo.o b1mess.o file.o keys.o trm.o os.o

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(ABC)

# Dependencies

keydef.o : keys/keydef.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bfil.h bhdrs/bmem.h ehdrs/oper.h \
  ehdrs/getc.h atari/trm.h bhdrs/release.h keys/keydef.h bhdrs/port.h 
keyhlp.o : keys/keyhlp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h ehdrs/oper.h ehdrs/getc.h 
e1getc.o : bed/e1getc.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h ehdrs/oper.h \
  ehdrs/getc.h bhdrs/port.h 
b1file.o : b/b1file.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bfil.h bhdrs/bmem.h bhdrs/port.h 
b1memo.o : b/b1memo.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h 
b1mess.o : b/b1mess.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h bhdrs/port.h 
file.o : atari/file.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/main.h bhdrs/port.h 
keys.o : atari/keys.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h ehdrs/getc.h ehdrs/oper.h 
trm.o : atari/trm.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/trm.h 
os.o : atari/os.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/main.h bhdrs/port.h 

