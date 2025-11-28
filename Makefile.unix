#######################################################################
#                                                                     #
#                 Makefile for ABC system under unix.                 #
#                                                                     #
#######################################################################

# --- Some make's only make love with the Bourne shell ---
#

SHELL=	/bin/sh


# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# +++ Start of editable macro definitions; filled in by ./Setup      +++
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# --- pass make options ---
#
# On 4.{23}BSD the macro $(MFLAGS) is set by make to the collection of
# command line options (such as -k, -i) and passed to make in subdirectories.
# For System V use $(MAKEFLAGS). Otherwise just fill in 'make'.

MAKE=	make $(MFLAGS)


# --- C compiler to use ---
#
# In case you might want to change it to e.g. 'gcc' (the GNU C compiler).

CC=	cc


# --- Where to install the stuff ---
#
# These should all be absolute pathnames.

# destination directory for binaries 'abc' and 'abckeys':

DESTABC=/usr/local

# destination directory for auxiliary data files:

DESTLIB=/usr/local/lib/abc

# destination directory for 'abc.1' manual page:

DESTMAN=/usr/man/manl

# local destination if you cross-compile; empty otherwise:

DESTROOT=
# you should first generate unix/mach.h remotely;
# see 'make mach' below.

# directory where 'abc' and 'abckeys' should locate auxiliary files,
# when created in the current directory;
# change this to some proper absolute pathname, like /ufs/$user,
# if `pwd` gives an unsteady pathname, e.g. when using NFS mounted filesystems.

DESTALL="`pwd`"


# --- Software floating point needed? ---

FLOAT=


# --- Flags to the C compiler ---

# DEFS=	-DNDEBUG
DEFS=
# CFLAGS= -O $(FLOAT) $(DEFS)
CFLAGS= -m32 -fcommon $(FLOAT) $(DEFS)


# --- Flags to the loader ---

LDFLAGS= -m32


# --- Specify termcap or terminfo library ---
#
# Set TERMLIB to the appropriate termcap or terminfo library specification
# (either -lxxx option or absolute pathname) if your system has one.
# Otherwise leave TERMLIB empty and remove the comment symbols before
# the definitions of OWNTLIB, KOWNTLIB and OWNTBASE to install the
# public domain version from ./tc.

TERMLIB= /usr/lib/i386-linux-gnu/libtinfo.so.6

OWNTLIB=
KOWNTLIB=
OWNTBASE=

TCFLAGS= -O -DCM_N -DCM_GT -DCM_B -DCM_D


# --- Libraries for editor-interpreter 'abc' ---

LIBS=	-lm $(TERMLIB) $(OWNTLIB)


# --- Libraries for utility 'abckeys' ---

KLIBS=	$(TERMLIB) $(KOWNTLIB)


# +++++++++++++++++++++++++++++++++++++++++
# +++ End of editable macro definitions +++
# +++++++++++++++++++++++++++++++++++++++++
#
# The remaining macro definitions should only have to be edited
# if you make very drastic changes.

# --- Specify system directory ---

SYSTEM= unix

# --- Include flags to the C compiler for editor and interpreter directories ---

BINCL=	-I../bhdrs -I../$(SYSTEM)
EINCL=	-I../bhdrs -I../ehdrs -I../$(SYSTEM) -I../btr
IINCL=	-I../bhdrs -I../ihdrs -I../$(SYSTEM)
UINCL=	-I../bhdrs -I../ehdrs -I../ihdrs -I../$(SYSTEM)

# --- Editor and interpreter directories ---

CDIRS=	b bed bint1 bint2 bint3 btr $(SYSTEM) stc bio

BDIRS=	b
EDIRS=	bed
IDIRS=	bint1 bint2 bint3 btr stc bio
UDIRS=	$(SYSTEM)

# --- Editor and interpreter files ---

