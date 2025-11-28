/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/* Sources: maintaining units and values on external files */

#include "b.h"
#include "bint.h"
#include "bmem.h"
#include "bobj.h"
#include "bfil.h"
#include "i2par.h"
#include "i2nod.h"
#include "i3bws.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3scr.h"
#include "i3in2.h"
#include "i3sou.h"
#include "port.h"

Forward Hidden bool do_discard();
Forward Hidden bool smash();
Forward Hidden value which_funprd();
Forward Hidden Procedure ed_unit();
Forward Hidden Procedure edunit();
Forward Hidden Procedure free_original();
Forward Hidden bool same_heading();
Forward Hidden bool rnm_file();
Forward Hidden bool still_there();
Forward Hidden bool ens_filed();
Forward Hidden int err_line();
Forward Hidden Procedure ed_target();
Forward Hidden bool env_ok();
Forward Hidden Procedure put_targs();
Forward Hidden Procedure lst_fileheading();
Forward Hidden bool ens_tfiled();

extern int winheight;
bool ask_for();

#ifdef TYPE_CHECK
value stc_code();
#endif

#define Is_filed(v) (Is_indirect(v))

/*****************************************************************************/

/* def_perm(): add/replace the filename mapping of a how-to/location */

Visible Procedure def_perm(pname, fname)
     value pname;
     value fname;
{
	e_replace(fname, &(cur_env->perm), pname);
	cur_env->permchanges = Yes;
}

/* free_perm(): remove the filename mapping of a how-to/location */

Visible Procedure free_perm(pname)
     value pname;
{
	e_delete(&(cur_env->perm), pname);
	cur_env->permchanges = Yes;
}

/* p_exists(): search the filename mapping of a how-to/location */

Visible bool p_exists(pname, aa)
     value pname;
     value **aa;
{
	return in_env(cur_env->perm, pname, aa);
}

/*****************************************************************************/

/* def_unit(): add/replace the how-to mapping of a user how-to */

Visible Procedure def_unit(pname, u)
     value pname;
     value u;
{
	e_replace(u, &(cur_env->units), pname);
}

/* free_unit(): remove the how-to mapping of a user how-to */

Hidden Procedure free_unit(pname)
     value pname;
{
	e_delete(&(cur_env->units), pname);
}

/* u_exists(): search the how-to mapping of a user how-to */

Visible bool u_exists(pname, aa)
     value pname;
     value **aa;
{
	return in_env(cur_env->units, pname, aa);
}

/*****************************************************************************/

/* def_std_howto(): add/replace the how-to mapping of a standard how-to */

Visible Procedure def_std_howto(pname, h)
     value pname;
     value h;
{
	e_replace(h, &(std_env->units), pname);
}

/*****************************************************************************/

#define t_exists(name, aa)	(in_env(prmnv->tab, name, aa))

Visible Procedure def_target(name, t) value name, t; {
	e_replace(t, &prmnv->tab, name);
}

#define free_target(name)	(e_delete(&prmnv->tab, name))

/************************** UNITS ************************************/

#define Is_funprd(u)		(Is_function(u) || Is_predicate(u))
#define Is_predefined(u)	(Is_funprd(u) && Funprd(u)->pre != Use)

#define USR_ALL		'1'
#define USR_PARSED	'2'

Hidden Procedure freeunits(which) literal which; {
	intlet k, len;
	value vkey, vassoc;
	
	len= length(cur_env->units);
	for (k= len-1; k >= 0; --k) {
		/* Reverse loop so deletions don't affect the numbering! */
		vkey= *key(cur_env->units, k);
		vassoc= *assoc(cur_env->units, k);
		switch (which) {
		case USR_ALL:
			if (!Is_predefined(vassoc)) free_unit(vkey);
			break;
		case USR_PARSED:
			if (!Is_predefined(vassoc) &&
					!How_to(vassoc)->unparsed)
				free_unit(vkey);
			break;
		}
	}
}

Visible Procedure rem_unit(u, wse)
     parsetree u;
     wsenvptr wse;
{
	value pname;
	wsenvptr old_env;

	old_env = setcurenv(wse);
	pname = get_pname(u);
	free_unit(pname);
	release(pname);
	resetcurenv(old_env);
}

/********************************************************************** */

Visible value permkey(name, type) value name; literal type; {
	char t[2];
	value v, w;
	
	if (!Valid(name))
		return Vnil;
	t[0]= type; t[1]= '\0';
	w= mk_text(t);
	v= concat(w, name); release(w);
	return v;
}

Visible string lastunitname() {
	value *aa;
	
	if (p_exists(last_unit, &aa))
		return sstrval(Permname(*aa));
	return NULL;
}

#define CANTGETFNAME	MESS(4000, "cannot create file name for %s")

/* get_ufname() is only invoked in the using workspace environment */

Hidden value get_ufname(pname, silently) value pname; bool silently; {
	value fname;
	value *aa;
	
	if (p_exists(pname, &aa))
		fname= copy(*aa);
	else {
		value name= Permname(pname);
		literal type= Permtype(pname);
		
		fname= new_fname(name, type);
		if (Valid(fname))
			def_perm(pname, fname);
		else if (!silently)
			interrV(CANTGETFNAME, name);
		release(name);
	}
	return fname;
}

