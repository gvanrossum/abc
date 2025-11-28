/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * read grammar from file into tables.
 *
 * There's a little parser here, to read the grammar from the file.
 * See the file "grammar.abc" for the possible formats.
 *
 * We use namelist[] to store all names. At the end of the reading process
 * the cross-references between classdef[] and symdef[] will be in terms
 * of indices in namelist[]. In fill.c they will be replaced by indices
 * directly into the other one.
 * This organisation is necessary to keep the order of the Symbol-definitions
 * the same as in the input file.
 *
 * Definitions for "Suggestion", "Sugghowname", "Optional" and "Hole" are
 * added at the end; see comment below.
 */

#include "b.h"
#include "main.h"

#define COMMENT '#' /* Not ABC-like but very UNIX-like, and we used cpp ... */
#define QUOTE '"'

Hidden char nextc; /* Next character to be analyzed */
Hidden bool eof; /* EOF seen? */
Hidden int lcount; /* Current line number */
Hidden int errcount; /* Number of errors detected */

Hidden string dname= NULL; /* name currently being defined (at linestart) */
/* VARARGS 1 */
Hidden Procedure error(format, arg1, arg2, arg3, arg4, arg5)
	char *format;
	char *arg1, *arg2, *arg3, *arg4, *arg5;
{
	fprintf(stderr, 
		"%s: error in grammar file %s, line %d, defining name %s\n\t",
		progname, gfile, lcount, (dname==NULL ? "???" : dname));
	fprintf(stderr, format, arg1, arg2, arg3, arg4, arg5);
	putc('\n', stderr);
	errcount++;
}

Visible Procedure read_grammar_into_tables() {
	errcount= 0;
	lcount= 1;
	eof= No;
	do {
		adv();
		skipspace();
		if (nextc != COMMENT && nextc != '\n')
			getdefinition();
		while (nextc != '\n')
			adv();
		lcount++;
	} while (!eof);
	
	if (errcount > 0) {
		fatal("You 'd better fix that grammar description first");
	}
	
	add_special_definitions();
}

Hidden Procedure adv()
{
	int c;

	if (eof)
		return;
	c= getc(gfp);
	if (c == EOF) {
		nextc= '\n';
		eof= Yes;
	}
	else {
		nextc= c;
	}
}

Hidden Procedure skipspace()
{
	while (nextc == ' ' || nextc == '\t')
		adv();
}

Hidden Procedure skipwhite()
{
	while (nextc == ' ' || nextc == '\t' || nextc == '\n') {
		if (nextc == '\n')
			lcount++;
		adv();
	}
}

Hidden Procedure skipdef()	/* to synchronize after error in def */
{				/* assumes at least points are allright */
	while (nextc != '.') {
		adv();
	}
}

Hidden Procedure skipstring()	/* idem for string, must end with '"' */
{
	while (nextc != '\"') {
		adv();
	}
}

Hidden string getname() {
	char buffer[NAMELEN];
	string bp;
	
	if (!isascii(nextc) || !isalpha(nextc)) {
		if (!isascii(nextc) || (!isprint(nextc) && nextc != ' '))
			sprintf(buffer, "\\%03o", nextc);
		else
			sprintf(buffer, "'%c'", nextc);
		error("illegal character at start of name: %s", buffer);
		return NULL;
	}
	bp= buffer;
	*bp++= nextc;
	adv();
	while (isascii(nextc)
		&&
	       (isalnum(nextc) || nextc == '_')
	      ) {
		if (bp < buffer + sizeof buffer - 1)
			*bp++= nextc;
		adv();
	}
	*bp= '\0';
	return savestr((string)buffer);
}

