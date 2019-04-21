
#include <Windows.h>
#include <winioctl.h>
#include <stdio.h>

#define BUFF_LENGTH 8192

typedef enum 
{
	Nothing		   = 0,
	DeviceProperty    = 1,
	DirectInformation = 2
}DEVICE_INFO_ENUM;

typedef struct  _DEVICE_OBJECT_INFO {
    SHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    PVOID DriverObject;
    PVOID NextDevice;
    PVOID AttachedDevice;
    PVOID CurrentIrp;
    PVOID Timer;
    ULONG Flags;            
    ULONG Characteristics;                 
    PVOID Vpb;
    PVOID DeviceExtension;
    ULONG DeviceType;
    CCHAR StackSize;
    ULONG AlignmentRequirement;
    ULONG ActiveThreadCount;
    PVOID SecurityDescriptor;
    USHORT SectorSize;
    USHORT Spare1;
    PVOID DeviceObjectExtension;
} DEVICE_OBJECT_INFO, *PDEVICE_OBJECT_INFO;


typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY 
{
    HANDLE Section; 
    PVOID MappedBase;
    PVOID Base;
    ULONG Size;
    ULONG Flags;
    USHORT Index;
/* Length of module name not including the path, 
this field contains valid value only for NTOSKRNL module */
    USHORT NameLength;
    USHORT LoadCount;
    USHORT PathLength;
    CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION {
    ULONG Count; //the total number of loaded modules
    SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

extern VOID DisplayDirectInfo(HANDLE hDevice);
extern VOID DisplayDeviceProperty(HANDLE hDevice);
