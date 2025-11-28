.c.obj:
	cl -c -AL -DNDEBUG $*.c

BOBJS=	b1file.obj b1grab.obj b1memo.obj b1mess.obj b1outp.obj getopt.obj
EOBJS=	e1cell.obj e1code.obj e1comm.obj e1deco.obj e1edit.obj e1edoc.obj \
	e1erro.obj e1etex.obj e1eval.obj e1getc.obj e1goto.obj e1gram.obj \
	e1help.obj e1ins2.obj e1inse.obj e1lexi.obj e1line.obj e1move.obj \
	e1node.obj e1outp.obj e1que1.obj e1que2.obj e1save.obj e1scrn.obj \
	e1spos.obj e1sugg.obj e1supr.obj e1tabl.obj e1wide.obj
I1OBJS=	i1btr.obj i1com.obj i1fun.obj i1lta.obj \
	i1nua.obj i1nuc.obj i1nug.obj i1nui.obj i1num.obj i1nuq.obj i1nur.obj \
	i1nut.obj i1obj.obj i1tex.obj i1tlt.obj i1tra.obj i2ana.obj i2cmd.obj \
	i2dis.obj i2exp.obj i2fix.obj i2gen.obj i2syn.obj i2tar.obj i2tca.obj \
	i2tce.obj i2tcp.obj i2tcu.obj i2tes.obj i2uni.obj
I2OBJS= i3bws.obj i3cen.obj i3com.obj \
	i3env.obj i3err.obj i3fil.obj i3fpr.obj i3gfx.obj i3imm.obj i3in2.obj \
	i3ini.obj i3int.obj i3loc.obj i3scr.obj i3sou.obj i3sta.obj i3typ.obj \
	i4bio.obj i4fil.obj i4grp.obj i4inp.obj i4lis.obj i4out.obj i4pack.obj \
	i4rec.obj
POBJS=	dir.obj edit.obj file.obj gfx.obj keys.obj main.obj os.obj sig.obj \
	trm.obj usage.obj

all: abc.exe

abc.exe: $(BOBJS) $(EOBJS) $(I1OBJS) $(I2OBJS) $(POBJS) modules mach.h
	link /noi /stack:8096 /segments:1024 @modules
	lzexe abc.exe

b1file.obj: b1file.c b.h bfil.h bmem.h port.h

b1grab.obj: b1grab.c b.h bint.h bedi.h bmem.h bobj.h

b1memo.obj: b1memo.c b.h bmem.h

b1mess.obj: b1mess.c b.h bmem.h bobj.h port.h

b1outp.obj: b1outp.c b.h bmem.h

dir.obj: dir.c b.h dir.h

e1cell.obj: e1cell.c b.h b0lan.h bedi.h bmem.h bobj.h node.h cell.h port.h

e1code.obj: e1code.c b.h code.h

e1comm.obj: e1comm.c b.h bedi.h bcom.h node.h supr.h tabl.h port.h bgfx.h

e1deco.obj: e1deco.c b.h bedi.h etex.h bobj.h erro.h node.h gram.h supr.h queu.h tabl.h

e1edit.obj: e1edit.c b.h bedi.h etex.h bmem.h erro.h bobj.h node.h tabl.h gram.h supr.h queu.h

e1edoc.obj: e1edoc.c b.h bedi.h etex.h bobj.h node.h erro.h gram.h oper.h queu.h supr.h tabl.h port.h

e1erro.obj: e1erro.c b.h bedi.h bmem.h bobj.h erro.h node.h trm.h port.h

e1etex.obj: e1etex.c b.h bedi.h bobj.h etex.h bmem.h

e1eval.obj: e1eval.c b.h b0lan.h bedi.h etex.h node.h gram.h

e1getc.obj: e1getc.c b.h bmem.h bobj.h oper.h getc.h port.h

e1goto.obj: e1goto.c b.h bedi.h etex.h bobj.h erro.h node.h gram.h supr.h

e1gram.obj: e1gram.c b.h bedi.h etex.h bmem.h bobj.h node.h gram.h supr.h tabl.h code.h port.h

e1help.obj: e1help.c b.h bedi.h bmem.h bobj.h oper.h getc.h port.h trm.h

e1ins2.obj: e1ins2.c b.h bedi.h etex.h bobj.h node.h supr.h queu.h gram.h tabl.h

e1inse.obj: e1inse.c b.h bedi.h etex.h bobj.h node.h gram.h supr.h tabl.h code.h

