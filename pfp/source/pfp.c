/************************************************************************
 Privacy For Pictures pfp.c  v 0.02 20.10.99
*************************************************************************/

/************************************************************************
 Privacy For Pictures pfp.c  v 0.03 28.6.02
 support for 4/2 1/1 1/1 decoding
 improved fit to screen algorithm
 spaces in pfp filename
 batch file for pfp generation added
*************************************************************************/

/************************************************************************
 Privacy For Pictures pfp.c  v 0.04 6.12.02
 support for 1/2 1/1 1/1 decoding
 default fit to screen now mode 4 (best quality)
 full size display now centered
*************************************************************************/

/************************************************************************
 Privacy For Pictures pfp.c  v 0.05 22.8.07
 path+filename 90 charcter -> 1024 charcter
*************************************************************************/

/************************************************************************
 DOS version:      compile with gcc
 WINDOWS 95/98/NT: compile with MS Visual C++
*************************************************************************/
/* Modified, Public Domain. */
#include <stdio.h>
#include <stdlib.h>

#include "pfp.h"

/* Functions defined here. */
void crypt(unsigned char*,unsigned int*);
int getC();
void cleanup();
void ende(int,int);
int get_byte();
int get_word();
void SOI();
void EOI();
void DQT();
void DHT();
void SOF();
void SOS();
void APP0();
void COM();
void UNKNOWN();
void out_string(char*);
void out_byte(int);
void out_bin(int,int);
void out_hex1(int);
void out_hex2(int);
void out_hex4(int);
void out_dec(int);
void print_string(char*);
void print_dec(int);
int get_bits(int);


/*unsigned char bgr[3*HBMP*VBMP]; declared in dos.c or win.c */

int    xold,yold,ldown,rdown;
 
int hdim, vdim;
int hoff, voff; 
int pictnr=0;
int jsize, joffset, jpos;
int decrypt=0;
unsigned char passwd[80],dat[8];
unsigned int key[52],key22,key28;
char filename[1030];
char *head="pfp v0.02 IDEA encrypted file \015\012";

/*#include "dos.c"*/   /* DOS version */
/*#include "win.c"*/   /* WIN version */

int pictpos[NPICT+5]={0};
int hmax, vmax, hunits, vunits; 
int htab[4][1024];
int qtab[4][64];
int tab_h[256], tab_v[256], tab_q[256];
int lastbyte=0, nextbyte1=0, nextbyte2=0, bitsinlastbyte=0;
int mask[17]={0x0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff,0x1ff,
              0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff};
#define lmt 256
unsigned char limit[256+lmt+lmt];
int symb0[1]={0},symb1[2],symb2[4],symb3[8],symb4[16],symb5[32],
    symb6[64],symb7[128],symb8[256],symb9[512],symb10[1024],symb11[2048];
int *symb[12]={symb0,symb1,symb2,symb3,symb4,symb5,symb6,symb7,
               symb8,symb9,symb10,symb11};

#include "fft.h"
int *fft_i4;

FILE *txt=NULL, *jpg=NULL, *fopen(), *fopenb();


/************************************************************************/

void setpict(i) int i;
{int j,k;
 if ((pictnr=i)>NPICT)pictnr=NPICT;
 if (pictnr<0) pictnr=0;
 if (decrypt)
    {j=pictpos[pictnr];
     jpos=j&0xfffffff8;
     fseek(jpg,joffset+jpos,SEEK_SET);
     k=(jpos>>3)+key22;
     key[22]=k&0xffff;
     key[28]=(key28+(k>>16))&0xffff;
     j=j&0x07;
     for (k=0;k<j;k++) getC();
    }
    else  fseek(jpg,pictpos[pictnr],SEEK_SET);
 nextbyte1=nextbyte2=0; get_byte(); get_byte();
}


/************************************************************************/

