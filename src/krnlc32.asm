; support routines
; written by Paul Edwards
; released to the public domain

.386p
.model flat

.code



extrn __imp__GetStdHandle@4:ptr
public _GetStdHandle
_GetStdHandle:
push 4[esp]
call [__imp__GetStdHandle@4]
ret
ret

extrn __imp__WriteFile@20:ptr
public _WriteFile
_WriteFile:
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
call [__imp__WriteFile@20]
ret
ret

extrn __imp__ExitProcess@4:ptr
public _ExitProcess
_ExitProcess:
push 4[esp]
call [__imp__ExitProcess@4]
ret
ret

extrn __imp__CloseHandle@4:ptr
public _CloseHandle
_CloseHandle:
push 4[esp]
call [__imp__CloseHandle@4]
ret
ret

extrn __imp__CreateFileA@28:ptr
public _CreateFileA
_CreateFileA:
push 28[esp]
push 28[esp]
push 28[esp]
push 28[esp]
push 28[esp]
push 28[esp]
push 28[esp]
call [__imp__CreateFileA@28]
ret
ret

extrn __imp__CreateDirectoryA@8:ptr
public _CreateDirectoryA
_CreateDirectoryA:
push 8[esp]
push 8[esp]
call [__imp__CreateDirectoryA@8]
ret
ret

extrn __imp__GetFileAttributesA@4:ptr
public _GetFileAttributesA
_GetFileAttributesA:
push 4[esp]
call [__imp__GetFileAttributesA@4]
ret
ret

extrn __imp__PathFileExistsA@4:ptr
public _PathFileExistsA
_PathFileExistsA:
push 4[esp]
call [__imp__PathFileExistsA@4]
ret
ret

extrn __imp__CreateProcessA@40:ptr
public _CreateProcessA
_CreateProcessA:
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
push 40[esp]
call [__imp__CreateProcessA@40]
ret
ret

extrn __imp__DeleteFileA@4:ptr
public _DeleteFileA
_DeleteFileA:
push 4[esp]
call [__imp__DeleteFileA@4]
ret
ret

extrn __imp__GetCommandLineA@0:ptr
public _GetCommandLineA
_GetCommandLineA:
call [__imp__GetCommandLineA@0]
ret
ret

extrn __imp__GetEnvironmentStrings@0:ptr
public _GetEnvironmentStrings
_GetEnvironmentStrings:
call [__imp__GetEnvironmentStrings@0]
ret
ret

extrn __imp__GetExitCodeProcess@8:ptr
public _GetExitCodeProcess
_GetExitCodeProcess:
push 8[esp]
push 8[esp]
call [__imp__GetExitCodeProcess@8]
ret
ret

extrn __imp__GetLastError@0:ptr
public _GetLastError
_GetLastError:
call [__imp__GetLastError@0]
ret
ret

extrn __imp__GetVersion@0:ptr
public _GetVersion
_GetVersion:
call [__imp__GetVersion@0]
ret
ret

extrn __imp__GlobalAlloc@8:ptr
public _GlobalAlloc
_GlobalAlloc:
push 8[esp]
push 8[esp]
call [__imp__GlobalAlloc@8]
ret
ret

extrn __imp__GlobalFree@4:ptr
public _GlobalFree
_GlobalFree:
push 4[esp]
call [__imp__GlobalFree@4]
ret
ret

extrn __imp__MoveFileA@8:ptr
public _MoveFileA
_MoveFileA:
push 8[esp]
push 8[esp]
call [__imp__MoveFileA@8]
ret
ret

extrn __imp__ReadFile@20:ptr
public _ReadFile
_ReadFile:
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
call [__imp__ReadFile@20]
ret
ret

extrn __imp__SetFilePointer@16:ptr
public _SetFilePointer
_SetFilePointer:
push 16[esp]
push 16[esp]
push 16[esp]
push 16[esp]
call [__imp__SetFilePointer@16]
ret
ret

extrn __imp__WaitForSingleObject@8:ptr
public _WaitForSingleObject
_WaitForSingleObject:
push 8[esp]
push 8[esp]
call [__imp__WaitForSingleObject@8]
ret
ret

extrn __imp__SetConsoleCtrlHandler@8:ptr
public _SetConsoleCtrlHandler
_SetConsoleCtrlHandler:
push 8[esp]
push 8[esp]
call [__imp__SetConsoleCtrlHandler@8]
ret
ret

extrn __imp__GetSystemTime@4:ptr
public _GetSystemTime
_GetSystemTime:
push 4[esp]
call [__imp__GetSystemTime@4]
ret
ret

extrn __imp__SetLastError@4:ptr
public _SetLastError
_SetLastError:
push 4[esp]
call [__imp__SetLastError@4]
ret
ret

extrn __imp__GetConsoleScreenBufferInfo@8:ptr
public _GetConsoleScreenBufferInfo
_GetConsoleScreenBufferInfo:
push 8[esp]
push 8[esp]
call [__imp__GetConsoleScreenBufferInfo@8]
ret
ret

extrn __imp__WriteConsoleOutputA@20:ptr
public _WriteConsoleOutputA
_WriteConsoleOutputA:
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
call [__imp__WriteConsoleOutputA@20]
ret
ret

extrn __imp__GetConsoleMode@8:ptr
public _GetConsoleMode
_GetConsoleMode:
push 8[esp]
push 8[esp]
call [__imp__GetConsoleMode@8]
ret
ret

extrn __imp__SetConsoleMode@8:ptr
public _SetConsoleMode
_SetConsoleMode:
push 8[esp]
push 8[esp]
call [__imp__SetConsoleMode@8]
ret
ret