BOBJS=	b/*.o
EOBJS=	bed/*.o
IOBJS=	bint1/*.o bint2/*.o bint3/*.o btr/*.o stc/*.o bio/*.o
UOBJS=	$(SYSTEM)/*.o

BSRCS=	b/*.c
ESRCS=	bed/*.c
ISRCS=	bint1/*.c bint2/*.c bint3/*.c btr/*.c stc/*.c bio/*.c
USRCS=	$(SYSTEM)/*.c

BHDRS=	bhdrs/*.h
EHDRS=	ehdrs/*.h
IHDRS=	ihdrs/*.h btr/*.h bio/*.h stc/*.h
UHDRS=	$(SYSTEM)/*.h

# --- Preliminary dependencies (do not change for Unix) ---

MACH=	$(SYSTEM)/mach.h
OSHDIR= $(SYSTEM)

DEST=	$(SYSTEM)/file.h

# --- Stuff for programmers ---

LINT=		lint
# change the next one to -p for ATT System V
LINTFLAGS=	-abhxp
LINCL=		-Ibhdrs -Iehdrs -Iihdrs -I$(SYSTEM) -Ibtr

TAGDIRS=b bed bint1 bint2 bint3 btr stc bio $(SYSTEM) keys bhdrs ehdrs ihdrs


# -------------------------------------------
# ---  make all: make everything locally  ---
# -------------------------------------------
#
# This makes all programs and utilities in the current directory.
# (Except for the ready-for-use default key definitions files).

all:	alldest abc.ld abckeys.ld
	@./ch_all "$(DESTROOT)"

# The target 'alldest' is used to communicate the place of auxiliary files.
# This target is also used for a separate 'make abc' or 'make abckeys'.
#
# Dependency on the (non-existent) file "ALWAYS" causes this entry to
# be (re)made unconditionally. Make won't complain about ALWAYS not being
# found because there is also a rule referencing it as target at the
# very end (which actually doesn't make it, but make doesn't care).

alldest: ALWAYS
	echo "#define ABCLIB \"$(DESTALL)\"" >dest
	if cmp -s dest $(DEST); then rm dest; else mv dest $(DEST); fi

# mach: generate include file with info about the hardware configuration.
#
# Special care is taken to remove an incomplete $(MACH) if mkmach
# fails halfway. Otherwise a subsequent 'make depend' will happily go on.

mach: $(MACH)

$(MACH): mkmach.c $(OSHDIR)/comp.h $(OSHDIR)/feat.h
	@./ch_mach "$(DESTROOT)" "$(MACH)"
	$(CC) $(CFLAGS) -I$(OSHDIR) mkmach.c -o mkmach
	./mkmach >$(MACH) || (rm -f $(MACH) && exit 1)

# abc: make the executable that is the kernel of the system.
#
# The load must be unconditional, since we cannot know whether
# any of the submakes had to update some subtarget.

abc: alldest abc.ld

abc.ld:	$(MACH) $(BDIRS) $(EDIRS) $(IDIRS) $(UDIRS) \
		$(OWNTLIB) $(OWNTBASE) ALWAYS
	$(CC) $(LDFLAGS) $(BOBJS) $(EOBJS) $(IOBJS) $(UOBJS) $(LIBS) -o abc

# Call make for each editor and interpreter subdirectory with proper flags.
#
# If a dependency line has more than one item left of the colon, the
# commands are executed for each of the items, with $@ substituted
# by the item's name.

$(BDIRS): $(MACH) ALWAYS
	cd $@; $(MAKE) -f MF -f DEP CC='$(CC)' CFLAGS='$(CFLAGS) $(BINCL)' all

$(EDIRS): $(MACH) ALWAYS
	cd $@; $(MAKE) -f MF -f DEP CC='$(CC)' CFLAGS='$(CFLAGS) $(EINCL)' all

$(IDIRS): $(MACH) ALWAYS
	cd $@; $(MAKE) -f MF -f DEP CC='$(CC)' CFLAGS='$(CFLAGS) $(IINCL)' all

$(UDIRS): $(MACH) ALWAYS
	cd $@; $(MAKE) -f MF -f DEP CC='$(CC)' CFLAGS='$(CFLAGS) $(UINCL)' all

# Make utility 'abckeys' for redefinition of keybindings.
#
# The submake will find out whether recompilation is necessary.

abckeys: alldest abckeys.ld

abckeys.ld: $(MACH) $(OWNTLIB) $(OWNTBASE) ALWAYS
	cd keys; \
	 $(MAKE) -f Makefile -f DEP SYSTEM="$(SYSTEM)" \
	 	CC='$(CC)' CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="$(KLIBS)" all


# ----------------------------------------------
# --- make examples: try the ABC interpreter ---
# ----------------------------------------------

examples:
	@cd ex; ./DoExamples local
#	ch_examples is embedded in DoExamples to cope with cross compilation.


# ---------------------------------------------------------
# --- make try_editor: try the ABC editor interactively ---
# ---------------------------------------------------------

try_editor:
	@cd ex; ./TryEditor local
#	ch_tryeditor embedded in TryEditor.

# ---------------------------------------------------------
# --- make install: install everything in public places ---
# ---------------------------------------------------------
#
# The dependency of 'install' on installdest communicates the place
# and place of auxiliary files to the binaries 'abc' and 'abckeys'.
# The unconditional submakes of the targets 'abc.ld' and 'abckeys.ld'
# causes the proper files to be remade.
#
# The directory ukeys contains default keydefinitions files for
# several terminals.

install: installdest abc.ld abckeys.ld
	cp abc abckeys $(DESTROOT)$(DESTABC)
	cp abc.msg abc.hlp $(DESTROOT)$(DESTLIB)
	cd ukeys; cp abc*.key $(DESTROOT)$(DESTLIB)
	cp abc.1 $(DESTROOT)$(DESTMAN)
	@./ch_install "$(DESTABC)" "$(DESTLIB)" "$(DESTMAN)" "$(DESTROOT)"

installdest: ALWAYS
	echo "#define ABCLIB \"$(DESTLIB)\"" >dest
	if cmp -s dest $(DEST); then rm dest; else mv dest $(DEST); fi


# -------------------------------------------------
# --- Make our own termcap library and database ---
# -------------------------------------------------
#
# For systems that really don't have any termlib-like library
# this makes our own from public domain sources in ./tc.
# See ./tc/README for details.
# This happens automatically if you remove the comment symbols before
# the definitions of OWNTLIB and OWNTBASE above.

libtermcap.a:
	cd tc; $(MAKE) CC='$(CC)' CFLAGS='$(CFLAGS) $(TCFLAGS)' library

termcap:
	cd tc; make database


# -----------------------------------
# ---  make clean: local cleanup  ---
# -----------------------------------

clean:
	rm -f */*.o mkmach $(MACH) $(DEST) ex/out ex/*/*.cts


# ---------------------------------------------
# ---  Utilities for the programmer/porter  ---
# ---------------------------------------------

mflags:
	echo MFLAGS="$(MFLAGS)", MAKEFLAGS="$(MAKEFLAGS)"

# --- make makefiles: construct trivial makefiles in subdirectories ---
#
# This reconstructs trivial makefiles called 'MF' in relevant subdirectories.
# See ./Port for details.

makefiles:
	for i in $(CDIRS); do \
		( cd $$i; echo all: *.c | sed 's/\.c/.o/g' >MF ) done

# No automatic makefile in ./keys. Edit that one yourself if need be.

# --- How to generate dependency information for make ---
#
# Set MKDEP to $(CC) -M $(DEFS) or to ../scripts/mkdep $(DEFS).
# 'cc -M' is a 4.2BSD-only feature which causes the C preprocessor
# to output a list of dependencies that is directly usable by make.
# This can be simulated exactly by piping the output of your preprocessor
# through the shell script ./scripts/mkdep.
# Check the comments there to see if it needs polishing for your system.

MKDEP=	$(CC) -M $(DEFS)

# --- make depend: construct makefiles with dependencies in subdirectories ---
#
# This reconstructs additional makefiles called 'DEP' in subdirectories
# containing the dependency information.
# Dependencies on system include files are grep'ed out, to make the
# resulting DEP files distributable.
# See ./Port.

depend: $(MACH) $(DEST) bdep edep idep udep kdep

bdep:
	for i in $(BDIRS); do \
	( echo $$i; cd $$i; $(MKDEP) $(BINCL) *.c | grep -v ': /' >DEP ) done

edep:
	for i in $(EDIRS); do \
	( echo $$i; cd $$i; $(MKDEP) $(EINCL) *.c | grep -v ': /' >DEP ) done

idep:
	for i in $(IDIRS); do \
	( echo $$i; cd $$i; $(MKDEP) $(IINCL) *.c | grep -v ': /' >DEP ) done

udep:
	for i in $(UDIRS); do \
	( echo $$i; cd $$i; $(MKDEP) $(UINCL) *.c | grep -v ': /' >DEP ) done

kdep:
	cd keys; \
	$(MAKE) MKDEP="$(MKDEP)" DEFS="$(DEFS)" SYSTEM="$(SYSTEM)" depend \
	 | grep -v ': /' >DEP

$(DEST): alldest

# --- make messages ---
#
# Make new messages file when you have changed or added any MESS/GMESS
# definitions in the source.
# Note: the Collect and Change scripts can be found in ./scripts.
# See ./Problems for details.

messages: abc.msg

abc.msg: $(BSRCS) $(ESRCS) $(ISRCS) $(USRCS) \
		ihdrs/i0err.h ehdrs/erro.h bio/i4bio.h
	./scripts/Collect $(BSRCS) $(ESRCS) $(ISRCS) $(USRCS) \
		ihdrs/i0err.h ehdrs/erro.h bio/i4bio.h >abc.msg

# --- make help ---
#
# Help file from manual entry.
# Sorry, the file unix/abc.mac was created from copyrighted material;
# therefore, it is not in the distribution.
#

help: abc.hlp

abc.hlp: unix/abc.mac abc.1
	nroff unix/abc.mac abc.1 >abc.help
	(echo "SUMMARY OF SPECIAL ACTIONS"; \
	 sed -e '1,/^SUMMARY/d' abc.help; \
	 echo " "; \
	 sed -e '/^SUMMARY/,$$d' abc.help) >abc.hlp
	rm abc.help

# ---  make clobber: additional local cleanup   ---

clobber:
	rm -f abc abckeys */tags tags ID

# ---  make dist: turn an ABC version into a distribution  ---
# cleans auxiliary files and CVS administration

dist:	clean clobber
	find . -name CVS.adm -exec rm -rf {} \;
	find . \( -name \*~ -o -name .\*~ \) -exec rm -f {} \;
	rm -f Makefile answers unix/abc.mac
	rm -f userdirs repositories repos-userdirs
	rm -f Dbxabc Lint Out @*

# --- make lint, tags and id ---

lint:	$(DEST) abclint klint

abclint:
	$(LINT) $(LINTFLAGS) $(DEFS) $(LINCL) \
		$(BSRCS) $(ESRCS) $(ISRCS) $(USRCS)

klint:
	cd keys; \
	 $(MAKE) LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" DEFS="$(DEFS)" \
		SYSTEM="$(SYSTEM)" lint

tags:	ALWAYS
	etags */*.h */*.c
	rm -f tags   # Remove it so it will be remade when an interrupt hits
	for i in $(TAGDIRS); \
	do \
		( echo $$i; cd $$i; ctags -w *.[ch]; \
		  sed "s,	,	$$i/," tags \
		) \
	done | sort -o tags


id:	ALWAYS
	mkid */*.[hc]


ALWAYS: # Must not exist, but must be mentioned in the makefile
