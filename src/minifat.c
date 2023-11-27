/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  minifat - minimal fat.c with everything required by PLOAD        */
/*                                                                   */
/*  layout is boot sector, then FAT (clusters) then root directory   */
/*  then file storage area                                           */
/*                                                                   */
/*********************************************************************/

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "fat.h"
#include "bos.h"
#include "unused.h"
#include "pos.h"

static int fatEndCluster(FAT *fat, unsigned long cluster);
static void fatPosition(FAT *fat, const char *fnm);
static void fatNextSearch(FAT *fat, char *search, const char **upto);
static void fatRootSearch(FAT *fat, char *search);
static void fatDirSearch(FAT *fat, char *search);
static void fatClusterAnalyse(FAT *fat,
                              unsigned long cluster,
                              unsigned long *startSector,
                              unsigned long *nextCluster);
static void fatDirSectorSearch(FAT *fat,
                               char *search,
                               unsigned long startSector,
                               int numsectors,
                               unsigned char *lfn,
                               unsigned int *lfn_len,
                               unsigned char *checksum);
static void fatReadLogical(FAT *fat, unsigned long sector, void *buf);

/* Functions for LFNs. */
static unsigned int isLFN(const char *fnm, unsigned int length);
unsigned char readLFNEntry(DIRENT *p, unsigned char *lfn,
                                  unsigned int *length);
unsigned char generateChecksum(const char *fnm);
static unsigned int cicmp(const unsigned char *first,
                          const unsigned char *second,
                          unsigned int length);

static unsigned char dir_buf[MAXSECTSZ];


/*
 * fatDefaults - initial call which cannot fail
 */

void fatDefaults(FAT *fat)
{
    fat->sectors_per_cylinder = 1;
    fat->num_heads = 1;
    fat->sectors_per_track = 1;
    fat->bootstart = 0;
    fat->fatstart = 1;
    fat->dbuf = dir_buf;
    return;
}


/*
 * fatInit - initialize the FAT handler.
 */

int fatInit(FAT *fat,
            unsigned char *bpb,
            void (*readLogical)(void *diskptr, unsigned long sector,
                                void *buf),
            void (*writeLogical)(void *diskptr, unsigned long sector,
                                 void *buf),
            void *parm,
            void (*getDateTime)(FAT_DATETIME *ptr)
            )
{
    fat->cachedSector = 0;
    fat->readLogical = readLogical;
    fat->writeLogical = writeLogical;
    fat->parm = parm;
    fat->getDateTime = getDateTime;
    /* BPB passed by PDOS is already at offset 11
     * (skipping jump code and OEM info). */
    fat->sector_size = bpb[0] | ((unsigned int)bpb[1] << 8);
    fat->sectors_per_cluster = bpb[2];
    fat->bytes_per_cluster = fat->sector_size * fat->sectors_per_cluster;
    /* Reserved sectors before FATs. 2 bytes.
     * FATs are located right after them. */
    fat->reserved_sectors = bpb[3] | ((unsigned int)bpb[4] << 8);
    fat->fatstart = fat->reserved_sectors;
#if 0
    printf("sector_size is %x\n", (int)fat->sector_size);
    printf("sectors per cluster is %x\n", (int)fat->sectors_per_cluster);
#endif
    fat->numfats = bpb[5];
    /* Number of sectors per FAT. Only for FAT 12/16. */
    fat->fatsize = bpb[11] | ((unsigned int)bpb[12] << 8);
    /* Total sectors on the volume. If 0, checks offset 32 (bpb[21]). */
    fat->sectors_per_disk = bpb[8] | ((unsigned int)bpb[9] << 8);
    if (fat->sectors_per_disk == 0)
    {
        fat->sectors_per_disk =
            bpb[21]
            | ((unsigned int)bpb[22] << 8)
            | ((unsigned long)bpb[23] << 16)
            | ((unsigned long)bpb[24] << 24);
    }
    fat->num_heads = bpb[15];
    fat->hidden = bpb[17]
                  | ((unsigned long)bpb[18] << 8)
                  | ((unsigned long)bpb[19] << 16)
                  | ((unsigned long)bpb[20] << 24);
    /* Calculates start sector of the root and data region. Only for FAT 12/16. */
    fat->rootstart = fat->fatsize * fat->numfats + fat->fatstart;
    fat->rootentries = bpb[6] | ((unsigned int)bpb[7] << 8);
    fat->rootsize = fat->rootentries / (fat->sector_size / 32);
    fat->sectors_per_track = (bpb[13] | ((unsigned int)bpb[14] << 8));
    fat->filestart = fat->rootstart + fat->rootsize;
#if 0
    printf("filestart is %d\n", fat->filestart);
#endif
    fat->num_tracks = fat->sectors_per_disk / fat->sectors_per_track;
    fat->num_cylinders = (unsigned int)(fat->num_tracks / fat->num_heads);
    fat->sectors_per_cylinder = fat->sectors_per_track * fat->num_heads;
    /* FAT type | Max. clusters
     *       12 |          4086
     *       16 |         65526
     *       32 |     268435456
     * Reason for this is unknown, but those numbers are confirmed. */
    /* however, other systems don't do that, so give priority to
       the fat32 signature */
    if ((bpb[0x37] == 0x29) && (bpb[0x1b] != 0x29))
    {
        fat->fat_type = 32;
    }
    else if ((fat->sectors_per_disk / fat->sectors_per_cluster) < 4087)
    {
        fat->fat_type = 12;
    }
    else if ((fat->sectors_per_disk / fat->sectors_per_cluster) < 65527)
    {
        fat->fat_type = 16;
    }
    else if ((fat->sectors_per_disk / fat->sectors_per_cluster) < 268435457)
    {
        fat->fat_type = 32;
    }
    /* EBPB comes right after BPB and is different for fat32. */
    if (fat->fat_type == 12 || fat->fat_type == 16)
    {
        /* Drive number. 0x00 for floppy disk, 0x80 for hard disk. */
        fat->drive = bpb[25];
    }
    else if (fat->fat_type == 32)
    {
        /* Sectors per FAT. 4 bytes. */
        fat->fatsize = bpb[25]
                  | ((unsigned long)bpb[26] << 8)
                  | ((unsigned long)bpb[27] << 16)
                  | ((unsigned long)bpb[28] << 24);
        /* Calculates root/data region start sector.
         * rootstart should be the same as filestart
         * on FAT 32 most (probably all) of the time.*/
        fat->rootstart = fat->fatsize * fat->numfats + fat->fatstart;
        fat->filestart = fat->rootstart + fat->rootsize;
        /* Root directory start cluster. 4 bytes. */
        fat->rootstartcluster = bpb[33]
                  | ((unsigned long)bpb[34] << 8)
                  | ((unsigned long)bpb[35] << 16)
                  | ((unsigned long)bpb[36] << 24);
        /* Drive number. 0x00 for floppy disk, 0x80 for hard disk. */
        fat->drive = bpb[53];

        /* FSInfo structure sector is not needed for minifat. */
    }
    return (0);
}