/* del_perm()
 * Remove the filename mapping, the file from disk and the filename
 * from the positions file.
 * This routine is only called in the using workspace environment.
 * Whenever this changes, you have to adjust the arguments of f_delete()
 * and idelpos(). Moreover, the position file of the central workspace
 * must be loaded.
 */

Hidden Procedure del_perm(pname)
     value pname;
{
	value *aa;
	if (p_exists(pname, &aa)) {
		f_delete(strval(*aa));
		idelpos(*aa);	/* delete file from positions file */
		free_perm(pname);
	}
}

/***********************************************************************/

Visible bool is_loaded(fname, pname, aa)
     char *fname;
     value pname;
     value **aa;
{
	value u = Vnil;
	value npname = Vnil;

	ifile = fopen(fname, "r");
	if (ifile == NULL) {
		vs_ifile();
		return No;
	}
	Eof = No;
	first_ilev();
	u = get_unit(&npname, Yes, No);
	if (still_ok)
		def_unit(npname, u);
	fclose(ifile);
	vs_ifile();
	Eof = No;
	if (still_ok && !u_exists(pname, aa)) {
		value name = Permname(pname);; 
		sethowtoname(copy(pname));
		curline = How_to(u)->unit; curlino = one;
		interrV(
		MESS(4001, "filename and how-to name incompatible for %s"),
		name);
		release(name);
	}
	release(u); release(npname);
	return still_ok;
}

/***********************************************************************/

#define CANT_WRITE \
	MESS(4002, "cannot create file %s; need write permission in directory")

#define CANT_READ \
	MESS(4003, "unable to find file")

Hidden Procedure u_name_type(v, name, type) parsetree v; value *name;
		literal *type; {
	intlet adic;
	switch (Nodetype(v)) {
		case HOW_TO:	*type= Cmd; break;
		case YIELD:	adic= intval(*Branch(v, FPR_ADICITY));
				*type= adic==0 ? Zfd : adic==1 ? Mfd : Dfd;
				break;
		case TEST:	adic= intval(*Branch(v, FPR_ADICITY));
				*type= adic==0 ? Zpd : adic==1 ? Mpd : Dpd;
				break;
		default:	syserr(MESS(4004, "wrong nodetype of how-to"));
	}
	*name= copy(*Branch(v, UNIT_NAME));
}

Visible value get_unit(pname, filed, editing) value *pname; bool filed, editing;
{
	value name; literal type;
	parsetree u= unit(No, editing);
	if (u == NilTree)
		return Vnil;
	u_name_type(u, &name, &type);
	*pname= permkey(name, type);
	release(name);
	switch (Nodetype(u)) {
		case HOW_TO:	return mk_how(u, filed);
		case YIELD:	return mk_fun(type, Use, u, filed);
		case TEST:	return mk_prd(type, Use, u, filed);
		default:	return Vnil; /* Keep lint happy */
	}
}

Visible value get_pname(v) parsetree v; {
	value pname, name; literal type;
	u_name_type(v, &name, &type);
	pname= permkey(name, type);
	release(name);
	return pname;
}

Hidden Procedure get_heading(h, pname) parsetree *h; value *pname; {
	*h= unit(Yes, No);
	*pname= still_ok ? get_pname(*h) : Vnil;
}

/********************************************************************** */

/* Check for certain types of name conflicts.
   The checks made are:
   - unit with the same name
   - function and predicate with the same name (and different or same
     adicity)
   - function or predicate with the same name as a target
   - zeroadic and monadic unit with the same name
   - zeroadic and dyadic unit with the same name.
*/

Hidden bool isfunction(name, type, wse)
     value name;
     literal *type;
     wsenvptr *wse;
{
	if      (is_unit(name, Zfd, PPnil, wse)) *type = Zfd;
	else if (is_unit(name, Mfd, PPnil, wse)) *type = Mfd;
	else if (is_unit(name, Dfd, PPnil, wse)) *type = Dfd;
	else return No;
	return Yes;
}

Hidden bool ispredicate(name, type, wse)
     value name;
     literal *type;
     wsenvptr *wse;
{
	if      (is_unit(name, Zpd, PPnil, wse)) *type = Zpd;
	else if (is_unit(name, Mpd, PPnil, wse)) *type = Mpd;
	else if (is_unit(name, Dpd, PPnil, wse)) *type = Dpd;
	else return No;
	return Yes;
}

Hidden bool islocation(name, type, wse)
     value name;
     literal *type;
     wsenvptr *wse;
{
	value pname = permkey(name, Tar);
	value *aa;

	if (p_exists(pname, &aa) || t_exists(name, &aa)) {
		*type = Tar;
		*wse = use_env;
		release(pname);
		return Yes;
	}
	release(pname);
	return No;
}

#define CR_EXIST \
        MESS(4005, "there is already a how-to with this name")
#define CR_CEN_EXIST \
        MESS(4006, "there is already a central how-to with this name")
#define CR_STD_EXIST \
        MESS(4007, "there is already a standard how-to with this name")