extrn __imp__GetNumberOfConsoleMouseButtons@4:ptr
public _GetNumberOfConsoleMouseButtons
_GetNumberOfConsoleMouseButtons:
push 4[esp]
call [__imp__GetNumberOfConsoleMouseButtons@4]
ret
ret

extrn __imp__FindFirstFileA@8:ptr
public _FindFirstFileA
_FindFirstFileA:
push 8[esp]
push 8[esp]
call [__imp__FindFirstFileA@8]
ret
ret

extrn __imp__FindNextFileA@8:ptr
public _FindNextFileA
_FindNextFileA:
push 8[esp]
push 8[esp]
call [__imp__FindNextFileA@8]
ret
ret

extrn __imp__FindClose@4:ptr
public _FindClose
_FindClose:
push 4[esp]
call [__imp__FindClose@4]
ret
ret

extrn __imp__GetCurrentDirectoryA@8:ptr
public _GetCurrentDirectoryA
_GetCurrentDirectoryA:
push 8[esp]
push 8[esp]
call [__imp__GetCurrentDirectoryA@8]
ret
ret

extrn __imp__AllocConsole@0:ptr
public _AllocConsole
_AllocConsole:
call [__imp__AllocConsole@0]
ret
ret

extrn __imp__AttachConsole@0:ptr
public _AttachConsole
_AttachConsole:
call [__imp__AttachConsole@0]
ret
ret

extrn __imp__CallNamedPipeA@0:ptr
public _CallNamedPipeA
_CallNamedPipeA:
call [__imp__CallNamedPipeA@0]
ret
ret

extrn __imp__CancelIo@0:ptr
public _CancelIo
_CancelIo:
call [__imp__CancelIo@0]
ret
ret

extrn __imp__ClearCommBreak@0:ptr
public _ClearCommBreak
_ClearCommBreak:
call [__imp__ClearCommBreak@0]
ret
ret

extrn __imp__ClearCommError@0:ptr
public _ClearCommError
_ClearCommError:
call [__imp__ClearCommError@0]
ret
ret

extrn __imp__CompareFileTime@0:ptr
public _CompareFileTime
_CompareFileTime:
call [__imp__CompareFileTime@0]
ret
ret

extrn __imp__CompareStringW@0:ptr
public _CompareStringW
_CompareStringW:
call [__imp__CompareStringW@0]
ret
ret

extrn __imp__ConnectNamedPipe@0:ptr
public _ConnectNamedPipe
_ConnectNamedPipe:
call [__imp__ConnectNamedPipe@0]
ret
ret

extrn __imp__CreateFileW@0:ptr
public _CreateFileW
_CreateFileW:
call [__imp__CreateFileW@0]
ret
ret

extrn __imp__CreateNamedPipeA@0:ptr
public _CreateNamedPipeA
_CreateNamedPipeA:
call [__imp__CreateNamedPipeA@0]
ret
ret

extrn __imp__CreatePipe@0:ptr
public _CreatePipe
_CreatePipe:
call [__imp__CreatePipe@0]
ret
ret

extrn __imp__CreateProcessW@0:ptr
public _CreateProcessW
_CreateProcessW:
call [__imp__CreateProcessW@0]
ret
ret

extrn __imp__CreateRemoteThread@0:ptr
public _CreateRemoteThread
_CreateRemoteThread:
call [__imp__CreateRemoteThread@0]
ret
ret

extrn __imp__CreateTapePartition@0:ptr
public _CreateTapePartition
_CreateTapePartition:
call [__imp__CreateTapePartition@0]
ret
ret

extrn __imp__CreateThread@0:ptr
public _CreateThread
_CreateThread:
call [__imp__CreateThread@0]
ret
ret

extrn __imp__DebugBreak@0:ptr
public _DebugBreak
_DebugBreak:
call [__imp__DebugBreak@0]
ret
ret

extrn __imp__DeleteCriticalSection@0:ptr
public _DeleteCriticalSection
_DeleteCriticalSection:
call [__imp__DeleteCriticalSection@0]
ret
ret

extrn __imp__DeviceIoControl@0:ptr
public _DeviceIoControl
_DeviceIoControl:
call [__imp__DeviceIoControl@0]
ret
ret

extrn __imp__DisconnectNamedPipe@0:ptr
public _DisconnectNamedPipe
_DisconnectNamedPipe:
call [__imp__DisconnectNamedPipe@0]
ret
ret

extrn __imp__DuplicateHandle@0:ptr
public _DuplicateHandle
_DuplicateHandle:
call [__imp__DuplicateHandle@0]
ret
ret

extrn __imp__EnterCriticalSection@0:ptr
public _EnterCriticalSection
_EnterCriticalSection:
call [__imp__EnterCriticalSection@0]
ret
ret

extrn __imp__EraseTape@0:ptr
public _EraseTape
_EraseTape:
call [__imp__EraseTape@0]
ret
ret

extrn __imp__EscapeCommFunction@0:ptr
public _EscapeCommFunction
_EscapeCommFunction:
call [__imp__EscapeCommFunction@0]
ret
ret

extrn __imp__ExitThread@0:ptr
public _ExitThread
_ExitThread:
call [__imp__ExitThread@0]
ret
ret

extrn __imp__ExpandEnvironmentStringsW@0:ptr
public _ExpandEnvironmentStringsW
_ExpandEnvironmentStringsW:
call [__imp__ExpandEnvironmentStringsW@0]
ret
ret