/*
 * fatTerm - terminate the FAT handler.
 */

void fatTerm(FAT *fat)
{
    unused(fat);
    return;
}


/*
 * fatEndCluster - determine whether the cluster number provided
 * is an indicator of "no more clusters, it's time to go to bed",
 * courtesy Neil, Young Ones.
 */

static int fatEndCluster(FAT *fat, unsigned long cluster)
{
    if (fat->fat_type == 16)
    {
        if ((cluster >= 0xfff8U) || (cluster < 0x2U))
        {
            return (1);
        }
    }
    else if (fat->fat_type == 12)
    {
        if ((cluster >= 0xff8U) || (cluster == 0xff0U)
            || (cluster < 0x2U))
        {
            return (1);
        }
    }
    if (fat->fat_type == 32)
    {
        /* "UL" at the end makes it unsigned long,
         * the same type as the variable. */
        if (((cluster & 0x0fffffff) >= 0x0ffffff8UL)
            || ((cluster & 0x0fffffff) < 0x2UL))
        {
            return (1);
        }
    }
    return (0);
}


/* Return codes should be Positive */
/* If opening root directory, abide by rootentries.  If opening
   subdirectory or file, follow clusters */

unsigned int fatOpenFile(FAT *fat, const char *fnm, FATFILE *fatfile)
{
    DIRENT *p;

    fat->currfatfile=fatfile;
    if ((fnm[0] == '\\') || (fnm[0] == '/'))
    {
        fnm++;
    }
    if (strcmp(fnm, "") == 0)
    {
        if (fat->rootsize)
        {
            fatfile->root = 1;
            fatfile->nextCluster = 0xffff;
            fatfile->sectorCount = fat->rootsize;
            fatfile->sectorStart = fat->rootstart;
            fatfile->lastBytes = 0;
            fatfile->lastSectors = fat->rootsize;
            fat->currcluster = 0;
            fatfile->dir = 1;
            fatfile->attr=DIRENT_SUBDIR;
        }
        /* Expandable root directory behaves
         * like a normal subdirectory, but
         * it cannot be found with fatPosition,
         * so fat->rootstartcluster is used. */
        else
        {
            fatfile->root = 1;
            fatfile->lastBytes = 0;
            fatfile->lastSectors = 0;
            fat->currcluster = fat->rootstartcluster;
            fatfile->dir = 1;
            fatfile->attr=DIRENT_SUBDIR;

            fatClusterAnalyse(fat,
                          fat->currcluster,
                          &fatfile->sectorStart,
                          &fatfile->nextCluster);
            fatfile->sectorCount = fat->sectors_per_cluster;
        }
        /* Sets the current position to the beginning. */
        fatfile->currpos = 0;
    }
    else
    {
        fatfile->root = 0;
        fatPosition(fat, fnm);
        if (fat->notfound) return (POS_ERR_FILE_NOT_FOUND);
        p = fat->de;
        fatfile->dirSect = fat->dirSect;
        fatfile->dirOffset = ((unsigned char*)p - fat->dbuf);
        fatfile->startcluster = fat->currcluster;
        fatfile->fileSize = p->file_size[0]
                            | ((unsigned long)p->file_size[1] << 8)
                            | ((unsigned long)p->file_size[2] << 16)
                            | ((unsigned long)p->file_size[3] << 24);
        fatfile->currpos = 0;
        /* empty files have a start cluster of 0 */
        if ((fatfile->fileSize == 0)
            && (fatfile->startcluster == 0))
        {
            if (fat->fat_type == 16)
            {
                fat->currcluster = fatfile->startcluster = 0xfff8;
            }
            else if (fat->fat_type == 12)
            {
                fat->currcluster = fatfile->startcluster = 0xff8;
            }
            if (fat->fat_type == 32)
            {
                fat->currcluster = fatfile->startcluster
                = (unsigned long) 0xffffff8;
            }
        }

        fatfile->attr = p->file_attr;

        fatfile->ftime= p->last_modtime[0]
                    | ((unsigned int) p->last_modtime[1] << 8);

        fatfile->fdate= p->last_moddate[0]
                    | ((unsigned int) p->last_moddate[1] << 8);

        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % (fat->sectors_per_cluster
                                 * fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
        fatfile->dir = (fatfile->fileSize == 0);
        fatClusterAnalyse(fat,
                          fat->currcluster,
                          &fatfile->sectorStart,
                          &fatfile->nextCluster);
        fatfile->sectorCount = fat->sectors_per_cluster;
    }
    fatfile->currentCluster = fat->currcluster;
    fatfile->sectorUpto = 0;
    fatfile->byteUpto = 0;
    return (0);
}


/*
 * fatReadFile - read from an already-open file.
 */

int fatReadFile(FAT *fat, FATFILE *fatfile, void *buf, unsigned int szbuf,
                unsigned int *readbytes)
{
    size_t bytesRead = 0;
    static unsigned char bbuf[MAXSECTSZ];
    int bytesAvail;
    unsigned int sectorsAvail;

    if (fatfile->currpos < 0)
    {
        /* Position is before the file,
         * so nothing is read and error is returned. */
        *readbytes = 0;
        /* +++Find out what error should be returned. */
        return (POS_ERR_ACCESS_DENIED);
    }
    if (!(fatfile->dir))
    {
        /* +++Remove (unsigned long) after support routine
         * is added to near.asm. */
        fatfile->byteUpto = (unsigned long)fatfile->currpos % fat->sector_size;
        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % (fat->sectors_per_cluster
                                 * fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
    }
    /* until we reach the end of the chain
     * (ignored if we are reading a fixed size root directory) */
    while (!fatEndCluster(fat, fatfile->currentCluster)
           || ((fatfile->root) && (fat->rootsize)))
    {
        /* assume all sectors in cluster are available */
        sectorsAvail = fatfile->sectorCount;

        if (fatEndCluster(fat, fatfile->nextCluster) && !fatfile->dir)
        {
            if (fatfile->fileSize == 0)
            {
                sectorsAvail = 0;
            }
            /* exception - a full cluster has 0 last sectors */
            else if ((fatfile->lastSectors == 0) && (fatfile->lastBytes == 0))
            {
                /* do nothing */
            }
            else
            {
                /* reduce sectors available */
                sectorsAvail = fatfile->lastSectors + 1;
            }
        }

        /* cycle through the sectors */
        while (fatfile->sectorUpto != sectorsAvail)
        {
            /* assume whole sector is available */
            bytesAvail = fat->sector_size;

            /* but if this is the last cluster, we need special processing */
            if (fatEndCluster(fat, fatfile->nextCluster) && !fatfile->dir)
            {
                /* exception - a full cluster has 0 last sectors */
                if ((fatfile->lastSectors == 0)
                    && (fatfile->lastBytes == 0))
                {
                    /* do nothing */
                }
                /* if we have reached the last sector, abide by lastBytes */
                else if (fatfile->sectorUpto == fatfile->lastSectors)
                {
                    bytesAvail = fatfile->lastBytes;
                }
            }
            /* while we haven't used up the bytesAvail */
            while (fatfile->byteUpto != bytesAvail)
            {
                /* read data */
                fatReadLogical(fat,
                               fatfile->sectorStart + fatfile->sectorUpto,
                               bbuf);
                /* copy data to supplied buffer */
                if ((bytesAvail - fatfile->byteUpto)
                    > (szbuf - bytesRead))
                {
                    memcpy((char *)buf + bytesRead,
                           bbuf + fatfile->byteUpto,
                           szbuf - bytesRead);
                    fatfile->currpos += (szbuf - bytesRead);
                    fatfile->byteUpto += (szbuf - bytesRead);
                    bytesRead = szbuf;
                    *readbytes = bytesRead;
                    return (0);
                }
                else
                {
                    memcpy((char *)buf + bytesRead,
                           bbuf + fatfile->byteUpto,
                           bytesAvail - fatfile->byteUpto);
                    bytesRead += (bytesAvail - fatfile->byteUpto);
                    fatfile->currpos += (bytesAvail - fatfile->byteUpto);
                    fatfile->byteUpto += (bytesAvail - fatfile->byteUpto);
                }
            }
            fatfile->sectorUpto++;
            fatfile->byteUpto = 0;
        }
        /* don't go past EOF */
        if (fatEndCluster(fat, fatfile->nextCluster) && !fatfile->dir)
        {
            /* exception - a full cluster has 0 last sectors */
            if ((fatfile->lastSectors == 0)
                && (fatfile->lastBytes == 0))
            {
                /* do nothing */
            }
            else
            {
                fatfile->sectorUpto = ((unsigned long)fatfile->currpos %
                                      (fat->sectors_per_cluster * MAXSECTSZ))
                                       / MAXSECTSZ;
                fatfile->byteUpto = (unsigned long)fatfile->currpos
                                    % fat->sector_size;
            }
            *readbytes = bytesRead;
            return (0);
        }
        fatfile->currentCluster = fatfile->nextCluster;
        fatClusterAnalyse(fat,
                          fatfile->currentCluster,
                          &fatfile->sectorStart,
                          &fatfile->nextCluster);
        fatfile->sectorUpto = 0;
    }
    *readbytes = bytesRead;
    return (0);
}

/*
 * fatPosition - given a filename, retrieve the starting
 * cluster, or set not found flag if not found. pos_result
 * is also set so other functions have more information.
 */

static void fatPosition(FAT *fat, const char *fnm)
{
    fat->notfound = 0;
    fat->pos_result = FATPOS_FOUND;
    /* Resets c_path pointer. */
    fat->c_path = fat->corrected_path;
    fat->upto = fnm;
    fat->currcluster = 0;
    fat->found_deleted = 0;
    fat->processing_root = 0;
    fatNextSearch(fat, fat->search, &fat->upto);
    if (fat->notfound) return;
    fatRootSearch(fat, fat->search);
    if (fat->last) fat->processing_root = 1;
    if (fat->notfound)
    {
        if (fat->found_deleted)
        {
            fat->currcluster = fat->temp_currcluster;
            fat->de = fat->temp_de;
            fat->dirSect = fat->temp_dirSect;
            /* Reads the sector in which deleted entry was found. */
            fatReadLogical(fat, fat->dirSect, fat->dbuf);
            fat->pos_result = FATPOS_ONEMPTY;
        }
        /* Add null terminator to corrected_path
         * if we are positioned on empty DIRENT
         * or at the endcluster. */
        if (fat->pos_result == FATPOS_ONEMPTY ||
        fat->pos_result == FATPOS_ENDCLUSTER)
        fat->c_path[0] = '\0';

        return;
    }
    while (!fat->last)
    {
        fatNextSearch(fat, fat->search, &fat->upto);
        if (fat->notfound) break;
        fat->found_deleted = 0;
        fatDirSearch(fat, fat->search);
    }
    if ((fat->pos_result == FATPOS_ONEMPTY ||
        fat->pos_result == FATPOS_ENDCLUSTER)
        && fat->found_deleted)
    {
        fat->currcluster = fat->temp_currcluster;
        fat->de = fat->temp_de;
        fat->dirSect = fat->temp_dirSect;
        /* Reads the sector in which deleted entry was found. */
        fatReadLogical(fat, fat->dirSect, fat->dbuf);
        fat->pos_result = FATPOS_ONEMPTY;
    }
    /* Add null terminator to corrected_path
     * if we are positioned on empty DIRENT
     * or at the endcluster. */
    if (fat->pos_result == FATPOS_ONEMPTY ||
    fat->pos_result == FATPOS_ENDCLUSTER)
    fat->c_path[0] = '\0';

    return;
}


/*
 * fatNextSearch - get the next part of the filename in order
 * to do a search.
 *
 * Inputs:
 * fat - pointer to the current FAT object, unused.
 * *upto - the bit of the filename we are ready to search
 *
 * Outputs:
 * *upto - the bit of the filename to search next time
 * search - set to the portion of the filename we should look
 *          for in the current directory
 * fat->last - set to 1 or 0 depending on whether this was the last
 *         portion of the filename.(int last defined in struct FAT)
 *
 * e.g. if the original filename was \FRED\MARY\JOHN.TXT and
 * *upto was pointing to MARY... on input, search will get
 * "MARY            " in it, and *upto will point to "JOHN...".
 */

static void fatNextSearch(FAT *fat, char *search, const char **upto)
{
    const char *p;
    const char *q;
    int i;

    unused(fat);
    p = strchr(*upto, '\\');
    q = strchr(*upto, '/');
    if (p == NULL)
    {
        p = q;
    }
    else if (q != NULL)
    {
        if (q < p)
        {
            p = q;
        }
    }
    if (p == NULL)
    {
        p = *upto + strlen(*upto);
        fat->last = 1;
    }
    else
    {
        fat->last = 0;
    }
    /* Checks if the next part of path is LFN.
     * If it is, its length is stored. */
    fat->lfn_search_len = isLFN(*upto, (unsigned int)(p - *upto));
    if (!fat->lfn_search_len)
    {
        /* Next part of path is 8.3 name and might need padding. */
        q = memchr(*upto, '.', p - *upto);
        if (q != NULL)
        {
            /* Uppers the name part. */
            for (i = 0; i < q - *upto; i++)
            {
                search[i] = toupper((*upto)[i]);
            }
            /* Pads the name part with spaces. */
            memset(search + (q - *upto), ' ', 8 - (q - *upto));
            /* Uppers the extension part. */
            for (i = 0; i < p - q - 1; i++)
            {
                (search + 8)[i] = toupper((q + 1)[i]);
            }
            /* Pads the extension part with spaces. */
            memset(search + 8 + (p - q) - 1, ' ', 3 - ((p - q) - 1));
        }
        else
        {
            /* Uppers the name part. */
            for (i = 0; i < p - *upto; i++)
            {
                search[i] = toupper((*upto)[i]);
            }
            /* Pads the name part with spaces. */
            memset(search + (p - *upto), ' ', 11 - (p - *upto));
        }
    }
    /* If the part of path is LFN, we just copy it into search. */
    else memcpy(search, *upto, p - *upto);
    /* Stores length of this part of path. */
    fat->path_part_len = p - *upto;
    if (fat->last)
    {
        *upto = p;
    }
    else
    {
        *upto = p + 1;
    }
    return;
}


/*
 * fatRootSearch - search the root directory for an entry
 * (given by "search").  The root directory is defined by a
 * starting sector number and the previously determined size
 * (number of sectors).
 */

static void fatRootSearch(FAT *fat, char *search)
{
    /* Because LFNs can cross clusters, it is
     * necessary to have LFN variables here for
     * fixed size roots only. */
    unsigned char lfn[MAXFILENAME]; /* +++Add UCS-2 support. */
    unsigned int lfn_len = 0;
    unsigned char checksum;
    if (fat->rootsize)
    {
        fatDirSectorSearch(fat, search, fat->rootstart, fat->rootsize,
                           lfn, &lfn_len, &checksum);
    }
    else
    {
        fat->currcluster = fat->rootstartcluster;
        fatDirSearch(fat, search);
    }
    return;
}


/*
 * fatDirSearch - search a directory chain looking for the
 * search string.
 */

static void fatDirSearch(FAT *fat, char *search)
{
    unsigned long nextCluster;
    /* Because LFNs can cross clusters, it is
     * necessary to have LFN variables here. */
    unsigned char lfn[MAXFILENAME]; /* +++Add UCS-2 support. */
    unsigned int lfn_len = 0;
    unsigned char checksum;
    fat->fnd=0;

    fatClusterAnalyse(fat, fat->currcluster, &fat->startSector, &nextCluster);
    fatDirSectorSearch(fat,
                       search,
                       fat->startSector,
                       fat->sectors_per_cluster,
                       lfn, &lfn_len, &checksum);

    while (fat->fnd == 0)    /* not found but not end */
    {
        if (fatEndCluster(fat, nextCluster))
        {
            fat->notfound = 1;
            fat->pos_result = FATPOS_ENDCLUSTER;
            return;
        }
        fat->currcluster = nextCluster;
        fatClusterAnalyse(fat, fat->currcluster, &fat->startSector, &nextCluster);
        fatDirSectorSearch(fat,
                           search,
                           fat->startSector,
                           fat->sectors_per_cluster,
                           lfn, &lfn_len, &checksum);
    }
    return;
}


static void fatReadLogicalCached(FAT *fat, unsigned long sector, void *buf)
{
    if (fat->cachedSector == sector)
    {
        memcpy(buf, fat->cachedBuf, 512);
    }
    else
    {
        fatReadLogical(fat, sector, buf);
        memcpy(fat->cachedBuf, buf, 512);
        fat->cachedSector = sector;
    }
    return;
}


#define fatReadLogical fatReadLogicalCached


/*
 * fatClusterAnalyse - given a cluster number, this function
 * determines the sector that the cluster starts and also the
 * next cluster number in the chain.
 */

static void fatClusterAnalyse(FAT *fat,
                              unsigned long cluster,
                              unsigned long *startSector,
                              unsigned long *nextCluster)
{
    unsigned long fatSector;
    static unsigned char buf[MAXSECTSZ];
    int offset;

    *startSector = (cluster - 2) * (unsigned long)fat->sectors_per_cluster
                   + fat->filestart;
    if (fat->fat_type == 16)
    {
        fatSector = fat->fatstart + (cluster * 2) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 2) % fat->sector_size;
        *nextCluster = buf[offset] | ((unsigned int)buf[offset + 1] << 8);
    }
    else if (fat->fat_type == 12)
    {
        fatSector = fat->fatstart + (cluster * 3 / 2) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 3 / 2) % fat->sector_size;
        *nextCluster = buf[offset];
        if (offset == (fat->sector_size - 1))
        {
            fatReadLogical(fat, fatSector + 1, buf);
            offset = -1;
        }
        *nextCluster = *nextCluster | ((unsigned int)buf[offset + 1] << 8);
        if ((cluster * 3 % 2) == 0)
        {
            *nextCluster = *nextCluster & 0xfffU;
        }
        else
        {
            *nextCluster = *nextCluster >> 4;
        }
    }
    else if (fat->fat_type == 32)
    {
        fatSector = fat->fatstart + (cluster * 4) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 4) % fat->sector_size;
        /* Highest 4 bits are reserved. */
        *nextCluster = (buf[offset]
                       | ((unsigned long)buf[offset + 1] << 8)
                       | ((unsigned long)buf[offset + 2] << 16)
                       | ((unsigned long)buf[offset + 3] << 24))
                       & 0x0fffffff;
    }
    return;
}