#define CR_TAR \
        MESS(4008, "there is already a permanent location with this name")

/* name_conflict_while_creating()
 * Is there a name conflict while creating a how-to ?
 */

Hidden bool name_conflict_while_creating(pname)
     value pname;
{
	value name;
	literal type;
	wsenvptr wse;
	literal ntype;

	name = Permname(pname);
	type = Permtype(pname);
	if (smash(name, type, &ntype, &wse)) {
		if (ntype == Tar) {
			interr(CR_TAR);
		}
		else {
			if (IsUsingEnv(wse)) interr(CR_EXIST);
			else if (IsCentralEnv(wse)) interr(CR_CEN_EXIST);
			else interr(CR_STD_EXIST);
			if (!IsStandardEnv(wse)) {
				value npname = permkey(name, ntype);
				wsenvptr old_env = setcurenv(wse);

				def_perm(last_unit, npname);
				cur_env->errlino = 0;
				resetcurenv(old_env);
				release(npname);
			}
		}
		release(name);
		return Yes;
	}
	release(name);
	return No;
}

#define ED_EXIST	MESS(4009, "*** the how-to name is already in use;\n*** should the old how-to be discarded?\n*** (if not you have to change the how-to name)\n")

#define ED_TAR		MESS(4010, "*** the how-to name is already in use for a permanent location;\n*** should that location be deleted?\n*** (if not you have to change the how-to name)\n")

#define ED_CHANGE       MESS(4011, "*** the how-to name is already in use;\n*** you have to change the how-to name\n")

#define ED_CONTINUE     MESS(4012, "*** Press [SPACE] or [RETURN] to continue")

/* name_clash_after_editing()
 * Is there a name clash after editing a how-to ?
 */

Hidden bool name_clash_after_editing(pname)
     value pname;
{
	value name;
	literal type;
	wsenvptr wse;
	literal ntype;

	if (!Valid(pname))
		return No;
	name = Permname(pname);
	type = Permtype(pname);
	while (smash(name, type, &ntype, &wse)) {
		if (IsUsingEnv(wse)) { 
			/* Currently, you can only remove files from
			 * the using workspace environment;
			 * See also: del_perm()
			 */

			if (!do_discard(name, ntype)) {
				release(name);
				return Yes;
			}
			/* the old howto has been discarded;
			 * continue checking, for there can be both 
			 * a monadic and a dyadic version
			 */
			continue;
		}
		else {
			putmess(ED_CHANGE);
			VOID ask_for(ED_CONTINUE);
			release(name);
			return Yes;
		}
	}
	release(name);
	return No;
}

/* do_discard() is only invoked in the using workspace environment */
/* see comment at invoker above */

Hidden bool do_discard(name, type)
     value name;
     literal type;
{
	if (is_intended(type == Tar ? ED_TAR : ED_EXIST)) {
		if (type == Tar)
			del_target(name);
		else {
			value pname = permkey(name, type);
			free_unit(pname);
			del_perm(pname);
			release(pname);
		}
		return Yes;
	}
	return No;
}

Hidden bool smash(name, type, ntype, wse)
     value name;
     literal type;
     literal *ntype;
     wsenvptr *wse;
{
	if (is_unit(name, type, PPnil, wse) && IsUsingEnv(*wse)) {
		*ntype = type;
		return Yes;
	}

	switch (type) {

	      case Cmd:
		return No;

	      case Zfd:
		return is_unit(name, *ntype = Mfd, PPnil, wse)
		       || is_unit(name, *ntype = Dfd, PPnil, wse)
		       || ispredicate(name, ntype, wse)
		       || islocation(name, ntype, wse);

	      case Mfd:
	      case Dfd:
		return is_unit(name, *ntype = Zfd, PPnil, wse)
		       || ispredicate(name, ntype, wse)
		       || islocation(name, ntype, wse);

	      case Zpd:
		return is_unit(name, *ntype = Mpd, PPnil, wse)
		       || is_unit(name, *ntype = Dpd, PPnil, wse)
		       || isfunction(name, ntype, wse)
		       || islocation(name, ntype, wse);

	      case Mpd:
	      case Dpd:
		return is_unit(name, *ntype = Zpd, PPnil, wse)
		       || isfunction(name, ntype, wse)
		       || islocation(name, ntype, wse);

	      default:
		return No;
	}
}

/***********************************************************************/

/* Create a unit via the editor or from the input stream. */

extern bool OPTunpack; /* -u on the command line */

Visible bool need_rec_suggestions= No; /* e.g. after abc -u file ... */

