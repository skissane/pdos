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

Note that although pdld can create both the DLL and correct
--kill-at import library in a single command, the GNU tools
we use are not capable of creating a correct --kill-at
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
In this case, GNU ld adds an additional member - the same
as Microsoft's "link" seen in makek32.vsc to the library
that it later depends on. Something to do with a
symbol __IMPORT_DESCRIPTOR_kernel32
There is nothing wrong with doing this - it doesn't violate
any spec. And so long as you use the same tool that that
vendor used to produce the final executable, it will retrieve
that metadata and use it, and all is well. But this is why
you can't match things willy nilly and blame the vendor of
one tool for the fact that another vendor's tool doesn't
accept the library it produced. It is not required to. These
are internal details that toolchain authors can decide for
themselves. So long as the tool produces a technically correct
end executable, there is no basis for complaint. This doesn't
just apply to linkers, it also applies to things like C
compilers. You can't add the preprocessor flag and look at
the intermediate data and say "oh look - you failed to parse
this variable - you have a bug!". You instead need to report
a bug in terms of the final documented output - in this case
the assembler code - and say "ok, this C code produced this
assembler code, and it should have instead produced xyz - I
would suggest a failure in the preprocessor step, because I
noticed abc - but that's just a guess - it's your baby - only
you know how it all hangs together internally - you're the
author!". Or the end result might instead be "oh - the result
is correct after all - the C compiler author must have a way
of detecting that post-processing - maybe I'll ring up up and
have a Diet Coke with him and ask him how the internals work.
Or maybe not. He lives on the other side of the world!".

It so happens that pdld doesn't use the same method that
Microsoft uses to pass the idata table from the library to
the final executable. pdld instead generates its own idata
table. As mentioned, nothing wrong with either approach.

But it does mean that Microsoft's link can't handle a pdld
archive, as that (undocumented internals) extra member is
not present. Again - don't mix and match tools from vendors.

It's not just Microsoft who can't handle it. GNU ld can't
handle it either, and instead of reporting "this archive
isn't from my tool so it is missing the idata table" or
some other technical issue that would occur if you fed it
say a motion jpeg of a dog licking its balls instead of
an archive that meets all its technical requirements (as
would have been met if you had simply used their tool in
the first place instead of mixing and matching), it
instead produces a technically invalid executable (in this
circumstance it is not possible to have an executable with
no idata), without warning. And you instead have to take
your chances with some mailing list and hope that someone
out there gives a shit. So this behavior is not acceptable.
I would report the issue myself but I've got a sore back
because all communists always have sore backs when it's
time for "from each according to their ability".

So there's something wrong with GNU ld, but there's nothing
wrong with Microsoft, even though neither other them will
accept a pdld-produced library. And there's nothing wrong
with pdld either, because it is not violating any spec,
and if you use the tools as documented in this readme.txt,
it all works perfectly fine. First you create a library
with pdld and then you link against that library
*** WITH pdld ***.

YOU CANNOT MIX AND MATCH WILLY NILLY.

It so happens that pdld is a FUCKING BRILLIANT TOOL written
by a guy in Slovakia, and it can handle its own format
PLUS Microsoft-produced libraries, plus GNU-produced libraries.
So IN THIS SINGLE CIRCUMSTANCE yes you can mix and match and
get away with it. Use pdld in the final step and it will take
care of all the shit from all the other vendors with different
object formats and everything and put it in one technically
valid executable and the author is a super fantastic guy who
responds to bug reports with bug fixes normally within 24
FUCKING HOURS you literally can't ask for any more than that
and he does this ALL FOR FUCKING FREE. Slovakia is on my
list of places to travel to to be sure!!! GO GO SLOVAKIA!!!

Some more technical information about the libraries produced
by both pdld and ld - the first member of the library is
known as the "archive symbol table" and despite the fact
that kill-at has been used, they maintain the @nn decoration.
Also note that dlltwin (gnu tool) has a "-k" option, or you
can write it in full as "--kill-at", which does the same
thing as a single pdld command, which is to leave the
decorated @@ in the symbol table names, but remove them from
the .idata import names. Or to put it more simply - it all
works using magic.

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
