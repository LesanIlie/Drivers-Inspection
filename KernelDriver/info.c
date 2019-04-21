#include "driver.h"

/* Used in IoCompleteRequest to return number of bytes copied in user mode */
extern ULONG ulBytesReturned;

/*++
 * @name PnpInitPDOStack
 *
 * @description  Initialize stack location for getting PDO
 *
 * @param pIoStackLocation
 *        Pointer to a IO_STACK_LOCATION to be initialized
 *
 * @param pFileObject
 *        Pointer to a FILE_OBJECT belongs to the driver that query his PDO
 *
 * @return Return a status code from ntstatus.h or defined by user,
 *         if function succeded it return STATUS_SUCCESS  
 *
 * @remarks This function intialize stack location for getting device PDO
 *
 *--*/
NTSTATUS PnpInitPDOStack(PIO_STACK_LOCATION pIoStackLocation, 
                         PFILE_OBJECT pFileObject)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

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
NTSTATUS GenericDeviceInformation(PDEVICE_OBJECT pDeviceObject,
                                  PIRP pIrp,
                                  DEVICE_REGISTRY_PROPERTY DeviceRegistryProperty,
                                  PVOID pDeviceProperty,
                                  PFILE_OBJECT pFileObject,
                                  PULONG  pOutputLength)
{
    PDEVICE_OBJECT     pDevicePDO           = NULL;
    NTSTATUS           ntStatus             = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIoStackLocation     = NULL;
    ULONG              ulResultLength       = 0;
    ULONG              ulOutputBufferLength = 0;
    IO_STACK_LOCATION  InfoStackLocation;
    PDEVICE_RELATIONS  pvDeviceRelation     = NULL;
    /* Get current stack location */
    pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Get the size of the buffer */
    ulOutputBufferLength = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Initialize stack location for PDO query */
    ntStatus = PnpInitPDOStack(&InfoStackLocation, pFileObject);
   
    /* Check if the stack location was successfully initialized */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to initialize stack location\n");
        goto clean_up;
    }
    
    /* Get device PDO */
    ntStatus = PnpQueryDeviceInformation(pDeviceObject, 
                                         &InfoStackLocation,
                                         (PVOID*)&pvDeviceRelation);
   
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to get device PDO\n");
        goto clean_up;
    }

    /* Access the struct to, get the PDO */
    pDevicePDO = (PDEVICE_OBJECT)pvDeviceRelation->Objects[0];
    ntStatus = IoGetDeviceProperty(pDevicePDO,
                                   DeviceRegistryProperty, 
                                   ulOutputBufferLength,
                                   pDeviceProperty,
                                   &ulResultLength);

    /* Check if the operation was successful */
    if (!NT_SUCCESS(ntStatus))
    {
        *pOutputLength = 0;
        DbgPrint("Failed to get device property with status 0x%x\n", ntStatus);
        goto clean_up;
    }

    /* Check if buffer fits in destination */
    if (ulResultLength > ulOutputBufferLength)
    {
        *pOutputLength = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else 
    {
        *pOutputLength = ulResultLength;
    }

clean_up:
        return ntStatus;
}

/*++
 * @name DeviceDescriptionFunction
 *
 * @description Request a string that describing the device.
 *
 * @param pDeviceObject
 *        Device object being queryed    
 *
 * @param pIrp
 *        Pointer to device driver  
 *
 * @param pFileObject
 *        Pointer to FILE_OBJECT associated with this DEVICE_OBJECT
 *
 * @param pDeviceProperty
 *        Pointer to output buffer
 *
 * @param pOutputLength
 *        Pointer to output length of the buffer
 *
 * @return Return a status code from ntstatus.h or defined by user,
 *         if function succeded it return STATUS_SUCCESS             
 *
 * @remarks This function return a pointer to a WCHAR if soucced
 *
 *--*/
