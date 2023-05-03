; winsupa.asm - support code for C programs for Win32
;
; This program written by Paul Edwards
; Released to the public domain

.386
.model flat

.code

extrn _GetStdHandle@4:proc
public GetStdHandle
GetStdHandle: jmp _GetStdHandle@4

extrn _WriteFile@20:proc
public WriteFile
WriteFile: jmp _WriteFile@20

extrn _ExitProcess@4:proc
public ExitProcess
ExitProcess: jmp _ExitProcess@4

extrn _CloseHandle@4:proc
public CloseHandle
CloseHandle: jmp _CloseHandle@4

extrn _CreateFileA@28:proc
public CreateFileA
CreateFileA: jmp _CreateFileA@28

extrn _CreateDirectoryA@8:proc
public CreateDirectoryA
CreateDirectoryA: jmp _CreateDirectoryA@8

extrn _GetFileAttributesA@4:proc
public GetFileAttributesA
GetFileAttributesA: jmp _GetFileAttributesA@4

extrn _PathFileExistsA@4:proc
public PathFileExistsA
PathFileExistsA: jmp _PathFileExistsA@4

extrn _CreateProcessA@40:proc
public CreateProcessA
CreateProcessA: jmp _CreateProcessA@40

extrn _DeleteFileA@4:proc
public DeleteFileA
DeleteFileA: jmp _DeleteFileA@4

extrn _GetCommandLineA@0:proc
public GetCommandLineA
GetCommandLineA: jmp _GetCommandLineA@0

extrn _GetEnvironmentStrings@0:proc
public GetEnvironmentStrings
GetEnvironmentStrings: jmp _GetEnvironmentStrings@0

extrn _GetExitCodeProcess@8:proc
public GetExitCodeProcess
GetExitCodeProcess: jmp _GetExitCodeProcess@8

extrn _GetVersion@0:proc
public GetVersion
GetVersion: jmp _GetVersion@0

extrn _GetLastError@0:proc
public GetLastError
GetLastError: jmp _GetLastError@0

extrn _GlobalAlloc@8:proc
public GlobalAlloc
GlobalAlloc: jmp _GlobalAlloc@8

extrn _GlobalFree@4:proc
public GlobalFree
GlobalFree: jmp _GlobalFree@4

extrn _MoveFileA@8:proc
public MoveFileA
MoveFileA: jmp _MoveFileA@8

extrn _ReadFile@20:proc
public ReadFile
ReadFile: jmp _ReadFile@20

extrn _SetFilePointer@16:proc
public SetFilePointer
SetFilePointer: jmp _SetFilePointer@16

extrn _WaitForSingleObject@8:proc
public WaitForSingleObject
WaitForSingleObject: jmp _WaitForSingleObject@8

extrn _SetConsoleCtrlHandler@8:proc
public SetConsoleCtrlHandler
SetConsoleCtrlHandler: jmp _SetConsoleCtrlHandler@8

extrn _GetSystemTime@4:proc
public GetSystemTime
GetSystemTime: jmp _GetSystemTime@4

extrn _SetLastError@4:proc
public SetLastError
SetLastError: jmp _SetLastError@4

extrn _GetConsoleScreenBufferInfo@8:proc
public GetConsoleScreenBufferInfo
GetConsoleScreenBufferInfo: jmp _GetConsoleScreenBufferInfo@8

extrn _WriteConsoleOutputA@20:proc
public WriteConsoleOutputA
WriteConsoleOutputA: jmp _WriteConsoleOutputA@20

extrn _GetConsoleMode@8:proc
public GetConsoleMode
GetConsoleMode: jmp _GetConsoleMode@8

extrn _SetConsoleMode@8:proc
public SetConsoleMode
SetConsoleMode: jmp _SetConsoleMode@8

extrn _GetNumberOfConsoleMouseButtons@4:proc
public GetNumberOfConsoleMouseButtons
GetNumberOfConsoleMouseButtons: jmp _GetNumberOfConsoleMouseButtons@4

extrn _FindFirstFileA@8:proc
public FindFirstFileA
FindFirstFileA: jmp _FindFirstFileA@8

extrn _FindNextFileA@8:proc
public FindNextFileA
FindNextFileA: jmp _FindNextFileA@8