extrn __imp__FillConsoleOutputAttribute@20:ptr
public _FillConsoleOutputAttribute
_FillConsoleOutputAttribute:
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
call [__imp__FillConsoleOutputAttribute@20]
ret
ret

extrn __imp__FillConsoleOutputCharacterA@20:ptr
public _FillConsoleOutputCharacterA
_FillConsoleOutputCharacterA:
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
push 20[esp]
call [__imp__FillConsoleOutputCharacterA@20]
ret
ret

extrn __imp__FindFirstVolumeW@0:ptr
public _FindFirstVolumeW
_FindFirstVolumeW:
call [__imp__FindFirstVolumeW@0]
ret
ret

extrn __imp__FindNextVolumeW@0:ptr
public _FindNextVolumeW
_FindNextVolumeW:
call [__imp__FindNextVolumeW@0]
ret
ret

extrn __imp__FindVolumeClose@0:ptr
public _FindVolumeClose
_FindVolumeClose:
call [__imp__FindVolumeClose@0]
ret
ret

extrn __imp__FlushConsoleInputBuffer@0:ptr
public _FlushConsoleInputBuffer
_FlushConsoleInputBuffer:
call [__imp__FlushConsoleInputBuffer@0]
ret
ret

extrn __imp__FlushFileBuffers@0:ptr
public _FlushFileBuffers
_FlushFileBuffers:
call [__imp__FlushFileBuffers@0]
ret
ret

extrn __imp__FlushViewOfFile@0:ptr
public _FlushViewOfFile
_FlushViewOfFile:
call [__imp__FlushViewOfFile@0]
ret
ret

extrn __imp__FreeConsole@0:ptr
public _FreeConsole
_FreeConsole:
call [__imp__FreeConsole@0]
ret
ret

extrn __imp__FreeEnvironmentStringsW@0:ptr
public _FreeEnvironmentStringsW
_FreeEnvironmentStringsW:
call [__imp__FreeEnvironmentStringsW@0]
ret
ret

extrn __imp__FreeLibrary@0:ptr
public _FreeLibrary
_FreeLibrary:
call [__imp__FreeLibrary@0]
ret
ret

extrn __imp__GetBinaryTypeW@0:ptr
public _GetBinaryTypeW
_GetBinaryTypeW:
call [__imp__GetBinaryTypeW@0]
ret
ret

extrn __imp__GetCPInfo@0:ptr
public _GetCPInfo
_GetCPInfo:
call [__imp__GetCPInfo@0]
ret
ret

extrn __imp__GetCommModemStatus@0:ptr
public _GetCommModemStatus
_GetCommModemStatus:
call [__imp__GetCommModemStatus@0]
ret
ret

extrn __imp__GetCommState@0:ptr
public _GetCommState
_GetCommState:
call [__imp__GetCommState@0]
ret
ret

extrn __imp__GetCommandLineW@0:ptr
public _GetCommandLineW
_GetCommandLineW:
call [__imp__GetCommandLineW@0]
ret
ret

extrn __imp__GetComputerNameA@0:ptr
public _GetComputerNameA
_GetComputerNameA:
call [__imp__GetComputerNameA@0]
ret
ret

extrn __imp__GetComputerNameW@0:ptr
public _GetComputerNameW
_GetComputerNameW:
call [__imp__GetComputerNameW@0]
ret
ret

extrn __imp__GetConsoleCP@0:ptr
public _GetConsoleCP
_GetConsoleCP:
call [__imp__GetConsoleCP@0]
ret
ret

extrn __imp__GetConsoleCursorInfo@0:ptr
public _GetConsoleCursorInfo
_GetConsoleCursorInfo:
call [__imp__GetConsoleCursorInfo@0]
ret
ret

extrn __imp__GetConsoleWindow@0:ptr
public _GetConsoleWindow
_GetConsoleWindow:
call [__imp__GetConsoleWindow@0]
ret
ret

extrn __imp__GetCurrentProcess@0:ptr
public _GetCurrentProcess
_GetCurrentProcess:
call [__imp__GetCurrentProcess@0]
ret
ret

extrn __imp__GetCurrentProcessId@0:ptr
public _GetCurrentProcessId
_GetCurrentProcessId:
call [__imp__GetCurrentProcessId@0]
ret
ret

extrn __imp__GetCurrentThread@0:ptr
public _GetCurrentThread
_GetCurrentThread:
call [__imp__GetCurrentThread@0]
ret
ret

extrn __imp__GetCurrentThreadId@0:ptr
public _GetCurrentThreadId
_GetCurrentThreadId:
call [__imp__GetCurrentThreadId@0]
ret
ret

extrn __imp__GetEnvironmentStringsW@0:ptr
public _GetEnvironmentStringsW
_GetEnvironmentStringsW:
call [__imp__GetEnvironmentStringsW@0]
ret
ret

extrn __imp__GetEnvironmentVariableA@0:ptr
public _GetEnvironmentVariableA
_GetEnvironmentVariableA:
call [__imp__GetEnvironmentVariableA@0]
ret
ret

extrn __imp__GetEnvironmentVariableW@0:ptr
public _GetEnvironmentVariableW
_GetEnvironmentVariableW:
call [__imp__GetEnvironmentVariableW@0]
ret
ret

extrn __imp__GetExitCodeThread@0:ptr
public _GetExitCodeThread
_GetExitCodeThread:
call [__imp__GetExitCodeThread@0]
ret
ret

extrn __imp__GetFileSize@0:ptr
public _GetFileSize
_GetFileSize:
call [__imp__GetFileSize@0]
ret
ret

