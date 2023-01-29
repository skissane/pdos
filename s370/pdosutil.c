/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  pdosutil - utilities used by PDOS and possibly PLOAD             */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "pdosutil.h"

#define MAXBLKSZ 32767

int int_rdblock(int dev, int cyl, int head, int rec,
                void *buf, int len, int cmd);

#define rdblock(dev, cyl, head, rec, buf, len, cmd) \
    int_rdblock(dev, cyl, head, rec, buf, len, cmd)


/* find a file on disk */
/* 0 = success, else negative return code */

int findFile(int ipldev, char *dsn, int *c, int *h, int *r)
{
    char *raw;
    char *initial;
    char *load;
    /* Standard C programs can start at a predictable offset */
    int (*entry)(void *);
    int cyl;
    int head;
    int rec;
    int i;
    int j;
    char tbuf[MAXBLKSZ];
    char srchdsn[FILENAME_MAX+10]; /* give an extra space */
    int cnt = -1;
    int lastcnt = 0;
    int ret = 0;
    struct {
        char ds1dsnam[44];
        char ds1fmtid;
        char unused1[60];
        char unused2;
        char unused3;
        char startcchh[4];
        char endcchh[4];
    } dscb1;
    int len;
    int errcnt = 0;

    if (memchr(dsn, '\0', FILENAME_MAX) == NULL)
    {
        len = FILENAME_MAX;
    }
    else
    {
        len = strlen(dsn);
    }
    memcpy(srchdsn, dsn, len);
    strcpy(srchdsn + len, " ");
    len++; /* force a search for the blank */
    
    /* read VOL1 record which starts on cylinder 0, head 0, record 3 */
    cnt = rdblock(ipldev, 0, 0, 3, tbuf, MAXBLKSZ, 0x0e);
    if (cnt >= 20)
    {
        cyl = head = rec = 0;
        /* +++ probably time to create some macros for this */
        memcpy((char *)&cyl + sizeof(int) - 2, tbuf + 15, 2);
        memcpy((char *)&head + sizeof(int) - 2, tbuf + 17, 2);
        memcpy((char *)&rec + sizeof(int) - 1, tbuf + 19, 1);
        
        while (errcnt < 4)
        {
            cnt = rdblock(ipldev, cyl, head, rec, &dscb1, sizeof dscb1, 0x0e);
            if (cnt < 0)
            {
                errcnt++;
                if (errcnt == 1)
                {
                    rec++;
                }
                else if (errcnt == 2)
                {
                    rec = 1;
                    head++;
                }
                else if (errcnt == 3)
                {
                    rec = 1;
                    head = 0;
                    cyl++;
                }
                continue;
            }
            errcnt = 0;
            if (cnt >= sizeof dscb1)
            {
                if (dscb1.ds1fmtid == '1')
                {
                    dscb1.ds1fmtid = ' '; /* for easy comparison */
                    if (memcmp(dscb1.ds1dsnam,
                               srchdsn,
                               len) == 0)
                    {
                        cyl = head = 0;
                        rec = 1;
                        /* +++ more macros needed here */
                        memcpy((char *)&cyl + sizeof(int) - 2, 
                               dscb1.startcchh, 2);
                        memcpy((char *)&head + sizeof(int) - 2,
                               dscb1.startcchh + 2, 2);
                        break;
                    }
                }
                else if (dscb1.ds1dsnam[0] == '\0')
                {
                    cnt = -1;
                    break;
                }
            }
            rec++;
        }        
    }
    
    if (cnt <= 0)
    {
        /* not found */
        return (-1);
    }
    *c = cyl;
    *h = head;
    *r = rec;
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

int fixPE(char *buf, int *len, int *entry, int rlad)
{
    char *p;
    char *q;
    int z;
    typedef struct {
        char pds2name[8];
        char unused1[19];
        char pds2epa[3];
        char pds2ftb1;
        char pds2ftb2;
        char pds2ftb3;
    } IHAPDS;
    IHAPDS *ihapds;
    int rmode;
    int amode;
    int ent;
    int rec = 0;
    int corrupt = 1;
    int rem = *len;
    int l;
    int l2;
    int lastt = -1;
    char *lasttxt = NULL;
    char *upto = buf;
    int initlen = *len;
    
    if ((*len <= 8) || (*((int *)buf + 1) != 0xca6d0f))
    {
        printf("Not an MVS PE executable\n");
        return (-1);
    }
#if PE_DEBUG
    printf("MVS PE total length is %d\n", *len);
#endif
    p = buf;
    while (1)
    {
        rec++;
        l = *(short *)p;
        /* keep track of remaining bytes, and ensure they really exist */
        if (l > rem)
        {
            break;
        }
        rem -= l;
#if PE_DEBUG
        printf("rec %d, offset %d is len %d\n", rec, p - buf, l);
#endif
#if 0
        if (1)
        {
            for (z = 0; z < l; z++)
            {
                printf("z %d %x %c\n", z, p[z], isprint(p[z]) ? p[z] : ' ');
            }
        }
#endif
        if (rec == 3) /* directory record */
        {
            /* there should only be one directory entry, 
               which is 4 + 276 + 12 */
            if (l < 292)
            {
                break;
            }
            q = p + 24;
            l2 = *(short *)q;
            if (l2 < 32) break;
            ihapds = (IHAPDS *)(q + 2);
            rmode = ihapds->pds2ftb2 & 0x10;
            amode = (ihapds->pds2ftb2 & 0x0c) >> 2;
            ent = 0;
            memcpy((char *)&ent + sizeof(ent) - 3, ihapds->pds2epa, 3);
            *entry = (int)(buf + ent);
#if PE_DEBUG
            printf("module name is %.8s\n", ihapds->pds2name);
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
            printf("entry point is %x\n", ent);
#endif            
        }
        else if (rec > 3)
        {
            int t;
            int r2;
            int l2;
            int term = 0;
            
            if (l < (4 + 12))
            {
                break;
            }
            q = p + 4 + 10;
            r2 = l - 4 - 10;
            while (1)
            {
                l2 = *(short *)q;
                r2 -= sizeof(short);
                if (l2 > r2)
                {
                    term = 1;
                    break;
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
                if ((lastt == 1) || (lastt == 3) || (lastt == 0x0d))
                {
#if PE_DEBUG
                    printf("rectype: program text\n");
#endif
                    lasttxt = q;
                    memmove(upto, q, l2);
                    upto += l2;
                    t = -1;
                    if (lastt == 0x0d)
                    {
                        term = 1;
                        corrupt = 0;
                        break;
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
                    /* printf("rectype: RLD\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 3)
                {
                    int l3;
                    
                    /* printf("rectype: Dicionary = Control + RLD\n"); */
                    l3 = *(short *)(q + 6) + 16;
#if 0
                    printf("l3 is %d\n", l3);
#endif
                    if (processRLD(buf, rlad, q, l3) != 0)
                    {
                        term = 1;
                        break;
                    }
                }
                else if (t == 0x0e)
                {
                    /* printf("rectype: Last record of module\n"); */
                    if (processRLD(buf, rlad, q, l2) != 0)
                    {
                        term = 1;
                        break;
                    }
                    term = 1;
                    corrupt = 0;
                    break;
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
        p = p + l;
        if (rem == 0)
        {
#if PE_DEBUG
            printf("breaking cleanly\n");
#endif
        }
        else if (rem < 2)
        {
            break;
        }
    }
    if (corrupt)
    {
        printf("corrupt module\n");
        return (-1);
    }
#if 0
    printf("dumping new module\n");
#endif
    *len = upto - buf; /* return new module length */
    return (0);
}


int processRLD(char *buf, int rlad, char *rld, int len)
{
    int l;
    char *r;
    int cont = 0;
    char *fin;
    int negative;
    int ll;
    int a;
    int newval;
    int *zaploc;
    
    r = rld + 16;
    fin = rld + len;
    while (r != fin)
    {
        if (!cont)
        {
            r += 4; /* skip R & P */
            if (r >= fin)
            {
                printf("corrupt1 at position %x\n", r - rld - 4);
                return (-1);
            }
        }
        negative = *r & 0x02;
        if (negative)
        {
            printf("got a negative adjustment - unsupported\n");
            return (-1);
        }
        ll = (*r & 0x0c) >> 2;
        ll++;
        if ((ll != 4) && (ll != 3))
        {
            printf("untested and unsupported relocation %d\n", ll);
            return (-1);
        }
        if (ll == 3)
        {
            if (rlad > 0xffffff)
            {
                printf("AL3 prevents relocating this module to %x\n", rlad);
                return (-1);
            }
        }
        cont = *r & 0x01; /* do we have A & F continous? */
        r++;
        if ((r + 3) > fin)
        {
            printf("corrupt2 at position %x\n", r - rld);
            return (-1);
        }
        a = 0;
        memcpy((char *)&a + sizeof(a) - 3, r, 3);
        /* +++ need bounds checking on this OS code */
        /* printf("need to zap %d bytes at offset %6x\n", ll, a); */
        zaploc = (int *)(buf + a - ((ll == 3) ? 1 : 0));
        newval = *zaploc;
        /* printf("which means that %8x ", newval); */
        newval += rlad;
        /* printf("becomes %8x\n", newval); */
        *zaploc = newval;
        r += 3;
    }
    return (0);
}
