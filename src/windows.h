/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  windows prototypes                                               */
/*                                                                   */
/*********************************************************************/

#include <stddef.h>

#ifndef __EXPORT__
#define WINAPI __declspec(dllimport) __stdcall
#else
#define WINAPI __declspec(dllexport) __stdcall
#endif

#define PASCAL __stdcall

#define STD_INPUT_HANDLE ((DWORD)-10)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_ERROR_HANDLE ((DWORD)-12)
#define INVALID_HANDLE_VALUE ((HANDLE)-1)

#define FALSE 0
#define TRUE  1

#define INFINITE 0xFFFFFFFF

/* Access mask. */
#define GENERIC_ALL     0x10000000
#define GENERIC_EXECUTE 0x20000000
#define GENERIC_WRITE   0x40000000
#define GENERIC_READ    0x80000000

#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

#define FILE_SHARE_DELETE 0x00000004
#define FILE_SHARE_READ   0x00000001
#define FILE_SHARE_WRITE  0x00000002

#define FILE_ATTRIBUTE_ARCHIVE               0x00000020
#define FILE_ATTRIBUTE_COMPRESSED            0x00000800
#define FILE_ATTRIBUTE_DEVICE                0x00000040
#define FILE_ATTRIBUTE_DIRECTORY             0x00000010
#define FILE_ATTRIBUTE_ENCRYPTED             0x00004000
#define FILE_ATTRIBUTE_HIDDEN                0x00000002
#define FILE_ATTRIBUTE_INTEGRITY_STREAM      0x00008000
#define FILE_ATTRIBUTE_NORMAL                0x00000080
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED   0x00002000
#define FILE_ATTRIBUTE_NO_SCRUB_DATA         0x00020000
#define FILE_ATTRIBUTE_OFFLINE               0x00001000
#define FILE_ATTRIBUTE_READONLY              0x00000001
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#define FILE_ATTRIBUTE_RECALL_ON_OPEN        0x00040000
#define FILE_ATTRIBUTE_REPARSE_POINT         0x00000400
#define FILE_ATTRIBUTE_SPARSE_FILE           0x00000200
#define FILE_ATTRIBUTE_SYSTEM                0x00000004
#define FILE_ATTRIBUTE_TEMPORARY             0x00000100
#define FILE_ATTRIBUTE_VIRTUAL               0x00010000

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
#define DLL_THREAD_ATTACH  2
#define DLL_THREAD_DETACH  3

#define SHIFT_PRESSED                           0x0010
#define RIGHT_CTRL_PRESSED                      0x0004
#define LEFT_CTRL_PRESSED                       0x0008
#define RIGHT_ALT_PRESSED                       0x0001
#define LEFT_ALT_PRESSED                        0x0002
#define FOREGROUND_BLUE                         0x0001
#define FOREGROUND_GREEN                        0x0002
#define FOREGROUND_RED                          0x0004

#define KEY_EVENT                               0x0001
#define MOUSE_EVENT                             0x0002
#define WINDOW_BUFFER_SIZE_EVENT                0x0004

#define ENABLE_WINDOW_INPUT                     0x0008
#define ENABLE_PROCESSED_INPUT                  0x0001
#define ENABLE_PROCESSED_OUTPUT                 0x0001
#define ENABLE_LINE_INPUT                       0x0002
#define ENABLE_ECHO_INPUT                       0x0004
#define ENABLE_MOUSE_INPUT                      0x0010

typedef size_t SIZE_T;
typedef unsigned int UINT;
typedef unsigned int DWORD;
typedef unsigned int *LPDWORD;
typedef unsigned int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef long LONG;
typedef char TCHAR;
typedef long *PLONG;
typedef BOOL *LPBOOL;
typedef BYTE *LPBYTE;
typedef WORD *LPWORD;
typedef void *HANDLE;
typedef HANDLE HINSTANCE;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;
typedef TCHAR *LPTCH;
typedef void *HGLOBAL;
typedef void *LPOVERLAPPED;
typedef BOOL (__stdcall *PHANDLER_ROUTINE)(DWORD CtrlType);
typedef unsigned char CHAR;
typedef unsigned short WCHAR;
typedef short SHORT;
typedef void VOID;
typedef unsigned short USHORT;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

#define STARTUPINFO STARTUPINFOA
typedef struct _STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO;

typedef CHAR_INFO *PCHAR_INFO;

typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;

