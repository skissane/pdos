/******************************************************************************
 * @file            user32.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

/* Note that without __EXPORT__ defined, WINAPI gets
   __declspec(dllimport) and tries to import the functions. */
/* So make sure you define that */

#include "windows.h"
#include "pos.h"
#include "svga.h"

/* From pdos.c. */
void *kmalloc(size_t size);
void kfree(void *ptr);

/* Memory allocated this way should be inside user process range
 * and should be accessible to user processes.
 * (It might be good to put nearly everything into user memory later
 *  so it can be easily freed when the process crashes/exits
 *  by freeing whole user memory.)
 */
#define umalloc kmalloc
#define ufree kfree

/* Does nothing for now but in the future
 * it should check that the pointer
 * points somewhere into memory range of user processes,
 * not into kernel memory.
 * (Pointing into nonpresent memory is fine,
 *  it can be handled through page fault handler,
 *  but processes must not read or write kernel memory.)
 */
#define CHECK_POINTER(p) (0)
#define CRASH_PROCESS()
#define CHECK_PTR_AND_C(p) do { if (CHECK_POINTER (p)) CRASH_PROCESS(); } while (0)

struct ll {
    void *c;
    void *next;
};

struct winproc_info {
    struct ll *classes_ll;
    struct window_info *wi;
    struct device_context *one_compatible_dc;
    struct bitmap *one_default_bitmap;
    struct bitmap *one_bitmap;
    int quitting;
    WPARAM quit_wParam;
};

struct device_context {
    struct bitmap *bi;
};

struct bitmap {
    long w, h;
    unsigned char *buf;
};

struct window_info {
    const WNDCLASSA *wc;
    struct device_context *dc;
};

/* In the future this struct should be process-local
 * and part of PCB.
 * (Just replace the "__" with "->" then.)
 */
struct winproc_info s_curPCB__w;
struct winproc_info *curPCB__w = &s_curPCB__w;

/* All this graphics stuff should be elsewhere
 * because whole OS should use it
 * but for now it is here.
 */
#include "../util/font8x8_basic.h"
#include <stdarg.h>

#define BYTES_PER_PIXEL SVGA_BYTES_PER_PIXEL

static unsigned char *screen_buf;
static unsigned int screen_w, screen_h;

static void draw_char_8x8 (char ch,
                           unsigned long foreground_color,
                           unsigned long background_color,
                           unsigned long x,
                           unsigned long y)
{
    int ch_x, ch_y;
    unsigned char *glyph;
    unsigned long *p;

    glyph = font8x8_basic[(unsigned char)ch];

    p = (void *)screen_buf;
    p += y * screen_w + x; 

    for (ch_y = 0; ch_y < 8; ch_y++) {
        for (ch_x = 0; ch_x < 8; ch_x++) {
            if (*glyph & (1 << ch_x)) *p = foreground_color;
            else *p = background_color;
            p++;
        }
        glyph++;
        p += screen_w - 8;
    }     
}

static void draw_string (const char *str,
                         unsigned long fg_color,
                         unsigned long bg_color,
                         unsigned long x,
                         unsigned long y)
{
    while (*str) {
        draw_char_8x8 (*str, fg_color, bg_color, x, y);
        x += 8;
        str++;
    }
}

static void gprintf (const char *fmt, ...)
{
    va_list ap;
    char str[100];
    
    va_start (ap, fmt);
    
    vsprintf (str, fmt, ap);
    draw_string (str, 0xff0000, 0x000000, 30, 30);
    va_end (ap);

    svga_swap_buffer (screen_buf);
}

static void ACTIVATE_SVGA (void)
{
    /* graphtst.c used 1024 * 768. */
    if (svga_vbe_init (1024, 768)) {
        printf ("failed to initialize graphics\n");
        return;
    }

    /* No printf (or PosWriteFile(1,...)) from now on. */

    screen_w = svga_get_width ();
    screen_h = svga_get_height ();

    if (!(screen_buf = kmalloc (screen_w * screen_h * BYTES_PER_PIXEL))) {
        svga_vbe_restore_old_mode ();
        printf ("failed to allocate screen buffer\n");
        return;
    }
    memset (screen_buf, 0, screen_w * screen_h * BYTES_PER_PIXEL);
}

