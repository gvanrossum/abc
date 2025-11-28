# ABC Makefile for atari
#
MAKE=/usr/local/gnu/make

ABC=abc.tos

CC=/ufs/frank/cross-gcc-atari/bin/cgcc

LIBS=-lpml -lgem

PARENT=../..

CPATH=$(PARENT):$(PARENT)/b:$(PARENT)/bed:$(PARENT)/bint1:$(PARENT)/bint2:$(PARENT)/bint3:$(PARENT)/btr:$(PARENT)/stc:$(PARENT)/bio:$(PARENT)/atari
vpath %.c $(CPATH)

HPATH=$(PARENT)
vpath %.h $(HPATH)

INCL=-I$(PARENT)/bhdrs -I$(PARENT)/ehdrs -I$(PARENT)/btr -I$(PARENT)/ihdrs -I$(PARENT)/atari

CFLAGS= -O -DNDEBUG $(INCL)

OBJECTS= \
	 b1file.o b1grab.o b1memo.o b1mess.o b1outp.o getopt.o \
	 e1cell.o e1code.o e1comm.o e1deco.o e1edit.o e1edoc.o e1erro.o e1eval.o e1getc.o e1goto.o e1gram.o e1help.o e1ins2.o e1inse.o e1lexi.o e1line.o e1move.o e1node.o e1outp.o e1que1.o e1que2.o e1save.o e1scrn.o e1spos.o e1sugg.o e1supr.o e1tabl.o e1wide.o \
	 i1com.o i1fun.o i1nua.o i1nuc.o i1nug.o i1nui.o i1num.o i1nuq.o i1nur.o i1nut.o i1tra.o \
	 i2ana.o i2cmd.o i2dis.o i2exp.o i2fix.o i2gen.o i2syn.o i2tar.o i2tes.o i2uni.o \
	 i3bws.o i3cen.o i3com.o i3env.o i3err.o i3fil.o i3fpr.o i3gfx.o i3imm.o i3in2.o i3ini.o i3int.o i3loc.o i3scr.o i3sou.o i3sta.o i3typ.o \
	 e1etex.o i1btr.o i1lta.o i1obj.o i1tex.o i1tlt.o \
	 i2tca.o i2tce.o i2tcp.o i2tcu.o \
	 i4bio.o i4fil.o i4grp.o i4inp.o i4lis.o i4out.o i4pack.o i4rec.o \
	 dir.o edit.o file.o keys.o main.o os.o sig.o trm.o usage.o \
#END OBJECTS