extrn __imp__GetFileSizeEx@0:ptr
public _GetFileSizeEx
_GetFileSizeEx:
call [__imp__GetFileSizeEx@0]
ret
ret

extrn __imp__GetFileType@0:ptr
public _GetFileType
_GetFileType:
call [__imp__GetFileType@0]
ret
ret

extrn __imp__GetHandleInformation@0:ptr
public _GetHandleInformation
_GetHandleInformation:
call [__imp__GetHandleInformation@0]
ret
ret

extrn __imp__GetLocaleInfoA@0:ptr
public _GetLocaleInfoA
_GetLocaleInfoA:
call [__imp__GetLocaleInfoA@0]
ret
ret

extrn __imp__GetLocaleInfoW@0:ptr
public _GetLocaleInfoW
_GetLocaleInfoW:
call [__imp__GetLocaleInfoW@0]
ret
ret

extrn __imp__GetLogicalDriveStringsA@0:ptr
public _GetLogicalDriveStringsA
_GetLogicalDriveStringsA:
call [__imp__GetLogicalDriveStringsA@0]
ret
ret

extrn __imp__GetLogicalDrives@0:ptr
public _GetLogicalDrives
_GetLogicalDrives:
call [__imp__GetLogicalDrives@0]
ret
ret

extrn __imp__GetMailslotInfo@0:ptr
public _GetMailslotInfo
_GetMailslotInfo:
call [__imp__GetMailslotInfo@0]
ret
ret

extrn __imp__GetModuleFileNameA@0:ptr
public _GetModuleFileNameA
_GetModuleFileNameA:
call [__imp__GetModuleFileNameA@0]
ret
ret

extrn __imp__GetModuleFileNameW@0:ptr
public _GetModuleFileNameW
_GetModuleFileNameW:
call [__imp__GetModuleFileNameW@0]
ret
ret

extrn __imp__GetModuleHandleA@0:ptr
public _GetModuleHandleA
_GetModuleHandleA:
call [__imp__GetModuleHandleA@0]
ret
ret

extrn __imp__GetModuleHandleExW@0:ptr
public _GetModuleHandleExW
_GetModuleHandleExW:
call [__imp__GetModuleHandleExW@0]
ret
ret

extrn __imp__GetModuleHandleW@0:ptr
public _GetModuleHandleW
_GetModuleHandleW:
call [__imp__GetModuleHandleW@0]
ret
ret

extrn __imp__GetNumberOfConsoleInputEvents@0:ptr
public _GetNumberOfConsoleInputEvents
_GetNumberOfConsoleInputEvents:
call [__imp__GetNumberOfConsoleInputEvents@0]
ret
ret

extrn __imp__GetOverlappedResult@0:ptr
public _GetOverlappedResult
_GetOverlappedResult:
call [__imp__GetOverlappedResult@0]
ret
ret

extrn __imp__GetPriorityClass@0:ptr
public _GetPriorityClass
_GetPriorityClass:
call [__imp__GetPriorityClass@0]
ret
ret

extrn __imp__GetProcAddress@0:ptr
public _GetProcAddress
_GetProcAddress:
call [__imp__GetProcAddress@0]
ret
ret

extrn __imp__GetProcessWorkingSetSize@0:ptr
public _GetProcessWorkingSetSize
_GetProcessWorkingSetSize:
call [__imp__GetProcessWorkingSetSize@0]
ret
ret

extrn __imp__GetStartupInfoA@0:ptr
public _GetStartupInfoA
_GetStartupInfoA:
call [__imp__GetStartupInfoA@0]
ret
ret

extrn __imp__GetStartupInfoW@0:ptr
public _GetStartupInfoW
_GetStartupInfoW:
call [__imp__GetStartupInfoW@0]
ret
ret

extrn __imp__GetSystemDirectoryW@0:ptr
public _GetSystemDirectoryW
_GetSystemDirectoryW:
call [__imp__GetSystemDirectoryW@0]
ret
ret

extrn __imp__GetSystemInfo@0:ptr
public _GetSystemInfo
_GetSystemInfo:
call [__imp__GetSystemInfo@0]
ret
ret

extrn __imp__GetSystemTimeAsFileTime@0:ptr
public _GetSystemTimeAsFileTime
_GetSystemTimeAsFileTime:
call [__imp__GetSystemTimeAsFileTime@0]
ret
ret

extrn __imp__GetSystemWindowsDirectoryW@0:ptr
public _GetSystemWindowsDirectoryW
_GetSystemWindowsDirectoryW:
call [__imp__GetSystemWindowsDirectoryW@0]
ret
ret

extrn __imp__GetSystemWow64DirectoryW@0:ptr
public _GetSystemWow64DirectoryW
_GetSystemWow64DirectoryW:
call [__imp__GetSystemWow64DirectoryW@0]
ret
ret

extrn __imp__GetTapeParameters@0:ptr
public _GetTapeParameters
_GetTapeParameters:
call [__imp__GetTapeParameters@0]
ret
ret

extrn __imp__GetTapePosition@0:ptr
public _GetTapePosition
_GetTapePosition:
call [__imp__GetTapePosition@0]
ret
ret

extrn __imp__GetTapeStatus@0:ptr
public _GetTapeStatus
_GetTapeStatus:
call [__imp__GetTapeStatus@0]
ret
ret

extrn __imp__GetThreadContext@0:ptr
public _GetThreadContext
_GetThreadContext:
call [__imp__GetThreadContext@0]
ret
ret