int init()
{int i,j,k,l;
 char *p;
 jpg = fopen(filename,"rb");
    if (jpg==NULL) {
        ende(2,1);
        return 1;
    }

 tryagain:
 p=head;  while (*p && (*p == getc(jpg))) p++;
 if (! *p)  
    {get_passwd();
     for (i=0;i<8; i++) key[i] =   ((passwd[3*i  ]&0x3f)<<10)
                                  + ((passwd[3*i+1]&0x1f)<< 5)
                                  + ((passwd[3*i+2]&0x1f)    );
     for (i=8;i<52;i++) key[i] = ((key[(i+1)%8?i-7:i-15]<<9) |
                                  (key[(i+2)%8<2?i-14:i-6]>>7))&0xffff; 

     for (i=0;i<52;i++) key[i] = key[i] ^ 0x0dae;  /* avoid week keys */
     decrypt=1; jpos=0; jsize=16; j=0;
     for (i=0;i<8;i++) if (i!=getC()){fseek(jpg,0,0); goto tryagain;}
     for (i=0;i<8;i++) j=(j<<8)+getC();
     jsize=j;
     joffset=ftell(jpg); jpos=0; key22=key[22]; key28=key[28];
    } 

 else fseek(jpg,0,0);   

 get_byte(); get_byte();


j=-1;
for (i=1; i<12; i++)
    {l=(1<<(i-1));
     for (k=0; k<l; k++) symb[i][k]=j++;
     j=-j+1;
     for (k=l; k<2*l; k++) symb[i][k]=j++;
     j=-(j*2-1);
    }

j=0;
for (i=0;i<lmt;i++) limit[j++]=0;
for (i=0;i<255;i++) limit[j++]=i;
for (i=0;i<lmt;i++) limit[j++]=255;

    return 0;
}


/************************************************************************/

void cleanup (void)
{
    if (jpg) {
        fclose (jpg);
        jpg = NULL;
    }

    if (txt) {
        fclose (txt);
        txt = NULL;
    }
}


/************************************************************************/

void scale1() 
{hoff=0; voff=0;
 if (hdim>HWIN) {hoff=(hdim-HWIN)/2; hdim=HWIN;}
 if (vdim>VWIN) {voff=(vdim-VWIN)/2; vdim=VWIN;}
}


/************************************************************************/

void scale2() 
{hoff=0; voff=0;
 if (hdim>HWIN) {hdim=HWIN;}
 if (vdim>VWIN) {vdim=VWIN;}
}


/************************************************************************/

void scale3() 
{hoff=0; voff=0;
 if (hdim>HWIN) {hoff=hdim-HWIN; hdim=HWIN;}
 if (vdim>VWIN) {vdim=VWIN;}
}


/************************************************************************/

void scale4() 
{hoff=0; voff=0;
 if (hdim>HWIN) {hdim=HWIN;}
 if (vdim>VWIN) {voff=(vdim-VWIN); vdim=VWIN;}
}


/************************************************************************/

void scale5() 
{hoff=0; voff=0;
 if (hdim>HWIN) {hoff=hdim-HWIN; hdim=HWIN;}
 if (vdim>VWIN) {voff=vdim-VWIN; vdim=VWIN;}
}


/************************************************************************/

void scale6() 
{int i,j,k,l1,l2,l3;
 hoff=0; voff=0;
 while (hdim>HWIN || vdim>VWIN)
  {i=(hdim+HWIN-1)/HWIN; j=(vdim+VWIN-1)/VWIN;
   if (j>i) i=j;
   if (i==3 || i==9)
    {hdim=hdim/3; vdim=vdim/3;
        
     for (i=0; i<vdim; i++)
      for (j=0; j<hdim; j++)
        {k=(i*HBMP+j)*3; 
         l1=((3*i)*HBMP+3*j)*3;
         l2=l1+HBMP*3;
         l3=l1+HBMP*3;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l1+6]+
                        bgr[l2]+bgr[l2+3]+bgr[l2+6]+
                        bgr[l3]+bgr[l3+3]+bgr[l3+6])/9; l1++;l2++;l3++;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l1+6]+
                        bgr[l2]+bgr[l2+3]+bgr[l2+6]+
                        bgr[l3]+bgr[l3+3]+bgr[l3+6])/9; l1++;l2++;l3++;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l1+6]+
                        bgr[l2]+bgr[l2+3]+bgr[l2+6]+
                        bgr[l3]+bgr[l3+3]+bgr[l3+6])/9;
        }
    }
    else
    {hdim=hdim/2; vdim=vdim/2;
        
     for (i=0; i<vdim; i++)
      for (j=0; j<hdim; j++)
        {k=(i*HBMP+j)*3; 
         l1=((2*i)*HBMP+2*j)*3;
         l2=l1+HBMP*3;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l2]+bgr[l2+3])>>2; l1++;l2++;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l2]+bgr[l2+3])>>2; l1++;l2++;
         bgr[k++]=((int)bgr[l1]+bgr[l1+3]+bgr[l2]+bgr[l2+3])>>2;
        }
    }
  }
}