Visible Procedure create_unit() {
	value pname= Vnil; parsetree heading= NilTree;
	if (!interactive) {
		value v= get_unit(&pname, No, No);
		if (still_ok) def_unit(pname, v);
 		if (OPTunpack) { /* write out to file */
 			value fname;
 			VOID ens_filed(pname, &fname, Yes);
 			release(fname);
			need_rec_suggestions= Yes;
 		}
		release(v); release(pname);
		return;
	}
	get_heading(&heading, &pname);
	curline= heading; curlino= one; /* For all error messages */
	if (still_ok && !name_conflict_while_creating(pname)) {
		value fname= get_ufname(pname, No);

		if (Valid(fname)) {
			FILE *fp= fopen(strval(fname), "w");
			if (fp == NULL)
				interrV(CANT_WRITE, fname);
			else {
				txptr tp= fcol();
				do { fputc(Char(tp), fp); }
				while (Char(tp++) != '\n');
				fputc('\n', fp);
				f_close(fp);
				ed_unit(&pname, &fname, Yes);
			}
		}
		release(fname);
	}
	release(pname); release(heading);
}


/***********************************************************************/

/* Edit a unit. The name of the unit is either given, or is defaulted
   to the last unit edited or the last unit that gave an error, whichever
   was most recent.
   It is possible for the user to mess things up with the w command, for
   instance, but this is not checked. It is allowed to rename the unit though,
   or delete it completely. If the file is empty, the unit is disposed of.
   Otherwise, the name and adicity are determined and if these have changed,
   the new unit is written out to a new file, and the original deleted.
   Thus the original is not saved.

   The function edit_unit parses the command line and does some
   high-level bookkeeping; ed_unit does the lower-level bookkeeping;
   f_edit is called to pass control to the editor and wait till it
   finishes its job.  Note that the editor reads the unit from the file
   and writes it back (if changed); there is no sharing of data
   structures such as parse trees in this version of the system.

   Renaming, deleting, or changing the adicity of a test or yield
   unfortunately requires all other units to be thrown away internally
   (by freeunits), since the unit parse trees may be wrong. For instance,
   consider the effect on the following of making a formerly monadic
   function f, into a zeroadic function:
	WRITE f root 2
*/

#define CANT_EDIT	MESS(4013, "I find nothing editible here")

Visible value last_unit= Vnil; /* last edited/erroneous unit */

Visible Procedure edit_unit() {
	value name= Vnil, pname= Vnil; 
	value fname, *aa;
	char *kw;

	if (Ceol(tx)) {
		if (!p_exists(last_unit, &aa))
			parerr(MESS(4014, "no current how-to"));
		else pname= copy(*aa);
	}
	else if (is_cmdname(ceol, &kw)) {
		name= mk_text(kw);
		pname= permkey(name, Cmd);
	}
	else if (is_tag(&name))
		pname= which_funprd(name);
	else
		parerr(CANT_EDIT);

	if (still_ok && ens_filed(pname, &fname, No)) {
		ed_unit(&pname, &fname, No);
		release(fname);
	}
	release(name); release(pname);
}

#define ED_MONDYA	MESS(4015, "*** do you want to visit the version with %c or %c operands?\n")
#define ONE_PAR '1'
#define TWO_PAR '2'

#define NO_HOWTO MESS(4016, "%s isn't a how-to in this workspace")

Hidden value which_funprd(name)
     value name;
{
	/* There may be two units with the same name (functions
	   or predicates of different adicity).  Check if this
	   is the case, and if so, ask which one is meant.
	*/
	wsenvptr wse;
	bool mfd, dfd, mpd, dpd;

	if (is_unit(name, Zfd, PPnil, &wse) && IsUsingEnv(wse))
		return permkey(name, Zfd);
	if (is_unit(name, Zpd, PPnil, &wse) && IsUsingEnv(wse))
		return permkey(name, Zpd);

	mfd = is_unit(name, Mfd, PPnil, &wse) && IsUsingEnv(wse);
	mpd = is_unit(name, Mpd, PPnil, &wse) && IsUsingEnv(wse);
	dfd = is_unit(name, Dfd, PPnil, &wse) && IsUsingEnv(wse);
	dpd = is_unit(name, Dpd, PPnil, &wse) && IsUsingEnv(wse);

	if ((mfd || mpd) && (dfd || dpd)) {
		char qans = q_answer(ED_MONDYA, ONE_PAR, TWO_PAR, TWO_PAR);
		if (qans == ONE_PAR) {
			return permkey(name, mfd ? Mfd : Mpd);
		}
		else if (qans == TWO_PAR) {
			return permkey(name, dfd ? Dfd : Dpd);
		}
		else {
			/* interrupted */
			still_ok = No;
			return Vnil;
		}
	}
	else if (mfd) return permkey(name, Mfd);
	else if (mpd) return permkey(name, Mpd);
	else if (dfd) return permkey(name, Dfd);
	else if (dpd) return permkey(name, Dpd);
	else {
		pprerrV(NO_HOWTO, name);
		return Vnil;
	}
}
	
#define NO_U_WRITE	MESS(4017, "*** you have no write permission in this workspace:\n*** you may not change the how-to\n*** do you still want to display the how-to?\n")

/* Edit a unit.  Parameters are the prmnv key and the file name.
   This is called in response to the ':' command and when a new unit is
   created (the header of the new unit must already be written to the
   file).
   Side effects are many, e.g. on prmnv: the unit may be deleted or
   renamed.  When renamed, the original unit is lost.
   The unit is reparsed after editing.  A check is made for illegal
   name conflicts (e.g., a zeroadic and a monadic unit of the same
   name), and this is resolved by forcing the user to edit the unit
   again. In that case the edit is done on a temporary file.
   The new unit name is kept as the current unit name; when the unit is
   deleted the current unit name is set to Vnil. */

