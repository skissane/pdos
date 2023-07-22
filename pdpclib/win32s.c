/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  win32s - attempt to produce a msvcrt.dll suitable for Win32      */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>

#include <windows.h>

LONG FAR PASCAL simpwin(HWND hwnd, WORD msg, WORD wparam, LONG lparam);

int WINAPI _WinMainStart(IN HINSTANCE inst,
                   IN HINSTANCE previnst,
                   IN LPSTR cmd,
                   IN int cmdshow)
{
    WNDCLASS wndClass = { 0, (WNDPROC)simpwin, 0, 0, 0, NULL, NULL, 
                          (HBRUSH)(COLOR_WINDOW+1), "simpwin", "simpwin" };
    HWND hwnd;            
    MSG  msg;              
 
    (void)previnst;
    (void)cmd;       
    wndClass.hInstance = inst;
    RegisterClass(&wndClass);
    hwnd = CreateWindow("simpwin", "Simple Windows", 
                        WS_SYSMENU,
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, inst, NULL );                 
    ShowWindow(hwnd, cmdshow);
    UpdateWindow(hwnd);
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

LONG FAR PASCAL simpwin(HWND hwnd, WORD msg, WORD wparam, LONG lparam)
{
    int calldefault = 1;
    LONG ret = 0;
    static int cnt = 0;
    static char newbuf[80] = "N";
    
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;

            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            TextOut(hdc, 0, 0, newbuf, strlen(newbuf));
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            calldefault = 0;
            break;
            
        case WM_NCPAINT:
        case WM_GETICON:
        case WM_WINDOWPOSCHANGING:
        case WM_ACTIVATEAPP:
        case WM_NCACTIVATE:
        case WM_ACTIVATE:
        case WM_IME_NOTIFY:
        case WM_SHOWWINDOW:
            break;

        case WM_IME_SETCONTEXT:
            lparam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
            break;

        case WM_SETFOCUS:
            ret = 0;
            calldefault = 0;
            break;

#if 1
        case WM_ERASEBKGND:
            /* not sure how to actually erase the background - print spaces? */
            ret = 1;
            calldefault = 0;
            break;
#endif

        case WM_WINDOWPOSCHANGED:
        {
            char buf[80];
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;
            static int loccnt = 0;

            loccnt++;
            cnt++;
            /* sprintf(newbuf, "PPP %d %d %d %X %X\n",
                    loccnt, cnt, (int)msg, (int)wparam, (int)lparam); */
            InvalidateRect(hwnd, NULL, TRUE);
#if 0
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            /* TextOut(hdc, 0, 0, newbuf, strlen(newbuf)); */
            TextOut(hdc, 0, 0, buf, strlen(buf));
            EndPaint(hwnd, &ps);
#endif
            break;
        }
            /* lparam is WINDOWPOS and may have useful information */
            ret = 0;
            calldefault = 0;
            break;  

        case WM_SIZE:
            /* low 16 bits of lparam are width - in pixels it seems */
            /* high 16 bits are height */
            /* unclear why this message is being received in the first
               place when the documentation says if I handle WINDOWPOSCHANGED
               then these messages are not generated. Maybe what they mean
               is that the WM_SIZE will just be a once-off message */
            ret = 0;
            calldefault = 0;
            break;

        case WM_MOVE:
            /* low 16 bits of lparam are x-pos (start). high 16 bits are
               y-pos - same consideration as WM_SIZE */
            ret = 0;
            calldefault = 0;
            break;

        case WM_GETMINMAXINFO:
            /* MINMAXINFO in lparam may be useful or need to be set */
            ret = 0;
            calldefault = 0;
            break;

        case WM_NCCALCSIZE:
            /* if 0, this may have useful information in RECT */
            if (wparam == 0)
            {
                ret = 0;
                /* if we don't call the default, we don't get a window! */
                calldefault = 1;
            }
            else
            {
                cnt += 50000; /* detect if we're getting true */
            }
            break;
            
        case WM_NCCREATE:
            /* lparam is CREATESTRUCT which may have useful information */
            ret = 1;
            calldefault = 0;
            break;
                        
        case WM_CREATE:
            /* lparam is CREATESTRUCT which may have useful information */
            ret = 0;
            calldefault = 0;
            break;

#if 0
        case WM_KEYUP:
        {
            char buf[80] = "DUMMY";
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;
            
            cnt++;
            sprintf(newbuf, "YYY %d %X %X\n", (int)msg, (int)wparam, (int)lparam);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
#endif
        
        case WM_KEYDOWN:
        {
            char buf[80] = "DUMMY";
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;
            
            cnt++;
            /* sprintf(newbuf, "GGG %d %X %X\n", (int)msg, (int)wparam, (int)lparam); */
            if (lparam & (1 << 24))
            {
                memcpy(newbuf, "HHH", 3);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        
        /* default: */
        case WM_CHAR:
        {
            char buf[80] = "DUMMY";
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;
            
            cnt++;
            /* sprintf(newbuf, "ZZZ %d %X %X\n", (int)msg, (int)wparam, (int)lparam); */
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        /* we're still getting some of these, but we no longer care */
        /* case WM_KEYDOWN: */
        default:
            break;
    }
    if (calldefault)
    {
        ret = DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return (ret);
}


#ifdef __WATCOMC__
int __watcall main(int argc, int argv)
{
    return (0);
}
#endif