/************************************************************************/

void scale7() 
{int i,j,k,l,m,n;
 hoff=0; voff=0;
 n=hdim; m=vdim;
 if (hdim>HWIN ||vdim>VWIN)
    {if (hdim*VWIN>vdim*HWIN)
        {m=HWIN*vdim/hdim; n=HWIN;
         for (i=0; i<m; i++)
           for (j=0; j<n; j++)
            {k=(i*HBMP+j)*3;l=((i*hdim/HWIN)*HBMP+j*hdim/HWIN)*3;
             bgr[k++]=bgr[l++];
             bgr[k++]=bgr[l++];
             bgr[k++]=bgr[l++];
            }
        }

       else

        {m=VWIN; n=VWIN*hdim/vdim;
         for (i=0; i<VWIN; i++)
           for (j=0; j<n; j++)
            {k=(i*HBMP+j)*3; l=((i*vdim/VWIN)*HBMP+j*vdim/VWIN)*3;
             bgr[k++]=bgr[l++];
             bgr[k++]=bgr[l++];
             bgr[k++]=bgr[l++];
            }
        }
     hdim=n; vdim=m;
    }
}


/************************************************************************/

/* ein Punkt wird zur Mittelwertbildung herangezogen, falls
   mindestens p2/p1 davon benoetigt werden (muss < 1/2 sein).*/
#define p1 3
#define p2 1

void scale8() 
{int i,j,k,l1,l2,l3,m,n;
 int jmin,jmax,jdiff;
 hoff=0; voff=0;
 if (hdim>HWIN ||vdim>VWIN)
    {if (hdim*VWIN>vdim*HWIN) {m=HWIN*vdim/hdim; n=HWIN;}
                         else {m=VWIN; n=VWIN*hdim/vdim;}

     for (i=0; i<m; i++)
      {jmin=(p1*i*vdim+p2*m)/(p1*m);
       jmax=(p1*i*vdim+p1*vdim-p2*m)/(p1*m);
       jdiff=jmax-jmin+1;
       for (k=0; k<hdim*3; k++)
        {l1=0; for (j=jmin; j<=jmax; j++) l1=l1+bgr[3*j*HBMP+k];
         bgr[3*i*HBMP+k]=l1/jdiff;
        }
      }
     vdim=m;

     for (i=0; i<n; i++)
      {jmin=(p1*i*hdim+p2*n)/(p1*n);
       jmax=(p1*i*hdim+p1*hdim-p2*n)/(p1*n);
       jdiff=jmax-jmin+1;
       for (k=0; k<vdim; k++)
        {l1=l2=l3=0; 
         for (j=jmin; j<=jmax; j++) 
           {l1=l1+bgr[3*k*HBMP+j*3];
            l2=l2+bgr[3*k*HBMP+j*3+1];
            l3=l3+bgr[3*k*HBMP+j*3+2];
           }
         bgr[3*k*HBMP+i*3  ]=l1/jdiff;
         bgr[3*k*HBMP+i*3+1]=l2/jdiff;
         bgr[3*k*HBMP+i*3+2]=l3/jdiff;
        }
      }

     hdim=n; 
    }
}

/************************************************************************/