Hidden bool clash;

#define First_edit (!clash)

#ifdef TYPE_CHECK
Hidden value old_typecode= Vnil;
#define Sametypes(old, new) ((!Valid(old) && !Valid(new)) || \
		(Valid(old) && Valid(new) && compare(old, new) == 0))
#endif

Hidden Procedure ed_unit(pname, fname, creating) value *pname, *fname;
		bool creating;
{
#ifdef CK_WS_WRITABLE
	if (!ckws_writable(NO_U_WRITE))
	  /* workspace is read-only and user don't want to display */
	  return;
#endif
#ifdef CLEAR_MEM
	clear_perm();
		/* To give the editor as much space as possible, remove
		   all parse trees and target values from memory.
		   (targets that have been modified are first written
		   out, of course).
		*/
#endif
	clash= No;
#ifdef TYPE_CHECK
	old_typecode= stc_code(*pname);
	if (!creating) del_types();
#endif
	do edunit(pname, fname, creating); while (clash);
#ifdef SAVE_PERM
	put_perm();
#endif
#ifdef TYPE_CHECK
	release(old_typecode);
#endif
}

Hidden Procedure edunit(p_pname, p_fname, creating) value *p_pname, *p_fname;
		bool creating; {
	value pname= *p_pname, fname= *p_fname;
	value npname= Vnil, u;
	bool new_def, changed, samehead;
#ifdef TYPE_CHECK
	value new_typecode;
#endif

	sethowtoname(copy(pname));
	changed= f_edit(fname, err_line(pname), ':', creating && First_edit)
		 || creating;
	cur_env->errlino= 0;
	if (First_edit && !changed) {
		/* Remember it as current unit: */

		def_perm(last_unit, pname);
#ifdef TYPE_CHECK
		if (!creating) adjust_types(Yes);
#endif
		return;
	}
	if (!still_there(fname)) {
		free_original(pname);
#ifdef TYPE_CHECK
		if (!creating) adjust_types(No);
#endif
		idelpos(fname);	/* delete file from positions file */
		free_perm(last_unit);
		clash= No;
		return;
	}
	first_ilev();
	u= get_unit(&npname, Yes, Yes);
		/* the second Yes means the user may edit the heading;
		 * therefore no type check now in unit() */
	fclose(ifile); vs_ifile(); Eof= No;
	
	if (First_edit && same_heading(pname, npname, u)) {
		new_def= Yes;
		samehead= Yes;
	}
	else {
		samehead= No;
		free_original(pname);
		if (!name_clash_after_editing(npname)
		    && rnm_file(fname, npname)
		   ) {
			clash= No;
		}
		else {
			/* edit again with npname and temp fname */
			release(*p_pname);
			*p_pname= copy(npname);
			if (First_edit) {
				value tfile= mk_text(temp1file);
				f_rename(fname, tfile);
				imovpos(fname, tfile);
				/* move position in positions file */
				release(*p_fname);
				*p_fname= tfile;
			}
			clash= Yes;
		}
		new_def= !clash;
	}
	if (new_def) {
		/* changed heading now def_perm()'ed, so now typecheck */
#ifdef TYPE_CHECK
		type_check((Is_funprd(u) ? Funprd(u)->unit : How_to(u)->unit));
		new_typecode= stc_code(npname);
		if (!creating)
			adjust_types(samehead &&
				     Sametypes(old_typecode, new_typecode));
		release(new_typecode);
#endif
		if (still_ok) def_unit(npname, u);
		else free_unit(npname);
		def_perm(last_unit, npname);
	}
	release(npname); release(u);
}

Hidden Procedure free_original(pname) value pname; {
	if (First_edit) {
		free_unit(pname); 
		free_perm(pname);
		freeunits(USR_PARSED);
	}
}

#define cmd_unit(pname)	(Permtype(pname) == Cmd)

Hidden bool same_heading(pname, npname, u_new) value pname, npname, u_new; {
	value *aa;
	
	if (!Valid(u_new) || !Valid(npname))
		return No;
	else if (compare(pname, npname) != 0)
		return No;
	else if (!cmd_unit(pname))
		return Yes;
	else if (!u_exists(pname, &aa))
		return Yes;
	else {
		parsetree old= How_to(*aa)->unit;
		parsetree new= How_to(u_new)->unit;
		parsetree old_kw, old_fml, old_next;
		parsetree new_kw, new_fml, new_next;
		
		old= *Branch(old, HOW_FORMALS);
		new= *Branch(new, HOW_FORMALS);
		do {
			old_kw= *Branch(old, FML_KEYW);
			old_fml= *Branch(old, FML_TAG);
			old_next= *Branch(old, FML_NEXT);
			new_kw= *Branch(new, FML_KEYW);
			new_fml= *Branch(new, FML_TAG);
			new_next= *Branch(new, FML_NEXT);
			
			if (compare(old_kw, new_kw) != 0)
				return No;
			else if (old_fml == NilTree && new_fml != NilTree)
				return No;
			else if (old_fml != NilTree && new_fml == NilTree)
				return No;
			else if (old_next == NilTree && new_next != NilTree)
				return No;
			else if (old_next != NilTree && new_next == NilTree)
				return No;
			old= old_next;
			new= new_next;
		}
		while (old != NilTree);
		return Yes;
	}
}

