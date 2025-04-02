rem runs the pseudobios created by makebios.mvs

rem it assumes that the fba1b1.vhd that comes with
rem z/PDOS-generic has been independently loaded
rem with commands like this:
rem del \mvs380\dasd\d33902.cckd
rem dasdload -bz2 d33902.ctl \mvs380\dasd\d33902.cckd

rem and a d33902.ctl that looks similar to this:
rem (I don't think that "separate" is required since we don't IPL it)
rem (not sure what the * is for)

rem D33902       3390-2 * separate
rem SYSVTOC      VTOC             CYL  2
rem FBA1B1.VHD   SEQ \zpg\fba1b1.vhd    CYL  2000 1 0 PS F 512 512

rem and also that mvs380.conf has had this added:
rem 01b6      3390      dasd/d33902.cckd

del temp.zip
zip -9X -j temp bios.exe \zpg\tapes\pdos.exe \zpg\tapes\pcomm.exe
call runmvs mvspb.jcl output.txt temp.zip