#undef fatReadLogical


/*
 * fatDirSectorSearch - go through a block of sectors (as specified
 * by startSector and numsectors) which consist entirely of directory
 * entries, looking for the "search" string.  When found, retrieve some
 * information about the file, including the starting cluster and the
 * file size.  If we get to the end of the directory (NUL in first
 * character of directory entry), set the notfound flag.  Otherwise,
 * if we reach the end of this block of sectors without reaching the
 * end of directory marker, we do not change anything.
 */

static void fatDirSectorSearch(FAT *fat,
                               char *search,
                               unsigned long startSector,
                               int numsectors,
                               unsigned char *lfn,
                               unsigned int *lfn_len,
                               unsigned char *checksum)
{
    int x;
    unsigned char *buf;
    DIRENT *p;
    int i;

    buf = fat->dbuf;
    for (x = 0; x < numsectors; x++)
    {
        fatReadLogical(fat, startSector + x, buf);
        for (p = (DIRENT *) buf; (unsigned char *) p < buf + fat->sector_size;
             p++)
        {
            /* Order of the conditionals is important,
             * because deleted LFNs exist and "empty"
             * LFN might also appear.
             * Order is: end?, deleted entry?, LFN? */
            /* Checks if we reached the end of the directory.
             * The end has the first name character '\0' and
             * it is an empty entry. */
            if (p->file_name[0] == '\0')
            {
                fat->fnd=1;
                fat->de = p;
                fat->dirSect = startSector + x;
                fat->notfound = 1;
                fat->pos_result = FATPOS_ONEMPTY;
                return;
            }
            /* Checks for deleted entries.
             * All deleted entries have DIRENT_DEL
             * (0xE5) at the beginning of their name. */
            else if (p->file_name[0] == DIRENT_DEL)
            {
                /* If we do not have stored deleted entry,
                 * it is stored now. */
                if (!fat->found_deleted)
                {
                    fat->found_deleted = 1;
                    fat->temp_currcluster = fat->currcluster;
                    fat->temp_de = p;
                    fat->temp_dirSect = startSector + x;
                }
            }
            /* Checks for LFN.
             * All LFNs have attribute
             * DIRENT_LFN (0x0F). */
            else if (p->file_attr == DIRENT_LFN)
            {
                *checksum = readLFNEntry(p, lfn, lfn_len);
            }
            /* If it is not the end, deleted entry or LFN,
             * it must be a normal entry (8.3 name). */
            else
            {
                /* If we are looking for LFN, this should
                 * be 8.3 entry for it. */
                if (fat->lfn_search_len)
                {
                    /* Checks if the found LFN has the
                     * same length as the one we search for. */
                    if (fat->lfn_search_len == *lfn_len &&
                        /* Checks if the checksum corresponds with 8.3 name. */
                        generateChecksum(p->file_name) == *checksum &&
                        /* Case-insensitive comparison to check
                         * if they are same. */
                        !cicmp(fat->search, lfn, *lfn_len))
                    {
                        /* Does the same as is done when 8.3 name is found. */
                        fat->fnd=1;
                        fat->currcluster = p->start_cluster[0]
                            | ((unsigned long) p->start_cluster[1] << 8);
                        if (fat->fat_type == 32)
                        {
                            fat->currcluster = fat->currcluster |
                            (((unsigned long) p->access_rights[0] << 16) |
                             ((unsigned long) p->access_rights[1] << 24));
                        }
                        fat->de = p;
                        fat->dirSect = startSector + x;
                        /* Adds the LFN to fat->corrected_path. */
                        memcpy(fat->c_path, lfn, *lfn_len);
                        fat->c_path += *lfn_len;
                        /* Adds '\' if the path continues,
                         * otherwise adds null terminator. */
                        if (!fat->last) fat->c_path[0] = '\\';
                        else fat->c_path[0] = '\0';
                        fat->c_path++;
                        return;
                    }

                    /* lfn_len is set to 0 to not use this LFN again. */
                    *lfn_len = 0;
                }

                /* If we are looking for 8.3 name, check if the
                 * entry is not the one we are looking for. */
                else if (memcmp(p->file_name, search, 11) == 0)
                {
                    fat->fnd=1;
                    fat->currcluster = p->start_cluster[0]
                        | ((unsigned long) p->start_cluster[1] << 8);
                    if (fat->fat_type == 32)
                    {
                        fat->currcluster = fat->currcluster |
                        (((unsigned long) p->access_rights[0] << 16) |
                         ((unsigned long) p->access_rights[1] << 24));
                    }
                    fat->de = p;
                    fat->dirSect = startSector + x;
                    /* Checks, if there is LFN associated
                     * with this 8.3 entry. */
                    if (*lfn_len &&
                        generateChecksum(p->file_name) == *checksum)
                    {
                        /* Adds the LFN to fat->corrected_path. */
                        memcpy(fat->c_path, lfn, *lfn_len);
                        fat->c_path += *lfn_len;
                        /* Adds '\' if the path continues,
                         * otherwise adds null terminator. */
                        if (!fat->last) fat->c_path[0] = '\\';
                        else fat->c_path[0] = '\0';
                        fat->c_path++;
                    }
                    else
                    {
                        /* If no LFN is found, upper the original
                         * path part and add it to corrected_path. */
                        if (!fat->last)
                        {
                            /* If it is not the last part of the path,
                             * *upto points at the character after '\',
                             * not at the '\' as we need, so we read
                             * from fat->upto - 1 - fat->path_part_len.*/
                            for (i = 0; i < fat->path_part_len; i++)
                            {
                                fat->c_path[i] =
                                toupper((fat->upto - 1
                                         - fat->path_part_len)[i]);
                            }
                        }
                        else
                        {
                            /* It is the last part of the path, so
                             * *upto points to '\', so we read from
                             * fat->upto - fat->path_part_len. */
                            for (i = 0; i < fat->path_part_len; i++)
                            {
                                fat->c_path[i] =
                                toupper((fat->upto
                                         - fat->path_part_len)[i]);
                            }
                        }
                        fat->c_path += fat->path_part_len;
                        /* Adds '\' if the path continues,
                         * otherwise adds null terminator. */
                        if (!fat->last) fat->c_path[0] = '\\';
                        else fat->c_path[0] = '\0';
                        fat->c_path++;
                    }
                    return;
                }
            }
        }
    }
    return;
}


