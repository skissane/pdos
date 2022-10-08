/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fat - file allocation table routines                             */
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

static void checkFSInfoSignature(FAT *fat);
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
                               unsigned char *checksum,
                               DIRENT **lfn_start_de,
                               unsigned long *lfn_start_cluster,
                               unsigned long *lfn_start_dirSect);
static void fatReadLogical(FAT *fat, unsigned long sector, void *buf);
static void fatWriteLogical(FAT *fat, unsigned long sector, void *buf);
static void fatMarkCluster(FAT *fat, unsigned long cluster);
static unsigned int fatFindFreeCluster(FAT *fat);
static void fatChain(FAT *fat, FATFILE *fatfile);
static void fatNuke(FAT *fat, unsigned long cluster);
static void fatPopulateDateTime(FAT *fat, DIRENT *d,
                                unsigned char update_time);
static void fatUpdateLastAccess(FAT *fat, FATFILE *fatfile);

/* Functions for LFNs. */
static unsigned int isLFN(const char *fnm, unsigned int length);
static unsigned int cicmp(const unsigned char *first,
                          const unsigned char *second,
                          unsigned int length);
static int createLFNs(FAT *fat, char *lfn, unsigned int lfn_len);
static void generate83Name(unsigned char *lfn, unsigned int lfn_len,
                           unsigned char *shortname);
static int checkNumericTail(FAT *fat, char *shortname);
static int incrementNumericTail(char *shortname);
static int findFreeSpaceForLFN(FAT *fat, unsigned int required_free,
                               char *shortname, unsigned int *numsectors);
static void deleteLFNs(FAT *fat);

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
    fat->readLogical = readLogical;
    fat->writeLogical = writeLogical;
    fat->parm = parm;
    fat->getDateTime = getDateTime;
    /* BPB passed by PDOS is already at offset 11
     * (skipping jump code and OEM info). */
    fat->sector_size = bpb[0] | ((unsigned int)bpb[1] << 8);
    /* avoid divide by zero later */
    if (fat->sector_size == 0)
    {
        return (-1);
    }
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
    /* Calculates start sector of the root and data region.
       Only for FAT 12/16. */
    fat->rootstart = fat->fatsize * fat->numfats + fat->fatstart;
    fat->rootentries = bpb[6] | ((unsigned int)bpb[7] << 8);
    fat->rootsize = fat->rootentries / (fat->sector_size / 32);
    fat->sectors_per_track = (bpb[13] | ((unsigned int)bpb[14] << 8));
    /* avoid divide by zero later */
    if (fat->sectors_per_track == 0)
    {
        return (-1);
    }
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

        /* FSInfo sector number. 2 bytes. */
        fat->fsinfosector = bpb[37] | ((unsigned int)bpb[38] << 8);
        /* Checks if the FSInfo has correct signature. */
        if (fat->fsinfosector) checkFSInfoSignature(fat);
    }
    /* Sets the externally set flag to default value. */
    fat->last_access_recording = 0;
    return (0);
}

/*
 * checkFSInfoSignature - checks signature of FSInfo sector.
 */
