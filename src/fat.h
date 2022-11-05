#include <stddef.h>

#ifndef FAT_INCLUDED
#define FAT_INCLUDED

#define MAXSECTSZ 512
/* 255 UCS-2 characters for LFN is
 * artificial limit set by Microsoft.
 * Null terminator is not included. */
#define MAXFILENAME 255

/* Because these first two FAT entries
store special values, there are no data
clusters 0 or 1. The first data cluster
(after the root directory if FAT12/FAT16)
is cluster 2, marking the beginning of the data area. */
#define FIRST_DATA_CLUSTER 2

#define FATPOS_FOUND 1
#define FATPOS_ONEMPTY 2
#define FATPOS_ENDCLUSTER 3

#define FATDATETIME_UPDATE_MODIFY 0x1
#define FATDATETIME_UPDATE_CREATE 0x2
#define FATDATETIME_UPDATE_ACCESS 0x4

/*File name special values*/
#define DIRENT_AVA 0x00
#define DIRENT_IC 0x05 /* what is this? Does it need to be 0x2D for EBCDIC? */
#define DIRENT_DOT '.'
#ifdef EBCDIC
#define DIRENT_DEL 0x47
#else
#define DIRENT_DEL 0xE5
#endif
/**/

/*File attributes*/
#define DIRENT_READONLY 0x01
#define DIRENT_HIDDEN 0x02
#define DIRENT_SYSTEM 0x04
#define DIRENT_LABEL 0x08
#define DIRENT_SUBDIR 0x10
#define DIRENT_ARCHIVE 0x20
#define DIRENT_DEVICE 0X40
#define DIRENT_RESERVED 0x80
#define DIRENT_LFN 0x0f
/**/

/*Last access attributes*/
#define DIRENT_EXTRAB7  0x80
#define DIRENT_EXTRAB6  0x40
#define DIRENT_EXTRAB5  0x20
#define DIRENT_EXTRAB4  0x10
#define DIRENT_EXTRAB3  0x08
#define DIRENT_EXTRAB2  0x04
#define DIRENT_EXTRAB1  0x02
#define DIRENT_EXTRAB0  0x01
/**/

/*File access rights bitmap*/
#define DIRENT_ACCESSB0 0x0001
#define DIRENT_ACCESSB1 0x0002
#define DIRENT_ACCESSB2 0x0004
#define DIRENT_ACCESSB3 0x0008
#define DIRENT_ACCESSB4 0x0010
#define DIRENT_ACCESSB5 0x0020
#define DIRENT_ACCESSB6 0x0040
#define DIRENT_ACCESSB7 0x0080
#define DIRENT_ACCESSB8 0x0100
#define DIRENT_ACCESSB9 0x0200
#define DIRENT_ACCESSB10 0x0400
#define DIRENT_ACCESSB11 0x0800
/**/

typedef struct {
    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;
    unsigned int hundredths;
} FAT_DATETIME;

/*Structure for Directory Entry */
typedef struct {
    unsigned char file_name[8]; /*Short file name (0x00)*/
    unsigned char file_ext[3];  /*Short file extension (0x08)*/
    unsigned char file_attr;    /*file attributes (0x0B)*/
    unsigned char extra_attributes;  /*extra attributes (0x0C)*/
    unsigned char first_char;        /*first character of deleted file
                                       (0x0D)*/
    unsigned char create_time[2];    /*create time(0x0E)*/
    unsigned char create_date[2];    /*create date(0x10)*/
    unsigned char last_access[2];    /*last access (0x12)*/
    unsigned char access_rights[2];  /*file access rights (0x14)
                                      *2 high bytes of start cluster (fat32)*/
    unsigned char last_modtime[2];   /*last modified time (0x16)*/
    unsigned char last_moddate[2];   /*last modified date (0x18)*/
    unsigned char start_cluster[2];  /*Size of file in clusters (0x1A)*/
    unsigned char file_size[4];      /*Size of file in bytes (0x1C) */
}DIRENT;
/**/

typedef struct {
    /* FATFILE structure is used for storing information about
     * opened/created file for working with it and each file
     * handle has one FATFILE which other file handles cannot
     * access, so the information persist even if other file
     * handle or function is used. */
    int root;
    unsigned long startcluster; /* start cluster for this file */
    unsigned long fileSize;
    unsigned int lastSectors; /* when opening a file for read, this
        contains the number of sectors expected in the last cluster,
        based on file size and cluster size. */
    unsigned int lastBytes; /* when opening a file for read, this
        contains the number of bytes expected in the last sector.
        when opening for write, it contains how many bytes were
        written in the last sector. */
    unsigned long currentCluster; /* cluster currently up to for
        reading or writing. */
    unsigned int ftime;
    unsigned int fdate;
    unsigned int last_access;
    unsigned int attr;
    unsigned int sectorCount; /* how many sectors in the current cluster */
    unsigned long sectorStart; /* starting sector for this cluster */
    unsigned long nextCluster; /* next cluster in chain */
    unsigned int byteUpto;
    unsigned int sectorUpto; /* which sector currently being processed,
    zero-based */
    unsigned long dirSect; /* sector which contains directory entry */
    unsigned int dirOffset; /* offset in sector for this directory entry */
    long currpos; /* current position in the file */
    int dir; /* whether this is a directory or a file */
} FATFILE;