BSRCS=$(PARENT)/b/*.c
ESRCS=$(PARENT)/bed/*.c
ISRCS=$(PARENT)/bint1/*.c $(PARENT)/bint2/*.c $(PARENT)/bint3/*.c $(PARENT)/btr/*.c $(PARENT)/stc/*.c $(PARENT)/bio/*.c
SSRCS=$(PARENT)/atari/*.c

all: abc abckeys

abc: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(ABC)

abckeys: ALWAYS
	cd keys; \
	$(MAKE) -f Makefile all

messages: abc.msg

abc.msg: $(BSRCS) $(ESRCS) $(ISRCS) $(SSRCS) \
		$(PARENT)/ihdrs/i0err.h \
		$(PARENT)/ehdrs/erro.h \
		$(PARENT)/bio/i4bio.h
	$(PARENT)/scripts/Collect $(BSRCS) $(ESRCS) $(ISRCS) $(SSRCS) \
		$(PARENT)/ihdrs/i0err.h \
		$(PARENT)/ehdrs/erro.h \
		$(PARENT)/bio/i4bio.h >abc.msg

clean:
	rm -f *.o keys/*.o core

ALWAYS: # Must not exist, but must be mentioned in the makefile

# Dependencies

b1file.o : b/b1file.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bfil.h bhdrs/bmem.h bhdrs/port.h 
b1grab.o : b/b1grab.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bedi.h bhdrs/bmem.h \
  bhdrs/bobj.h 
b1memo.o : b/b1memo.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h 
b1mess.o : b/b1mess.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h bhdrs/port.h 
b1outp.o : b/b1outp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h 
getopt.o : b/getopt.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h 
e1cell.o : bed/e1cell.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/b0lan.h bhdrs/bedi.h bhdrs/bmem.h \
  bhdrs/bobj.h ehdrs/node.h ehdrs/cell.h bhdrs/port.h 
e1code.o : bed/e1code.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h ehdrs/code.h 
e1comm.o : bed/e1comm.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bcom.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/tabl.h bhdrs/port.h 
e1deco.o : bed/e1deco.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/erro.h \
  ehdrs/node.h ehdrs/gram.h ehdrs/supr.h ehdrs/queu.h ehdrs/tabl.h 
e1edit.o : bed/e1edit.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bmem.h ehdrs/erro.h \
  bhdrs/bobj.h ehdrs/node.h ehdrs/tabl.h ehdrs/gram.h ehdrs/supr.h \
  ehdrs/queu.h 
e1edoc.o : bed/e1edoc.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/erro.h ehdrs/gram.h ehdrs/oper.h ehdrs/queu.h ehdrs/supr.h \
  ehdrs/tabl.h bhdrs/port.h 
e1erro.o : bed/e1erro.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bmem.h bhdrs/bobj.h \
  ehdrs/erro.h ehdrs/node.h atari/trm.h bhdrs/port.h 
e1eval.o : bed/e1eval.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/b0lan.h bhdrs/bedi.h btr/etex.h \
  ehdrs/node.h ehdrs/gram.h 
e1getc.o : bed/e1getc.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h ehdrs/oper.h \
  ehdrs/getc.h bhdrs/port.h 
e1goto.o : bed/e1goto.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/erro.h \
  ehdrs/node.h ehdrs/gram.h ehdrs/supr.h 
e1gram.o : bed/e1gram.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bmem.h bhdrs/bobj.h \
  ehdrs/node.h ehdrs/gram.h ehdrs/supr.h ehdrs/tabl.h ehdrs/code.h \
  bhdrs/port.h 
e1help.o : bed/e1help.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bmem.h bhdrs/bobj.h \
  ehdrs/oper.h ehdrs/getc.h bhdrs/port.h atari/trm.h 
e1ins2.o : bed/e1ins2.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/queu.h ehdrs/gram.h ehdrs/tabl.h 
e1inse.o : bed/e1inse.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/gram.h ehdrs/supr.h ehdrs/tabl.h ehdrs/code.h 
e1lexi.o : bed/e1lexi.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/tabl.h 
e1line.o : bed/e1line.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/gram.h ehdrs/supr.h 
e1move.o : bed/e1move.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/gram.h ehdrs/tabl.h 
e1node.o : bed/e1node.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  bhdrs/bmem.h 
e1outp.o : bed/e1outp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h bhdrs/bmem.h \
  ehdrs/node.h ehdrs/supr.h ehdrs/gram.h ehdrs/cell.h ehdrs/tabl.h \
  atari/trm.h 
e1que1.o : bed/e1que1.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/queu.h ehdrs/gram.h ehdrs/tabl.h 
e1que2.o : bed/e1que2.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/queu.h ehdrs/gram.h ehdrs/tabl.h ehdrs/code.h 
e1save.o : bed/e1save.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/b0lan.h bhdrs/bedi.h btr/etex.h \
  bhdrs/bmem.h bhdrs/bobj.h ehdrs/node.h ehdrs/gram.h 
e1scrn.o : bed/e1scrn.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/erro.h \
  ehdrs/node.h ehdrs/supr.h ehdrs/gram.h ehdrs/cell.h atari/trm.h \
  bhdrs/port.h 
e1spos.o : bed/e1spos.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bobj.h bhdrs/bfil.h \
  ehdrs/node.h ehdrs/supr.h bhdrs/bmem.h 
e1sugg.o : bed/e1sugg.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/b0lan.h bhdrs/bmem.h bhdrs/bedi.h \
  btr/etex.h bhdrs/bobj.h bhdrs/bfil.h ehdrs/node.h ehdrs/supr.h ehdrs/gram.h \
  ehdrs/tabl.h ehdrs/queu.h bhdrs/port.h 
e1supr.o : bed/e1supr.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/erro.h \
  ehdrs/node.h ehdrs/supr.h ehdrs/gram.h ehdrs/tabl.h 
e1tabl.o : bed/e1tabl.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h ehdrs/tabl.h 
e1wide.o : bed/e1wide.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h btr/etex.h bhdrs/bobj.h ehdrs/node.h \
  ehdrs/supr.h ehdrs/gram.h ehdrs/tabl.h 
i1com.o : bint1/i1com.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i2nod.h \
  ihdrs/i2gen.h ihdrs/i3env.h 
i1fun.o : bint1/i1fun.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i0err.h ihdrs/i1num.h 
i1nua.o : bint1/i1nua.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i0err.h ihdrs/i1num.h 
i1nuc.o : bint1/i1nuc.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h ihdrs/i1num.h 
i1nug.o : bint1/i1nug.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i1num.h 
i1nui.o : bint1/i1nui.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i1num.h 
i1num.o : bint1/i1num.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i1num.h 
i1nuq.o : bint1/i1nuq.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i1num.h 
i1nur.o : bint1/i1nur.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i0err.h ihdrs/i1num.h 
i1nut.o : bint1/i1nut.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i1num.h 
i1tra.o : bint1/i1tra.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h ihdrs/i0err.h ihdrs/i1num.h 
i2ana.o : bint2/i2ana.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  ihdrs/i2nod.h ihdrs/i2gen.h ihdrs/i3env.h ihdrs/i3sou.h 
i2cmd.o : bint2/i2cmd.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  bhdrs/b0lan.h ihdrs/i2par.h ihdrs/i2nod.h ihdrs/i3env.h 
i2dis.o : bint2/i2dis.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  bhdrs/b0lan.h ihdrs/i2par.h ihdrs/i2nod.h 
i2exp.o : bint2/i2exp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i0err.h ihdrs/i2par.h ihdrs/i2nod.h ihdrs/i2gen.h ihdrs/i2exp.h \
  ihdrs/i3sou.h 
i2fix.o : bint2/i2fix.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  ihdrs/i2exp.h ihdrs/i2nod.h ihdrs/i2gen.h ihdrs/i2par.h ihdrs/i3env.h 
i2gen.o : bint2/i2gen.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  ihdrs/i2nod.h ihdrs/i2gen.h ihdrs/i2par.h ihdrs/i3cen.h ihdrs/i3env.h \
  ihdrs/i3int.h ihdrs/i3sou.h 
i2syn.o : bint2/i2syn.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  bhdrs/b0lan.h ihdrs/i2par.h ihdrs/i3scr.h ihdrs/i3env.h 
i2tar.o : bint2/i2tar.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i2par.h \
  ihdrs/i2nod.h 
i2tes.o : bint2/i2tes.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  bhdrs/b0lan.h ihdrs/i2par.h ihdrs/i2nod.h 
i2uni.o : bint2/i2uni.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  bhdrs/b0lan.h ihdrs/i2par.h ihdrs/i2nod.h ihdrs/i3env.h ihdrs/i3sou.h 
i3bws.o : bint3/i3bws.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bfil.h bhdrs/bmem.h \
  bhdrs/bobj.h ihdrs/i2par.h ihdrs/i3bws.h ihdrs/i3cen.h ihdrs/i3env.h \
  ihdrs/i3scr.h ihdrs/i3sou.h bhdrs/port.h 
i3cen.o : bint3/i3cen.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bfil.h bhdrs/bmem.h \
  bhdrs/bobj.h ihdrs/i3bws.h ihdrs/i3cen.h ihdrs/i3env.h ihdrs/i3sou.h \
  bhdrs/port.h 
i3com.o : bint3/i3com.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h bhdrs/bcom.h \
  ihdrs/i3scr.h bhdrs/port.h 
i3env.o : bint3/i3env.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i3env.h 
i3err.o : bint3/i3err.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bint.h bhdrs/bobj.h \
  ihdrs/i0err.h ihdrs/i2par.h ihdrs/i3cen.h ihdrs/i3env.h ihdrs/i3scr.h \
  ihdrs/i3sou.h 
i3fil.o : bint3/i3fil.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bint.h bhdrs/bobj.h \
  ihdrs/i2nod.h ihdrs/i2par.h ihdrs/i3scr.h ihdrs/i3sou.h 
i3fpr.o : bint3/i3fpr.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  bhdrs/b0lan.h ihdrs/i1num.h ihdrs/i2par.h ihdrs/i3cen.h ihdrs/i3sou.h \
  bhdrs/port.h 
i3gfx.o : bint3/i3gfx.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h bhdrs/bgfx.h 
i3imm.o : bint3/i3imm.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h bhdrs/b0lan.h \
  ihdrs/i2par.h ihdrs/i3env.h ihdrs/i3scr.h 
i3in2.o : bint3/i3in2.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  ihdrs/i3cen.h ihdrs/i3env.h ihdrs/i3in2.h ihdrs/i3sou.h 
i3ini.o : bint3/i3ini.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h bhdrs/bfil.h \
  bhdrs/port.h bhdrs/release.h ihdrs/i3env.h ihdrs/i3scr.h ihdrs/i3sou.h 
i3int.o : bint3/i3int.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i0err.h ihdrs/i2nod.h ihdrs/i3env.h ihdrs/i3int.h ihdrs/i3in2.h \
  ihdrs/i3sou.h ihdrs/i3sta.h 
i3loc.o : bint3/i3loc.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i0err.h \
  ihdrs/i3env.h ihdrs/i3in2.h 
i3scr.o : bint3/i3scr.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  bhdrs/bcom.h ihdrs/i2nod.h ihdrs/i2par.h ihdrs/i3typ.h ihdrs/i3env.h \
  ihdrs/i3in2.h ihdrs/i3scr.h 
i3sou.o : bint3/i3sou.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  bhdrs/bfil.h ihdrs/i2par.h ihdrs/i2nod.h ihdrs/i3bws.h ihdrs/i3cen.h \
  ihdrs/i3env.h ihdrs/i3scr.h ihdrs/i3in2.h ihdrs/i3sou.h bhdrs/port.h 
i3sta.o : bint3/i3sta.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i0err.h ihdrs/i1num.h ihdrs/i2nod.h ihdrs/i3cen.h ihdrs/i3env.h \
  ihdrs/i3int.h ihdrs/i3in2.h ihdrs/i3sou.h 
i3typ.o : bint3/i3typ.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bobj.h ihdrs/i3env.h \
  ihdrs/i3typ.h 
e1etex.o : btr/e1etex.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bedi.h bhdrs/bobj.h btr/etex.h bhdrs/bmem.h 
i1btr.o : btr/i1btr.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h btr/i1btr.h btr/i1tlt.h 
i1lta.o : btr/i1lta.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h btr/i1btr.h ihdrs/i3scr.h \
  btr/i1tlt.h 
i1obj.o : btr/i1obj.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h btr/i1btr.h btr/i1tlt.h \
  ihdrs/i3typ.h 
i1tex.o : btr/i1tex.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h btr/i1btr.h btr/i1tlt.h 
i1tlt.o : btr/i1tlt.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h btr/i1btr.h btr/i1tlt.h 
i2tca.o : stc/i2tca.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bfil.h bhdrs/bint.h \
  bhdrs/bobj.h bhdrs/b0lan.h ihdrs/i2nod.h ihdrs/i2par.h stc/i2stc.h \
  ihdrs/i3bws.h ihdrs/i3cen.h ihdrs/i3env.h ihdrs/i3sou.h bhdrs/port.h 
i2tce.o : stc/i2tce.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h stc/i2stc.h 
i2tcp.o : stc/i2tcp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h stc/i2stc.h 
i2tcu.o : stc/i2tcu.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bobj.h stc/i2stc.h 
i4bio.o : bio/i4bio.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i2par.h ihdrs/i3scr.h ihdrs/i3sou.h bio/i4bio.h bhdrs/port.h 
i4fil.o : bio/i4fil.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bfil.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i3sou.h bhdrs/port.h atari/dir.h 
i4grp.o : bio/i4grp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bfil.h bhdrs/bobj.h ihdrs/i3bws.h \
  ihdrs/i3sou.h bio/i4bio.h 
i4inp.o : bio/i4inp.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h bio/i4bio.h 
i4lis.o : bio/i4lis.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bmem.h bhdrs/bobj.h bhdrs/bfil.h \
  ihdrs/i3cen.h ihdrs/i3sou.h bio/i4bio.h 
i4out.o : bio/i4out.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i3env.h ihdrs/i3scr.h ihdrs/i3sou.h bio/i4bio.h 
i4pack.o : bio/i4pack.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/b0lan.h bhdrs/bmem.h bhdrs/bobj.h \
  ihdrs/i3cen.h ihdrs/i3sou.h bio/i4bio.h 
i4rec.o : bio/i4rec.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/bint.h bhdrs/bfil.h bhdrs/bmem.h \
  bhdrs/bobj.h ihdrs/i2nod.h ihdrs/i2par.h ihdrs/i3bws.h ihdrs/i3cen.h \
  ihdrs/i3scr.h ihdrs/i3sou.h bio/i4bio.h bhdrs/port.h 
dir.o : atari/dir.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h 
edit.o : atari/edit.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/main.h 
file.o : atari/file.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/main.h bhdrs/port.h 
keys.o : atari/keys.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h ehdrs/getc.h ehdrs/oper.h 
main.o : atari/main.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h bhdrs/getopt.h bhdrs/port.h 
os.o : atari/os.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/main.h bhdrs/port.h 
sig.o : atari/sig.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h 
trm.o : atari/trm.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h atari/trm.h 
usage.o : atari/usage.c bhdrs/b.h atari/os.h atari/comp.h atari/feat.h \
  atari/mach.h bhdrs/bvis.h 
