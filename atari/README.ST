	
	ABC for the Atari ST
	====================

There are five files supplied: abc.tos, the program itself,
abc.msg, the error message file, abc.hlp the help file, abckeys.tos,
the program to redefine the keybindings for the ABC editor, and
abcintro.doc, which gives a quick introduction to ABC.

If you start ABC up from the desktop, the messages file must be either in the
same directory as the abc.tos file, or in the workspaces directory abc.
If you start ABC up from a shell, the messages file must be in the
current directory, or in the abc directory.

If you start ABC up from the desktop, and you want to use the options,
like -w, you should rename abc.tos to abc.ttp.

There is an additional facility for redirecting input and output:
the parameter >outfile, or 1>outfile, redirects all output from ABC
to the file called outfile, the parameter 2>errfile redirects all
error messages from ABC to the file called errfile, and <infile,
or 0<infile, takes its input from the file called infile.

Unfortunately, there is a bug in earlier versions of the Atari
desktop: all lower-case letters passed as parameters are converted to
upper-case. This means that you would not be able to use any of the
lower-case letter options -g -w -c -e -r -i -o -l -p -u -r -x.

For this reason, the Atari ST version also accepts the upper-case options
-G -W -C -E -I -O -L -P -U -R -X, as equivalent to the lower-case versions.

This restriction only applies if you start ABC up from the desktop.
Starting it from a shell, you can use lower-case letters as usual.