static void DEACTIVATE_SVGA (void)
{
    kfree (screen_buf);
    screen_buf = NULL;

    svga_vbe_restore_old_mode ();
}

/* BIOS cannot write on screen if it is in SVGA mode. */
#define PosWriteFile(a, b, c, d) \
 do { if (screen_buf) gprintf ((b)); else PosWriteFile ((a), (b), (c), (d)); } while (0)

static void display_window (void)
{
    unsigned char *dst;
    const unsigned char *src;
    long ix, iy;
    long w, h;

    dst = screen_buf;
    src = curPCB__w->wi->dc->bi->buf;
    w = curPCB__w->wi->dc->bi->w;
    h = curPCB__w->wi->dc->bi->h;

    for (iy = 0; iy < h; iy++) {
        for (ix = 0; ix < w; ix++) {
            dst[ix * BYTES_PER_PIXEL] = src[ix * 3];
            dst[ix * BYTES_PER_PIXEL + 1] = src[ix * 3 + 1];
            dst[ix * BYTES_PER_PIXEL + 2] = src[ix * 3 + 2];
        }

        dst += screen_w * BYTES_PER_PIXEL;
        src += w * 3;
    }

    svga_swap_buffer (screen_buf);
}

static void START_WINGUI_PROGRAM (void)
{
    memset (curPCB__w, 0, sizeof *curPCB__w);

    ACTIVATE_SVGA ();
}

static void END_WINGUI_PROGRAM (void)
{
    kfree (curPCB__w->one_default_bitmap);
    curPCB__w->one_default_bitmap = NULL;

    {
        struct ll *ll, *next_ll;

        for (ll = curPCB__w->classes_ll; ll; ll = next_ll) {
             next_ll = ll->next;
             kfree (ll->c);
             kfree (ll);
        }
    }

    ufree (curPCB__w->wi->dc->bi->buf);
    kfree (curPCB__w->wi->dc->bi);
    kfree (curPCB__w->wi->dc);
    kfree (curPCB__w->wi);

    DEACTIVATE_SVGA ();
}

/* kernel32.dll */
HMODULE WINAPI GetModuleHandleA (LPCSTR lpModuleName)
{
    if (lpModuleName) return NULL;

    /* Should return handle to the file used to create the process,
     * but the handle has been closed already and no longer exists...
     * (see exeloado.c)
     * so just return something eye-catching.
     */
    return (HMODULE)0x12345678;
}

/* user32.dll */
HDC WINAPI BeginPaint (HWND hWnd,
                       LPPAINTSTRUCT lpPaint)
{
    unsigned int len = 26;
    PosWriteFile(1, "BeginPaint unimplemented\r\n", len, &len);
    for (;;) ;
}

HWND WINAPI CreateWindowExA (DWORD dwExStyle,
                             LPCSTR lpClassName,
                             LPCSTR lpWindowName,
                             DWORD dwStyle,
                             int X,
                             int Y,
                             int nWidth,
                             int nHeight,
                             HWND hWndParent,
                             HMENU hMenu,
                             HINSTANCE hInstance,
                             LPVOID lpParam)
{
    WNDCLASSA *wc;
    struct window_info *wi;

    CHECK_PTR_AND_C (lpClassName);
    CHECK_PTR_AND_C (lpWindowName);
    CHECK_PTR_AND_C (lpParam);

    /* Only full screen windows are supported for now. */
    if (X != 0
        || Y != 0
        || nWidth != screen_w
        || nHeight != screen_h
        || hWndParent != NULL
        || hMenu != NULL
        || lpParam != NULL) return NULL;

    if (lpClassName == NULL) return NULL;

    {
        struct ll *ll;

        for (ll = curPCB__w->classes_ll; ll; ll = ll->next) {
            if (!strcmp (((WNDCLASSA *)(ll->c))->lpszClassName,
                         lpClassName)) {
                wc = ll->c;
                break;
            }
        }

        if (!ll) return NULL;
    }

    {
        struct device_context *dc = NULL;;
        struct bitmap *bi = NULL;
        void *buf;

        if (!(wi = kmalloc (sizeof *wi))
            || !(dc = kmalloc (sizeof *dc))
            || !(bi = kmalloc (sizeof *bi))
            || !(buf = umalloc (3 * nWidth * nHeight))) {
            kfree (wi);
            kfree (dc);
            kfree (bi);
            return NULL;
        }

        memset (wi, 0, sizeof *wi);
        memset (dc, 0, sizeof *dc);
        memset (bi, 0, sizeof *bi);

        bi->w = nWidth;
        bi->h = nHeight;
        bi->buf = buf;

        dc->bi = bi;

        wi->dc = dc;
        wi->wc = wc;
    }

    curPCB__w->wi = wi;
    
    return wi;
}

