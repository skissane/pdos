/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pcomm - command processor for pdos                               */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "pos.h"
#include "dostime.h"
#include "memmgr.h"

/* In C99 or higher we would just include <stdbool.h>. But, we need to
 * support older C compilers (C89/C90) which don't come with <stdbool.h>
 */
#define true 1
#define false 0
#define bool int

/* Written By NECDET COKYAZICI, Public Domain */
void putch(int a);
void bell(void);
void safegets(char *buffer, int size);



static char buf[200];
static unsigned char cmdt[140];
static char path[500] = ";" ; /* Used to store path */
static struct {
    int env;
    unsigned char *cmdtail;
    char *fcb1;
    char *fcb2;
} parmblock = { 0, cmdt, NULL, NULL };
static size_t len;
static char drive[2] = "A";
static char cwd[65];
/* Sometimes I test PCOMM under MS-DOS.
 * And then I get confused about whether I am in PCOMM or COMMAND.COM.
 * Making the default prompt for PCOMM different from that of COMMAND.COM
 * Helps avoid that confusion.
 * - Simon Kissane
 */
static char prompt[50] = "]";
static int singleCommand = 0;
static int primary = 0;
static int term = 0;

/* Name of currently running command (in uppercase) */
static char curCmdName[50];

/* GOTO target label */
static char gotoTarget[32];

/* Flag to trigger GOTO to be taken */
static int executeGoto = 0;

/* Echo each command in batch file */
static int echoFlag = 0;

/* Abort batch file if any command fails */
static int abortFlag = 0;

/* Flag indicating batch file currently running */
static int inBatch = 0;

/* PDPCLIB defines this as pointer to environment */
extern char **__envptr;

static void parseArgs(int argc, char **argv);
static int processInput(void);
static void putPrompt(void);
static int doExec(char *b,char *p);
static int changedisk(int drive);
static int ins_strcmp(char *one, char *two);
static int ins_strncmp(char *one, char *two, size_t len);
static int readBat(char *fnm);
static int exists(char *fnm);
static int isBlankString(char *str);
static void showArgsUnsupportedMsg(void);
static void strtoupper(char *s);
static char *stringTrimBoth(char *s);

/*===== OPTION FACILITY =====*/
/* The "OPTION" command is used to view or set flags which
 * control system behaviour. Some flags may control PCOMM
 * behaviour while other flags control PDOS itself. The
 * options registry stores which options are known.
 */

/* Function signature for option get routine */
typedef int (*optGet)(void);

/* Function signature for option set routine */
typedef void (*optSet)(int flag);

/* An entry in the options registry */
typedef struct optBlock
{
    char key;
    char *name;
    char *description;
    optGet getProc;
    optSet setProc;
} optBlock;

/* Define option macro.
 * Parameters are:
 * key         - single uppercase character which uniquely identifies this
 *               option
 * name        - name of this option (lowercase, unquoted)
 * description - one-line description of what this option does
 */
#define OPTDEF(key,name,description) { key, #name, description, \
    opt_##name##_get, opt_##name##_set }

/* Sentinel record for end of command registry */
#define OPT_REGISTRY_END { 0, NULL, NULL, NULL, NULL }

/* Macro to generate prototypes for option get/set routines */
#define OPTPROTO(name) \
    static int opt_##name##_get(void); \
    static void opt_##name##_set(int);

/* Prototypes for option get/set routines */
OPTPROTO(abort);
OPTPROTO(break);
OPTPROTO(echo);
OPTPROTO(logunimp);
OPTPROTO(verify);

/* The options registry stores definitions of all options. */
static optBlock optRegistry[] =
{
    OPTDEF('A',abort,"Exit batch file on first failing command"),
    OPTDEF('B',break,"Check Ctrl+Break state more frequently"),
    OPTDEF('E',echo,"Echo each command executed in batch file"),
    OPTDEF('U',logunimp,"Log unimplemented DOS calls"),
    OPTDEF('V',verify,"Verify disk writes"),
    OPT_REGISTRY_END
};

/*===== COMMAND REGISTRY =====*/

/* Macro to generate prototypes for command do/help routines */
#define CMDPROTO(name) \
    static int cmd_##name##_run(char *arg); \
    static void cmd_##name##_help(void);

/* Prototypes for command do/help routines */
CMDPROTO(cd);
CMDPROTO(copy);
CMDPROTO(date);
CMDPROTO(del);
CMDPROTO(dir);
CMDPROTO(echo);
CMDPROTO(exit);
CMDPROTO(goto);
CMDPROTO(help);
CMDPROTO(mcd);
CMDPROTO(md);
CMDPROTO(option);
CMDPROTO(path);
CMDPROTO(pause);
CMDPROTO(prompt);
CMDPROTO(rd);
CMDPROTO(reboot);
CMDPROTO(rem);
CMDPROTO(ren);
CMDPROTO(set);
CMDPROTO(time);
CMDPROTO(type);
CMDPROTO(unboot);
CMDPROTO(ver);

/* Function signature for command implementation functions */
typedef int (*cmdProc)(char *arg);

/* Function signature for command help display functions */
typedef void (*cmdHelp)(void);

/* Defines an internal command */
typedef struct cmdBlock
{
    char *name;
    char *description;
    cmdProc proc;
    cmdHelp help;
}
cmdBlock;

/* Define command macro.
 * Parameters are:
 * name - name of this command (lowercase, unquoted)
 * aliases - empty string "" for no aliases. Otherwise each alias in string
 *           prefixed with pipe character.
 * description - one-line description of command for help system.
 */
#define CMDDEF(name,aliases,description) { #name aliases , description , \
    cmd_##name##_run, cmd_##name##_help }

/* Sentinel record for end of command registry */
#define CMD_REGISTRY_END { NULL, NULL, NULL, NULL }

/* The command registry stores definitions of all internal commands.
 */
