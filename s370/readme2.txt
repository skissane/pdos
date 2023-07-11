This demonstrates 32-bit z/PDOS (available from
http://pdos.org), built using the i370 target of
GCC 3.2.3 running on a z/Arch machine as AM64 and
able to access the full 4 GiB of memory. Note that
it is perfectly legitimate for 32-bit programs to
use negative indexes with an expectation of a wrap,
but on AM64 this results in an index above 4 GiB.
That is why the 4 GiB - 8 GiB region is mapped to
the 0 GiB - 4 GiB region. Thanks goes to Joe Monk
for this idea.

Lots of versions of Hercules should work, but you
will need a 64-bit version to allocate the 4 GiB
of memory. You can get it from here for example:

http://www.hercules-390.eu/

But I use the one available from:

http://pdos.org


PDOS needs to know the console to write to. It will read
the config.sys to determine this, which by default has
device 9 as a 3270 as the first device, so this will be
selected. If you wish to use a different device, you can
override the device number and optionally device type via
an IPL parm, e.g. on Hercules you might go:

ipl 1b9 parm 019Z3215

If there is no override, and device 9 doesn't exist, it will
make a final attempt to use SSID x'10000' which will probably
work if your console is at least the lowest device number.


Run PDOS like this:

hercules -f pdos.cnf

then attach a 3270 emulator

then do "ipl 1b9".

It also runs on real hardware.

You can instead IPL from tape, by doing:

ipl 1c3

If you IPL from tape, you don't need to have a disk
available, but if you do, you can experiment like this:

ipl 1c3
dumpblk 1b1 0 0 3
(to see if there is a VOL1 label)
diskinit 1b1
(initialize the disk)
dumpblk 1b1 0 0 3
(the label should be gone now, it's a blank disk)
fil2dsk tap1c1: 1b1
(restores a 3390 disk image from tape to disk)
dumpblk 1b1 0 0 3
(you should now see a VOL1 label)
shutdown and
ipl 1b1
You can now IPL from what was the scratch disk


Note that this means you can have a working z/Arch system
without requiring any IBM software at all. (old IBM software,
MVS 3.8J, which is public domain, was used to build it though).
And obviously z/PDOS is far less functional than z/OS, but it
is still capable of running some MVS software (such as GCCMVS).

You can try "help" and "dir" and run "gcc --version" to see it
running.

You can run "memtest" which will write 4 bytes of memory
straddling the 2 GiB bar so that you can see that it can
be done.

If you display the memory you should see:

11:41:33 v 7ffffff0
11:41:33 V:000000007FFFFFF0 (primary) R:000000007FFFFFF0
11:41:33 V:000000007FFFFFF0:K:06=00000000 00000000 00000000 00000102  ................
11:41:33 V:0000000080000000 (primary) R:0000000080000000
11:41:33 V:0000000080000000:K:06=03040000 00000000 00000000 00000000  ................
11:41:33 V:0000000080000010:K:06=00000000 00000000 00000000 00000000  ................
11:41:33 V:0000000080000020:K:06=00000000 00000000 00000000 00000000  ................

Another S/390 project that has demonstrated
the ability to cross the 2 GiB bar is UDOS/KinnowOS:

https://github.com/udos-project/KinnowOS

But they switched to 64-bit, so you can't see that anymore.

You can backup and restore 3390 disks by running fil2dsk
or dsk2fil. Example usage is obtained by just running
the command.

The files in this package were created by getting the PDOS
source and going to pdos/s370 and running doit.bat on Windows,
combined with Hercules/380 and MVS/380.

Then when Hercules starts:

copy out.aws while it is still empty
startterm
ipl 1b9
mkipltap tav1c2:
(note - it needs to be tav, not tap, because mkipltap writes
RDWs, and z/PDOS needs to know to convert those into RECFM=U
blocks)
quit
copy out.aws ipltap.aws
copy pdos00.ckd
copy pdos.cnf
copy readme2.txt to readme.txt
manually inspect the pdos00.ckd to make sure zapcons.bat is accurate
scratch.cckd was copied from a previous run of doit.bat


In separate runs of z/PDOS you can ...

You can create an AWS tape with the disk image by:
dsk2fil 1b9 tap1c2:

You didn't previously need to do that, because tap1c1: points to
the actual disk image already, making it a valid tape.

However, because of the difficulty of creating an unlabeled AWS tape
from a flat file on the zPDT (unless the file is a multiple of 80 and
then you can use a card program?), I am now including the 3390
image as an AWS tape.

Create an IPL-from-memory (.ins) using:
mkiplmem dev1c2:
(note that in Hercules you can do ipl pdos.ins and wait a while - it
looks like it has frozen because the instruction count remains as 0
but it hasn't really)

Create an IPL card deck using:
mkiplcrd dev1c2:


Note that if you are using a 3270 terminal, the only editor
available currently is edlin, which is copyrighted, so separately
distributed.

If you require a purely public domain distribution, you can
still (barely) edit files, by using copycon, xychop and
assemble. So if you have temp.001, temp.002, temp.003 you
can do "assemble temp.c" and it will combine the files.
You can use hexdump to figure out where to split a file.

Also, you can (barely) code in C instead of needing to zap
machine code. You can try the example program:

multisc mvssamp.c mvssamp.com
mvssamp

(you can create .com files the same as MSDOS used)

It is unclear if you can fight your way out in C using just
the provided tools.


The IPL tape provided is designed to drive a 3270.
If you wish to instead drive a 3275 (EBCDIC ANSI
encapsulated in 3270 data stream which requires a
special terminal emulator to decipher - specifically
mfterm -3275 which comes with PDOS/386), so that
you can run microemacs or whatever, then you will
either need to use a pre-CZAM z/Arch machine that
stored the IPL PARM in registers that were easy to
access (albeit undocumented), so that you can
override the 3270 with something like 9Z3275 which
is device 9, Z is a separator (must be Z), and 3275
is the device type. If you are using a CZAM machine
you have no choice but to zap the included config.sys
whose location varies, but you can find it via a
hexdump and look for 3270 somewhere. There are multiple
places with 3270, so you're looking for the one that
starts 0009 with no comment.

01D3A0  A3157BF0 F0F0F940 40404040 40F3F2F1  ..{....@@@@@@...
01D3B0  F5404040 40404040 95969799 969497A3  .@@@@@@@........

3215 - no use

01D3C0  15F0F0F0 F9404040 404040F3 F2F7F015  .....@@@@@@.....

009 and 3270 and newline (x'15') - perfect.

So in this case you would zap 0x1d3ce to 0xf5

01D3D0  7BF0F0F0 F9404040 404040F1 F0F5F240  {....@@@@@@....@
01D3E0  40404040 40409596 97999694 97A3157B  @@@@@@.........{
01D3F0  40F1F0F5 F2404E40 85828384 89834083  @....@N@......@.
01D400  998581A3 85A240A6 8881A340 9486A385  ......@....@....
01D410  99944083 819393A2 408140F1 F0F5F715  ..@.....@.@.....
01D420  7BF0F0F0 F9404040 404040F1 F0F5F240  {....@@@@@@....@
01D430  40404040 40408582 83848983 40959697  @@@@@@......@...
01D440  99969497 A3157BF0 F0F0F940 40404040  ......{....@@@@@
01D450  40F1F0F5 F260C340 40404040 40614095  @....`.@@@@@@a@.
01D460  96979996 9497A315                    ........

Note that this changes every release, so the above is just
an example. You need to find the right offset yourself.
Here is another example:

C:\devel\pdos\s370\tapes\xxx\tapes>hexdump ipltap.aws 0x1d346 1
01D346  F0                                   .


If you are using zPDT (aka z1090/zOPD), then use the "-x" option to mfterm
to get it to do extended negotiation (so that it can decline
the TN3270E request, plus other differences compared to
Hercules).

There is a "ramdisk" command, and I think you can reference it
as device 20000. Although I thought it became the default drive,
so you should be able to reference it as device 0. But try
drv20000: first

Some commands don't give feedback, but run successfully. Can't
remember which ones.

Just run a command with no parameters and it normally gives usage.



There is virus-licensed software known to work on z/PDOS, including
GCCMVS, EDLIN, UEMACS and DIFF, but it is not included any more,
because I have switched to a pure public domain distribution.
You can probably put this onto another 3390 disk pack and load it
with fil2dsk, but you'll still need to write another utility,
presumably written in multisc C, to get it onto the main disk.
Actually, you should be able to just use the "copy" command and
copy it from a RECFM=U tape. And indeed, the OZPD and PDOS code
can be zipped up without compression (and junking directory paths)
and put on the mainframe the same way and mvsunzip can be used to
unzip it.


Also note that you can change the config.sys to use a 1052 or a
1057. Both of these are identical to PDOS - EBCDIC ANSI - but
mfterm sees a distinction. The 1052 is the only thing that is
expecting to see ASCII. Hercules/380 is able to convert the EBCDIC
ANSI (1057) into ASCII ANSI (1052) for use with a normal ASCII
ANSI telnet connection.


Note that on a CZAM machine the registers may be available at
location x'1280' for 8 bytes - hasn't been tested.

Also note that the startup code for all MVS programs probably
needs to clean all registers using LA - currently there may be
a reliance on the high 32 bits already being 0 - which is
normally the case, but not guaranteed, and at least one person
has an exit on their site to force junk into the high 32 bits.

There is a goal to update the codebase so that z/PDOS could
actually have run starting on a S/360. So that means the code
generator should use MVC instead of MVCL. It should IPL in
AM24 and then attempt to switch to AM64 or AM32 or AM31, whatever
is available. Whether to use SIO or SSCH can be determined by
looking at the IPL device to see if it is less than x'1000'
(SIO). EC mode PSW should be used, which means on a S/360 that
will activate ASCII, which should then be switched off. That
can be done by loading 1 into a register, then do a CVD of that
register, then check if the rightmost byte in the doubleword
stored by CVD is X'1C' or X'1A', for EBCDIC or USASCII-8
respectively (thanks Tony Harminc for that).


Also note that I use LPSW but I really need LPSWE, introduced
with z/Arch, before executables can reside above 2 GiB.

Note that LPSWE assembles to:

0000FE B2B2C160                     326+         DC    0XL4'00',X'B2B2',S(WRWTNOER)

compared to:

000102 8200 C160      001E8         327          LPSW  WRWTNOER     Wait for an interrupt

This would be the only instruction that didn't already exist
in S/370-XA. Although things like doing a BSM with x'01' didn't
exist until z/Arch either.


The listing of the building of z/PDOS, including the expanded
macros, which would be useful for writing a C-callable replacement,
has been made available as "listing.txt".

The best way to write and build a C (subset) program is
demonstrated by exprog.c and exprog.mak. Just do:
type exprog.c
type exprog.mak
pdmake -f exprog.mak
dir
exprog
to see it in action.

MicroEMACS 3.6 was still public domain, so a modified
version of that is available as e.exe. So you can do e.g.:
"e readme.txt" if you have an EBCDIC ANSI terminal (as mfterm
on PDOS/386 is able to provide by using either 1052 or 1057.
But you'll need to zap the config.sys to set 1052 as device
9 or do an IPL override if available).