Hidden string getstring()
{
	char buf[STRINGLEN]; /* Arbitrary limit */
	char c;
	int len= 0;

	if (nextc != QUOTE) {
		return NULL;
	}
	adv();
	while (nextc != QUOTE) {
		if (nextc == '\n') {
			error("end of line in string");
			skipstring();
			break;
		}
		if (nextc != '\\') {
			c= nextc;
			adv();
		}
		else {
			adv();
			switch (nextc) {

			case 'r': c= '\r'; adv(); break;
			case 'n': c= '\n'; adv(); break;
			case 'b': c= '\b'; adv(); break;
			case 't': c= '\t'; adv(); break;
			case 'f': c= '\f'; adv(); break;
			case 'v': c= '\v'; adv(); break;
			/* '\\', '\'' and '\"' handled by default below */

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c= nextc-'0';
				adv();
				if (nextc >= '0' && nextc < '8') {
					c= 8*c + nextc-'0';
					adv();
					if (nextc >= '0' && nextc < '8') {
						c= 8*c + nextc-'0';
						adv();
					}
				}
				break;

			default: c=nextc; adv(); break;

			}
		}
		if (len >= (sizeof(buf)-1)) {
			error("string too long");
			skipstring();
			len= sizeof(buf)-1;
			break;
		}
		buf[len++]= c;
	}
	adv();
	buf[len]= '\0';
	return savestr((string)buf);
}

Hidden Procedure storename(name, pi, pt) string name; item *pi; char *pt; {
	int iname;
	struct nameinfo *pname;
	char *pc;
	char type;
	
	for (iname= 0; iname < nname; iname++) {
		pname= &namelist[iname];
		if (strcmp(name, pname->n_name) == 0) {
			/* stored already */
			*pi= (item) iname;
			*pt= pname->n_type;
			return;
		}
	}
	/* not stored yet; reserve entry and check type */
	Assert(nname < maxname);
	type= Errtype;
	if (isupper(name[0]) && isupper(name[1])) {
		for (pc= &name[2]; *pc != '\0'; pc++)
			if (isalpha(*pc) && !isupper(*pc))
				break;
		if (*pc == '\0')
			type= Lex;
	}
	if (isupper(name[0]) && islower(name[1])) {
		for (pc= &name[2]; *pc != '\0'; pc++)
			if (isalpha(*pc) && !islower(*pc))
				break;
		if (*pc == '\0')
			type= Sym;
	}
	if (islower(name[0])) {
		for (pc= &name[1]; *pc != '\0'; pc++)
			if (isalpha(*pc) && !islower(*pc))
				break;
		if (*pc == '\0')
			type= Class;
	}
	*pt= type;
	if (type == Errtype)
		error("cannot determine type of name '%s'", name);
	pname= &namelist[nname];
	pname->n_name= name;
	pname->n_type= type;
	pname->n_index= Nilitem;	/* filled in iff definition found */
	*pi= (item) nname;
	nname++;
}

Hidden Procedure getdefinition()
{
	string defname;
	item defitem;
	char deftype;
	
	defname= getname();
	if (defname == NULL)
		return;
	dname= defname;
	
	storename(defname, &defitem, &deftype);
	
	skipwhite();
	if (nextc != ':') {
		error("defined name not followed by ':'");
		dname= NULL;
		return;
	}
	adv();
	skipwhite();
	
	switch (deftype) {
	case Class:
		getclassdef(defname, defitem);
		break;
	case Sym:
		getsymdef(defname, defitem);
		break;
	case Lex:
		getlexdef(defname, defitem);
		break;
	case Errtype:
	default:
		error("skipping definition");
		break;
	}
	
	dname= NULL;
}

Hidden Procedure getclassdef(defname, defitem) string defname; item defitem; {
	int iclass;
	string sname;
	item sitem;
	char stype;
	item symarray[SYMLEN];
	int s;
	
	iclass= nclass++;
	namelist[defitem].n_index= iclass;
	classdef[iclass].c_name= defname;
	
	for (s= 0; s < SYMLEN-1; s++) {
		sname= getname(); 
		if (sname == NULL) {
			error("giving up this definition");
			skipdef();
			break;
		}
		storename(sname, &sitem, &stype);
		if (stype == Sym || stype == Lex) {
			symarray[s]= sitem;
		}
		else if (stype == Class) {
			error("class '%s' used in class definition", sname);
		}
		
		skipwhite();
		if (nextc == '.') {
			break;
		}
		else if (nextc != ';') {
			error("missing ';'");
		}
		else {
			adv();
		}
		skipwhite();
	}
	if (s == SYMLEN-1 && nextc != '.') {
 error("too many alternatives in rule; skipping tail of definition");
 		skipdef();
	}
	else {
		s++;
	}
	adv();	/* skip '.' */
	symarray[s]= Nilitem;
	classdef[iclass].c_syms= savearray(symarray, s+1);
	classdef[iclass].c_insert= NULL;
	classdef[iclass].c_append= NULL;
	classdef[iclass].c_join= NULL;
}