int decode()
{int i;
 if (nextbyte1 == EOF) return (-1);
 while (nextbyte1 != EOF)
 {switch (i=get_word())
   {case 0xffd8: SOI(); break;
    case 0xffd9: EOI(); break;
    case 0xffdb: DQT(); break;
    case 0xffc4: DHT(); break;
    case 0xffc0: SOF(); break;
    case 0xffda: SOS(); 
         if (pictnr<NPICT)
            {if (decrypt)
                {pictpos[++pictnr]=jpos-2;
                }             
                else pictpos[++pictnr]=ftell(jpg)-2; 
            }
         return (0);
         
    case 0xffe0: APP0();break;
    case 0xfffe: COM(); break;
    default: UNKNOWN(i);
   } 
 }
}


/************************************************************************/

void SOI()
{
}


/************************************************************************/

void EOI()
{int i;
 while (nextbyte1 != EOF) 
    {if (nextbyte1==0xff && nextbyte2==0xd8) return;
     i=get_byte();
    }
}


/************************************************************************/

void DQT()
{int i,j,k;
 i=get_word()-2;
 while (i)
  {if (i<65) ende(3,2);
   j=get_byte();
   if (j>3) ende(4,2);
   for (k=0;k<64;k++) {qtab[j][k]=(get_byte());}
   i=i-65;
  }
}


/************************************************************************/

/*      a[i]: 0000xxyy
        xx: Anzahl der Stellen
        yy: Wert

        a[i]: -(00xxyyyy)
        yyyy: Index in a[] fuer Subtabelle
        xx:   2**xx = Groesse der Subtabelle   */


void DHT()
{int i,j,k,l,m,n,n1,n2,rest;
 int nlang[16];
 int *ht;
 
 rest=get_word()-2;
 if (rest<0) ende(5,3);
 while(rest)
 {rest=rest-17;
  if (rest<0) ende(6,3);
  j=get_byte();

  if      (j==0x00) ht=htab[0]; /* DC table 0 */
  else if (j==0x01) ht=htab[1]; /* DC table 1 */
  else if (j==0x10) ht=htab[2]; /* AC table 0 */
  else if (j==0x11) ht=htab[3]; /* AC table 1 */
  else ende(7,3);
    
  for (j=0;j<16;j++) {nlang[j]=get_byte(); rest=rest-nlang[j];}
  if (rest<0) ende (5,3);

  n=0;  n2=256;  m=16; l=-1;
  for (i=0;i<16;i++)
   {m=(m-1);
    for (j=0;j<nlang[i];j++)
    {if (i>7) {k=n>>8;
               if (k!=l) {l=k; n1=n2;}
               ht[k] = -( ((8-m)<<16)+n1 );
               n2=n1 + ( 1<<(8-m) );
              }
     n=n+(1<<m);
    }
   }

  n=0;
  m=0x10000;
  for (i=0;i<16;i++)
   {m=(m>>1);
    for (j=0;j<nlang[i];j++)
    {k=get_byte();
     if (i<8) {for (l=n>>8; l<(n+m)>>8; l++) ht[l]=((i+1)<<8)+k;}
         else {n1=(-ht[n>>8])>>16; n2=(-ht[n>>8])&0xffff; 
               for (l=0; l < (m>>(8-n1));l++)
                    ht[n2 + ( (n&0xff)>>(8-n1) ) + l]= ((i+1)<<8)+k;
              }
     n=n+m;
    }
   }
  }
}


/************************************************************************/

void SOF()
{int i,j,k,l;
 i=get_word()-8;
 if (i<0) ende(8,4);
 if ((j=get_byte())!=8) ende(9,4); 
 vdim=get_word(); hdim=get_word();
 if (vdim>VBMP || hdim>HBMP) ende(10,5);
 j=get_byte();
 if (i!=(j*3)) ende(11,4);
 hmax=1; vmax=1;
 for (i=0;i<j;i++)
   {k=get_byte(); l=get_byte();
    tab_h[k]=l>>4; tab_v[k]=l&0x0f; tab_q[k]=get_byte();
    if (tab_h[k]>hmax) hmax=tab_h[k];
    if (tab_v[k]>vmax) vmax=tab_v[k];
    if (tab_h[k]<1 || tab_h[k]>4 || tab_v[k]<1 || tab_v[k]>4) ende(12,4);
   }
 hunits= (((hdim+7)/8)+hmax-1)/hmax;
 vunits= (((vdim+7)/8)+vmax-1)/vmax;
}

