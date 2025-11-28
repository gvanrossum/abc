CFLAGS	      = -O -DCM_N -DCM_GT -DCM_B -DCM_D

DEST	      = ..

ETC	      = ..

HDRS	      =

LIBRARY	      = libtermcap.a

MAKEFILE      = Makefile

OBJS	      = termcap.o tgoto.o tputs.o

PRINT	      = print

SRCS	      = tc1.c tc2.c tc3.c \
		termcap.c tgoto.c tputs.c

all:		$(LIBRARY)

$(LIBRARY):	$(OBJS)
		ar cru $(LIBRARY) $(OBJS)
		ranlib $(LIBRARY)

clean:;		@rm -f $(OBJS) $(LIBRARY) tc1 tc2 tc3 tc1.o tc2.o tc3.o

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	library termcap

termcap:
		cp termcap $(ETC)

library:        $(LIBRARY)
		cp $(LIBRARY) $(DEST)

database:
		cp termcap $(ETC)

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

tc1:		tc1.o $(LIBRARY)
		cc $(CFLAGS) tc1.o $(LIBRARY) -o tc1
tc2:		tc1.o $(LIBRARY)
		cc $(CFLAGS) tc1.o $(LIBRARY) -o tc2
tc3:		tc1.o $(LIBRARY)
		cc $(CFLAGS) tc1.o $(LIBRARY) -o tc3