extrn _FindClose@4:proc
public FindClose
FindClose: jmp _FindClose@4

extrn _AllocConsole@0:proc
public AllocConsole
AllocConsole: jmp _AllocConsole@0

extrn _AttachConsole@0:proc
public AttachConsole
AttachConsole: jmp _AttachConsole@0

extrn _CallNamedPipeA@0:proc
public CallNamedPipeA
CallNamedPipeA: jmp _CallNamedPipeA@0

extrn _CancelIo@0:proc
public CancelIo
CancelIo: jmp _CancelIo@0

extrn _ClearCommBreak@0:proc
public ClearCommBreak
ClearCommBreak: jmp _ClearCommBreak@0

extrn _ClearCommError@0:proc
public ClearCommError
ClearCommError: jmp _ClearCommError@0

extrn _CompareFileTime@0:proc
public CompareFileTime
CompareFileTime: jmp _CompareFileTime@0

extrn _CompareStringW@0:proc
public CompareStringW
CompareStringW: jmp _CompareStringW@0

extrn _ConnectNamedPipe@0:proc
public ConnectNamedPipe
ConnectNamedPipe: jmp _ConnectNamedPipe@0

extrn _CreateFileW@0:proc
public CreateFileW
CreateFileW: jmp _CreateFileW@0

extrn _CreateNamedPipeA@0:proc
public CreateNamedPipeA
CreateNamedPipeA: jmp _CreateNamedPipeA@0

extrn _CreatePipe@0:proc
public CreatePipe
CreatePipe: jmp _CreatePipe@0

extrn _CreateProcessW@0:proc
public CreateProcessW
CreateProcessW: jmp _CreateProcessW@0

extrn _CreateRemoteThread@0:proc
public CreateRemoteThread
CreateRemoteThread: jmp _CreateRemoteThread@0

extrn _CreateTapePartition@0:proc
public CreateTapePartition
CreateTapePartition: jmp _CreateTapePartition@0

extrn _CreateThread@0:proc
public CreateThread
CreateThread: jmp _CreateThread@0

extrn _DebugBreak@0:proc
public DebugBreak
DebugBreak: jmp _DebugBreak@0

extrn _DeleteCriticalSection@0:proc
public DeleteCriticalSection
DeleteCriticalSection: jmp _DeleteCriticalSection@0

extrn _DeviceIoControl@0:proc
public DeviceIoControl
DeviceIoControl: jmp _DeviceIoControl@0

extrn _DisconnectNamedPipe@0:proc
public DisconnectNamedPipe
DisconnectNamedPipe: jmp _DisconnectNamedPipe@0

extrn _DuplicateHandle@0:proc
public DuplicateHandle
DuplicateHandle: jmp _DuplicateHandle@0

extrn _EnterCriticalSection@0:proc
public EnterCriticalSection
EnterCriticalSection: jmp _EnterCriticalSection@0

extrn _EraseTape@0:proc
public EraseTape
EraseTape: jmp _EraseTape@0

extrn _EscapeCommFunction@0:proc
public EscapeCommFunction
EscapeCommFunction: jmp _EscapeCommFunction@0

extrn _ExitThread@0:proc
public ExitThread
ExitThread: jmp _ExitThread@0

extrn _ExpandEnvironmentStringsW@0:proc
public ExpandEnvironmentStringsW
ExpandEnvironmentStringsW: jmp _ExpandEnvironmentStringsW@0

extrn _FillConsoleOutputAttribute@20:proc
public FillConsoleOutputAttribute
FillConsoleOutputAttribute: jmp _FillConsoleOutputAttribute@20

extrn _FillConsoleOutputCharacterA@20:proc
public FillConsoleOutputCharacterA
FillConsoleOutputCharacterA: jmp _FillConsoleOutputCharacterA@20

extrn _FindFirstVolumeW@0:proc
public FindFirstVolumeW
FindFirstVolumeW: jmp _FindFirstVolumeW@0

extrn _FindNextVolumeW@0:proc
public FindNextVolumeW
FindNextVolumeW: jmp _FindNextVolumeW@0

extrn _FindVolumeClose@0:proc
public FindVolumeClose
FindVolumeClose: jmp _FindVolumeClose@0