#define next8bits  ((((lastbyte<<8) + (nextbyte1&0xff)) >> bitsinlastbyte) & 0xff)

void SOS()
{int i,j,k,n;
 int i1,i2,i3,i4,i5,k1,k2,n1,n2,n3,n4;
 int komp_n, komp_h[4], komp_v[4], komp_q[4], komp_c[2][4], komp_dc[4];
 int typ;
 int a[16][64];
 int *b;

 if ( (i=get_word()-6) < 0 ) ende(13,6);
 if ( (komp_n=get_byte()) > 4 ) ende(14,6);
 if (i!=(komp_n*2)) ende(15,6);
 for (i=0;i<komp_n;i++)
   {j=get_byte();
    komp_h[i]=tab_h[j]; komp_v[i]=tab_v[j]; komp_q[i]=tab_q[j]; 
    komp_dc[i]=0;
    j=get_byte(); komp_c[0][i]=j>>4; komp_c[1][i]=2+(j&0x0f);
    if (komp_h[i]!=1 && komp_h[i]!=2 && komp_h[i]!=4) ende(16,7);
    if (komp_v[i]!=1 && komp_v[i]!=2) ende(17,7);
    if (komp_q[i] <0 || komp_q[i]> 3) ende(18,7);
    if (komp_c[0][i]!=0 && komp_c[0][i]!=1) ende(19,7);
    if (komp_c[1][i]!=2 && komp_c[1][i]!=3) ende(20,7);
   }
 if (get_byte()!=0) ende(21,7); 
 if (get_byte()!=63) ende(22,7); 
 if (get_byte()!=0) ende(23,7); 

 if (komp_h[0] != hmax || komp_v[0] != vmax) ende (4,7);

 if      (komp_n==1) typ=0;
 else if (komp_n==3)
         {if      (komp_h[0]==1 && komp_v[0]==1 && 
                   komp_h[1]==1 && komp_v[1]==1 && 
                   komp_h[2]==1 && komp_v[2]==1   ) typ=1;
          else if (komp_h[0]==2 && komp_v[0]==1 && 
                   komp_h[1]==1 && komp_v[1]==1 && 
                   komp_h[2]==1 && komp_v[2]==1   ) typ=2;
          else if (komp_h[0]==2 && komp_v[0]==2 && 
                   komp_h[1]==1 && komp_v[1]==1 && 
                   komp_h[2]==1 && komp_v[2]==1   ) typ=3;
          else if (komp_h[0]==4 && komp_v[0]==2 && 
                   komp_h[1]==1 && komp_v[1]==1 && 
                   komp_h[2]==1 && komp_v[2]==1   ) typ=4;
          else if (komp_h[0]==1 && komp_v[0]==2 && 
                   komp_h[1]==1 && komp_v[1]==1 && 
                   komp_h[2]==1 && komp_v[2]==1   ) typ=5;
          else ende (3,7);
         
         }
 else    ende(24,7);
 

 for (k1=0; k1<vunits; k1++)
 for (k2=0; k2<hunits; k2++)
   {if (nextbyte1==0xff && nextbyte2>=0xd0 && nextbyte2<=0xd7) 
       {nextbyte1=getC(); nextbyte2=getC(); bitsinlastbyte=0;
        for (i=0;i<komp_n;i++) komp_dc[i]=0;
       }    
    i5=0;
    for (i1=0; i1<komp_n    ; i1++)
    for (i2=0; i2<komp_v[i1]; i2++)
    for (i3=0; i3<komp_h[i1]; i3++)
      {j=0; i4=1; b=a[i5++];
       do
       {if ( (i=htab[komp_c[j][i1]][next8bits]) < 0)
         {get_bits(8);
          i=htab[komp_c[j][i1]][((-i)&0xffff)+(next8bits>>(8-((-i)>>16)))]-0x800;
         }
        get_bits(i>>8);
        i=i&0xff;

        if (!j)
           {komp_dc[i1] += symb[i][get_bits(i)];
            k=komp_dc[i1]*qtab[komp_q[i1]][0];
            for (n=0; n<64; n++) b[n]=k*fft[0][n]; 
            j=1;
           } 
        else
           {if      (i==0x00) i4=64;
            else if (i==0xf0) i4=i4+16;
            else {i4=i4+(i>>4);
                  i=i&0x0f;
                  k=symb[i][get_bits(i)]*qtab[komp_q[i1]][i4];
                  for (n=0; n<64; n++) b[n] += k*fft[i4][n];
                  i4++;
                 }
           }
       } while (i4<64);
      }     

      n3=3*(k1*vmax*8*HBMP+((k2*hmax)<<3));

      switch (typ)
       {case 0: 
          n1=0;
          for (i1=0; i1<8; i1++)
           {for (i2=0; i2<8; i2++)
             {bgr[n3++]=limit[((a[0][n1])>>16)+128+lmt];
              bgr[n3++]=limit[((a[0][n1])>>16)+128+lmt];
              bgr[n3++]=limit[((a[0][n1])>>16)+128+lmt];
              n1++;
             }
            n3=n3+(HBMP-8)*3;
           }
          break;

        case 1:
          for (i=0;i<64;i++) {a[1][i]=a[1][i]>>12; a[2][i]=a[2][i]>>12;}
          n1=0;
          for (i1=0; i1<8; i1++)
           {for (i2=0; i2<8; i2++)
             {bgr[n3++]=limit[((a[0][n1]+a[1][n1]*7258)>>16)+128+lmt];
              bgr[n3++]=limit[((a[0][n1]-a[1][n1]*1410-a[2][n1]*2925)>>16)+128+lmt];
              bgr[n3++]=limit[((a[0][n1]+a[2][n1]*5743)>>16)+128+lmt];
              n1++;
             }
            n3=n3+(HBMP-8)*3;
           }
          break;

        case 2:
          for (i=0;i<64;i++) {a[2][i]=a[2][i]>>12; a[3][i]=a[3][i]>>12;}
          for (i1=0; i1<8; i1++)
           {for (i2=0; i2<16; i2++)
             {n2=i1*8+i2/2; n4=i2/8; n1=i1*8+i2%8;
              bgr[n3++]=limit[((a[n4][n1]+a[2][n2]*7258)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]-a[2][n2]*1410-a[3][n2]*2925)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]+a[3][n2]*5743)>>16)+128+lmt];
             }
            n3=n3+(HBMP-16)*3;
           }
          break; 

        case 3:
          for (i=0;i<64;i++) {a[4][i]=a[4][i]>>12; a[5][i]=a[5][i]>>12;}
          for (i1=0; i1<16; i1++)
           {for (i2=0; i2<16; i2++)
             {n2=(i1/2)*8+i2/2; n4=(i1/8)*2+i2/8; n1=(i1%8)*8+i2%8;
              bgr[n3++]=limit[((a[n4][n1]+a[4][n2]*7258)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]-a[4][n2]*1410-a[5][n2]*2925)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]+a[5][n2]*5743)>>16)+128+lmt];
             }
            n3=n3+(HBMP-16)*3;
           }
          break; 

        case 4:
          for (i=0;i<64;i++) {a[8][i]=a[8][i]>>12; a[9][i]=a[9][i]>>12;}
          for (i1=0; i1<16; i1++)
           {for (i2=0; i2<32; i2++)
             {n2=(i1/2)*8+i2/4; n4=(i1/8)*4+i2/8; n1=(i1%8)*8+i2%8;
              bgr[n3++]=limit[((a[n4][n1]+a[8][n2]*7258)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]-a[8][n2]*1410-a[9][n2]*2925)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]+a[9][n2]*5743)>>16)+128+lmt];
             }
            n3=n3+(HBMP-32)*3;
           }
          break; 

        case 5:
          for (i=0;i<64;i++) {a[2][i]=a[2][i]>>12; a[3][i]=a[3][i]>>12;}
          for (i1=0; i1<16; i1++)
           {for (i2=0; i2<8; i2++)
             {n2=(i1/2)*8+i2; n4=i1/8; n1=(i1%8)*8+i2;
              bgr[n3++]=limit[((a[n4][n1]+a[2][n2]*7258)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]-a[2][n2]*1410-a[3][n2]*2925)>>16)+128+lmt];
              bgr[n3++]=limit[((a[n4][n1]+a[3][n2]*5743)>>16)+128+lmt];
            }
            n3=n3+(HBMP-8)*3;
           }
          break; 

        default: ende(25,7);
       }
   }
}


