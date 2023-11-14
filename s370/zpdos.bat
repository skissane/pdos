rem This script is only applicable if you are
rem using the Cygin version of Hercules/380

rem You will need to change pdos.cnf to put the
rem memory down to something more reasonable,
rem like 32 MiB, since this is 32-bit

rem You will also need to activate the 1052-C
rem line in pdos.cnf

rem And currently there is no way to quit. If
rem you type "exit", it will exit pcomm and then
rem hang because I haven't yet implemented a diag
rem to issue a Hercules "quit" command

rem And the output is a bit of a mess at startup
rem because Hercules verbiage is being displayed
rem and being mixed in with z/PDOS output. But if
rem you just type in a command like "dir" that
rem stuff goes away.

rem micro-emacs is not display double-quotes
rem properly, apparently because that is x'7f' in
rem EBCDIC and is being recognized as DEL instead of
rem being passed through and translated. But if you
rem do a "hexdump" of autoexec.bat the double-quotes
rem are fine

set HERCULES_RC=zpdos.rc
hercules -f pdos.cnf >hercules.log
