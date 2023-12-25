/******************************************************************************
 * @file            pfp.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define NPICT (1500)     /* Number of pictures you can go back */
#define HBMP  (1024*2)   /* must be a multiple of 16 */
#define VBMP  (1024*2)   /* must be a multiple of 16 */

/* pfp.c */
extern char filename[];
extern int hdim, vdim;
extern int pictnr;

void init ();
int decode ();

void scale1 ();
void scale2 ();
void scale3 ();
void scale4 ();
void scale5 ();
void scale6 ();
void scale7 ();
void scale8 ();

void setpict (int);

/* pfpsvga.c/win.c */
extern unsigned char *bgr; /* unsigned char bgr[3*HBMP*VBMP]; */
extern int HWIN, VWIN;

void get_passwd ();
