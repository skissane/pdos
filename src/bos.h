/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  bos.h - this is the interface to the BIOS for use by HLLs        */
/*                                                                   */
/*********************************************************************/

#define BOS_ERROR 1 

/* We multiply the BIOS tick count by 10. This enables 182 clock ticks per a
 * second rather than 18.2, which means we can do clock calculations
 * without having to use floating point.
 */
#define BOS_TICK_SCALE 10UL

/* Number of (scaled) BIOS clock ticks per a second */
#define BOS_TICK_PER_SECOND 182UL

/* Number of SI seconds per a day = 86,400 */
#define BOS_SECOND_PER_DAY 86400UL

/* Number of scaled BIOS clock ticks per a day */
#define BOS_TICK_PER_DAY (BOS_SECOND_PER_DAY*BOS_TICK_PER_SECOND)

#define BOS_TEXTMODE_ROMFONT_8X14 0x11
#define BOS_TEXTMODE_ROMFONT_8X8  0x12
#define BOS_TEXTMODE_ROMFONT_8X16 0x14

void BosPrintScreen(void); /* 5: */
unsigned int BosSetVideoMode(unsigned int mode); /* 10:0 */
void BosSetCursorType(unsigned int top, unsigned int bottom); /* 10:1 */
void BosSetCursorPosition(unsigned int page, unsigned int row, 
                          unsigned int column); /* 10:2 */
void BosReadCursorPosition(unsigned int page, /* 10:3 */
                           unsigned int *cursorStart,
                           unsigned int *cursorEnd,
                           unsigned int *row,
                           unsigned int *column);
void BosReadLightPen(int *trigger, /* 10:4 */
                    unsigned int *pcolumn,
                    unsigned int *prow1,
                    unsigned int *prow2,
                    int *crow,
                    int *ccol);
int BosSetActiveDisplayPage(int page); /* 10:5 */
                    
int BosScrollWindowUp(int numLines, /* 10:6 */
                      int attrib,
                      int startRow,
                      int startCol,
                      int endRow,
                      int endCol);
int BosScrollWindowDown(int numLines, /* 10:7 */
                        int attrib,
                        int startRow,
                        int startCol,
                        int endRow,
                        int endCol);
int BosReadCharAttrib(int page, int *ch, int *attrib); /* 10:8 */
int BosWriteCharAttrib(int page, 
                       int ch, 
                       int attrib, 
                       unsigned int num); /* 10:9 */
int BosWriteCharCursor(int page, int ch, int col, unsigned int num); /* 10:A */
int BosSetColorPalette(int id, int val); /* 10:B */
int BosWriteGraphicsPixel(int page, /* 10:C */
                          int color, 
                          unsigned int row,
                          unsigned int column);
int BosReadGraphicsPixel(int page, /* 10:D */
                         unsigned int row,
                         unsigned int column,
                         int *color);
int BosWriteText(int page, int ch, int color); /* 10:E */
int BosGetVideoMode(unsigned int *columns, unsigned int *mode, unsigned int *page); /* 10:F */

/* BosLoadActFonts - BIOS Int 10h Function 11h, SF 10h */
unsigned int BosLoadActFonts(int start,
                             int count,
                             int depth,
                             void *table,
                             int block);

int BosLoadTextModeRomFont(int font, int block); /* 10:11:{11,12,14} */

int BosVBEGetInfo(void *buffer); /* 10:4F00 */

int BosVBEGetModeInfo(unsigned int mode, void *buffer); /* 10:4F01 */

int BosVBESetMode(unsigned int mode, void *buffer); /* 10:4F02 */

int BosVBEGetMode(unsigned int *mode); /* 10:4F03 */

int BosVBEPaletteOps(unsigned int operation, /* 10:4F09 */
                     unsigned int entries,
                     unsigned int start_index,
                     void *buffer);

unsigned long BosGetMemorySize(void); /* 12 */

int BosDiskReset(unsigned int drive); /* 13:0 */

int BosDiskStatus(unsigned int drive, unsigned int *status); /* 13:1 */

int BosDiskSectorRead(void         *buffer, /* 13:2 */
                      unsigned int sectors,
                      unsigned int drive,
                      unsigned int track,
                      unsigned int head,
                      unsigned int sector);

int BosDiskSectorWrite(void         *buffer, /* 13:3 */
                       unsigned int  sectors,
                       unsigned int  drive,
                       unsigned int  track,
                       unsigned int  head,
                       unsigned int  sector);

int BosDriveParms(unsigned int drive, /* 13:8 */
                  unsigned int *tracks,
                  unsigned int *sectors,
                  unsigned int *heads,
                  unsigned int *attached,
                  unsigned char **parmtable,
                  unsigned int *drivetype);

int BosFixedDiskStatus(unsigned int drive); /* 13:10 */

/* 13:41 */
unsigned int BosLBAExtensions(unsigned int drive,
                              unsigned int *major,
                              unsigned int *support,
                              unsigned int *extra);

int BosDiskSectorRLBA(void         *buffer, /* 13:42 */
                      unsigned int sectors,
                      unsigned int drive,
                      unsigned long sector,
                      unsigned long hisector);

int BosDiskSectorWLBA(void         *buffer, /* 13:43 */
                      unsigned int sectors,
                      unsigned int drive,
                      unsigned long sector,
                      unsigned long hisector);

unsigned int BosSerialInitialize(unsigned int port, unsigned int parms);
  /* 14:0 */

unsigned int BosSerialWriteChar(unsigned int port, int ch); /* 14:1 */

unsigned int BosSerialReadChar(unsigned int port); /* 14:2 */

long BosExtendedMemorySize(void); /* 15:88 */

#ifdef __32BIT__
unsigned int BosSystemMemoryMap(unsigned char *buf,
                                int szbuf,
                                unsigned long *contval);
#endif

void BosReadKeyboardCharacter(int *scancode, int *ascii); /* 16:0 */

int BosReadKeyboardStatus(int *scancode, int *ascii); /* 16:1 */

int BosGetKeyboardShiftStatus(unsigned int *flags); /* 16:2 */

void BosSystemWarmBoot(void); /* 19: */

void BosGetSystemTime(unsigned long *ticks, unsigned int *midnight);/*1A.0*/

unsigned int BosGetSystemDate(int *century,int *year,int *month,
                              int *day);/*1A.4*/

void BosSetSystemDate(int century,int year,int month,
                              int day);/*1A.5*/

int BosPCICheck(unsigned char *hwcharacteristics, /*1A:B101*/
                unsigned char *majorversion,
                unsigned char *minorversion,
                unsigned char *lastbus);

/* Reads an arbitrary byte from BIOS Data Area (segment 0x40) */
unsigned char BosGetBiosDataAreaByte(int offset);

/* Reads an arbitrary 16-bit word from BIOS Data Area (segment 0x40) */
unsigned short BosGetBiosDataArea16(int offset);

/* Reads an arbitrary 32-bit DWORD from BIOS Data Area (segment 0x40) */
unsigned long BosGetBiosDataArea32(int offset);

/* Get BIOS text mode screen dimensions */
int BosGetTextModeCols(void);
int BosGetTextModeRows(void);
void BosClearScreen(unsigned int attr);

/* INT 15,5305 - APM 1.0+ CPU IDLE */
void BosCpuIdle(void);

/* Get BIOS clock tick count (DWORD at 40:6C).
 * Note this is not the raw value, it is multiplied by BOS_TICK_SCALE.
 */
unsigned long BosGetClockTickCount(void);

/* Sleep for given number of seconds */
void BosSleep(unsigned long seconds);