extrn __imp__GetThreadPriority@0:ptr
public _GetThreadPriority
_GetThreadPriority:
call [__imp__GetThreadPriority@0]
ret
ret

extrn __imp__GetTickCount@0:ptr
public _GetTickCount
_GetTickCount:
call [__imp__GetTickCount@0]
ret
ret

extrn __imp__GetTimeZoneInformation@0:ptr
public _GetTimeZoneInformation
_GetTimeZoneInformation:
call [__imp__GetTimeZoneInformation@0]
ret
ret

extrn __imp__GetVersionExA@0:ptr
public _GetVersionExA
_GetVersionExA:
call [__imp__GetVersionExA@0]
ret
ret

extrn __imp__GetVolumePathNamesForVolumeNameW@0:ptr
public _GetVolumePathNamesForVolumeNameW
_GetVolumePathNamesForVolumeNameW:
call [__imp__GetVolumePathNamesForVolumeNameW@0]
ret
ret

extrn __imp__GlobalLock@0:ptr
public _GlobalLock
_GlobalLock:
call [__imp__GlobalLock@0]
ret
ret

extrn __imp__GlobalMemoryStatusEx@0:ptr
public _GlobalMemoryStatusEx
_GlobalMemoryStatusEx:
call [__imp__GlobalMemoryStatusEx@0]
ret
ret

extrn __imp__GlobalSize@0:ptr
public _GlobalSize
_GlobalSize:
call [__imp__GlobalSize@0]
ret
ret

extrn __imp__GlobalUnlock@0:ptr
public _GlobalUnlock
_GlobalUnlock:
call [__imp__GlobalUnlock@0]
ret
ret

extrn __imp__InitializeCriticalSection@0:ptr
public _InitializeCriticalSection
_InitializeCriticalSection:
call [__imp__InitializeCriticalSection@0]
ret
ret

extrn __imp__IsBadStringPtrA@0:ptr
public _IsBadStringPtrA
_IsBadStringPtrA:
call [__imp__IsBadStringPtrA@0]
ret
ret

extrn __imp__IsDebuggerPresent@0:ptr
public _IsDebuggerPresent
_IsDebuggerPresent:
call [__imp__IsDebuggerPresent@0]
ret
ret

extrn __imp__IsProcessInJob@0:ptr
public _IsProcessInJob
_IsProcessInJob:
call [__imp__IsProcessInJob@0]
ret
ret

extrn __imp__IsProcessorFeaturePresent@0:ptr
public _IsProcessorFeaturePresent
_IsProcessorFeaturePresent:
call [__imp__IsProcessorFeaturePresent@0]
ret
ret

extrn __imp__LCMapStringW@0:ptr
public _LCMapStringW
_LCMapStringW:
call [__imp__LCMapStringW@0]
ret
ret

extrn __imp__LeaveCriticalSection@0:ptr
public _LeaveCriticalSection
_LeaveCriticalSection:
call [__imp__LeaveCriticalSection@0]
ret
ret

extrn __imp__LoadLibraryA@0:ptr
public _LoadLibraryA
_LoadLibraryA:
call [__imp__LoadLibraryA@0]
ret
ret

extrn __imp__LoadLibraryExA@0:ptr
public _LoadLibraryExA
_LoadLibraryExA:
call [__imp__LoadLibraryExA@0]
ret
ret

extrn __imp__LoadLibraryExW@0:ptr
public _LoadLibraryExW
_LoadLibraryExW:
call [__imp__LoadLibraryExW@0]
ret
ret

extrn __imp__LoadLibraryW@0:ptr
public _LoadLibraryW
_LoadLibraryW:
call [__imp__LoadLibraryW@0]
ret
ret

extrn __imp__LocalFree@0:ptr
public _LocalFree
_LocalFree:
call [__imp__LocalFree@0]
ret
ret

extrn __imp__MapViewOfFile@0:ptr
public _MapViewOfFile
_MapViewOfFile:
call [__imp__MapViewOfFile@0]
ret
ret

extrn __imp__MapViewOfFileEx@0:ptr
public _MapViewOfFileEx
_MapViewOfFileEx:
call [__imp__MapViewOfFileEx@0]
ret
ret

extrn __imp__MultiByteToWideChar@0:ptr
public _MultiByteToWideChar
_MultiByteToWideChar:
call [__imp__MultiByteToWideChar@0]
ret
ret

extrn __imp__OpenProcess@0:ptr
public _OpenProcess
_OpenProcess:
call [__imp__OpenProcess@0]
ret
ret

extrn __imp__OpenThread@0:ptr
public _OpenThread
_OpenThread:
call [__imp__OpenThread@0]
ret
ret

extrn __imp__OutputDebugStringA@0:ptr
public _OutputDebugStringA
_OutputDebugStringA:
call [__imp__OutputDebugStringA@0]
ret
ret

extrn __imp__PeekConsoleInputA@0:ptr
public _PeekConsoleInputA
_PeekConsoleInputA:
call [__imp__PeekConsoleInputA@0]
ret
ret

extrn __imp__PeekConsoleInputW@0:ptr
public _PeekConsoleInputW
_PeekConsoleInputW:
call [__imp__PeekConsoleInputW@0]
ret
ret

extrn __imp__PeekNamedPipe@0:ptr
public _PeekNamedPipe
_PeekNamedPipe:
call [__imp__PeekNamedPipe@0]
ret
ret

extrn __imp__PrepareTape@0:ptr
public _PrepareTape
_PrepareTape:
call [__imp__PrepareTape@0]
ret
ret

