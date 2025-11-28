# The original ABC programming language (CWI)

## History

ABC was Python's most direct predecessor;
I worked on it from around 1983-1986.

I downloaded these sources from
[cwi.nl](https://homepages.cwi.nl/~steven/abc/implementations.html),
specifically the abc-unix tarball.

Another copy of the ABC sources lives in Luciano Ramalho's
[GitHub](https://github.com/ramalho/abc/commits/master/ABC).
I hope one day to compare the two trees and unify them.

Most files have 1991 as their latest modification time in the tar ball;
a few have 1996 or 2021.

## Building

The old [README](./README) file has build instructions.

The current sources assume a 32-bit system where int and pointers have
the same size. I hope to eventually upgrade the source code to work on
64-bit systems too (where int is 32 bits and pointers are 64 bits).

## Licence

CWI never put a license on ABC, but it says:

**Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988-2011.**

I'll try to negotiate with Steven Pemberton eventually (hopefully MIT).

## Authors

Eddy Boeve, Frank van Dijk, Leo Geurts, Timo Krijnen, Lambert Meertens,
Steven Pemberton, Guido van Rossum.

## References

- Leo Geurts, Lambert Meertens and Steven Pemberton, The ABC Programmer's
  Handbook, Prentice-Hall, Englewood Cliffs, New Jersey, 1990, ISBN 0-13-
  000027-2.
- Steven Pemberton, An Alternative Simple Language and Environment for PCs,
  IEEE Software, Vol. 4, No. 1, January 1987, pp. 56-64.
  http://www.cwi.nl/~steven/abc.html