extrn _FlushConsoleInputBuffer@0:proc
public FlushConsoleInputBuffer
FlushConsoleInputBuffer: jmp _FlushConsoleInputBuffer@0

extrn _FlushFileBuffers@0:proc
public FlushFileBuffers
FlushFileBuffers: jmp _FlushFileBuffers@0

extrn _FlushViewOfFile@0:proc
public FlushViewOfFile
FlushViewOfFile: jmp _FlushViewOfFile@0

extrn _FreeConsole@0:proc
public FreeConsole
FreeConsole: jmp _FreeConsole@0

extrn _FreeEnvironmentStringsW@0:proc
public FreeEnvironmentStringsW
FreeEnvironmentStringsW: jmp _FreeEnvironmentStringsW@0

extrn _FreeLibrary@0:proc
public FreeLibrary
FreeLibrary: jmp _FreeLibrary@0

extrn _GetBinaryTypeW@0:proc
public GetBinaryTypeW
GetBinaryTypeW: jmp _GetBinaryTypeW@0

extrn _GetCPInfo@0:proc
public GetCPInfo
GetCPInfo: jmp _GetCPInfo@0

extrn _GetCommModemStatus@0:proc
public GetCommModemStatus
GetCommModemStatus: jmp _GetCommModemStatus@0

extrn _GetCommState@0:proc
public GetCommState
GetCommState: jmp _GetCommState@0

extrn _GetCommandLineW@0:proc
public GetCommandLineW
GetCommandLineW: jmp _GetCommandLineW@0

extrn _GetComputerNameA@0:proc
public GetComputerNameA
GetComputerNameA: jmp _GetComputerNameA@0

extrn _GetComputerNameW@0:proc
public GetComputerNameW
GetComputerNameW: jmp _GetComputerNameW@0

extrn _GetConsoleCP@0:proc
public GetConsoleCP
GetConsoleCP: jmp _GetConsoleCP@0

extrn _GetConsoleCursorInfo@0:proc
public GetConsoleCursorInfo
GetConsoleCursorInfo: jmp _GetConsoleCursorInfo@0

extrn _GetConsoleWindow@0:proc
public GetConsoleWindow
GetConsoleWindow: jmp _GetConsoleWindow@0

extrn _GetCurrentProcess@0:proc
public GetCurrentProcess
GetCurrentProcess: jmp _GetCurrentProcess@0

extrn _GetCurrentProcessId@0:proc
public GetCurrentProcessId
GetCurrentProcessId: jmp _GetCurrentProcessId@0

extrn _GetCurrentThread@0:proc
public GetCurrentThread
GetCurrentThread: jmp _GetCurrentThread@0

extrn _GetCurrentThreadId@0:proc
public GetCurrentThreadId
GetCurrentThreadId: jmp _GetCurrentThreadId@0

extrn _GetEnvironmentStringsW@0:proc
public GetEnvironmentStringsW
GetEnvironmentStringsW: jmp _GetEnvironmentStringsW@0

extrn _GetEnvironmentVariableA@0:proc
public GetEnvironmentVariableA
GetEnvironmentVariableA: jmp _GetEnvironmentVariableA@0

extrn _GetEnvironmentVariableW@0:proc
public GetEnvironmentVariableW
GetEnvironmentVariableW: jmp _GetEnvironmentVariableW@0

extrn _GetExitCodeThread@0:proc
public GetExitCodeThread
GetExitCodeThread: jmp _GetExitCodeThread@0

extrn _GetFileSize@0:proc
public GetFileSize
GetFileSize: jmp _GetFileSize@0

extrn _GetFileSizeEx@0:proc
public GetFileSizeEx
GetFileSizeEx: jmp _GetFileSizeEx@0

extrn _GetFileType@0:proc
public GetFileType
GetFileType: jmp _GetFileType@0

extrn _GetHandleInformation@0:proc
public GetHandleInformation
GetHandleInformation: jmp _GetHandleInformation@0

extrn _GetLocaleInfoA@0:proc
public GetLocaleInfoA
GetLocaleInfoA: jmp _GetLocaleInfoA@0

