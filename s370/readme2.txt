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

http://mvs380.sourceforge.net

You will need to zap the console device number to
whatever device your 3270 appears as (subchannel id),
typically x'00010000' (see zapcons.bat). Otherwise
it will guess that exact number based on the pdos.cnf
that became config.sys.

Run PDOS like this:

hercules -f pdos.cnf

then attach a 3270 emulator

then do "ipl 1b9".

You can also try it on real hardware.

You can instead IPL from tape, by doing:

ipl 1c3

If you IPL from tape, you don't need to have a disk
available, but if you do, you can experiment like this:

ipl 1c3
dumpblk 10001 0 0 3
(to see if there is a VOL1 label)
diskinit 10001
(initialize the disk)
dumpblk 10001 0 0 3
(the label should be gone now, it's a blank disk)
fil2dsk tap10003: 10001
(restores a 3390 disk image from tape to disk)
dumpblk 10001 0 0 3
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
mkipltap tav10004:
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
dsk2fil 10002 tap10004:

But you don't need to do that, because tap10003 points to
the actual disk image already, making it a valid tape.