/*
 * fatReadLogical - read a logical disk sector by calling the
 * function provided at initialization.
 */

static void fatReadLogical(FAT *fat, unsigned long sector, void *buf)
{
    fat->readLogical(fat->parm, sector, buf);
    return;
}

/* Functions for LFN. */
/* Checks if the filename is LFN (Case-insensitive).
 * If it is LFN, returns length, otherwise 0*/
static unsigned int isLFN(const char *fnm, unsigned int length)
{
    const char *p;
    int i;

    /* Checks if it is too long for 8.3 name. */
    if (length > 12) return (length);

    p = memchr(fnm, '.', length);
    /* Checks if the name or extension are not too long. */
    if (p && ((p - fnm > 8) || (length - (p - fnm) - 1 > 3)))  return (length);
    if (!p && length > 8) return length;

    /* Loops over the filename and checks
     * for invalid characters in 8.3 name. */
    for (i = 0; i < length; i++)
    {
        /* p points to the last dot in filename. */
        if (i == p - fnm) continue;
        /* Invalid characters are + , . ; = [ ] and all non-ASCII. */
        /* +++Add check fnm[i] > 127 when implementing UCS-2 support. */
        if (fnm[i] == '+' || fnm[i] == ',' || fnm[i] == '.' ||
            fnm[i] == ';' || fnm[i] == '=' || fnm[i] == '[' ||
            fnm[i] == ']') return (length);
    }

    /* +++Add logic for UCS-2 (Unicode). */
    return (0);
}

