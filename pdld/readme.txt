This is the documentation for the pdld product


Both the source code and documentation are fully public domain.



pdld is a linker originally designed to target Windows x86
but later expanded to support a variety of other targets of
interest.

pdld --help

tells you which targets are currently supported, by looking
at the list after:

--oformat FORMAT


The syntax of pdld is somewhat similar to ld, but pdld is
neither beholden to remain compatible with ld for anything,
and nor are the results expected to be identical or
interchangeable. Mixing and matching object code and tools
is done at your own risk.

Generally it is advised to follow an active build procedure
to get a working toolchain, e.g. doing:

pdmake -f makefile.std

in pdos/pdpclib

of the PDOS source tree. Read makefile.std first, to see
what the dependencies are.


Here are some other options:

--kill-at

Some external references used by some tools generate a
"decorated" function name, such as:

foo@4

where the "4" is the total length of the parameters passed on
the stack to that function.

In certain circumstances, such as when creating a DLL,
the DLL itself needs to have the @4 removed, while the
library itself needs to have the full, "decorated" name.
The kill-at kills the name in the DLL only, not the library.
This is intentional.

pdld differs slightly from GNU ld in this regard, because
instead of removing the @nn from the symbol table names
it instead removes it from the .idata import name. Both
ways are technically valid (according to what spec?),
but it is an example of where you cannot mix and match
tools willy nilly. In this specific case, if pdld is
used as the linker for a final executable, it can handle
a library that was created by either method, but GNU ld
at time of writing can only handle libraries created by
the former method.