/************************************************************************/

void APP0()
{int i,j,k;
 i=get_word();
 for (j=0;j<i-2;j++) 
     {k=get_byte();
     }
}


/************************************************************************/

void COM()
{int i,j,k;
 i=get_word();
 for (j=0;j<i-2;j++) 
     {k=get_byte();
     }
}


/************************************************************************/

void UNKNOWN(n) int n;
{int i,j,k;

if (n>=0xffc0 && n<0xffff)
   {i=get_word();
    for (j=0;j<i-2;j++) 
        {k=get_byte();
        }
   }
else
   {
    while (nextbyte1 != EOF) 
       {if (nextbyte1==0xff && nextbyte2==0xd8) return;
        i=get_byte();
       }
   }
}


/************************************************************************/

int get_bits(i) int i;
{int k;
 if (!i) return 0;

 if (i<=bitsinlastbyte)
    {
     bitsinlastbyte=bitsinlastbyte-i;
     return ((lastbyte>>bitsinlastbyte) & mask[i]);
    }

 if (i<=bitsinlastbyte+8)
    {
     k = (lastbyte<<8) + nextbyte1;
     lastbyte=nextbyte1; nextbyte1=nextbyte2; nextbyte2=getC();
     if (lastbyte==EOF) ende(26,8); 
     if (lastbyte==0xff) {if (nextbyte1!=0) ende(27,6); 
                          nextbyte1=nextbyte2;
                          nextbyte2=getC();
                         }
     bitsinlastbyte=bitsinlastbyte+8-i;
     return ((k>>bitsinlastbyte) & mask[i]);
    }

 if (i>16) ende(28,6);

    {
     k = (lastbyte<<8) + nextbyte1;
     lastbyte=nextbyte1; nextbyte1=nextbyte2; nextbyte2=getC();
     if (lastbyte==EOF) ende(29,8); 
     if (lastbyte==0xff) {if (nextbyte1!=0) ende(30,6); 
                          nextbyte1=nextbyte2;
                          nextbyte2=getC();
                         }
     k = (k<<8) + nextbyte1;
     lastbyte=nextbyte1; nextbyte1=nextbyte2; nextbyte2=getC();
     if (lastbyte==EOF) ende(31,8); 
     if (lastbyte==0xff) {if (nextbyte1!=0) ende(32,6); 
                          nextbyte1=nextbyte2;
                          nextbyte2=getC();
                         }
     bitsinlastbyte=bitsinlastbyte+16-i;
     return ((k>>bitsinlastbyte) & mask[i]);
    }
}

    
/************************************************************************/

