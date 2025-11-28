/* Transformed messages from stdin file to Rez input on stdout.
   Store resources (100*k)+i as string i+1 in STR# with ID=k.
   The input is assumed to be sorted but not contiguous.
   The initial number, optional star, and tab are removed. */

#include <stdio.h>
#include <ctype.h>

char *strchr();

int progress;

main(argc, argv)
	int argc;
	char **argv;
{
	char buf[512];
	int id= 0;
	int nstr= 0;
	int i;
	char *cp;
	
	progress= argc > 1 && strcmp(argv[1], "-p") == 0;
	
	for (;;) {
		if (fgets(buf, sizeof buf, stdin) == NULL)
			break;
		i= atoi(buf);
		if (i <= 0) {
			fprintf(stderr, "bad line: %s\n", buf);
			continue;
		}
		if (id != i/100) {
			finish(nstr);
			start(id= i/100);
			nstr= 0;
		}
		i= i%100;
		while (i > nstr) {
			if (nstr > 0)
				printf(";\n");
			printf("\"\"");
			++nstr;
		}
		cp= strchr(buf, '\n');
		if (cp)
			*cp= '\0';
		cp= buf;
		while (*cp != '\0' && !isspace(*cp))
			++cp;
		if (*cp == '\t' || *cp == ' ')
			++cp;
		if (nstr > 0)
			printf(";\n");
		printf("\"%s\"", cp);
		++nstr;
	}
	finish(nstr);
	exit(0);
}

start(id)
	int id;
{
	printf("resource 'STR#' (%d) {{\n", id);
	if (progress) {
		fprintf(stderr, "ID=%d...", id);
		fflush(stderr);
	}
}

finish(nstr)
	int nstr;
{
	if (nstr > 0) {
		printf("\n}};\n");
		if (progress)
			fprintf(stderr, " %d strings\n", nstr);
	}
}
