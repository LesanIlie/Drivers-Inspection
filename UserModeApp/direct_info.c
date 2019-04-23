#include "driver.h"
#include "ioctl.h"

/* Print device capabilities */
VOID PrintDeviceCapabilities(PVOID pDeviceInfo)
{
}

/* Print device PDO */
VOID PrintDeviceObject(PVOID pDeviceInfo)
{
    PDEVICE_OBJECT_INFO pDeviceObjectInfo = (PDEVICE_OBJECT_INFO)pDeviceInfo;
    
    if (pDeviceInfo)
    {
        wprintf(L"Type                 : %08X\n", pDeviceObjectInfo->Type);
        wprintf(L"Size                 : %08X\n", pDeviceObjectInfo->Size);
        wprintf(L"ReferenceCount       : %08X\n", pDeviceObjectInfo->ReferenceCount);
        wprintf(L"DriverObject         : %08X\n", pDeviceObjectInfo->DriverObject);
        wprintf(L"NextDevice           : %08X\n", pDeviceObjectInfo->NextDevice);
        wprintf(L"AttachedDevice       : %08X\n", pDeviceObjectInfo->AttachedDevice);
        wprintf(L"CurrentIrp           : %08X\n", pDeviceObjectInfo->CurrentIrp);
        wprintf(L"Timer                : %08X\n", pDeviceObjectInfo->Timer);
        wprintf(L"Flags                : %08X\n", pDeviceObjectInfo->Flags);
        wprintf(L"Characteristics      : %08X\n", pDeviceObjectInfo->Characteristics);
        wprintf(L"Vpb                  : %08X\n", pDeviceObjectInfo->Vpb);
        wprintf(L"DeviceExtension      : %08X\n", pDeviceObjectInfo->DeviceExtension);
        wprintf(L"DeviceType           : %08X\n", pDeviceObjectInfo->DeviceType);
        wprintf(L"StackSize            : %08X\n", pDeviceObjectInfo->StackSize);
        wprintf(L"AlignmentRequirement : %08X\n", pDeviceObjectInfo->AlignmentRequirement);
        wprintf(L"ActiveThreadCount    : %08X\n", pDeviceObjectInfo->ActiveThreadCount);
        wprintf(L"SecurityDescriptor   : %08X\n", pDeviceObjectInfo->SecurityDescriptor);
        wprintf(L"SectorSize           : %08X\n", pDeviceObjectInfo->SectorSize);
        wprintf(L"Spare1               : %08X\n", pDeviceObjectInfo->Spare1);
        wprintf(L"DeviceObjectExtension: %08X\n", pDeviceObjectInfo->DeviceObjectExtension);
    }
}

/* Print the state of the device */
VOID PrintDeviceState(PVOID pDeviceInfo)
{
    wprintf(L"Device state: %08X\n", *(ULONG*)pDeviceInfo);
}

/* Print the device text description */
VOID PrintDeviceText(PVOID pDeviceInfo)
{
    wprintf(L"Device Text: %ws \n", (PWCHAR)pDeviceInfo);
}

/* Print device resource requirements */
VOID PrintResourceRequirements(PVOID pDeviceInfo)
{    
}

/* Initialize table of function pointers */
VOID (*PrintDirectInfoFunctionTable[])(PVOID pDeviceProperty) = 
{
    PrintDeviceCapabilities,
    PrintDeviceObject,
    PrintDeviceText,
    PrintResourceRequirements,
    PrintDeviceState,
};

VOID DisplayDirectInfo(HANDLE hDevice)
{
    ULONG ulCounter = 0; 
    DWORD lpBytesReturned;
    PWCHAR pwcOutputDriverName;
    
    pwcOutputDriverName = (PWCHAR)malloc(sizeof(WCHAR)*BUFF_LENGTH);
    
    for (ulCounter = 0; ulCounter <= 4; ulCounter++)
    {
        RtlZeroMemory(pwcOutputDriverName, sizeof(WCHAR)*BUFF_LENGTH);
        if (DeviceIoControl(hDevice, 
                            IOCTL_GET_DIRECT_DEVICE_INFO, 
                            &ulCounter, 
                            sizeof(ULONG),
                            pwcOutputDriverName,
                            sizeof(WCHAR)*BUFF_LENGTH,
                            &lpBytesReturned,
                            NULL))
        {
            (*PrintDirectInfoFunctionTable[ulCounter])((PVOID)pwcOutputDriverName);
        }
        else
        {
            //printf("Function[%d] failed with status: 0x%08X\n", ulCounter, GetLastError());
        }
    }
    free(pwcOutputDriverName);
}