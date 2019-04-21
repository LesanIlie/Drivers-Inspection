#include "stddcls.h"
#include "driver.h"

extern ULONG ulBytesReturned;

/* This two tables are from Walter Oney book, all rights reserved */ 
char* SysNames[] = {
	"PowerSystemUnspecified",
	"PowerSystemWorking",
	"PowerSystemSleeping1",
	"PowerSystemSleeping2",
	"PowerSystemSleeping3",
	"PowerSystemHibernate",
	"PowerSystemShutdown",
	};

char* DevNames[] = {
	"PowerDeviceUnspecified",
	"PowerDeviceD0",
	"PowerDeviceD1",
	"PowerDeviceD2",
	"PowerDeviceD3",
    "PowerDeviceMaximum"
	};

/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
VOID PrintDeviceCapabilities(PDEVICE_CAPABILITIES pDeviceCapabilities)
{
    ULONG ulIndex = 0;
    ASSERT(pDeviceCapabilities != NULL);

    DbgPrint("Size              : %08X\n", pDeviceCapabilities->Size);
    DbgPrint("Version           : %08X\n", pDeviceCapabilities->Version);
    DbgPrint("DeviceD1          : %08X\n", pDeviceCapabilities->DeviceD1);
    DbgPrint("DeviceD2          : %08X\n", pDeviceCapabilities->DeviceD2);
    DbgPrint("LockSupported     : %08X\n", pDeviceCapabilities->LockSupported);
    DbgPrint("EjectSupported    : %08X\n", pDeviceCapabilities->EjectSupported);
    DbgPrint("Removable         : %08X\n", pDeviceCapabilities->Removable);
    DbgPrint("DockDevice        : %08X\n", pDeviceCapabilities->DockDevice);
    DbgPrint("UniqueID          : %08X\n", pDeviceCapabilities->UniqueID);
    DbgPrint("SilentInstall     : %08X\n", pDeviceCapabilities->SilentInstall);
    DbgPrint("RawDeviceOK       : %08X\n", pDeviceCapabilities->RawDeviceOK);
    DbgPrint("SurpriseRemovalOK : %08X\n", pDeviceCapabilities->SurpriseRemovalOK);
    DbgPrint("WakeFromD0        : %08X\n", pDeviceCapabilities->WakeFromD0);
    DbgPrint("WakeFromD1        : %08X\n", pDeviceCapabilities->WakeFromD1);
    DbgPrint("WakeFromD2        : %08X\n", pDeviceCapabilities->WakeFromD2);
    DbgPrint("WakeFromD3        : %08X\n", pDeviceCapabilities->WakeFromD3);
    DbgPrint("HardwareDisabled  : %08X\n", pDeviceCapabilities->HardwareDisabled);
    DbgPrint("NonDynamic        : %08X\n", pDeviceCapabilities->NonDynamic);
    DbgPrint("WarmEjectSupported: %08X\n", pDeviceCapabilities->WarmEjectSupported);
    DbgPrint("NoDisplayInUI     : %08X\n", pDeviceCapabilities->NoDisplayInUI);
    DbgPrint("Address           : %08X\n", pDeviceCapabilities->Address);
    DbgPrint("UINumber          : %08X\n", pDeviceCapabilities->UINumber);

    for (ulIndex = 0; ulIndex < (ULONG)POWER_SYSTEM_MAXIMUM; ulIndex++)
    {
        DbgPrint("DeviceState[%s]: %s\n", SysNames[ulIndex], DevNames[pDeviceCapabilities->DeviceState[ulIndex]]);      
    }

    DbgPrint("SystemWake: %s\n", SysNames[pDeviceCapabilities->SystemWake]);
    DbgPrint("DeviceWake: %s\n", DevNames[pDeviceCapabilities->DeviceWake]);
    DbgPrint("D1Latency: %08X\n", pDeviceCapabilities->D1Latency);
    DbgPrint("D2Latency: %08X\n", pDeviceCapabilities->D2Latency);
    DbgPrint("D3Latency: %08X\n", pDeviceCapabilities->D3Latency);
}

/*++
 * @name
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return 
 *           
 *
 * @remarks
 *
 *--*/