extrn _GetLocaleInfoW@0:proc
public GetLocaleInfoW
GetLocaleInfoW: jmp _GetLocaleInfoW@0

extrn _GetLogicalDriveStringsA@0:proc
public GetLogicalDriveStringsA
GetLogicalDriveStringsA: jmp _GetLogicalDriveStringsA@0

extrn _GetLogicalDrives@0:proc
public GetLogicalDrives
GetLogicalDrives: jmp _GetLogicalDrives@0

extrn _GetMailslotInfo@0:proc
public GetMailslotInfo
GetMailslotInfo: jmp _GetMailslotInfo@0

extrn _GetModuleFileNameA@0:proc
public GetModuleFileNameA
GetModuleFileNameA: jmp _GetModuleFileNameA@0

extrn _GetModuleFileNameW@0:proc
public GetModuleFileNameW
GetModuleFileNameW: jmp _GetModuleFileNameW@0

extrn _GetModuleHandleA@0:proc
public GetModuleHandleA
GetModuleHandleA: jmp _GetModuleHandleA@0

extrn _GetModuleHandleExW@0:proc
public GetModuleHandleExW
GetModuleHandleExW: jmp _GetModuleHandleExW@0

extrn _GetModuleHandleW@0:proc
public GetModuleHandleW
GetModuleHandleW: jmp _GetModuleHandleW@0

extrn _GetNumberOfConsoleInputEvents@0:proc
public GetNumberOfConsoleInputEvents
GetNumberOfConsoleInputEvents: jmp _GetNumberOfConsoleInputEvents@0

extrn _GetOverlappedResult@0:proc
public GetOverlappedResult
GetOverlappedResult: jmp _GetOverlappedResult@0

extrn _GetPriorityClass@0:proc
public GetPriorityClass
GetPriorityClass: jmp _GetPriorityClass@0

extrn _GetProcAddress@0:proc
public GetProcAddress
GetProcAddress: jmp _GetProcAddress@0

extrn _GetProcessWorkingSetSize@0:proc
public GetProcessWorkingSetSize
GetProcessWorkingSetSize: jmp _GetProcessWorkingSetSize@0

extrn _GetStartupInfoA@0:proc
public GetStartupInfoA
GetStartupInfoA: jmp _GetStartupInfoA@0

extrn _GetStartupInfoW@0:proc
public GetStartupInfoW
GetStartupInfoW: jmp _GetStartupInfoW@0

extrn _GetSystemDirectoryW@0:proc
public GetSystemDirectoryW
GetSystemDirectoryW: jmp _GetSystemDirectoryW@0

extrn _GetSystemInfo@0:proc
public GetSystemInfo
GetSystemInfo: jmp _GetSystemInfo@0

extrn _GetSystemTimeAsFileTime@0:proc
public GetSystemTimeAsFileTime
GetSystemTimeAsFileTime: jmp _GetSystemTimeAsFileTime@0

extrn _GetSystemWindowsDirectoryW@0:proc
public GetSystemWindowsDirectoryW
GetSystemWindowsDirectoryW: jmp _GetSystemWindowsDirectoryW@0

extrn _GetSystemWow64DirectoryW@0:proc
public GetSystemWow64DirectoryW
GetSystemWow64DirectoryW: jmp _GetSystemWow64DirectoryW@0

extrn _GetTapeParameters@0:proc
public GetTapeParameters
GetTapeParameters: jmp _GetTapeParameters@0

extrn _GetTapePosition@0:proc
public GetTapePosition
GetTapePosition: jmp _GetTapePosition@0

extrn _GetTapeStatus@0:proc
public GetTapeStatus
GetTapeStatus: jmp _GetTapeStatus@0

extrn _GetThreadContext@0:proc
public GetThreadContext
GetThreadContext: jmp _GetThreadContext@0

extrn _GetThreadPriority@0:proc
public GetThreadPriority
GetThreadPriority: jmp _GetThreadPriority@0

extrn _GetTickCount@0:proc
public GetTickCount
GetTickCount: jmp _GetTickCount@0

extrn _GetTimeZoneInformation@0:proc
public GetTimeZoneInformation
GetTimeZoneInformation: jmp _GetTimeZoneInformation@0