static void checkFSInfoSignature(FAT *fat)
{
    unsigned char buf[MAXSECTSZ];

    fatReadLogical(fat, fat->fsinfosector, buf);
    /* First signature 0x52 52 61 41. Offset 0. */
    if (buf[0] == 0x52 && buf[1] == 0x52 &&
        buf[2] == 0x61 && buf[3] == 0x41 &&
        /* Second signature 0x72 72 41 61. Offset 484. */
        buf[484] == 0x72 && buf[485] == 0x72 &&
        buf[486] == 0x41 && buf[487] == 0x61 &&
        /* Last signature 0x00 00 55 aa. Offset 508. */
        buf[508] == 0x00 && buf[509] == 0x00 &&
        buf[510] == 0x55 && buf[511] == 0xaa)
    /* If all signatures all valid, we return without changing fsinfosector. */
    return;
    /* Otherwise fsinfosector is set to 0 so functions know it is not valid. */
    fat->fsinfosector = 0;
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

unsigned int fatCreatFile(FAT *fat, const char *fnm, FATFILE *fatfile,
                          int attrib)
{
    DIRENT *p;
    int found = 0;
    unsigned char lbuf[MAXSECTSZ];
    FATFILE tempfatfile;
    int ret;
    unsigned long sector;

    if ((fnm[0] == '\\') || (fnm[0] == '/'))
    {
        fnm++;
    }
    /* don't allow filenames that start with blank or NUL */
    if ((fnm[0] == '\0') || (fnm[0] == ' '))
    {
        return (POS_ERR_ACCESS_DENIED);
    }
    fatPosition(fat,fnm);
    p = fat->de;

    if (fat->pos_result == FATPOS_FOUND)
    {
        if ((p->file_attr & DIRENT_SUBDIR) != 0)
        {
            return (POS_ERR_ACCESS_DENIED);
        }
        fat->currcluster = p->start_cluster[1] << 8
                           | p->start_cluster[0];
        if (fat->fat_type == 32)
        {
            fat->currcluster = fat->currcluster |
            (((unsigned long) p->access_rights[0] << 16) |
             ((unsigned long) p->access_rights[1] << 24));
        }
        fatNuke(fat, fat->currcluster);
        found = 1;
    }
    if (fat->pos_result == FATPOS_ENDCLUSTER)
    {
        /* MS-DOS expands directory when trying to add new
         * entry to a full cluster. */
        /* Root directory cannot be expanded if the size is already set. */
        if (fat->rootsize && fat->processing_root)
        {
            return (POS_ERR_PATH_NOT_FOUND);
        }

        fatChain(fat,&tempfatfile);
        fat->dirSect = tempfatfile.sectorStart;

        memset(lbuf, '\0', sizeof lbuf);
        for (sector = fat->dirSect;
             sector < fat->dirSect + fat->sectors_per_cluster;
             sector++)
        {
            fatWriteLogical(fat, sector, lbuf);
        }

        /* Position on the new empty slot. */
        fatPosition(fat,fnm);
        p = fat->de;
    }
    if (found || fat->pos_result == FATPOS_ONEMPTY)
    {
        /* If existing file was found, no new LFN entries are generated. */
        if (!found)
        {
            if (fat->lfn_search_len)
            {
                /* The name is LFN, so LFN entries need to be created. */
                /* createLFNs is similar to fatPosition, but it
                 * is guaranteed that it will end on empty/deleted
                 * slot or return error. */
                ret = createLFNs(fat, fat->search, fat->lfn_search_len);
                if (ret) return (ret);
                p = fat->de;
            }
            else if (fat->origshortname_len)
            {
                /* The name is mixed/lowercase 8.3 name,
                 * so one LFN entry is created to preserve case. */
                /* createLFNs is similar to fatPosition, but it
                 * is guaranteed that it will end on empty/deleted
                 * slot or return error. */
                ret = createLFNs(fat,
                                 fat->origshortname,
                                 fat->origshortname_len);
                if (ret) return (ret);
                p = fat->de;
            }
        }
        fatfile->startcluster=0;
        if (found)
        /* If the file already exists, name is not cleared and written again. */
        memset(p->file_name + sizeof(p->file_name) + sizeof(p->file_ext), '\0',
               sizeof(DIRENT) - sizeof(p->file_name) - sizeof(p->file_ext));
        else
        {
            memset(p, '\0', sizeof(DIRENT));
            memcpy(p->file_name, fat->search,
                  (sizeof(p->file_name)+sizeof(p->file_ext)));
        }
        p->file_attr = ((unsigned char)attrib) | DIRENT_ARCHIVE;
        fatfile->fileSize = 0;
        p->file_size[0] = fatfile->fileSize;
        p->file_size[1] = (fatfile->fileSize >> 8) & 0xff ;
        p->file_size[2] = (fatfile->fileSize >> 16) & 0xff ;
        p->file_size[3] = (fatfile->fileSize >> 24) & 0xff ;
        fatPopulateDateTime(fat, p, (FATDATETIME_UPDATE_MODIFY |
                                     FATDATETIME_UPDATE_CREATE |
                                     FATDATETIME_UPDATE_ACCESS));
        fatfile->last_access = (p->last_access[0]
                                | ((unsigned int) p->last_access[1] << 8));

        fatfile->dirSect = fat->dirSect;
        fatfile->dirOffset = ((unsigned char*)p - fat->dbuf);
        fatfile->lastBytes = 0;
        fatfile->currpos = 0;
        fatfile->dir = 0;

        /* This is needed if we switch to reading */
        fatfile->sectorCount = fat->sectors_per_cluster;
        fatfile->nextCluster = 0;
        fatfile->lastSectors = 0;

        fatWriteLogical(fat, fat->dirSect, fat->dbuf);
    }
    return (0);
}

unsigned int fatCreatDir(FAT *fat, const char *dnm, const char *parentname,
                         int attrib)
{
    DIRENT *p;
    DIRENT dot;
    unsigned char lbuf[MAXSECTSZ];
    unsigned long startcluster = 0;
    unsigned long parentstartcluster = 0;
    unsigned long startsector;
    FATFILE tempfatfile;
    int ret;
    unsigned long sector;
    unsigned int x;

    if ((dnm[0] == '\\') || (dnm[0] == '/'))
    {
        dnm++;
    }
    if ((parentname[0] == '\\') || (parentname[0] == '/'))
    {
        parentname++;
    }

    if(parentname[0] != '\0')
    {
        fatPosition(fat,parentname);
        if (!fat->currcluster) return (POS_ERR_PATH_NOT_FOUND);
        parentstartcluster = fat->currcluster;
    }
    else
    {
        /* If the root is parent, the dotdot start cluster is always 0. */
        parentstartcluster = 0;
    }

    fatPosition(fat,dnm);
    p = fat->de;

    if (fat->pos_result == FATPOS_FOUND)
    {
        return (POS_ERR_PATH_NOT_FOUND);
    }
    if (fat->pos_result == FATPOS_ENDCLUSTER)
    {
        /* MS-DOS expands directory when trying to add new
         * entry to a full cluster. */
        /* Root directory cannot be expanded if the size is already set. */
        if (fat->rootsize && fat->processing_root)
        {
            return (POS_ERR_PATH_NOT_FOUND);
        }

        fatChain(fat,&tempfatfile);
        fat->dirSect = tempfatfile.sectorStart;

        memset(lbuf, '\0', sizeof lbuf);
        for (sector = fat->dirSect;
             sector < fat->dirSect + fat->sectors_per_cluster;
             sector++)
        {
            fatWriteLogical(fat, sector, lbuf);
        }

        /* Position on the new empty slot. */
        fatPosition(fat,dnm);
        p = fat->de;
    }
    if (fat->pos_result == FATPOS_ONEMPTY)
    {
        if (fat->lfn_search_len)
        {
            /* The name is LFN, so LFN entries need to be created. */
            /* createLFNs is similar to fatPosition, but it
             * is guaranteed that it will end on empty/deleted
             * slot or return error. */
            ret = createLFNs(fat, fat->search, fat->lfn_search_len);
            if (ret) return (ret);
            p = fat->de;
        }
        else if (fat->origshortname_len)
        {
            /* The name is mixed/lowercase 8.3 name,
             * so one LFN entry is created to preserve case. */
            /* createLFNs is similar to fatPosition, but it
             * is guaranteed that it will end on empty/deleted
             * slot or return error. */
            ret = createLFNs(fat, fat->origshortname, fat->origshortname_len);
            if (ret) return (ret);
            p = fat->de;
        }
        startcluster = fatFindFreeCluster(fat);
        if(!(startcluster)) return (POS_ERR_PATH_NOT_FOUND);

        memset(p, '\0', sizeof(DIRENT));
        memcpy(p->file_name, fat->search,
              (sizeof(p->file_name)+sizeof(p->file_ext)));
        p->file_attr = ((unsigned char)attrib) | DIRENT_SUBDIR;
        p->start_cluster[1] = (startcluster >> 8) & 0xff;
        p->start_cluster[0] = startcluster & 0xff;
        if (fat->fat_type == 32)
        {
            p->access_rights[1] = (startcluster >> 24);
            p->access_rights[0] = (startcluster >> 16) & 0xff;
        }

        p->file_size[0] = p->file_size[1] = p->file_size[2]
            = p->file_size[3] = 0;
        fatPopulateDateTime(fat, p, (FATDATETIME_UPDATE_MODIFY |
                                     FATDATETIME_UPDATE_CREATE |
                                     FATDATETIME_UPDATE_ACCESS));

        fatWriteLogical(fat, fat->dirSect, fat->dbuf);

        fat->currcluster = startcluster;
        fatMarkCluster(fat, fat->currcluster);

        startsector = (fat->currcluster - FIRST_DATA_CLUSTER)
                * (long)fat->sectors_per_cluster
                + fat->filestart;

        memset(lbuf,'\0',sizeof(lbuf));
        for (x = 1; x < fat->sectors_per_cluster; x++)
        {
            fatWriteLogical(fat, startsector + x, lbuf);
        }

        memset(&dot,'\0',sizeof(dot));
        memset(dot.file_name, ' ', sizeof(dot.file_name)
               + sizeof(dot.file_ext));
        dot.file_name[0] = DIRENT_DOT;
        dot.file_attr = DIRENT_SUBDIR;

        dot.start_cluster[1] = (startcluster >> 8) & 0xff;
        dot.start_cluster[0] = startcluster & 0xff;
        if (fat->fat_type == 32)
        {
            dot.access_rights[1] = (startcluster >> 24);
            dot.access_rights[0] = (startcluster >> 16) & 0xff;
        }

        memcpy(lbuf,&dot,sizeof(dot));

        memset(&dot,'\0',sizeof(dot));
        memset(dot.file_name, ' ', sizeof(dot.file_name)
               + sizeof(dot.file_ext));
        dot.file_name[0] = DIRENT_DOT;
        dot.file_name[1] = DIRENT_DOT;
        dot.file_attr = DIRENT_SUBDIR;

        dot.start_cluster[1] = (parentstartcluster >> 8) & 0xff;
        dot.start_cluster[0] = parentstartcluster & 0xff;
        if (fat->fat_type == 32)
        {
            dot.access_rights[1] = (parentstartcluster >> 24);
            dot.access_rights[0] = (parentstartcluster >> 16) & 0xff;
        }

        memcpy(lbuf+sizeof(dot),&dot,sizeof(dot));

        fatWriteLogical(fat, startsector, lbuf);
    }
    return (0);
}

unsigned int fatCreatNewFile(FAT *fat, const char *fnm, FATFILE *fatfile,
                             int attrib)
{
    DIRENT *p;
    unsigned char lbuf[MAXSECTSZ];
    FATFILE tempfatfile;
    int ret;
    unsigned long sector;

    if ((fnm[0] == '\\') || (fnm[0] == '/'))
    {
        fnm++;
    }
    fatPosition(fat,fnm);
    p = fat->de;

    if (fat->pos_result == FATPOS_FOUND)
    {
        return (POS_ERR_FILE_EXISTS);
    }
    if (fat->pos_result == FATPOS_ENDCLUSTER)
    {
        /* MS-DOS expands directory when trying to add new
         * entry to a full cluster. */
        /* Root directory cannot be expanded if the size is already set. */
        if (fat->rootsize && fat->processing_root)
        {
            return (POS_ERR_PATH_NOT_FOUND);
        }

        fatChain(fat,&tempfatfile);
        fat->dirSect = tempfatfile.sectorStart;

        memset(lbuf, '\0', sizeof lbuf);
        for (sector = fat->dirSect;
             sector < fat->dirSect + fat->sectors_per_cluster;
             sector++)
        {
            fatWriteLogical(fat, sector, lbuf);
        }

        /* Position on the new empty slot. */
        fatPosition(fat,fnm);
        p = fat->de;
    }
    if (fat->pos_result == FATPOS_ONEMPTY)
    {
        if (fat->lfn_search_len)
        {
            /* The name is LFN, so LFN entries need to be created. */
            /* createLFNs is similar to fatPosition, but it
             * is guaranteed that it will end on empty/deleted
             * slot or return error. */
            ret = createLFNs(fat, fat->search, fat->lfn_search_len);
            if (ret) return (ret);
            p = fat->de;
        }
        else if (fat->origshortname_len)
        {
            /* The name is mixed/lowercase 8.3 name,
             * so one LFN entry is created to preserve case. */
            /* createLFNs is similar to fatPosition, but it
             * is guaranteed that it will end on empty/deleted
             * slot or return error. */
            ret = createLFNs(fat, fat->origshortname, fat->origshortname_len);
            if (ret) return (ret);
            p = fat->de;
        }
        fatfile->startcluster=0;
        memset(p, '\0', sizeof(DIRENT));
        memcpy(p->file_name, fat->search,
              (sizeof(p->file_name)+sizeof(p->file_ext)));
        p->file_attr = ((unsigned char)attrib) | DIRENT_ARCHIVE;
        fatfile->fileSize = 0;
        p->file_size[0] = fatfile->fileSize;
        p->file_size[1] = (fatfile->fileSize >> 8) & 0xff ;
        p->file_size[2] = (fatfile->fileSize >> 16) & 0xff ;
        p->file_size[3] = (fatfile->fileSize >> 24) & 0xff ;
        fatPopulateDateTime(fat, p, (FATDATETIME_UPDATE_MODIFY |
                                     FATDATETIME_UPDATE_CREATE |
                                     FATDATETIME_UPDATE_ACCESS));
        fatfile->last_access = (p->last_access[0]
                                | ((unsigned int) p->last_access[1] << 8));

        fatfile->dirSect = fat->dirSect;
        fatfile->dirOffset = ((unsigned char*)p - fat->dbuf);
        fatfile->lastBytes = 0;
        fatfile->currpos = 0;
        fatfile->dir = 0;

        fatWriteLogical(fat, fat->dirSect, fat->dbuf);
    }
    return (0);
}


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
        /* empty files have a start cluster of 0,
         * directories can never have a start cluster of 0 */
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
            fatfile->dir = 0;
        }
        else fatfile->dir = (fatfile->fileSize == 0);

        fatfile->attr = p->file_attr;

        fatfile->ftime= p->last_modtime[0]
                    | ((unsigned int) p->last_modtime[1] << 8);

        fatfile->fdate= p->last_moddate[0]
                    | ((unsigned int) p->last_moddate[1] << 8);

        fatfile->last_access = (p->last_access[0]
                                | ((unsigned int) p->last_access[1] << 8));

        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % ((unsigned long)fat->sectors_per_cluster
                                 * (unsigned long)fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
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

int fatReadFile(FAT *fat, FATFILE *fatfile, void *buf, size_t szbuf,
                size_t *readbytes)
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
    if (!fatfile->dir && (fatfile->currpos > fatfile->fileSize))
    {
        /* position is after EOF */
        /* so nothing is read and error is returned */
        *readbytes = 0;
        /* +++Find out what error should be returned. */
        return (POS_ERR_ACCESS_DENIED);
    }
    if (!(fatfile->dir))
    {
        fatfile->byteUpto = fatfile->currpos % fat->sector_size;
        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % ((unsigned long)fat->sectors_per_cluster
                                 * (unsigned long)fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
        /* Only files have the last access date updated. */
        if (fat->last_access_recording)
        {
            fatUpdateLastAccess(fat, fatfile);
        }
    }
    if (!((fatfile->root) && (fat->rootsize)))
    {
        /* fatfile->nextCluster has invalid value
         * because fatWriteFile was modifing the file
         * or fatSeek was used,
         * so we obtain the correct value again. */
        if (fatfile->nextCluster == 0)
        {
            fatClusterAnalyse(fat,
                              fatfile->currentCluster,
                              &fatfile->sectorStart,
                              &fatfile->nextCluster);
            /* sectorUpto is potentially wrong after a seek/write too,
               so we recalculate */
            fatfile->sectorUpto = (fatfile->currpos %
                                  (fat->sectors_per_cluster * MAXSECTSZ))
                                   / MAXSECTSZ;
        }
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
                fatfile->sectorUpto = (fatfile->currpos %
                                      (fat->sectors_per_cluster * MAXSECTSZ))
                                       / MAXSECTSZ;
                fatfile->byteUpto = fatfile->currpos % fat->sector_size;
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
 * fatWriteFile - write to an already-open file.
 */

int fatWriteFile(FAT *fat, FATFILE *fatfile, const void *buf, size_t szbuf,
                 size_t *writtenbytes)
{
    static unsigned char bbuf[MAXSECTSZ];
    size_t rem; /* remaining bytes in sector */
    size_t tsz; /* temporary size */
    size_t done; /* written bytes */
    DIRENT *d;

    if (fatfile->currpos < 0)
    {
        /* Position is negative, so nothing is written
         * and error is returned. */
        *writtenbytes = 0;
        /* +++Find out what error should be returned. */
        return (POS_ERR_ACCESS_DENIED);
    }
    /* Regardless of whether szbuf is 0 or remaining bytes is 0,
       we will not break into a new sector or cluster */
    if (szbuf == 0)
    {
        *writtenbytes = 0;
        return (0);
    }
    /* Marks fatfile->nextCluster invalid
     * as it is not kept updated during write. */
    fatfile->nextCluster = 0;

    if ((fatfile->startcluster==0)
        || ((fat->fat_type == 16) && (fatfile->startcluster == 0xfff8))
        || ((fat->fat_type == 12) && (fatfile->startcluster == 0xff8))
        || ((fat->fat_type == 32) && (fatfile->startcluster == 0xffffff8))
       )
    {
        fat->currcluster = fatFindFreeCluster(fat);
        fatfile->currentCluster = fat->currcluster;
        fatfile->startcluster = fat->currcluster;
        fatMarkCluster(fat, fat->currcluster);
        fatfile->sectorStart = (fat->currcluster - FIRST_DATA_CLUSTER)
                * (long)fat->sectors_per_cluster
                + fat->filestart;
        fatfile->sectorUpto = 0;

        fatReadLogical(fat,
                       fatfile->dirSect,
                       bbuf);

        d = (DIRENT *) (bbuf + fatfile->dirOffset);

        d->start_cluster[1] = (fat->currcluster >> 8) & 0xff;
        d->start_cluster[0] = fat->currcluster & 0xff;
        if (fat->fat_type == 32)
        {
            d->access_rights[1] = (fat->currcluster >> 24);
            d->access_rights[0] = (fat->currcluster >> 16) & 0xff;
        }

        fatWriteLogical(fat,
                        fatfile->dirSect,
                        bbuf);
    }
    /* If a new cluster is not needed, fat->currcluster
     * is restored from fatfile->currentCluster because
     * FAT structure is shared between file handles. */
    else fat->currcluster = fatfile->currentCluster;
    /* Current position in file is larger than size of file,
     * so we need to expand the file to reach it. */
    if (fatfile->currpos > fatfile->fileSize)
    {
        fatfile->lastBytes = fatfile->fileSize % MAXSECTSZ;
        /* fatSeek set everything up, so we are in the sector
         * and the cluster we should write into. */
        /* get remainder of bytes in current sector */
        rem = fat->sector_size - fatfile->lastBytes;
        if (fatfile->lastBytes != 0)
        {
            fatReadLogical(fat,
                           fatfile->sectorStart + fatfile->sectorUpto,
                           bbuf);
        }
        if (fatfile->currpos - fatfile->fileSize <= rem)
        {
            /* We are modifying only this sector to reach currpos. */
            memset(bbuf + fatfile->lastBytes, '\0',
                   fatfile->currpos - fatfile->fileSize);
            fatWriteLogical(fat,
                            fatfile->sectorStart + fatfile->sectorUpto,
                            bbuf);
            fatfile->fileSize = fatfile->currpos;
        }
        else
        {
            if (rem != 0)
            {
                /* Fills the rest of the last written sector with '\0'. */
                memset(bbuf + fatfile->lastBytes, '\0', rem);
                fatWriteLogical(fat,
                                fatfile->sectorStart + fatfile->sectorUpto,
                                bbuf);
            }
            fatfile->fileSize += rem;
            fatfile->sectorUpto++;
            /* If we are at the end of cluster, allocate next one. */
            if (fatfile->sectorUpto == fat->sectors_per_cluster)
            {
                fatChain(fat, fatfile);
            }
            /* Fills full sectors with '\0'. */
            memset(bbuf, '\0', fat->sector_size);
            while (fatfile->currpos - fatfile->fileSize > fat->sector_size)
            {
                fatWriteLogical(fat,
                                fatfile->sectorStart + fatfile->sectorUpto,
                                bbuf);
                fatfile->sectorUpto++;
                fatfile->fileSize += fat->sector_size;

                /* If we are at the end of cluster, allocate next one. */
                if (fatfile->sectorUpto == fat->sectors_per_cluster)
                {
                    fatChain(fat, fatfile);
                }
            }
            fatWriteLogical(fat,
                            fatfile->sectorStart + fatfile->sectorUpto,
                            bbuf);
            fatfile->fileSize = fatfile->currpos;
        }
    }
    /* Calculates position in the current sector from
     * absolute position. */
    fatfile->lastBytes = fatfile->currpos % MAXSECTSZ;
    /* Checks if we are not at end of last sector of the file,
     * because % will not show it. Exception is only at the absolute
     * beginning of the file, because first sector is not yet used. */
    if (fatfile->lastBytes == 0 && fatfile->currpos == fatfile->fileSize
        && fatfile->currpos)
    {
        fatfile->lastBytes = 512;
    }

    /* get remainder of bytes in current sector */
    rem = fat->sector_size - fatfile->lastBytes;

    /* If the current position is before the end of file,
     * we must always update (and thus read) this sector.
     * If the current position and the end of file are the same,
     * we check if the last written bytes were 0,
     * or the remainder is 0. If that is true, we do not
     * need to update (and thus read) this sector. */
    if ((fatfile->currpos < fatfile->fileSize) ||
        ((fatfile->lastBytes != 0) && (rem != 0)))
    {
        fatReadLogical(fat,
                       fatfile->sectorStart + fatfile->sectorUpto,
                       bbuf);
    }

    /* if the size of data to be written fits in to the remaining
       space, then update the just-read buffer */
    if (szbuf <= rem)
    {
        memcpy(bbuf + fatfile->lastBytes,
               buf,
               szbuf);
        fatWriteLogical(fat,
                        fatfile->sectorStart + fatfile->sectorUpto,
                        bbuf);
        fatfile->currpos += szbuf;
        done = szbuf;
    }
    else
    {
        /* we have more data than can fit in this sector.
           so use up remaining space */
        if (rem != 0)
        {
            memcpy(bbuf + fatfile->lastBytes,
                   buf,
                   rem);
            fatWriteLogical(fat,
                            fatfile->sectorStart + fatfile->sectorUpto,
                            bbuf);
        }
        done = rem; /* advance position in user-provided buffer */
        fatfile->currpos += rem;
        tsz = szbuf - rem; /* size of user-provided data remaining */
        fatfile->sectorUpto++;
        /* If we are at the end of cluster, find or allocate next one. */
        if (fatfile->sectorUpto == fat->sectors_per_cluster)
        {
            if (fatfile->currpos < fatfile->fileSize)
            {
                /* We are before the end of file,
                 * so we find the next cluster. */
                fatClusterAnalyse(fat, fat->currcluster,
                          &fatfile->sectorStart, &fatfile->currentCluster);
                fatfile->sectorUpto = 0;
                /* We use fatfile->currentCluster as variable for next
                 * cluster as it needs to be updated, so we do not need
                 * separate nextCluster variable. */
                fat->currcluster = fatfile->currentCluster;
                /* The sectorStart provided by fatClusterAnalyse
                 * is for the old cluster, so we calculate
                 * the new startSector ourselves. */
                fatfile->sectorStart = (fat->currcluster - FIRST_DATA_CLUSTER)
                * (long)fat->sectors_per_cluster
                + fat->filestart;
            }
            else
            {
                /* We are at the end or behind it,
                 * so we allocate the next cluster. */
                fatChain(fat, fatfile);
            }
        }
        /* Goes through data, each full sector at a time, what means that
         * there is no need to read even if we updating already written data.*/
        while (tsz > fat->sector_size)
        {
            memcpy(bbuf, (char *)buf + done, fat->sector_size);
            fatWriteLogical(fat,
                            fatfile->sectorStart + fatfile->sectorUpto,
                            bbuf);
            fatfile->sectorUpto++;

            tsz -= fat->sector_size;
            done += fat->sector_size;
            fatfile->currpos += fat->sector_size;

            /* If we are at the end of cluster, find or allocate next one. */
            if (fatfile->sectorUpto == fat->sectors_per_cluster)
            {
                if (fatfile->currpos < fatfile->fileSize)
                {
                    /* We are before the end of file,
                     * so we find the next cluster. */
                    fatClusterAnalyse(fat, fat->currcluster,
                              &fatfile->sectorStart, &fatfile->currentCluster);
                    fatfile->sectorUpto = 0;
                    /* We use fatfile->currentCluster as variable for next
                     * cluster as it needs to be updated, so we do not need
                     * separate nextCluster variable. */
                    fat->currcluster = fatfile->currentCluster;
                    /* The sectorStart provided by fatClusterAnalyse
                     * is for the old cluster, so we calculate
                     * the new startSector ourselves. */
                    fatfile->sectorStart =
                        (fat->currcluster - FIRST_DATA_CLUSTER)
                        * (long)fat->sectors_per_cluster
                        + fat->filestart;
                }
                else
                {
                    /* We are at the end or behind it,
                     * so we allocate the next cluster. */
                    fatChain(fat, fatfile);
                }
            }
        }
        /* If we are updating the last sector and would write less than there
         * was originally, the part of original remaining should be preserved.
         */
        if (fatfile->currpos + tsz < fatfile->fileSize)
        {
            fatReadLogical(fat,
                           fatfile->sectorStart + fatfile->sectorUpto,
                           bbuf);
        }
        memcpy(bbuf, (char *)buf + done, tsz);
        fatWriteLogical(fat,
                        fatfile->sectorStart + fatfile->sectorUpto,
                        bbuf);
        fatfile->currpos += tsz;
        done += tsz;
        /* I think in the circumstance where we are updating an existing
           file, we need to advance sectorUpto, but when creating a new
           file we want to stay put */
        if ((fatfile->currpos < fatfile->fileSize) && (tsz == fat->sector_size))
        {
            fatfile->sectorUpto++;
        }
    }
    /* fileSize contains the size of the entire file and it is changed
     * only if something is written after original file size. */
    if (fatfile->currpos > fatfile->fileSize)
    fatfile->fileSize = fatfile->currpos;

    /* now update the directory sector with the new file size */
    fatReadLogical(fat,
                   fatfile->dirSect,
                   bbuf);

    d = (DIRENT *) (bbuf + fatfile->dirOffset);
    d->file_size[0]=fatfile->fileSize & 0xff;
    d->file_size[1]=(fatfile->fileSize >> 8) & 0xff;
    d->file_size[2]=(fatfile->fileSize >> 16) & 0xff;
    d->file_size[3]=(fatfile->fileSize >> 24)  & 0xff;
    d->file_attr |= DIRENT_ARCHIVE;
    fatPopulateDateTime(fat, d, (FATDATETIME_UPDATE_MODIFY
                                 | FATDATETIME_UPDATE_ACCESS));
    fatfile->last_access = (d->last_access[0]
                            | ((unsigned int) d->last_access[1] << 8));

    fatWriteLogical(fat,
                    fatfile->dirSect,
                    bbuf);
    *writtenbytes = done;
    return (0);
}

static void fatPopulateDateTime(FAT *fat, DIRENT *d,
                                unsigned char update_type)
{
    FAT_DATETIME dt;
    unsigned int moddate;
    unsigned int modtime;
    unsigned int modhund;

    fat->getDateTime(&dt);
    moddate = ((dt.year - 1980) << 9)
               | (dt.month << 5)
               | dt.day;
    modtime = (dt.hours << 11)
              | (dt.minutes << 5)
              | dt.seconds / 2;
    if (update_type & FATDATETIME_UPDATE_MODIFY)
    {
        d->last_moddate[1] = (moddate >> 8);
        d->last_moddate[0] = moddate & 0xff;
        d->last_modtime[1] = (modtime >> 8);
        d->last_modtime[0] = modtime & 0xff;
    }
    if (update_type & FATDATETIME_UPDATE_CREATE)
    {
        modhund = (dt.seconds % 2) * 100 + dt.hundredths;
        d->first_char = (unsigned char)modhund;
        d->create_date[1] = (moddate >> 8);
        d->create_date[0] = moddate & 0xff;
        d->create_time[1] = (modtime >> 8);
        d->create_time[0] = modtime & 0xff;
    }
    if (update_type & FATDATETIME_UPDATE_ACCESS)
    {
        if (fat->last_access_recording)
        {
            d->last_access[1] = (moddate >> 8);
            d->last_access[0] = moddate & 0xff;
        }
    }
    return;
}

static void fatUpdateLastAccess(FAT *fat, FATFILE *fatfile)
{
    FAT_DATETIME dt;
    unsigned int access_date;
    static unsigned char bbuf[MAXSECTSZ];
    DIRENT *d;

    fat->getDateTime(&dt);
    access_date = (((dt.year - 1980) << 9)
                   | (dt.month << 5)
                   | dt.day);

    /* Directory entry is not updated if the date is correct. */
    if (fatfile->last_access == access_date) return;

    /* Updates the date stored in FATFILE. */
    fatfile->last_access = access_date;

    fatReadLogical(fat,
                   fatfile->dirSect,
                   bbuf);

    d = (DIRENT *) (bbuf + fatfile->dirOffset);
    d->last_access[1] = (access_date >> 8);
    d->last_access[0] = access_date & 0xff;

    fatWriteLogical(fat,
                    fatfile->dirSect,
                    bbuf);

    return;
}

/*
 * fatSeek - changes current read/write position, but does
 * not modify the file.
 */
long fatSeek(FAT *fat, FATFILE *fatfile, long offset, int whence)
{
    long newpos;
    unsigned long currclusters;
    unsigned long reqclusters;
    long realpos;
    int adjust;

    if (whence == SEEK_SET) newpos = offset;
    else if (whence == SEEK_CUR) newpos = fatfile->currpos + offset;
    else if (whence == SEEK_END) newpos = fatfile->fileSize + offset;
    /* Invalid whence was passed although the caller should ensure
     * that it is valid as error cannot be reported from here.
     * newpos is set to -1 to cause errors on read
     * and write to prevent data corruption. */
    else newpos = -1;

    /* If the current position is at the end or after the end of file
     * and the new position is at the end or after the end, we only need to
     * update fatfile->currpos and let fatReadFile and fatWriteFile deal
     * with it as seeking must not modify the file. */
    if (newpos >= fatfile->fileSize && fatfile->currpos >= fatfile->fileSize)
    {
        fatfile->currpos = newpos;
        /* Marks fatfile->nextCluster as invalid so fatReadFile will
           update it. */
        fatfile->nextCluster = 0;
        /* and we might be reading, so we need these things */
        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % ((unsigned long)fat->sectors_per_cluster
                                 * (unsigned long)fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
        return (newpos);
    }
    /* If the new position is zero or negative,
     * we seek to the start of the file. */
    if (newpos <= 0)
    {
        fatfile->currentCluster = fatfile->startcluster;
        fatfile->sectorStart = (fatfile->currentCluster - FIRST_DATA_CLUSTER)
        * (long)fat->sectors_per_cluster
        + fat->filestart;
        fatfile->sectorUpto = 0;
        /* Even if the new position is negative, we change the current position
         * to the new position, because DOS systems should accept
         * negative position as a valid position. */
        fatfile->currpos = newpos;
        /* Marks fatfile->nextCluster as invalid so fatReadFile will
           update it. */
        fatfile->nextCluster = 0;
        /* and we might be reading, so we need these things */
        fatfile->lastBytes = (unsigned int)
                             (fatfile->fileSize
                              % ((unsigned long)fat->sectors_per_cluster
                                 * (unsigned long)fat->sector_size));
        fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
        fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
        return (newpos);
    }
    /* (fat->sectors_per_cluster * MAXSECTSZ) is size of cluster in bytes. */
    currclusters = fatfile->currpos / (fat->sectors_per_cluster * MAXSECTSZ);

    /* If we are seeking past the end of the file, then just position on
       the end of the file instead. */
    if (fatfile->fileSize < newpos)
    {
        realpos = fatfile->fileSize;
    }
    else
    {
        realpos = newpos;
    }
    reqclusters = realpos
                  / (fat->sectors_per_cluster * MAXSECTSZ);

    /* If we are positioned at the end of a sector, then sectorUpto
       will be one too big, as we don't break into a new sector or
       cluster until we actually have new data. Adjusting by one
       byte will do the trick. */
    if ((realpos != 0) && (fatfile->fileSize == newpos))
    {
        adjust = 1;
    }
    else
    {
        adjust = 0;
    }
    fatfile->sectorUpto = ((realpos - adjust) %
                           (fat->sectors_per_cluster * MAXSECTSZ))
                            / MAXSECTSZ;

    /* If we are seeking forward, we don't need to go back
       to the start unless we have hit EOF */
    if ((reqclusters >= currclusters)
        && !fatEndCluster(fat, fatfile->currentCluster))
    {
        /* We need to seek fewer clusters because we are starting
         * from the current cluster. */
        reqclusters -= currclusters;
    }
    else
    {
        /* We are seeking before the current cluster,
         * so we have to go from the start cluster. */
        fatfile->currentCluster = fatfile->startcluster;
    }
    /* Seeks clusters until the required cluster is found. */
    for(; reqclusters > 0; reqclusters--)
    {
        fat->currcluster = fatfile->currentCluster;
        fatClusterAnalyse(fat, fat->currcluster, &(fatfile->sectorStart),
                          &(fatfile->currentCluster));
    }

    /* Update sectorStart for the current cluster. */
    fatfile->sectorStart = (fatfile->currentCluster - FIRST_DATA_CLUSTER)
        * (long)fat->sectors_per_cluster
        + fat->filestart;
    fatfile->currpos = newpos;
    /* Marks fatfile->nextCluster as invalid so fatReadFile will update it. */
    fatfile->nextCluster = 0;
    /* and we might be reading, so we need these things */
    fatfile->lastBytes = (unsigned int)
                         (fatfile->fileSize
                          % ((unsigned long)fat->sectors_per_cluster
                             * (unsigned long)fat->sector_size));
    fatfile->lastSectors = fatfile->lastBytes / fat->sector_size;
    fatfile->lastBytes = fatfile->lastBytes % fat->sector_size;
    return (newpos);
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
        fat->origshortname_len = 0;
    }
    else
    {
        fat->last = 0;
    }
    /* Checks if the next part of path is LFN.
     * If it is, its length is stored. */
    fat->lfn_search_len = isLFN(*upto, p - *upto);
    if (!fat->lfn_search_len)
    {
        /* Next part of path is 8.3 name and might need padding. */
        q = memchr(*upto, '.', p - *upto);
        if (q != NULL)
        {
            /* Uppers the name part. */
            for (i = 0; i < q - *upto; i++)
            {
                search[i] = toupper((unsigned char)((*upto)[i]));
                /* Checks if the 8.3 name containts lowercase
                 * if this is the last part of path and if yes,
                 * stores length of original path part. */
                if (fat->last && (search[i] != (*upto)[i]))
                {
                    fat->origshortname_len = p - *upto;
                }
            }
            /* Pads the name part with spaces. */
            memset(search + (q - *upto), ' ', 8 - (q - *upto));
            /* Uppers the extension part. */
            for (i = 0; i < p - q - 1; i++)
            {
                (search + 8)[i] = toupper((unsigned char)((q + 1)[i]));
                /* Checks if the 8.3 extension containts lowercase
                 * if this is the last part of path and if yes,
                 * stores length of original path part. */
                if (fat->last && ((search + 8)[i] != (q + 1)[i]))
                {
                    fat->origshortname_len = p - *upto;
                }
            }
            /* Pads the extension part with spaces. */
            memset(search + 8 + (p - q) - 1, ' ', 3 - ((p - q) - 1));
        }
        else
        {
            /* Uppers the name part. */
            for (i = 0; i < p - *upto; i++)
            {
                search[i] = toupper((unsigned char)((*upto)[i]));
                /* Checks if the 8.3 name containts lowercase
                 * if this is the last part of path and if yes,
                 * stores length of original path part. */
                if (fat->last && (search[i] != (*upto)[i]))
                {
                    fat->origshortname_len = p - *upto;
                }
            }
            /* Pads the name part with spaces. */
            memset(search + (p - *upto), ' ', 11 - (p - *upto));
        }
        /* If this is the last part of path, it is stored with case
         * preserved so mixed case 8.3 names can be used as LFNs,
         * if it has any lowercase. */
        if (fat->last && fat->origshortname_len)
        {
            memcpy(fat->origshortname, *upto, fat->origshortname_len);
        }
    }
    /* If the part of path is LFN, we just copy it into search. */
    else memcpy(search, *upto, p - *upto);
    /* Stores length of this part of path. */
    fat->path_part_len = p - *upto;
    /* sometimes people go \fred\\mary\john.txt so skip multiple
       directory separators */
    if (!fat->last)
    {
        p++;
        while ((*p == '\\') || (*p == '/'))
        {
            p++;
        }
    }
    *upto = p;
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
    /* We also store some information about start entry
     * of LFN so it will be possible to delete LFN. */
    DIRENT *lfn_start_de;
    unsigned long lfn_start_cluster;
    unsigned long lfn_start_dirSect;
    if (fat->rootsize)
    {
        fatDirSectorSearch(fat, search, fat->rootstart, fat->rootsize,
                           lfn, &lfn_len, &checksum, &lfn_start_de,
                           &lfn_start_cluster, &lfn_start_dirSect);
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
    /* We also store some information about start entry
     * of LFN so it will be possible to delete LFN. */
    DIRENT *lfn_start_de;
    unsigned long lfn_start_cluster;
    unsigned long lfn_start_dirSect;

    fat->fnd=0;

    fatClusterAnalyse(fat, fat->currcluster, &fat->startSector, &nextCluster);
    fatDirSectorSearch(fat,
                       search,
                       fat->startSector,
                       fat->sectors_per_cluster,
                       lfn, &lfn_len, &checksum, &lfn_start_de,
                       &lfn_start_cluster, &lfn_start_dirSect);

    while (fat->fnd == 0)    /* not found but not end */
    {
        if (fatEndCluster(fat, nextCluster))
        {
            fat->notfound = 1;
            fat->pos_result = FATPOS_ENDCLUSTER;
            return;
        }
        fat->currcluster = nextCluster;
        fatClusterAnalyse(fat,
                          fat->currcluster,
                          &fat->startSector,
                          &nextCluster);
        fatDirSectorSearch(fat,
                           search,
                           fat->startSector,
                           fat->sectors_per_cluster,
                           lfn, &lfn_len, &checksum, &lfn_start_de,
                           &lfn_start_cluster, &lfn_start_dirSect);
    }
    return;
}


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

    /* protect against EOF */
    if (fatEndCluster(fat, cluster)) return;

    *startSector = (cluster - FIRST_DATA_CLUSTER)
                   * (long)fat->sectors_per_cluster
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
                               unsigned char *checksum,
                               DIRENT **lfn_start_de,
                               unsigned long *lfn_start_cluster,
                               unsigned long *lfn_start_dirSect)
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
            else if (p->file_attr == DIRENT_LABEL)
            {
                /* ignore labels */
            }
            /* Checks for LFN.
             * All LFNs have attribute
             * DIRENT_LFN (0x0F). */
            else if (p->file_attr == DIRENT_LFN)
            {
                *checksum = readLFNEntry(p, lfn, lfn_len);
                /* We check using ordinal entry if this is
                 * the last (first physical) LFN entry.
                 * If first byte has 6. bit set, it is
                 * the last LFN entry. */
                if (p->file_name[0] & 0x40)
                {
                    /* Stores some information about this
                     * entry so it can be later found when
                     * it is going to be deleted. */
                    *lfn_start_de = p;
                    *lfn_start_cluster = fat->currcluster;
                    *lfn_start_dirSect = startSector + x;
                }
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
                        /* Information about the last (first physical)
                         * entry are stored in FAT structure so they
                         * can be accessed by other functions. */
                        fat->temp_de = *lfn_start_de;
                        fat->temp_currcluster = *lfn_start_cluster;
                        fat->temp_dirSect = *lfn_start_dirSect;
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
                        /* Information about the last (first physical)
                         * entry are stored in FAT structure so they
                         * can be accessed by other functions. */
                        fat->temp_de = *lfn_start_de;
                        fat->temp_currcluster = *lfn_start_cluster;
                        fat->temp_dirSect = *lfn_start_dirSect;
                        /* We also let other functions know that
                         * this file has LFN associated, so they
                         * should keep that in mind. */
                        fat->lfn_search_len = *lfn_len;
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
                                toupper((unsigned char)(fat->upto - 1
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
                                toupper((unsigned char)((fat->upto
                                               - fat->path_part_len)[i]));
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


/*
 * fatWriteLogical - write a logical disk sector by calling the
 * function provided at initialization.
 */

static void fatWriteLogical(FAT *fat, unsigned long sector, void *buf)
{
    fat->writeLogical(fat->parm, sector, buf);
    return;
}


/* fatMarkCluster - mark a cluster as end of chain */

static void fatMarkCluster(FAT *fat, unsigned long cluster)
{
    unsigned long fatSector;
    static unsigned char buf[MAXSECTSZ];
    int offset;
    unsigned long freeclusters;

    if (fat->fat_type == 16)
    {
        fatSector = fat->fatstart + (cluster * 2) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 2) % fat->sector_size;
        buf[offset] = 0xff;
        buf[offset + 1] = 0xff;
        fatWriteLogical(fat, fatSector, buf);
    }
    else if (fat->fat_type == 12)
    {
        fatSector = fat->fatstart + (cluster * 3 / 2) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 3 / 2) % fat->sector_size;

        /* Clusters divisible by 2 have format 0x23 0x?1,
         * other have format 0x3? 0x12. */

        if (cluster % 2) buf[offset] = buf[offset] | 0xf0;
        else buf[offset] = 0xff;

        if (offset == (fat->sector_size - 1))
        {
            fatWriteLogical(fat, fatSector, buf);
            fatSector++;
            fatReadLogical(fat, fatSector, buf);
            offset = -1;
        }
        if (cluster % 2) buf[offset+1] = 0xff;
        else buf[offset+1] = buf[offset+1] | 0xf;

        fatWriteLogical(fat, fatSector, buf);
    }
    if (fat->fat_type == 32)
    {
        fatSector = fat->fatstart + (cluster * 4) / fat->sector_size;
        fatReadLogical(fat, fatSector, buf);
        offset = (cluster * 4) % fat->sector_size;
        /* 4 highest bits are reserved and must not be changed. */
        buf[offset] = 0xff;
        buf[offset + 1] = 0xff;
        buf[offset + 2] = 0xff;
        buf[offset + 3] = buf[offset + 3] | 0xf;
        fatWriteLogical(fat, fatSector, buf);
        /* If FSInfo is on the disk, we update it. */
        if (fat->fsinfosector)
        {
            fatReadLogical(fat, fat->fsinfosector, buf);
            /* Free cluster count is at offset 488. */
            freeclusters = buf[488]
                  | ((unsigned long)buf[489] << 8)
                  | ((unsigned long)buf[490] << 16)
                  | ((unsigned long)buf[491] << 24);
            if (!(freeclusters > fat->sectors_per_disk
                / fat->sectors_per_cluster || freeclusters == 0xffffffff))
            {
                /* If free cluster count is valid,
                 * we decrement it and write into FSInfo. */
                freeclusters--;
                buf[488] = freeclusters & 0xff;
                buf[489] = (freeclusters >> 8) & 0xff;
                buf[490] = (freeclusters >> 16) & 0xff;
                buf[491] = (freeclusters >> 24) & 0xff;
            }

            /* Last allocated cluster is at offset 492. */
            buf[492] = cluster & 0xff;
            buf[493] = (cluster >> 8) & 0xff;
            buf[494] = (cluster >> 16) & 0xff;
            buf[495] = (cluster >> 24) & 0xff;
            fatWriteLogical(fat, fat->fsinfosector, buf);
        }
    }

    return;
}


/* fatFindFreeCluster - get next available free cluster */

static unsigned int fatFindFreeCluster(FAT *fat)
{
    static unsigned char buf[MAXSECTSZ];
    unsigned long fatSector;
    int found = 0;
    int x;
    unsigned long ret;

    if (fat->fat_type == 16)
    {
        for (fatSector = fat->fatstart;
             fatSector < fat->rootstart;
             fatSector++)
        {
            fatReadLogical(fat, fatSector, buf);
            for (x = 0; x < MAXSECTSZ; x += 2)
            {
                if ((buf[x] == 0) && (buf[x + 1] == 0))
                {
                    found = 1;
                    break;
                }
            }
            if (found) break;
        }
        if (found)
        {
            ret = (fatSector-fat->fatstart)*MAXSECTSZ/2 + x/2;
            return (ret);
        }
    }
    else if (fat->fat_type == 12)
    {
        /* FAT-12 is complicated, so we iterate over clusters instead of
         * sectors and offsets. */
        fatSector = 0;
        for (ret = 0;
             ret <= (fat->rootstart - fat->fatstart)*fat->sector_size*2/3;
             ret++)
        {
            /* Calculates offset. offset = cluster * 1.5 % sector size */
            x = (ret * 3 / 2) % fat->sector_size;
            /* Checks if currently loaded sector is the one in which currently
             * checked cluster resides. sector from fatstart = cluster * 1.5 /
             * sector size */
            if (fatSector != (ret * 3 / 2 / fat->sector_size + fat->fatstart))
            {
                /* If it is not, we set fatSector and load the sector. */
                fatSector = ret * 3 / 2 / fat->sector_size + fat->fatstart;
                fatReadLogical(fat, fatSector, buf);
            }

            if (ret % 2 && (buf[x] & 0xf0)) continue;
            else if (!(ret % 2) && buf[x]) continue;

            if (x == (fat->sector_size - 1))
            {
                fatSector++;
                fatReadLogical(fat, fatSector, buf);
                x = -1;
            }

            if (ret % 2 && buf[x+1]) continue;
            else if (!(ret % 2) && (buf[x+1] & 0xf)) continue;

            return (ret);
        }
    }
    else if (fat->fat_type == 32)
    {
        ret = 0;
        /* If FSInfo is on the disk, we read last allocated cluster from it. */
        if (fat->fsinfosector)
        {
            fatReadLogical(fat, fat->fsinfosector, buf);
            /* Last allocated cluster is at offset 492. */
            ret = buf[492]
                  | ((unsigned long)buf[493] << 8)
                  | ((unsigned long)buf[494] << 16)
                  | ((unsigned long)buf[495] << 24);
            /* If it is 0xffffffff, we should start from cluster 0x02. */
            if (ret == 0xffffffff) ret = 0x02;
            /* Checks if it is a valid cluster number.
             * If it is not, starts from cluster 0.*/
            if (ret >= fat->sectors_per_disk / fat->sectors_per_cluster)
            ret = 0;
        }
        /* Calculates from where should we start looking for clusters. */
        fatSector = fat->fatstart + (ret * 4) / fat->sector_size;
        x = (ret * 4) % fat->sector_size;

        /* Looks for free clusters starting from the last allocated cluster. */
        for (;
             fatSector < fat->rootstart;
             fatSector++)
        {
            fatReadLogical(fat, fatSector, buf);
            for (; x < MAXSECTSZ; x += 4)
            {
                /* 4 highest bits are reserved, thus ignored. */
                if ((buf[x] == 0) && (buf[x + 1] == 0) &&
                    (buf[x + 2] == 0) && ((buf[x + 3] & 0xf) == 0))
                {
                    found = 1;
                    break;
                }
            }
            if (found) break;
            /* Resets offset. */
            x = 0;
        }
        /* If we started from last allocated cluster, but did not find
         * anything, we search from fatstart to the cluster. */
        if (!found && ret)
        {
            for (fatSector = fat->fatstart;
                 fatSector <= fat->fatstart + (ret * 4) / fat->sector_size;
                 fatSector++)
            {
                fatReadLogical(fat, fatSector, buf);
                for (x = 0; x < MAXSECTSZ; x += 4)
                {
                    /* 4 highest bits are reserved, thus ignored. */
                    if ((buf[x] == 0) && (buf[x + 1] == 0) &&
                        (buf[x + 2] == 0) && ((buf[x + 3] & 0xf) == 0))
                    {
                        found = 1;
                        break;
                    }
                }
                if (found) break;
            }
        }
        if (found)
        {
            ret = (fatSector-fat->fatstart)*MAXSECTSZ/4 + x/4;
            return (ret);
        }
    }
    return (0);
}


/* fatChain - break into a new cluster */

static void fatChain(FAT *fat, FATFILE *fatfile)
{
    unsigned long fatSector;
    static unsigned char buf[MAXSECTSZ];
    int offset;
    unsigned long newcluster;
    unsigned long oldcluster;

    oldcluster = fat->currcluster;
    newcluster = fatFindFreeCluster(fat);
    if (fat->fat_type == 16)
    {
        /* for fat-16, each cluster in the FAT takes up 2 bytes */
        fatSector = fat->fatstart + (oldcluster * 2) / fat->sector_size;
        /* read the FAT sector that contains this cluster entry */
        fatReadLogical(fat, fatSector, buf);
        offset = (oldcluster * 2) % fat->sector_size;
        /* point the old cluster towards the new one */
        buf[offset] = newcluster & 0xff;
        buf[offset + 1] = newcluster >> 8;
        fatWriteLogical(fat, fatSector, buf);

        /* mark the new cluster as last */
        fatMarkCluster(fat, newcluster);

        /* update to new cluster */
        fat->currcluster = newcluster;
        fatfile->currentCluster = newcluster;

        /* update to new set of sectors */
        fatfile->sectorStart = (fat->currcluster - FIRST_DATA_CLUSTER)
                    * (long)fat->sectors_per_cluster
                    + fat->filestart;
        fatfile->sectorUpto = 0;
    }
    else if (fat->fat_type == 12)
    {
        /* for fat-12, each cluster in the FAT takes up 1.5 bytes */
        fatSector = fat->fatstart + (oldcluster * 3 / 2) / fat->sector_size;
        /* read the FAT sector that contains this cluster entry */
        fatReadLogical(fat, fatSector, buf);
        offset = (oldcluster * 3 / 2) % fat->sector_size;
        /* point the old cluster towards the new one */
        /* Clusters divisible by 2 have format 0x23 0x?1,
         * other have format 0x3? 0x12. */
        if (oldcluster % 2)
        buf[offset] = (buf[offset] & 0xf) | ((newcluster & 0xf) << 4);
        else buf[offset] = newcluster & 0xff;

        if (offset == (fat->sector_size - 1))
        {
            fatWriteLogical(fat, fatSector, buf);
            fatSector++;
            fatReadLogical(fat, fatSector, buf);
            offset = -1;
        }
        if (oldcluster % 2) buf[offset+1] = newcluster >> 4;
        else buf[offset+1] = (buf[offset+1] & 0xf0) | (newcluster >> 8);

        fatWriteLogical(fat, fatSector, buf);

        /* mark the new cluster as last */
        fatMarkCluster(fat, newcluster);

        /* update to new cluster */
        fat->currcluster = newcluster;
        fatfile->currentCluster = newcluster;

        /* update to new set of sectors */
        fatfile->sectorStart = (fat->currcluster - FIRST_DATA_CLUSTER)
                    * (long)fat->sectors_per_cluster
                    + fat->filestart;
        fatfile->sectorUpto = 0;
    }
    if (fat->fat_type == 32)
    {
        /* for fat-32, each cluster in the FAT takes up 4 bytes */
        fatSector = fat->fatstart + (oldcluster * 4) / fat->sector_size;
        /* read the FAT sector that contains this cluster entry */
        fatReadLogical(fat, fatSector, buf);
        offset = (oldcluster * 4) % fat->sector_size;
        /* point the old cluster towards the new one */
        buf[offset] = newcluster & 0xff;
        buf[offset + 1] = newcluster >> 8;
        buf[offset + 2] = newcluster >> 16;
        buf[offset + 3] = (buf[offset + 3] & 0xf0) | ((newcluster >> 24) & 0xf);
        fatWriteLogical(fat, fatSector, buf);

        /* mark the new cluster as last */
        fatMarkCluster(fat, newcluster);

        /* update to new cluster */
        fat->currcluster = newcluster;
        fatfile->currentCluster = newcluster;

        /* update to new set of sectors */
        fatfile->sectorStart = (fat->currcluster - FIRST_DATA_CLUSTER)
                    * (long)fat->sectors_per_cluster
                    + fat->filestart;
        fatfile->sectorUpto = 0;
    }

    return;
}


/*
  Delete a file by searching for its start sector and setting all the
  clusters to point to zero, by calling the fatnuke function
*/
unsigned int fatDeleteFile(FAT *fat,const char *fnm)
{
    if ((fnm[0] == '\\') || (fnm[0] == '/'))
    {
        fnm++;
    }
    fatPosition(fat,fnm);

    if(fat->notfound)
    {
        return(POS_ERR_FILE_NOT_FOUND);
    }
    else
    {
        fatNuke(fat,fat->currcluster);
        if (fat->lfn_search_len)
        {
            /* If this file has LFN, all associated
             * LFN entries must be deleted. */
            deleteLFNs(fat);
        }
        fat->de->file_name[0]=DIRENT_DEL;
        fatWriteLogical(fat, fat->dirSect, fat->dbuf);
        return(0);
    }
}
/**/

/*
  To rename a given file from old to new
*/
unsigned int fatRenameFile(FAT *fat,const char *old,const char *new)
{
    FATFILE fatfile;
    /* 260 is MAX_PATH from pos.h. */
    char fnm[260]; /* +++Add support for UCS-2. */
    char *p;
    unsigned int lfn_len;
    char old_lfn[MAXFILENAME];
    unsigned int old_lfn_len;
    DIRENT old_dirent;
    int ret;
    /* +++Add support for moving files. */

    if ((old[0] == '\\') || (old[0] == '/'))
    {
        old++;
    }
    fat->currfatfile = &fatfile;
    strcpy(fnm,old);
    p=strrchr(fnm,'\\');
    if (p == NULL)
    {
        strcpy(fnm, new);
    }
    else
    {
        strcpy(p+1,new);
    }

    fatPosition(fat,fnm);
    if(!fat->notfound) return(POS_ERR_FILE_NOT_FOUND);
    /* Stores length of LFN so it can be used later. */
    lfn_len = fat->lfn_search_len;
    if (!lfn_len)
    {
        /* fatPosition generates 8.3 name for us, so we just take it. */
        memcpy(fat->new_file, fat->search, 11);
    }
    /* Copies LFN from new. */
    else memcpy(fat->new_file, new, lfn_len);
    fatPosition(fat, old);
    if(fat->notfound) return(POS_ERR_FILE_NOT_FOUND);
    if (!lfn_len && !fat->lfn_search_len)
    {
        /* It is 8.3 to 8.3 rename, so no entries are created or deleted. */
        memcpy(fat->de->file_name, fat->new_file, 11);
        /* If the file is not a subdirectory, we must set the Archive
           attribute. */
        if (!((fat->de->file_attr) & DIRENT_SUBDIR))
        {
            fat->de->file_attr |= DIRENT_ARCHIVE;
        }
        fatWriteLogical(fat, fat->dirSect, fat->dbuf);
        return (0);
    }
    /* Stores old DIRENT in case it will be needed later. */
    memcpy(&old_dirent, fat->de, sizeof(DIRENT));
    /* Stores length of old LFN so it can be restored. */
    old_lfn_len = fat->lfn_search_len;
    if (fat->lfn_search_len)
    {
        /* Stores the old LFN. */
        memcpy(old_lfn, strchr(fat->corrected_path, '\0') - old_lfn_len - 1,
               old_lfn_len);
        /* Original has LFN, so LFN entries must be deleted. */
        deleteLFNs(fat);
    }
    /* Deletes the original DIRENT and writes data to disk. */
    fat->de->file_name[0]=DIRENT_DEL;
    fatWriteLogical(fat, fat->dirSect, fat->dbuf);
    /* fatPosition is ran again to
     * set FAT variables to proper values. */
    fatPosition(fat,fnm);
    if (lfn_len)
    {
        /* If the new name is LFN, LFN entries are created. */
        ret = createLFNs(fat, fat->new_file, lfn_len);
        if (ret)
        {
            /* If createLFNs had any problems (full root/disk),
             * we must restore old entry/entries. */
            /* fatPosition gets us on the first available slot. */
            fatPosition(fat,old);
            if (old_lfn_len)
            {
                /* If old file had LFN, LFN entries must be recreated.
                 * This might change the 8.3 name, but that should not
                 * cause any issues. */
                createLFNs(fat, old_lfn, old_lfn_len);
                memcpy(fat->de, &old_dirent, sizeof(DIRENT));
                memcpy(fat->de, fat->search, 11);
            }
            /* Otherwise we place old directory entry on the free slot. */
            else memcpy(fat->de, &old_dirent, sizeof(DIRENT));
            fatWriteLogical(fat, fat->dirSect, fat->dbuf);
            return (POS_ERR_PATH_NOT_FOUND);
        }
        /* createLFNs left us with everything
         * prepared for creating new entry. */
        memcpy(fat->de, &old_dirent, sizeof(DIRENT));
        memcpy(fat->de, fat->search, 11);
    }
    else
    {
        /* If new name is 8.3 name, fatPosition
         * was used to get onto first deleted entry.
         * No checks are necessary, because space
         * was already freed by deleteLFNs. */
        memcpy(fat->de, &old_dirent, sizeof(DIRENT));
        memcpy(fat->de, fat->search, 11);
    }
    /* If the file is not a subdirectory, we must set the Archive attribute. */
    if (!((fat->de->file_attr) & DIRENT_SUBDIR))
    {
        fat->de->file_attr |= DIRENT_ARCHIVE;
    }
    fatWriteLogical(fat, fat->dirSect, fat->dbuf);

    return (0);
}
/**/

/*To get the attributes of the file given by filename fnm*/
unsigned int fatGetFileAttributes(FAT *fat,const char *fnm,int *attr)
{
    unsigned int ret;
    FATFILE fatfile;

    ret=fatOpenFile(fat,fnm,&fatfile);
    if(ret==0)
    {
       *attr=fatfile.attr;
    }
    else
    {
       *attr=0;
    }
    return(ret);
}
/**/


/*To set the attributes of the file given by file name fnm*/
unsigned int fatSetFileAttributes(FAT *fat,const char *fnm,int attr)
{
    if ((fnm[0] == '\\') || (fnm[0] == '/'))
    {
        fnm++;
    }
    fatPosition(fat,fnm);

    if(fat->notfound)
    {
        return(POS_ERR_FILE_NOT_FOUND);
    }
    else
    {
        fat->de->file_attr = attr;
        fatWriteLogical(fat, fat->dirSect, fat->dbuf);
        return(0);
    }
}
/**/

/**/
unsigned int fatUpdateDateAndTime(FAT *fat,FATFILE *fatfile)
{
    static unsigned char bbuf[MAXSECTSZ];
    DIRENT *d;

    fatReadLogical(fat,
                   fatfile->dirSect,
                   bbuf);

    d = (DIRENT *) (bbuf + fatfile->dirOffset);
    d->last_moddate[0]= fatfile->fdate & 0xff;
    d->last_moddate[1]= (fatfile->fdate >> 8) & 0xff;
    d->last_modtime[0]= fatfile->ftime & 0xff;
    d->last_modtime[1]= (fatfile->ftime >> 8) & 0xff;

    fatWriteLogical(fat,
                    fatfile->dirSect,
                    bbuf);
    return 0;
}
/**/

/* Delete a file by setting cluster chain to zeros */
static void fatNuke(FAT *fat, unsigned long cluster)
{
    unsigned long fatSector;
    static unsigned char buf[MAXSECTSZ];
    int offset;
    unsigned long buffered = 0;
    unsigned long oldcluster;
    unsigned long deletedclusters = 0;
    unsigned long freeclusters;

    if (cluster == 0) return;
    while (!fatEndCluster(fat, cluster))
    {
        if (fat->fat_type == 16)
        {
            fatSector = fat->fatstart + (cluster * 2) / fat->sector_size;
            if (buffered != fatSector)
            {
                if (buffered != 0)
                {
                    fatWriteLogical(fat, buffered, buf);
                }
                fatReadLogical(fat, fatSector, buf);
                buffered = fatSector;
            }
            offset = (cluster * 2) % fat->sector_size;
            cluster = buf[offset + 1] << 8 | buf[offset];
            buf[offset] = 0x00;
            buf[offset + 1] = 0x00;
        }
        else if (fat->fat_type == 12)
        {
            fatSector = fat->fatstart + (cluster * 3 / 2) / fat->sector_size;
            if (buffered != fatSector)
            {
                if (buffered != 0)
                {
                    fatWriteLogical(fat, buffered, buf);
                }
                fatReadLogical(fat, fatSector, buf);
                buffered = fatSector;
            }
            offset = (cluster * 3 / 2) % fat->sector_size;
            oldcluster = cluster;

            /* Clusters divisible by 2 have format 0x23 0x?1,
             * other have format 0x3? 0x12. */
            if (oldcluster % 2)
            {
                cluster = (buf[offset] & 0xf0) >> 4;
                buf[offset] = buf[offset] & 0xf;
            }
            else
            {
                cluster = buf[offset];
                buf[offset] = 0x00;
            }
            if (offset == (fat->sector_size - 1))
            {
                fatWriteLogical(fat, fatSector, buf);
                fatSector++;
                buffered = fatSector;
                fatReadLogical(fat, fatSector, buf);
                offset = -1;
            }
            if (oldcluster % 2)
            {
                cluster = (buf[offset+1] << 4) | cluster;
                buf[offset+1] = 0x00;
            }
            else
            {
                cluster = ((buf[offset+1] & 0xf) << 8) | cluster;
                buf[offset+1] = buf[offset+1] & 0xf0;
            }
        }
        if (fat->fat_type == 32)
        {
            fatSector = fat->fatstart + (cluster * 4) / fat->sector_size;
            if (buffered != fatSector)
            {
                if (buffered != 0)
                {
                    fatWriteLogical(fat, buffered, buf);
                }
                fatReadLogical(fat, fatSector, buf);
                buffered = fatSector;
            }
            offset = (cluster * 4) % fat->sector_size;
            cluster = (buf[offset]
                       | ((unsigned long)buf[offset + 1] << 8)
                       | ((unsigned long)buf[offset + 2] << 16)
                       | ((unsigned long)buf[offset + 3] << 24))
                       & 0x0fffffff;
            /* 4 highest bits are reserved and must not be changed. */
            buf[offset] = 0x00;
            buf[offset + 1] = 0x00;
            buf[offset + 2] = 0x00;
            buf[offset + 3] = buf[offset + 3] & 0xf0;
            deletedclusters++;
        }
    }
    if (buffered != 0)
    {
        fatWriteLogical(fat, buffered, buf);
    }
    /* If we are on FAT-32 and have FSInfo, we update it. */
    if (fat->fat_type == 32 && fat->fsinfosector)
    {
        fatReadLogical(fat, fat->fsinfosector, buf);
        /* Free cluster count is at offset 488. */
        freeclusters = buf[488]
              | ((unsigned long)buf[489] << 8)
              | ((unsigned long)buf[490] << 16)
              | ((unsigned long)buf[491] << 24);
        if (!(freeclusters > fat->sectors_per_disk
            / fat->sectors_per_cluster || freeclusters == 0xffffffff))
        {
            /* If free cluster count is valid,
             * we increment it and write into FSInfo. */
            freeclusters += deletedclusters;
            buf[488] = freeclusters & 0xff;
            buf[489] = (freeclusters >> 8) & 0xff;
            buf[490] = (freeclusters >> 16) & 0xff;
            buf[491] = (freeclusters >> 24) & 0xff;
        }

        fatWriteLogical(fat, fat->fsinfosector, buf);
    }
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

/* Creates LFN entries, generates 8.3 name and
 * sets all variables so 8.3 entry will be properly
 * created. */
static int createLFNs(FAT *fat, char *lfn, unsigned int lfn_len)
{
    char shortname[11];
    char orig_lfn[MAXFILENAME]; /* +++Add UCS-2 support. */
    unsigned int required_free;
    int ret;
    unsigned char checksum;
    unsigned int numsectors;
    unsigned char *buf;
    unsigned long nextCluster;
    unsigned int x;
    DIRENT *p;
    unsigned int i;
    unsigned int offset;

    /* Stores the provided LFN because it will be changed by
     * fatPosition in findFreeSpaceForLFN. */
    memcpy(orig_lfn, lfn, lfn_len);

    /* Checks if the provided name is just mixed/lowercase 8.3 name. */
    if (fat->origshortname_len)
    {
        /* Sets flag that tells that generated name
         * is just uppercase padded original without
         * numeric tail. */
        fat->lossy_conversion = 0;
        /* fatPosition already generated suitable
         * uppercase and padded 8.3 name. */
        memcpy(shortname, fat->search, 11);
    }
    else
    {
        /* Otherwise we set a flag and generate 8.3 name ourselves. */
        fat->lossy_conversion = 1;
        generate83Name(orig_lfn, lfn_len, shortname);
    }

    /* Calculates how many free entries will
     * be need to store LFN and 8.3 entry. */
    /* 13 UCS-2 characters fit into one LFN entry. */
    required_free = lfn_len / 13 + 1;
    /* Even if it is not whole 13 characters,
     * another entry will be needed. */
    if (lfn_len % 13) required_free++;

    ret = findFreeSpaceForLFN(fat, required_free, shortname, &numsectors);
    if (ret) return (ret);
    /* Checks if the numeric tail is unique
     * if the generated name has numeric tail. */
    if (fat->lossy_conversion)
    {
        ret = checkNumericTail(fat, shortname);
        if (ret) return (ret);
    }
    /* Checksum must be generated after finding free space,
     * because that function modifies shortname if any
     * conflicts were found. */
    checksum = generateChecksum(shortname);
    buf = fat->dbuf;
    while (!fatEndCluster(fat, fat->currcluster)
           || ((fat->processing_root) && (fat->rootsize)))
    {
        /* If we are in fixed size root, we set the startSector
         * to fat->rootstart. */
        if (fat->processing_root && fat->rootsize)
        {
            fat->startSector = fat->rootstart;
        }
        /* Otherwise fatClusterAnalyse can provide us
         * with startSector. */
        else
        {
            fatClusterAnalyse(fat, fat->currcluster,
                              &fat->startSector, &nextCluster);
        }
        /* If fat->de points to a DIRENT, we should start
         * from sector findFreeSpaceForLFN decided. */
        if (fat->de) x = fat->dirSect - fat->startSector;
        /* Otherwise start from the beginning of cluster. */
        else x = 0;
        for (; x < numsectors; x++)
        {
            fatReadLogical(fat, fat->startSector + x, buf);
            for (p = (DIRENT *) buf;
                 (unsigned char *) p < buf + fat->sector_size;
                 p++)
            {
                required_free--;
                if (required_free == 0)
                {
                    /* All LFN entries are written, so we
                     * set fat->de and fat->dirSect so
                     * other functions can work with them. */
                    fat->de = p;
                    fat->dirSect = fat->startSector + x;
                    /* If we ended on deleted slot, we let the
                     * function that called this function know
                     * it. */
                    if (p->file_name[0] == DIRENT_DEL) fat->found_deleted = 1;
                    /* Copies final 8.3 name into fat->search,
                     * because other functions use it from there. */
                    memcpy(fat->search, shortname, 11);
                    /* Other functions must write fat->dbuf to
                     * write some last physical LFN entries
                     * on disk. */
                    return (0);
                }
                if (fat->de)
                {
                    /* Start from the DIRENT provided by
                     * findFreeSpaceForLFN. */
                    p = fat->de;
                    fat->de = NULL;
                    memset(p, '\0', sizeof(DIRENT));
                    /* Because this the first physical LFN
                     * entry, it should have bit 6 set to
                     * 1 to mark it is the first. */
                    p->file_name[0] = 0x40;
                }
                else memset(p, '\0', sizeof(DIRENT));
                /* Creates LFN entry. */
                /* Sets the ordinal field using OR to not change
                 * other bits than 0-5. */
                p->file_name[0] = p->file_name[0] | (required_free);
                /* Sets attribute so it will be detected as LFN entry. */
                p->file_attr = DIRENT_LFN;
                /* Sets checksum (offset 13). */
                p->first_char = checksum;
                /* First 5 characters are at 1 - 11,
                 * so offset is 1. */
                offset = 1;
                /* Writes part of LFN to the current DIRENT. */
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

                    /* Checks if we are still inside the LFN. */
                    if ((required_free - 1)*13 + i < lfn_len)
                    {
                        /* If using ASCII, first byte of UCS-2 character
                         * is the ASCII character and second one is 0x00. */
                        p->file_name[i * 2 + offset] =
                        orig_lfn[(required_free - 1)*13 + i];
                        p->file_name[i * 2 + offset + 1] = 0x00;
                        /* +++Add support for UCS-2. Second byte is
                         * p->file_name[i * 2 + offset + 1]. */
                    }
                    /* If we are at the end of LFN,
                     * the character must be set to 0x0000. */
                    else if ((required_free - 1)*13 + i == lfn_len)
                    {
                        p->file_name[i * 2 + offset] = 0x00;
                        p->file_name[i * 2 + offset + 1] = 0x00;
                    }
                    /* If we are past the end of LFN,
                     * the character must be set to 0xffff. */
                    else
                    {
                        p->file_name[i * 2 + offset] = 0xff;
                        p->file_name[i * 2 + offset + 1] = 0xff;
                    }
                }
            }
            /* If we ended using this sector, it is written
             * so the LFN entries created will be stored. */
            fatWriteLogical(fat, fat->startSector + x, buf);
        }
        /* We are working with a fixed size root directory
         * and we ran out of sectors, so we must return an error. */
        if ((fat->processing_root) && (fat->rootsize))
        {
            return (POS_ERR_PATH_NOT_FOUND);
        }
        /* Advances to the next cluster. */
        fat->currcluster = nextCluster;
    }
    /* Function findFreeSpaceForLFN makes sure that if everything
     * goes well, this function cannot have any issues. This return
     * is here just in case something went very wrong, because
     * normally the return (0) activates or it fails on finding
     * free space for LFN entries. */
    return (POS_ERR_PATH_NOT_FOUND);
}

/* Generates initial 8.3 name from LFN. If it is lossy
 * conversion (not just changed case), it will be necessary
 * to check if the same does not exist already elsewhere and
 * modify the numerical tail for it to be unique. */
static void generate83Name(unsigned char *lfn, unsigned int lfn_len,
                           unsigned char *shortname)
{
    unsigned int up_char;
    unsigned int offset = 0;
    unsigned int i;
    unsigned int dot_pos = MAXFILENAME;
    unsigned int j;

    /* Fills the short name with spaces. */
    memset(shortname, ' ', 11);
    /* Strips all leading dots and spaces by increasing
     * offset. */
    for (i = 0; i < lfn_len; i++)
    {
        if (lfn[i] == '.' || lfn[i] == ' ') offset++;
        else break;
    }
    /* Searches the LFN from end to start to find the
     * last dot. Trailing dots cannot be in LFN at this
     * stage and they should be handled elsewhere. */
    for (i = lfn_len - offset - 1; i > 0; i--)
    {
        if ((lfn + offset)[i] == '.')
        {
            /* Copies three characters after the dot into 8.3 name,
             * if there is enough of them. If the character is
             * '+', ',', ';', '=', '[', ']' or larger than 127 (0x7F)
             * (ASCII limit), it is replaced with '_'. */
            for (j = 0; j < 3 && j + i + 1 < lfn_len - offset; j++)
            {
                up_char = toupper((lfn + offset)[i + j + 1]);
                if (up_char > 0x7F || up_char == '+' || up_char == ',' ||
                    up_char == ';' || up_char == '=' || up_char == '[' ||
                    up_char == ']')
                shortname[8 + j] = '_';
                else shortname[8 + j] = (char) up_char;
            }
            /* Also position of the dot is stored so we know when to
             * stop when creating the 8 part of short_name. */
            dot_pos = i;
            break;
        }
    }
    /* Fills 8 part of the short name. All characters
     * must be uppered and invalid characters must be
     * replaced with '_'. Spaces and dots are ignored.
     * At most 6 characters can be put into 8 part to
     * keep space for numeric tail. */
    j = 0;
    /* i < lfn_len is there in case the LFN is shorter
     * than 6 characters and has no extension. */
    for (i = 0; j < 6 && i < dot_pos && i < lfn_len; i++)
    {
        if ((lfn + offset)[i] == ' ' || (lfn + offset)[i] == '.') continue;
        up_char = toupper((lfn + offset)[i]);
        if (up_char > 0x7F || up_char == '+' || up_char == ',' ||
            up_char == ';' || up_char == '=' || up_char == '[' ||
            up_char == ']')
        shortname[j] = '_';
        else shortname[j] = (char) up_char;
        j++;
    }
    /* Adds numeric tail after the last character
     * of the 8 part. */
    shortname[j] = '~';
    shortname[j + 1] = '1';
}

/* Checks if there are any files with the same generated 8.3 name
 * as shortname and if they do, increments numeric tail until it is unique. */
static int checkNumericTail(FAT *fat, char *shortname)
{
    DIRENT *stored_de;
    unsigned long stored_currcluster;
    unsigned long stored_dirSect;
    /* +++Add UCS-2 support. */
    unsigned char path[260]; /* MAX_PATH from pos.h. */
    unsigned int numsectors;
    unsigned long nextCluster;
    unsigned long startSector;
    unsigned long startcluster;
    unsigned int x;
    unsigned char *buf = fat->dbuf;
    DIRENT *p;
    int ret;
    int notfound;

    /* Stores variables from FAT which are needed for writing LFNs. */
    stored_de = fat->de;
    stored_currcluster = fat->currcluster;
    stored_dirSect = fat->dirSect;

    numsectors = fat->sectors_per_cluster;
    /* We use fatPosition with corrected_path
     * to get start cluster of directory we
     * are in. */
    if (!fat->processing_root)
    {
        strcpy(path, fat->corrected_path);
        fatPosition(fat, path);
        fat->processing_root = 0;
    }
    else
    {
        /* If it is expandable root, we use rootstartcluster. */
        if (fat->rootsize == 0)
        {
            fat->currcluster = fat->rootstartcluster;
        }
        /* If it is fixed size root, we set startSector to
         * the beginning of root and amount of sectors that
         * should be searched to the size of root. */
        else
        {
            numsectors = fat->rootsize;
            startSector = fat->rootstart;
        }
    }
    startcluster = fat->currcluster;
    /* Loops over the directory until the numeric
     * tail is unique or error occurs. */
    while (1)
    {
        notfound = 1;
        /* Loops over clusters. */
        while (!fatEndCluster(fat, fat->currcluster) ||
               (fat->processing_root && fat->rootsize))
        {
            /* Unless we are processing fixed size root,
             * we use fatClusterAnalyse to get startSector. */
            if (!(fat->processing_root && fat->rootsize))
            {
                fatClusterAnalyse(fat,
                                  fat->currcluster,
                                  &startSector,
                                  &nextCluster);
            }
            for (x = 0; x < numsectors; x++)
            {
                fatReadLogical(fat, startSector + x, buf);
                for (p = (DIRENT *) buf;
                     (unsigned char *) p < buf + fat->sector_size;
                     p++)
                {
                    if (p->file_name[0] == '\0')
                    {
                        /* If it is end of directory,
                         * we end the current dirent loop. */
                        break;
                    }
                    if (p->file_name[0] == DIRENT_DEL ||
                        p->file_attr == DIRENT_LFN)
                    {
                        /* Deleted entries and LFN entries are ignored. */
                        continue;
                    }
                    if (memcmp(p->file_name, shortname, 11) == 0)
                    {
                        /* Conflict is found, so numeric
                         * tail is incremented. */
                        ret = incrementNumericTail(shortname);
                        if (ret) return (ret);
                        notfound = 0;
                    }
                }
                if (p->file_name[0] == '\0')
                {
                    /* If it is end of directory,
                     * we end the current sector loop. */
                    break;
                }
            }
            if ((fat->processing_root && fat->rootsize)
                || p->file_name[0] == '\0')
            {
                /* If it is end of directory,
                 * we end the current cluster loop. */
                break;
            }
            fat->currcluster = nextCluster;
        }
        if (notfound)
        {
            /* Restores variables in FAT. */
            fat->de = stored_de;
            fat->currcluster = stored_currcluster;
            fat->dirSect = stored_dirSect;
            return (0);
        }
        /* Sets the currcluster to beginning of the directory. */
        fat->currcluster = startcluster;
    }
}

/* Increments numeric tail of generated 8.3 name. */
static int incrementNumericTail(char *shortname)
{
    char *p;
    char *q;

    /* Sets pointer at the last digit of the tail by setting right
     * before the first space. Also stores pointer to the first space. */
    p = memchr(shortname, ' ', 8);
    if (!p)
    {
        p = shortname + 7;
        q = NULL;
    }
    else
    {
        q = p;
        p--;
    }

    while (1)
    {
        /* If the digit we are currently on is less than 9,
         * we increment it and return. */
        if (p[0] != '9')
        {
            p[0]++;
            return (0);
        }
        /* Otherwise set it to zero. */
        p[0] = '0';
        /* Unless the character in front is '~',
         * we let another loop handle it. */
        if ((p-1)[0] != '~')
        {
            p--;
            continue;
        }
        /* If the character in front of this one is '~',
         * we must move some parts of 8.3 name. */
        if (q)
        {
            /* We have some free space in back of the name,
             * so we move all digits one character to right
             * and put '1' in front of them. */
            memmove(p + 1, p, q - p);
            p[0] = '1';
            return (0);
        }
        /* If there is no more space at the end of name, we must
         * move '~' one character to left and fill the new space
         * with '1'. */
        /* Checks if the tilde was not the first character of name,
         * because then we have no place use and must return error. */
        if (p - 1 == shortname) return (POS_ERR_PATH_NOT_FOUND);
        p -= 2;
        p[0] = '~';
        p[1] = '1';
        return (0);
    }
    return (0);
}

/* Goes through whole directory and finds free continuous
 * space for LFN entries and checks for 8.3 name conflicts. */
static int findFreeSpaceForLFN(FAT *fat, unsigned int required_free,
                               char *shortname, unsigned int *numsectors)
{
    unsigned char path[260]; /* MAX_PATH from pos.h. */
    unsigned long nextCluster;
    unsigned long startSector;
    unsigned int x;
    unsigned char *buf = fat->dbuf;
    DIRENT *p;
    unsigned int deleted_count = 0;
    int at_end = 0;
    FATFILE tempfatfile;
    int ret;
    unsigned char lbuf[MAXSECTSZ];
    unsigned long sector;

    /* We use numsectors as pointer to let the
     * other function know what we set it to. */
    *numsectors = fat->sectors_per_cluster;
    /* We use fatPosition with corrected_path
     * to get start cluster of directory we
     * are in. */
    if (fat->corrected_path[0] != '\0')
    {
        strcpy(path, fat->corrected_path);
        fatPosition(fat, path);
        fat->processing_root = 0;
    }
    /* If corrrected_path is empty, we must be in
     * the root directory. */
    else
    {
        fat->processing_root = 1;
        /* If it is expandable root, we use rootstartcluster. */
        if (fat->rootsize == 0)
        {
            fat->currcluster = fat->rootstartcluster;
        }
        /* If it is fixed size root, we set startSector to
         * the beginning of root and amount of sectors that
         * should be searched to the size of root. */
        else
        {
            *numsectors = fat->rootsize;
            startSector = fat->rootstart;
        }
    }
    /* Infinite loop is used because the code inside
     * can expand directories by adding clusters until
     * enough of free space is found. */
    while (1)
    {
        /* Unless we are processing fixed size root,
         * we use fatClusterAnalyse to get startSector. */
        if (!(fat->processing_root && fat->rootsize))
        {
            fatClusterAnalyse(fat,
                              fat->currcluster,
                              &startSector,
                              &nextCluster);
        }
        for (x = 0; x < *numsectors; x++)
        {
            fatReadLogical(fat, startSector + x, buf);
            for (p = (DIRENT *) buf;
                 (unsigned char *) p < buf + fat->sector_size;
                 p++)
            {
                /* If we are at the end of directory,
                 * we expand it. */
                if (at_end) p->file_name[0] = '\0';
                if (p->file_name[0] == '\0')
                {
                    /* End of the directory was reached,
                     * so even if there is not enough
                     * of deleted entries, we can add
                     * more and there cannot be any more
                     * conflicts with generated 8.3 name. */
                    /* If we have enough of empty entries,
                     * we can end. */
                    if (deleted_count == required_free)
                    {
                        /* We write the current sector, if we expanded
                         * the directory. */
                        if (at_end) fatWriteLogical(fat, startSector + x, buf);
                        /* Sets current cluster to cluster in which
                         * first empty entry was found. */
                        fat->currcluster = fat->temp_currcluster;
                        return (0);
                    }
                    /* If we do not have enough empty entries,
                     * we continue counting because all entries
                     * at the end and after it are empty. */
                    deleted_count++;
                    /* If this is the first free entry, pointer
                     * to it is stored and other variables too. */
                    if (deleted_count == 1)
                    {
                        fat->de = p;
                        /* Sector is stored so this sector can be
                         * accessed later by other functions. */
                        fat->dirSect = startSector + x;
                        /* Current cluster is also stored so
                         * other functions can resume from it
                         * in case multiple clusters are required. */
                        fat->temp_currcluster = fat->currcluster;
                    }
                    /* Sets the at_end to know that we are
                     * at the end of directory and any more
                     * read DIRENTs should have the first byte
                     * set to '\0' and their content should be
                     * ignored. */
                    at_end = 1;
                    /* All the rest of conditionals should be
                     * skipped, because there should be just
                     * empty entries. */
                    continue;
                }
                if (p->file_name[0] == DIRENT_DEL)
                {
                    /* There is no need to count deleted entries
                     * after we have enough of them. */
                    if (deleted_count < required_free)
                    {
                        /* deleted_count is increased and if this
                         * is first deleted entry after other entry
                         * pointer to it is stored. */
                        deleted_count++;
                        if (deleted_count == 1)
                        {
                            fat->de = p;
                            /* Sector is stored so this sector can be
                             * accessed later by other functions. */
                            fat->dirSect = startSector + x;
                            /* Current cluster is also stored so
                             * other functions can resume from it
                             * in case multiple clusters are required. */
                            fat->temp_currcluster = fat->currcluster;
                        }
                    }
                    continue;
                }
                /* Checks if the numeric tail is unique
                 * if the generated name has numeric tail. */
                if (fat->lossy_conversion &&
                    p->file_attr != DIRENT_LFN &&
                    memcmp(p->file_name, shortname, 11) == 0)
                {
                    ret = incrementNumericTail(shortname);
                    if (ret) return (ret);
                }
                /* Deleted entry count is reset when a
                 * non-deleted entry is found but only
                 * if there is not enough of them for
                 * LFN entries and 8.3 entry. */
                if (deleted_count < required_free)
                {
                    deleted_count = 0;
                }
            }
            /* We write the current sector, if we expanded
             * the directory. */
            if (at_end) fatWriteLogical(fat, startSector + x, buf);
        }
        if (fat->processing_root && fat->rootsize)
        {
            /* If we are in fixed size root and did not manage
             * to find enough empty slots for LFN entries and
             * 8.3 name, return error. */
            if (deleted_count < required_free) return (POS_ERR_PATH_NOT_FOUND);
            /* Otherwise we are at the end of fixed size root,
             * with sufficient amount of empty slots, so we
             * do the same as we would if we were still inside. */
            /* Sets current cluster to cluster in which
             * first empty entry was found. */
            fat->currcluster = fat->temp_currcluster;
            return (0);
        }
        /* Directory is extended if end cluster is reached. */
        if (fatEndCluster(fat, nextCluster))
        {
            fatChain(fat, &tempfatfile);
            /* We are extending the directory, so all
             * entries after this point are empty. */
            at_end = 1;

            memset(lbuf, '\0', sizeof lbuf);
            for (sector = tempfatfile.sectorStart;
                 sector < tempfatfile.sectorStart + fat->sectors_per_cluster;
                 sector++)
            {
                fatWriteLogical(fat, sector, lbuf);
            }
        }
        else fat->currcluster = nextCluster;
    }
}

/* Deletes LFN entries using information stored
 * by fatPosition in FAT structure. fatPosition
 * guarantees that this function cannot fail,
 * so no checks are necessary. */
static void deleteLFNs(FAT *fat)
{
    unsigned int remaining;
    unsigned long nextCluster;
    int x;
    unsigned int numsectors;
    unsigned char *buf;
    DIRENT *p;

    numsectors = fat->sectors_per_cluster;

    /* Makes information stored by fatPosition active. */
    fat->de = fat->temp_de;
    fat->currcluster = fat->temp_currcluster;
    fat->dirSect = fat->temp_dirSect;

    /* Calculates how many entries should be deleted from
     * the length of LFN. Each entry stores 13 characters
     * and the last one can store fewer. */
    remaining = fat->lfn_search_len / 13;
    if (fat->lfn_search_len % 13) remaining++;

    buf = fat->dbuf;
    while (1)
    {
        /* If we are in fixed size root, we set the startSector
         * to fat->rootstart. */
        if (fat->processing_root && fat->rootsize)
        {
            fat->startSector = fat->rootstart;
            numsectors = fat->rootsize;
        }
        /* Otherwise fatClusterAnalyse can provide us
         * with startSector. */
        else
        {
            fatClusterAnalyse(fat, fat->currcluster,
                              &fat->startSector, &nextCluster);
        }
        /* If fat->de points to a DIRENT, we should start
         * from sector in which first physical LFN entry
         * was found. */
        if (fat->de) x = fat->dirSect - fat->startSector;
        /* Otherwise start from the beginning of cluster. */
        else x = 0;
        for (; x < numsectors; x++)
        {
            fatReadLogical(fat, fat->startSector + x, buf);
            for (p = (DIRENT *) buf;
                 (unsigned char *) p < buf + fat->sector_size;
                 p++)
            {
                if (fat->de)
                {
                    /* If we are just starting, we should start
                     * from where fatPosition found first LFN entry. */
                    p = fat->de;
                    fat->de = NULL;
                }
                if (remaining == 0)
                {
                    /* All LFN entries are deleted, so we
                     * set fat->de and fat->dirSect so
                     * other functions can work with them. */
                    fat->de = p;
                    fat->dirSect = fat->startSector + x;
                    /* Other functions must write fat->dbuf to
                     * delete some last physical LFN entries
                     * on disk. */
                    return;
                }
                p->file_name[0] = DIRENT_DEL;
                remaining--;
            }
            /* If we ended using this sector, it is written
             * so the LFN entries will be deleted on disk. */
            fatWriteLogical(fat, fat->startSector + x, buf);
        }
        /* Advances to the next cluster. */
        fat->currcluster = nextCluster;
    }
}

/**/
