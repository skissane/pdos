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
#include <windows.h>
#include <pos.h>

#include "dllsyscall.h"
#define INDEX_START 1

HDC WINAPI BeginPaint (HWND hWnd,
                       LPPAINTSTRUCT lpPaint)
{
    return (HDC)system_call_2 (INDEX_START, hWnd, lpPaint);
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
    return (HWND)system_call_12 (INDEX_START + 1,
                                 dwExStyle,
                                 lpClassName,
                                 lpWindowName,
                                 dwStyle,
                                 X,
                                 Y,
                                 nWidth,
                                 nHeight,
                                 hWndParent,
                                 hMenu,
                                 hInstance,
                                 lpParam);
}

LRESULT WINAPI DefWindowProcA (HWND hWnd,
                               UINT Msg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    return (LRESULT)system_call_4 (INDEX_START + 2,
                                   hWnd,
                                   Msg,
                                   wParam,
                                   lParam);
}

BOOL WINAPI DestroyWindow (HWND hWnd)
{
    return (BOOL)system_call_1 (INDEX_START + 3, hWnd);
}

INT_PTR WINAPI DialogBoxIndirectParamA (HINSTANCE hInstance,
                                        LPCDLGTEMPLATEA hDialogTemplate,
                                        HWND hWndParent,
                                        DLGPROC lpDialogFunc,
                                        LPARAM dwInitParam)
{
    return (INT_PTR)system_call_5 (INDEX_START + 4,
                                   hInstance,
                                   hDialogTemplate,
                                   hWndParent,
                                   lpDialogFunc,
                                   dwInitParam);
}

LRESULT WINAPI DispatchMessageA (const MSG *lpMsg)
{
    return (LRESULT)system_call_1 (INDEX_START + 5, lpMsg);
}

BOOL WINAPI EndDialog (HWND hDlg,
                       INT_PTR nResult)
{
    return (BOOL)system_call_2 (INDEX_START + 6,
                                hDlg,
                                nResult);
}

BOOL WINAPI EndPaint (HWND hWnd,
                      const PAINTSTRUCT *lpPaint)
{
    return (BOOL)system_call_2 (INDEX_START + 7,
                                hWnd,
                                lpPaint);
}

BOOL WINAPI GetClientRect (HWND hWnd,
                           LPRECT lpRect)
{
    return (BOOL)system_call_2 (INDEX_START + 8,
                                hWnd,
                                lpRect);
}

HDC WINAPI GetDC (HWND hWnd)
{
    return (HDC)system_call_1 (INDEX_START + 9, hWnd);
}

UINT WINAPI GetDlgItemTextA (HWND hDlg,
                             int nIDDlgItem,
                             LPSTR lpString,
                             int cchMax)
{
    return (UINT)system_call_4 (INDEX_START + 10,
                                hDlg,
                                nIDDlgItem,
                                lpString,
                                cchMax);
}

BOOL WINAPI GetMessageA (LPMSG lpMsg,
                         HWND hWnd,
                         UINT wMsgFilterMin,
                         UINT wMsgFilterMax)
{
    return (BOOL)system_call_4 (INDEX_START + 11,
                                lpMsg,
                                hWnd,
                                wMsgFilterMin,
                                wMsgFilterMax);
}

int WINAPI GetSystemMetrics (int nIndex)
{
    return (int)system_call_1 (INDEX_START + 12, nIndex);
}

BOOL WINAPI InvalidateRect (HWND hWnd,
                            const RECT *lpRect,
                            BOOL bErase)
{
    return (BOOL)system_call_3 (INDEX_START + 13,
                                hWnd,
                                lpRect,
                                bErase);
}

HCURSOR WINAPI LoadCursorA (HINSTANCE hInstance,
                            LPCSTR lpCursorName)
{
    return (HCURSOR)system_call_2 (INDEX_START + 14,
                                   hInstance,
                                   lpCursorName);
}

DWORD WINAPI MsgWaitForMultipleObjects (DWORD nCount,
                                        const HANDLE *pHandles,
                                        BOOL fWaitAll,
                                        DWORD dwMilliseconds,
                                        DWORD dwWakeMask)
{
    return (DWORD)system_call_5 (INDEX_START + 15,
                                 nCount,
                                 pHandles,
                                 fWaitAll,
                                 dwMilliseconds,
                                 dwWakeMask);
}

DWORD WINAPI MsgWaitForMultipleObjectsEx (DWORD nCount,
                                          const HANDLE *pHandles,
                                          DWORD dwMilliseconds,
                                          DWORD dwWakeMask,
                                          DWORD dwFlags)
{
    return (DWORD)system_call_5 (INDEX_START + 16,
                                 nCount,
                                 pHandles,
                                 dwMilliseconds,
                                 dwWakeMask,
                                 dwFlags);
}

BOOL WINAPI PeekMessageA (LPMSG lpMsg,
                          HWND hWnd,
                          UINT wMsgFilterMin,
                          UINT wMsgFilterMax,
                          UINT wRemoveMsg)
{
    return (BOOL)system_call_5 (INDEX_START + 17,
                                lpMsg,
                                hWnd,
                                wMsgFilterMin,
                                wMsgFilterMax,
                                wRemoveMsg);
}

void WINAPI PostQuitMessage (int nExitCode)
{
    system_call_1 (INDEX_START + 18, nExitCode);
}

ATOM WINAPI RegisterClassA (const WNDCLASSA *lpWndClass)
{
    return (ATOM)system_call_1 (INDEX_START + 19, lpWndClass);
}

int WINAPI ReleaseDC (HWND hWnd,
                      HDC hDC)
{
    return (int)system_call_2 (INDEX_START + 20,
                               hWnd,
                               hDC);
}

BOOL WINAPI ShowWindow (HWND hWnd,
                        int nCmdShow)
{
    return (BOOL)system_call_2 (INDEX_START + 21,
                                hWnd,
                                nCmdShow);
}

BOOL WINAPI TranslateMessage (const MSG *lpMsg)
{
    return (BOOL)system_call_1 (INDEX_START + 22, lpMsg);
}

BOOL WINAPI UpdateWindow (HWND hWnd)
{
    return (BOOL)system_call_1 (INDEX_START + 23, hWnd);
}