#define CANT_GET_FNAME	MESS(4018, "*** cannot create file name;\n*** you have to change the how-to name\n")

/* rnm_file() is only invoked in the using workspace environment */

Hidden bool rnm_file(fname, pname) value fname, pname; {
	value nfname;
	
	nfname= (Valid(pname) ? get_ufname(pname, Yes) : Vnil);
	
	if (Valid(nfname)) {
		f_rename(fname, nfname);
		imovpos(fname, nfname); /* move position in positions file */
		release(nfname);
		return Yes;
	}
	else {
		putmess(CANT_GET_FNAME);
		return No;
	}
}

/* Find out if the file exists, and is not empty. Some editors don't
   allow a file to be edited to empty, but insist it should be at least
   one empty line.  Therefore, a file with one, empty, line is also
   considered empty.
   As a side effect, if the file is 'still there', ifile is set to it
   and it remains open, positioned at the beginning.
   (A previous version of this function would leave it positioned after
   an initial \n, if there was one; this version just rewinds the file.)
   */

Hidden bool still_there(fname) value fname; {
	int k;

	ifile= fopen(strval(fname), "r");
	if (ifile == NULL) {
		vs_ifile();
		return No;
	} else {
		if ((k= getc(ifile)) == EOF ||
				(k == '\n' && (k= getc(ifile)) == EOF)) {
			fclose(ifile);
			f_delete(strval(fname));
			vs_ifile();
			return No;
		}
		rewind(ifile);
		return Yes;
	}
}

/* Ensure the unit is filed. If the unit was read non-interactively (eg passed
   as a parameter to abc), it is only held in store.
   Editing it puts it into a file. This is the safest way to copy a unit from
   one workspace to another.
*/

Hidden bool ens_filed(pname, fname, unpacking) value pname, *fname; bool unpacking; {
	value *aa;
	if (p_exists(pname, &aa) && !unpacking) {
		*fname= copy(*aa);
		return Yes;
	} else if (!u_exists(pname, &aa) || How_to(*aa)->unit == NilTree) {
		value name= Permname(pname);
		pprerrV(NO_HOWTO, name);
		release(name);
 		*fname= Vnil;
		return No;
	} else {
		how *du= How_to(*aa); FILE *fp;
		if (du->filed == Yes) {
			syserr(MESS(4019, "ens_filed()"));
			*fname= Vnil;
			return No;
		}
		*fname= get_ufname(pname, No);
		if (!Valid(*fname))
			return No;
		fp= fopen(strval(*fname), "w");
		if (!fp) {
			interrV(CANT_WRITE, *fname);
			release(*fname);
			*fname= Vnil;
			return No;
		} else {
			display(fp, du->unit, No);
			f_close(fp);
			du->filed= Yes;
			return Yes;
		}
	}
}

Hidden int err_line(pname) value pname; {
	value *aa;
	if (!p_exists(last_unit, &aa) || compare(*aa, pname) != 0)
		return 0;
	else
		return cur_env->errlino;
}

/************************** VALUES ***************************************/
/* The permanent environment in the old format was kept as a single file */
/* but this caused slow start ups if the file was big.			 */
/* Thus the new version stores each permanent target on a separate file, */
/* that furthermore is only loaded on demand.				 */
/* To achieve this, a directory is kept of the permanent tags and their  */
/* file names. Care has to be taken that disaster occurring in		 */
/* the middle of an update of this directory does the least harm.	 */
/* Having the directory refer to a non-existent file is considered less  */
/* harmful than leaving a file around that can never be accessed, for	 */
/* instance, so a file is deleted before its directory entry,		 */
/* and so forth.							 */
/*************************************************************************/

Visible value errtname= Vnil;

Hidden Procedure tarfiled(name, v) value name, v; {
	value p= mk_indirect(v);
	def_target(name, p);
	release(p);
}

Visible value last_target= Vnil; /* last edited target */

Visible Procedure del_target(name)
     value name;
{
	value pname = permkey(name, Tar);
	value *aa;
	wsenvptr oldwse;

	oldwse = setcurenv(use_env);

	free_target(name);
	del_perm(pname);
	if (p_exists(last_target, &aa) && (compare(name, *aa) == 0))
		free_perm(last_target);
	release(pname);

	resetcurenv(oldwse);
}

/* get_tfname() is only invoked in the using workspace environment */

Hidden value get_tfname(name) value name; {
	value fname;
	value pname= permkey(name, Tar);
	value *aa;
	
	if (p_exists(pname, &aa))
		fname= copy(*aa);
	else {
		fname= new_fname(name, Tar);
		if (Valid(fname))
			def_perm(pname, fname);
		else
			interrV(CANTGETFNAME, name);
	}
	release(pname);
	return fname;
}