NTSTATUS PnpInitCapabilitiesInfoStack(PIO_STACK_LOCATION pIoStackLocation, 
                                      PFILE_OBJECT pFileObject,
                                      PDEVICE_CAPABILITIES pDeviceCapabilities)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (pIoStackLocation == NULL)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));
    pIoStackLocation->MinorFunction = IRP_MN_QUERY_CAPABILITIES ;
    pIoStackLocation->MajorFunction = IRP_MJ_PNP;
    pIoStackLocation->Parameters.DeviceCapabilities.Capabilities = pDeviceCapabilities;
    pIoStackLocation->FileObject = pFileObject;

    return ntStatus;
}

/*++
 * @name QueryCapabilities
 *
 * @description  
 *
 * @param 
 *         
 *
 * @param 
 *       
 *
 * @return   
 *         
 *
 * @remarks 
 *
 *--*/
NTSTATUS QueryCapabilities(PDEVICE_OBJECT pDeviceObject,
                           PIRP pIrp,
                           PFILE_OBJECT pFileObject,
                           ULONG ulOutputUserBufferLength,
                           PVOID pvOutBuffer,
                           PULONG pulReturnedLength)
{
    NTSTATUS             ntStatus            = STATUS_SUCCESS;
    PDEVICE_CAPABILITIES pDeviceCapabilities = NULL;
    IO_STACK_LOCATION    InfoStackLocation;
    IO_STATUS_BLOCK      IoStatusBlock;

    ASSERT(pulReturnedLength);

    /* Allocate memory */
    pDeviceCapabilities = (PDEVICE_CAPABILITIES)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_CAPABILITIES), 'CAP ');

    /* Zero it */
    RtlZeroMemory(pDeviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    /* Initialize stack location for device capabilities */
    ntStatus = PnpInitCapabilitiesInfoStack(&InfoStackLocation,
                                            pFileObject,
                                            pDeviceCapabilities);
    
    /* Check if the stack location was successfuly initialized */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to intialize stack location\n");
        goto clean_up;
    }

    /* Initalize mandatory filds
     * https://msdn.microsoft.com/en-us/library/windows/hardware/ff551664(v=vs.85).aspx
     */
    pDeviceCapabilities->Size     = sizeof(DEVICE_CAPABILITIES);
    pDeviceCapabilities->Version  = 0x01;
    pDeviceCapabilities->Address  = -1;
    pDeviceCapabilities->UINumber = -1;
    
    /* Query device capabilities */
    ntStatus = PnpQueryDeviceInformation(pDeviceObject, 
                                         &InfoStackLocation,
                                         (PVOID*)pDeviceCapabilities);

    /* Check if operation succeded */
    if (!NT_SUCCESS(ntStatus) || pDeviceCapabilities == NULL)
    {
        DbgPrint("Failed to get device PDO\n");
        goto clean_up;
    }
    
    /* Display all the information */
    PrintDeviceCapabilities(pDeviceCapabilities);

    if (ulOutputUserBufferLength >= sizeof(DEVICE_CAPABILITIES))
    {
        *pulReturnedLength = sizeof(DEVICE_CAPABILITIES);
        RtlCopyMemory(pvOutBuffer, pDeviceCapabilities, sizeof(DEVICE_CAPABILITIES));
    }
    else 
    {
        *pulReturnedLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
clean_up:
    /* Free the allocated memory */
    ExFreePoolWithTag(pDeviceCapabilities, 'CAP ');
    
    return ntStatus;
}
/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/

VOID PrintDeviceObject(PDEVICE_RELATIONS pDeviceRelations)
{
    ASSERT(pDeviceRelations != NULL);

    DbgPrint("PDO device object: %08X\n", pDeviceRelations->Objects[0]);
}

/*++
 * @name
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return 
 *           
 *
 * @remarks
 *
 *--*/
NTSTATUS PnpInitDeviceRelationsInfoStack(PIO_STACK_LOCATION pIoStackLocation, 
                                         PFILE_OBJECT pFileObject,
                                         PDEVICE_RELATIONS pDeviceRelations)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
 
    UNREFERENCED_PARAMETER(pDeviceRelations);

    if (pIoStackLocation == NULL)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));
    pIoStackLocation->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    pIoStackLocation->MajorFunction = IRP_MJ_PNP;
    pIoStackLocation->Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;
    pIoStackLocation->FileObject = pFileObject;

    return ntStatus;
}

/*++
 * @name QueryDeviceRelations
 *
 * @description  
 *
 * @param 
 *         
 *
 * @param 
 *       
 *
 * @return   
 *         
 *
 * @remarks 
 *
 *--*/
