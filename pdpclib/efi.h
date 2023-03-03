/******************************************************************************
 * @file            efi.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
typedef unsigned char BOOLEAN;
#ifdef __64BIT__
typedef long long INTN;
typedef unsigned long long UINTN;
#else
typedef int INTN;
typedef unsigned int UINTN;
#endif

typedef unsigned char UINT8;
typedef unsigned short UINT16;

typedef unsigned long UINT32;

typedef struct {
    UINT32 a;
    UINT32 b;
} UINT64;

typedef char CHAR8;
typedef short CHAR16;

typedef struct {
    UINT32 a;
    UINT16 b;
    UINT16 c;
    UINT8 d[8];
} EFI_GUID;
    
typedef UINTN EFI_STATUS;
typedef void * EFI_HANDLE;
typedef void * EFI_EVENT;
typedef UINT64 EFI_LBA;

#define IN
#define OUT
#define OPTIONAL
#define CONST
#define EFIAPI

#define EFI_SUCCESS 0
/* Highest bit of EFI_STATUS set. */
#define EFI_LOAD_ERROR 1
#define EFI_INVALID_PARAMETER 2
#define EFI_UNSUPPORTED 3
#define EFI_BAD_BUFFER_SIZE 4
#define EFI_BUFFER_TOO_SMALL 5
#define EFI_NOT_READY 6
#define EFI_DEVICE_ERROR 7
#define EFI_WRITE_PROTECTED 8
#define EFI_OUT_OF_RESOURCES 9
#define EFI_VOLUME_CORRUPTED 10
#define EFI_VOLUME_FULL 11
#define EFI_NO_MEDIA 12
#define EFI_MEDIA_CHANGED 13
#define EFI_NOT_FOUND 14
#define EFI_ACCESS_DENIED 15

struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET) (IN struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, IN BOOLEAN ExtendedVerification);

typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (IN struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, OUT EFI_INPUT_KEY *Key);

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET Reset;
    EFI_INPUT_READ_KEY ReadKeyStroke;
    EFI_EVENT WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;


struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING) (IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, IN CHAR16 *String);

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void *Reset;
    EFI_TEXT_STRING OutputString;
    void *TestString;
    void *QueryMode;
    void *SetMode;
    void *SetAttribute;
    void *ClearScreen;
    void *SetCursorPosition;
    void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;


typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData, /* Should be used for most allocations. */
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPYMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL) (IN EFI_MEMORY_TYPE PoolType, IN UINTN Size, OUT void **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL) (IN void *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT) (IN UINTN NumberOfEvents, IN EFI_EVENT *Event, OUT UINTN *Index);

typedef EFI_STATUS (EFIAPI *EFI_SET_WATCHDOG_TIMER) (IN UINTN Timeout, IN UINT64 WatchdogCode, IN UINTN DataSize, IN CHAR16 *WatchdogData);

typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT void **Interface);

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL) (IN EFI_HANDLE Handle,
                                                IN EFI_GUID *Protocol,
                                                OUT void **Interface OPTIONAL,
                                                IN EFI_HANDLE AgentHandle,
                                                IN EFI_HANDLE ControllerHandle,
                                                IN UINT32 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_PROTOCOL) (IN EFI_HANDLE Handle,
                                                 IN EFI_GUID *Protocol,
                                                 IN EFI_HANDLE AgentHandle,
                                                 IN EFI_HANDLE ControllerHandle);

typedef struct {
    EFI_TABLE_HEADER Hdr;

    void *RaiseTPL;
    void *RestoreTPL;

    void *AllocatePages;
    void *FreePages;
    void *GetMemoryMap;
    EFI_ALLOCATE_POOL AllocPool;
    EFI_FREE_POOL FreePool;

    void *CreateEvent;
    void *SetTimer;
    EFI_WAIT_FOR_EVENT WaitForEvent;
    void *SignalEvent;
    void *CloseEvent;
    void *CheckEvent;

    void *InstallProtocolInterface;
    void *ReinstallProtocolInterface;
    void *UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL HandleProtocol;
    void *RegisterProtocolNotify;
    void *LocateHandle;
    void *LocateDevicePath;
    void *InstallConfigurationTable;

    void *LoadImage;
    void *StartImage;
    void *Exit;
    void *UnloadImage;
    void *ExitBootServices;

    void *GetNextMonotonicCount;
    void *Stall;
    EFI_SET_WATCHDOG_TIMER SetWatchdogTimer;
    
    void *ConnectController;
    void *DisconnectController;

    EFI_OPEN_PROTOCOL OpenProtocol; /* EFI 1.1+ */
    EFI_CLOSE_PROTOCOL CloseProtocol; /* EFI 1.1+ */
    void *OpenProtocolInformation;

    void *ProtocolsPerHandle;
    void *LocateHandleBuffer;
    void *LocateProtocol;
    void *InstallMultipleProtocolInterfaces;
    void *UninstallMultipleProtocolInterfaces;

    void *CalculateCrc32;

    void *CopyMem;
    void *SetMem;
    void *CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16 *FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    void *RuntimeServices;
    EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries;
    void *ConfigurationTable;
} EFI_SYSTEM_TABLE;

