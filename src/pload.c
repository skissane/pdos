/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pload - boot loader/IO.SYS designed to reside at location        */
/*  0x600 but all offsets will be from 0x500 because it is being     */
/*  built as a .COM file so there is 0x100 reserved for a PSP, even  */
/*  though we don't really have one in our situation.                */
/*                                                                   */
/*  This program may only have the first 3 sectors loaded into       */
/*  memory when execution starts (a limitaion of the boot sector)    */
/*  so we first load ourselves (and just assume we are the maximum   */
/*  58 sectors before we hit other data), and then we proceed to     */
/*  load the kernel/OS (PDOS) at 0x10600.                            */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bos.h"
#include "support.h"
#include "int13x.h"
#include "unused.h"

typedef struct
{
    unsigned long hidden;
    unsigned long sectors_per_cylinder;
    unsigned int num_heads;
    unsigned int sectors_per_track;
    unsigned int sector_size;
    unsigned int numfats;
    unsigned int fatstart;
    unsigned int rootstart;
    unsigned int filestart;
    unsigned int drive;
    unsigned int rootentries;
    unsigned int rootsize;
    unsigned int fatsize;
} DISKINFO;

void pdosload(void);
static void loadIO(void);
static void AnalyseBpb(DISKINFO *diskinfo, unsigned char *bpb);
static void ReadLogical(DISKINFO *diskinfo, long sector, void *buf);
int readAbs(void *buf, int sectors, int drive, int track, int head, int sect);

int main(void)
{
    /* we now get the boot sector to load all the sectors */
    /* loadIO(); */
#ifdef CONTINUOUS_LOOP
    for (;;)
#endif        
    pdosload();
#ifndef CONTINUOUS_LOOP
    return (0);
#endif        
}

/* we need to enter here to avoid Watcom name mangling of main() */
void dstart(void)
{
    main();
}

static void loadIO(void)
{
    long sector = 0;
    unsigned char *p;
    DISKINFO diskinfo;
    int x;

#if 0        
    diskinfo.sectors_per_cylinder = 1;
    diskinfo.num_heads = 1;
    diskinfo.sectors_per_track = 1;
    diskinfo.bootstart = 0;
    diskinfo.fatstart = 1;
    p = (char *)0xc000;
    ReadLogical(&diskinfo, sector, p);
    p += 11;
#endif
    p = (unsigned char *)(0x7c00 - 0x600 + 11);    
    AnalyseBpb(&diskinfo, p);
    p = (unsigned char *)0x100;
    sector = diskinfo.filestart;
    
    /* You can't load more than 58 sectors, otherwise you will
       clobber the disk parameter table being used, located in
       the 7b00-7d00 sector (ie at 7c3e) */
    for (x = 0; x < 58; x++)
    {
#if 1 /* (!defined(USING_EXE)) */
        ReadLogical(&diskinfo, sector + x, p);
#endif        
        p += 512;
    }
    return;
}

static void AnalyseBpb(DISKINFO *diskinfo, unsigned char *bpb)
{
    diskinfo->sector_size = bpb[0] | ((unsigned int)bpb[1] << 8);
    diskinfo->numfats = bpb[5];
    diskinfo->fatsize = bpb[11] | ((unsigned int)bpb[12] << 8);
    diskinfo->num_heads = bpb[15] | ((unsigned int)bpb[16] << 8);
    diskinfo->hidden = bpb[17]
                       | ((unsigned long)bpb[18] << 8)
                       | ((unsigned long)bpb[19] << 16)
                       | ((unsigned long)bpb[20] << 24);
    diskinfo->drive = bpb[25];
    diskinfo->fatstart = 1;
    diskinfo->rootstart = diskinfo->fatsize 
                          * diskinfo->numfats + diskinfo->fatstart;
    diskinfo->rootentries = bpb[6] | ((unsigned int)bpb[7] << 8);
    diskinfo->rootsize = diskinfo->rootentries / (diskinfo->sector_size / 32);
    diskinfo->sectors_per_track = (bpb[13] | ((unsigned int)bpb[14] << 8));
    diskinfo->filestart = diskinfo->rootstart + diskinfo->rootsize;
    diskinfo->sectors_per_cylinder = diskinfo->sectors_per_track 
                                     * diskinfo->num_heads;
    return;
}

static void ReadLogical(DISKINFO *diskinfo, long sector, void *buf)
{
    int track;
    int head;
    int sect;

    sector += diskinfo->hidden;
    track = (int)(sector / diskinfo->sectors_per_cylinder);
    head = (int)(sector % diskinfo->sectors_per_cylinder);
    sect = head % diskinfo->sectors_per_track + 1;
    head = head / diskinfo->sectors_per_track;
    readAbs(buf, 1, diskinfo->drive, track, head, sect);
    return;
}

int readAbs(void *buf, int sectors, int drive, int track, int head, int sect)
{
    int rc;
    int ret = -1;
    int tries;
    
    unused(sectors);
    tries = 0;
    while (1) /* (tries < 5) */
    {
        rc = BosDiskSectorRead(buf, 1, drive, track, head, sect);
        if (rc == 0)
        {
            ret = 0;
            break;
        }
        while (BosDiskReset(drive) != 0)
        {
            /* infinite retries */
        }
        tries++;
    }
    return (ret);
}

static int BosDiskSectorRead(void        *buffer,
                             unsigned int sectors,
                             unsigned int drive,
                             unsigned int track,
                             unsigned int head,
                             unsigned int sector)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;
    
    regsin.h.ah = 0x02;
    regsin.h.al = (unsigned char)sectors;
    regsin.h.ch = (unsigned char)(track & 0xff);
    regsin.h.cl = (unsigned char)sector;
    regsin.h.cl |= (((unsigned int)track & 0x0300U) >> 2);
    regsin.h.dh = (unsigned char)head;
    regsin.h.dl = (unsigned char)drive;
    sregs.es = FP_SEG(buffer);
    regsin.x.bx = FP_OFF(buffer);
#ifdef __32BIT__
    regsin.d.ebx = (unsigned int)buffer;
#endif        
    int13x(&regsin, &regsout, &sregs);
    return (regsout.x.cflag);
}

static int BosDiskReset(unsigned int drive)
{
    union REGS regsin;
    union REGS regsout;
    struct SREGS sregs;

    regsin.h.ah = 0x00;
    regsin.h.dl = (unsigned char)drive;    
    int13x(&regsin, &regsout, &sregs);
    return (regsout.x.cflag);
}