NTSTATUS QueryDeviceRelations(PDEVICE_OBJECT pDeviceObject,
                              PIRP pIrp,
                              PFILE_OBJECT pFileObject,
                              ULONG ulOutputUserBufferLength,
                              PVOID pvOutBuffer,
                              PULONG pulReturnedLength)
{
    NTSTATUS             ntStatus            = STATUS_SUCCESS;
    PDEVICE_RELATIONS    pDeviceRelations    = NULL;
    IO_STACK_LOCATION    InfoStackLocation;
    IO_STATUS_BLOCK      IoStatusBlock;

    ASSERT(pulReturnedLength);

    /* Initialize stack location for device capabilities */
    ntStatus = PnpInitDeviceRelationsInfoStack(&InfoStackLocation,
                                               pFileObject,
                                               pDeviceRelations);
    
    /* Check if the stack location was successfuly initialized */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to intialize stack location\n");
        goto clean_up;
    }

    /* Query device capabilities */
    ntStatus = PnpQueryDeviceInformation(pDeviceObject, 
                                         &InfoStackLocation,
                                         (PVOID*)&pDeviceRelations);

    /* Check if operation succeded */
    if (!NT_SUCCESS(ntStatus) || pDeviceRelations == NULL)
    {
        DbgPrint("Failed to get device PDO\n");
        goto clean_up;
    }
    
    /* Display all the information */
    PrintDeviceObject(pDeviceRelations);

    /* We only copy DEVICE_OBJECT, and NOT DEVICE_RELATIONS structure */
    if (ulOutputUserBufferLength >= sizeof(DEVICE_OBJECT))
    {
        *pulReturnedLength = sizeof(DEVICE_OBJECT);
        RtlCopyMemory(pvOutBuffer, pDeviceRelations->Objects[0], sizeof(DEVICE_OBJECT));
    }
    else 
    {
        *pulReturnedLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
clean_up:
    return ntStatus;
}
/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
VOID PrintDeviceText(PWCHAR pwcDeviceText)
{
    ASSERT(pwcDeviceText != NULL);
    DbgPrint("Device text: %ws \n", pwcDeviceText);
}