typedef EFI_STATUS (EFIAPI *EFI_IMAGE_ENTRY_POINT) (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
 {0x5b1b31a1,0x9562,0x11d2,{0x8e,0x3f,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

typedef struct {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    EFI_SYSTEM_TABLE *SystemTable;

    /* Source location of the image. */
    EFI_HANDLE DeviceHandle;
    void *FilePath;
    void *Reserved;

    UINT32 LoadOptionsSize;
    void *LoadOptions;

    void *ImageBase;
    UINT64 ImageSize;
    EFI_MEMORY_TYPE ImageCodeType;
    EFI_MEMORY_TYPE ImageDataType;
    void *Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

/* File system protocol. */
#define EFI_FILE_PROTOCOL_REVISION  0x00010000
#define EFI_FILE_PROTOCOL_REVISION2 0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

struct _EFI_FILE_PROTOCOL;

#define EFI_FILE_MODE_READ   0x0000000000000001
#define EFI_FILE_MODE_WRITE  0x0000000000000002
#define EFI_FILE_MODE_CREATE 0x8000000000000000

#define EFI_FILE_READ_ONLY  0x0000000000000001
#define EFI_FILE_HIDDEN     0x0000000000000002
#define EFI_FILE_SYSTEM     0x0000000000000004
#define EFI_FILE_RESERVED   0x0000000000000008
#define EFI_FILE_DIRECTORY  0x0000000000000010
#define EFI_FILE_ARCHIVE    0x0000000000000020
#define EFI_FILE_VALID_ATTR 0x0000000000000037

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN) (IN struct _EFI_FILE_PROTOCOL *This,
                                            OUT struct _EFI_FILE_PROTOCOL **NewHandle,
                                            IN CHAR16 *FileName,
                                            IN UINT64 OpenMode,
                                            IN UINT64 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE) (IN struct _EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE) (IN struct _EFI_FILE_PROTOCOL *This);
typedef EFI_STATUS (EFIAPI *EFI_FILE_READ) (IN struct _EFI_FILE_PROTOCOL *This,
                                            IN OUT UINTN *BufferSize,
                                            OUT void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE) (IN struct _EFI_FILE_PROTOCOL *This,
                                             IN OUT UINTN *BufferSize,
                                             IN const void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION) (IN struct _EFI_FILE_PROTOCOL *This,
                                                    OUT UINT64 *Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION) (IN struct _EFI_FILE_PROTOCOL *This,
                                                    IN UINT64 Position);


typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH) (IN struct _EFI_FILE_PROTOCOL *This);


typedef struct _EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_FILE_OPEN Open;
    EFI_FILE_CLOSE Close;
    EFI_FILE_DELETE Delete;
    EFI_FILE_READ Read;
    EFI_FILE_WRITE Write;
    EFI_FILE_GET_POSITION GetPosition;
    EFI_FILE_SET_POSITION SetPosition;
    void *GetInfo;
    void *SetInfo;
    EFI_FILE_FLUSH Flush;
    /* Added with revision 2. */
    void *OpenEx;
    void *ReadEx;
    void *WriteEx;
    void *FlushEx;
} EFI_FILE_PROTOCOL;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
 {0x964e5b22,0x6459,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION 0x00010000

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
                                                                          OUT EFI_FILE_PROTOCOL **Root);

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 Revision;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

#define EFI_BLOCK_IO_PROTOCOL_GUID \
 {0x964e5b21,0x6459,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_BLOCK_IO_PROTOCOL_REVISION2 0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3 0x0002001f

typedef struct {
    UINT32 MediaId;
    BOOLEAN RemovableMedia;
    BOOLEAN MediaPresent;
    BOOLEAN LogicalPartition;
    BOOLEAN ReadOnly;
    BOOLEAN WriteCaching;
    UINT32 BlockSize;
    UINT32 IoAlign;
    EFI_LBA LastBlock;

    /* Added with revision 2. */
    EFI_LBA LowestAlignedLba;
    UINT32 LogicalBlocksPerPhysicalBlock;

    /* Added with revision 3. */
    UINT32 OptimalTransferLengthGranularity;
} EFI_BLOCK_MEDIA;

struct _EFI_BLOCK_IO_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_BLOCK_RESET) (IN struct _EFI_BLOCK_IO_PROTOCOL *This,
                                              IN BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_READ) (IN struct _EFI_BLOCK_IO_PROTOCOL *This,
                                             IN UINT32 MediaId,
                                             IN EFI_LBA LBA,
                                             IN UINTN BufferSize,
                                             OUT void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_WRITE) (IN struct _EFI_BLOCK_IO_PROTOCOL *This,
                                              IN UINT32 MediaId,
                                              IN EFI_LBA LBA,
                                              IN UINTN BufferSize,
                                              IN void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_BLOCK_FLUSH) (IN struct _EFI_BLOCK_IO_PROTOCOL *This);

typedef struct _EFI_BLOCK_IO_PROTOCOL {
    UINT64 Revision;
    EFI_BLOCK_MEDIA *Media;
    EFI_BLOCK_RESET Reset;
    EFI_BLOCK_READ ReadBlocks;
    EFI_BLOCK_WRITE WriteBlocks;
    EFI_BLOCK_FLUSH FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;


/* UEFI Shell */
typedef void *SHELL_FILE_HANDLE;

#define EFI_SHELL_PROTOCOL_GUID \
 {0x6302d008,0x7f9b,0x4f30,{0x87,0xac,0x60,0xc9,0xfe,0xf5,0xda,0x4e}}

#define EFI_SHELL_MAJOR_VERSION 2
#define EFI_SHELL_MINOR_VERSION 2

typedef EFI_STATUS (EFIAPI *EFI_SHELL_READ_FILE) (IN SHELL_FILE_HANDLE FileHandle,
                                                  IN OUT UINTN *BufferSize,
                                                  OUT void *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_SHELL_WRITE_FILE) (IN SHELL_FILE_HANDLE FileHandle,
                                                   IN OUT UINTN *BufferSize,
                                                   IN void *Buffer);

typedef struct _EFI_SHELL_PROTOCOL {
    void *Execute;
    void *GetEnv;
    void *SetEnv;
    void *GetAlias;
    void *SetAlias;
    void *GetHelpText;
    void *GetDevicePathFromMap;
    void *GetMapFromDevicePath;
    void *GetDevicePathFromFilePath;
    void *GetFilePathFromDevicePath;
    void *SetMap;

    void *GetCurDir;
    void *SetCurDir;
    void *OpenFileList;
    void *FreeFileList;
    void *RemoveDupInFileList;

    void *BatchIsActive;
    void *IsRootShell;
    void *EnablePageBreak;
    void *DisablePageBreak;
    void *GetPageBreak;
    void *GetDeviceName;

    void *GetFileInfo;
    void *SetFileInfo;
    void *OpenFileByName;
    void *CloseFile;
    void *CreateFile;
    EFI_SHELL_READ_FILE ReadFile;
    EFI_SHELL_WRITE_FILE WriteFile;
    void *DeleteFile;
    void *DeleteFileByName;
    void *GetFilePosition;
    void *SetFilePosition;
    void *FlushFile;
    void *FindFiles;
    void *FindFilesInDir;
    void *GetFileSize;

    void *OpenRoot;
    void *OpenRootByHandle;

    EFI_EVENT ExecutionBreak;
    UINT32 MajorVersion;
    UINT32 MinorVersion;
    void *RegisterGuidName;
    void *GetGuidName;
    void *GetGuidFromName;
    /* Added with Shell 2.1. */
    void *GetEnvEx;
} EFI_SHELL_PROTOCOL;

#define EFI_SHELL_PARAMETERS_PROTOCOL_GUID \
 {0x752f3136,0x4e16,0x4fdc,{0xa2,0x2a,0xe5,0xf4,0x68,0x12,0xf4,0xca}}

typedef struct _EFI_SHELL_PARAMETERS_PROTOCOL {
    CHAR16 **Argv;
    UINTN Argc;
    SHELL_FILE_HANDLE StdIn;
    SHELL_FILE_HANDLE StdOut;
    SHELL_FILE_HANDLE StdErr;
} EFI_SHELL_PARAMETERS_PROTOCOL;


extern EFI_HANDLE *__gIH;
extern EFI_SYSTEM_TABLE *__gST;
extern EFI_BOOT_SERVICES *__gBS;