extrn __imp__PurgeComm@0:ptr
public _PurgeComm
_PurgeComm:
call [__imp__PurgeComm@0]
ret
ret

extrn __imp__QueryDosDeviceW@0:ptr
public _QueryDosDeviceW
_QueryDosDeviceW:
call [__imp__QueryDosDeviceW@0]
ret
ret

extrn __imp__QueryInformationJobObject@0:ptr
public _QueryInformationJobObject
_QueryInformationJobObject:
call [__imp__QueryInformationJobObject@0]
ret
ret

extrn __imp__QueryPerformanceCounter@0:ptr
public _QueryPerformanceCounter
_QueryPerformanceCounter:
call [__imp__QueryPerformanceCounter@0]
ret
ret

extrn __imp__QueryPerformanceFrequency@0:ptr
public _QueryPerformanceFrequency
_QueryPerformanceFrequency:
call [__imp__QueryPerformanceFrequency@0]
ret
ret

extrn __imp__QueueUserAPC@0:ptr
public _QueueUserAPC
_QueueUserAPC:
call [__imp__QueueUserAPC@0]
ret
ret

extrn __imp__ReadConsoleInputA@16:ptr
public _ReadConsoleInputA
_ReadConsoleInputA:
push 16[esp]
push 16[esp]
push 16[esp]
push 16[esp]
call [__imp__ReadConsoleInputA@16]
ret
ret

extrn __imp__ReadConsoleInputW@0:ptr
public _ReadConsoleInputW
_ReadConsoleInputW:
call [__imp__ReadConsoleInputW@0]
ret
ret

extrn __imp__ReadConsoleOutputW@0:ptr
public _ReadConsoleOutputW
_ReadConsoleOutputW:
call [__imp__ReadConsoleOutputW@0]
ret
ret

extrn __imp__ReadProcessMemory@0:ptr
public _ReadProcessMemory
_ReadProcessMemory:
call [__imp__ReadProcessMemory@0]
ret
ret

extrn __imp__ReleaseMutex@0:ptr
public _ReleaseMutex
_ReleaseMutex:
call [__imp__ReleaseMutex@0]
ret
ret

extrn __imp__ReleaseSemaphore@0:ptr
public _ReleaseSemaphore
_ReleaseSemaphore:
call [__imp__ReleaseSemaphore@0]
ret
ret

extrn __imp__ResetEvent@0:ptr
public _ResetEvent
_ResetEvent:
call [__imp__ResetEvent@0]
ret
ret

extrn __imp__ResumeThread@0:ptr
public _ResumeThread
_ResumeThread:
call [__imp__ResumeThread@0]
ret
ret

extrn __imp__RtlCaptureContext@0:ptr
public _RtlCaptureContext
_RtlCaptureContext:
call [__imp__RtlCaptureContext@0]
ret
ret

extrn __imp__RtlUnwind@0:ptr
public _RtlUnwind
_RtlUnwind:
call [__imp__RtlUnwind@0]
ret
ret

extrn __imp__ScrollConsoleScreenBufferA@0:ptr
public _ScrollConsoleScreenBufferA
_ScrollConsoleScreenBufferA:
call [__imp__ScrollConsoleScreenBufferA@0]
ret
ret

extrn __imp__SetCommBreak@0:ptr
public _SetCommBreak
_SetCommBreak:
call [__imp__SetCommBreak@0]
ret
ret

extrn __imp__SetCommMask@0:ptr
public _SetCommMask
_SetCommMask:
call [__imp__SetCommMask@0]
ret
ret

extrn __imp__SetCommState@0:ptr
public _SetCommState
_SetCommState:
call [__imp__SetCommState@0]
ret
ret

extrn __imp__SetCommTimeouts@0:ptr
public _SetCommTimeouts
_SetCommTimeouts:
call [__imp__SetCommTimeouts@0]
ret
ret

extrn __imp__SetConsoleCursorInfo@0:ptr
public _SetConsoleCursorInfo
_SetConsoleCursorInfo:
call [__imp__SetConsoleCursorInfo@0]
ret
ret

extrn __imp__SetConsoleCursorPosition@8:ptr
public _SetConsoleCursorPosition
_SetConsoleCursorPosition:
push 8[esp]
push 8[esp]
call [__imp__SetConsoleCursorPosition@8]
ret
ret

extrn __imp__SetConsoleTextAttribute@0:ptr
public _SetConsoleTextAttribute
_SetConsoleTextAttribute:
call [__imp__SetConsoleTextAttribute@0]
ret
ret

extrn __imp__SetConsoleTitleW@4:ptr
public _SetConsoleTitleW
_SetConsoleTitleW:
push 4[esp]
call [__imp__SetConsoleTitleW@4]
ret
ret

extrn __imp__SetConsoleTitleA@4:ptr
public _SetConsoleTitleA
_SetConsoleTitleA:
push 4[esp]
call [__imp__SetConsoleTitleA@4]
ret
ret

extrn __imp__GetConsoleTitleA@8:ptr
public _GetConsoleTitleA
_GetConsoleTitleA:
push 8[esp]
push 8[esp]
call [__imp__GetConsoleTitleA@8]
ret
ret

extrn __imp__SetEnvironmentVariableW@0:ptr
public _SetEnvironmentVariableW
_SetEnvironmentVariableW:
call [__imp__SetEnvironmentVariableW@0]
ret
ret

extrn __imp__SetErrorMode@0:ptr
public _SetErrorMode
_SetErrorMode:
call [__imp__SetErrorMode@0]
ret
ret