e1lexi.obj: e1lexi.c b.h bedi.h bobj.h node.h tabl.h

e1line.obj: e1line.c b.h bedi.h etex.h bobj.h node.h gram.h supr.h

e1move.obj: e1move.c b.h bedi.h etex.h bobj.h node.h supr.h gram.h tabl.h

e1node.obj: e1node.c b.h bedi.h etex.h bobj.h node.h bmem.h

e1outp.obj: e1outp.c b.h bedi.h etex.h bobj.h bmem.h node.h supr.h gram.h cell.h tabl.h trm.h

e1que1.obj: e1que1.c b.h bedi.h etex.h bobj.h node.h supr.h queu.h gram.h tabl.h

e1que2.obj: e1que2.c b.h bedi.h etex.h bobj.h node.h supr.h queu.h gram.h tabl.h code.h

e1save.obj: e1save.c b.h b0lan.h bedi.h etex.h bmem.h bobj.h node.h gram.h

e1scrn.obj: e1scrn.c b.h bedi.h etex.h bobj.h erro.h node.h supr.h gram.h cell.h trm.h port.h

e1spos.obj: e1spos.c b.h bedi.h bobj.h bfil.h node.h supr.h bmem.h

e1sugg.obj: e1sugg.c b.h b0lan.h bmem.h bedi.h etex.h bobj.h bfil.h node.h supr.h gram.h tabl.h queu.h port.h

e1supr.obj: e1supr.c b.h bedi.h etex.h bobj.h erro.h node.h supr.h gram.h tabl.h

e1tabl.obj: e1tabl.c b.h bedi.h tabl.h

e1wide.obj: e1wide.c b.h bedi.h etex.h bobj.h node.h supr.h gram.h tabl.h

edit.obj: edit.c b.h main.h

file.obj: file.c b.h main.h port.h

getopt.obj: getopt.c b.h

gfx.obj: gfx.c bgfx.h

i1btr.obj: i1btr.c b.h bmem.h i1btr.h i1tlt.h

i1com.obj: i1com.c b.h bint.h bobj.h i2nod.h i2gen.h i3env.h

i1fun.obj: i1fun.c b.h bobj.h i0err.h i1num.h

i1lta.obj: i1lta.c b.h bobj.h i1btr.h i3scr.h i1tlt.h

i1nua.obj: i1nua.c b.h bobj.h i0err.h i1num.h

i1nuc.obj: i1nuc.c b.h bmem.h bobj.h i1num.h

i1nug.obj: i1nug.c b.h bobj.h i1num.h

i1nui.obj: i1nui.c b.h bobj.h i1num.h

i1num.obj: i1num.c b.h bobj.h i1num.h

i1nuq.obj: i1nuq.c b.h bobj.h i1num.h

i1nur.obj: i1nur.c b.h bobj.h i0err.h i1num.h

i1nut.obj: i1nut.c b.h bobj.h i1num.h

i1obj.obj: i1obj.c b.h bmem.h bobj.h i1btr.h i1tlt.h i3typ.h

i1tex.obj: i1tex.c b.h bmem.h bobj.h i1btr.h i1tlt.h

i1tlt.obj: i1tlt.c b.h bobj.h i1btr.h i1tlt.h

i1tra.obj: i1tra.c b.h bobj.h i0err.h i1num.h

i2ana.obj: i2ana.c b.h bint.h bobj.h i0err.h i2nod.h i2gen.h i3env.h i3sou.h

i2cmd.obj: i2cmd.c b.h bint.h bobj.h i0err.h b0lan.h i2par.h i2nod.h i3env.h

i2dis.obj: i2dis.c b.h bint.h bmem.h bobj.h b0lan.h i2par.h i2nod.h

i2exp.obj: i2exp.c b.h bint.h bmem.h bobj.h i0err.h i2par.h i2nod.h i2gen.h i2exp.h i3sou.h

i2fix.obj: i2fix.c b.h bint.h bobj.h i0err.h i2exp.h i2nod.h i2gen.h i2par.h i3env.h

i2gen.obj: i2gen.c b.h bint.h bobj.h i0err.h i2nod.h i2gen.h i2par.h i3cen.h i3env.h i3int.h i3sou.h

i2syn.obj: i2syn.c b.h bint.h bmem.h bobj.h b0lan.h i2par.h i3scr.h i3env.h

i2tar.obj: i2tar.c b.h bint.h bobj.h i2par.h i2nod.h