/* auto-genned dummy functions */
void WINAPI ActivateKeyboardLayout(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ActivateKeyboardLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddClipboardFormatListener(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "AddClipboardFormatListener unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddVisualIdentifier(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "AddVisualIdentifier unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AdjustWindowRect(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "AdjustWindowRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AdjustWindowRectEx(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "AdjustWindowRectEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AdjustWindowRectExForDpi(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "AdjustWindowRectExForDpi unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AlignRects(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "AlignRects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AllowForegroundActivation(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "AllowForegroundActivation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AllowSetForegroundWindow(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "AllowSetForegroundWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AnimateWindow(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "AnimateWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AnyPopup(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "AnyPopup unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AppendMenuA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "AppendMenuA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AppendMenuW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "AppendMenuW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AreDpiAwarenessContextsEqual(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "AreDpiAwarenessContextsEqual unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ArrangeIconicWindows(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ArrangeIconicWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AttachThreadInput(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "AttachThreadInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BeginDeferWindowPos(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "BeginDeferWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BlockInput(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "BlockInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BringWindowToTop(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "BringWindowToTop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BroadcastSystemMessage(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "BroadcastSystemMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BroadcastSystemMessageA(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "BroadcastSystemMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BroadcastSystemMessageExA(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "BroadcastSystemMessageExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BroadcastSystemMessageExW(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "BroadcastSystemMessageExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BroadcastSystemMessageW(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "BroadcastSystemMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BuildReasonArray(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "BuildReasonArray unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CalcMenuBar(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CalcMenuBar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CalculatePopupWindowPosition(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "CalculatePopupWindowPosition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallMsgFilter(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CallMsgFilter unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallMsgFilterA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CallMsgFilterA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallMsgFilterW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CallMsgFilterW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallNextHookEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CallNextHookEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallWindowProcA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CallWindowProcA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallWindowProcW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CallWindowProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CancelShutdown(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CancelShutdown unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CascadeChildWindows(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CascadeChildWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CascadeWindows(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CascadeWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeClipboardChain(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ChangeClipboardChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeDisplaySettingsA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ChangeDisplaySettingsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeDisplaySettingsExA(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "ChangeDisplaySettingsExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeDisplaySettingsExW(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "ChangeDisplaySettingsExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeDisplaySettingsW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ChangeDisplaySettingsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeMenuA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ChangeMenuA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeMenuW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ChangeMenuW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeWindowMessageFilter(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "ChangeWindowMessageFilter unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChangeWindowMessageFilterEx(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "ChangeWindowMessageFilterEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharLowerA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharLowerA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharLowerBuffA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharLowerBuffA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharLowerBuffW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharLowerBuffW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharLowerW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharLowerW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharNextA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CharNextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharNextExA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CharNextExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharNextW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CharNextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharPrevA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CharPrevA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharPrevExA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CharPrevExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharPrevW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CharPrevW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharToOemA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharToOemA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharToOemBuffA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharToOemBuffA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharToOemBuffW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharToOemBuffW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharToOemW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharToOemW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharUpperA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharUpperA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharUpperBuffA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharUpperBuffA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharUpperBuffW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CharUpperBuffW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CharUpperW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CharUpperW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckBannedOneCoreTransformApi(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "CheckBannedOneCoreTransformApi unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckDBCSEnabledExt(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CheckDBCSEnabledExt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckDlgButton(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CheckDlgButton unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckMenuItem(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CheckMenuItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckMenuRadioItem(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CheckMenuRadioItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckProcessForClipboardAccess(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "CheckProcessForClipboardAccess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckProcessSession(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CheckProcessSession unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckRadioButton(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CheckRadioButton unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckWindowThreadDesktop(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "CheckWindowThreadDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChildWindowFromPoint(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ChildWindowFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChildWindowFromPointEx(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ChildWindowFromPointEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CliImmSetHotKey(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CliImmSetHotKey unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClientThreadSetup(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ClientThreadSetup unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClientToScreen(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ClientToScreen unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClipCursor(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "ClipCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseClipboard(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CloseClipboard unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseDesktop(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "CloseDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseGestureInfoHandle(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "CloseGestureInfoHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseTouchInputHandle(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CloseTouchInputHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseWindow(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CloseWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseWindowStation(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CloseWindowStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ConsoleControl(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ConsoleControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ControlMagnification(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ControlMagnification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyAcceleratorTableA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CopyAcceleratorTableA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyAcceleratorTableW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CopyAcceleratorTableW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyIcon(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "CopyIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyImage(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CopyImage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyRect(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "CopyRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CountClipboardFormats(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CountClipboardFormats unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateAcceleratorTableA(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "CreateAcceleratorTableA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateAcceleratorTableW(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "CreateAcceleratorTableW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateCaret(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CreateCaret unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateCursor(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "CreateCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDCompositionHwndTarget(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "CreateDCompositionHwndTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDesktopA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CreateDesktopA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDesktopExA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreateDesktopExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDesktopExW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreateDesktopExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDesktopW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CreateDesktopW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDialogIndirectParamA(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "CreateDialogIndirectParamA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDialogIndirectParamAorW(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "CreateDialogIndirectParamAorW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDialogIndirectParamW(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "CreateDialogIndirectParamW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDialogParamA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateDialogParamA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDialogParamW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateDialogParamW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateIcon(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CreateIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateIconFromResource(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "CreateIconFromResource unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateIconFromResourceEx(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "CreateIconFromResourceEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateIconIndirect(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateIconIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateMDIWindowA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreateMDIWindowA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateMDIWindowW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreateMDIWindowW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateMenu(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CreateMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePopupMenu(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CreatePopupMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateSyntheticPointerDevice(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "CreateSyntheticPointerDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateSystemThreads(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CreateSystemThreads unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowExW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CreateWindowExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowInBand(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateWindowInBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowInBandEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreateWindowInBandEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowIndirect(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreateWindowIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowStationA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreateWindowStationA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateWindowStationW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreateWindowStationW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CsrBroadcastSystemMessageExW(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "CsrBroadcastSystemMessageExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CtxInitUser32(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CtxInitUser32 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeAbandonTransaction(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "DdeAbandonTransaction unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeAccessData(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DdeAccessData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeAddData(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DdeAddData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeClientTransaction(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "DdeClientTransaction unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeCmpStringHandles(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DdeCmpStringHandles unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeConnect(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DdeConnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeConnectList(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DdeConnectList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeCreateDataHandle(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DdeCreateDataHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeCreateStringHandleA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "DdeCreateStringHandleA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeCreateStringHandleW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "DdeCreateStringHandleW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeDisconnect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DdeDisconnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeDisconnectList(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DdeDisconnectList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeEnableCallback(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DdeEnableCallback unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeFreeDataHandle(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DdeFreeDataHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeFreeStringHandle(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DdeFreeStringHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeGetData(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DdeGetData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeGetLastError(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DdeGetLastError unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeGetQualityOfService(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "DdeGetQualityOfService unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeImpersonateClient(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "DdeImpersonateClient unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeInitializeA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DdeInitializeA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeInitializeW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DdeInitializeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeKeepStringHandle(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DdeKeepStringHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeNameService(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DdeNameService unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdePostAdvise(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DdePostAdvise unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeQueryConvInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DdeQueryConvInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeQueryNextServer(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "DdeQueryNextServer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeQueryStringA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DdeQueryStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeQueryStringW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DdeQueryStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeReconnect(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "DdeReconnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeSetQualityOfService(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "DdeSetQualityOfService unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeSetUserHandle(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DdeSetUserHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeUnaccessData(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DdeUnaccessData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdeUninitialize(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DdeUninitialize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefDlgProcA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DefDlgProcA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefDlgProcW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DefDlgProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefFrameProcA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DefFrameProcA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefFrameProcW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DefFrameProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefMDIChildProcA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DefMDIChildProcA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefMDIChildProcW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DefMDIChildProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefRawInputProc(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DefRawInputProc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DefWindowProcW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DefWindowProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeferWindowPos(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DeferWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeferWindowPosAndBand(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "DeferWindowPosAndBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DelegateInput(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DelegateInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeleteMenu(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DeleteMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeregisterShellHookWindow(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "DeregisterShellHookWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyAcceleratorTable(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "DestroyAcceleratorTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyCaret(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "DestroyCaret unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyCursor(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DestroyCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyDCompositionHwndTarget(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "DestroyDCompositionHwndTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyIcon(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DestroyIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyMenu(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DestroyMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyReasons(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DestroyReasons unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroySyntheticPointerDevice(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "DestroySyntheticPointerDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DialogBoxIndirectParamAorW(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "DialogBoxIndirectParamAorW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DialogBoxIndirectParamW(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "DialogBoxIndirectParamW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DialogBoxParamA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DialogBoxParamA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DialogBoxParamW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DialogBoxParamW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DisableProcessWindowsGhosting(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "DisableProcessWindowsGhosting unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DispatchMessageW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DispatchMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DisplayConfigGetDeviceInfo(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "DisplayConfigGetDeviceInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DisplayConfigSetDeviceInfo(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "DisplayConfigSetDeviceInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DisplayExitWindowsWarnings(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "DisplayExitWindowsWarnings unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirListA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DlgDirListA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirListComboBoxA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DlgDirListComboBoxA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirListComboBoxW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DlgDirListComboBoxW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirListW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DlgDirListW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirSelectComboBoxExA(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "DlgDirSelectComboBoxExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirSelectComboBoxExW(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "DlgDirSelectComboBoxExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirSelectExA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DlgDirSelectExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DlgDirSelectExW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DlgDirSelectExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DoSoundConnect(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DoSoundConnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DoSoundDisconnect(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DoSoundDisconnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DragDetect(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DragDetect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DragObject(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DragObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawAnimatedRects(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DrawAnimatedRects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawCaption(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DrawCaption unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawCaptionTempA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DrawCaptionTempA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawCaptionTempW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DrawCaptionTempW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawEdge(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DrawEdge unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawFocusRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "DrawFocusRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawFrame(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DrawFrame unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawFrameControl(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DrawFrameControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawIcon(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DrawIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawIconEx(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DrawIconEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawMenuBar(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DrawMenuBar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawMenuBarTemp(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "DrawMenuBarTemp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawStateA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DrawStateA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawStateW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DrawStateW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawTextA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DrawTextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawTextExA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DrawTextExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawTextExW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DrawTextExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawTextW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DrawTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmGetDxRgn(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "DwmGetDxRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmGetDxSharedSurface(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "DwmGetDxSharedSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmGetRemoteSessionOcclusionEvent(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "DwmGetRemoteSessionOcclusionEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmGetRemoteSessionOcclusionState(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "DwmGetRemoteSessionOcclusionState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmKernelShutdown(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DwmKernelShutdown unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmKernelStartup(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DwmKernelStartup unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmLockScreenUpdates(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "DwmLockScreenUpdates unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmValidateWindow(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DwmValidateWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EditWndProc(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EditWndProc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EmptyClipboard(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "EmptyClipboard unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableMenuItem(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "EnableMenuItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableMouseInPointer(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "EnableMouseInPointer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableNonClientDpiScaling(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "EnableNonClientDpiScaling unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableOneCoreTransformMode(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "EnableOneCoreTransformMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableScrollBar(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EnableScrollBar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableSessionForMMCSS(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "EnableSessionForMMCSS unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableWindow(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "EnableWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndDeferWindowPos(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EndDeferWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndDeferWindowPosEx(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EndDeferWindowPosEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndMenu(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "EndMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndTask(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "EndTask unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnterReaderModeHelper(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "EnterReaderModeHelper unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumChildWindows(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EnumChildWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumClipboardFormats(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "EnumClipboardFormats unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDesktopWindows(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "EnumDesktopWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDesktopsA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EnumDesktopsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDesktopsW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EnumDesktopsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplayDevicesA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumDisplayDevicesA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplayDevicesW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumDisplayDevicesW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplayMonitors(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumDisplayMonitors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplaySettingsA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "EnumDisplaySettingsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplaySettingsExA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EnumDisplaySettingsExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplaySettingsExW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EnumDisplaySettingsExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumDisplaySettingsW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "EnumDisplaySettingsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumPropsA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EnumPropsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumPropsExA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "EnumPropsExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumPropsExW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "EnumPropsExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumPropsW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EnumPropsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumThreadWindows(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EnumThreadWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumWindowStationsA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumWindowStationsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumWindowStationsW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumWindowStationsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumWindows(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EnumWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EqualRect(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "EqualRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EvaluateProximityToPolygon(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "EvaluateProximityToPolygon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EvaluateProximityToRect(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "EvaluateProximityToRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExcludeUpdateRgn(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "ExcludeUpdateRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExitWindowsEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ExitWindowsEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FillRect(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "FillRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindWindowA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "FindWindowA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindWindowExA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "FindWindowExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindWindowExW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "FindWindowExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindWindowW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "FindWindowW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlashWindow(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "FlashWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlashWindowEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "FlashWindowEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FrameRect(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "FrameRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FreeDDElParam(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "FreeDDElParam unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FrostCrashedWindow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "FrostCrashedWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetActiveWindow(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetActiveWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAltTabInfo(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetAltTabInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAltTabInfoA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetAltTabInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAltTabInfoW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetAltTabInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAncestor(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetAncestor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAppCompatFlags(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetAppCompatFlags unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAppCompatFlags2(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetAppCompatFlags2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAsyncKeyState(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetAsyncKeyState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAutoRotationState(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetAutoRotationState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAwarenessFromDpiAwarenessContext(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "GetAwarenessFromDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCIMSSM(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetCIMSSM unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCapture(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetCapture unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCaretBlinkTime(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetCaretBlinkTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCaretPos(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetCaretPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassInfoA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassInfoExA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetClassInfoExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassInfoExW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetClassInfoExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassInfoW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassLongA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassLongA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassLongW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassLongW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassNameA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassNameW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClassNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClassWord(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetClassWord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipCursor(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetClipCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardAccessToken(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetClipboardAccessToken unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardData(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetClipboardData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardFormatNameA(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetClipboardFormatNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardFormatNameW(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetClipboardFormatNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardMetadata(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetClipboardMetadata unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardOwner(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetClipboardOwner unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardSequenceNumber(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetClipboardSequenceNumber unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipboardViewer(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetClipboardViewer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetComboBoxInfo(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetComboBoxInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentInputMessageSource(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetCurrentInputMessageSource unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCursor(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCursorFrameInfo(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetCursorFrameInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCursorInfo(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetCursorInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCursorPos(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetCursorPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCEx(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "GetDCEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCompositionHwndBitmap(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GetDCompositionHwndBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDesktopID(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetDesktopID unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDesktopWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetDesktopWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDialogBaseUnits(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetDialogBaseUnits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDialogControlDpiChangeBehavior(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "GetDialogControlDpiChangeBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDialogDpiChangeBehavior(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetDialogDpiChangeBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDisplayAutoRotationPreferences(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "GetDisplayAutoRotationPreferences unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDisplayConfigBufferSizes(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetDisplayConfigBufferSizes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDlgCtrlID(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetDlgCtrlID unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDlgItem(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetDlgItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDlgItemInt(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetDlgItemInt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDlgItemTextW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetDlgItemTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDoubleClickTime(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetDoubleClickTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDpiAwarenessContextForProcess(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "GetDpiAwarenessContextForProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDpiForMonitorInternal(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetDpiForMonitorInternal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDpiForSystem(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetDpiForSystem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDpiForWindow(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetDpiForWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDpiFromDpiAwarenessContext(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "GetDpiFromDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetExtendedPointerDeviceProperty(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "GetExtendedPointerDeviceProperty unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFocus(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GetFocus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetForegroundWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetForegroundWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGUIThreadInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGUIThreadInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGestureConfig(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGestureConfig unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGestureExtraArgs(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetGestureExtraArgs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGestureInfo(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetGestureInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGuiResources(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetGuiResources unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetIconInfo(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetIconInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetIconInfoExA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetIconInfoExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetIconInfoExW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetIconInfoExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetInputDesktop(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetInputDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetInputLocaleInfo(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetInputLocaleInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetInputState(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetInputState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetInternalWindowPos(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetInternalWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKBCodePage(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetKBCodePage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyNameTextA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetKeyNameTextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyNameTextW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetKeyNameTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyState(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetKeyState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardLayout(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetKeyboardLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardLayoutList(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetKeyboardLayoutList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardLayoutNameA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetKeyboardLayoutNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardLayoutNameW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetKeyboardLayoutNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardState(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetKeyboardState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKeyboardType(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetKeyboardType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLastActivePopup(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetLastActivePopup unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLastInputInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetLastInputInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLayeredWindowAttributes(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetLayeredWindowAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetListBoxInfo(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetListBoxInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMagnificationDesktopColorEffect(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "GetMagnificationDesktopColorEffect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMagnificationDesktopMagnification(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "GetMagnificationDesktopMagnification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMagnificationDesktopSamplingMode(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "GetMagnificationDesktopSamplingMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMagnificationLensCtxInformation(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "GetMagnificationLensCtxInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenu(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "GetMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuBarInfo(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetMenuBarInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuCheckMarkDimensions(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetMenuCheckMarkDimensions unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuContextHelpId(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetMenuContextHelpId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuDefaultItem(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetMenuDefaultItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuInfo(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetMenuInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuItemCount(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetMenuItemCount unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuItemID(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetMenuItemID unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuItemInfoA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetMenuItemInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuItemInfoW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetMenuItemInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuItemRect(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetMenuItemRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuState(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetMenuState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuStringA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetMenuStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMenuStringW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetMenuStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMessageExtraInfo(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetMessageExtraInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMessagePos(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetMessagePos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMessageTime(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetMessageTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMessageW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMonitorInfoA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetMonitorInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMonitorInfoW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetMonitorInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMouseMovePointsEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetMouseMovePointsEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNextDlgGroupItem(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetNextDlgGroupItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNextDlgTabItem(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetNextDlgTabItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOpenClipboardWindow(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetOpenClipboardWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetParent(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetParent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPhysicalCursorPos(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetPhysicalCursorPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerCursorId(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetPointerCursorId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDevice(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetPointerDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDeviceCursors(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetPointerDeviceCursors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDeviceInputSpace(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetPointerDeviceInputSpace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDeviceOrientation(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetPointerDeviceOrientation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDeviceProperties(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetPointerDeviceProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDeviceRects(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetPointerDeviceRects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerDevices(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetPointerDevices unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameArrivalTimes(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetPointerFrameArrivalTimes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameInfo(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetPointerFrameInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameInfoHistory(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetPointerFrameInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFramePenInfo(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetPointerFramePenInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFramePenInfoHistory(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "GetPointerFramePenInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameTimes(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetPointerFrameTimes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameTouchInfo(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetPointerFrameTouchInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerFrameTouchInfoHistory(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "GetPointerFrameTouchInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerInfo(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetPointerInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerInfoHistory(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetPointerInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerInputTransform(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetPointerInputTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerPenInfo(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetPointerPenInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerPenInfoHistory(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetPointerPenInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerTouchInfo(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetPointerTouchInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerTouchInfoHistory(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetPointerTouchInfoHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPointerType(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetPointerType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPriorityClipboardFormat(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetPriorityClipboardFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessDefaultLayout(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetProcessDefaultLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessDpiAwarenessInternal(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "GetProcessDpiAwarenessInternal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessUIContextInformation(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "GetProcessUIContextInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessWindowStation(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetProcessWindowStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProgmanWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetProgmanWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPropA(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GetPropA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPropW(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GetPropW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetQueueStatus(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetQueueStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawInputBuffer(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetRawInputBuffer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawInputData(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetRawInputData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawInputDeviceInfoA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetRawInputDeviceInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawInputDeviceInfoW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetRawInputDeviceInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawInputDeviceList(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetRawInputDeviceList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRawPointerDeviceData(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetRawPointerDeviceData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetReasonTitleFromReasonCode(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetReasonTitleFromReasonCode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRegisteredRawInputDevices(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetRegisteredRawInputDevices unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetScrollBarInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetScrollBarInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetScrollInfo(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetScrollInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetScrollPos(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetScrollPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetScrollRange(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetScrollRange unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSendMessageReceiver(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetSendMessageReceiver unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetShellChangeNotifyWindow(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetShellChangeNotifyWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetShellWindow(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetShellWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSubMenu(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetSubMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSysColor(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetSysColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSysColorBrush(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetSysColorBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemDpiForProcess(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetSystemDpiForProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemMenu(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetSystemMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemMetricsForDpi(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetSystemMetricsForDpi unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTabbedTextExtentA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetTabbedTextExtentA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTabbedTextExtentW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetTabbedTextExtentW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTaskmanWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetTaskmanWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetThreadDesktop(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetThreadDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetThreadDpiAwarenessContext(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetThreadDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetThreadDpiHostingBehavior(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetThreadDpiHostingBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTitleBarInfo(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetTitleBarInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTopLevelWindow(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetTopLevelWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTopWindow(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTopWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTouchInputInfo(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetTouchInputInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUnpredictedMessagePos(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetUnpredictedMessagePos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUpdateRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetUpdateRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUpdateRgn(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetUpdateRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUpdatedClipboardFormats(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetUpdatedClipboardFormats unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUserObjectInformationA(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GetUserObjectInformationA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUserObjectInformationW(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GetUserObjectInformationW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetUserObjectSecurity(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetUserObjectSecurity unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWinStationInfo(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetWinStationInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindow(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowBand(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetWindowBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowCompositionAttribute(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "GetWindowCompositionAttribute unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowCompositionInfo(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowCompositionInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowContextHelpId(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetWindowContextHelpId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowDC(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetWindowDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowDisplayAffinity(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowDisplayAffinity unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowDpiAwarenessContext(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetWindowDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowDpiHostingBehavior(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetWindowDpiHostingBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowFeedbackSetting(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowFeedbackSetting unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowInfo(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetWindowInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowLongA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowLongA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowLongW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowLongW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowMinimizeRect(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetWindowMinimizeRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowModuleFileName(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetWindowModuleFileName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowModuleFileNameA(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowModuleFileNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowModuleFileNameW(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowModuleFileNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowPlacement(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetWindowPlacement unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowProcessHandle(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetWindowProcessHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetWindowRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowRgn(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetWindowRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowRgnBox(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetWindowRgnBox unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowRgnEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowRgnEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowTextA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowTextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowTextLengthA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetWindowTextLengthA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowTextLengthW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetWindowTextLengthW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowTextW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowThreadProcessId(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetWindowThreadProcessId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowWord(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetWindowWord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GhostWindowFromHungWindow(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GhostWindowFromHungWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GrayStringA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GrayStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GrayStringW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GrayStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HandleDelegatedInput(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "HandleDelegatedInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HideCaret(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "HideCaret unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HiliteMenuItem(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "HiliteMenuItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HungWindowFromGhostWindow(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "HungWindowFromGhostWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPGetIMEA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "IMPGetIMEA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPGetIMEW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "IMPGetIMEW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPQueryIMEA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IMPQueryIMEA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPQueryIMEW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IMPQueryIMEW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPSetIMEA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "IMPSetIMEA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IMPSetIMEW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "IMPSetIMEW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ImpersonateDdeClientWindow(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "ImpersonateDdeClientWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InSendMessage(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "InSendMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InSendMessageEx(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "InSendMessageEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InflateRect(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "InflateRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InheritWindowMonitor(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "InheritWindowMonitor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitDManipHook(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "InitDManipHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializeGenericHidInjection(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "InitializeGenericHidInjection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializeInputDeviceInjection(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "InitializeInputDeviceInjection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializeLpkHooks(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "InitializeLpkHooks unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializePointerDeviceInjection(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "InitializePointerDeviceInjection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializePointerDeviceInjectionEx(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "InitializePointerDeviceInjectionEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializeTouchInjection(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "InitializeTouchInjection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectDeviceInput(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "InjectDeviceInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectGenericHidInput(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "InjectGenericHidInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectKeyboardInput(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "InjectKeyboardInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectMouseInput(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "InjectMouseInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectPointerInput(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "InjectPointerInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectSyntheticPointerInput(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "InjectSyntheticPointerInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InjectTouchInput(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "InjectTouchInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InputSpaceRegionFromPoint(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "InputSpaceRegionFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InsertMenuA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "InsertMenuA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InsertMenuItemA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "InsertMenuItemA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InsertMenuItemW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "InsertMenuItemW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InsertMenuW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "InsertMenuW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InternalGetWindowIcon(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "InternalGetWindowIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InternalGetWindowText(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "InternalGetWindowText unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IntersectRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "IntersectRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InvalidateRgn(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "InvalidateRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InvertRect(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "InvertRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharAlphaA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharAlphaA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharAlphaNumericA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "IsCharAlphaNumericA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharAlphaNumericW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "IsCharAlphaNumericW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharAlphaW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharAlphaW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharLowerA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharLowerA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharLowerW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharLowerW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharUpperA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharUpperA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsCharUpperW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsCharUpperW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsChild(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "IsChild unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsClipboardFormatAvailable(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "IsClipboardFormatAvailable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsDialogMessage(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "IsDialogMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsDialogMessageA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "IsDialogMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsDialogMessageW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "IsDialogMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsDlgButtonChecked(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "IsDlgButtonChecked unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsGUIThread(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "IsGUIThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsHungAppWindow(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "IsHungAppWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsIconic(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "IsIconic unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsImmersiveProcess(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "IsImmersiveProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsInDesktopWindowBand(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "IsInDesktopWindowBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsMenu(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "IsMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsMouseInPointerEnabled(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "IsMouseInPointerEnabled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsOneCoreTransformMode(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "IsOneCoreTransformMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsProcessDPIAware(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "IsProcessDPIAware unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsRectEmpty(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "IsRectEmpty unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsSETEnabled(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "IsSETEnabled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsServerSideWindow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "IsServerSideWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsThreadDesktopComposited(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "IsThreadDesktopComposited unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsThreadMessageQueueAttached(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "IsThreadMessageQueueAttached unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsThreadTSFEventAware(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "IsThreadTSFEventAware unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsTopLevelWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "IsTopLevelWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsTouchWindow(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "IsTouchWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsValidDpiAwarenessContext(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "IsValidDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWinEventHookInstalled(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "IsWinEventHookInstalled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindow(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "IsWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowArranged(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "IsWindowArranged unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowEnabled(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "IsWindowEnabled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowInDestroy(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "IsWindowInDestroy unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowRedirectedForPrint(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "IsWindowRedirectedForPrint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowUnicode(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "IsWindowUnicode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWindowVisible(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "IsWindowVisible unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsWow64Message(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "IsWow64Message unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsZoomed(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "IsZoomed unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI KillTimer(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "KillTimer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadAcceleratorsA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LoadAcceleratorsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadAcceleratorsW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LoadAcceleratorsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadBitmapA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "LoadBitmapA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadBitmapW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "LoadBitmapW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadCursorFromFileA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LoadCursorFromFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadCursorFromFileW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LoadCursorFromFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadCursorW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "LoadCursorW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadIconA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "LoadIconA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadIconW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "LoadIconW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadImageA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "LoadImageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadImageW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "LoadImageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadKeyboardLayoutA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LoadKeyboardLayoutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadKeyboardLayoutEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "LoadKeyboardLayoutEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadKeyboardLayoutW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LoadKeyboardLayoutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadLocalFonts(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "LoadLocalFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadMenuA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "LoadMenuA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadMenuIndirectA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LoadMenuIndirectA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadMenuIndirectW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LoadMenuIndirectW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadMenuW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "LoadMenuW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadRemoteFonts(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "LoadRemoteFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadStringA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "LoadStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadStringW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "LoadStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LockSetForegroundWindow(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "LockSetForegroundWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LockWindowStation(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LockWindowStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LockWindowUpdate(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "LockWindowUpdate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LockWorkStation(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "LockWorkStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LogicalToPhysicalPoint(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "LogicalToPhysicalPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LogicalToPhysicalPointForPerMonitorDPI(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "LogicalToPhysicalPointForPerMonitorDPI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LookupIconIdFromDirectory(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "LookupIconIdFromDirectory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LookupIconIdFromDirectoryEx(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "LookupIconIdFromDirectoryEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MBToWCSEx(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "MBToWCSEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MBToWCSExt(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "MBToWCSExt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MB_GetString(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "MB_GetString unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MITGetCursorUpdateHandle(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "MITGetCursorUpdateHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MITSetLastInputRecipient(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "MITSetLastInputRecipient unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MITSynthesizeTouchInput(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "MITSynthesizeTouchInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MakeThreadTSFEventAware(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "MakeThreadTSFEventAware unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapDialogRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "MapDialogRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapPointsByVisualIdentifier(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "MapPointsByVisualIdentifier unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapVirtualKeyA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "MapVirtualKeyA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapVirtualKeyExA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "MapVirtualKeyExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapVirtualKeyExW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "MapVirtualKeyExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapVirtualKeyW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "MapVirtualKeyW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapVisualRelativePoints(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "MapVisualRelativePoints unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapWindowPoints(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "MapWindowPoints unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MenuItemFromPoint(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "MenuItemFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MenuWindowProcA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "MenuWindowProcA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MenuWindowProcW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "MenuWindowProcW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBeep(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "MessageBeep unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "MessageBoxA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxExA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "MessageBoxExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxExW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "MessageBoxExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxIndirectA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "MessageBoxIndirectA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxIndirectW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "MessageBoxIndirectW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxTimeoutA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "MessageBoxTimeoutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxTimeoutW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "MessageBoxTimeoutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MessageBoxW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "MessageBoxW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModifyMenuA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ModifyMenuA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModifyMenuW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ModifyMenuW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MonitorFromPoint(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "MonitorFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MonitorFromRect(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "MonitorFromRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MonitorFromWindow(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "MonitorFromWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MoveWindow(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "MoveWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI NotifyOverlayWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "NotifyOverlayWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI NotifyWinEvent(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "NotifyWinEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OemKeyScan(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "OemKeyScan unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OemToCharA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "OemToCharA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OemToCharBuffA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "OemToCharBuffA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OemToCharBuffW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "OemToCharBuffW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OemToCharW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "OemToCharW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OffsetRect(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "OffsetRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenClipboard(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "OpenClipboard unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenDesktopA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "OpenDesktopA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenDesktopW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "OpenDesktopW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenIcon(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "OpenIcon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenInputDesktop(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "OpenInputDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenThreadDesktop(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "OpenThreadDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenWindowStationA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "OpenWindowStationA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenWindowStationW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "OpenWindowStationW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PackDDElParam(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "PackDDElParam unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PackTouchHitTestingProximityEvaluation(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "PackTouchHitTestingProximityEvaluation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PaintDesktop(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PaintDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PaintMenuBar(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PaintMenuBar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PaintMonitor(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PaintMonitor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PeekMessageW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PeekMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PhysicalToLogicalPoint(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "PhysicalToLogicalPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PhysicalToLogicalPointForPerMonitorDPI(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "PhysicalToLogicalPointForPerMonitorDPI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PostMessageA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PostMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PostMessageW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PostMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PostThreadMessageA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "PostThreadMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PostThreadMessageW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "PostThreadMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrintWindow(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "PrintWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrivateExtractIconExA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "PrivateExtractIconExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrivateExtractIconExW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "PrivateExtractIconExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrivateExtractIconsA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "PrivateExtractIconsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrivateExtractIconsW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "PrivateExtractIconsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrivateRegisterICSProc(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "PrivateRegisterICSProc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PtInRect(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "PtInRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryBSDRWindow(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "QueryBSDRWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryDisplayConfig(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "QueryDisplayConfig unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QuerySendMessage(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "QuerySendMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMAddInputObserver(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RIMAddInputObserver unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMAreSiblingDevices(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RIMAreSiblingDevices unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMDeviceIoControl(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RIMDeviceIoControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMEnableMonitorMappingForDevice(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "RIMEnableMonitorMappingForDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMFreeInputBuffer(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RIMFreeInputBuffer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetDevicePreparsedData(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "RIMGetDevicePreparsedData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetDevicePreparsedDataLockfree(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "RIMGetDevicePreparsedDataLockfree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetDeviceProperties(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RIMGetDeviceProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetDevicePropertiesLockfree(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "RIMGetDevicePropertiesLockfree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetPhysicalDeviceRect(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "RIMGetPhysicalDeviceRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMGetSourceProcessId(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RIMGetSourceProcessId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMObserveNextInput(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RIMObserveNextInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMOnAsyncPnpWorkNotification(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "RIMOnAsyncPnpWorkNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMOnPnpNotification(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RIMOnPnpNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMOnTimerNotification(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RIMOnTimerNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMQueryDevicePath(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RIMQueryDevicePath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMReadInput(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "RIMReadInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMRegisterForInput(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RIMRegisterForInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMRegisterForInputEx(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RIMRegisterForInputEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMRemoveInputObserver(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RIMRemoveInputObserver unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMSetExtendedDeviceProperty(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "RIMSetExtendedDeviceProperty unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMSetTestModeStatus(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RIMSetTestModeStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMUnregisterForInput(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RIMUnregisterForInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RIMUpdateInputObserverRegistration(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "RIMUpdateInputObserverRegistration unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RealChildWindowFromPoint(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "RealChildWindowFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RealGetWindowClass(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RealGetWindowClass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RealGetWindowClassA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RealGetWindowClassA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RealGetWindowClassW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RealGetWindowClassW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReasonCodeNeedsBugID(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ReasonCodeNeedsBugID unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReasonCodeNeedsComment(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ReasonCodeNeedsComment unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RecordShutdownReason(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RecordShutdownReason unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RedrawWindow(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "RedrawWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterBSDRWindow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RegisterBSDRWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterClassExA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "RegisterClassExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterClassExW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "RegisterClassExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterClassW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "RegisterClassW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterClipboardFormatA(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "RegisterClipboardFormatA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterClipboardFormatW(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "RegisterClipboardFormatW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterDManipHook(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "RegisterDManipHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterDeviceNotificationA(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "RegisterDeviceNotificationA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterDeviceNotificationW(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "RegisterDeviceNotificationW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterErrorReportingDialog(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "RegisterErrorReportingDialog unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterForCustomDockTargets(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "RegisterForCustomDockTargets unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterForTooltipDismissNotification(void)
{
    unsigned int len = 53;
    PosWriteFile(1, "RegisterForTooltipDismissNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterFrostWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RegisterFrostWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterGhostWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RegisterGhostWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterHotKey(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "RegisterHotKey unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterLogonProcess(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RegisterLogonProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterMessagePumpHook(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "RegisterMessagePumpHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterPointerDeviceNotifications(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "RegisterPointerDeviceNotifications unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterPointerInputTarget(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "RegisterPointerInputTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterPointerInputTargetEx(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "RegisterPointerInputTargetEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterPowerSettingNotification(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "RegisterPowerSettingNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterRawInputDevices(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "RegisterRawInputDevices unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterServicesProcess(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "RegisterServicesProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterSessionPort(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RegisterSessionPort unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterShellHookWindow(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "RegisterShellHookWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterSuspendResumeNotification(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "RegisterSuspendResumeNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterSystemThread(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "RegisterSystemThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterTasklist(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "RegisterTasklist unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterTouchHitTestingWindow(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "RegisterTouchHitTestingWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterTouchWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RegisterTouchWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterUserApiHook(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RegisterUserApiHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterWindowMessageA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RegisterWindowMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RegisterWindowMessageW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RegisterWindowMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReleaseCapture(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ReleaseCapture unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReleaseDwmHitTestWaiters(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "ReleaseDwmHitTestWaiters unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveClipboardFormatListener(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "RemoveClipboardFormatListener unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveInjectionDevice(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RemoveInjectionDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveMenu(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "RemoveMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemovePropA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "RemovePropA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemovePropW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "RemovePropW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveThreadTSFEventAwareness(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "RemoveThreadTSFEventAwareness unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveVisualIdentifier(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "RemoveVisualIdentifier unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReplyMessage(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ReplyMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReportInertia(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ReportInertia unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResolveDesktopForWOW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ResolveDesktopForWOW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReuseDDElParam(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ReuseDDElParam unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScreenToClient(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ScreenToClient unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScrollChildren(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ScrollChildren unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScrollDC(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "ScrollDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScrollWindow(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ScrollWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScrollWindowEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ScrollWindowEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendDlgItemMessageA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SendDlgItemMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendDlgItemMessageW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SendDlgItemMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendIMEMessageExA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SendIMEMessageExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendIMEMessageExW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SendIMEMessageExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendInput(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SendInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SendMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageCallbackA(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SendMessageCallbackA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageCallbackW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SendMessageCallbackW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageTimeoutA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SendMessageTimeoutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageTimeoutW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SendMessageTimeoutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendMessageW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SendMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendNotifyMessageA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SendNotifyMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SendNotifyMessageW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SendNotifyMessageW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetActiveWindow(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetActiveWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetAdditionalForegroundBoostProcesses(void)
{
    unsigned int len = 53;
    PosWriteFile(1, "SetAdditionalForegroundBoostProcesses unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCapture(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetCapture unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCaretBlinkTime(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetCaretBlinkTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCaretPos(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "SetCaretPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetClassLongA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetClassLongA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetClassLongW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetClassLongW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetClassWord(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetClassWord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetClipboardData(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetClipboardData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetClipboardViewer(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetClipboardViewer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCoalescableTimer(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SetCoalescableTimer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCoreWindow(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetCoreWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCursor(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCursorContents(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetCursorContents unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCursorPos(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetCursorPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDebugErrorLevel(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetDebugErrorLevel unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDeskWallpaper(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetDeskWallpaper unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDesktopColorTransform(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "SetDesktopColorTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDialogControlDpiChangeBehavior(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "SetDialogControlDpiChangeBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDialogDpiChangeBehavior(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "SetDialogDpiChangeBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDisplayAutoRotationPreferences(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "SetDisplayAutoRotationPreferences unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDisplayConfig(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetDisplayConfig unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDlgItemInt(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetDlgItemInt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDlgItemTextA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetDlgItemTextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDlgItemTextW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetDlgItemTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDoubleClickTime(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetDoubleClickTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFeatureReportResponse(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "SetFeatureReportResponse unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFocus(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "SetFocus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetForegroundRedirectionForActivationObject(void)
{
    unsigned int len = 59;
    PosWriteFile(1, "SetForegroundRedirectionForActivationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetForegroundWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SetForegroundWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFullscreenMagnifierOffsetsDWMUpdated(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "SetFullscreenMagnifierOffsetsDWMUpdated unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetGestureConfig(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetGestureConfig unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetInternalWindowPos(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetInternalWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetKeyboardState(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetKeyboardState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetLastErrorEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetLastErrorEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetLayeredWindowAttributes(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "SetLayeredWindowAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagnificationDesktopColorEffect(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "SetMagnificationDesktopColorEffect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagnificationDesktopMagnification(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "SetMagnificationDesktopMagnification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagnificationDesktopMagnifierOffsetsDWMUpdated(void)
{
    unsigned int len = 65;
    PosWriteFile(1, "SetMagnificationDesktopMagnifierOffsetsDWMUpdated unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagnificationDesktopSamplingMode(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "SetMagnificationDesktopSamplingMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagnificationLensCtxInformation(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "SetMagnificationLensCtxInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenu(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "SetMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuContextHelpId(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetMenuContextHelpId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuDefaultItem(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetMenuDefaultItem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuInfo(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "SetMenuInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuItemBitmaps(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetMenuItemBitmaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuItemInfoA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetMenuItemInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMenuItemInfoW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetMenuItemInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMessageExtraInfo(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SetMessageExtraInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMessageQueue(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetMessageQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMirrorRendering(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetMirrorRendering unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetParent(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetParent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPhysicalCursorPos(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetPhysicalCursorPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPointerDeviceInputSpace(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "SetPointerDeviceInputSpace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessDPIAware(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetProcessDPIAware unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessDefaultLayout(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "SetProcessDefaultLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessDpiAwarenessContext(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "SetProcessDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessDpiAwarenessInternal(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "SetProcessDpiAwarenessInternal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessLaunchForegroundPolicy(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "SetProcessLaunchForegroundPolicy unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessRestrictionExemption(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "SetProcessRestrictionExemption unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessWindowStation(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "SetProcessWindowStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProgmanWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetProgmanWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPropA(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "SetPropA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPropW(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "SetPropW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetRect(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "SetRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetRectEmpty(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetRectEmpty unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetScrollInfo(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetScrollInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetScrollPos(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetScrollPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetScrollRange(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetScrollRange unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetShellChangeNotifyWindow(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "SetShellChangeNotifyWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetShellWindow(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetShellWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetShellWindowEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetShellWindowEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSysColors(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetSysColors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSysColorsTemp(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetSysColorsTemp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSystemCursor(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetSystemCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSystemMenu(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetSystemMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTaskmanWindow(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetTaskmanWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadCursorCreationScaling(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "SetThreadCursorCreationScaling unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadDesktop(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetThreadDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadDpiAwarenessContext(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "SetThreadDpiAwarenessContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadDpiHostingBehavior(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "SetThreadDpiHostingBehavior unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadInputBlocked(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SetThreadInputBlocked unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTimer(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "SetTimer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetUserObjectInformationA(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "SetUserObjectInformationA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetUserObjectInformationW(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "SetUserObjectInformationW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetUserObjectSecurity(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SetUserObjectSecurity unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWinEventHook(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetWinEventHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowBand(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetWindowBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowCompositionAttribute(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "SetWindowCompositionAttribute unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowCompositionTransition(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "SetWindowCompositionTransition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowContextHelpId(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "SetWindowContextHelpId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowDisplayAffinity(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "SetWindowDisplayAffinity unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowFeedbackSetting(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "SetWindowFeedbackSetting unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowLongA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowLongA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowLongW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowLongW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowPlacement(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetWindowPlacement unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowPos(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowRgn(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetWindowRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowRgnEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowRgnEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowStationUser(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetWindowStationUser unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowTextA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowTextA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowTextW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowTextW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowWord(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetWindowWord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowsHookA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetWindowsHookA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowsHookExA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetWindowsHookExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowsHookExAW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetWindowsHookExAW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowsHookExW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetWindowsHookExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowsHookW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetWindowsHookW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellHandwritingDelegateInput(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "ShellHandwritingDelegateInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellHandwritingHandleDelegatedInput(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "ShellHandwritingHandleDelegatedInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellHandwritingUndelegateInput(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "ShellHandwritingUndelegateInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellMigrateWindow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "ShellMigrateWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellRegisterHotKey(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ShellRegisterHotKey unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShellSetWindowPos(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ShellSetWindowPos unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowCaret(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "ShowCaret unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowCursor(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "ShowCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowOwnedPopups(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ShowOwnedPopups unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowScrollBar(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ShowScrollBar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowStartGlass(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "ShowStartGlass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowSystemCursor(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "ShowSystemCursor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShowWindowAsync(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ShowWindowAsync unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShutdownBlockReasonCreate(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "ShutdownBlockReasonCreate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShutdownBlockReasonDestroy(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "ShutdownBlockReasonDestroy unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ShutdownBlockReasonQuery(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "ShutdownBlockReasonQuery unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SignalRedirectionStartComplete(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "SignalRedirectionStartComplete unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SkipPointerFrameMessages(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "SkipPointerFrameMessages unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SoftModalMessageBox(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SoftModalMessageBox unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SoundSentry(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "SoundSentry unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SubtractRect(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SubtractRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwapMouseButton(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SwapMouseButton unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwitchDesktop(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SwitchDesktop unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwitchDesktopWithFade(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SwitchDesktopWithFade unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwitchToThisWindow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SwitchToThisWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SystemParametersInfoA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SystemParametersInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SystemParametersInfoForDpi(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "SystemParametersInfoForDpi unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SystemParametersInfoW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SystemParametersInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TabbedTextOutA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "TabbedTextOutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TabbedTextOutW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "TabbedTextOutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TileChildWindows(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "TileChildWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TileWindows(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "TileWindows unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ToAscii(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "ToAscii unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ToAsciiEx(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "ToAsciiEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ToUnicode(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "ToUnicode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ToUnicodeEx(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ToUnicodeEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TrackMouseEvent(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "TrackMouseEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TrackPopupMenu(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "TrackPopupMenu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TrackPopupMenuEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "TrackPopupMenuEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateAccelerator(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "TranslateAccelerator unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateAcceleratorA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "TranslateAcceleratorA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateAcceleratorW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "TranslateAcceleratorW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateMDISysAccel(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "TranslateMDISysAccel unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateMessageEx(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "TranslateMessageEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UndelegateInput(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "UndelegateInput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnhookWinEvent(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "UnhookWinEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnhookWindowsHook(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "UnhookWindowsHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnhookWindowsHookEx(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "UnhookWindowsHookEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnionRect(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "UnionRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnloadKeyboardLayout(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "UnloadKeyboardLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnlockWindowStation(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "UnlockWindowStation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnpackDDElParam(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "UnpackDDElParam unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterClassA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "UnregisterClassA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterClassW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "UnregisterClassW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterDeviceNotification(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "UnregisterDeviceNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterHotKey(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "UnregisterHotKey unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterMessagePumpHook(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "UnregisterMessagePumpHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterPointerInputTarget(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "UnregisterPointerInputTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterPointerInputTargetEx(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "UnregisterPointerInputTargetEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterPowerSettingNotification(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "UnregisterPowerSettingNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterSessionPort(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "UnregisterSessionPort unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterSuspendResumeNotification(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "UnregisterSuspendResumeNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterTouchWindow(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "UnregisterTouchWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnregisterUserApiHook(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "UnregisterUserApiHook unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateDefaultDesktopThumbnail(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "UpdateDefaultDesktopThumbnail unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateLayeredWindow(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "UpdateLayeredWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateLayeredWindowIndirect(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "UpdateLayeredWindowIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdatePerUserSystemParameters(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "UpdatePerUserSystemParameters unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateWindowInputSinkHints(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "UpdateWindowInputSinkHints unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI User32InitializeImmEntryTable(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "User32InitializeImmEntryTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserClientDllInitialize(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "UserClientDllInitialize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserHandleGrantAccess(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "UserHandleGrantAccess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserLpkPSMTextOut(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "UserLpkPSMTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserLpkTabbedTextOut(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "UserLpkTabbedTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserRealizePalette(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "UserRealizePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UserRegisterWowHandlers(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "UserRegisterWowHandlers unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VRipOutput(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "VRipOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VTagOutput(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "VTagOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ValidateRect(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ValidateRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ValidateRgn(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ValidateRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VkKeyScanA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "VkKeyScanA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VkKeyScanExA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "VkKeyScanExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VkKeyScanExW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "VkKeyScanExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VkKeyScanW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "VkKeyScanW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WCSToMBEx(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "WCSToMBEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WINNLSEnableIME(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "WINNLSEnableIME unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WINNLSGetEnableStatus(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "WINNLSGetEnableStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WINNLSGetIMEHotkey(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "WINNLSGetIMEHotkey unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitForInputIdle(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "WaitForInputIdle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitForRedirectionStartComplete(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "WaitForRedirectionStartComplete unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitMessage(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "WaitMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WinHelpA(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "WinHelpA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WinHelpW(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "WinHelpW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WindowFromDC(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "WindowFromDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WindowFromPhysicalPoint(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "WindowFromPhysicalPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WindowFromPoint(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "WindowFromPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Wow64Transition(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "Wow64Transition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI _UserTestTokenForInteractive(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "_UserTestTokenForInteractive unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gSharedInfo(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "gSharedInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gapfnScSendMessage(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "gapfnScSendMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI keybd_event(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "keybd_event unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI mouse_event(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "mouse_event unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI wsprintfA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "wsprintfA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI wsprintfW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "wsprintfW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI wvsprintfA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "wvsprintfA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI wvsprintfW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "wvsprintfW unimplemented\r\n", len, &len);
    for (;;) ;
}
/* end of auto-genned functions */