extrn _GetVersionExA@0:proc
public GetVersionExA
GetVersionExA: jmp _GetVersionExA@0

extrn _GetVolumePathNamesForVolumeNameW@0:proc
public GetVolumePathNamesForVolumeNameW
GetVolumePathNamesForVolumeNameW: jmp _GetVolumePathNamesForVolumeNameW@0

extrn _GlobalLock@0:proc
public GlobalLock
GlobalLock: jmp _GlobalLock@0

extrn _GlobalMemoryStatusEx@0:proc
public GlobalMemoryStatusEx
GlobalMemoryStatusEx: jmp _GlobalMemoryStatusEx@0

extrn _GlobalSize@0:proc
public GlobalSize
GlobalSize: jmp _GlobalSize@0

extrn _GlobalUnlock@0:proc
public GlobalUnlock
GlobalUnlock: jmp _GlobalUnlock@0

extrn _InitializeCriticalSection@0:proc
public InitializeCriticalSection
InitializeCriticalSection: jmp _InitializeCriticalSection@0

extrn _IsBadStringPtrA@0:proc
public IsBadStringPtrA
IsBadStringPtrA: jmp _IsBadStringPtrA@0

extrn _IsDebuggerPresent@0:proc
public IsDebuggerPresent
IsDebuggerPresent: jmp _IsDebuggerPresent@0

extrn _IsProcessInJob@0:proc
public IsProcessInJob
IsProcessInJob: jmp _IsProcessInJob@0

extrn _IsProcessorFeaturePresent@0:proc
public IsProcessorFeaturePresent
IsProcessorFeaturePresent: jmp _IsProcessorFeaturePresent@0

extrn _LCMapStringW@0:proc
public LCMapStringW
LCMapStringW: jmp _LCMapStringW@0

extrn _LeaveCriticalSection@0:proc
public LeaveCriticalSection
LeaveCriticalSection: jmp _LeaveCriticalSection@0

extrn _LoadLibraryA@0:proc
public LoadLibraryA
LoadLibraryA: jmp _LoadLibraryA@0

extrn _LoadLibraryExA@0:proc
public LoadLibraryExA
LoadLibraryExA: jmp _LoadLibraryExA@0

extrn _LoadLibraryExW@0:proc
public LoadLibraryExW
LoadLibraryExW: jmp _LoadLibraryExW@0

extrn _LoadLibraryW@0:proc
public LoadLibraryW
LoadLibraryW: jmp _LoadLibraryW@0

extrn _LocalFree@0:proc
public LocalFree
LocalFree: jmp _LocalFree@0

extrn _MapViewOfFile@0:proc
public MapViewOfFile
MapViewOfFile: jmp _MapViewOfFile@0

extrn _MapViewOfFileEx@0:proc
public MapViewOfFileEx
MapViewOfFileEx: jmp _MapViewOfFileEx@0

extrn _MultiByteToWideChar@0:proc
public MultiByteToWideChar
MultiByteToWideChar: jmp _MultiByteToWideChar@0

extrn _OpenProcess@0:proc
public OpenProcess
OpenProcess: jmp _OpenProcess@0

extrn _OpenThread@0:proc
public OpenThread
OpenThread: jmp _OpenThread@0

extrn _OutputDebugStringA@0:proc
public OutputDebugStringA
OutputDebugStringA: jmp _OutputDebugStringA@0

extrn _PeekConsoleInputA@0:proc
public PeekConsoleInputA
PeekConsoleInputA: jmp _PeekConsoleInputA@0

extrn _PeekConsoleInputW@0:proc
public PeekConsoleInputW
PeekConsoleInputW: jmp _PeekConsoleInputW@0

extrn _PeekNamedPipe@0:proc
public PeekNamedPipe
PeekNamedPipe: jmp _PeekNamedPipe@0

extrn _PrepareTape@0:proc
public PrepareTape
PrepareTape: jmp _PrepareTape@0

extrn _PurgeComm@0:proc
public PurgeComm
PurgeComm: jmp _PurgeComm@0

extrn _QueryDosDeviceW@0:proc
public QueryDosDeviceW
QueryDosDeviceW: jmp _QueryDosDeviceW@0

