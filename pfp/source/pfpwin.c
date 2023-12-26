#define xLOOPALL 0
/* 
6.10.99  V 0.02
28.6.02  v 0.03  improved fit to screen algorithm -> pfp.c
                 spaces in pfp filename
6.12.02  V 0.04  scale1-8; Offset in bitblt
22.8.07  V 0.05  path+filename 90 charcter -> 1024 charcter
*/

#include <windows.h>
#include <stdlib.h>

#include "pfp.h"

/**************************************************************************/

HINSTANCE globalhInstance;
HDC    globalhdc;
HDC    bmphdc;
HBRUSH blackbrush;
unsigned char   *bgr;
HWND   bmphandle,windowhandle;
int    xpos=0,ypos=0,HWIN,VWIN;

/**************************************************************************/
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

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void zeichne(HDC);

void (*scale)() = scale8; 
/* 1: no scale  (center)
   2: no scale  (left upper)
   3: no scale  (right upper)
   4: no scale  (left lower)
   5: no scale  (right lower)
   6: fast  3
   7 slower but better 
   8: best 
*/
/**************************************************************************/

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine, int       nCmdShow)
{MSG msg;
 WNDCLASS    wc; 
 BITMAPINFO  bmpinfo;
 int i;
 char *p,*q;

 globalhInstance=hInstance;
 wc.style         = CS_HREDRAW | CS_VREDRAW;
 wc.lpfnWndProc   = (WNDPROC)WndProc;
 wc.cbClsExtra    = 0;
 wc.cbWndExtra    = 0;
 wc.hInstance     = hInstance;
 wc.hIcon         = NULL;
 wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
 wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
 wc.lpszMenuName  = NULL;
 wc.lpszClassName = "pfp";

 bmpinfo.bmiHeader.biSize=40;
 bmpinfo.bmiHeader.biWidth=HBMP;
 bmpinfo.bmiHeader.biHeight=-VBMP;
 bmpinfo.bmiHeader.biPlanes=1;
 bmpinfo.bmiHeader.biBitCount=24;
 bmpinfo.bmiHeader.biCompression=BI_RGB;
 bmpinfo.bmiHeader.biSizeImage=0;
 bmpinfo.bmiHeader.biXPelsPerMeter=1000;
 bmpinfo.bmiHeader.biYPelsPerMeter=1000;
 bmpinfo.bmiHeader.biClrUsed=0;
 bmpinfo.bmiHeader.biClrImportant=0;


 if (!RegisterClass(&wc)) return (FALSE);
 HWIN=GetSystemMetrics(SM_CXSCREEN);
 VWIN=GetSystemMetrics(SM_CYSCREEN);
 if (!(windowhandle=CreateWindow("pfp", "", WS_POPUP|WS_VISIBLE,xpos,
       ypos, HWIN, VWIN, NULL, NULL, hInstance, NULL))) return (FALSE);
 globalhdc=GetDC(windowhandle);

 if (!(bmphdc=CreateCompatibleDC(NULL))) return (FALSE);
 if (!(bmphandle=
       CreateDIBSection(globalhdc,&bmpinfo,DIB_RGB_COLORS,(void **)&bgr,NULL,0))
      )
    return (FALSE);
 if (!(SelectObject(bmphdc,bmphandle))) return (FALSE);

 blackbrush=CreateSolidBrush(0);

 p=lpCmdLine; 
 q=filename;
 i=0;
 while (*p == ' ') p++; 
 if (*p=='"') {p++; while ( (*q++ = *p++)!='"' && (i++ < 1025) );} 
         else {     while ( (*q++ = *p++) >' ' && (i++ < 1025) );}
 *(--q)=0; 

 init(); decode(); (*scale)(); zeichne(globalhdc);
 
/**************************************************************************/

 while (GetMessage(&msg, NULL,0,0)) 
       {TranslateMessage(&msg); DispatchMessage(&msg); }
 DeleteObject(blackbrush); 
 DeleteObject(bmphandle); 
 DeleteDC(bmphdc);
 ReleaseDC(NULL,globalhdc);
 return (msg.wParam);
}