static cmdBlock cmdRegistry[] =
{
    CMDDEF(cd,"|chdir","Changes the current directory"),
    CMDDEF(copy,"","Copies files and directories"),
    CMDDEF(date,"","Shows the date"),
    CMDDEF(del,"|erase","Deletes files"),
    CMDDEF(dir,"","Lists files and directories"),
    CMDDEF(echo,"","Displays a message"),
    CMDDEF(exit,"","Exits PCOMM"),
    CMDDEF(goto,"","GOTO label in batch file"),
    CMDDEF(help,"","Provides information about PDOS commands"),
    CMDDEF(mcd,"","Make new directory and change to it"),
    CMDDEF(md,"|mkdir","Creates new directories"),
    CMDDEF(option,"","Controls behavioural flags"),
    CMDDEF(path,"","Displays or modifies PATH variable"),
    CMDDEF(pause,"","Wait for user to press any key"),
    CMDDEF(prompt,"","Displays or modifies PROMPT variable"),
    CMDDEF(rd,"|rmdir","Removes directories"),
    CMDDEF(reboot,"","Reboots the computer"),
    CMDDEF(rem,"","Comment in a batch file"),
    CMDDEF(ren,"|rename","Renames files and directories"),
    CMDDEF(set,"","Show/modify environment variables"),
    CMDDEF(time,"","Shows the time"),
    CMDDEF(type,"","Reads and displays a text file"),
    CMDDEF(unboot,"","Marks a disk as non-bootable"),
    CMDDEF(ver,"","Displays the current version of PDOS"),
    CMD_REGISTRY_END
};

/* utility function - check if string is blank */
static bool isBlankString(char *str)
{
    while (*str != 0)
    {
        if (!isspace(*str))
        {
            return false;
        }
        str++;
    }
    return true;
}

/* utility function - show an error if command doesn't accept arguments */
static void showArgsUnsupportedMsg(void)
{
    printf("ERROR: Command '%s' does not accept any arguments\n", curCmdName);
}

/* utility function - show an error if command requires arguments */
static void showArgsRequiredMsg(void)
{
    printf("ERROR: Command '%s' requires arguments, but none supplied\n", 
            curCmdName);
}

/* utility macro - fail command if non-blank arguments supplied */
#define CMD_HAS_NO_ARGS(args) \
    do { \
        if (!isBlankString(args)) { \
            showArgsUnsupportedMsg(); \
            return 1; \
        } \
    }  while (0)

/* utility macro - fail command if non-blank arguments missing */
#define CMD_REQUIRES_ARGS(args) \
    do { \
        if (isBlankString(args)) { \
            showArgsRequiredMsg(); \
            return 1; \
        } \
    }  while (0)

