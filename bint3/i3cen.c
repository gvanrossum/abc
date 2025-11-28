/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1991. */

#include "b.h"
#include "bint.h"
#include "bfil.h"
#include "bmem.h"
#include "bobj.h"
#include "i3bws.h"
#include "i3cen.h"
#include "i3env.h"
#include "i3sou.h"
#include "port.h"

/* The central workspace looks like any other workspace.
 * The difference is that you can call the how-to's of the central workspace
 * in the other workspaces.
 * However, the environment of the global locations inside central how-to's
 * is the current using workspace.
 * If you want to edit a central how-to, you have to switch to the central
 * workspace.
 */

Forward Hidden value getcentralinfo();
Forward Hidden bool unitinenv();
Forward Hidden char *mkfilepath();

Hidden wsenv useenv = {Vnil, Vnil, Vnil, No, No, 0}; /* using workspace env */
Hidden wsenv cenenv = {Vnil, Vnil, Vnil, No, No, 0}; /* central workspace env */
Hidden wsenv stdenv = {Vnil, Vnil, Vnil, No, No, 0}; /* standard fun/prd env */

Visible wsenvptr use_env = &useenv; /* pointer to env of using workspace */
Visible wsenvptr cen_env = &cenenv; /* pointer to env of central workspace */
Visible wsenvptr std_env = &stdenv; /* pointer to env of standard fun/prd */
Visible wsenvptr cur_env;           /* pointer to current env */

Hidden bool iscentralws = No; /* does exist a central workspace ? */
Visible bool incentralws = No; /* is the current workspace the central one ? */
Visible char *cen_dir;        /* absolute path to central workspace */

/* initcurenv()
 * Initialize global variable 'cur_env'
 */

Visible Procedure initcurenv()
{
	cur_env = use_env;
}

/* setcurenv()
 * Set 'cur_env' and returns the old value
 */

Visible wsenvptr setcurenv(wse)
     wsenvptr wse;
{
	wsenvptr old = cur_env;
	cur_env = wse;
	return old;
}

/* resetcurenv()
 * Reset 'cur_env'
 */

Visible Procedure resetcurenv(wse)
     wsenvptr wse;
{
	cur_env = wse;
}

/*****************************************************************************/

/* initcentralworkspace()
 * Initialize central workspace adm at startup or after a workspace change.
 */

Visible Procedure initcentralworkspace(startup)
     bool startup;
{
	iscentralws = (cen_dir != (char *)0 && D_exists(cen_dir));
	incentralws = (iscentralws && strcmp(cen_dir, cur_dir) == 0);

	if (startup && !incentralws) {
		/* read central perm/types info from disk */
		release(cen_env->perm);
		cen_env->perm = getcentralinfo(permfile);
		release(cen_env->abctypes);
		cen_env->abctypes = getcentralinfo(typesfile);
		if (iscentralws)
			initcensugg();
	}
	if (incentralws) {
		release(cen_env->perm);
		cen_env->perm = mk_elt();
		release(cen_env->abctypes);
		cen_env->abctypes = mk_elt();
		use_env->errlino = cen_env->errlino;
	}
	release(cen_env->units);
	cen_env->units = mk_elt();
}

/* endcentralworkspace()
 * Finish central workspace adm before a workspace change or at QUITing.
 */

Visible Procedure endcentralworkspace(last)
     bool last;
{
	if (!incentralws && iscentralws) {
		/* write central perm/types info to disk */
		wsenvptr old_env = setcurenv(cen_env);
		put_perm();
		put_types();
		resetcurenv(old_env);
	}
	if (!last) {
		if (incentralws) {
			/* save central perm/types info */
			release(cen_env->perm);
			cen_env->perm = copy(use_env->perm);
			release(cen_env->abctypes);
			cen_env->abctypes = copy(use_env->abctypes);
			cen_env->errlino = use_env->errlino;
		}
		release(cen_env->units);
		cen_env->units = Vnil;
	}
#ifdef MEMTRACE
	else {
		release(cen_env->perm);
		cen_env->perm = Vnil;
		release(cen_env->abctypes);
		cen_env->abctypes = Vnil;
		release(cen_env->units);
		cen_env->units = Vnil;
	}
#endif
}

