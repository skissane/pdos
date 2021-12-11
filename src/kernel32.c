/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  kernel32 - implementation of Windows based on PDOS/386           */
/*                                                                   */
/*********************************************************************/

#include <windows.h>

#include <string.h>

#include <pos.h>

HANDLE WINAPI GetStdHandle(DWORD nStdHandle)
{
    if (nStdHandle == -10) return ((HANDLE)0);
    if (nStdHandle == -11) return ((HANDLE)1);
    if (nStdHandle == -12) return ((HANDLE)2);
    return ((HANDLE)-1);
}

BOOL WINAPI WriteFile(HANDLE hFile,
                      LPCVOID lpBuffer,
                      DWORD nNumberOfBytesToWrite,
                      LPDWORD lpNumberOfBytesWritten,
                      LPOVERLAPPED lpOverlapped)
{
    size_t written;
    int ret;

    ret = PosWriteFile((int)hFile, lpBuffer,
                       (size_t)nNumberOfBytesToWrite, &written);
    *lpNumberOfBytesWritten = written;
    /* Positive return code means success. */
    return (!ret);
}


void WINAPI ExitProcess(UINT uExitCode)
{
    PosTerminate(uExitCode);
}

BOOL WINAPI CloseHandle(HANDLE hObject)
{
    int ret;

    ret = PosCloseFile((int)hObject);
    return (ret == 0);
}

HANDLE WINAPI CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    int handle;

    if (dwCreationDisposition == CREATE_ALWAYS)
    {
        if (PosCreatFile(lpFileName, 0, &handle))
        {
            return (INVALID_HANDLE_VALUE);
        }
        return ((HANDLE)handle);
    }
    if (dwCreationDisposition == OPEN_EXISTING)
    {
        if (PosOpenFile(lpFileName, 0, &handle))
        {
            return (INVALID_HANDLE_VALUE);
        }
        return ((HANDLE)handle);
    }

    return (INVALID_HANDLE_VALUE);
}

BOOL WINAPI CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    return (PosMakeDir(lpPathName));
}

DWORD WINAPI GetFileAttributesA(
    LPCSTR lpFileName)
{
    int attr;
    PosGetFileAttributes(lpFileName, &attr);
    return attr;
}

BOOL WINAPI PathFileExistsA(
    LPCSTR pszPath)
{
    int attr;
    PosGetFileAttributes(pszPath, &attr);
    return (attr != -1);
}

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
    LPPROCESS_INFORMATION lpProcessInformation)
{
    POSEXEC_PARMBLOCK parmblock = { 0, NULL, NULL, NULL };

    if (lpCommandLine == NULL)
    {
        return (1);
    }
    parmblock.cmdtail = (char *)lpCommandLine;
    PosExec((char *)lpApplicationName, &parmblock);
    return (1);
}

BOOL WINAPI DeleteFileA(LPCSTR lpFileName)
{
    int ret;

    ret = PosDeleteFile((char *)lpFileName);
    /* 0 from PosDeleteFile means success */
    /* 0 from DeleteFileA means failure */
    return (ret == 0);
}

LPTSTR WINAPI GetCommandLineA(void)
{
    return ((LPTSTR)PosGetCommandLine());
}

LPTCH WINAPI GetEnvironmentStrings(void)
{
    char *env = PosGetEnvBlock();

    return ((LPTCH)env);
}

BOOL WINAPI GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode)
{
    *lpExitCode = (DWORD)PosGetReturnCode();
    return (1);
}

DWORD WINAPI GetLastError(void)
{
    return (0);
}

HGLOBAL WINAPI GlobalAlloc(UINT uFlags, SIZE_T dwBytes)
{
    return (PosAllocMem(dwBytes, POS_LOC32));
}

HGLOBAL WINAPI GlobalFree(HGLOBAL hMem)
{
    PosFreeMem(hMem);
    return (NULL);
}

BOOL WINAPI MoveFileA(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName)
{
    return (PosRenameFile(lpExistingFileName, lpNewFileName) == 0);
}