/**************************************************************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{PAINTSTRUCT ps;
 HDC hdc;
 static int dir=0;
  
 switch (message) 
     {case WM_CHAR:
           if (wParam == ' ')  
             {if (dir<=0) {setpict(pictnr-dir); dir=1; decode(); (*scale)();}
              zeichne(globalhdc);
              if (decode()) dir=0;
              (*scale)();
             }

           else if (wParam == 'b' || wParam == 'B')  
             {if (dir>=0) {setpict(pictnr-2-dir); dir= -1; decode(); (*scale)();}
              zeichne(globalhdc);
              if (pictnr==1) dir=0; else {setpict(pictnr-2); decode(); (*scale)();}
             }

           else if (wParam >= '1' && wParam <= '8')  
             {if         (wParam=='1') scale = scale1;
                 else if (wParam=='2') scale = scale2;
                 else if (wParam=='3') scale = scale3;
                 else if (wParam=='4') scale = scale4;
                 else if (wParam=='5') scale = scale5;
                 else if (wParam=='6') scale = scale6;
                 else if (wParam=='7') scale = scale7;
                 else                scale = scale8;
              setpict(pictnr-1-dir); dir= 0; decode(); (*scale)();
              zeichne(globalhdc);
             }


           else if (wParam == 'x' || wParam == 'X')  PostQuitMessage(0);
           break;
           
      case WM_LBUTTONDOWN:
#ifdef LOOPALL
loop:
#endif
           if (dir<=0) {setpict(pictnr-dir); dir=1; decode(); (*scale)();}
           zeichne(globalhdc);
           if (decode()) dir=0;
           (*scale)();
#ifdef LOOPALL
if (dir==1) goto loop;
#endif
           break;
      case WM_LBUTTONUP:
           break;

      case WM_RBUTTONDOWN:
           if (dir>=0) {setpict(pictnr-2-dir); dir= -1; decode(); (*scale)();}
           zeichne(globalhdc);
           if (pictnr==1) dir=0; else {setpict(pictnr-2); decode(); (*scale)();}
           break;
      case WM_RBUTTONUP:
           break;

      case WM_MOUSEMOVE:
           break;

      case WM_PAINT:
           hdc = BeginPaint (hWnd, &ps);
           zeichne(hdc); 
           EndPaint (hWnd, &ps);
           break; 

      case WM_DESTROY:
           cleanup();
           PostQuitMessage(0);
           break;

      default:
           return (DefWindowProc(hWnd, message, wParam, lParam));
     }
 return (0);
}

/**************************************************************************/

void zeichne(hdc) 
HDC hdc;
{SelectObject(hdc,blackbrush);
 Rectangle(hdc,0,0,HWIN,VWIN);
 BitBlt(hdc,(HWIN-hdim)/2,(VWIN-vdim)/2,hdim,vdim,bmphdc,hoff,voff,SRCCOPY);
}

BOOL CALLBACK Dialog (hwndDlg, message, wParam, lParam)
HWND hwndDlg;
UINT message;
WPARAM wParam;
LPARAM lParam;
{switch (message)
  {case WM_INITDIALOG:
        return TRUE;
   case WM_COMMAND:
        switch (LOWORD(wParam))
          {case IDOK:
                passwd[30]=GetDlgItemText(hwndDlg,0,(char *)passwd,26);
                EndDialog(hwndDlg, wParam);
                return TRUE;
           case IDCANCEL:
                passwd[30]=GetDlgItemText(hwndDlg,0,(char *)passwd,26);
                while (passwd[30]<24) passwd[passwd[30]++]=' ';
                passwd[30]=24;
                EndDialog(hwndDlg, wParam);
                return TRUE;
          }
  }
 return FALSE;
} 

void get_passwd()
{int i;
 char *p;
 typedef struct
   {DWORD style;
    DWORD xstyle;
    WORD  cdit;
    short x;
    short y;
    short cx;
    short cy;
    WORD  menu;
    WORD  class;
    WORD  title;
    
    DWORD style1;
    DWORD xstyle1;
    short x1;
    short y1;
    short cx1;
    short cy1;
    WORD  id1;
    WORD  class1a;
    WORD  class1b;
    char  title1[40];
    WORD  data1;
   }dboxt;

 dboxt dbox1;
 dbox1.style=WS_POPUP|WS_BORDER;
 dbox1.xstyle=0;
 dbox1.cdit=1;
 dbox1.x=10;
 dbox1.y=100;
 dbox1.cx=200;
 dbox1.cy=30;
 dbox1.menu=0;
 dbox1.class=0;
 dbox1.title=0;
 dbox1.style1=WS_CHILD|WS_VISIBLE|SS_LEFT;
 dbox1.xstyle1=0;
 dbox1.x1=10;
 dbox1.y1=10;
 dbox1.cx1=180;
 dbox1.cy1=10;
 dbox1.id1=0;
 dbox1.class1a=0xffff;
 dbox1.class1b=0x0081;
 dbox1.data1=0;
 p="Enter Password       ";

 do
   {i=0; while (i<40) {dbox1.title1[i++]=*p++;dbox1.title1[i++]=0; }
    dbox1.title1[38]=0;
    DialogBoxIndirect(globalhInstance,(LPDLGTEMPLATE) &dbox1,
                      windowhandle,(DLGPROC)Dialog);
    /* MessageBox(windowhandle,passwd,"Password",MB_OK); */
    p="Password to short     ";
   } while (passwd[30]<24);
}

int main ()
{
    /* Ordinarily this would be done in real entry point
     * which would call the WinMain directly
     * but such thing is not available yet.
     */
    char *cmdline;
    
    cmdline = GetCommandLine ();
    
    if (*cmdline == '"') {
        cmdline++;
        while (*cmdline && *cmdline != '"') cmdline++;
        if (*cmdline == '"') cmdline++;
    } else {
        while (*cmdline && *cmdline != ' ') cmdline++;
    }
    
    while (*cmdline && *cmdline == ' ') cmdline++;
    
    return WinMain (GetModuleHandle (NULL), NULL, cmdline, 1);
}
