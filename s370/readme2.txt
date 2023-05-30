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

ipl 1b9 parm 019 3215

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
manually inspect the pdos00.ckd to make sure zapcons.bat is accurate
scratch.cckd was copied from a previous run of doit.bat

You can create an AWS tape with the disk image by:
dsk2fil 1b9 tap1c2:

But you don't need to do that, because tap1c1: points to
the actual disk image already, making it a valid tape.


Note that if you are using a 3270 terminal, the only editor
provided is edlin, which is copyrighted.

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
