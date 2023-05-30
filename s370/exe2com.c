/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  exe2com - convert a .exe file into a .com file                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

static FILE *fp;
static FILE *fq;

static unsigned char buf[10000];

static void processPE(void);

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        printf("usage: exe2com <exe file> <com file>\n");
        return (EXIT_FAILURE);
    }
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("failed to open %s for reading\n", argv[1]);
        return (EXIT_FAILURE);
    }
    fq = fopen(argv[2], "wb");
    if (fq == NULL)
    {
        printf("failed to open %s for writing\n", argv[2]);
        return (EXIT_FAILURE);
    }
    fread(buf, 1, 8, fp);
    if (memcmp(buf, "\x00\x38\x00\x00\x00\xCA\x6D\x0F", 8) != 0)
    {
        printf("exe files (MVS PE format) begin with\n");
        printf("00380000 00CA6D0F\n");
        printf("as the result of a load module being run through\n");
        printf("IEBCOPY unload followed by COPYFILE from\n");
        printf("RECFM=V to RECFM=U to get explicit RDWs\n");
        return (EXIT_FAILURE);
    }
    processPE();
    fclose(fp);
    fclose(fq);
    return (0);
}


/* Structures here are documented in Appendix B and E of
   MVS Program Management: Advanced Facilities SA22-7644-14:
   http://publibfp.dhe.ibm.com/cgi-bin/bookmgr/BOOKS/iea2b2b1/CCONTENTS
   and the IEBCOPY component is documented here:
   Appendix B of OS/390 DFSMSdfp Utilities SC26-7343-00
   or Appendix B of the z/OS equivalent SC26-7414-05 available here at
   http://publibz.boulder.ibm.com/epubs/pdf/dgt2u140.pdf
*/

#define PE_DEBUG 0