Hidden Procedure getsymdef(defname, defitem) string defname; item defitem; {
	int isym;
	struct syminfo *psym;
	string str;
	string cname;
	item citem;
	char ctype;
	int ich;
	
	isym= nsym++;
	namelist[defitem].n_index= isym;
	
	psym= &symdef[isym];
	psym->s_name= defname;
	
	for (ich= 0; ich <= MAXCHILD; ich++) {
		str= getstring();
		psym->s_repr[ich]= str;
		
		if (str != NULL) {
			skipwhite();
			if (nextc == '.')
				break;	/* for ich */
			else if (nextc == ',') {
				adv();
				skipwhite();
			}
			else {
				error("missing ','");
			}
		}
		
		if (ich == MAXCHILD) {
			error("too many children in Symbol definition");
			skipdef();
			break;
		}
		
		cname= getname(); 
		if (cname == NULL) {
			error("missing class name");
			skipdef();
			break;
		}
		storename(cname, &citem, &ctype);
		if (ctype == Class || ctype == Lex) {
			psym->s_class[ich]= citem;
		}
		else if (ctype == Sym) {
			error("Symbol '%s' used in Symbol definition", cname);
		}
		
		skipwhite();
		if (nextc == '.') {
			/* ich < MAXCHILD */
			ich++;
			psym->s_repr[ich]= NULL;
			break;
		}
		else if (nextc != ',') {
			error("missing ','");
		}
		else {
			adv();
			skipwhite();
		}
	}
	
	if (nextc == '.') {
		adv();
	}
	while (ich < MAXCHILD) {
		psym->s_class[ich]= Nilitem;
		ich++;
		psym->s_repr[ich]= NULL;
	}
}

Hidden item nilarray[]= {Nilitem, Nilitem};

Forward string bodyname();

Hidden Procedure getlexdef(defname, defitem) string defname; item defitem; {
	int ilex;
	struct lexinfo *plex;
	string str1;
	string str2;
	struct classinfo *pclass;
	struct syminfo *psym;
	int ich;

	ilex= nlex++;
	namelist[defitem].n_index= ilex;
	
	plex= &lexdef[ilex];
	plex->l_name= defname;
	
	str1= getstring();
	if (str1 == NULL) {
		error("no string of start chars in lexical definition");
		skipdef();
		return;
	}
	plex->l_start= str1;
	skipwhite();
	if (nextc != ',') {
		error("missing ',' between start and continuation string");
	}
	else {
		adv();
		skipwhite();
	}
	str2= getstring();
	if (str2 == NULL) {
		error("no string of continuation chars in lexical definition");
		skipdef();
		return;
	}
	plex->l_cont= str2;
	skipwhite();
	if (nextc != '.') {
		error("missing '.' after lexical definition");
	}
	else {
		adv();
	}
	/* And now the tricky part:
	 * the lexical will be enveloped in the following definitions:
	 *	l_body: LEXICAL.
	 *	L_sym: l_body.
	 *	l_class: L_sym.
	 * Wherever the lexical is used in a class or symbol definition
	 * the latter two definitions will be used.
	 * The first is only referenced indirectly.
	 * Even Guido forgot why this was necessary for the ABC editor.
	 *
	 * Here we only reserve the space, and keep the indexes.
	 * The names must be converted into legal C identifiers
	 * differing from the original one. (they will show up
	 * in a generated headerfile as debugging info).
	 * The definitions must be filled with Nil's to prevent them
	 * from being interpreted as namelist-indices in the replacement
	 * process in fill.c. There the correct definitions will be filled in.
	 *
	 * For "SUGGESTION" we only do the first step; an entry for
	 *	Suggestion: suggestion_body.
	 * will be added below in add_special_definitions().
	 * Idem for "SUGGHOWNAME".
	 */
	pclass= &classdef[nclass];
	pclass->c_name= bodyname(defname);
	pclass->c_syms= savearray(nilarray, 2);
	pclass->c_insert= NULL;
	pclass->c_append= NULL;
	pclass->c_join= NULL;
	plex->l_body= nclass++;
	
	if (strcmp(defname, "SUGGESTION") == 0) {
		lsuggestion= ilex;	/* later needed for filling in */
		nsuggstnbody= nclass-1;	/* also used to check presence */
		return;
	}
	if (strcmp(defname, "SUGGHOWNAME") == 0) {
		lsugghowname= ilex;	/* later needed for filling in */
		nsugghowbody= nclass-1;	/* also used to check presence */
		return;
	}
	
	psym= &symdef[nsym];
	psym->s_name= savestr(defname);
	symname(psym->s_name);
	for (ich= 0; ; ich++) {
		psym->s_repr[ich]= NULL;
		if (ich == MAXCHILD)
			break;
		psym->s_class[ich]= Nilitem;
	}
	plex->l_sym= nsym++;
	
	pclass= &classdef[nclass];
	pclass->c_name= savestr(defname);
	classname(pclass->c_name);
	pclass->c_syms= savearray(nilarray, 2);
	pclass->c_insert= NULL;
	pclass->c_append= NULL;
	pclass->c_join= NULL;
	plex->l_class= nclass++;
}