/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS PnpInitDeviceTextInfoStack(PIO_STACK_LOCATION pIoStackLocation,
                                    PFILE_OBJECT pFileObject,
                                    PWCHAR pDeviceText)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(pDeviceText);
    
    if (pIoStackLocation == NULL)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    /* Zero it before we initialize */
    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));
    
    pIoStackLocation->MajorFunction = IRP_MN_QUERY_DEVICE_TEXT;
    pIoStackLocation->MajorFunction = IRP_MJ_PNP;
    pIoStackLocation->Parameters.QueryDeviceText.DeviceTextType = DeviceTextDescription;
    pIoStackLocation->FileObject = pFileObject;
    
    return ntStatus;
}
/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS QueryDeviceText(PDEVICE_OBJECT pDeviceObject,
                         PIRP pIrp,
                         PFILE_OBJECT pFileObject,
                         ULONG ulOutputUserBufferLength,
                         PVOID pvOutBuffer,
                         PULONG pulReturnedLength)
{
    NTSTATUS           ntStatus          = STATUS_SUCCESS;
    PWCHAR             pwcDeviceText     = NULL;
    IO_STACK_LOCATION  InfoStackLocation;
    IO_STATUS_BLOCK    IoStatusBlock;

    ASSERT(pulReturnedLength);

    /* Build the stack location */
    ntStatus = PnpInitDeviceTextInfoStack(&InfoStackLocation,
                                          pFileObject,
                                          pwcDeviceText);

    /* Check if operation was succesfull */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to initialize stack location\n");
        goto clean_up;
    }

    ntStatus = PnpQueryDeviceInformation(pDeviceObject,
                                         &InfoStackLocation,
                                         (PVOID*)&pwcDeviceText);

    /* If operation was unsucessfull, just return error status */
    if (!NT_SUCCESS(ntStatus) || pwcDeviceText == NULL)
    {
        DbgPrint("Failed to get device PDO\n");
        goto clean_up;
    }

    /* Print the device text */
    PrintDeviceText(pwcDeviceText);

    /* Make sure that we have enough space */
    if (ulOutputUserBufferLength >= (wcslen(pwcDeviceText) / sizeof(WCHAR)))
    {
        *pulReturnedLength = wcslen(pwcDeviceText) / sizeof(WCHAR);
        RtlCopyMemory(pvOutBuffer, pwcDeviceText, wcslen(pwcDeviceText) / sizeof(WCHAR));
    }
    /* User buffer is to small */
    else
    {
        *pulReturnedLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
clean_up:
    return ntStatus;
}
/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS PrintResourceRequirements(PIO_RESOURCE_REQUIREMENTS_LIST pReqList)
{
    NTSTATUS                Status              = STATUS_SUCCESS;
    PIO_RESOURCE_LIST       pResourceList       = NULL;
    PIO_RESOURCE_DESCRIPTOR pResourceDescriptor = NULL;
    ULONG                   AltListCount        = 0;
    ULONG                   ResListCount        = 0;
    CM_RESOURCE_TYPE        x;
    
    ASSERT(pReqList);
    for (AltListCount = 0; AltListCount < pReqList->AlternativeLists; AltListCount++)
    {
        pResourceList = (PIO_RESOURCE_LIST)pReqList->List + AltListCount;

        for (ResListCount = 0; ResListCount < pResourceList->Count; ResListCount++)
        {
            pResourceDescriptor = (PIO_RESOURCE_DESCRIPTOR)pResourceList->Descriptors + ResListCount;

            switch (pResourceDescriptor->Type)
            {
                case CmResourceTypeNull:
                    break;
                case CmResourceTypePort:
                    DbgPrint("CmResourceTypePort: Alignment 0x%x\n", pResourceDescriptor->u.Port.Alignment);
                    DbgPrint("CmResourceTypePort: Length 0x%x\n", pResourceDescriptor->u.Port.Length);
                    DbgPrint("CmResourceTypePort: MaximumAddress 0x%x\n", pResourceDescriptor->u.Port.MaximumAddress);
                    DbgPrint("CmResourceTypePort: MinimumAddress 0x%x\n", pResourceDescriptor->u.Port.MinimumAddress);
                    break;
                case CmResourceTypeInterrupt:
                    DbgPrint("CmResourceTypeInterrupt: AffinityPolicy 0x%x\n", pResourceDescriptor->u.Interrupt.AffinityPolicy);
                    DbgPrint("CmResourceTypeInterrupt: MaximumVector 0x%x\n", pResourceDescriptor->u.Interrupt.MaximumVector);
                    DbgPrint("CmResourceTypeInterrupt: MinimumVector 0x%x\n", pResourceDescriptor->u.Interrupt.MinimumVector);
                    DbgPrint("CmResourceTypeInterrupt: PriorityPolicy 0x%x\n", pResourceDescriptor->u.Interrupt.PriorityPolicy);
                    DbgPrint("CmResourceTypeInterrupt: TargetedProcessors 0x%x\n", pResourceDescriptor->u.Interrupt.TargetedProcessors);
                    break;
                case CmResourceTypeMemory:
                    DbgPrint("CmResourceTypeMemory: Alignment 0x%x\n", pResourceDescriptor->u.Memory.Alignment);
                    DbgPrint("CmResourceTypeMemory: Length 0x%x\n", pResourceDescriptor->u.Memory.Length);
                    DbgPrint("CmResourceTypeMemory: MaximumAddress 0x%x\n", pResourceDescriptor->u.Memory.MaximumAddress);
                    DbgPrint("CmResourceTypeMemory: MinimumAddress 0x%x\n", pResourceDescriptor->u.Memory.MinimumAddress);
                    break;
                case CmResourceTypeDma: 
                    DbgPrint("CmResourceTypeDma: MaximumChannel 0x%x\n", pResourceDescriptor->u.Dma.MaximumChannel);
                    DbgPrint("CmResourceTypeDma: MinimumChannel 0x%x\n", pResourceDescriptor->u.Dma.MinimumChannel);
                    break;
                case CmResourceTypeDeviceSpecific:
                    break;
                case CmResourceTypeBusNumber: 
                    DbgPrint("CmResourceTypeBusNumber: Alignment 0x%x\n", pResourceDescriptor->u.BusNumber.Length);
                    DbgPrint("CmResourceTypeBusNumber: Length 0x%x\n", pResourceDescriptor->u.BusNumber.MaxBusNumber);
                    DbgPrint("CmResourceTypeBusNumber: Alignment 0x%x\n", pResourceDescriptor->u.BusNumber.MinBusNumber);
                    DbgPrint("CmResourceTypeBusNumber: Length 0x%x\n", pResourceDescriptor->u.BusNumber.Reserved);
                    break;
                case CmResourceTypeMemoryLarge:
                    break;
                    //case CmResourceTypeConfigData: 
                case CmResourceTypeNonArbitrated: 
                    break;
                case CmResourceTypeDevicePrivate:
                    DbgPrint("CmResourceTypeBusNumber: Alignment 0x%x\n", pResourceDescriptor->u.DevicePrivate.Data);
                    break;
                case CmResourceTypePcCardConfig:
                    break;
                case CmResourceTypeMfCardConfig:
                    break;
                default:
                    Status = STATUS_NOT_SUPPORTED;
                    break;
            }
        }
    }
    return Status;
}
/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS PnpIntDeviceResourceRequirementsInfoStack(PIO_STACK_LOCATION pIoStackLocation,
                                                   PFILE_OBJECT pFileObject,
                                                   PIO_RESOURCE_REQUIREMENTS_LIST pResoureRequirements)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pResoureRequirements);

    if (pIoStackLocation == NULL)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));
    pIoStackLocation->MinorFunction = IRP_MN_QUERY_RESOURCE_REQUIREMENTS;
    pIoStackLocation->MajorFunction = IRP_MJ_PNP;
    pIoStackLocation->FileObject = pFileObject;

    return ntStatus;
}