extrn _QueryInformationJobObject@0:proc
public QueryInformationJobObject
QueryInformationJobObject: jmp _QueryInformationJobObject@0

extrn _QueryPerformanceCounter@0:proc
public QueryPerformanceCounter
QueryPerformanceCounter: jmp _QueryPerformanceCounter@0

extrn _QueryPerformanceFrequency@0:proc
public QueryPerformanceFrequency
QueryPerformanceFrequency: jmp _QueryPerformanceFrequency@0

extrn _QueueUserAPC@0:proc
public QueueUserAPC
QueueUserAPC: jmp _QueueUserAPC@0

extrn _ReadConsoleInputA@16:proc
public ReadConsoleInputA
ReadConsoleInputA: jmp _ReadConsoleInputA@16

extrn _ReadConsoleInputW@0:proc
public ReadConsoleInputW
ReadConsoleInputW: jmp _ReadConsoleInputW@0

extrn _ReadConsoleOutputW@0:proc
public ReadConsoleOutputW
ReadConsoleOutputW: jmp _ReadConsoleOutputW@0

extrn _ReadProcessMemory@0:proc
public ReadProcessMemory
ReadProcessMemory: jmp _ReadProcessMemory@0

extrn _ReleaseMutex@0:proc
public ReleaseMutex
ReleaseMutex: jmp _ReleaseMutex@0

extrn _ReleaseSemaphore@0:proc
public ReleaseSemaphore
ReleaseSemaphore: jmp _ReleaseSemaphore@0

extrn _ResetEvent@0:proc
public ResetEvent
ResetEvent: jmp _ResetEvent@0

extrn _ResumeThread@0:proc
public ResumeThread
ResumeThread: jmp _ResumeThread@0

extrn _RtlCaptureContext@0:proc
public RtlCaptureContext
RtlCaptureContext: jmp _RtlCaptureContext@0

extrn _RtlUnwind@0:proc
public RtlUnwind
RtlUnwind: jmp _RtlUnwind@0

extrn _ScrollConsoleScreenBufferA@0:proc
public ScrollConsoleScreenBufferA
ScrollConsoleScreenBufferA: jmp _ScrollConsoleScreenBufferA@0

extrn _SetCommBreak@0:proc
public SetCommBreak
SetCommBreak: jmp _SetCommBreak@0

extrn _SetCommMask@0:proc
public SetCommMask
SetCommMask: jmp _SetCommMask@0

extrn _SetCommState@0:proc
public SetCommState
SetCommState: jmp _SetCommState@0

extrn _SetCommTimeouts@0:proc
public SetCommTimeouts
SetCommTimeouts: jmp _SetCommTimeouts@0

extrn _SetConsoleCursorInfo@0:proc
public SetConsoleCursorInfo
SetConsoleCursorInfo: jmp _SetConsoleCursorInfo@0

extrn _SetConsoleCursorPosition@8:proc
public SetConsoleCursorPosition
SetConsoleCursorPosition: jmp _SetConsoleCursorPosition@8

extrn _SetConsoleTextAttribute@0:proc
public SetConsoleTextAttribute
SetConsoleTextAttribute: jmp _SetConsoleTextAttribute@0

extrn _SetConsoleTitleW@4:proc
public SetConsoleTitleW
SetConsoleTitleW: jmp _SetConsoleTitleW@4

extrn _SetConsoleTitleA@4:proc
public SetConsoleTitleA
SetConsoleTitleA: jmp _SetConsoleTitleA@4

extrn _GetConsoleTitleA@8:proc
public GetConsoleTitleA
GetConsoleTitleA: jmp _GetConsoleTitleA@8

extrn _SetEnvironmentVariableW@0:proc
public SetEnvironmentVariableW
SetEnvironmentVariableW: jmp _SetEnvironmentVariableW@0

extrn _SetErrorMode@0:proc
public SetErrorMode
SetErrorMode: jmp _SetErrorMode@0

extrn _SetEvent@0:proc
public SetEvent
SetEvent: jmp _SetEvent@0

extrn _SetFilePointerEx@0:proc
public SetFilePointerEx
SetFilePointerEx: jmp _SetFilePointerEx@0

extrn _SetHandleInformation@0:proc
public SetHandleInformation
SetHandleInformation: jmp _SetHandleInformation@0