/*****************************************************************************/

/* getcentralinfo()
 * Read the filename/typecode mapping of the central workspace.
 */

Hidden value getcentralinfo(file)
     string file;
{
	char *cenfile;
	value v = Vnil;

	if (!iscentralws) /* no central workspace */
		return mk_elt();
	cenfile = makepath(cen_dir, file);
	if (F_readable(cenfile)) {
		value fname = mk_text(cenfile);
		v = getval(fname, In_prmnv);
		if (!still_ok) {
			release(v);
			v = Vnil;
			still_ok = Yes;
		}
		release(fname);
	}
	free_path(cenfile);
	return Valid(v) ? v : mk_elt();
}

/*****************************************************************************/

/* is_unit()
 * Does exist the unit with id 'name, type' in the using/central/standard env 
 * AND, if the unit is loaded, *without* faults ?
 * Returns pointer to unit in 'howto', if 'howto' isn't nil.
 * Returns current env in 'wse', if 'wse' isn't nil
 */

Visible bool is_unit(name, type, howto, wse)
     value name;
     literal type;
     value **howto;   /* if non-NULL set to root of loaded unit */
     wsenvptr *wse;   /* if non-NULL set to workspace env of unit */
{
	value pname = permkey(name, type);
	bool is = Yes;
	wsenvptr w = (wsenvptr)0;
	bool gethowenv = (wse != (wsenvptr *)0);

	if (unitinenv(pname, howto, cur_env)) {
		w = cur_env;
	}
	else if (InUsingEnv() && unitinenv(pname, howto, cen_env)) {
		w = cen_env;
	}
	else if (type != Cmd && unitinenv(pname, howto, std_env)) {
		w = std_env;
	}
	else is = No;

	release(pname);
	if (gethowenv) *wse = w;
	return is;
}

/* unitinenv()
 * Does exist the unit with id 'pname' in the env 'wse'
 * AND, if the unit is loaded, *without* faults ?
 * Returns pointer to unit in 'howto', if 'howto' isn't nil.
 */

Hidden bool unitinenv(pname, howto, wse)
     value pname;
     value **howto; /* if non-NULL set to root of loaded unit */
     wsenvptr wse;
{
	value *h;
	value *fname;
	bool rethow = (howto != (value **)0);
	bool is;
	wsenvptr old_env;

/*	if (IsCentralEnv(wse) && (!iscentralws || incentralws))
		return No;
*/
	old_env = setcurenv(wse);

	if (u_exists(pname, &h)) { /* how-to mapping ? */
		if (rethow) *howto = h;
		is = Yes;
	}
	else if (IsStandardEnv(wse)) { /* in standard env */
		is = No;
	}
	else if (!p_exists(pname, &fname)) { /* filename mapping ? */
		is = No;
	}
	else if (!rethow) { /* don't load unit */
		is = Yes;
	}
	else { /* load unit */
		char *name = mkfilepath(*fname, wse);
		is = is_loaded(name, pname, &h);
		freestr(name);
		if (is) *howto = h;
	}
	resetcurenv(old_env);
	return is;
}

/*****************************************************************************/

Hidden char *mkfilepath(fname, wse)
     value fname;
     wsenvptr wse;
{
	if (IsCentralEnv(wse)) return makepath(cen_dir, strval(fname));
	else return (char *) savestr(strval(fname));
}

/*****************************************************************************/

/* initstdenv()
 * Initialize the env of the standard functions/predicates
 */

Visible Procedure initstdenv() 
{
	std_env->units = mk_elt();
}

/* endstdenv()
 * Finish the env of the standard functions/predicates
 */

Visible Procedure endstdenv()
{
#ifdef MEMTRACE
	release(std_env->units);
	std_env->units = Vnil;
#endif
}