Visible Procedure edit_target() {
	value name= Vnil;
	value fname, *aa;
	if (Ceol(tx)) {
		if (!p_exists(last_target, &aa))
			parerr(MESS(4020, "no current location"));
		else
			name= copy(*aa);
	} else if (!is_tag(&name))
		parerr(CANT_EDIT);
	if (still_ok && ens_tfiled(name, &fname)) {
		ed_target(name, fname);
		release(fname);
	}
	release(name);
}

#define NO_T_WRITE	MESS(4021, "*** you have no write permission in this workspace:\n*** you may not change the location\n*** do you still want to display the location?\n")

/* Edit a target. The value in the target is written to the file,
   and then removed from the internal permanent environment so that
   if a syntax error occurs when reading the value back, the value is
   absent from the internal permanent environment.
   Thus when editing the file to correct the syntax error, the
   file doesn't get overwritten.
   The contents may be completely deleted in which case the target is
   deleted. */

Hidden Procedure ed_target(name, fname) value name, fname; {
	value v;

#ifdef CK_WS_WRITABLE
	if (!ckws_writable(NO_T_WRITE))
	  /* workspace is read-only and user don't want to display */
	  return;
#endif
#ifdef CLEAR_MEM
	clear_perm(); /* To give the editor as much space as possible */
#endif
	def_perm(last_target, name);
	if (!f_edit(fname, 0, '=', No))
		/* File is unchanged */
		return;
	if (!still_there(fname)) {
		del_target(name);
#ifdef SAVE_PERM
		put_perm();
#endif
		return;
	}
	fclose(ifile); /* Since still_there leaves it open */
	/* vs_ifile(); ? */
	v= getval(fname, In_edval);
	if (still_ok) def_target(name, v);
	release(v);
}

#define NO_TARGET MESS(4022, "%s isn't a location in this workspace")

Hidden bool ens_tfiled(name, fname) value name, *fname; {
	value *aa;
	if (!t_exists(name, &aa)) {
		pprerrV(NO_TARGET, name);
		return No;
	} else {
		*fname= get_tfname(name);
		if (!Valid(*fname))
			return No;
		if (!Is_filed(*aa)) {
			puttarval(*aa, cur_dir, *fname, name, No);
			tarfiled(name, *aa);
		}
		return Yes;
	}
}

/***************************** Values on files ****************************/

Visible value getval(fname, ct) value fname; literal ct; {
	char *buf; int k; parsetree w, code= NilTree; value v= Vnil;
	ifile= fopen(strval(fname), "r");
	if (ifile) {
		txptr fcol_save= first_col, tx_save= tx; context c;
		sv_context(&c);
		cntxt= ct;
		buf= (char *) getmem((unsigned)(f_size(ifile)+2)*sizeof(char));
		first_col= tx= ceol= buf;
		while ((k= getc(ifile)) != EOF)
			if (k != '\n') *ceol++= k;
		*ceol= '\n';
		fclose(ifile); vs_ifile();
		w= expr(ceol);
		if (still_ok) fix_nodes(&w, &code);
		curline= w; curlino= one;
		v= evalthread(code); 
		if (!env_ok(v)) {
			release(v);
			v= Vnil;
		}
		curline= Vnil;
		release(w);
		freemem((ptr) buf);
		set_context(&c);
		first_col= fcol_save; tx= tx_save;
	} else {
		interr(CANT_READ);
		vs_ifile();
	}
	return v;
}

Hidden bool env_ok(v) value v; {
	if (cntxt == In_prmnv || cntxt == In_wsgroup) {
		if (!Is_table(v)) {
			interr(MESS(4023, "value is not a table"));
			return No;
		}
		else if (!Is_ELT(v) && !Is_text(*key(v, 0))) {
			interr(MESS(4024, "in t[k], k is not a text"));
			return No;
		}
	}
	return Yes;
}

Visible value gettarval(fname, name)
     value fname;
     value name;
{
	release(errtname); errtname = copy(name);
	return getval(fname, In_tarval);
}

Visible Procedure initperm() {
	if (F_readable(permfile)) {
		value fn, name;
		intlet k, len;
		value v, pname;
		
		fn= mk_text(permfile);
		v= getval(fn, In_prmnv);
		release(fn);
		if (Valid(v)) {
			release(cur_env->perm);
			cur_env->perm= v;
		}
		len= length(cur_env->perm);
		for (k= 0; k < len; k++) {
			pname= *key(cur_env->perm, k);
			if (Permtype(pname) == Tar) {
				name= Permname(pname);
				tarfiled(name, Vnil);
				release(name);
			}
		}
	}
	cur_env->permchanges= No;
}

Visible Procedure putval(v, dir, name, ct, silently)
     value v;
     char *dir;
     char *name;
     literal ct;
     bool silently;
{
	char *temp, *file;
	value tname, fname;
	FILE *fp;
	bool was_ok = still_ok;
	context c;

	/* ensure that both args of f_rename() are on the same file system */
	file = makepath(dir, name);
	temp = makepath(dir, tempfile);
	fname = mk_text(file);
	tname = mk_text(temp);

	sv_context(&c);
	cntxt = ct;
	curline = Vnil;
	curlino = one;
	fp = fopen(temp, "w");
	if (fp != NULL) {
		still_ok = Yes;
		wri(fp, v, No, No, Yes);
		fputc('\n', fp);
		f_close(fp);
		if (still_ok) {
			f_rename(tname, fname);
		}
	}
	else if (!silently) interrV(CANT_WRITE, fname);
	still_ok = was_ok;
	set_context(&c);

	release(tname);
	release(fname);
	free_path(temp);
	free_path(file);
}