extrn __imp__SetEvent@0:ptr
public _SetEvent
_SetEvent:
call [__imp__SetEvent@0]
ret
ret

extrn __imp__SetFilePointerEx@0:ptr
public _SetFilePointerEx
_SetFilePointerEx:
call [__imp__SetFilePointerEx@0]
ret
ret

extrn __imp__SetHandleInformation@0:ptr
public _SetHandleInformation
_SetHandleInformation:
call [__imp__SetHandleInformation@0]
ret
ret

extrn __imp__SetNamedPipeHandleState@0:ptr
public _SetNamedPipeHandleState
_SetNamedPipeHandleState:
call [__imp__SetNamedPipeHandleState@0]
ret
ret

extrn __imp__SetPriorityClass@0:ptr
public _SetPriorityClass
_SetPriorityClass:
call [__imp__SetPriorityClass@0]
ret
ret

extrn __imp__SetProcessWorkingSetSize@0:ptr
public _SetProcessWorkingSetSize
_SetProcessWorkingSetSize:
call [__imp__SetProcessWorkingSetSize@0]
ret
ret

extrn __imp__SetStdHandle@8:ptr
public _SetStdHandle
_SetStdHandle:
push 8[esp]
push 8[esp]
call [__imp__SetStdHandle@8]
ret
ret

extrn __imp__SetSystemTime@0:ptr
public _SetSystemTime
_SetSystemTime:
call [__imp__SetSystemTime@0]
ret
ret

extrn __imp__SetTapeParameters@0:ptr
public _SetTapeParameters
_SetTapeParameters:
call [__imp__SetTapeParameters@0]
ret
ret

extrn __imp__SetTapePosition@0:ptr
public _SetTapePosition
_SetTapePosition:
call [__imp__SetTapePosition@0]
ret
ret

extrn __imp__SetThreadAffinityMask@0:ptr
public _SetThreadAffinityMask
_SetThreadAffinityMask:
call [__imp__SetThreadAffinityMask@0]
ret
ret

extrn __imp__SetThreadContext@0:ptr
public _SetThreadContext
_SetThreadContext:
call [__imp__SetThreadContext@0]
ret
ret

extrn __imp__SetThreadPriority@0:ptr
public _SetThreadPriority
_SetThreadPriority:
call [__imp__SetThreadPriority@0]
ret
ret

extrn __imp__Sleep@0:ptr
public _Sleep
_Sleep:
call [__imp__Sleep@0]
ret
ret

extrn __imp__SuspendThread@0:ptr
public _SuspendThread
_SuspendThread:
call [__imp__SuspendThread@0]
ret
ret

extrn __imp__SwitchToThread@0:ptr
public _SwitchToThread
_SwitchToThread:
call [__imp__SwitchToThread@0]
ret
ret

extrn __imp__TerminateProcess@0:ptr
public _TerminateProcess
_TerminateProcess:
call [__imp__TerminateProcess@0]
ret
ret

extrn __imp__TerminateThread@0:ptr
public _TerminateThread
_TerminateThread:
call [__imp__TerminateThread@0]
ret
ret

extrn __imp__TlsAlloc@0:ptr
public _TlsAlloc
_TlsAlloc:
call [__imp__TlsAlloc@0]
ret
ret

extrn __imp__TlsFree@0:ptr
public _TlsFree
_TlsFree:
call [__imp__TlsFree@0]
ret
ret

extrn __imp__TlsGetValue@0:ptr
public _TlsGetValue
_TlsGetValue:
call [__imp__TlsGetValue@0]
ret
ret

extrn __imp__TlsSetValue@0:ptr
public _TlsSetValue
_TlsSetValue:
call [__imp__TlsSetValue@0]
ret
ret

extrn __imp__TransmitCommChar@0:ptr
public _TransmitCommChar
_TransmitCommChar:
call [__imp__TransmitCommChar@0]
ret
ret

extrn __imp__UnmapViewOfFile@0:ptr
public _UnmapViewOfFile
_UnmapViewOfFile:
call [__imp__UnmapViewOfFile@0]
ret
ret

extrn __imp__VirtualAlloc@0:ptr
public _VirtualAlloc
_VirtualAlloc:
call [__imp__VirtualAlloc@0]
ret
ret

extrn __imp__VirtualFree@0:ptr
public _VirtualFree
_VirtualFree:
call [__imp__VirtualFree@0]
ret
ret

extrn __imp__VirtualProtect@0:ptr
public _VirtualProtect
_VirtualProtect:
call [__imp__VirtualProtect@0]
ret
ret

extrn __imp__VirtualProtectEx@0:ptr
public _VirtualProtectEx
_VirtualProtectEx:
call [__imp__VirtualProtectEx@0]
ret
ret

extrn __imp__VirtualQuery@0:ptr
public _VirtualQuery
_VirtualQuery:
call [__imp__VirtualQuery@0]
ret
ret

extrn __imp__VirtualQueryEx@0:ptr
public _VirtualQueryEx
_VirtualQueryEx:
call [__imp__VirtualQueryEx@0]
ret
ret

extrn __imp__WaitCommEvent@0:ptr
public _WaitCommEvent
_WaitCommEvent:
call [__imp__WaitCommEvent@0]
ret
ret

extrn __imp__WaitForMultipleObjects@0:ptr
public _WaitForMultipleObjects
_WaitForMultipleObjects:
call [__imp__WaitForMultipleObjects@0]
ret
ret

extrn __imp__WaitNamedPipeW@0:ptr
public _WaitNamedPipeW
_WaitNamedPipeW:
call [__imp__WaitNamedPipeW@0]
ret
ret