NTSTATUS DeviceDescriptionFunction(PDEVICE_OBJECT pDeviceObject, 
                                   PIRP pIrp,
                                   PFILE_OBJECT pFileObject,
                                   PVOID pDeviceProperty,
                                   PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyDeviceDescription,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);

    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        DbgPrint("DeviceDescription: %ws \n", pDeviceProperty);
        *pOutputLength = ulOutputReturnedLength;
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS HardwareIDFunction(PDEVICE_OBJECT pDeviceObject, 
                            PIRP pIrp,
                            PFILE_OBJECT pFileObject,
                            PVOID pDeviceProperty,
                            PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyHardwareID,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        DbgPrint("HardwareID: %ws \n", pDeviceProperty);
        *pOutputLength = ulOutputReturnedLength;
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS CompatibleIDsFunction(PDEVICE_OBJECT pDeviceObject, 
                               PIRP pIrp,
                               PFILE_OBJECT pFileObject,
                               PVOID pDeviceProperty,
                               PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyCompatibleIDs,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        DbgPrint("CompatibleIDs: %ws \n", pDeviceProperty);
        *pOutputLength = ulOutputReturnedLength; 
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS BootConfigurationFunction(PDEVICE_OBJECT pDeviceObject, 
                                   PIRP pIrp,
                                   PFILE_OBJECT pFileObject,
                                   PVOID pDeviceProperty,
                                   PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyBootConfiguration,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        /* Add more hanling here */
        DbgPrint("BusNumber: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->BusNumber);
        DbgPrint("InterfaceType: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->InterfaceType);
        *pOutputLength = sizeof(CM_RESOURCE_LIST);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS BootConfigurationTranslatedFunction(PDEVICE_OBJECT pDeviceObject, 
                                             PIRP pIrp,
                                             PFILE_OBJECT pFileObject,
                                             PVOID pDeviceProperty,
                                             PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyBootConfigurationTranslated,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        /* Add more handling code here */
        *pOutputLength = sizeof(CM_RESOURCE_LIST);
        DbgPrint("BusNumber: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->BusNumber);
        DbgPrint("BusNumber: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->InterfaceType);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS ClassNameFunction(PDEVICE_OBJECT pDeviceObject, 
                           PIRP pIrp,
                           PFILE_OBJECT pFileObject,
                           PVOID pDeviceProperty,
                           PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyClassName,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("ClassName: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS ClassGuidFunction(PDEVICE_OBJECT pDeviceObject, 
                           PIRP pIrp,
                           PFILE_OBJECT pFileObject,
                           PVOID pDeviceProperty,
                           PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyClassGuid,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
         //DbgPrint("ClassGuid: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS DriverKeyNameFunction(PDEVICE_OBJECT pDeviceObject, 
                               PIRP pIrp, 
                               PFILE_OBJECT pFileObject,
                               PVOID pDeviceProperty,
                               PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyDriverKeyName,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("DriverKeyName: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS ManufacturerFunction(PDEVICE_OBJECT pDeviceObject, 
                              PIRP pIrp,
                              PFILE_OBJECT pFileObject,
                              PVOID pDeviceProperty,
                              PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyManufacturer,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("Manufacturer: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS FriendlyNameFunction(PDEVICE_OBJECT pDeviceObject, 
                              PIRP pIrp,
                              PFILE_OBJECT pFileObject,
                              PVOID pDeviceProperty,
                              PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyFriendlyName,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("FriendlyName: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS LocationInformationFunction(PDEVICE_OBJECT pDeviceObject, 
                                     PIRP pIrp, 
                                     PFILE_OBJECT pFileObject,
                                     PVOID pDeviceProperty,
                                     PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyLocationInformation,
                                      (PVOID)pDeviceProperty,
                                      pFileObject, 
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("LocationInformation: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS PhysicalDeviceObjectNameFunction(PDEVICE_OBJECT pDeviceObject, 
                                          PIRP pIrp, 
                                          PFILE_OBJECT pFileObject,
                                          PVOID pDeviceProperty,
                                          PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyPhysicalDeviceObjectName,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("PhysicalDeviceObjectName: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS BusTypeGuidFunction(PDEVICE_OBJECT pDeviceObject, 
                             PIRP pIrp,
                             PFILE_OBJECT pFileObject,
                             PVOID pDeviceProperty,
                             PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyBusTypeGuid,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Bus type guid %08X\n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS LegacyBusTypeFunction(PDEVICE_OBJECT pDeviceObject, 
                               PIRP pIrp,
                               PFILE_OBJECT pFileObject,
                               PVOID pDeviceProperty,
                               PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyLegacyBusType,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
         *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Legacy bus type %08X\n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS BusNumberFunction(PDEVICE_OBJECT pDeviceObject, 
                           PIRP pIrp,
                           PFILE_OBJECT pFileObject,
                           PVOID pDeviceProperty,
                           PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyBusNumber,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
                                    
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("BusNumber: %X \n", *(PULONG)pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS EnumeratorNameFunction(PDEVICE_OBJECT pDeviceObject, 
                                PIRP pIrp,
                                PFILE_OBJECT pFileObject,
                                PVOID pDeviceProperty,
                                PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyEnumeratorName,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("EnumeratorName: %ws \n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS AddressFunction(PDEVICE_OBJECT pDeviceObject, 
                         PIRP pIrp,
                         PFILE_OBJECT pFileObject,
                         PVOID pDeviceProperty,
                         PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyAddress,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("Address: %X \n", *(PULONG)pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS UINumberFunction(PDEVICE_OBJECT pDeviceObject, 
                          PIRP pIrp,
                          PFILE_OBJECT pFileObject,
                          PVOID pDeviceProperty,
                          PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyUINumber,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        DbgPrint("UINumber: %X \n", *(PULONG)pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS InstallStateFunction(PDEVICE_OBJECT pDeviceObject, 
                              PIRP pIrp,
                              PFILE_OBJECT pFileObject,
                              PVOID pDeviceProperty,
                              PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyInstallState,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Install state %08X\n", pDeviceProperty);
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS RemovalPolicyFunction(PDEVICE_OBJECT pDeviceObject, 
                               PIRP pIrp,
                               PFILE_OBJECT pFileObject,
                               PVOID pDeviceProperty,
                               PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyRemovalPolicy,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Removal policy %08X\n", pDeviceProperty);
    
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS ResourceRequirementsFunction(PDEVICE_OBJECT pDeviceObject, 
                                      PIRP pIrp,
                                      PFILE_OBJECT pFileObject,
                                      PVOID pDeviceProperty,
                                      PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyResourceRequirements,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Resource requirements %08X\n", pDeviceProperty);
    
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS AllocatedResourcesFunction(PDEVICE_OBJECT pDeviceObject, 
                                    PIRP pIrp,
                                    PFILE_OBJECT pFileObject,
                                    PVOID pDeviceProperty,
                                    PULONG pOutputLength)
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyAllocatedResources,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
        *pOutputLength = ulOutputReturnedLength;
        //DbgPrint("Allocate ressources %08X\n", pDeviceProperty);
    
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS ContainerIDFunction(PDEVICE_OBJECT pDeviceObject, 
                             PIRP pIrp,
                             PFILE_OBJECT pFileObject, 
                             PVOID pDeviceProperty,
                             PULONG pOutputLength)
{    
    NTSTATUS Status                 = STATUS_SUCCESS;
    ULONG    ulOutputReturnedLength = 0;
    
    Status = GenericDeviceInformation(pDeviceObject,
                                      pIrp,
                                      DevicePropertyContainerID,
                                      (PVOID)pDeviceProperty,
                                      pFileObject,
                                      &ulOutputReturnedLength);
    if (NT_SUCCESS(Status) && &ulOutputReturnedLength > 0)
    {
    }
    else 
    {
        *pOutputLength = 0;
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
NTSTATUS (*FunctionTable[])(PDEVICE_OBJECT pDeviceObject, 
                            PIRP pIrp, 
                            PFILE_OBJECT pFileObject,
                            PVOID pDeviceProperty,
                            PULONG pOutputLength) = 
  {
      DeviceDescriptionFunction,
      HardwareIDFunction,
      CompatibleIDsFunction,
      BootConfigurationFunction,
      BootConfigurationTranslatedFunction,
      ClassNameFunction,
      ClassGuidFunction,
      DriverKeyNameFunction,
      ManufacturerFunction, 
      FriendlyNameFunction,
      LocationInformationFunction, 
      PhysicalDeviceObjectNameFunction, 
      BusTypeGuidFunction,
      LegacyBusTypeFunction, 
      BusNumberFunction, 
      EnumeratorNameFunction,
      AddressFunction, 
      UINumberFunction, 
      InstallStateFunction, 
      RemovalPolicyFunction,
      ResourceRequirementsFunction,
      AllocatedResourcesFunction,
      ContainerIDFunction,
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
NTSTATUS DeviceProperty(IN PDEVICE_OBJECT pDeviceObject,
                        IN PIRP pIrp,
                        IN PFILE_OBJECT pFileObject)
{   
    PVOID                    pDeviceProperty;
    DEVICE_REGISTRY_PROPERTY DeviceRegistryProperty;
    PIO_STACK_LOCATION       pStackLocation;
    PDEVICE_OBJECT           pDevicePDO         = NULL;
    NTSTATUS                 Status             = STATUS_SUCCESS;
    ULONG                    OutputBufferLength = 0;
    ULONG                    InputBufferLength  = 0;
    ULONG                    InfoType;
    ULONG                    ReturnedLength     = 0;
    
    pStackLocation     = IoGetCurrentIrpStackLocation(pIrp); 
    InputBufferLength  = pStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Check if we have a command */
    if (InputBufferLength >= sizeof(ULONG))
    {

        /* Copy data form user mode */
        RtlCopyMemory(&InfoType, pIrp->AssociatedIrp.SystemBuffer, sizeof(ULONG));
        /* Check if parameters are valid */
        if (InfoType <= DevicePropertyContainerID)
        {
            DeviceRegistryProperty = (DEVICE_REGISTRY_PROPERTY)InfoType;
        }

        /* Invalid parameters sent form user mode */
        else 
        {    
            Status = STATUS_UNSUCCESSFUL;
            goto clean_up;
        }
    }

    /* We must have 4 bytes length, so we can't proceed */
    else 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }
    /* Allocate memory for the output buffer */
    pDeviceProperty = (PVOID)ExAllocatePoolWithTag(NonPagedPool, OutputBufferLength, 'DEVP');

    /* Make a check if we did not past out of bounds */
    Status = (*FunctionTable[DeviceRegistryProperty])(pDeviceObject,
                                                      pIrp, 
                                                      pFileObject,
                                                      pDeviceProperty,
                                                      &ReturnedLength);

    /* Check for success */
    if (NT_SUCCESS(Status))
    {
        /* Copy memory from kernel in user space */
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer,
                      pDeviceProperty,
                      ReturnedLength);
         ulBytesReturned = ReturnedLength;
    }
    else 
    {
        /* Set returned length to 0 */
        ulBytesReturned = 0;
    }
    ExFreePoolWithTag(pDeviceProperty, 'DEVP');
clean_up:
    return Status;
}
