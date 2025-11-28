/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/*
 * Dump info in files for ABC editor.
 */

#include "b.h"
#include "main.h"

#define Copyright \
 "/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */\n\n"
#define Warning \
 "/* WARNING: this file is constructed by 'mktable'. */\n"
#define Change \
 "/* If you want to change the grammar, see ../boot/README. */\n\n"

Visible Procedure dump_files() {
	
	dump_tables();
	
	dump_include();
}
	
Hidden Procedure dump_tables() {
	
	dump_title(tfp, "Data file with grammar tables");
	fprintf(tfp, "#include \"b.h\"\n");
	fprintf(tfp, "#include \"bedi.h\"\n");
	fprintf(tfp, "#include \"%s\"\n\n", hfile);
	dump_classdef();
	dump_symdef();
	dump_lexdef();
}

Hidden Procedure dump_title(xfp, title) FILE *xfp; string title; {
	fprintf(xfp, Copyright);
	fprintf(xfp, "/* %s. */\n\n", title);
	fprintf(xfp, Warning);
	fprintf(xfp, Change);
}

Hidden Procedure dump_classdef() {
	struct classinfo *pclass;
	int iclass;
	
	for (iclass= 0; iclass < nclass; iclass++) {
		pclass= &classdef[iclass];
		dump_array(pclass->c_syms, 's', iclass);
		dump_array(pclass->c_insert, 'i', iclass);
		dump_array(pclass->c_append, 'a', iclass);
		dump_array(pclass->c_join, 'j', iclass);
	}
	fprintf(tfp, "\nstruct classinfo cl[%d] = {\n", nclass);
	for (iclass= 0; iclass < nclass; iclass++) {
		pclass= &classdef[iclass];
		fprintf(tfp, "   {");
		dump_adr(pclass->c_syms, 's', iclass);
		fprintf(tfp, ", ");
		dump_adr(pclass->c_insert, 'i', iclass);
		fprintf(tfp, ", ");
		dump_adr(pclass->c_append, 'a', iclass);
		fprintf(tfp, ", ");
		dump_adr(pclass->c_join, 'j', iclass);
		if (iclass < nclass-1)
			fprintf(tfp, "},");
		else
			fprintf(tfp, "}");
		fprintf(tfp, "	/* %s */\n", pclass->c_name);
	}
	fprintf(tfp, "};\n\n");
}	

Hidden Procedure dump_array(parray, ch, icl) itemptr parray; char ch; int icl; {
	int w;	/* guess line width */
	int a;
	
	if (parray == NULL)
		return;
	fprintf(tfp, "classelem %c%d[]= {", ch, icl);
	w= 18;
	while (!Isnilitem(*parray)) {
		if (w >= 70) {
			fprintf(tfp, "\n\t");
			w= 8;
		}
		a= (int)*parray;
		fprintf(tfp, "%d,", a);
		w+= (a>99 ? 4: a>9 ? 3: 2);
		parray++;
	}
	fprintf(tfp, "0};\n");
}

Hidden Procedure dump_adr(parray, ch, icl) itemptr parray; char ch; int icl; {
	
	if (parray == NULL)
		fprintf(tfp, "0");
	else
		fprintf(tfp, "%c%d", ch, icl);
}

Hidden Procedure dump_symdef() {
	int isym;

	fprintf(tfp, "static struct table abc_grammar[%d] = {\n", nsym);
	for (isym= 0; isym < nsym; ++isym)
		dumpsymbol(isym);
	fprintf(tfp, "};\n\n");
	
	fprintf(tfp, "struct table *table= abc_grammar;\n");
}
	
Hidden Procedure dumpsymbol(isym) int isym; {
	struct syminfo *psym;
	int ich;
	
	fprintf(tfp, "   /* %-3d */ {", isym);
	psym= &symdef[isym];
	dumpstring(psym->s_name);
	fprintf(tfp, ", {");
	for (ich= 0; ich <= MAXCHILD; ++ich) {
		dumpstring(psym->s_repr[ich]);
		if (ich == MAXCHILD)
			break;
		fprintf(tfp, ",");
	}
	fprintf(tfp, "}, {");
	for (ich= 0; ich < MAXCHILD; ++ich) {
		dump_cl(psym->s_class[ich]);
		if (ich == MAXCHILD-1)
			break;
		fprintf(tfp, ",");
	}
	fprintf(tfp, "}, 0}%s\n",  (isym==nsym-1 ? "" : ","));
}