BOOL WINAPI ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped)
{
    return (!PosReadFile((int)hFile, lpBuffer,
                         nNumberOfBytesToRead, lpNumberOfBytesRead));
}

DWORD WINAPI SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod)
{
    int ret;
    long newpos;

    ret = PosMoveFilePointer((int)hFile,
                             (long)lDistanceToMove,
                             dwMoveMethod,
                             &newpos);
    return (newpos);
}

DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
    return (0);
}

void WINAPI GetSystemTime(LPSYSTEMTIME lpSystemTime)
{
    unsigned int year, month, day, dow, olddow;
    unsigned int hour, min, sec, hundredths;

    PosGetSystemDate(&year, &month, &day, &dow);
    PosGetSystemTime(&hour, &min, &sec, &hundredths);
    olddow = dow;
    PosGetSystemDate(&year, &month, &day, &dow);
    if (olddow != dow)
    {
        PosGetSystemTime(&hour, &min, &sec, &hundredths);
    }
    lpSystemTime->wYear = year;
    lpSystemTime->wMonth = month;
    lpSystemTime->wDay = day;
    lpSystemTime->wDayOfWeek = dow;
    lpSystemTime->wHour = hour;
    lpSystemTime->wMinute = min;
    lpSystemTime->wSecond = sec;
    lpSystemTime->wMilliseconds = hundredths * 10;
    return;
}

DWORD WINAPI GetVersion(void)
{
    return (0);
}

BOOL WINAPI SetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add)
{
    return (0);
}

void WINAPI SetLastError(DWORD dwErrCode)
{
    return;
}

BOOL WINAPI GetConsoleScreenBufferInfo(
    HANDLE hFile,
    CONSOLE_SCREEN_BUFFER_INFO *pcsbi)
{
    pcsbi->dwSize.X = 80;
    pcsbi->dwSize.Y = 25;
    pcsbi->dwMaximumWindowSize = pcsbi->dwSize;
    return (TRUE);
}

BOOL WINAPI WriteConsoleOutputA(
    HANDLE hFile,
    const CHAR_INFO *cinfo,
    COORD bufferSize,
    COORD bufferCoord,
    SMALL_RECT *rect)
{
    char *vidptr = (char *)0xb8000;
    int tot;
    int x;

    tot = bufferSize.X * bufferSize.Y;
    for (x = 0; x < tot; x++)
    {
        *(vidptr + (10 * 80 + x) * 2) = cinfo[x].Char.AsciiChar;
        *(vidptr + (10 * 80 + x) * 2 + 1) = (char)cinfo[x].Attributes;
    }
    return (TRUE);
}

BOOL WINAPI GetConsoleMode(HANDLE hFile, DWORD *dw)
{
    unsigned int devinfo;

    *dw = 0;
    PosGetDeviceInformation((int)hFile, &devinfo);
    if ((devinfo & (1 << 5)) == 0)
    {
        *dw = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
    }
    return (1);
}

BOOL WINAPI SetConsoleMode(HANDLE hFile, DWORD dw)
{
    unsigned int devinfo = 0;

    if ((dw & ENABLE_PROCESSED_INPUT) == 0)
    {
        devinfo = 1 << 5;
    }
    PosSetDeviceInformation((int)hFile, devinfo);
    return (1);
}

BOOL WINAPI GetNumberOfConsoleMouseButtons(LPDWORD lpd)
{
    size_t len = 44;
    PosWriteFile(1, "GetNumberOfConsoleMouseButtons unimplemented\r\n",
                 len, &len);
    for (;;) ;
    return (0);
}

HANDLE WINAPI FindFirstFileA(LPCSTR lpFileName, WIN32_FIND_DATA *FindFileData)
{
    DTA *dta;
    int ret;

    dta = PosGetDTA();
    ret = PosFindFirst((char *)lpFileName, 0x10);
    if (ret == 2) return (INVALID_HANDLE_VALUE);
    strcpy(FindFileData->cFileName, dta->file_name);
    return ((HANDLE)dta);
}