typedef struct _KEY_EVENT_RECORD {
    BOOL  bKeyDown;
    WORD  wRepeatCount;
    WORD  wVirtualKeyCode;
    WORD  wVirtualScanCode;
    union {
      WCHAR UnicodeChar;
      CHAR  AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD;

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD;

typedef struct _WINDOW_BUFFER_SIZE_RECORD {
  COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
    KEY_EVENT_RECORD KeyEvent;
    MOUSE_EVENT_RECORD MouseEvent;
    WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;

typedef struct {
    char unused1a[32];
    DWORD nFileSizeLow;
    char unused1b[8];
    char cFileName[260];
    char unused2[16];
} WIN32_FIND_DATA, WIN32_FIND_DATAA;

HANDLE WINAPI GetStdHandle(DWORD nStdHandle);

BOOL WINAPI SetStdHandle(DWORD nStdHandle, HANDLE hHandle);

BOOL WINAPI WriteFile(HANDLE hFile,
                      LPCVOID lpBuffer,
                      DWORD nNumberOfBytesToWrite,
                      LPDWORD lpNumberOfBytesWritten,
                      LPOVERLAPPED lpOverlapped);

__declspec(noreturn) void WINAPI ExitProcess(UINT uExitCode);

BOOL WINAPI CloseHandle(HANDLE hObject);

#define CreateFile CreateFileA
HANDLE WINAPI CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

#define CreateDirectory CreateDirectoryA
BOOL WINAPI CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);

#define GetFileAttributes GetFileAttributesA
DWORD WINAPI GetFileAttributesA(
    LPCSTR lpFileName);

#define PathFileExists PathFileExistsA
BOOL WINAPI PathFileExistsA(
    LPCSTR pszPath);

#define CreateProcess CreateProcessA
BOOL WINAPI CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

#define DeleteFile DeleteFileA
BOOL WINAPI DeleteFileA(LPCSTR lpFileName);

#define GetCommandLine GetCommandLineA
LPTSTR WINAPI GetCommandLineA(void);

LPTCH WINAPI GetEnvironmentStrings(void);

BOOL WINAPI GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);

DWORD WINAPI GetLastError(void);

HGLOBAL WINAPI GlobalAlloc(UINT uFlags, SIZE_T dwBytes);

HGLOBAL WINAPI GlobalFree(HGLOBAL hMem);

#define MoveFile MoveFileA
BOOL WINAPI MoveFileA(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName);

BOOL WINAPI ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped);

DWORD WINAPI SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod);

DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

void WINAPI GetSystemTime(LPSYSTEMTIME lpSystemTime);

BOOL WINAPI GetConsoleScreenBufferInfo(
    HANDLE hFile,
    CONSOLE_SCREEN_BUFFER_INFO *pcsbi);

#define WriteConsoleOutput WriteConsoleOutputA
BOOL WINAPI WriteConsoleOutputA(
    HANDLE hFile,
    const CHAR_INFO *cinfo,
    COORD bufferSize,
    COORD bufferCoord,
    SMALL_RECT *rect);

BOOL WINAPI GetConsoleMode(HANDLE hFile, DWORD *dw);

BOOL WINAPI SetConsoleMode(HANDLE hFile, DWORD dw);

BOOL WINAPI FreeConsole(void);

#define SetConsoleTitle SetConsoleTitleA
BOOL WINAPI SetConsoleTitleA(LPCTSTR x);

#define GetConsoleTitle GetConsoleTitleA
BOOL WINAPI GetConsoleTitleA(LPCTSTR x, DWORD y);

BOOL WINAPI SetConsoleCursorPosition(HANDLE h, COORD c);

#define FillConsoleOutputCharacter FillConsoleOutputCharacterA
BOOL WINAPI FillConsoleOutputCharacterA(HANDLE h,
                                        TCHAR t,
                                        DWORD d,
                                        COORD c,
                                        LPDWORD lpd);

BOOL WINAPI FillConsoleOutputAttribute(HANDLE h,
                                       WORD w,
                                       DWORD d,
                                       COORD c,
                                       LPDWORD lpd);

#define ReadConsoleInput ReadConsoleInputA
BOOL WINAPI ReadConsoleInputA(HANDLE h, PINPUT_RECORD pi, DWORD d, LPDWORD lpd);

BOOL WINAPI AllocConsole(void);

BOOL WINAPI GetNumberOfConsoleMouseButtons(LPDWORD lpd);

#define FindFirstFile FindFirstFileA
HANDLE WINAPI FindFirstFileA(LPCSTR lpFileName, WIN32_FIND_DATA *FindFileData);

#define FindNextFile FindNextFileA
BOOL WINAPI FindNextFileA(HANDLE h, WIN32_FIND_DATA *FindFileData);

BOOL WINAPI FindClose(HANDLE h);