Hidden Procedure dumpstring(s) string s; {
	char c;
	
	if (s == NULL) {
		fprintf(tfp, "0");
		return;
	}
	fputc('"', tfp);
	for (; (c= *s) != '\0'; ++s) {
		if (c == '\b')
			fprintf(tfp, "\\b");
		else if (c == '\t')
			fprintf(tfp, "\\t");
		else if (c == '\n')
			fprintf(tfp, "\\n");
		else if (c == '\\' || c == '"')
			fprintf(tfp, "\\%c", c);
		else
			fputc(c, tfp);
	}
	fprintf(tfp, "\"");
}

Hidden Procedure dump_cl(ind) item ind; {
	if (ind >= 0)
		fprintf(tfp, "&cl[%d]", ind);
	else
		fprintf(tfp, "0");
}

Hidden Procedure dump_lexdef() {
	int ilex;

	fprintf(tfp, "\nstatic struct lexinfo abc_lexicals[%d] = {\n", nlex);
	for (ilex= 0; ilex < nlex; ++ilex)
		dumplex(&lexdef[ilex], (ilex==nlex-1 ? "" : ","));
	fprintf(tfp, "};\n\n");
	
	fprintf(tfp, "struct lexinfo *lextab= abc_lexicals;\n");
}

Hidden Procedure dumplex(plex, sep) struct lexinfo *plex; string sep; {
	
	fprintf(tfp, "   {");
	dumpstring(plex->l_start);
	fprintf(tfp, ", ");
	dumpstring(plex->l_cont);
	fprintf(tfp, "}%s	/* %s */\n", sep, plex->l_name);
}
	
Hidden Procedure dump_include() {
	
	dump_title(ifp, "Header file with grammar table structure");
	if (nsym+nlex < 128)
		fprintf(ifp, "typedef char classelem;\n");
	else
		fprintf(ifp, "typedef short classelem;\n");
	
	fprintf(ifp, "typedef classelem *classptr;\n\n");
	
	fprintf(ifp, "struct classinfo {\n");
	fprintf(ifp, "   classptr c_class;\n");
	fprintf(ifp, "   classptr c_insert;\n");
	fprintf(ifp, "   classptr c_append;\n");
	fprintf(ifp, "   classptr c_join;\n");
	fprintf(ifp, "};\n\n");
	
	fprintf(ifp, "#define MAXCHILD %d\n\n", MAXCHILD);
	
	fprintf(ifp, "struct table {\n");
	fprintf(ifp, "   string r_name;\n");
	fprintf(ifp, "   string r_repr[MAXCHILD+1];\n");
	fprintf(ifp, "   struct classinfo *r_class[MAXCHILD];\n");
	fprintf(ifp, "   node r_node;\n");
	fprintf(ifp, "};\n\n");
	
	fprintf(ifp, "extern struct table *table;\n");
	fprintf(ifp, "#define TABLEN %d\n", nsym);
	
	fprintf(ifp, "struct lexinfo {\n");
	fprintf(ifp, "   string l_start;\n");
	fprintf(ifp, "   string l_continue;\n");
	fprintf(ifp, "};\n\n");
	
	fprintf(ifp, "extern struct lexinfo *lextab;\n");
	
	dump_isymdef();
	
	dump_ilexdef();
}

Hidden Procedure dump_isymdef() {
	int isym;
	
	fprintf(ifp, "\n/* Symbols indexing grammar table */\n\n");
	for (isym= 0; isym < nsym; isym++) {
		fprintf(ifp, "#define %s %d\n", symdef[isym].s_name, isym);
	}
}

Hidden Procedure dump_ilexdef() {
	int ilex;
	
	fprintf(ifp, "\n/* LEXICAL symbols */\n");
	fprintf(ifp, "\n#define LEXICAL %d\n\n", nlexical);
	for (ilex= 0; ilex < nlex; ilex++) {
		fprintf(ifp, "#define %s %d\n", 
			lexdef[ilex].l_name, nlexical+ilex);
	}
	fprintf(ifp, "\n#define NLEX %d\n", nlex);
}
