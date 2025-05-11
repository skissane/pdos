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

In certain circumstances, such as when creating a system
DLL like kernel32.dll, the DLL itself needs to have the @4
removed, while the library itself needs to have the full,
"decorated" name. The kill-at kills the name in the DLL only,
not the library. This is intentional.

Note that although pdld can create both the DLL and correct
--kill-at import library in a single command, the GNU tools
we used to use are not capable of creating a correct --kill-at
import library (the DLL is fine though), so instead
you need to separately create
the DLL (using ldwin) and the library (using dlltwin). You
can see an example of this being done in PDPCLIB (src/makek32.w32).
This is why it is always best to start with a working system and
make changes to get where you are trying to go, so that if
something breaks you can step back and try a different
approach. Hopefully someone else has already suffered the
heartache in getting a working system. You get to inherit that
knowledge.

In the case of both pdld and ld the end result is the same -
a valid DLL and a valid library.

However, sometimes tools have dependencies above and beyond
what the minimum requirements are to be "technically valid".
Microsoft's linker adds an additional
symbol __IMPORT_DESCRIPTOR_kernel32
There is nothing wrong with doing this - it doesn't violate
any spec. And so long as you use the same tool that that
vendor used to produce the final executable, it will retrieve
that metadata and use it, and all is well.

It so happens that pdld (when generating an executable)
doesn't use the same method that
Microsoft uses to pass the idata table from the library to
the final executable. pdld instead generates its own idata
table. As mentioned, nothing wrong with either approach.

But it does mean that Microsoft's link can't handle a pdld
archive, as that (undocumented internals) extra member is
not present.

HOWEVER. Due to popular demand, pdld now has a non-default
option "--implib-compat" which creates an import library
that Microsoft link (and other tools, like modern GNU ld)
will accept. This option has no relationship to the ldwin
"--compat-implib" option.

Some more technical information about the libraries produced
by both pdld and ld - the first member of the library is
known as the "archive symbol table" and despite the fact
that kill-at has been used, they maintain the @nn decoration.
Also note that dlltwin (gnu tool) has a "-k" option, or you
can write it in full as "--kill-at", which does the same
thing as a single pdld command, which is to leave the
decorated @@ in the symbol table names, but remove them from
the .idata import names.

Clarification:

I have used the word "library" above to refer to a traditional
.lib file (.a on Unix), and DLL to refer to ".dll" (.so on
Unix), but in actual fact, both of those things are in fact
"libraries", the former being more technically a "import
library" and the DLL being a "dynamic link library".

The PE/COFF specification has the concept of an "archive", and
import libraries are included in this. The first member of an
archive/import library is called the "first linker member"
and can be considered to be an "archive symbol table".