/* Searches command registry for command with given name */
cmdBlock *findCommand(char *cmdName)
{
    char tmp[128];
    char *p1, *p2;
    int i;
    cmdBlock *block;

    /* Ignore initial whitespace */
    while (isspace(*cmdName))
    {
        cmdName++;
    }

    /* Ignore final whitespace */
    for (i = 0; cmdName[i] != 0; i++)
    {
        if (isspace(cmdName[i]))
        {
            cmdName[i] = 0;
            break;
        }
    }

    /* Search the registry for the command */
    for (i = 0; cmdRegistry[i].name != NULL; i++)
    {
        block = &(cmdRegistry[i]);
        if (strchr(block->name,'|') == NULL)
        {
            if (ins_strcmp(block->name,cmdName) == 0)
            {
                return block;
            }
        }
        else
        {
            strcpy(tmp,block->name);
            p1 = tmp;
            while (1)
            {
                p2 = strchr(p1,'|');
                if (p2 == NULL)
                {
                    if (ins_strcmp(p1,cmdName) == 0)
                    {
                        return block;
                    }
                    break;
                }
                *p2 = 0;
                p2++;
                if (ins_strcmp(p1,cmdName) == 0)
                {
                    return block;
                }
                p1 = p2;
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
#ifdef USING_EXE
    pdosRun();
#endif
#ifdef __32BIT__
    install_runtime();
#endif
    parseArgs(argc, argv);
    if (singleCommand)
    {
        processInput();
        return (0);
    }
    if (primary)
    {
        printf("welcome to pcomm\n");
        readBat("AUTOEXEC.BAT");
    }
    else
    {
        printf("welcome to pcomm - exit to return\n");
    }
    while (!term)
    {
        putPrompt();
        safegets(buf, sizeof buf);

        processInput();
    }
    printf("thankyou for using pcomm!\n");
    return (0);
}

static void parseArgs(int argc, char **argv)
{
    int x;

    if (argc > 1)
    {
        if ((argv[1][0] == '-') || (argv[1][0] == '/'))
        {
            if ((argv[1][1] == 'C') || (argv[1][1] == 'c'))
            {
                singleCommand = 1;
            }
            if ((argv[1][1] == 'P') || (argv[1][1] == 'p'))
            {
                primary = 1;
            }
        }
    }
    if (singleCommand)
    {
        strcpy(buf, "");
        for (x = 2; x < argc; x++)
        {
            strcat(buf, *(argv + x));
            strcat(buf, " ");
        }
        len = strlen(buf);
        if (len > 0)
        {
            len--;
            buf[len] = '\0';
        }
    }
    return;
}

static int processInput(void)
{
    char *p;
    cmdBlock *block;

    /* Remove newline at end of buffer */
    len = strlen(buf);
    if ((len > 0) && (buf[len - 1] == '\n'))
    {
        len--;
        buf[len] = '\0';
    }

    /* If command line is blank, do nothing */
    if (isBlankString(buf))
    {
        return 0;
    }

    /* Split command and arguments */
    p = strchr(buf, ' ');
    if (p != NULL)
    {
        *p++ = '\0';
    }
    else
    {
        p = buf + len;
    }
    len -= (size_t)(p - buf);

    /* Check for special case syntax first */
    if (buf[0] == ':')
    {
        /* Labels are ignored here */
        return 0;
    }
    else if (ins_strncmp(buf, "cd.", 3) == 0)
    {
        return cmd_cd_run(buf + 2);
    }
    else if (ins_strncmp(buf, "cd\\", 3) == 0)
    {
        return cmd_cd_run(buf + 2);
    }
    else if ((strlen(buf) == 2) && (buf[1] == ':'))
    {
        return changedisk(buf[0]);
    }
    else
    {
        /* Look for internal command in registry */
        block = findCommand(buf);
        if (block != NULL)
        {
            /* Command was found, call implementation if present */
            if (block->proc == NULL)
            {
                printf("ERROR: No implementation found for command '%s'\n",
                        buf);
                return 1;
            }
            else
            {
                /* Populate curCmdName (for use in error messages) */
                strcpy(curCmdName, buf);
                strtoupper(curCmdName);
                /* Call implementation */
                return block->proc(p);
            }
        }
        else
        {
            /* No internal command found, presume external command */
            return doExec(buf,p);
        }
    }
}

static void putPrompt(void)
{
    int d;

    d = PosGetDefaultDrive();
    drive[0] = d + 'A';
    PosGetCurDir(0, cwd);
    printf("\n%s:\\%s%s", drive, cwd, prompt);
    fflush(stdout);
    return;
}

static int cmd_exit_run(char *ignored)
{
    CMD_HAS_NO_ARGS(ignored);
#ifdef CONTINUOUS_LOOP
    primary = 0;
#endif
    if (!primary)
    {
        term = 1;
    }
    return 0;
}

static int cmd_echo_run(char *msg)
{
    printf("%s\n", msg);
    return 0;
}

static int cmd_reboot_run(char *ignored)
{
    CMD_HAS_NO_ARGS(ignored);
    PosReboot();
    /* if we return from PosReboot(), we know it failed */
    printf("ERROR: Reboot failed\n");
    return 1;
}

static int cmd_type_run(char *file)
{
    FILE *fp;
    int p = 0, l = 0, count = 1, pa = 1;
    CMD_REQUIRES_ARGS(file);

    /* Collect optional /L and /P arguments */
    file = stringTrimBoth(file);
    for (;;)
    {
        if (ins_strncmp(file,"/L ",3) == 0)
        {
            file += 3;
            l = 1;
            file = stringTrimBoth(file);
            continue;
        }
        if (ins_strncmp(file,"/P ",3) == 0)
        {
            file += 3;
            p = 1;
            file = stringTrimBoth(file);
            continue;
        }
        break;
    }

    /* Ensure we have an actual file name (not just /L /P options) */
    CMD_REQUIRES_ARGS(file);

    /* Open the file */
    fp = fopen(file, "r");

    if (fp != NULL)
    {
        while (fgets(buf, sizeof buf, fp) != NULL)
        {
            /* Print line numbers if requested by /L */
            if (l)
            {
                printf("%d ", count);
            }

            /* Print the line */
            fputs(buf, stdout);

            /* Increment line number count */
            count++;

            /* Process paging */
            pa++;
            if (pa == 24)
            {
                pa = 1;
                if (p)
                {
                    cmd_pause_run("");
                }
            }
        }
        fclose(fp);
    }

    else
    {
       printf("file not found: %s\n", file);
       return 1;
    }

    return 0;
}

static int cmd_del_run(char *fnm)
{
    int ret;
    DTA *dta;
    CMD_REQUIRES_ARGS(fnm);

    dta = PosGetDTA();
    ret = PosFindFirst(fnm,0x10);

    if(ret == 2)
    {
        printf("File not found \n");
        printf("No Files Deleted \n");
        return 1;
    }
    while(ret == 0)
    {
        PosDeleteFile(dta->file_name);
        ret = PosFindNext();
    }
    return 0;
}

static int cmd_dir_run(char *pattern)
{
    DTA *dta;
    int ret;
    char *p;
    time_t tt;
    struct tm *tms;
    int genuine_pdos;

    /* We only print LFN if genuine PDOS. So if testing under
     * some DOS that doesn't support LFNs, we don't print garbage.
     * (Actually maybe check should be if current DOS supports
     * LFNs, i.e. INT 21,71, but PDOS doesn't implement that yet.)
     */
    genuine_pdos = (PosGetMagic() == PDOS_MAGIC);
    dta = PosGetDTA();
    if (isBlankString(pattern))
    {
        p = "*.*";
    }
    else
    {
        p = pattern;
    }
    ret = PosFindFirst(p, 0x10);
    while (ret == 0)
    {
        tt = dos_to_timet(dta->file_date,dta->file_time);
        tms = localtime(&tt);
        printf("%-13s %9ld %02x %04d-%02d-%02d %02d:%02d:%02d %s\n",
               dta->file_name,
               dta->file_size,
               dta->attrib,
               tms->tm_year + 1900,
               tms->tm_mon + 1,
               tms->tm_mday,
               tms->tm_hour,
               tms->tm_min,
               tms->tm_sec,
               genuine_pdos ? dta->lfn : "");
        ret = PosFindNext();
    }
    return 0;
}

#ifdef __32BIT__
#define PDOS_FLAVOR "PDOS-32"
#else
#define PDOS_FLAVOR "PDOS-16"
#endif

/* Trim whitespace from end of string */
static char *stringTrimRight(char *s)
{
    while (strlen(s) > 0 && isspace(s[strlen(s)-1]))
    {
        s[strlen(s)-1] = 0;
    }
    return s;
}

/* Trim whitespace from start of string */
static char *stringTrimLeft(char *s)
{
    while (*s != 0 && isspace(*s))
    {
        s++;
    }
    return s;
}

/* Trim whitespace from both ends of string */
static char *stringTrimBoth(char *s)
{
    return stringTrimLeft(stringTrimRight(s));
}

static void errorBadArgs(void)
{
    printf("ERROR: Bad arguments to %s command\n", curCmdName);
}

static void errorBatchOnly(void)
{
    printf("ERROR: Command %s allowed in batch file only\n", curCmdName);
}

static void errorBadDosVersion(void)
{
    printf("ERROR: Bad version number in VER DOS=\n");
}

/* Left shift by 6 bits converts units of paragraphs (16 bytes)
 * to KB (1024 bytes). */
#define PARAS_TO_KB(paras) ((paras)>>6)

static int cmd_ver_run(char *arg)
{
    int ver, major, minor, genuine;
    unsigned int maxPages;
    MEMMGRSTATS stats;

    /* Only supported param is DOS=x.yy to set DOS version */
    if (!isBlankString(arg))
    {
        /* Trim whitespace */
        arg = stringTrimBoth(arg);

        /* Ensure we have DOS= argument (other args not supported */
        if (ins_strncmp(arg, "DOS=", 4) != 0)
        {
            errorBadArgs();
            return 1;
        }
        /* Strip the "DOS=" prefix */
        arg += 4;
        /* Expect start of major version */
        if (!isdigit(*arg))
        {
            errorBadDosVersion();
            return 1;
        }
        /* Parse major version */
        major = strtol(arg, &arg, 10);
        /* Expect dot (.) */
        if (arg[0] != '.' || !isdigit(arg[1]))
        {
            errorBadDosVersion();
            return 1;
        }
        /* Skip dot */
        arg++;
        /* Parse minor version */
        minor = strtol(arg, &arg, 10);
        /* Arg should now point to NUL. If not, there is junk after minor */
        if (arg[0] != 0)
        {
            errorBadDosVersion();
            return 1;
        }
        /* Okay, we have major and minor, construct version */
        ver = ((minor&0xFF)<<8) | (major&0xFF);
        /* Set DOS version */
        PosSetDosVersion(ver);
        /* Read back and compare to confirm it worked */
        ver = PosGetDosVersion();
        if ((major != ((ver>>8)&0xFF)) || (minor != (ver & 0xFF)))
        {
            printf("ERROR: Setting DOS version failed\n");
            return 1;
        }
        printf("SUCCESS: Set reported DOS version to %d.%d\n", major, minor);
        return 0;
    }

    ver = PosGetDosVersion();
    minor = (ver & 0xFF);
    major = ((ver>>8)&0xFF);
    printf("PCOMM for " PDOS_FLAVOR "\n");
    printf("Reporting DOS version %d.%d\n", major, minor);
    printf("PCOMM built at %s %s\n", __DATE__, __TIME__);
    printf("Booted from drive %c\n", (PosGetBootDrive() - 1 + 'A'));

    /* Test if this is genuine PDOS and not something else like MS-DOS.
     * No point calling PDOS extension APIs if not really PDOS, they will
     * return meaningless values.
     */
    if (PosGetMagic() != PDOS_MAGIC)
    {
        printf("WARNING: PDOS not detected, "
                "PCOMM running under another DOS\n");
        printf("Some features of PCOMM may not work.\n");
        return 0;
    }
    printf("Detected genuine " PDOS_FLAVOR "\n");

    /* Get and display memory allocation statistics */
    PosGetMemoryManagementStats(&stats);
    /* Weird printf bug, only first two %d work, the third becomes 0.
     * Workaround is split printf calls up */
    printf("%d KB memory free in %d blocks",
            PARAS_TO_KB(stats.totalFree), stats.countFree);
    printf(" (largest free block %d KB)\n",
            PARAS_TO_KB(stats.maxFree));
    printf("%d KB memory allocated in %d blocks",
            PARAS_TO_KB(stats.totalAllocated), stats.countAllocated);
    printf(" (largest allocated block %d KB)\n",
            PARAS_TO_KB(stats.maxAllocated));
    printf("%d KB memory total in %d blocks",
            PARAS_TO_KB(stats.totalAllocated+stats.totalFree),
            stats.countAllocated+stats.countFree);
    printf(" (largest block %d KB)\n",
            PARAS_TO_KB(stats.maxAllocated > stats.maxFree ?
                stats.maxAllocated : stats.maxFree));
    return 0;
}

static int cmd_path_run(char *s)
{
     if (isBlankString(s))
     {
        char *t;

        t = path;

        if (*t == ';')
        {
            t++;
        }

        if (*t == '\0')
        {
            printf("No Path defined\n");
        }

        else
        {
            printf("%s\n", t);
        }
     }

     else if (*s == ';')
     {
        strcpy(path, s);
     }

     else
     {
        strcpy(path, ";");
        strcat(path, s);
     }
     return 0;
}

static int cmd_prompt_run(char *s)
{
     if (isBlankString(s))
     {
        printf("Current PROMPT is %s\n", prompt);
     }

     else
     {
        strcpy(prompt, s);
     }
     return 0;
}

static int doExec(char *b,char *p)
{
    char *r;
    char *s;
    char *t;
    size_t ln;
    size_t ln2;
    char tempbuf[FILENAME_MAX];

    s = path;
    ln = strlen(p);
    cmdt[0] = ln;
    memcpy(cmdt + 1, p, ln);
    memcpy(cmdt + ln + 1, "\r", 2);

    while(*s != '\0')
    {
        t = strchr(s, ';');

        if (t == NULL)
        {
            t = s + strlen(s);
        }

        ln2 = t - s;
        memcpy(tempbuf, s ,ln2);

        if (ln2 != 0)
        {
            tempbuf[ln2++] = '\\';
        }

        strcpy(tempbuf + ln2, b);
        ln2 += strlen(b);
        strcpy(tempbuf + ln2, ".com");

        if (exists(tempbuf))
        {
            PosExec(tempbuf, &parmblock);
            break;
        }

        strcpy(tempbuf + ln2 ,".exe");

        if (exists(tempbuf))
        {
            PosExec(tempbuf, &parmblock);
            break;
        }

        strcpy(tempbuf + ln2 ,".bat");

        if (exists(tempbuf))
        {
            readBat(tempbuf);
            break;
        }

        s = t;

        if (*s == ';')
        {
            s++;
        }
    }

    if (*s == '\0')
    {
        printf("Command '%s' not found\n", b);
        return 1;
    }

    return PosGetReturnCode();
}

static int cmd_copy_run(char *b)
{
    char bbb[512];
    char *src;
    char *dest;
    FILE *fsrc;
    FILE *fdest;
    int bytes_read = 0;
    int rv = 0;
    unsigned long total_read = 0, verify_read = 0;
    CMD_REQUIRES_ARGS(b);

    src = b;
    dest = strchr(b,' ');

    if (dest == NULL)
    {
        printf("\n Two Filenames required");
        return 1;
    }

    *dest++ ='\0';

    printf("Source is %s \n",src);
    printf("Dest is %s \n",dest);

    fsrc = fopen(src,"rb");

    if (fsrc != NULL)
    {
        fdest = fopen(dest,"wb");

        if (fdest != NULL)
        {
            while((bytes_read = fread(bbb,1,sizeof(bbb),fsrc)) != 0)
            {
                total_read += bytes_read;
                fwrite(bbb,1,bytes_read,fdest);
            }

            fclose(fdest);
        }

        else
        {
            printf("Destination %s file not found \n",dest);
            rv = 1;
        }

        fclose(fsrc);
        printf("Copied %ld bytes\n", total_read);
        /* Verify copied file */
        if (PosGetVerifyFlag())
        {
            printf("Verifying copied file...\n");
            fdest = fopen(dest, "rb");
            if (dest == NULL)
            {
                printf(
                  "ERROR: failed opening destination file for verification\n");
                return 1;
            }
            while((bytes_read = fread(bbb,1,sizeof(bbb),fdest)) != 0)
            {
                verify_read += bytes_read;
            }
            fclose(dest);
            if (verify_read == total_read)
            {
                printf("SUCCESS: Read %ld bytes from destination file\n",
                        verify_read);
            }
            else
            {
                printf(
                     "ERROR: Read %ld bytes from destination (expected %ld)\n",
                        verify_read, total_read);
                return 1;
            }
        }
    }

    else
    {
        printf("Source file %s not found \n",src);
        rv = 1;
    }

    return rv;
}

static int cmd_cd_run(char *to)
{
    int ret;
    int d;

    /* No arguments means to print current drive and directory */
    if (isBlankString(to))
    {
        d = PosGetDefaultDrive();
        drive[0] = d + 'A';
        PosGetCurDir(0, cwd);
        printf("%s:\\%s\n", drive, cwd);
        return 0;
    }

    ret = PosChangeDir(to);

    if (ret == POS_ERR_PATH_NOT_FOUND)
    {
        printf("Invalid directory\n");
        return 1;
    }

    return 0;
}

static int changedisk(int drive)
{
    int selected;
    PosSelectDisk(toupper(drive) - 'A');
    selected = PosGetDefaultDrive() + 'A';
    if (selected != toupper(drive))
    {
        printf("ERROR: Failed changing to drive %c:\n", toupper(drive));
        return 1;
    }
    return 0;
}

/*
 * **** HELP ROUTINES ***
 */
static void cmd_exit_help(void)
{
    printf("EXIT\n");
}

static void cmd_path_help(void)
{
    printf("PATH variable provides a list of directories\n");
    printf("which are searched when looking for executables.\n\n");
    printf("PATH\n");
    printf("PATH [path]\n");
    printf("PATH [;]\n");
}

static void cmd_prompt_help(void)
{
    printf("PROMPT variable stores the PCOMM prompt.\n");
    printf("PROMPT\n");
    printf("PROMPT [prompt]\n");
}

static void cmd_type_help(void)
{
    printf("TYPE [/L] [/P] [path]\n");
    printf("/L = print line numbers\n");
    printf("/P = pause after each screenful\n");
}

static void cmd_dir_help(void)
{
    printf("DIR\n");
}

static void cmd_ver_help(void)
{
    printf("VER\n");
    printf("VER DOS=x.yy\n\n");
    printf("Can be used to display PDOS/PCOMM version.\n");
    printf("Can also be used to change reported DOS version.\n");
    printf("For example:\n");
    printf("\tVER DOS=6.22\n");
    printf("Changes reported DOS version to 6.22\n");
}

static void cmd_echo_help(void)
{
    printf("ECHO [message]\n");
}

static void cmd_cd_help(void)
{
    printf("CD [path]\n");
}

static void cmd_reboot_help(void)
{
    printf("REBOOT\n");
}

static void cmd_del_help(void)
{
    printf("DEL [path]\n");
}

static void cmd_copy_help(void)
{
    printf("COPY [path1] [path2]\n");
}

static void cmd_help_help(void)
{
    printf("HELP\n");
    printf("HELP [command]\n");
}

static void cmd_md_help(void)
{
    printf("MD [path]\n");
}

static void cmd_mcd_help(void)
{
    printf("MCD [path]\n");
}

static void cmd_rd_help(void)
{
    printf("RD [path]\n");
}

static void cmd_ren_help(void)
{
    printf("REN [path] [name]\n");
}

static void cmd_date_help(void)
{
    printf("DATE\n\n");
    printf("Limitations (may be removed in future versions):\n");
    printf("- No arguments supported at present\n");
    printf("- No facility to change the date\n");
}

static void cmd_time_help(void)
{
    printf("TIME\n\n");
    printf("Limitations (may be removed in future versions):\n");
    printf("- No arguments supported at present\n");
    printf("- No facility to change the time\n");
}

static void cmd_rem_help(void)
{
    printf("REM [any arguments ignored]\n");
}

static void cmd_pause_help(void)
{
    printf("PAUSE\n");
}

static void cmd_set_help(void)
{
    printf("SET\n\n");
    printf("Limitations (may be removed in future versions):\n");
    printf("- Setting environment variables not presently supported\n");
}

static void cmd_option_help(void)
{
    printf("OPTION {{+-}X}*\n\n");
    printf("where X is an option character\n");
    printf("+X sets option X, -X clears option X\n");
    printf("Call with no arguments for current state of options\n");
}

static void cmd_unboot_help(void)
{
    printf("UNBOOT X:\n\n");
    printf("Marks disk X: as non-bootable\n");
    printf("It does this by rewriting the boot sector to\n");
    printf("remove the 55AA boot signature.\n");
}

static void cmd_goto_help(void)
{
    printf("GOTO [/16 | /32] label\n\n");
    printf("Transfer batch file execution to :label\n");
    printf("Option /16 means only take branch if running PDOS-16\n");
    printf("Option /32 means only take branch if running PDOS-32\n");
}

/* Modify string in-place to be all upper case */
static void strtoupper(char *s)
{
    while (*s != 0)
    {
        *s = toupper(*s);
        s++;
    }
}

/* Assumption as to number of lines before we print
 * "Press any key to continue...". Actually we should call some API to find
 * out screen dimensions and calculate dynamically. But for now, we will just
 * assume a standard 25 line display. The paging offset then is a bit less
 * than 25 to take into account the 3 line header, etc.
 */
#define HELP_PAGE_AMOUNT 19

/* cmd_help_run function and condition for it originally written by Alica
 * Okano. Rewritten to use table-driven design by Simon Kissane.
 */
static int cmd_help_run(char *cmd)
{
    int i = 0;
    int pageAt = HELP_PAGE_AMOUNT;
    int maxCmdNameLen = 0, curCmdNameLen = 0;
    cmdBlock *block = NULL;
    char *pipe = NULL;
    char tmp[128];
    if (isBlankString(cmd))
    {
        printf("Use HELP [command] to obtain more information about\n");
        printf("specific command.\n\n");

        /* Find max command name length. Used to construct columns. */
        for (i = 0; cmdRegistry[i].name != NULL; i++)
        {
            block = &(cmdRegistry[i]);
            curCmdNameLen = strlen(block->name);
            if (curCmdNameLen > maxCmdNameLen)
                maxCmdNameLen = curCmdNameLen;
        }

        /* Print each line */
        for (i = 0; cmdRegistry[i].name != NULL; i++)
        {
            block = &(cmdRegistry[i]);

            /* Allocate temporary buffer to modify name */
            memset(tmp, 0, 128);
            strcpy(tmp,block->name);

            /* Strip aliases */
            pipe = strchr(tmp,'|');
            if (pipe != NULL)
            {
                while (*pipe != 0)
                {
                    *pipe = 0;
                    pipe++;
                }
            }

            /* Make upper case */
            strtoupper(tmp);

            /* Left pad with spaces */
            while (strlen(tmp) < maxCmdNameLen)
                tmp[strlen(tmp)] = ' ';
            tmp[strlen(tmp)] = ' ';

            /* Print help line */
            printf("%s %s.\n", tmp, block->description);

            /* Page the screen if necessary */
            if (i >= pageAt && cmdRegistry[i+1].name != NULL)
            {
                cmd_pause_run("");
                pageAt += HELP_PAGE_AMOUNT;
            }
        }
    }
    else
    {
        /* Lookup command, report error if not found */
        block = findCommand(cmd);
        if (block == NULL)
        {
            printf("ERROR: No such command \"%s\"\n",cmd);
            return 1;
        }

        /* Allocate temporary buffer to modify name */
        memset(tmp, 0, 128);
        strcpy(tmp,block->name);

        /* Strip aliases */
        pipe = strchr(tmp,'|');
        if (pipe != NULL)
        {
            while (*pipe != 0)
            {
                *pipe = 0;
                pipe++;
            }
        }

        /* Make upper case */
        strtoupper(tmp);

        /* Print header line */
        printf("%s - %s\n\n", tmp,
                (block->description != NULL ? block->description :
                 "(no description)"));

        /* Print aliases */
        if (strchr(block->name,'|') != NULL)
        {
            memset(tmp, 0, 128);
            strcpy(tmp,block->name);
            pipe = strchr(tmp,'|');
            pipe++;
            for (i = 0; pipe[i] != 0; i++)
            {
                pipe[i] = toupper(pipe[i]);
                if (pipe[i] == '|')
                {
                    pipe[i] = ' ';
                }
            }
            printf("Aliases: %s\n\n", pipe);
        }

        /* Print detailed help */
        if (block->help != NULL)
        {
            (block->help)();
        }
    }

    return 0;
}

static int cmd_md_run(char *dnm)
{
    int ret;
    CMD_REQUIRES_ARGS(dnm);

    ret = PosMakeDir(dnm);

    if (ret == POS_ERR_PATH_NOT_FOUND)
    {
        printf("Unable to create directory\n");
        return 1;
    }
    if (ret == POS_ERR_ACCESS_DENIED)
    {
        printf("Access denied\n");
        return 1;
    }

    return 0;
}

static int cmd_mcd_run(char *dnm)
{
    int rc = 0;
    CMD_REQUIRES_ARGS(dnm);
    rc = cmd_md_run(dnm);
    if (rc != 0)
    {
        rc = cmd_cd_run(dnm);
    }
    return rc;
}

static int cmd_rd_run(char *dnm)
{
    int ret;
    CMD_REQUIRES_ARGS(dnm);

    ret = PosRemoveDir(dnm);

    if (ret == POS_ERR_PATH_NOT_FOUND)
    {
        printf("Invalid path, not directory,\nor directory not empty\n");
        return 1;
    }
    else if (ret == POS_ERR_ACCESS_DENIED)
    {
        printf("Access denied\n");
        return 1;
    }
    else if (ret == POS_ERR_INVALID_HANDLE)
    {
        printf("Invalid handle\n");
        return 1;
    }
    else if (ret == POS_ERR_ATTEMPTED_TO_REMOVE_CURRENT_DIRECTORY)
    {
        printf("Attempt to remove current directory\n");
        return 1;
    }

    return 0;
}

static int cmd_ren_run(char *src)
{
    int ret;
    char *dest;
    CMD_REQUIRES_ARGS(src);

    dest = strchr(src, ' ');
    if (*src == '\0' || dest == NULL)
    {
        printf("Required Parameter Missing\n");
        return 1;
    }

    *dest = '\0';
    dest++;

    ret = PosRenameFile(src, dest);

    if (ret == POS_ERR_FILE_NOT_FOUND)
    {
        printf("File not found\n");
        return 1;
    }
    else if (ret == POS_ERR_PATH_NOT_FOUND)
    {
        printf("Invalid path\n");
        return 1;
    }
    else if (ret == POS_ERR_ACCESS_DENIED)
    {
        printf("Access denied\n");
        return 1;
    }
    else if (ret == POS_ERR_FORMAT_INVALID)
    {
        printf("Invalid format\n");
        return 1;
    }

    return 0;
}

static int ins_strcmp(char *one, char *two)
{
    while (toupper(*one) == toupper(*two))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
    }
    if (toupper(*one) < toupper(*two))
    {
        return (-1);
    }
    return (1);
}

static int ins_strncmp(char *one, char *two, size_t len)
{
    size_t x = 0;

    if (len == 0) return (0);
    while ((x < len) && (toupper(*one) == toupper(*two)))
    {
        if (*one == '\0')
        {
            return (0);
        }
        one++;
        two++;
        x++;
    }
    if (x == len) return (0);
    return (toupper(*one) - toupper(*two));
}

static int readBat(char *fnm)
{
    FILE *fp;
    int rc;

    inBatch = 1;

    fp = fopen(fnm, "r");
    if (fp != NULL)
    {
        while (fgets(buf, sizeof buf, fp) != NULL)
        {
            /* If GOTO target set, scan for matching label */
            if (*gotoTarget != 0)
            {
                if (ins_strncmp(buf, gotoTarget, strlen(gotoTarget)) == 0)
                {
                    /* Found gotoTarget, clear it */
                    gotoTarget[0] = 0;
                }
                /* We don't echo or execute while doing GOTO scan */
                continue;

            }
            /* If OPTION +E print the command before running it */
            if (echoFlag)
            {
                putPrompt();
                printf("%s\n", buf);
            }
            rc = processInput();
            /* Abort if OPTION +A and batch file command failed */
            if (abortFlag && rc != 0)
            {
                printf("ERROR: Aborting batch file due to command failure\n");
                rc = 1;
                break;
            }
            /* If executeGoto=1, reopen file to commence GOTO execution.
             * Basically, on every GOTO, we rewind file to start
             */
            if (executeGoto)
            {
                fclose(fp);
                fp = fopen(fnm, "r");
                executeGoto = 0;
            }
        }
        fclose(fp);
        if (*gotoTarget != 0)
        {
            printf("ERROR: GOTO target '%s' not found\n", gotoTarget);
            gotoTarget[0] = 0;
            rc = 1;
        }
    }
    else
    {
        rc = 1;
    }

    inBatch = 0;
    return rc;
}

static int exists(char *fnm)
{
    FILE *fp;

    fp = fopen(fnm,"rb");

    if (fp != NULL)
    {
        fclose(fp);
        return 1;
    }

    return 0;
}

/* Written By NECDET COKYAZICI, Public Domain */


void putch(int a)
{
    putchar(a);
    fflush(stdout);
}


void bell(void)
{
    putch('\a');
}


void safegets(char *buffer, int size)
{
    int a;
    int shift;
    int i = 0;

    while (1)
    {

        a = PosGetCharInputNoEcho();

/*
        shift = BosGetShiftStatus();

        if (shift == 1)
        {
            if ((a != '\n') && (a != '\r') && (a != '\b'))
            if (islower(a))
            a -= 32;

            else if ((a >= '0') && (a <= '9'))
            a -= 16;
        }

*/

        if (i == size)
        {
            buffer[size] = '\0';

            if ((a == '\n') || (a == '\r'))
            return;

            bell();
        }

        if ((i == 0) && (a == '\b'))
        continue;

        if (i < size)
        {

            if ((a == '\n') || (a == '\r'))
            {
                putch('\n');

                buffer[i] = '\0';

                return;
            }


            if (a == '\b')
            {

                if (i == 0)
                continue;

                else
                i--;

                putch('\b');
                putch(' ');
                putch('\b');

                continue;
            }
            else
            putch(a);


            if (isprint((unsigned char)a))
            {
                buffer[i] = a;
                i++;
            }
            else bell();

        }
        else bell();

    }

}

static int cmd_date_run(char *ignored)
{
    int y, m, d, dw;
    CMD_HAS_NO_ARGS(ignored);
    PosGetSystemDate(&y,&m,&d,&dw);
    printf("%04d-%02d-%02d\n", y, m, d);
    return 0;
}

static int cmd_time_run(char *ignored)
{
    int hr, min, sec, hund;
    CMD_HAS_NO_ARGS(ignored);
    PosGetSystemTime(&hr,&min,&sec,&hund);
    printf("%02d:%02d:%02d\n", hr, min, sec);
    return 0;
}

static int cmd_rem_run(char *ignored)
{
    /* Nothing to do for comment */
    return 0;
}

static int cmd_pause_run(char *ignored)
{
    CMD_HAS_NO_ARGS(ignored);
    printf("Press any key to continue . . .\n");
    PosDirectCharInputNoEcho();
    return 0;
}

static int cmd_set_run(char *ignored)
{
    char *env = (char*)__envptr;
    size_t len;
    if (env == NULL)
    {
        return 0;
    }
    for (;;)
    {
        len = strlen(env);
        if (len == 0)
        {
            return 0;
        }
        printf("%s\n", env);
        env += (len + 1);
    }
}

/* Find option in options registry */
static optBlock *findOption(char optKey)
{
    optBlock *cur;
    int i;
    for (i = 0; optRegistry[i].name != NULL; i++)
    {
        cur = &(optRegistry[i]);
        if (cur->key == optKey)
        {
            return cur;
        }
    }
    return NULL;
}

static int opt_logunimp_get(void)
{
    /* If not PDOS, no such flag, just always return 0=off */
    if (PosGetMagic() != PDOS_MAGIC)
    {
        return 0;
    }
    return PosGetLogUnimplemented();
}

static void opt_logunimp_set(int flag)
{
    /* Only try set flag if PDOS, API doesn't exist otherwise */
    if (PosGetMagic() == PDOS_MAGIC)
    {
        PosSetLogUnimplemented(!!flag);
    }
}

static int opt_verify_get(void)
{
    return PosGetVerifyFlag();
}

static void opt_verify_set(int flag)
{
    PosSetVerifyFlag(!!flag);
}

static int opt_break_get(void)
{
    return PosGetBreakFlag();
}

static void opt_break_set(int flag)
{
    PosSetBreakFlag(!!flag);
}

static int opt_echo_get(void)
{
    return echoFlag;
}

static void opt_echo_set(int flag)
{
    echoFlag = !!flag;
}

static int opt_abort_get(void)
{
    return abortFlag;
}

static void opt_abort_set(int flag)
{
    abortFlag = !!flag;
}

static int cmd_option_run(char *arg)
{
    optBlock *cur;
    int i, set;
    char key;
    int flag;

    /* No args -> display current options */
    if (isBlankString(arg))
    {
        for (i = 0; optRegistry[i].name != NULL; i++)
        {
            cur = &(optRegistry[i]);
            flag = (cur->getProc)();
            printf("%c%c %s\n", flag ? '+' : '-', cur->key, cur->description);
        }
        return 0;
    }

    /* Process args */
    for (i = 0; arg[i] != 0; i++)
    {
        /* Skip whitespace */
        if (isspace(arg[i]))
        {
            continue;
        }

        /* Expect + or - */
        if (arg[i] != '+' && arg[i] != '-')
        {
            printf("ERROR: Bad syntax in OPTION command (expected + or -)\n");
            return 1;
        }

        /* Note which one we got */
        set = (arg[i] == '+');

        /* Next character must be option key */
        i++;
        if (arg[i] == 0 || !isalpha(arg[i]))
        {
            printf("ERROR: Bad syntax in OPTION command "
                    "(expected option character)\n");
            return 1;
        }

        /* Find option with that key */
        key = toupper(arg[i]);
        cur = findOption(key);
        if (cur == NULL)
        {
            printf("ERROR: Unknown option %c%c\n", (set ? '+' : '-'), key);
            return 1;
        }

        /* Perform the set */
        cur->setProc(set);

        /* Confirm new value set */
        if (cur->getProc() != set)
        {
            printf("ERROR: Option %c%c failed to take effect\n",
                    (set ? '+' : '-'), key);
            return 1;
        }

        /* Success message */
        printf("Set option %c%c (%s)\n", (set ? '+' : '-'), key,
                cur->description);
    }
    return 0;
}

static void bootSectorAccessError(int drive, int rc, int writing)
{
    printf("ERROR: Error %d %s boot sector of drive %c:\n",
            rc, writing ? "writing" : "reading", (drive + 'A'));
}

static int cmd_unboot_run(char *arg)
{
    int drive, rc, i;
    unsigned char buf[512];

    CMD_REQUIRES_ARGS(arg);
    arg = stringTrimBoth(arg);

    /* Validate parameters - X: is drive to make unbootable */
    if (!isalpha(arg[0]) || arg[1] != ':' || arg[2] != 0)
    {
            errorBadArgs();
            return 1;
    }
    drive = toupper(arg[0]) - 'A';

    /* Read the boot sector */
    rc = PosAbsoluteDiskRead(drive, 0, 1, buf);
    if (rc != 0)
    {
        bootSectorAccessError(drive, rc, 0);
        return 1;
    }

    /* If disk is already non-bootable, nothing to do */
    if (buf[510] != 0x55 || buf[511] != 0xAA)
    {
        printf("\n");
        printf("UNBOOT: Disk %c: already not bootable, nothing to do\n",
                (drive + 'A'));
        return 0;
    }

    /* Modify boot sector to make it unbootable */
    buf[510] = 0x12;
    buf[511] = 0x34;

    /* For unfathomable reasons, VirtualBox will try to boot a floppy in
     * drive A even if it lacks the 55AA boot signature. For it to consider
     * the disk unbootable, the first and second 16-bit words have to be
     * identical. (In November 2017, it was changed to the first and third
     * words instead.) So, to convince VirtualBox that the disk can't be
     * booted, we will overwrite the first ten bytes of a floppy boot sector
     * with 0xE9. MS-DOS will still consider the disk valid (first byte is E9),
     * while VirtualBox will consider the floppy non-bootable. Note that since
     * this corrupts the OEM name, DOS may ignore the BPB. On a floppy with
     * standard geometry, that doesn't really matter. (We won't do this on
     * hard disks, where it matters more.)
     */
    if (drive < 2)
    {
        for (i = 0; i < 0xB; i++)
        {
            buf[i] = 0xE9;
        }
    }

    /* Write the modified boot sector */
    rc = PosAbsoluteDiskWrite(drive, 0, 1, buf);
    if (rc != 0)
    {
        bootSectorAccessError(drive, rc, 1);
        return 1;
    }

    /* To confirm it was written correctly, read it back in */
    memset(buf, 0, 512);
    rc = PosAbsoluteDiskRead(drive, 0, 1, buf);
    if (rc != 0)
    {
        bootSectorAccessError(drive, rc, 0);
        return 1;
    }
    if (buf[510] != 0x12 || buf[511] != 0x34)
    {
        printf("ERROR: UNBOOT boot sector verification failed.\n");
        return 1;
    }

    /* Print SUCCESS message then done. */
    printf("SUCCESS: Disk in drive %c: made unbootable.\n", (drive + 'A'));
    return 0;
}

static int cmd_goto_run(char *arg)
{
    int is16 = 0, is32 = 0;

    CMD_REQUIRES_ARGS(arg);
    if (!inBatch)
    {
        errorBatchOnly();
        return 1;
    }
    arg = stringTrimBoth(arg);

    if (ins_strncmp(arg,"/16 ", 4)==0)
    {
        is16 = 1;
        arg += 4;
    }
    else if (ins_strncmp(arg,"/32 ", 4)==0)
    {
        is32 = 1;
        arg += 4;
    }
    arg = stringTrimBoth(arg);
    if (isBlankString(arg))
    {
        errorBadArgs();
        return 1;
    }

    /* Respect /16 and /32 args */
#ifdef __32BIT__
    if (is16)
    {
        return 0;
    }
#else
    if (is32)
    {
        return 0;
    }
#endif

    /* Actually perform the GOTO */
    gotoTarget[0] = ':';
    strcpy(gotoTarget + 1, arg);
    executeGoto = 1;
    return 0;
}