int get_word()
{int i,j;
 if ( (i=get_byte())==EOF) ende(33,8); 
 if ( (j=get_byte())==EOF) ende(34,8); 
 return (i<<8) + j;
}


/************************************************************************/

int get_byte()
{lastbyte=nextbyte1; nextbyte1=nextbyte2; nextbyte2=getC(); 
 if (lastbyte==EOF) ende(35,8); 
 bitsinlastbyte=0;
 return lastbyte;
}


/************************************************************************/

int getC()
{int i,j;
 if (decrypt)
    {
     if (jpos>=jsize) return EOF;
     if (!(i=(jpos++)&0x07))
        {for (j=0;j<8;j++) dat[j]=getc(jpg);
         crypt(dat,key);
         /* modify the key after any 64 bit decryption */
         if (!(key[22]=(key[22]+1)&0xffff)) key[28]=(key[28]+1)&0xffff;
        }
      return dat[i];
    }
    else return getc(jpg);
}


#define mul(x,y) (z1=(x)*(y))?((z2=z1&0xffff)>=(z3=(z1>>16))?(z2-z3)&0xffff:(z2-z3+1)&0xffff):((1-(x)-(y))&0xffff)


/************************************************************************/

void crypt(p,q) unsigned char *p; unsigned int *q;
{unsigned int i,i1,i2,i22,i3,i4,i5,i6,z1,z2,z3;
 i1=(*p++)<<8; i1 += *p++;
 i2=(*p++)<<8; i2 += *p++;
 i3=(*p++)<<8; i3 += *p++;
 i4=(*p++)<<8; i4 += *p;
 for (i=0; i<8; i++)
     {i1 = mul(i1,*q); q++; 
      i22= (i2 + *q++)&0xffff;
      i3 = (i3 + *q++)&0xffff;
      i4 = mul(i4,*q); q++;
      i5 = mul(i1^i3,*q); q++;
      i6 = mul( ((i22^i4)+i5)&0xffff,*q); q++;
      i5 = (i5+i6)&0xffff;
      i1 = i1^i6;
      i2 = i3^i6;
      i3 = i22^i5;
      i4 = i4^i5;
     }
 i1 = mul(i1,*q); q++;
 i22= (i3 + *q++)&0xffff;
 i3 = (i2 + *q++)&0xffff;
 i4 = mul(i4,*q); q++;
 *p-- = i4;
 *p-- = i4>>8;
 *p-- = i3;
 *p-- = i3>>8;
 *p-- = i22;
 *p-- = i22>>8;
 *p-- = i1;
 *p   = i1>>8;
}

