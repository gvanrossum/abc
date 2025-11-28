# EDIT MY ANCESTOR Makefile.bsd
# AND SAY 'make -f Makefile.bsd Makefile'
#
# BSD Makefile for booting grammar tables with mktable from grammar file.
#

# --- Where to install the stuff ---

CFILE=../bed/e1tabl.c
HFILE=../ehdrs/tabl.h

# --- What is the C preprocessor called ---
#
# ../scripts/mkdep has the right CPP if Setup succeeded and your UNIX ain't BSD

CPP=	/bin/cc -E

# --- Flags to the C compiler ---

BINCL=	-I../bhdrs -I../ehdrs -I../uhdrs
DEFS=	-DNDEBUG -DBSD
CFLAGS= -O $(DEFS) $(BINCL)
LDFLAGS=-s
LIBS=	
GDEFS=

# --- Stuff for lint ---

LINT=		lint
LINTFLAGS=	-abh
LBINCL=		$(BINCL)

# --- Relevant files ---

OBJS=	main.o alloc.o read.o fill.o comp.o dump.o code.o

SRCS=	main.c alloc.c read.c fill.c comp.c dump.c ../bed/e1code.c

HDRS=	../bhdrs/b.h main.h ../ehdrs/code.h lang.h

# --- Main entries of the makefile ---

all: tabl.c.out tabl.h.out

tabl.c.out tabl.h.out: grammar mktable
	mktable -g grammar -h tabl.h -t tabl.c.out -i tabl.h.out

grammar: grammar.abc lang.h
	$(CPP) $(GDEFS) grammar.abc 2>/dev/null | sed -e "/^$$/d" -e "/^#/d" >grammar

mktable: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o mktable

install: $(CFILE) $(HFILE)

$(CFILE): tabl.c.out
	cp tabl.c.out $(CFILE)

$(HFILE): tabl.h.out
	cp tabl.h.out $(HFILE)

clean:
	rm -f *.o mktable grammar tabl.c.out tabl.h.out tabl.c tabl.h

clobber: clean
	rm -f lint tags

code.o: ../bed/e1code.c
	$(CC) -c $(CFLAGS) ../bed/e1code.c -o code.o

# --- Utilities for the programmer ---

mflags:
	echo MFLAGS=\"$(MFLAGS)\", MAKEFLAGS=\"$(MAKEFLAGS)\"

# If your UNIX isn't BSD4.2 or higher, use:
# MKDEP=../scripts/mkdep
MKDEP=$(CC) -M

Makefile: ALWAYS
	rm -f Makefile
	(echo "# EDIT MY ANCESTOR Makefile.bsd"; \
	 echo "# AND SAY 'make -f Makefile.bsd Makefile'"; \
	 cat Makefile.bsd; \
	 $(MKDEP) $(DEFS) $(BINCL) $(SRCS); \
	) >Makefile

lint:	$(SRCS) $(HDRS)
	$(LINT) $(LINTFLAGS) $(DEFS) $(LBINCL) $(SRCS) >lint

tags:	$(HDRS) $(SRCS)
	rm -f tags
	ctags $(HDRS) $(SRCS)

test:	all
	cp tabl.h.out tabl.h
	cp tabl.c.out tabl.c
	cc -c $(CFLAGS) tabl.c

ALWAYS:	#dummy

###
main.o: main.c
main.o: ../bhdrs/b.h
main.o: ../uhdrs/osconf.h
main.o: /usr/include/stdio.h
main.o: ../uhdrs/os.h
main.o: /usr/include/math.h
main.o: /usr/include/ctype.h
main.o: /usr/include/strings.h
main.o: /usr/include/sys/types.h
main.o: /usr/include/sys/stat.h
main.o: /usr/include/sys/file.h
main.o: ../uhdrs/conf.h
main.o: ../uhdrs/config.h
main.o: ./main.h
alloc.o: alloc.c
alloc.o: ../bhdrs/b.h
alloc.o: ../uhdrs/osconf.h
alloc.o: /usr/include/stdio.h
alloc.o: ../uhdrs/os.h
alloc.o: /usr/include/math.h
alloc.o: /usr/include/ctype.h
alloc.o: /usr/include/strings.h
alloc.o: /usr/include/sys/types.h
alloc.o: /usr/include/sys/stat.h
alloc.o: /usr/include/sys/file.h
alloc.o: ../uhdrs/conf.h
alloc.o: ../uhdrs/config.h
alloc.o: ./main.h
read.o: read.c
read.o: ../bhdrs/b.h
read.o: ../uhdrs/osconf.h
read.o: /usr/include/stdio.h
read.o: ../uhdrs/os.h
read.o: /usr/include/math.h
read.o: /usr/include/ctype.h
read.o: /usr/include/strings.h
read.o: /usr/include/sys/types.h
read.o: /usr/include/sys/stat.h
read.o: /usr/include/sys/file.h
read.o: ../uhdrs/conf.h
read.o: ../uhdrs/config.h
read.o: ./main.h
fill.o: fill.c
fill.o: ../bhdrs/b.h
fill.o: ../uhdrs/osconf.h
fill.o: /usr/include/stdio.h
fill.o: ../uhdrs/os.h
fill.o: /usr/include/math.h
fill.o: /usr/include/ctype.h
fill.o: /usr/include/strings.h
fill.o: /usr/include/sys/types.h
fill.o: /usr/include/sys/stat.h
fill.o: /usr/include/sys/file.h
fill.o: ../uhdrs/conf.h
fill.o: ../uhdrs/config.h
fill.o: ./main.h
comp.o: comp.c
comp.o: ../bhdrs/b.h
comp.o: ../uhdrs/osconf.h
comp.o: /usr/include/stdio.h
comp.o: ../uhdrs/os.h
comp.o: /usr/include/math.h
comp.o: /usr/include/ctype.h
comp.o: /usr/include/strings.h
comp.o: /usr/include/sys/types.h
comp.o: /usr/include/sys/stat.h
comp.o: /usr/include/sys/file.h
comp.o: ../uhdrs/conf.h
comp.o: ../uhdrs/config.h
comp.o: ./main.h
comp.o: ../ehdrs/code.h
dump.o: dump.c
dump.o: ../bhdrs/b.h
dump.o: ../uhdrs/osconf.h
dump.o: /usr/include/stdio.h
dump.o: ../uhdrs/os.h
dump.o: /usr/include/math.h
dump.o: /usr/include/ctype.h
dump.o: /usr/include/strings.h
dump.o: /usr/include/sys/types.h
dump.o: /usr/include/sys/stat.h
dump.o: /usr/include/sys/file.h
dump.o: ../uhdrs/conf.h
dump.o: ../uhdrs/config.h
dump.o: ./main.h
e1code.o: ../bed/e1code.c
e1code.o: ../bhdrs/b.h
e1code.o: ../uhdrs/osconf.h
e1code.o: /usr/include/stdio.h
e1code.o: ../uhdrs/os.h
e1code.o: /usr/include/math.h
e1code.o: /usr/include/ctype.h
e1code.o: /usr/include/strings.h
e1code.o: /usr/include/sys/types.h
e1code.o: /usr/include/sys/stat.h
e1code.o: /usr/include/sys/file.h
e1code.o: ../uhdrs/conf.h
e1code.o: ../uhdrs/config.h
e1code.o: ../ehdrs/code.h