/* Reads LFN entry and puts the characters into
 * provided array. Also reads checksum and returns
 * it. */
/* +++Add support for UCS-2. */
unsigned char readLFNEntry(DIRENT *p, unsigned char *lfn,
                           unsigned int *length)
{
    unsigned int lfn_place;
    int i;
    int offset;

    /* LFN entries go from the end to beginning,
     * with each one having 13 Unicode characters. */
    /* Structure of LFN entry (both ends are included):
     * 0 = Ordinal field
     * 1 - 10 = 5 UCS-2 characters
     * 11 = Attribute (DIRENT_LFN)
     * 12 = Reserved (always 0x00)
     * 13 = Checksum (takes place of p->first_char)
     * 14 - 25 = 6 UCS-2 characters
     * 26 - 27 = Start cluster (always 0x0000)
     * 28 - 31 = 2 UCS-2 characters */
    /* Ordinal field (both ends are included):
     * 0 - 5 = Place of LFN entry in the order
     * 6 = Last LFN entry (so the first one on disk)
     * 7 = Deleted LFN (???) */
    /* +++Find explanation for deleted LFN bit. */
    /* Ordinal field information is one based,
     * not zero based. */
    lfn_place = (p->file_name[0] & 0x3f) - 1;
    /* Checks if it is not the last LFN entry. */
    if (p->file_name[0] & 0x40)
    {
        /* Read the characters from the entry. Because
         * it is the last LFN entry, it might not have
         * all 13 UCS-2 characters. */
        /* First 5 characters are at 1 - 11,
         * so offset is 1. */
        offset = 1;
        for (i = 0; i < 13; i++)
        {
            /* Second 6 characters are at 14 - 25, so 4
             * bytes offset. 3 non-character bytes are
             * between first 5 and second 6 characters.*/
            if (i == 5) offset += 3;
            /* Third 2 characters are at 28 - 31, so 6
             * bytes offset. 2 non-character bytes are
             * between second 6 and third 2 characters.*/
            if (i == 11) offset += 2;

            /* Checks if it is not 0x0000, what marks
             * the end of the LFN. */
            if (p->file_name[i * 2 + offset] == 0x00 &&
                p->file_name[i * 2 + offset + 1] == 0x00)
            break;

            /* First byte of UCS-2 character is ASCII
             * character very often and the second one is
             * just 0x00. */
            lfn[lfn_place*13+i] = p->file_name[i * 2 + offset];
            /* +++Add support for UCS-2. Second byte is
             * p->file_name[i * 2 + offset + 1]. */
        }
        /* Sets length to the length of LFN. Because
         * this is the last LFN entry, lfn_place * 13
         * + i gives us the length. */
        *length = lfn_place * 13 + i;
    }
    else
    {
        /* Read the characters from the entry. */
        /* First 5 characters are at 1 - 11,
         * so offset is 1. */
        offset = 1;
        for (i = 0; i < 13; i++)
        {
            /* Second 6 characters are at 14 - 25, so 4
             * bytes offset. 3 non-character bytes are
             * between first 5 and second 6 characters.*/
            if (i == 5) offset += 3;
            /* Third 2 characters are at 28 - 31, so 6
             * bytes offset. 2 non-character bytes are
             * between second 6 and third 2 characters.*/
            if (i == 11) offset += 2;

            /* First byte of UCS-2 character is ASCII
             * character very often and the second one is
             * just 0x00. */
            lfn[lfn_place*13+i] = p->file_name[i * 2 + offset];
            /* +++Add support for UCS-2. Second byte is
             * p->file_name[i * 2 + offset + 1]. */
        }
    }
    /* Returns checksum read from 13. byte. */
    return (p->first_char);
}