Hidden string bodyname(s) string s; {
	char lexbuffer[NAMELEN];
	
	strcpy(lexbuffer, s);
	classname(lexbuffer);
	strcat(lexbuffer, "-body");
	return savestr((string)lexbuffer);
}

Hidden Procedure symname(s) string s; {	
	string t= s+1;
	char c;
	
	while (*t) {
		if (isupper(*t)) {
			c= tolower(*t);
			*t= c;
		}
		t++;
	}
}

Hidden Procedure classname(s) string s; {	
	string t= s;
	char c;
	
	while (*t) {
		if (isupper(*t)) {
			c= tolower(*t);
			*t= c;
		}
		t++;
	}
}

/* At the end we must add two Symbol definitions
 * that could not be entered in the grammar:
 *	Optional: .
 *	Hole: "?".
 * The ABC editor expects these to be at the end of the symdef[] table.
 *
 * Just before that entries for:
 * 	Suggestion: suggestion_body.
 *	Sugghowname: sugghowname_body.
 * will be defined iff the corresponding lexical symbol has
 * been defined in the grammar.
 *
 * 'Suggestion', 'Sugghowname' and 'Optional' are already in the namelist[],
 * but still undefined.
 * To replace the references made to them (later, in fill_and_check_tables())
 * we must add their definitions here first, mimicking the reading procedure.
 *
 * 'Hole' should not be used, only by the ABC editor, so we don't
 * bother about any links to it. (check_defined() will fail if this
 * is violated).
 */

Hidden Procedure add_special_definitions() {
	
	if (lsuggestion >= 0) {	/* SUGGESTION defined */
		add_symbol("Suggestion", &nsuggestion, Yes);
	}
	if (lsugghowname >= 0) { /* SUGGHOWNAME defined */
		add_symbol("Sugghowname", &nsugghowname, Yes);
	}
	
	add_symbol("Optional", &noptional, Yes);
	add_symbol("Hole", &nhole, No);
	symdef[nhole].s_repr[0]= "?";
}

Hidden Procedure add_symbol(name, pn, referenced)
string name; int *pn; bool referenced;
{
	struct syminfo *psym;
	item i;
	char t;
	int ich;
	
	*pn= nsym++;
	if (referenced) {
		storename(name, &i, &t);
		namelist[i].n_index= *pn;
	}
	psym= &symdef[*pn];
	psym->s_name= name;
	for (ich= 0; ; ich++) {
		psym->s_repr[ich]= NULL;
		if (ich == MAXCHILD)
			break;
		psym->s_class[ich]= Nilitem;
	}
}
