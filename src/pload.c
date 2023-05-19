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
    unsigned long fatsize;
    unsigned int num_heads;
    unsigned int sectors_per_track;
    unsigned int sector_size;
    unsigned int reserved_sectors;
    unsigned int numfats;
    unsigned int fatstart;
    unsigned int rootstart;
    unsigned int filestart;
    unsigned int drive;
    unsigned int rootentries;
    unsigned int rootsize;
} DISKINFO;

void pdosload(void);
static void loadIO(int drive, char *edata);
static void AnalyseBpb(DISKINFO *diskinfo, unsigned char *bpb);
static void ReadLogical(DISKINFO *diskinfo, long sector, void *buf);
int readAbs(void *buf, int sectors, int drive, int track, int head, int sect);
void clrbss(void);

int main(void)
{
#ifdef CONTINUOUS_LOOP
    for (;;)
#endif        

    pdosload();

#ifndef CONTINUOUS_LOOP
    return (0);
#endif        
}

/* we need to enter here to avoid Watcom name mangling of main() */
void dstart(int drive, char *edata)
{

/* Until we have loaded all the sectors, we can't rely on the
   data section being valid - and if we were to write anything
   to the data section, it would be clobbered by the load.
   Also the BSS has not been initialized to 0 yet, so we can't
   rely on that, and when we do finish the loading, we need to
   clear the BSS, so it would be wiped anyway. We could wipe
   it first, but then we would have difficulty loading sectors
   of a multiple of 512 which could encroach into the BSS. We
   could try aligning the BSS in the executable, and make sure
   we load the exact correct number of sectors. But it is safer
   all around to simply use stack variables until this process
   is complete. ie this is not normal C programming, so don't
   be surprised that it's a bit hairy. */

#ifdef NEWMODEL
    loadIO(drive, edata);

    clrbss();
    /* now you can do debugging with dumplong/dumpbuf, without
       needing to adjust the boot sector to load more sectors */
#endif

    main();
    return;
}

static void loadIO(int drive, char *edata)
{
    long sector = 0;
    unsigned char *p;
    DISKINFO diskinfo;
    int x;
    int numsects = 0;
    int fat32 = 0;
    unsigned long datastart;
    unsigned char secbuf[512];
    unsigned long rootcluster;
    int sectors_per_cluster;
    unsigned long cluster;

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
#ifdef NEWMODEL
    p = ABS2ADDR(0x600);
    if (p[1] == 0x58)
    {
        fat32 = 1;
    }
    p = ABS2ADDR(0x600 + 11); /* was 0x7c00, now 0x600 - ditto below */
#else
    p = (unsigned char *)(0x600 - 0x600 + 11);
#endif
    AnalyseBpb(&diskinfo, p);
    diskinfo.drive = drive;
    sector = diskinfo.filestart;
    if (fat32)
    {
        diskinfo.fatsize = p[0x24 - 0xb + 0]
                           | ((unsigned long)p[0x24 - 0xb + 1] << 8)
                           | ((unsigned long)p[0x24 - 0xb + 2] << 16)
                           | ((unsigned long)p[0x24 - 0xb + 3] << 24);
        rootcluster = p[0x2c - 0xb + 0]
                      | ((unsigned long)p[0x2c - 0xb + 1] << 8)
                      | ((unsigned long)p[0x2c - 0xb + 2] << 16)
                      | ((unsigned long)p[0x2c - 0xb + 3] << 24);
        sectors_per_cluster = p[0xd - 0xb];
        datastart = diskinfo.reserved_sectors
                    + diskinfo.fatsize * diskinfo.numfats;
        ReadLogical(&diskinfo, datastart, secbuf);
        p = secbuf;
        if ((p[0xb] & 0x8) != 0)
        {
            p += 0x20; /* skip presumed label */
        }
        /* we should now be at the io.sys directory entry */
        cluster = p[0x1a + 0]
                  | ((unsigned long)p[0x1a + 1] << 8)
                  | ((unsigned long)p[0x14 + 0] << 16)
                  | ((unsigned long)p[0x14 + 1] << 24);
        sector = (cluster - rootcluster) * sectors_per_cluster;
        sector += datastart;
    }
#ifdef NEWMODEL
    p = ABS2ADDR(0x700);
    /* add 1 sector for good measure, and the clear of bss will
       fix up any mess */
    numsects = (ADDR2ABS(edata) - 0x700) / 512 + 1;
#else
    p = (unsigned char *)0x100;
#endif
    
    /* You can't load more than 58 sectors, otherwise you will
       clobber the disk parameter table being used, located in
       the 7b00-7d00 sector (ie at 7c3e) */
    /* It has been relocated now */
    for (x = 0; x < numsects; x++) /* was 58 */
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
    diskinfo->reserved_sectors = bpb[3] | ((unsigned int)bpb[4] << 8);
    diskinfo->fatstart = diskinfo->reserved_sectors;
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

#ifdef NEED_DUMP
void dumpbuf(unsigned char *buf, int len)
{
    int x;

    for (x = 0; x < len; x++)
    {
        BosWriteText(0, buf[x], 0);
    }
    return;
}

void dumplong(unsigned long x)
{
    int y;
    char z[16]; /* = "0123456789abcdef"; */
    char buf[9];

/* can't rely on non-existent constants. could potentially force
   constants into code section, but compiler may not allow that */
    z[0] = '0';
    z[1] = '1';
    z[2] = '2';
    z[3] = '3';
    z[4] = '4';
    z[5] = '5';
    z[6] = '6';
    z[7] = '7';
    z[8] = '8';
    z[9] = '9';
    z[10] = 'A';
    z[11] = 'B';
    z[12] = 'C';
    z[13] = 'D';
    z[14] = 'E';
    z[15] = 'F';
    for (y = 0; y < 8; y++)
    {
        buf[7 - y] = z[x & 0x0f];
        x /= 16;
    }
    buf[8] = '\0';
    dumpbuf(buf, 8);
    return;
}
#endif