/* Generates checksum from 8.3 name. */
unsigned char generateChecksum(const char *fnm)
{
    /* Steps are:
     * 1. Use 0 as start.
     * 2. Rotate the sum bitwise right.
     * 3. Add ASCII value of a character from 8.3 name
     * (go from start to end and do not use the dot, as usual).
     * 4. Repeat 2. and 3. until all characters (11) are used. */
    unsigned char checksum = 0;
    int i;

    for (i = 0; i < 11; i++)
    {
        /* Bitwise right rotation is that we shift (move) all
         * bits to right, but preserve the bit (bit 0) that would get
         * deleted by this and put it on the beginning ( << 7). */
        checksum = ((checksum & 1) << 7) | (checksum >> 1);
        /* Adds the ASCII value of the character from 8.3 name. */
        checksum += fnm[i];
    }

    return (checksum);
}

/* Does case-insensitive comparison of 2 arrays,
 * with provided length. Returns 0 if they are
 * same, otherwise 1. */
/* +++Add UCS-2 support. Changing char to int should be enough. */
static unsigned int cicmp(const unsigned char *first,
                          const unsigned char *second,
                          unsigned int length)
{
    unsigned int i;

    for (i = 0; i < length; i++)
    {
        if (toupper(first[i]) != toupper(second[i])) return (1);
    }

    return (0);
}

/**/