/************************************************************************/

void out_byte(i)   int i; {static int spalte=0;
                           if (!txt) return;
                           putc(i,txt); 
                           if (i==0x0a) spalte=-1;
                           if(++spalte>71) 
                             {spalte=0; putc('\015',txt); putc('\012',txt);}
                          }
void out_bin(i,n)  int i,n; {int j; for (j=0;j<16-n;j++) out_byte(' ');
                   j=(1<<n); while (j){if (i&j) out_byte('1'); 
                   else out_byte('0'); j=(j>>1);}}
void out_hex1(i)   int i; { static char hex[] = {'0','1','2','3','4','5',
                   '6','7','8','9','a','b','c','d','e','f'}; 
                   out_byte(hex[(i>>4)&15]); out_byte(hex[i&15]);}
void out_hex2(i)   int i; {out_hex1(i>>8); out_hex1(i&0xff);}
void out_hex4(i)   int i; {out_hex2(i>>16); out_hex2(i&0xffff);}
void out_dec(i)    int i; {int k,j=1; if (i<0) {out_byte('-'); i = -i;}
                   k=i; while (k /= 10) j *= 10; while (j) {k=i/j;
                   out_byte(k+'0'); i -= k*j; j /= 10;}}

void out_string(p) char *p; {while (*p) out_byte(*p++);}


/************************************************************************/

void ende (int j, int i)
{
    static char *(meld[]) = {
        /*  0 */ "can't open log file" ,
        /*  1 */ "can't open input file" ,
        /*  2 */ "Error in Quant. Table" ,
        /*  3 */ "Error in Huffm. Table" ,
        /*  4 */ "Error in  Frame" ,
        /*  5 */ "Picture too big increase HBMP/VBMP" ,
        /*  6 */ "Error in  Scan" ,
        /*  7 */ "unimplemented format" ,
        /*  8 */ "unexpected end of input file" ,
        /*  9 */ "you never should read this" ,
        /* 10 */ "you never should read this" ,
        /* 11 */ "you never should read this" ,
        /* 12 */ "you never should read this" ,
        /* 13 */ "you never should read this" ,
    };

    txt = fopen("pfp.log","wb");
    {
        out_string("\15\12error: Nr.: "); out_dec(j);
        out_string(" in picture Nr.: "); out_dec(pictnr+1);
        out_string(" : ");
        out_string(meld[i]); out_string("\15\12");
    }

    cleanup ();
}

/************************************************************************/