LRESULT WINAPI DefWindowProcA (HWND hWnd,
                               UINT Msg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    unsigned int len = 30;
    PosWriteFile(1, "DefWindowProcA unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI DestroyWindow (HWND hWnd)
{
    unsigned int len = 29;
    PosWriteFile(1, "DestroyWindow unimplemented\r\n", len, &len);
    for (;;) ;
}

INT_PTR WINAPI DialogBoxIndirectParamA (HINSTANCE hInstance,
                                        LPCDLGTEMPLATEA hDialogTemplate,
                                        HWND hWndParent,
                                        DLGPROC lpDialogFunc,
                                        LPARAM dwInitParam)
{
    unsigned int len = 39;
    PosWriteFile(1, "DialogBoxIndirectParamA unimplemented\r\n", len, &len);
    for (;;) ;
}

LRESULT WINAPI DispatchMessageA (const MSG *lpMsg)
{
    CHECK_PTR_AND_C (lpMsg);

    if (lpMsg->hwnd != curPCB__w->wi) return 0; /* Not sure what to do. */

    /* The function called is user function, not kernel function,
     * so safer call should be done in future.
     */
    return (*curPCB__w->wi->wc->lpfnWndProc) (lpMsg->hwnd,
                                              lpMsg->message,
                                              lpMsg->wParam,
                                              lpMsg->lParam);
}

BOOL WINAPI EndDialog (HWND hDlg,
                       INT_PTR nResult)
{
    unsigned int len = 25;
    PosWriteFile(1, "EndDialog unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI EndPaint (HWND hWnd,
                      const PAINTSTRUCT *lpPaint)
{
    unsigned int len = 24;
    PosWriteFile(1, "EndPaint unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI GetClientRect (HWND hWnd,
                           LPRECT lpRect)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClientRect unimplemented\r\n", len, &len);
    for (;;) ;
}

HDC WINAPI GetDC (HWND hWnd)
{
    if (hWnd == NULL) return NULL;
    if (hWnd == curPCB__w->wi) return curPCB__w->wi->dc;

    return NULL;
}

UINT WINAPI GetDlgItemTextA (HWND hDlg,
                             int nIDDlgItem,
                             LPSTR lpString,
                             int cchMax)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetDlgItemTextA unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI GetMessageA (LPMSG lpMsg,
                         HWND hWnd,
                         UINT wMsgFilterMin,
                         UINT wMsgFilterMax)
{
    CHECK_PTR_AND_C (lpMsg);

    if (hWnd == (HWND)-1
        || (hWnd != NULL && hWnd != curPCB__w->wi)) return -1;

    if (wMsgFilterMin || wMsgFilterMax) return -1;

    memset (lpMsg, 0, sizeof *lpMsg);
    lpMsg->hwnd = curPCB__w->wi;

    if (curPCB__w->quitting) {
        lpMsg->message = WM_QUIT;
        lpMsg->wParam = curPCB__w->quit_wParam;
        return 0;
    } else {
        unsigned int c;

        /* Ask for a key in blocking way because message queues do not exist yet. */
        c = PosDirectCharInputNoEcho ();

        /* Correctly WM_KEYDOWN should be generated
         * and the message would be duplicated
         * and translated in TranslateMessage()
         * but without message queue existing
         * it is not possible.
         */
        lpMsg->message = WM_CHAR;
        lpMsg->wParam = c;
        lpMsg->lParam = 0;
    }
        
    return 1;
}

int WINAPI GetSystemMetrics (int nIndex)
{
    switch (nIndex) {
        case SM_CXSCREEN: return screen_w;
        case SM_CYSCREEN: return screen_h;
    }

    return 0; /* Error. */
}

BOOL WINAPI InvalidateRect (HWND hWnd,
                            const RECT *lpRect,
                            BOOL bErase)
{
    unsigned int len = 30;
    PosWriteFile(1, "InvalidateRect unimplemented\r\n", len, &len);
    for (;;) ;
}

HCURSOR WINAPI LoadCursorA (HINSTANCE hInstance,
                            LPCSTR lpCursorName)
{
    /* Just return error because no cursors exist yet. */
    return NULL;
}

DWORD WINAPI MsgWaitForMultipleObjects (DWORD nCount,
                                        const HANDLE *pHandles,
                                        BOOL fWaitAll,
                                        DWORD dwMilliseconds,
                                        DWORD dwWakeMask)
{
    unsigned int len = 41;
    PosWriteFile(1, "MsgWaitForMultipleObjects unimplemented\r\n", len, &len);
    for (;;) ;
}

DWORD WINAPI MsgWaitForMultipleObjectsEx (DWORD nCount,
                                          const HANDLE *pHandles,
                                          DWORD dwMilliseconds,
                                          DWORD dwWakeMask,
                                          DWORD dwFlags)
{
    unsigned int len = 43;
    PosWriteFile(1, "MsgWaitForMultipleObjectsEx unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI PeekMessageA (LPMSG lpMsg,
                          HWND hWnd,
                          UINT wMsgFilterMin,
                          UINT wMsgFilterMax,
                          UINT wRemoveMsg)
{
    unsigned int len = 28;
    PosWriteFile(1, "PeekMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}

void WINAPI PostQuitMessage (int nExitCode)
{
    /* Should actually post a WM_QUIT message to message queue. */
    curPCB__w->quit_wParam = nExitCode;
    curPCB__w->quitting = 1;
}

ATOM WINAPI RegisterClassA (const WNDCLASSA *lpWndClass)
{
    /* Should be first function
     * (except for GetModuleHandle() and GetCommandline())
     * called in GUI programs.
     */
    START_WINGUI_PROGRAM ();
    
    CHECK_PTR_AND_C (lpWndClass);
    CHECK_PTR_AND_C (lpWndClass->lpfnWndProc);
    CHECK_PTR_AND_C (lpWndClass->lpszMenuName);
    CHECK_PTR_AND_C (lpWndClass->lpszClassName);

    /* Restriction imposed by the API. */
    if (strlen (lpWndClass->lpszClassName) > 256) return 0;

    {
        WNDCLASSA *new;
        struct ll *ll;

        if (!(new = kmalloc (sizeof *new))) return 0;
        if (!(ll = kmalloc (sizeof *ll))) {
            kfree (new);
            return 0;
        }

        memcpy (new, lpWndClass, sizeof *new);
        ll->c = new;
        ll->next = curPCB__w->classes_ll;
        curPCB__w->classes_ll = ll;
    }

    /* Not sure how to assign ATOM values,
     * so for now return non-0 to indicate success.
     */
    return 1;
}

int WINAPI ReleaseDC (HWND hWnd,
                      HDC hDC)
{
    if (hWnd != NULL) return 0;
    if (hDC != curPCB__w->wi->dc) return 0;
    
    /* Not sure how to hook this up to the process terminating
     * and this the last function called in pfpwin.
     */
    END_WINGUI_PROGRAM ();

    return 1;
}

BOOL WINAPI ShowWindow (HWND hWnd,
                        int nCmdShow)
{
    unsigned int len = 26;
    PosWriteFile(1, "ShowWindow unimplemented\r\n", len, &len);
    for (;;) ;
}

BOOL WINAPI TranslateMessage (const MSG *lpMsg)
{
    CHECK_PTR_AND_C (lpMsg);

    switch (lpMsg->message) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            /* Nonzero should be always returned
             * regardless of the translation for these.
             */
            return 1;

        default:
            return 0;
    }
}

BOOL WINAPI UpdateWindow (HWND hWnd)
{
    unsigned int len = 28;
    PosWriteFile(1, "UpdateWindow unimplemented\r\n", len, &len);
    for (;;) ;
}

/* gdi32.dll */
BOOL WINAPI BitBlt (HDC hdc,
                    int x,
                    int y,
                    int cx,
                    int cy,
                    HDC hdcSrc,
                    int x1,
                    int y1,
                    DWORD rop)
{
    unsigned char *dst;
    const unsigned char *src;
    long w, h, w1, h1;
    int ix, iy;

    if (hdc != curPCB__w->wi->dc
        || hdcSrc != curPCB__w->one_compatible_dc) return 0;
    if (rop != SRCCOPY) return 0;
    
    dst = curPCB__w->wi->dc->bi->buf;
    src = curPCB__w->one_compatible_dc->bi->buf;
    w = curPCB__w->wi->dc->bi->w;
    h = curPCB__w->wi->dc->bi->h;
    w1 = curPCB__w->one_compatible_dc->bi->w;
    h1 = curPCB__w->one_compatible_dc->bi->h;

    if (!(x < w) || !(y < h) || !(x1 < w1) || !(y1 < h1)) return 0;

    /* Clipping should be done on the destination. */
    if (x + cx > w) cx = w - x;
    if (y + cy > h) cy = h - y;

    dst += y * w * 3;
    src += y1 * w1 * 3;

    for (iy = 0; iy < cy; iy++) {
        for (ix = 0; ix < cx; ix++) {
            dst[(x + ix) * 3] = src[(x1 + ix) * 3];
            dst[(x + ix) * 3 + 1] = src[(x1 + ix) * 3 + 1];
            dst[(x + ix) * 3 + 2] = src[(x1 + ix) * 3 + 2];
        }
        dst += w * 3;
        src += w1 * 3;
    }

    display_window ();

    return 1;
}

HDC WINAPI CreateCompatibleDC (HDC hdc)
{
    if (hdc == NULL) {
        struct device_context *dc;
        struct bitmap *bi;

        /* Only one compatible DC is allowed for now. */
        if (curPCB__w->one_compatible_dc) return NULL;
        
        if (!(dc = kmalloc (sizeof *dc))) return NULL;
        if (!(bi = kmalloc (sizeof *bi + 1))) {
            kfree (dc);
            return NULL;
        }

        dc->bi = bi;
        curPCB__w->one_default_bitmap = bi;

        bi->w = bi->h = 1;
        bi->buf = (void *)(bi + 1);
        
        curPCB__w->one_compatible_dc = dc;
        return dc;
    }

    return NULL;
}

HBITMAP WINAPI CreateDIBSection (HDC hdc,
                                 const BITMAPINFO *pbmi,
                                 UINT usage,
                                 VOID **ppvBits,
                                 HANDLE hSection,
                                 DWORD offset)
{
    struct bitmap *bi;
    const BITMAPINFOHEADER *bmihdr;
    
    CHECK_PTR_AND_C (pbmi);
    CHECK_PTR_AND_C (ppvBits);

    *ppvBits = NULL;

    if (usage != DIB_RGB_COLORS
        || hSection != NULL) return NULL;

    bmihdr = &pbmi->bmiHeader;
    if (bmihdr->biSize != 40
        || bmihdr->biPlanes != 1
        || bmihdr->biBitCount != 24
        || bmihdr->biCompression != BI_RGB) return NULL;

    if (bmihdr->biWidth <= 0
        || bmihdr->biHeight >= 0) return NULL;

    /* Negative biHeight means it is top down bitmap. */

    if (!(bi = kmalloc (sizeof *bi))) return NULL;
    if (!(bi->buf = umalloc (3 * bmihdr->biWidth * -bmihdr->biHeight))) {
        kfree (bi);
        return NULL;
    }

    bi->w = bmihdr->biWidth;
    bi->h = -bmihdr->biHeight;
    curPCB__w->one_bitmap = bi;

    *ppvBits = bi->buf;
    
    return bi;
}

HBRUSH WINAPI CreateSolidBrush (COLORREF color)
{
    /* Just return error for now. */
    return NULL;
}

BOOL WINAPI DeleteDC (HDC hdc)
{
    if (hdc != curPCB__w->one_compatible_dc) return 0;

    kfree (curPCB__w->one_compatible_dc);
    curPCB__w->one_compatible_dc = NULL;

    return 1;
}

BOOL WINAPI DeleteObject (HGDIOBJ ho)
{
    if (ho != curPCB__w->one_bitmap) return 0;
;
    ufree (curPCB__w->one_bitmap->buf);
    kfree (curPCB__w->one_bitmap);
    curPCB__w->one_bitmap = NULL;

    return 1;
}

BOOL WINAPI Rectangle (HDC hdc,
                       int left,
                       int top,
                       int right,
                       int bottom)
{
    unsigned char *dst;
    long w;
    int ix, iy;

    if (hdc != curPCB__w->wi->dc) return 0;

    if (left < 0 || top < 0
        || right < 0 || bottom < 0) return 0;

    if (left >= curPCB__w->wi->dc->bi->w
        || top >= curPCB__w->wi->dc->bi->h) return 0;

    dst = curPCB__w->wi->dc->bi->buf;
    w = curPCB__w->wi->dc->bi->w;

    dst += top * w * 3;

    for (iy = top; iy < bottom; iy++) {
        for (ix = left; ix < right; ix++) {
            /* Brushes are not supported yet,
             * so assume black brush is desired.
             */
            dst[ix * 3] = 0;
            dst[ix * 3 + 1] = 0;
            dst[ix * 3 + 2] = 0;
        }
        dst += w * 3;
    }

    display_window ();
    
    return 1;
}

HGDIOBJ WINAPI SelectObject (HDC hdc,
                             HGDIOBJ h)
{
    HGDIOBJ orig_h;
    
    if (hdc != curPCB__w->one_compatible_dc
        || h != curPCB__w->one_bitmap) return NULL;

    orig_h = curPCB__w->one_compatible_dc->bi;
    curPCB__w->one_compatible_dc->bi = h;

    return orig_h;
}

BOOL WINAPI TextOutA (HDC hdc,
                      int x,
                      int y,
                      LPCSTR lpString,
                      int c)
{
    unsigned int len = 24;
    PosWriteFile(1, "TextOutA unimplemented\r\n", len, &len);
    for (;;) ;
}

struct syscall_entry {
    void *function;
    unsigned int num_args;
};

struct syscall_entry syscall_table[] = {
    /* kernel32.dll */
    {&GetModuleHandleA, 1},
    /* user32.dll */
    {&BeginPaint, 2},
    {&CreateWindowExA, 12},
    {&DefWindowProcA, 4},
    {&DestroyWindow, 1},
    {&DialogBoxIndirectParamA, 5},
    {&DispatchMessageA, 1},
    {&EndDialog, 2},
    {&EndPaint, 2},
    {&GetClientRect, 2},
    {&GetDC, 1},
    {&GetDlgItemTextA, 4},
    {&GetMessageA, 4},
    {&GetSystemMetrics, 1},
    {&InvalidateRect, 3},
    {&LoadCursorA, 2},
    {&MsgWaitForMultipleObjects, 5},
    {&MsgWaitForMultipleObjectsEx, 5},
    {&PeekMessageA, 5},
    {&PostQuitMessage, 1},
    {&RegisterClassA, 1},
    {&ReleaseDC, 2},
    {&ShowWindow, 2},
    {&TranslateMessage, 1},
    {&UpdateWindow, 1},
    /* gdi32.dll */
    {&BitBlt, 9},
    {&CreateCompatibleDC, 1},
    {&CreateDIBSection, 6},
    {&CreateSolidBrush, 1},
    {&DeleteDC, 1},
    {&DeleteObject, 1},
    {&Rectangle, 5},
    {&SelectObject, 2},
    {&TextOutA, 5}    
};

unsigned int PosWinSyscall (unsigned int function_index, void *arguments)
{
    if (function_index >= sizeof (syscall_table) / sizeof (syscall_table[0])) {
        printf ("wrong arguments passed to PosWinSyscall, halting\n"); for (;;);
    }

    if (CHECK_POINTER (arguments)
        || CHECK_POINTER ((void *)(((unsigned int *)arguments)
                                   + syscall_table[function_index].num_args))) {
        printf ("wrong arguments passed to PosWinSyscall, halting\n"); for (;;);
    }
    
    return stdcallWithArgCopy (syscall_table[function_index].function,
                               arguments,
                               syscall_table[function_index].num_args);
}
