#
# Makefile for ../abckeys.
#
# This is intended to be used through ../Makefile, where several
# generic macro's are filled in.
# The following macro's are more specific and should be changed here
# in case you have to make very drastic changes.

# --- Include flags to the C compiler ---

KINCL=	-I../bhdrs -I../ehdrs -I../$(SYSTEM)

# --- Include flags for lint ---

LKINCL=	$(KINCL)

# --- Relevant files ---

OBJS=	keydef.o keyhlp.o \
	e1getc.o \
	b1file.o b1memo.o b1mess.o \
	file.o keys.o trm.o os.o

SRCS=	keydef.c keyhlp.c \
	../bed/e1getc.c \
	../b/b1file.c ../b/b1memo.c ../b/b1mess.c \
	../$(SYSTEM)/file.c ../$(SYSTEM)/keys.c ../$(SYSTEM)/trm.c ../$(SYSTEM)/os.c

HDRS=

# --- Main entries of the makefile ---

all:	../abckeys

../abckeys: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o ../abckeys

keydef.o: keydef.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) keydef.c

keyhlp.o: keyhlp.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) keyhlp.c

e1getc.o: ../bed/e1getc.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../bed/e1getc.c

b1file.o: ../b/b1file.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../b/b1file.c

b1memo.o: ../b/b1memo.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../b/b1memo.c

b1mess.o: ../b/b1mess.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../b/b1mess.c

file.o: ../$(SYSTEM)/file.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../$(SYSTEM)/file.c

keys.o: ../$(SYSTEM)/keys.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../$(SYSTEM)/keys.c

trm.o: ../$(SYSTEM)/trm.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../$(SYSTEM)/trm.c

os.o: ../$(SYSTEM)/os.c
	$(CC) -c $(CFLAGS) -DKEYS $(KINCL) ../$(SYSTEM)/os.c

# --- Utilities for the programmer ---

depend:
	@$(MKDEP) $(DEFS) $(KINCL) $(SRCS)

lint:
	$(LINT) $(LINTFLAGS) $(DEFS) -DKEYS $(LKINCL) $(SRCS)

ALWAYS:	#dummy

###
# Dependency information is taken from ./DEP; see ../Makefile.
###