/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS QueryResourceRequirements(PDEVICE_OBJECT pDeviceObject,
                                   PIRP pIrp,
                                   PFILE_OBJECT pFileObject,
                                   ULONG ulOutputUserBufferLength,
                                   PVOID pvOutBuffer,
                                   PULONG pulReturnedLength)
{
    NTSTATUS                       ntStatus              = STATUS_SUCCESS;
    PIO_RESOURCE_REQUIREMENTS_LIST pResourceRequirements = NULL;
    IO_STACK_LOCATION              InfoStackLocation; 
    IO_STATUS_BLOCK                IoStatusBlock;

    ASSERT(pulReturnedLength);

    ntStatus = PnpIntDeviceResourceRequirementsInfoStack(&InfoStackLocation,
                                                         pFileObject,
                                                         pResourceRequirements);
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to initializer stack location\n");
        goto clean_up;
    }

    ntStatus = PnpQueryDeviceInformation(pDeviceObject,
                                         &InfoStackLocation, 
                                         (PVOID*)&pResourceRequirements);

    if (!NT_SUCCESS(ntStatus) || pResourceRequirements == NULL)
    {
        DbgPrint("Failed to get device PDO");
        goto clean_up;
    }

    PrintResourceRequirements(pResourceRequirements);

    if (ulOutputUserBufferLength > sizeof(IO_RESOURCE_REQUIREMENTS_LIST))
    {
        *pulReturnedLength = sizeof(IO_RESOURCE_REQUIREMENTS_LIST);
        RtlCopyMemory(pvOutBuffer, pResourceRequirements, sizeof(IO_RESOURCE_REQUIREMENTS_LIST));
    }
    else 
    {
        *pulReturnedLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
clean_up:
    return ntStatus;
}

/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
VOID PrintDeviceState(PPNP_DEVICE_STATE pDeviceState)
{
    ASSERT(pDeviceState);

    DbgPrint("DeviceState: %08X\n", pDeviceState);
}

NTSTATUS PnpInitDeviceStateInfoStack(PIO_STACK_LOCATION pIoStackLocation,
                                     PFILE_OBJECT pFileObject,
                                     PPNP_DEVICE_STATE pDeviceState)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (pIoStackLocation == NULL)
    {
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));
    pIoStackLocation->MinorFunction = IRP_MN_QUERY_PNP_DEVICE_STATE;
    pIoStackLocation->MajorFunction = IRP_MJ_PNP;

    return ntStatus;
}

/*++
 * @name 
 *
 * @description  
 *
 * @param 
 *        
 *
 * @param 
 *        
 *
 * @return
 *        
 *
 * @remarks 
 *
 *--*/