typedef struct {
    /* FAT structure stores general information about file system
     * and has variables for manipulating with it (reading/modifying
     * fat cluster table...) and also temporary variables for functions.
     * FAT structure is shared between all handles on the same drive. */
    unsigned long num_tracks;
    unsigned int num_cylinders;
    unsigned int num_heads;
    unsigned int sectors_per_track;
    unsigned int sector_size;
    unsigned long sectors_per_cylinder;
    unsigned long sectors_per_disk;
    unsigned int sectors_per_cluster;
    unsigned int bytes_per_cluster;
    unsigned int reserved_sectors;
    unsigned int numfats;
    unsigned int bootstart;
    unsigned int fatstart;
    unsigned long rootstart;
    unsigned long filestart;
    unsigned int drive;
    unsigned int rootentries;
    unsigned int rootsize;
    unsigned int fsinfosector;
    unsigned long rootstartcluster;
    unsigned long fatsize;
    unsigned long data_area;
    unsigned long num_clusters;
    int fat_type;
    unsigned long hidden;
    unsigned long startSector;
    int notfound;
    int pos_result;
    int found_deleted;
    int processing_root;
    /* Flag used when creating LFN entries to know if
     * the provided name is not just mixed/lowercase 8.3 name. */
    int lossy_conversion;
    unsigned long currcluster;
    unsigned long temp_currcluster;
    FATFILE *currfatfile;
    void (*readLogical)(void *diskptr, unsigned long sector, void *buf);
    void (*writeLogical)(void *diskptr, unsigned long sector, void *buf);
    void *parm;
    void (*getDateTime)(FAT_DATETIME *ptr);
    /* +++Add support for UCS-2. */
    char new_file[MAXFILENAME]; /*new filename for rename*/
    int last;
    unsigned char search[MAXFILENAME]; /* +++Add support for UCS-2. */
    /* Length of search if search is LFN, 0 if 8.3 name. */
    unsigned int lfn_search_len;
    /* Stores original 8.3 name with case preserved. */
    char origshortname[12];
    unsigned int origshortname_len;
    /* Duplicate path but with all names replaced with read LFNs,
     * when possible and uppered 8.3 names otherwise.
     * 260 is MAX_PATH from pos.h. */
    unsigned char corrected_path[260]; /* +++Add support for UCS-2. */
    /* Pointer to corrected_path, used for modifying it. */
    unsigned char *c_path;
    /* Length of path part between 2 "\" or at the end. */
    unsigned int path_part_len;
    const char *upto;
    unsigned char *dbuf;
    DIRENT *de;
    DIRENT *temp_de;
    unsigned long dirSect; /* sector which contains directory entry */
    unsigned long temp_dirSect;
    int fnd;
    /* Flag deciding whether Last access date should be recorded or not.
     * Affects creation and modification of files, not only reading.
     * Default is 0, value is later changed externally. */
    int last_access_recording;
} FAT;


void fatDefaults(FAT *fat);
int fatInit(FAT *fat,
            unsigned char *bpb,
            void (*readLogical)(void *diskptr, unsigned long sector, void *buf),
            void (*writeLogical)(void *diskptr, unsigned long sector, void *buf),
            void *parm,
            void (*getDateTime)(FAT_DATETIME *ptr)
            );
void fatTerm(FAT *fat);
unsigned int fatCreatFile(FAT *fat, const char *fnm, FATFILE *fatfile,
                          int attrib);
unsigned int fatCreatDir(FAT *fat, const char *dnm, const char *parentname,
                         int attrib);
unsigned int fatCreatNewFile(FAT *fat, const char *fnm, FATFILE *fatfile,
                             int attrib);
unsigned int fatOpenFile(FAT *fat, const char *fnm, FATFILE *fatfile);
int fatReadFile(FAT *fat, FATFILE *fatfile, void *buf, size_t szbuf,
                size_t *readbytes);
int fatWriteFile(FAT *fat, FATFILE *fatfile, const void *buf, size_t szbuf,
                 size_t *writtenbytes);
long fatSeek(FAT *fat, FATFILE *fatfile, long offset, int whence);
unsigned int fatDeleteFile(FAT *fat,const char *fnm);
/*To delete a file from a given directory*/
unsigned int fatRenameFile(FAT *fat,const char *old,const char *new);
/*To rename a given file*/
unsigned int fatGetFileAttributes(FAT *fat,const char *fnm,int *attr);
/*To Get the file attributes from the file name specified by fnm*/
unsigned int fatSetFileAttributes(FAT *fat,const char *fnm,int attr);
/*To Set the attributes of the given file*/
unsigned int fatUpdateDateAndTime(FAT *fat,FATFILE *fatfile);
/*To update the date and time of the given file*/

/*LFN functions that are used by ff_search.*/
/*Reads LFN entry and returns checksum.*/
unsigned char readLFNEntry(DIRENT *p, unsigned char *lfn,
                           unsigned int *length);
/*Generates checksum from 8.3 name. */
unsigned char generateChecksum(const char *fnm);
#endif