Visible Procedure puttarval(v, dir, fname, tname, silently)
     value v;
     char *dir;
     value fname;
     value tname;
     bool silently;
{
	release(errtname); errtname = copy(tname);
	putval(v, dir, strval(fname), In_tarval, silently);
}

Visible Procedure endperm() {
	static bool active;
	bool was_ok= still_ok;
	
	if (active)
		return;
	active= Yes;
	still_ok= Yes;
	put_targs();
	put_perm();
	still_ok= was_ok;
	active= No;
}

Hidden Procedure put_targs() {
	int k, len;
	value v, name;
	
	len= Valid(prmnv->tab) ? length(prmnv->tab) : 0;
	for (k= 0; k < len; k++) {
		v= copy(*assoc(prmnv->tab, k));
		name= copy(*key(prmnv->tab, k));
		if (!Is_filed(v)) {
			value fname= get_tfname(name);
			if (Valid(fname)) {
				puttarval(v, cur_dir, fname, name, Yes);
			}
			release(fname);
		}
		tarfiled(name, Vnil);
		release(v); release(name);
	}
}

Visible Procedure put_perm()
{
	char *dir;
	intlet len;
	value v = cur_env->perm;
	
	if (!cur_env->permchanges || !Valid(v))
		return;
	dir = InUsingEnv() ? cur_dir : cen_dir;
	len = length(v);
	if (len != 0) {
		putval(v, dir, permfile, In_prmnv, Yes);
	}
	else { /* Remove the file if the filename table is empty */
		char *file = makepath(dir, permfile);
		f_delete(file);
		free_path(file);
	}
	cur_env->permchanges= No;
}

Visible Procedure clear_perm() {
	freeunits(USR_ALL);
	endperm();
}

Visible Procedure initsou() {
	release(cur_env->units); cur_env->units= mk_elt();
	release(cur_env->perm); cur_env->perm= mk_elt();
}

Visible Procedure endsou() {
	if (terminated)
		return;	/* hack; to prevent seemingly endless QUIT */
	release(cur_env->units); cur_env->units= Vnil;
	release(cur_env->perm); cur_env->perm= Vnil;
}

/*
 * lst_uhds() displays the first line of the unit without a possible
 * present simple command
 */
 
#define MORE MESS(4025, "Press [SPACE] for more, [RETURN] to exit list")

Hidden bool isunitentry(pname) value pname; {
	if (!Is_text(pname) ||
	    Permtype(pname) == Tar ||
	    (Valid(last_unit) && compare(pname, last_unit) == 0) ||
	    (Valid(last_target) && compare(pname, last_target) == 0) ) {
		return No;
	}
	return Yes;
}

Visible Procedure lst_uhds() {
	intlet k, len;
	value pname, *aa;
	how *u;
	int nprinted= 0;
	bool more= Yes;

	len= length(cur_env->perm);
	for (k= 0; k<len && still_ok && more; ++k) {
		pname= *key(cur_env->perm, k);
		if (!isunitentry(pname))
		        continue;
		/* reduce disk access: */
		if (u_exists(pname, &aa) && !Is_predefined(*aa))
			display(CONSOLE, How_to(*aa)->unit, Yes);
		else {
			lst_fileheading(*assoc(cur_env->perm, k));
		}
		c_flush();
		if (++nprinted >= winheight) {
			more= ask_for(MORE);
			nprinted= 0;
		}
	}
	/* not interactive units */
	len= length(cur_env->units);
	for (k= 0; k<len && still_ok && more; ++k) {
		u= How_to(*assoc(cur_env->units, k));
		if (u -> filed == No && !p_exists(*key(cur_env->units, k), &aa)) {
			display(CONSOLE, u -> unit, Yes);
			c_flush();
			if (++nprinted >= winheight) {
				more= ask_for(MORE);
				nprinted= 0;
			}
		}

	}

}

Hidden Procedure lst_fileheading(v) value v; {
	FILE *fn;
	char *line;
	char *pcolon, *pc;

	if (!Is_text(v))
		return;
	fn= fopen(strval(v), "r");
	if (!fn)
		return;
	if ((line= f_getline(fn)) != NULL) {
		/* returns line from file, the newline character included */
		pcolon= strchr(line, C_COLON);
		if (pcolon != NULL) {
			pc= ++pcolon;
			while (Space(*pc)) ++pc;
			if (*pc != C_COMMENT && *pc != '\n') {
				/* single command after colon;
				 * don't show it.
				 */
				*(pcolon+1)= '\n';
				*(pcolon+2)= '\0';
			}
		}
		c_putstr(line);
		freestr(line);
	}
	fclose(fn);
}
