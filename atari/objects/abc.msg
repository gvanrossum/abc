100	removing non-existent list entry
101	cannot remove from large range
102	cannot insert in large range
103	in keys t, t is not a table
104	in t[k], t is not a table
105	in t[k], k is not a key of t
106*	comp_text (106)
200	in t|n, t is not a text
201	in t|n, n is not a number
202	in t|n, n is not an integer
203	in t|n, n is < 0
204	in t@n, t is not a text
205	in t@n, n is not a number
206	in t@n, n is not an integer
207	in t@n, n is > #t + 1
208	in t^u, t or u is not a text
209	in t^u, the result is too long
210	in t^^n, t is not a text
211	in t^^n, n is not a number
212	in t^^n, n is not an integer
213	in t^^n, n is negative
214	in t^^n, the result is too long
215*	charval on non-char (215)
216*	strval on big text (216)
217*	curtail on very big text (217)
218*	behead on very big text (218)
219*	concat on very big text (219)
300	in #t, t is not a text list or table
301	in e#t, t is not a text list or table
302	in e#t, t is a text, but e is not a character
303	in min t, t is not a text list or table
304	in min t, t is empty
305	in max t, t is not a text list or table
306	in max t, t is empty
307	in e min t, t is not a text list or table
308	in e min t, t is empty
309	in e min t, t is a text, but e is not a character
310	in e min t, no element of t exceeds e
311	in e max t, t is not a text list or table
312	in e max t, t is empty
313	in e max t, t is a text, but e is not a character
314	in e max t, no element of t is less than e
315	in t item n, t is not a text list or table
316	in t item n, t is empty
317	in t item n, n is not a number
318	in t item n, n is not an integer
319	in t item n, n is < 1
320	in t item n, n exceeds #t
321	in n th'of t, t is not a text list or table
322	in n th'of t, t is empty
323	in n th'of t, n is not a number
324	in n th'of t, n is not an integer
325	in n th'of t, n is < 1
326	in n th'of t, n exceeds #t
327*	Bigsize in Bottom or Crange (327)
400*	unknown flag in ccopybtreenode (400)
401*	releasing unreferenced btreenode (401)
402*	wrong flag in relbtree() (402)
500	incompatible types %s and %s
501*	comparison of unknown types (501)
502*	hash called with unknown type (502)
503*	unknown type in convert (503)
600	in x mod y, y is zero
601	in n round x, n is not an integer
602	in */n, n is an approximate number
603	in /*n, n is an approximate number
604	in n root x, n is zero
605	in root x, x is negative
606	result of math function too large
607	argument to math function too large
608	math library error
609	in log x, x <= 0
610	in b log x, b <= 0
611	in b log x, x <= 0
700	approximate number too large
701*	app_floor: result not integral (701)
800*	numconst: can't happen (800)
801	excessive exponent in e-notation
900*	dig_gcd of number(s) <= 0 (900)
901*	gcd_small of numbers > smallint (901)
902*	gcd of number(s) <= 0 (902)
903	exceptionally large rational number
1000*	dig_gadd: nto < nfrom (1000)
1001*	int_tento(-n) (1001)
1100*	zero division (int_ldiv) (1100)
1101*	int_ldiv internal failure (1101)
1200*	mk_rat(x, y) with y=0 (1200)
1300	number not an integer
1301	exceedingly large integer
1302*	intval on non-number (1302)
1303*	num_comp (1303)
1304	value not a number
1305	approximate number too large to be handled
1306	exceptionally large number
1400	in p..q, p is neither a text nor a number
1401	in p..q, p is a number but not an integer
1402	in p..q, p is a number, but q is not
1403	in p..q, q is a number but not an integer
1404	in p..q, p is a text but not a character
1405	in p..q, p is a text, but q is not
1406	in p..q, q is a text, but not a character
1500*	big grabber (1500)
1501*	big regrabber (1501)
1502*	getsyze called with unknown type (1502)
1503*	releasing unreferenced value (1503)
1600	in choice t, t is not a text list or table
1601	in choice t, t is empty
1700	can't open input file %s\n
1701	Type '?' for help.\n
1800	in i/j, j is zero
1801	in 0**y or y root 0, y is negative
1802	in x**(p/q) or (q/p) root x, x is negative and q is even
1803	in x**y or y root x, x is negative and y is not exact
1804	ambiguous expression; please use ( and ) to resolve
1805	no expression where expected
1806	no test where expected
1807	something unexpected in expression
1808	something unexpected in test
1809	misformed address
1810	%s hasn't been initialised or (properly) defined
1811	%s hasn't been (properly) defined
1812	%s has not yet received a value
1813	function returns no value
1814	predicate reports no outcome
1815	a refinement may not be used as an address
1816	bad node in while
1817	bad node in testsuite
1818	indentation not used consistently
1819	indentation must be at least 2
1820	selection on non-table
1900*	a_fpr_formals (1900)
1901*	analyze bad tree (1901)
2000	no command suite where expected
2001	no command where expected
2002	something unexpected in this line
2003	no parameter where expected
2005	IN after colon
2006	no alternative suite for SELECT
2007	after ELSE no more alternatives allowed
2100	nothing instead of expected expression
2101	point without digits
2102	e not followed by exponent
2103	cannot find matching %s
2200*	fix bad tree (2200)
2201*	fix unparsed with bad flag (2201)
2202	command cannot be reached
2203	refinement returns no value or reports no outcome
2204	wrong keyword %s
2205	missing actual parameter after %s
2206	can't find expected %s
2207	unexpected actual parameter after %s
2208	unexpected keyword %s
2209	compound parameter has wrong length
2210	refinement with parameters
2211	you haven't told me HOW TO %s
2212*	f_fpr_formals (2212)
2213	%s cannot be used in an expression
2214	%s is neither a refined test nor a zeroadic predicate
2300	wrong argument of type_check()
2301	next line must be impossible as a refinement name, e.g. with a space:
2302	returned value
2303	RETURN not in function or expression refinement
2304	Empty polytype stack
2400	cannot find expected %s
2401	no name where expected
2402	no keyword where expected
2403	something unexpected following %s
2404	according to the syntax I expected %s
2500	nothing where address expected
2501	no address where expected
2502	something unexpected in address
2600	I found type EG %s where I expected %s
2601	I found type %s where I expected %s
2602	I thought %s was of type EG %s
2603	I thought %s was of type %s
2604	incompatible type for %s
2605	list or table of 
2606	list or table
2607	"", or list or table of ""
2608	text or list or table
2609	%s
2700	HAS follows colon
2701	nothing instead of expected test
2800	how-to starts with indentation
2801	no how-to name where expected
2802	no how-to keyword where expected
2803	%s is a reserved keyword
2804	%s is already a formal parameter or operand
2805	%s is already a shared name
2806	%s is already a refinement name
2807	cannot find function name
2808	user defined functions must be names
2809	something unexpected in formula template
2810	nothing instead of expected template operand
2811	no template operand where expected
2812	nothing instead of expected name
2813	no name where expected
2814	something unexpected in name
2900	change of workspace not allowed
2901	no previous workspace
2902	I find no workspace name here
2903	I can't goto/create workspace %s
2904	*** I cannot find parent directory\n
2905	*** I cannot find workspace\n
2906	*** I cannot find your home directory\n
2907	*** I shall use the current directory as your single workspace\n
2908	*** %s isn't an ABC name\n
2909	*** I shall try the default workspace\n
2910	*** I cannot find the central workspace\n
3000*	replacing in non-environment (3000)
3001*	deleting from non-environment (3001)
3002*	selection on non-environment (3002)
3100	 in your command\n
3101	 in your expression to be read\n
3102	 in your edited value\n
3103	 in your location %s\n
3104	 in your permanent environment\n
3105	 in your workspace index\n
3106	 in your how-to %s\n
3107	 in your central how-to %s\n
3108	 in line %d of your how-to %s\n
3109	 in line %d of your central how-to %s\n
3110	*** (detected after reading 1 line of your input file standard input)\n
3111	*** (detected after reading %d lines of your input file standard input)\n
3112	*** (detected after reading 1 line of your input file %s)\n
3113	*** (detected after reading %d lines of your input file %s)\n
3114	*** The problem is:
3115	*** Sorry, ABC system malfunction\n
3116	*** Sorry, memory exhausted
3117	*** There's something I don't understand
3118	*** There's something I can't resolve
3119	*** Can't cope with problem
3120	*** Cannot reconcile the types
3121	*** Your check failed
3122	*** interrupted\n
3123	unexpected arithmetic overflow
3200	in x %s y, x is not a number
3201	in x %s y, y is not a number
3202	in x %s y, y is not a compound of two numbers
3203	in c %s x, c is zero
3204	in %s x, x is not a number
3205	in %s y, y is not a compound of two numbers
3206	in %s t, t is not a text
3207*	pre-defined fpr wrong (3207)
3208	in the test exact x, x is not a number
3209	in the test e in t, t is not a text list or table
3210	in the test e in t, t is a text, but e is not a character
3211	in the test e not.in t, t is not a text list or table
3212	in the test e not.in t, t is a text, but e isn't a character
3213*	predicate not covered by proposition (3213)
3300	terminating commands only allowed in how-to's and refinements
3301	share-command only allowed in a how-to
3302	I don't recognise this as a command
3303	outer indentation not zero
3304	special commands only interactively
3305*	special (3305)
3400	in ... i IN e, e is not a text, list or table
3500	unexpected program halt
3501*	run: bad thread (3501)
3502	none of the alternative tests of SELECT succeeds
3503	test refinement reports no outcome
3504	refinement returns no value
3505	run-time error %s
3506	run: cannot execute how-to definition
3507*	bad FPR_FORMAL (3507)
3508	QUIT may only occur in a command or command-refinement
3509	RETURN may only occur in a function or expression-refinement
3510	REPORT may only occur in a predicate or test-refinement
3511	SUCCEED may only occur in a predicate or test-refinement
3512	FAIL may only occur in a predicate or test-refinement
3513*	run: bad node type (3513)
3600	location not initialised
3601	%s hasn't been initialised
3602	key not in table
3603	inserting in non-list
3604	removing from non-list
3605	removing from empty list
3606	selection on empty table
3607*	call of location with improper type (3607)
3608*	uniquifying text-selection location (3608)
3609*	uniquifying comploc (3609)
3610*	uniquifying non-location (3610)
3611	text-selection (@ or |) on non-text
3612	in the location t@p or t|p, t does not contain a text
3613	in the location t@p or t|p, p is out of bounds
3614	selection on location of improper type
3615	text-selection (@ or |) out of bounds
3616	putting non-text in text-selection (@ or |)
3617	putting non-compound in compound location
3618	putting compound in compound location of different length
3619	putting in non-location
3620	putting different values in same location
3621	deleting non-location
3622	deleting text-selection (@ or |) location
3623	deleting non-existent location
3624	binding non-location
3625	unbinding non-location
3700	write error (disk full?)
3800	value too big to output
3801*	writing value of unknown type (3801)
3802	*** Please answer with '%c' or '%c'\n
3803	*** Just '%c' or '%c', please\n
3804	*** This is your last chance. Take it. I really don't know what you want.\n    So answer the question\n
3805	*** Well, I shall assume that your refusal to answer the question means '%c'!\n
3806	End of input encountered during READ command
3807	End of input encountered during READ t RAW
3808	type of expression does not agree with that of EG sample
3809	*** Please try again\n
4000	cannot create file name for %s
4001	filename and how-to name incompatible for %s
4002	cannot create file %s; need write permission in directory
4003	unable to find file
4004*	wrong nodetype of how-to (4004)
4005	there is already a how-to with this name
4006	there is already a central how-to with this name
4007	there is already a standard how-to with this name
4008	there is already a permanent location with this name
4009	*** the how-to name is already in use;\n*** should the old how-to be discarded?\n*** (if not you have to change the how-to name)\n
4010	*** the how-to name is already in use for a permanent location;\n*** should that location be deleted?\n*** (if not you have to change the how-to name)\n
4011	*** the how-to name is already in use;\n*** you have to change the how-to name\n
4012	*** Press [SPACE] or [RETURN] to continue
4013	I find nothing editible here
4014	no current how-to
4015	*** do you want to visit the version with %c or %c operands?\n
4016	%s isn't a how-to in this workspace
4017	*** you have no write permission in this workspace:\n*** you may not change the how-to\n*** do you still want to display the how-to?\n
4018	*** cannot create file name;\n*** you have to change the how-to name\n
4019*	ens_filed() (4019)
4020	no current location
4021	*** you have no write permission in this workspace:\n*** you may not change the location\n*** do you still want to display the location?\n
4022	%s isn't a location in this workspace
4023	value is not a table
4024	in t[k], k is not a text
4025	Press [SPACE] for more, [RETURN] to exit list
4100*	stack underflow (4100)
4101*	bad call type (4101)
4102*	stack clobbered (4102)
4103	You haven't told me HOW TO REPORT %s
4104	You haven't told me HOW TO RETURN %s
4105*	invoked how-to has other adicity than invoker (4105)
4106*	udfpr with predefined how-to (4106)
4107*	formula called with non-function (4107)
4108*	proposition called with non-predicate (4108)
4109*	extract (4109)
4110	putting non-compound in compound parameter
4111	parameter has wrong length
4112*	not a compound in sub_epibreer (4112)
4113*	bad nodetype in sub_epibreer (4113)
4114*	too many tags in sub_putback (4114)
4115*	not a compound in sub_putback (4115)
4116*	bad node type in sub_putback (4116)
4117*	not a compound in collect_value (4117)
4118*	bad node type in collect_value (4118)
4119	on return, part of compound holds no value
4120	value of expression parameter changed
4121*	bad def in x_user_command (4121)
4122	You haven't told me HOW TO %s
4200*	loctype asked of non-location (4200)
4201*	valtype called with unknown type (4201)
4400	in ... i IN e, i contains a non-local name
4500*	in cmdline() (4500)
4600	*** %s isn't the name of a location\n
4601	*** %s hasn't been initialised\n
4602	*** %s isn't a table\n
4603	*** Errors while recovering workspace:\n
4604	*** %s: cannot derive a location name\n
4605	*** %s: cannot read this file\n
4606	*** %s: cannot derive a how-to name\n
4607	*** %s: cannot rename this file\n
4608	*** %s: the ABC name for this file is already in use\n
4609	*** %s: cannot create this file\n
4610	*** Errors while recovering the workspace index\n
4611	*** %s: cannot derive an ABC name for this workspace\n
4612	*** %s: the ABC name for this workspace is already in use\n
6000	Empty copy buffer
6001	Trouble with your how-to, see last line. Hit [interrupt] if you don't want this
6002	Spaces and tabs mixed for indentation; check your program layout
6003	There are still holes left.  Please fill or delete these first.
6004	I cannot [goto] that position
6005	Sorry, I could not [goto] that position
6006	You can't use [goto] in recording mode
6007	Cannot insert '%c'
6008	No keystrokes recorded
6009	Keystrokes recorded, use [play] to play back
6010	This redo brought you to an older version.  Use [undo] to cancel
6011	Sorry, I failed to suspend ABC
6200	Sorry, I can't edit file \"%s\"
6201	excessively nested indentation
6202	indentation messed up
6203	unexpected indentation increase
6204*	readsym: ungetc failed (6204)
6300	Cannot save how-to on file \"%s\"
6400	Recording
6401	Copy buffer
6500	Errors in key definitions:\n
6501	Cannot verify the bindings for the interrupt character
6502	Definition for command %s is empty.
6503	Definition for command %s starts with '%c'.
6504	Definition for command %s would produce an interrupt.
6505	Too many key definitions
6506	no '[' before name
6507	No name after '['
6508	no ']' after name
6509	opening string quote not found
6510	closing string quote not found in definition
6511	definition string too long
6512	opening string quote not found in representation
6513	closing string quote not found in representation
6514	unprintable character in representation
6515	representation string too long
6516	Name %s not followed by '='
6517	Unknown command name: %s
6518	Rebinding %s in keysfile not allowed
6519	No '=' after definition for name %s
6520*	too many predefined keys (6520)
6521	Sorry, I failed to suspend ABC\n
6600	*** Bad $TERM or termcap, or dumb terminal\n
6601	*** Bad SCREEN environment\n
6602	*** Cannot reach keyboard or screen\n
6700	Press [SPACE] for more, [RETURN] to exit help
6701	Press [SPACE] or [RETURN] to exit help
6702	*** Cannot find or read help file [%s]
6900	*** incompatible workspace options\n
6901	*** you have not set your environment variable EDITOR\n
7000	*** can't finish writing suggestion file [%s]
7100*	s_up failed (7100)
7101*	s_downi failed (7101)
7102*	s_down failed (7102)
7103*	s_downrite failed (7103)
7200	\nUsage: abc [-g ws.group] [-w ws.name] [-c ws.name]\n
7201	   [ -e [files] | -i tab | -o tab | -l | -p | -u [files] | -r | -x | file ...]\n
7202	\nWorkspace Options:\n
7203	   -g dir       use group of workspaces in 'dir' (default $HOME/abc)\n
7204	   -w name      start in workspace 'name' (default: last workspace)\n
7205	   -w path      use 'path' as current workspace (no -g or -c 'name' allowed)\n
7206	   -c name      use 'name` as central workspace (default: $HOME/abc/abc)\n
7207	   -c path      use 'path' as central workspace\n
7208	Run Options:\n
7209	   -e [files]   Use ${EDITOR} as editor to edit definitions\n
7210	   file ...     Read commands from file(s) ('-' for standard input)\n
7211	Special tasks:\n
7212	   -i tab       Fill table 'tab' with text lines from standard input\n
7213	   -o tab       Write text lines from table 'tab' to standard output\n
7214	   -l           List the how-to's in a workspace on standard output\n
7215	   -p           Pack how-to's and locations from workspace to standard output\n
7216	   -u [files]   Unpack workspace from files or standard input\n
7217	   -r           Recover a workspace when its index is lost\n
7218	   -x           Recover the index of a group of workspaces\n
7219	\nUse 'abckeys' to change key bindings\n
8000	argument to graphics command not a vector
8001	no graphics hardware available