NTSTATUS QueryDeviceState(PDEVICE_OBJECT pDeviceObject,
                          PIRP pIrp,
                          PFILE_OBJECT pFileObject,
                          ULONG ulOutputUserBufferLength,
                          PVOID pvOutBuffer,
                          PULONG pulReturnedLength)
{
    NTSTATUS          ntStatus     = STATUS_SUCCESS;
    PPNP_DEVICE_STATE  pDeviceState = NULL;
    IO_STACK_LOCATION InfoStackLocation;
    IO_STATUS_BLOCK   IoStatusBlock;

    ASSERT(pulReturnedLength);

    /* Initialize stack location for device state */
    ntStatus = PnpInitDeviceStateInfoStack(&InfoStackLocation,
                                           pFileObject,
                                           pDeviceState);
    /* Check if operation was successfull */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to initialize stack location \n");
        goto clean_up;
    }

    /* Query device capabilities */
    ntStatus = PnpQueryDeviceInformation(pDeviceObject,
                                         &InfoStackLocation,
                                         (PVOID*)&pDeviceState);
    /* Chekc if operation was successfull */
    if (!NT_SUCCESS(ntStatus) || pDeviceState == NULL)
    {
        DbgPrint("Failed to get PDO\n");
        goto clean_up;
    }

    /* Print the device state */
    PrintDeviceState(pDeviceState);

    ASSERT(pulReturnedLength);

    /* Copy data in user mode */
    if (ulOutputUserBufferLength >= sizeof(PNP_DEVICE_STATE))
    {
        *pulReturnedLength = sizeof(PNP_DEVICE_STATE);
        RtlCopyMemory(pvOutBuffer, &pDeviceState, sizeof(PNP_DEVICE_STATE));
    }
    else 
    {
        *pulReturnedLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

clean_up:
    return ntStatus;
}

/*++
 * @name
 *
 * @description  
 *
 * @param 
 *         
 *
 * @param 
 *       
 *
 * @return   
 *         
 *
 * @remarks 
 *
 *--*/

NTSTATUS (*DirectInfoFunctionTable[])(PDEVICE_OBJECT pDeviceObject,
                                      PIRP pIrp,
                                      PFILE_OBJECT pFileObject,
                                      ULONG ulInputBufferLength,
                                      PVOID pvOutBuffer,
                                      PULONG pulOututBufferSize) = 
{
    QueryCapabilities,
    QueryDeviceRelations,
    QueryDeviceText,
    QueryResourceRequirements,
    QueryDeviceState
    /* Add more */
};

/*++
 * @name QueryCapabilities
 *
 * @description  
 *
 * @param 
 *         
 *
 * @param 
 *       
 *
 * @return   
 *         
 *
 * @remarks 
 *
 *--*/

NTSTATUS DirectDeviceInformation(IN PDEVICE_OBJECT pDeviceObject,
                                 IN PIRP pIrp,
                                 IN PFILE_OBJECT pFileObject)
{   
    PVOID                    pDirectDeviceInfo;
    PIO_STACK_LOCATION       pStackLocation;
    NTSTATUS                 Status             = STATUS_SUCCESS;
    ULONG                    OutputBufferLength = 0;
    ULONG                    InputBufferLength  = 0;
    ULONG                    ReturnedLength     = 0;
    ULONG                    ulRequestedInformation = 0;
    
    pStackLocation     = IoGetCurrentIrpStackLocation(pIrp); 
    InputBufferLength  = pStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Check if we have a command */
    if (InputBufferLength >= sizeof(ULONG))
    {

        /* Copy data form user mode */
        RtlCopyMemory(&ulRequestedInformation, pIrp->AssociatedIrp.SystemBuffer, sizeof(ULONG));
        
        /*Get the number of function in the table, and check if we don't exced the limit */
        if (ulRequestedInformation > (sizeof(DirectInfoFunctionTable) / sizeof(DirectInfoFunctionTable[0])))
        {    
            Status = STATUS_UNSUCCESSFUL;
            goto clean_up;
        }
    }

    /* We must have 4 bytes length, so we can't proced */
    else 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }
    /* Allocate memory for the output buffer */
    pDirectDeviceInfo = (PVOID)ExAllocatePoolWithTag(NonPagedPool, OutputBufferLength, 'DEVP');

    /* Make a check if we did not past out of bounds */
    Status = (*DirectInfoFunctionTable[ulRequestedInformation])(pDeviceObject,
                                                      pIrp, 
                                                      pFileObject,
                                                      OutputBufferLength,
                                                      pDirectDeviceInfo,
                                                      &ReturnedLength);

    /* Check if operation was succesful*/
    if (NT_SUCCESS(Status))
    {
        /* Update the length */
        ulBytesReturned = ReturnedLength;
    }
    else
    {
        /* Set returned length to 0 */
        ulBytesReturned = 0;
    }
    ExFreePoolWithTag(pDirectDeviceInfo, 'DEVP');
clean_up:
    return Status;
}
