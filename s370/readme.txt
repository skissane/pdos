Welcome to PDOS/390 (and friends).

This distribution comes with the PDOS operating system
installed on a 3390.

Note - this document refers to subsystem IDs, but we now
have the ability to specify unit numbers. Also there is
a replacement readme. I'll probably fix some of this one day.



RUNNING ON HERCULES UNDER WINDOWS
---------------------------------

Users of Hercules under Windows need to create a directory 
such as c:\pdos, unzip this package into there, preferably
create an environment variable PDOS pointing to that directory,
as well as adding it to the path, and then type in "pdos".
After it starts, try running "world abc def", which has a 
loop in it which will hopefully take long enough for you 
to see that it executes in 31-bit mode, even though it waits
in 24-bit mode.



RUNNING ON HERCULES UNDER LINUX
-------------------------------

Unfortunately you will need to convert the pdos.bat file
yourself. Actually you just need the single call to
"hercules". All the other stuff is optional.



OBTAINING SOURCE
----------------

The source for everything included in PDOS is available,
although some things came from different packages (e.g. the
GCC compiler), so please download those separate packages
to obtain the source and read their license agreements.
Everything is linked to under the PDOS sourceforge site 
(including the MVS/380 site). Also note that pdptop.mac is 
copied from pdp390.mac.



TECHNICAL NOTES
---------------

The executables that are supported by PDOS are called
"MVS PE (Portable Executable)" format, which is simply
an IEBCOPY unloaded, followed by converting the resultant
RECFM=V into RECFM=U with RDWs added (same as what happens
if you ftp from the mainframe with the RDW option). While
PDOS only supports a fraction of normal MVS calls, that
fraction is sufficient to do such things as run a C
compiler to the point where it can even recompile itself.
Although note that that is currently fudged, so don't try
to write files more than 1 cylinder in size.


FURTHER INFORMATION
-------------------

Support/feedback can be obtained here:
http://tech.groups.yahoo.com/group/hercules-os380/

Package is available here:
http://pdos.sourceforge.net







ADDENDUM: RUNNING ON REAL IRON
------------------------------

If you wish to run on real iron, you will need to dump the 
3390-1 disk image (pdos00.cckd), restore it to a real 3390, 
then zap the PDOS.SYS dataset at location 7FC.  Note 
that there is an eyecatcher "ZAPCONSL" just before the 
location.

0007E0  00000000 00000000 00000000 00000000   00000000 E9C1D7C3 D6D5E2D3 00000000   *....................ZAPCONSL....*
000800  47F0F00C 067C7CC3 D9E3F000 90ECD00C   18CF58FD 004C50DF 000450FD 000818DF   *.00..@@CRT0..........<&...&.....*

The following JCL will do that:

//ZAP      JOB CLASS=C,REGION=0K
//*
//* zap a dataset
//*
//ZAP      EXEC PGM=IMASPZAP
//SYSPRINT DD  SYSOUT=*
//SYSLIB   DD  DSN=SYS1.PDOS,DISP=SHR,UNIT=SYSALLDA,VOL=SER=PDOS00
//SYSIN    DD  *
 CCHHR 0001000001
 VER 07FC 00000000
 REP 07FC 00010038
 ABSDUMPT ALL
/*
//*
//

(the example sets the subchannel to x'10038')

If you do not zap it, it will attempt to figure out the
3215 console address by searching for "3215" (which
includes "3215-C") in the CONFIG.SYS.  Based on this
device's logical position within the devices, it will
determine the address.  E.g. if this is the 5th device you have
defined, it will set an address of x'10004' (ie it uses
0-based counting). This matches what Hercules does. In
S/370 and S/380 modes, it instead uses the specified device
address.


Options for dumping the data would be to run ADRDSSU on a Hercules
system running z/OS, or there is also a CCKDDUMP/CCKDLOAD. I've
never actually used either, as I don't have access to a real system.

I'm hoping that DSSDUMP can also be enhanced to do a physical
dump, but at time of writing, Gerhard is waiting on some
sample dumps to be made available. Maybe dumping this data
(uncompressed) with ADRDSSU would give him what he needs.

Note that PDOS is expecting the console to be a 3215, not a
3270. Since real sites are expected to be using a PC emulator
of some description, or perhaps telnet, it is unclear whether
it is possible to get this to work in the real world, even
assuming some of the programming shortcuts (ie not checking
for device errors) work in the real world.

Here is an example of how to define the disk under Hercules:

01b9      3390      dasd/pdos00.cckd