static void processPE(void)
{
    char *p;
    unsigned char *q;
    int z;
    typedef struct {
        unsigned char pds2name[8];
        unsigned char unused1[19];
        unsigned char pds2epa[3];
        unsigned char pds2ftb1;
        unsigned char pds2ftb2;
        unsigned char pds2ftb3;
    } IHAPDS;
    IHAPDS *ihapds;
    int rmode;
    int amode;
    int ent;
    int rec = 1;
    int corrupt = 1;
    int l;
    int l2;
    int lastt = -1;
    char *lasttxt = NULL;
    char *upto = buf;

    fread(buf, 1, 0x38 - 8, fp); /* discard remainder of first record */
    while (1)
    {
        rec++;
        fread(buf, 1, 4, fp);
        if (feof(fp))
        {
            printf("seems to have worked\n");
            exit(EXIT_SUCCESS);
        }
        l = (buf[0] << 8) | buf[1];
#if PE_DEBUG
        printf("rec %d, offset %ld is len %d\n", rec, ftell(fp), l);
#endif
        l -= 4;
        fread(buf, 1, l, fp);
        if (rec == 3) /* directory record */
        {
            /* there should only be one directory entry, 
               which is 276 + 12 */
            if (l < 288)
            {
                printf("corrupt directory record\n");
                exit(EXIT_FAILURE);
            }
            q = buf + 20;
            l2 = *(short *)q;
            if (l2 < 32) break;
            ihapds = (IHAPDS *)(q + 2);
            rmode = ihapds->pds2ftb2 & 0x10;
            amode = (ihapds->pds2ftb2 & 0x0c) >> 2;
            ent = 0;
            memcpy((char *)&ent + sizeof(ent) - 3, ihapds->pds2epa, 3);
            if (ent != 0)
            {
                printf("entry point must be 0 in order to convert to COM\n");
                exit(EXIT_FAILURE);
            }
#if PE_DEBUG
            /* printf("module name is %.8s\n", ihapds->pds2name); */
            printf("module name starts with %x\n", ihapds->pds2name[0]);
            printf("rmode is %s\n", rmode ? "ANY" : "24");
            printf("amode is ");
            if (amode == 0)
            {
                printf("24");
            }
            else if (amode == 2)
            {
                printf("31");
            }
            else if (amode == 1)
            {
                printf("64");
            }
            else if (amode == 3)
            {
                printf("ANY");
            }
            printf("\n");
#endif            
        }
        else if (rec > 3)
        {
            int t;
            int r2;
            int l2;
            int term = 0;
            
            if (l < 12)
            {
                printf("short record found, offset is %ld\n", ftell(fp));
                exit(EXIT_FAILURE);
            }
            q = buf + 10;
            r2 = l - 10;
            while (1)
            {
                l2 = (q[0] << 8) | q[1];
                r2 -= sizeof(short);
                if (l2 > r2)
                {
                    printf("mismatched record size at %ld\n", ftell(fp));
                    exit(EXIT_FAILURE);
                }
                r2 -= l2;

                if (l2 == 0) break;
                q += sizeof(short);
#if PE_DEBUG
                printf("load module record is of type %2x (len %5d)"
                       " offset %d\n", 
                       *q, l2, q - p);
#endif

                t = *q;
#if PE_DEBUG
                printf("lastt is %x, this t is %x\n", lastt, t);
#endif
                if ((lastt == 1) || (lastt == 3) || (lastt == 0x0d))
                {
#if PE_DEBUG
                    printf("rectype: program text\n");
#endif
                    lasttxt = q;
                    fwrite(q, 1, l2, fq);
/*                    memmove(upto, q, l2);
                    upto += l2; */
                    t = -1;
                    /* not sure about this. would need to check the manual */
                    /* I'll assume that we allow one bit of program text */
                    /* after the d record */
                    if (lastt == 0x0d)
                    {
                        printf("seem to have finished now\n");
                        exit(EXIT_SUCCESS);
#if 0
                        printf("corrupt program text at %ld\n", ftell(fp));
                        exit(EXIT_FAILURE);
#endif
                    }
                }
                else if (t == 0x20)
                {
                    /* printf("rectype: CESD\n"); */
                }
                else if (t == 1)
                {
                    /* printf("rectype: Control\n"); */
                }
                else if (t == 0x0d)
                {
                    /* printf("rectype: Control, about to end\n"); */
                }
                else if (t == 2)
                {
                    printf("this executable has relocatables\n");
                    printf("not allowed for a COM file\n");
                    exit(EXIT_FAILURE);
                }
#if 0
                    /* printf("rectype: RLD\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
#endif
                else if (t == 3)
                {
                    int l3;
                    
                    printf("got 3\n");
                    exit(EXIT_FAILURE);
                    /* printf("rectype: Dicionary = Control + RLD\n"); */
                    l3 = *(short *)(q + 6) + 16;
#if 0
                    printf("l3 is %d\n", l3);
#endif
#if 0
                    if (processRLD(buf, rlad, q, l3) != 0)
                    {
                        term = 1;
                        break;
                    }
#endif
                }
                else if (t == 0x0e)
                {
                    printf("got e\n");
                    exit(EXIT_FAILURE);
#if 0
                    /* printf("rectype: Last record of module\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                    term = 1;
                    corrupt = 0;
                    break;
#endif
                }
                else if (t == 0x80)
                {
                    /* printf("rectype: CSECT\n"); */
                }
                else
                {
                    /* printf("rectype: unknown %x\n", t); */
                }
#if 0
                if ((t == 0x20) || (t == 2))
                {
                    for (z = 0; z < l; z++)
                    {
                        printf("z %d %x %c\n", z, q[z], 
                               isprint(q[z]) ? q[z] : ' ');
                    }
                }
#endif
                lastt = t;

                q += l2;
                if (r2 == 0)
                {
#if PE_DEBUG
                    printf("another clean exit\n");
#endif
                    break;
                }
                else if (r2 < (10 + sizeof(short)))
                {
                    /* printf("another unclean exit\n"); */
                    term = 1;
                    break;
                }
                r2 -= 10;
                q += 10;
            }
            if (term) break;            
        }        
    }
    return;
}