extrn _SetNamedPipeHandleState@0:proc
public SetNamedPipeHandleState
SetNamedPipeHandleState: jmp _SetNamedPipeHandleState@0

extrn _SetPriorityClass@0:proc
public SetPriorityClass
SetPriorityClass: jmp _SetPriorityClass@0

extrn _SetProcessWorkingSetSize@0:proc
public SetProcessWorkingSetSize
SetProcessWorkingSetSize: jmp _SetProcessWorkingSetSize@0

extrn _SetStdHandle@8:proc
public SetStdHandle
SetStdHandle: jmp _SetStdHandle@8

extrn _SetSystemTime@0:proc
public SetSystemTime
SetSystemTime: jmp _SetSystemTime@0

extrn _SetTapeParameters@0:proc
public SetTapeParameters
SetTapeParameters: jmp _SetTapeParameters@0

extrn _SetTapePosition@0:proc
public SetTapePosition
SetTapePosition: jmp _SetTapePosition@0

extrn _SetThreadAffinityMask@0:proc
public SetThreadAffinityMask
SetThreadAffinityMask: jmp _SetThreadAffinityMask@0

extrn _SetThreadContext@0:proc
public SetThreadContext
SetThreadContext: jmp _SetThreadContext@0

extrn _SetThreadPriority@0:proc
public SetThreadPriority
SetThreadPriority: jmp _SetThreadPriority@0

extrn _Sleep@0:proc
public Sleep
Sleep: jmp _Sleep@0

extrn _SuspendThread@0:proc
public SuspendThread
SuspendThread: jmp _SuspendThread@0

extrn _SwitchToThread@0:proc
public SwitchToThread
SwitchToThread: jmp _SwitchToThread@0

extrn _TerminateProcess@0:proc
public TerminateProcess
TerminateProcess: jmp _TerminateProcess@0

extrn _TerminateThread@0:proc
public TerminateThread
TerminateThread: jmp _TerminateThread@0

extrn _TlsAlloc@0:proc
public TlsAlloc
TlsAlloc: jmp _TlsAlloc@0

extrn _TlsFree@0:proc
public TlsFree
TlsFree: jmp _TlsFree@0

extrn _TlsGetValue@0:proc
public TlsGetValue
TlsGetValue: jmp _TlsGetValue@0

extrn _TlsSetValue@0:proc
public TlsSetValue
TlsSetValue: jmp _TlsSetValue@0

extrn _TransmitCommChar@0:proc
public TransmitCommChar
TransmitCommChar: jmp _TransmitCommChar@0

extrn _UnmapViewOfFile@0:proc
public UnmapViewOfFile
UnmapViewOfFile: jmp _UnmapViewOfFile@0

extrn _VirtualAlloc@0:proc
public VirtualAlloc
VirtualAlloc: jmp _VirtualAlloc@0

extrn _VirtualFree@0:proc
public VirtualFree
VirtualFree: jmp _VirtualFree@0

extrn _VirtualProtect@0:proc
public VirtualProtect
VirtualProtect: jmp _VirtualProtect@0

extrn _VirtualProtectEx@0:proc
public VirtualProtectEx
VirtualProtectEx: jmp _VirtualProtectEx@0

extrn _VirtualQuery@0:proc
public VirtualQuery
VirtualQuery: jmp _VirtualQuery@0

extrn _VirtualQueryEx@0:proc
public VirtualQueryEx
VirtualQueryEx: jmp _VirtualQueryEx@0

extrn _WaitCommEvent@0:proc
public WaitCommEvent
WaitCommEvent: jmp _WaitCommEvent@0

extrn _WaitForMultipleObjects@0:proc
public WaitForMultipleObjects
WaitForMultipleObjects: jmp _WaitForMultipleObjects@0

extrn _WaitNamedPipeW@0:proc
public WaitNamedPipeW
WaitNamedPipeW: jmp _WaitNamedPipeW@0

extrn _WideCharToMultiByte@0:proc
public WideCharToMultiByte
WideCharToMultiByte: jmp _WideCharToMultiByte@0

extrn _WriteConsoleOutputW@0:proc
public WriteConsoleOutputW
WriteConsoleOutputW: jmp _WriteConsoleOutputW@0