BOOL WINAPI FindNextFileA(HANDLE h, WIN32_FIND_DATA *FindFileData)
{
    DTA *dta = (DTA *)h;
    int ret;

    ret = PosFindNext();
    if (ret != 0) return (0);
    strcpy(FindFileData->cFileName, dta->file_name);
    return (1);
}

BOOL WINAPI FindClose(HANDLE h)
{
    return (1);
}


/* auto-genned dummy functions */

BOOL WINAPI AllocConsole(void)
{
    size_t len = 28;
    PosWriteFile(1, "AllocConsole unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
void WINAPI AttachConsole(void)
{
    size_t len = 29;
    PosWriteFile(1, "AttachConsole unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CallNamedPipeA(void)
{
    size_t len = 30;
    PosWriteFile(1, "CallNamedPipeA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CancelIo(void)
{
    size_t len = 24;
    PosWriteFile(1, "CancelIo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClearCommBreak(void)
{
    size_t len = 30;
    PosWriteFile(1, "ClearCommBreak unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClearCommError(void)
{
    size_t len = 30;
    PosWriteFile(1, "ClearCommError unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CompareFileTime(void)
{
    size_t len = 31;
    PosWriteFile(1, "CompareFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CompareStringW(void)
{
    size_t len = 30;
    PosWriteFile(1, "CompareStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ConnectNamedPipe(void)
{
    size_t len = 32;
    PosWriteFile(1, "ConnectNamedPipe unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFileW(void)
{
    size_t len = 27;
    PosWriteFile(1, "CreateFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateNamedPipeA(void)
{
    size_t len = 32;
    PosWriteFile(1, "CreateNamedPipeA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePipe(void)
{
    size_t len = 26;
    PosWriteFile(1, "CreatePipe unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateProcessW(void)
{
    size_t len = 30;
    PosWriteFile(1, "CreateProcessW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateRemoteThread(void)
{
    size_t len = 34;
    PosWriteFile(1, "CreateRemoteThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateTapePartition(void)
{
    size_t len = 35;
    PosWriteFile(1, "CreateTapePartition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateThread(void)
{
    size_t len = 28;
    PosWriteFile(1, "CreateThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DebugBreak(void)
{
    size_t len = 26;
    PosWriteFile(1, "DebugBreak unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeleteCriticalSection(void)
{
    size_t len = 37;
    PosWriteFile(1, "DeleteCriticalSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeviceIoControl(void)
{
    size_t len = 31;
    PosWriteFile(1, "DeviceIoControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DisconnectNamedPipe(void)
{
    size_t len = 35;
    PosWriteFile(1, "DisconnectNamedPipe unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DuplicateHandle(void)
{
    size_t len = 31;
    PosWriteFile(1, "DuplicateHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnterCriticalSection(void)
{
    size_t len = 36;
    PosWriteFile(1, "EnterCriticalSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EraseTape(void)
{
    size_t len = 25;
    PosWriteFile(1, "EraseTape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EscapeCommFunction(void)
{
    size_t len = 34;
    PosWriteFile(1, "EscapeCommFunction unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExitThread(void)
{
    size_t len = 26;
    PosWriteFile(1, "ExitThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExpandEnvironmentStringsW(void)
{
    size_t len = 41;
    PosWriteFile(1, "ExpandEnvironmentStringsW unimplemented\r\n", len, &len);
    for (;;) ;
}
BOOL WINAPI FillConsoleOutputAttribute(HANDLE h,
                                       WORD w,
                                       DWORD d,
                                       COORD c,
                                       LPDWORD lpd)
{
    size_t len = 42;
    PosWriteFile(1, "FillConsoleOutputAttribute unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
BOOL WINAPI FillConsoleOutputCharacterA(HANDLE h,
                                        TCHAR t,
                                        DWORD d,
                                        COORD c,
                                        LPDWORD lpd)
{
    size_t len = 43;
    PosWriteFile(1, "FillConsoleOutputCharacterA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindFirstVolumeW(void)
{
    size_t len = 32;
    PosWriteFile(1, "FindFirstVolumeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindNextVolumeW(void)
{
    size_t len = 31;
    PosWriteFile(1, "FindNextVolumeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FindVolumeClose(void)
{
    size_t len = 31;
    PosWriteFile(1, "FindVolumeClose unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlushConsoleInputBuffer(void)
{
    size_t len = 39;
    PosWriteFile(1, "FlushConsoleInputBuffer unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlushFileBuffers(void)
{
    size_t len = 32;
    PosWriteFile(1, "FlushFileBuffers unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlushViewOfFile(void)
{
    size_t len = 31;
    PosWriteFile(1, "FlushViewOfFile unimplemented\r\n", len, &len);
    for (;;) ;
}
BOOL WINAPI FreeConsole(void)
{
    size_t len = 27;
    PosWriteFile(1, "FreeConsole unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
void WINAPI FreeEnvironmentStringsW(void)
{
    size_t len = 39;
    PosWriteFile(1, "FreeEnvironmentStringsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FreeLibrary(void)
{
    size_t len = 27;
    PosWriteFile(1, "FreeLibrary unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBinaryTypeW(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetBinaryTypeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCPInfo(void)
{
    size_t len = 25;
    PosWriteFile(1, "GetCPInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCommModemStatus(void)
{
    size_t len = 34;
    PosWriteFile(1, "GetCommModemStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCommState(void)
{
    size_t len = 28;
    PosWriteFile(1, "GetCommState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCommandLineW(void)
{
    size_t len = 31;
    PosWriteFile(1, "GetCommandLineW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetComputerNameA(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetComputerNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetComputerNameW(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetComputerNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetConsoleCP(void)
{
    size_t len = 28;
    PosWriteFile(1, "GetConsoleCP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetConsoleCursorInfo(void)
{
    size_t len = 36;
    PosWriteFile(1, "GetConsoleCursorInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetConsoleWindow(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetConsoleWindow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentProcess(void)
{
    size_t len = 33;
    PosWriteFile(1, "GetCurrentProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentProcessId(void)
{
    size_t len = 35;
    PosWriteFile(1, "GetCurrentProcessId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentThread(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetCurrentThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentThreadId(void)
{
    size_t len = 34;
    PosWriteFile(1, "GetCurrentThreadId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnvironmentStringsW(void)
{
    size_t len = 38;
    PosWriteFile(1, "GetEnvironmentStringsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnvironmentVariableA(void)
{
    size_t len = 39;
    PosWriteFile(1, "GetEnvironmentVariableA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnvironmentVariableW(void)
{
    size_t len = 39;
    PosWriteFile(1, "GetEnvironmentVariableW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetExitCodeThread(void)
{
    size_t len = 33;
    PosWriteFile(1, "GetExitCodeThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFileSize(void)
{
    size_t len = 27;
    PosWriteFile(1, "GetFileSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFileSizeEx(void)
{
    size_t len = 29;
    PosWriteFile(1, "GetFileSizeEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFileType(void)
{
    size_t len = 27;
    PosWriteFile(1, "GetFileType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetHandleInformation(void)
{
    size_t len = 36;
    PosWriteFile(1, "GetHandleInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLocaleInfoA(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetLocaleInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLocaleInfoW(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetLocaleInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLogicalDriveStringsA(void)
{
    size_t len = 39;
    PosWriteFile(1, "GetLogicalDriveStringsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLogicalDrives(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetLogicalDrives unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMailslotInfo(void)
{
    size_t len = 31;
    PosWriteFile(1, "GetMailslotInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetModuleFileNameA(void)
{
    size_t len = 34;
    PosWriteFile(1, "GetModuleFileNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetModuleFileNameW(void)
{
    size_t len = 34;
    PosWriteFile(1, "GetModuleFileNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetModuleHandleA(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetModuleHandleA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetModuleHandleExW(void)
{
    size_t len = 34;
    PosWriteFile(1, "GetModuleHandleExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetModuleHandleW(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetModuleHandleW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNumberOfConsoleInputEvents(void)
{
    size_t len = 45;
    PosWriteFile(1, "GetNumberOfConsoleInputEvents unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOverlappedResult(void)
{
    size_t len = 35;
    PosWriteFile(1, "GetOverlappedResult unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPriorityClass(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetPriorityClass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcAddress(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetProcAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessWorkingSetSize(void)
{
    size_t len = 40;
    PosWriteFile(1, "GetProcessWorkingSetSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStartupInfoA(void)
{
    size_t len = 31;
    PosWriteFile(1, "GetStartupInfoA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStartupInfoW(void)
{
    size_t len = 31;
    PosWriteFile(1, "GetStartupInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemDirectoryW(void)
{
    size_t len = 35;
    PosWriteFile(1, "GetSystemDirectoryW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemInfo(void)
{
    size_t len = 29;
    PosWriteFile(1, "GetSystemInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemTimeAsFileTime(void)
{
    size_t len = 39;
    PosWriteFile(1, "GetSystemTimeAsFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemWindowsDirectoryW(void)
{
    size_t len = 42;
    PosWriteFile(1, "GetSystemWindowsDirectoryW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemWow64DirectoryW(void)
{
    size_t len = 40;
    PosWriteFile(1, "GetSystemWow64DirectoryW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTapeParameters(void)
{
    size_t len = 33;
    PosWriteFile(1, "GetTapeParameters unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTapePosition(void)
{
    size_t len = 31;
    PosWriteFile(1, "GetTapePosition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTapeStatus(void)
{
    size_t len = 29;
    PosWriteFile(1, "GetTapeStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetThreadContext(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetThreadContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetThreadPriority(void)
{
    size_t len = 33;
    PosWriteFile(1, "GetThreadPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTickCount(void)
{
    size_t len = 28;
    PosWriteFile(1, "GetTickCount unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTimeZoneInformation(void)
{
    size_t len = 38;
    PosWriteFile(1, "GetTimeZoneInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetVersionExA(void)
{
    size_t len = 29;
    PosWriteFile(1, "GetVersionExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetVolumePathNamesForVolumeNameW(void)
{
    size_t len = 48;
    PosWriteFile(1, "GetVolumePathNamesForVolumeNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GlobalLock(void)
{
    size_t len = 26;
    PosWriteFile(1, "GlobalLock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GlobalMemoryStatusEx(void)
{
    size_t len = 36;
    PosWriteFile(1, "GlobalMemoryStatusEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GlobalSize(void)
{
    size_t len = 26;
    PosWriteFile(1, "GlobalSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GlobalUnlock(void)
{
    size_t len = 28;
    PosWriteFile(1, "GlobalUnlock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InitializeCriticalSection(void)
{
    size_t len = 41;
    PosWriteFile(1, "InitializeCriticalSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsBadStringPtrA(void)
{
    size_t len = 31;
    PosWriteFile(1, "IsBadStringPtrA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsDebuggerPresent(void)
{
    size_t len = 33;
    PosWriteFile(1, "IsDebuggerPresent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsProcessInJob(void)
{
    size_t len = 30;
    PosWriteFile(1, "IsProcessInJob unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsProcessorFeaturePresent(void)
{
    size_t len = 41;
    PosWriteFile(1, "IsProcessorFeaturePresent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LCMapStringW(void)
{
    size_t len = 28;
    PosWriteFile(1, "LCMapStringW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LeaveCriticalSection(void)
{
    size_t len = 36;
    PosWriteFile(1, "LeaveCriticalSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadLibraryA(void)
{
    size_t len = 28;
    PosWriteFile(1, "LoadLibraryA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadLibraryExA(void)
{
    size_t len = 30;
    PosWriteFile(1, "LoadLibraryExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadLibraryExW(void)
{
    size_t len = 30;
    PosWriteFile(1, "LoadLibraryExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LoadLibraryW(void)
{
    size_t len = 28;
    PosWriteFile(1, "LoadLibraryW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LocalFree(void)
{
    size_t len = 25;
    PosWriteFile(1, "LocalFree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapViewOfFile(void)
{
    size_t len = 29;
    PosWriteFile(1, "MapViewOfFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MapViewOfFileEx(void)
{
    size_t len = 31;
    PosWriteFile(1, "MapViewOfFileEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MultiByteToWideChar(void)
{
    size_t len = 35;
    PosWriteFile(1, "MultiByteToWideChar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenProcess(void)
{
    size_t len = 27;
    PosWriteFile(1, "OpenProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OpenThread(void)
{
    size_t len = 26;
    PosWriteFile(1, "OpenThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OutputDebugStringA(void)
{
    size_t len = 34;
    PosWriteFile(1, "OutputDebugStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PeekConsoleInputA(void)
{
    size_t len = 33;
    PosWriteFile(1, "PeekConsoleInputA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PeekConsoleInputW(void)
{
    size_t len = 33;
    PosWriteFile(1, "PeekConsoleInputW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PeekNamedPipe(void)
{
    size_t len = 29;
    PosWriteFile(1, "PeekNamedPipe unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PrepareTape(void)
{
    size_t len = 27;
    PosWriteFile(1, "PrepareTape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PurgeComm(void)
{
    size_t len = 25;
    PosWriteFile(1, "PurgeComm unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryDosDeviceW(void)
{
    size_t len = 31;
    PosWriteFile(1, "QueryDosDeviceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryInformationJobObject(void)
{
    size_t len = 41;
    PosWriteFile(1, "QueryInformationJobObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryPerformanceCounter(void)
{
    size_t len = 39;
    PosWriteFile(1, "QueryPerformanceCounter unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryPerformanceFrequency(void)
{
    size_t len = 41;
    PosWriteFile(1, "QueryPerformanceFrequency unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueueUserAPC(void)
{
    size_t len = 28;
    PosWriteFile(1, "QueueUserAPC unimplemented\r\n", len, &len);
    for (;;) ;
}
BOOL WINAPI ReadConsoleInputA(HANDLE h, PINPUT_RECORD pi, DWORD d, LPDWORD lpd)
{
    size_t len = 33;
    PosWriteFile(1, "ReadConsoleInputA unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
void WINAPI ReadConsoleInputW(void)
{
    size_t len = 33;
    PosWriteFile(1, "ReadConsoleInputW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReadConsoleOutputW(void)
{
    size_t len = 34;
    PosWriteFile(1, "ReadConsoleOutputW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReadProcessMemory(void)
{
    size_t len = 33;
    PosWriteFile(1, "ReadProcessMemory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReleaseMutex(void)
{
    size_t len = 28;
    PosWriteFile(1, "ReleaseMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ReleaseSemaphore(void)
{
    size_t len = 32;
    PosWriteFile(1, "ReleaseSemaphore unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResetEvent(void)
{
    size_t len = 26;
    PosWriteFile(1, "ResetEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResumeThread(void)
{
    size_t len = 28;
    PosWriteFile(1, "ResumeThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RtlCaptureContext(void)
{
    size_t len = 33;
    PosWriteFile(1, "RtlCaptureContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RtlUnwind(void)
{
    size_t len = 25;
    PosWriteFile(1, "RtlUnwind unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScrollConsoleScreenBufferA(void)
{
    size_t len = 42;
    PosWriteFile(1, "ScrollConsoleScreenBufferA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCommBreak(void)
{
    size_t len = 28;
    PosWriteFile(1, "SetCommBreak unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCommMask(void)
{
    size_t len = 27;
    PosWriteFile(1, "SetCommMask unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCommState(void)
{
    size_t len = 28;
    PosWriteFile(1, "SetCommState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCommTimeouts(void)
{
    size_t len = 31;
    PosWriteFile(1, "SetCommTimeouts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetConsoleCursorInfo(void)
{
    size_t len = 36;
    PosWriteFile(1, "SetConsoleCursorInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
BOOL WINAPI SetConsoleCursorPosition(HANDLE h, COORD c)
{
    size_t len = 40;
    PosWriteFile(1, "SetConsoleCursorPosition unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
void WINAPI SetConsoleTextAttribute(void)
{
    size_t len = 39;
    PosWriteFile(1, "SetConsoleTextAttribute unimplemented\r\n", len, &len);
    for (;;) ;
}
BOOL WINAPI SetConsoleTitleW(LPCTSTR x)
{
    size_t len = 32;
    PosWriteFile(1, "SetConsoleTitleW unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
BOOL WINAPI SetConsoleTitleA(LPCTSTR x)
{
    size_t len = 32;
    PosWriteFile(1, "SetConsoleTitleA unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
BOOL WINAPI GetConsoleTitleA(LPCTSTR x, DWORD y)
{
    size_t len = 32;
    PosWriteFile(1, "GetConsoleTitleA unimplemented\r\n", len, &len);
    for (;;) ;
    return (0);
}
void WINAPI SetEnvironmentVariableW(void)
{
    size_t len = 39;
    PosWriteFile(1, "SetEnvironmentVariableW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetErrorMode(void)
{
    size_t len = 28;
    PosWriteFile(1, "SetErrorMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetEvent(void)
{
    size_t len = 24;
    PosWriteFile(1, "SetEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFilePointerEx(void)
{
    size_t len = 32;
    PosWriteFile(1, "SetFilePointerEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetHandleInformation(void)
{
    size_t len = 36;
    PosWriteFile(1, "SetHandleInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetNamedPipeHandleState(void)
{
    size_t len = 39;
    PosWriteFile(1, "SetNamedPipeHandleState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPriorityClass(void)
{
    size_t len = 32;
    PosWriteFile(1, "SetPriorityClass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetProcessWorkingSetSize(void)
{
    size_t len = 40;
    PosWriteFile(1, "SetProcessWorkingSetSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetStdHandle(void)
{
    size_t len = 28;
    PosWriteFile(1, "SetStdHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSystemTime(void)
{
    size_t len = 29;
    PosWriteFile(1, "SetSystemTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTapeParameters(void)
{
    size_t len = 33;
    PosWriteFile(1, "SetTapeParameters unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTapePosition(void)
{
    size_t len = 31;
    PosWriteFile(1, "SetTapePosition unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadAffinityMask(void)
{
    size_t len = 37;
    PosWriteFile(1, "SetThreadAffinityMask unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadContext(void)
{
    size_t len = 32;
    PosWriteFile(1, "SetThreadContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetThreadPriority(void)
{
    size_t len = 33;
    PosWriteFile(1, "SetThreadPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Sleep(void)
{
    size_t len = 21;
    PosWriteFile(1, "Sleep unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SuspendThread(void)
{
    size_t len = 29;
    PosWriteFile(1, "SuspendThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwitchToThread(void)
{
    size_t len = 30;
    PosWriteFile(1, "SwitchToThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TerminateProcess(void)
{
    size_t len = 32;
    PosWriteFile(1, "TerminateProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TerminateThread(void)
{
    size_t len = 31;
    PosWriteFile(1, "TerminateThread unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TlsAlloc(void)
{
    size_t len = 24;
    PosWriteFile(1, "TlsAlloc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TlsFree(void)
{
    size_t len = 23;
    PosWriteFile(1, "TlsFree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TlsGetValue(void)
{
    size_t len = 27;
    PosWriteFile(1, "TlsGetValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TlsSetValue(void)
{
    size_t len = 27;
    PosWriteFile(1, "TlsSetValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TransmitCommChar(void)
{
    size_t len = 32;
    PosWriteFile(1, "TransmitCommChar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnmapViewOfFile(void)
{
    size_t len = 31;
    PosWriteFile(1, "UnmapViewOfFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualAlloc(void)
{
    size_t len = 28;
    PosWriteFile(1, "VirtualAlloc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualFree(void)
{
    size_t len = 27;
    PosWriteFile(1, "VirtualFree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualProtect(void)
{
    size_t len = 30;
    PosWriteFile(1, "VirtualProtect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualProtectEx(void)
{
    size_t len = 32;
    PosWriteFile(1, "VirtualProtectEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualQuery(void)
{
    size_t len = 28;
    PosWriteFile(1, "VirtualQuery unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI VirtualQueryEx(void)
{
    size_t len = 30;
    PosWriteFile(1, "VirtualQueryEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitCommEvent(void)
{
    size_t len = 29;
    PosWriteFile(1, "WaitCommEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitForMultipleObjects(void)
{
    size_t len = 38;
    PosWriteFile(1, "WaitForMultipleObjects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WaitNamedPipeW(void)
{
    size_t len = 30;
    PosWriteFile(1, "WaitNamedPipeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WideCharToMultiByte(void)
{
    size_t len = 35;
    PosWriteFile(1, "WideCharToMultiByte unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WriteConsoleOutputW(void)
{
    size_t len = 35;
    PosWriteFile(1, "WriteConsoleOutputW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WriteConsoleW(void)
{
    size_t len = 29;
    PosWriteFile(1, "WriteConsoleW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WriteProcessMemory(void)
{
    size_t len = 34;
    PosWriteFile(1, "WriteProcessMemory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WriteTapemark(void)
{
    size_t len = 29;
    PosWriteFile(1, "WriteTapemark unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CompareStringA(void)
{
    size_t len = 30;
    PosWriteFile(1, "CompareStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FileTimeToLocalFileTime(void)
{
    size_t len = 39;
    PosWriteFile(1, "FileTimeToLocalFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FileTimeToSystemTime(void)
{
    size_t len = 36;
    PosWriteFile(1, "FileTimeToSystemTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FormatMessageA(void)
{
    size_t len = 30;
    PosWriteFile(1, "FormatMessageA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FreeEnvironmentStringsA(void)
{
    size_t len = 39;
    PosWriteFile(1, "FreeEnvironmentStringsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetACP(void)
{
    size_t len = 22;
    PosWriteFile(1, "GetACP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentDirectoryA(void)
{
    size_t len = 36;
    PosWriteFile(1, "GetCurrentDirectoryA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDriveTypeA(void)
{
    size_t len = 29;
    PosWriteFile(1, "GetDriveTypeA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFullPathNameA(void)
{
    size_t len = 32;
    PosWriteFile(1, "GetFullPathNameA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLocalTime(void)
{
    size_t len = 28;
    PosWriteFile(1, "GetLocalTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOEMCP(void)
{
    size_t len = 24;
    PosWriteFile(1, "GetOEMCP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStringTypeA(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetStringTypeA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStringTypeW(void)
{
    size_t len = 30;
    PosWriteFile(1, "GetStringTypeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HeapAlloc(void)
{
    size_t len = 25;
    PosWriteFile(1, "HeapAlloc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HeapCreate(void)
{
    size_t len = 26;
    PosWriteFile(1, "HeapCreate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HeapDestroy(void)
{
    size_t len = 27;
    PosWriteFile(1, "HeapDestroy unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HeapFree(void)
{
    size_t len = 24;
    PosWriteFile(1, "HeapFree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HeapReAlloc(void)
{
    size_t len = 27;
    PosWriteFile(1, "HeapReAlloc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LCMapStringA(void)
{
    size_t len = 28;
    PosWriteFile(1, "LCMapStringA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LocalFileTimeToFileTime(void)
{
    size_t len = 39;
    PosWriteFile(1, "LocalFileTimeToFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetCurrentDirectoryA(void)
{
    size_t len = 36;
    PosWriteFile(1, "SetCurrentDirectoryA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetEndOfFile(void)
{
    size_t len = 28;
    PosWriteFile(1, "SetEndOfFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetEnvironmentVariableA(void)
{
    size_t len = 39;
    PosWriteFile(1, "SetEnvironmentVariableA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFileTime(void)
{
    size_t len = 27;
    PosWriteFile(1, "SetFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetHandleCount(void)
{
    size_t len = 30;
    PosWriteFile(1, "SetHandleCount unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SystemTimeToFileTime(void)
{
    size_t len = 36;
    PosWriteFile(1, "SystemTimeToFileTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnhandledExceptionFilter(void)
{
    size_t len = 40;
    PosWriteFile(1, "UnhandledExceptionFilter unimplemented\r\n", len, &len);
    for (;;) ;
}

/* end of auto-genned functions */