i2tca.obj: i2tca.c b.h bmem.h bfil.h bint.h bobj.h b0lan.h i2nod.h i2par.h i2stc.h i3bws.h i3cen.h i3env.h i3sou.h port.h

i2tce.obj: i2tce.c b.h bmem.h bobj.h i2stc.h i2nod.h

i2tcp.obj: i2tcp.c b.h bobj.h i2stc.h

i2tcu.obj: i2tcu.c b.h bobj.h i2stc.h

i2tes.obj: i2tes.c b.h bint.h bobj.h i0err.h b0lan.h i2par.h i2nod.h

i2uni.obj: i2uni.c b.h bint.h bobj.h i0err.h b0lan.h i2par.h i2nod.h i3env.h i3sou.h

i3bws.obj: i3bws.c b.h bint.h bfil.h bmem.h bobj.h i2par.h i3bws.h i3cen.h i3env.h i3scr.h i3sou.h port.h

i3cen.obj: i3cen.c b.h bint.h bfil.h bmem.h bobj.h i3bws.h i3cen.h i3env.h i3sou.h port.h

i3com.obj: i3com.c b.h bmem.h bobj.h bcom.h i3scr.h port.h

i3env.obj: i3env.c b.h bint.h bobj.h i3env.h

i3err.obj: i3err.c b.h bmem.h bint.h bobj.h i0err.h i2par.h i3cen.h i3env.h i3scr.h i3sou.h bgfx.h

i3fil.obj: i3fil.c b.h bmem.h bint.h bobj.h i2nod.h i2par.h i3scr.h i3sou.h

i3fpr.obj: i3fpr.c b.h bint.h bobj.h i0err.h b0lan.h i1num.h i2par.h i3cen.h i3sou.h port.h

i3gfx.obj: i3gfx.c b.h bobj.h bgfx.h

i3imm.obj: i3imm.c b.h bint.h bobj.h b0lan.h i2par.h i3env.h i3scr.h

i3in2.obj: i3in2.c b.h bint.h bobj.h i0err.h i3cen.h i3env.h i3in2.h i3sou.h

i3ini.obj: i3ini.c b.h bint.h bobj.h bfil.h port.h release.h i3env.h i3scr.h i3sou.h port.h

i3int.obj: i3int.c b.h bint.h bmem.h bobj.h i0err.h i2nod.h i3env.h i3int.h i3in2.h i3sou.h i3sta.h

i3loc.obj: i3loc.c b.h bint.h bobj.h i0err.h i3env.h i3in2.h

i3scr.obj: i3scr.c b.h bint.h bmem.h bobj.h bcom.h i2nod.h i2par.h i3typ.h i3env.h i3in2.h i3scr.h

i3sou.obj: i3sou.c b.h bint.h bmem.h bobj.h bfil.h i2par.h i2nod.h i3bws.h i3cen.h i3env.h i3scr.h i3in2.h i3sou.h port.h

i3sta.obj: i3sta.c b.h bint.h bmem.h bobj.h i0err.h i1num.h i2nod.h i3cen.h i3env.h i3int.h i3in2.h i3sou.h

i3typ.obj: i3typ.c b.h bint.h bobj.h i3env.h i3typ.h

i4bio.obj: i4bio.c b.h bint.h bmem.h bobj.h i2par.h i3scr.h i3sou.h i4bio.h port.h

i4fil.obj: i4fil.c b.h bfil.h bmem.h bobj.h i3sou.h port.h dir.h

i4grp.obj: i4grp.c b.h bfil.h bobj.h i3bws.h i3sou.h i4bio.h

i4inp.obj: i4inp.c b.h bmem.h bobj.h i4bio.h

i4lis.obj: i4lis.c b.h bmem.h bobj.h bfil.h i3cen.h i3sou.h i4bio.h

i4out.obj: i4out.c b.h bint.h bmem.h bobj.h i3env.h i3scr.h i3sou.h i4bio.h

i4pack.obj: i4pack.c b.h b0lan.h bmem.h bobj.h i3cen.h i3sou.h i4bio.h

i4rec.obj: i4rec.c b.h bint.h bfil.h bmem.h bobj.h i2nod.h i2par.h i3bws.h i3cen.h i3scr.h i3sou.h i4bio.h port.h

keys.obj: keys.c b.h getc.h oper.h

main.obj: main.c b.h getopt.h port.h

os.obj: os.c b.h main.h port.h

sig.obj: sig.c b.h

trm.obj: trm.c b.h trm.h bgfx.h

usage.obj: usage.c b.h