extrn _WriteConsoleW@0:proc
public WriteConsoleW
WriteConsoleW: jmp _WriteConsoleW@0

extrn _WriteProcessMemory@0:proc
public WriteProcessMemory
WriteProcessMemory: jmp _WriteProcessMemory@0

extrn _WriteTapemark@0:proc
public WriteTapemark
WriteTapemark: jmp _WriteTapemark@0

extrn _CompareStringA@0:proc
public CompareStringA
CompareStringA: jmp _CompareStringA@0

extrn _FileTimeToLocalFileTime@0:proc
public FileTimeToLocalFileTime
FileTimeToLocalFileTime: jmp _FileTimeToLocalFileTime@0

extrn _FileTimeToSystemTime@0:proc
public FileTimeToSystemTime
FileTimeToSystemTime: jmp _FileTimeToSystemTime@0

extrn _FormatMessageA@0:proc
public FormatMessageA
FormatMessageA: jmp _FormatMessageA@0

extrn _FreeEnvironmentStringsA@0:proc
public FreeEnvironmentStringsA
FreeEnvironmentStringsA: jmp _FreeEnvironmentStringsA@0

extrn _GetACP@0:proc
public GetACP
GetACP: jmp _GetACP@0

extrn _GetCurrentDirectoryA@0:proc
public GetCurrentDirectoryA
GetCurrentDirectoryA: jmp _GetCurrentDirectoryA@0

extrn _GetDriveTypeA@0:proc
public GetDriveTypeA
GetDriveTypeA: jmp _GetDriveTypeA@0

extrn _GetFullPathNameA@0:proc
public GetFullPathNameA
GetFullPathNameA: jmp _GetFullPathNameA@0

extrn _GetLocalTime@0:proc
public GetLocalTime
GetLocalTime: jmp _GetLocalTime@0

extrn _GetOEMCP@0:proc
public GetOEMCP
GetOEMCP: jmp _GetOEMCP@0

extrn _GetStringTypeA@0:proc
public GetStringTypeA
GetStringTypeA: jmp _GetStringTypeA@0

extrn _GetStringTypeW@0:proc
public GetStringTypeW
GetStringTypeW: jmp _GetStringTypeW@0

extrn _HeapAlloc@0:proc
public HeapAlloc
HeapAlloc: jmp _HeapAlloc@0

extrn _HeapCreate@0:proc
public HeapCreate
HeapCreate: jmp _HeapCreate@0

extrn _HeapDestroy@0:proc
public HeapDestroy
HeapDestroy: jmp _HeapDestroy@0

extrn _HeapFree@0:proc
public HeapFree
HeapFree: jmp _HeapFree@0

extrn _HeapReAlloc@0:proc
public HeapReAlloc
HeapReAlloc: jmp _HeapReAlloc@0

extrn _LCMapStringA@0:proc
public LCMapStringA
LCMapStringA: jmp _LCMapStringA@0

extrn _LocalFileTimeToFileTime@0:proc
public LocalFileTimeToFileTime
LocalFileTimeToFileTime: jmp _LocalFileTimeToFileTime@0

extrn _SetCurrentDirectoryA@0:proc
public SetCurrentDirectoryA
SetCurrentDirectoryA: jmp _SetCurrentDirectoryA@0

extrn _SetEndOfFile@0:proc
public SetEndOfFile
SetEndOfFile: jmp _SetEndOfFile@0

extrn _SetEnvironmentVariableA@0:proc
public SetEnvironmentVariableA
SetEnvironmentVariableA: jmp _SetEnvironmentVariableA@0

extrn _SetFileTime@0:proc
public SetFileTime
SetFileTime: jmp _SetFileTime@0

extrn _SetHandleCount@0:proc
public SetHandleCount
SetHandleCount: jmp _SetHandleCount@0

extrn _SystemTimeToFileTime@0:proc
public SystemTimeToFileTime
SystemTimeToFileTime: jmp _SystemTimeToFileTime@0

extrn _UnhandledExceptionFilter@0:proc
public UnhandledExceptionFilter
UnhandledExceptionFilter: jmp _UnhandledExceptionFilter@0

end