extrn __imp__WideCharToMultiByte@0:ptr
public _WideCharToMultiByte
_WideCharToMultiByte:
call [__imp__WideCharToMultiByte@0]
ret
ret

extrn __imp__WriteConsoleOutputW@0:ptr
public _WriteConsoleOutputW
_WriteConsoleOutputW:
call [__imp__WriteConsoleOutputW@0]
ret
ret

extrn __imp__WriteConsoleW@0:ptr
public _WriteConsoleW
_WriteConsoleW:
call [__imp__WriteConsoleW@0]
ret
ret

extrn __imp__WriteProcessMemory@0:ptr
public _WriteProcessMemory
_WriteProcessMemory:
call [__imp__WriteProcessMemory@0]
ret
ret

extrn __imp__WriteTapemark@0:ptr
public _WriteTapemark
_WriteTapemark:
call [__imp__WriteTapemark@0]
ret
ret

extrn __imp__CompareStringA@0:ptr
public _CompareStringA
_CompareStringA:
call [__imp__CompareStringA@0]
ret
ret

extrn __imp__FileTimeToLocalFileTime@0:ptr
public _FileTimeToLocalFileTime
_FileTimeToLocalFileTime:
call [__imp__FileTimeToLocalFileTime@0]
ret
ret

extrn __imp__FileTimeToSystemTime@0:ptr
public _FileTimeToSystemTime
_FileTimeToSystemTime:
call [__imp__FileTimeToSystemTime@0]
ret
ret

extrn __imp__FormatMessageA@0:ptr
public _FormatMessageA
_FormatMessageA:
call [__imp__FormatMessageA@0]
ret
ret

extrn __imp__FreeEnvironmentStringsA@0:ptr
public _FreeEnvironmentStringsA
_FreeEnvironmentStringsA:
call [__imp__FreeEnvironmentStringsA@0]
ret
ret

extrn __imp__GetACP@0:ptr
public _GetACP
_GetACP:
call [__imp__GetACP@0]
ret
ret

extrn __imp__GetDriveTypeA@0:ptr
public _GetDriveTypeA
_GetDriveTypeA:
call [__imp__GetDriveTypeA@0]
ret
ret

extrn __imp__GetFullPathNameA@0:ptr
public _GetFullPathNameA
_GetFullPathNameA:
call [__imp__GetFullPathNameA@0]
ret
ret

extrn __imp__GetLocalTime@0:ptr
public _GetLocalTime
_GetLocalTime:
call [__imp__GetLocalTime@0]
ret
ret

extrn __imp__GetOEMCP@0:ptr
public _GetOEMCP
_GetOEMCP:
call [__imp__GetOEMCP@0]
ret
ret

extrn __imp__GetStringTypeA@0:ptr
public _GetStringTypeA
_GetStringTypeA:
call [__imp__GetStringTypeA@0]
ret
ret

extrn __imp__GetStringTypeW@0:ptr
public _GetStringTypeW
_GetStringTypeW:
call [__imp__GetStringTypeW@0]
ret
ret

extrn __imp__HeapAlloc@0:ptr
public _HeapAlloc
_HeapAlloc:
call [__imp__HeapAlloc@0]
ret
ret

extrn __imp__HeapCreate@0:ptr
public _HeapCreate
_HeapCreate:
call [__imp__HeapCreate@0]
ret
ret

extrn __imp__HeapDestroy@0:ptr
public _HeapDestroy
_HeapDestroy:
call [__imp__HeapDestroy@0]
ret
ret

extrn __imp__HeapFree@0:ptr
public _HeapFree
_HeapFree:
call [__imp__HeapFree@0]
ret
ret

extrn __imp__HeapReAlloc@0:ptr
public _HeapReAlloc
_HeapReAlloc:
call [__imp__HeapReAlloc@0]
ret
ret

extrn __imp__LCMapStringA@0:ptr
public _LCMapStringA
_LCMapStringA:
call [__imp__LCMapStringA@0]
ret
ret

extrn __imp__LocalFileTimeToFileTime@0:ptr
public _LocalFileTimeToFileTime
_LocalFileTimeToFileTime:
call [__imp__LocalFileTimeToFileTime@0]
ret
ret

extrn __imp__SetCurrentDirectoryA@0:ptr
public _SetCurrentDirectoryA
_SetCurrentDirectoryA:
call [__imp__SetCurrentDirectoryA@0]
ret
ret

extrn __imp__SetEndOfFile@0:ptr
public _SetEndOfFile
_SetEndOfFile:
call [__imp__SetEndOfFile@0]
ret
ret

extrn __imp__SetEnvironmentVariableA@0:ptr
public _SetEnvironmentVariableA
_SetEnvironmentVariableA:
call [__imp__SetEnvironmentVariableA@0]
ret
ret

extrn __imp__SetFileTime@0:ptr
public _SetFileTime
_SetFileTime:
call [__imp__SetFileTime@0]
ret
ret

extrn __imp__SetHandleCount@0:ptr
public _SetHandleCount
_SetHandleCount:
call [__imp__SetHandleCount@0]
ret
ret

extrn __imp__SystemTimeToFileTime@0:ptr
public _SystemTimeToFileTime
_SystemTimeToFileTime:
call [__imp__SystemTimeToFileTime@0]
ret
ret

extrn __imp__UnhandledExceptionFilter@0:ptr
public _UnhandledExceptionFilter
_UnhandledExceptionFilter:
call [__imp__UnhandledExceptionFilter@0]
ret
ret




end
