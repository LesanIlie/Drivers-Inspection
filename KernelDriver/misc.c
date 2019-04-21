#include "stddcls.h"
#include "driver.h"

/* Function pointers */
PZWOPENDIRECTORYOBJECT    pZwOpenDirectoryObject   = NULL;
PZWQUERYDIRECTORYOBJECT   pZwQueryDirectoryObject  = NULL;
POBRRFERENCEOBJECTBYNAME  pObReferenceObjectByName = NULL;
POBQUERYNAMESTRING        pObQueryNameString       = NULL;
ULONG                     ulBytesReturned          = 0;
WCHAR                     twcBufferToSend[8192]    = {0};

/*++
 * @name DispatchCommon
 *
 * @description Dispatch routine
 *
 * @param pDeviceObject
 *        Pointer to the target DEVICE_OBJECT
 *
 * @param pIrp
 *        Pointer to IRP send to this device
 *
 * @return Always return STATUS_SUCCESS
 *
 * @remarks This function always complete the IRP with status STATUS_SUCCESS
 *
 *--*/
NTSTATUS DispatchCommon(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    DbgPrint("DispatchCommon called\n");

    /* Set the status and complete the IRP */
    pIrp->IoStatus.Status = ntStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ntStatus;
}

/*++
 * @name DispatchDefault
 *
 * @description Dispatch routine
 *
 * @param pDeviceObject
 *        Pointer to the target DEVICE_OBJECT
 *
 * @param pIrp
 *        Pointer to IRP send to this device
 *
 * @return Always return STATUS_NOT_SUPPORTED
 *
 * @remarks This function always complete the IRP with status STATUS_NOT_SUPPORTED
 *
 *--*/
NTSTATUS DispatchDefault(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

    DbgPrint("DispatchDefault called\n");

    /* Set the status and complete the IRP */
    pIrp->IoStatus.Status = ntStatus;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ntStatus;
}

/*++
 * @name CompleteRequest
 *
 * @description Completes an IRP
 *
 * @param pIrp
 *        Pointer to the IRP we want to complete
 *
 * @param ntStatus
 *        Status to set 
 *
 * @param ulpInformation
 *        Additional information
 *
 * @return Return the set by caller
 *
 * @remarks This function complete an IRP with status and information set by the caller
 *
 *--*/
NTSTATUS CompleteRequest(IN PIRP pIrp, IN NTSTATUS ntStatus, IN ULONG_PTR ulpInformation)
{
    /* Set the status and information and complete the IRP */
    pIrp->IoStatus.Status = ntStatus;
    pIrp->IoStatus.Information = ulpInformation;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return ntStatus;
}

/*++
 * @name GetSystemRoutineAddress
 *
 * @description Returns a pointer to a function in case of success
 *
 * @param pcwName
 *        Pointer to the string that represent the function name
 *
 * @return Void pointer
 *
 * @remarks Return a pointer to a function specified in unicode string 
 *
 *--*/
PVOID GetSystemRoutineAddress(IN PCWSTR pcwName)
{
    UNICODE_STRING usRoutineName;
    PVOID          pvRoutineAddress = NULL;

    RtlInitUnicodeString(&usRoutineName, pcwName);
    
    /* Return a pointer to a function specified in unicode string */
    pvRoutineAddress = MmGetSystemRoutineAddress(&usRoutineName);

    return pvRoutineAddress;
}

/*++
 * @name PnpBuildIrp
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS PnpBuildIrp(IN PDEVICE_OBJECT pTargetDeviceObject, OUT PIRP *ppIrp)
{
    NTSTATUS           ntStatus         = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIoStackLocation = NULL;

    /**
     * Alocate one more stack location for this irp, so the calling driver 
     * don't need to allocate a stack location for itself
     * https://msdn.microsoft.com/en-us/library/windows/hardware/ff548257%28v=vs.85%29.aspx
     */
    *ppIrp = IoAllocateIrp(pTargetDeviceObject->StackSize+1, FALSE);
    
    /* Check if allocation succeded */
    if (*ppIrp == NULL)
    {
        DbgPrint("Failed to allocate memory for IRP\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    /* Zero next stack location */
    pIoStackLocation = IoGetNextIrpStackLocation(*ppIrp);
    RtlZeroMemory(pIoStackLocation, sizeof(IO_STACK_LOCATION));

    return ntStatus;
}

/*++
 * @name PnpQueryDeviceInformation
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS PnpQueryDeviceInformation(IN PDEVICE_OBJECT pTargetDeviceObject,
                                   IN PIO_STACK_LOCATION pIoSourceStackLocation,
                                   OUT PVOID *pvOutputData)
{
    PIRP               pIrp                 = NULL;
    NTSTATUS           ntStatus             = STATUS_SUCCESS;
    PIO_STACK_LOCATION pDestIoStackLocation = NULL;
    PDEVICE_RELATIONS  pDeviceRelations     = NULL;
    IO_STATUS_BLOCK    IoStatusBlock;
    KEVENT             Event;
    
    /* Build the IRP */
    ntStatus = PnpBuildIrp(pTargetDeviceObject, &pIrp);

    /* Check if operation succeded */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Error allocating the Irp \n");
        return ntStatus;
    }

    /* Associate irp with a thread */
    pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
    pIrp->RequestorMode       = KernelMode;
    pIrp->IoStatus.Status     = STATUS_NOT_SUPPORTED;
    pIrp->UserEvent           = &Event;
    pIrp->UserIosb            = &IoStatusBlock;

    /* Get next stack location */
    pDestIoStackLocation = IoGetNextIrpStackLocation(pIrp);
    
    /* Copy parameters */
    RtlCopyMemory(pDestIoStackLocation, pIoSourceStackLocation, sizeof(IO_STACK_LOCATION));
    
    /* Forward the irp to the lower driver */
    ntStatus = ForwardIrpAndWait(pTargetDeviceObject, pIrp);

    /* Return a pointer to the required data in case of success*/
    if (NT_SUCCESS(ntStatus) && pIrp->IoStatus.Information)
    {
        DbgPrint("PnpQueryDeviceInformation succeded\n");
        *pvOutputData =  (PVOID)pIrp->IoStatus.Information;
    }

    /* Free the irp */
    if (pIrp)
        IoFreeIrp(pIrp);

    return ntStatus;
}

/*++
 * @name ForwardIrpAndWait
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS ForwardIrpAndWait(IN PDEVICE_OBJECT pTargetDeviceObject, IN PIRP pIrp)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KEVENT   Event;
    
    /* Initialize the event that we will wait for */
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    /* Install a completion routine  */
    IoSetCompletionRoutine(pIrp,
                           (PIO_COMPLETION_ROUTINE)ForwardedIrpCompletion,
                           (PVOID)&Event,
                           TRUE,
                           TRUE, 
                           TRUE);

    /* Send the IRP to the target driver */
    ntStatus = IoCallDriver(pTargetDeviceObject, pIrp);

    /* If the driver returned a pending status, then we need to wait for the event */
    if (ntStatus == STATUS_PENDING)
    {
        /* Wait reason must be KernelMode to prevent the stack to be pageout */
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        ntStatus = pIrp->IoStatus.Status;
    }
    return ntStatus;
}

/*++
 * @name ForwardedIrpCompletion
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS ForwardedIrpCompletion(IN PDEVICE_OBJECT pTargetDeviceObject, 
                                IN PIRP pIrp,
                                IN PKEVENT Event)
{
    /*
     * If the pending flag is true, set the event, otherwise in unnecessary
     * and a performance bottleneck
     */
    if (pIrp->PendingReturned)
    {
        KeSetEvent(Event, 0, FALSE);
    }
    /* Stop completion */
    return STATUS_MORE_PROCESSING_REQUIRED;
}

/*++
 * @name GetDriverObjectList
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS GetDriverObjectList(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS              ntStatus             = STATUS_SUCCESS;
    HANDLE                hDirectory           = NULL;
    POBJECT_NAMETYPE_INFO pBuffer              = NULL;
    PDRIVER_OBJECT        pDriverObject        = NULL;
    PIO_STACK_LOCATION    pStackLocation       = NULL;
    ULONG                 ulContext            = 0;
    ULONG                 ulReturnedLength     = 0;
    OBJECT_ATTRIBUTES     ObjectAttributes;
    UNICODE_STRING        unDriverNameSpace;
    ULONG                 ulOutputBufferLength = 0;
    
    /* Get current stack location */
    pStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Save output buffer length */
    ulOutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Initialize the UNICODE_STRING with driver namespace */
    RtlInitUnicodeString(&unDriverNameSpace, L"\\Driver");
    
    /* Initialize object attributes that describe this driver namespace */
    InitializeObjectAttributes(&ObjectAttributes, &unDriverNameSpace, OBJ_CASE_INSENSITIVE, NULL, NULL);

    /* Get function pointers */
    pZwOpenDirectoryObject = (PZWOPENDIRECTORYOBJECT)GetSystemRoutineAddress(L"ZwOpenDirectoryObject");
    pZwQueryDirectoryObject = (PZWQUERYDIRECTORYOBJECT)GetSystemRoutineAddress(L"ZwQueryDirectoryObject");

    /* If one of the pointers is NULL, abort */
    if (pZwOpenDirectoryObject == NULL ||
        pZwQueryDirectoryObject == NULL)
    {
        DbgPrint("Error getting function pointer \n");
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }

    /* Get a handle to the "\\Driver\\" directory object */
    ntStatus = pZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &ObjectAttributes);

    /* Check for success */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ZwOpenDirectoryObject failed with status %08x \n");
        goto clean_up;
    }

    /* Allocate memory for 200 drivers */
    pBuffer = (POBJECT_NAMETYPE_INFO)ExAllocatePoolWithTag(NonPagedPool ,sizeof(OBJECT_NAMETYPE_INFO)*200, 'drv ');
    
    /* Check if allocation succedded */
    if (pBuffer == NULL)
    {
        DbgPrint("Allocation failed \n", ntStatus);
        goto clean_up;
    }

    /* 
     * https://msdn.microsoft.com/en-us/library/bb470238%28v=vs.85%29.aspx 
     * Note that we allocate enought memory, so for all the drivers in the "\\Driver\\" directory
     * We allocate memory for 200 entries
     */
    ntStatus = pZwQueryDirectoryObject(hDirectory, 
                                       pBuffer, 
                                       sizeof(OBJECT_NAMETYPE_INFO)*200,
                                       TRUE,
                                       TRUE,
                                       &ulContext, 
                                       &ulReturnedLength); 

    /* Check if query succeded */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ZwQueryDirectoryObject failed with status %08x \n", ntStatus);
        goto clean_up;
    }

    /* Zero the memory */
    RtlZeroMemory(twcBufferToSend, sizeof(twcBufferToSend));
    do 
    { 
        DbgPrint("Driver: %ws \n", pBuffer->ObjectName.Buffer);

        /* @TODO: Save the entire list of device objects */
        
        wcscat(twcBufferToSend,  pBuffer->ObjectName.Buffer);
        wcscat(twcBufferToSend,  L" ");
        /* 
         * Go to the next entry.
         * RestartScan must be FALSE, so we continue with the list
         */
        ntStatus = pZwQueryDirectoryObject(hDirectory, 
                                           pBuffer, 
                                           sizeof(OBJECT_NAMETYPE_INFO)*200,
                                           TRUE,
                                           FALSE,
                                           &ulContext, 
                                           &ulReturnedLength);
   }while (NT_SUCCESS(ntStatus));
    
    /* Check if we have space */
    if (ulOutputBufferLength >= sizeof(twcBufferToSend))
    {
        /* Copy data in user mode */
         ntStatus = STATUS_SUCCESS;
         RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, twcBufferToSend, sizeof(twcBufferToSend));
         RtlZeroMemory(twcBufferToSend, sizeof(twcBufferToSend));
         ulBytesReturned = sizeof(twcBufferToSend);
    }
    else
    {
        /* Buffer is too small */
        ulBytesReturned = 0;
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
    
    /* Free the memory */
    ExFreePoolWithTag(pBuffer,  'drv ');
clean_up:
    /* Our exit condition is a unsucessful status, so we will return STATUS_SUCCESS every time */
    return ntStatus;
}

/*++
 * @name GetDeviceObjectsFromDriverObject
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS GetDeviceObjectsFromDriverObject(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS                  ntStatus             = STATUS_SUCCESS;
    PDRIVER_OBJECT            plDriverObject       = NULL;
    PDEVICE_OBJECT            plDeviceObject       = NULL;
    PWCHAR                    pDriverName          = NULL;
    PIO_STACK_LOCATION        pStackLocation       = NULL;
    POBJECT_TYPE              pIoDriverObjectType  = NULL;
    PDEVICE_EXTENSION         pDeviceExtension     = NULL;
    POBJECT_NAME_INFORMATION  pObjectNameInfo      = NULL;
    UNICODE_STRING            unDriverNameSpace    = {0};
    UNICODE_STRING            unDriverName         = {0};
    ULONG                     ulReturnedLength     = 0;
    ULONG                     ulInputBufferLength  = 0;
    ULONG                     ulOutputBufferLength = 0;
    ULONG                     ulReturnedBytes      = 0;

    /* Get current stack location */
    pStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Get the length for input/output buffer */
    ulInputBufferLength = pStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Get a pointer to device extension */
    pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;

    /* Get a pointer to the system routine */
    pObReferenceObjectByName = (POBRRFERENCEOBJECTBYNAME)GetSystemRoutineAddress(L"ObReferenceObjectByName");

    /* Why we can use "extern POBJECT_TYPE IoDriverObjectType" ?*/
    pIoDriverObjectType = (POBJECT_TYPE)ExAllocatePoolWithTag(NonPagedPool, sizeof (100), 'drv ');
   

    /* Check for valid object pointer */
    if (pObReferenceObjectByName == NULL)
    {
        DbgPrint("Error getting ObReferenceObjectByName function pointer: 0%08x\n", ntStatus);
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }

    /* Check if buffer is not too large */
    if (ulInputBufferLength == 0 || wcslen((PWCHAR)pIrp->AssociatedIrp.SystemBuffer) > 90)
    {
        DbgPrint("Invalid input length \n");
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }

    /* Always reinitialize unicode data strucure, so we can reuse every time we call this function */
    pDeviceExtension->unDriverName.Length        = 0;
    pDeviceExtension->unDriverName.MaximumLength = 0x100;
      
    /* Set up the path for the driver we are looking for */
    RtlAppendUnicodeToString(&pDeviceExtension->unDriverName, L"\\Driver\\");
    
    /* Copy rest of the path from user mode */
    RtlAppendUnicodeToString(&pDeviceExtension->unDriverName, (PWCHAR)pIrp->AssociatedIrp.SystemBuffer);
    
    /* Get the DRIVER_OBJECT from Object Manager */
    ntStatus = pObReferenceObjectByName(&pDeviceExtension->unDriverName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        FILE_ALL_ACCESS,
                                        (POBJECT_TYPE)pIoDriverObjectType,
                                        KernelMode,
                                        NULL, 
                                        (PVOID*)&plDriverObject);

    /* Check for valid status and Driver Object */
    if (!NT_SUCCESS(ntStatus) || plDriverObject == NULL)
    {
        DbgPrint("Driver object is NULL\n");
        goto clean_up;
    }
        
    /* Get the function pointer */
    pObQueryNameString = (POBQUERYNAMESTRING)GetSystemRoutineAddress(L"ObQueryNameString");

    /* Check if function pointer is valid */
    if (pObQueryNameString == NULL)
    {
        DbgPrint("Failed to get ObQueryNameString function pointer \n");
        goto clean_up;
    }

    /* Loop through the list of device objects */
    for (plDeviceObject = plDriverObject->DeviceObject; plDeviceObject != NULL; plDeviceObject = plDeviceObject->NextDevice)
    {
        /* Get first the length of the name */
        pObQueryNameString(plDeviceObject, NULL, 0, &ulReturnedBytes);
        pObjectNameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulReturnedBytes, 'list');

        /* Check if allocation succeed  */
        if (pObjectNameInfo == NULL)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto clean_up;
        }

        /* Zero it */
        RtlZeroMemory(pObjectNameInfo, ulReturnedBytes);
        
        /* Get the name string */
        ntStatus = pObQueryNameString(plDeviceObject, pObjectNameInfo, ulReturnedBytes, &ulReturnedLength);


        /*
         * Check for success and valid pointer 
         * Function can return STATUS_SUCCESS with NULL buffer, if we check on 
         * status value, we risk to derefernce a NULL pointer
         * Documentation say this: "If the given object is unnamed, or if the object name was not successfully acquired, 
         * ObQueryNameString sets Name.Buffer to NULL and sets Name.Length and Name.MaximumLength to zero."
         * http://fsfilters.blogspot.ro/2010/11/obquerynamestring-can-return-names-with.html
         */
        if (NT_SUCCESS(ntStatus) && pObjectNameInfo->Name.Buffer != NULL)
        {
            DbgPrint("Device Name: %ws\n", pObjectNameInfo->Name.Buffer);

            /* Put the data in the buffer */
            wcscat(twcBufferToSend, pObjectNameInfo->Name.Buffer);
            wcscat(twcBufferToSend, L" ");
        }
        else 
        {
            DbgPrint("Unknow device name\n");
        }

        /* Free the allocated memory */
        ExFreePoolWithTag(pObjectNameInfo, 'list');
    }

    /* Check if we have enough room */
    if (ulOutputBufferLength >= sizeof(twcBufferToSend))
    {
        /* Copy data in user mode */
        RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, twcBufferToSend, sizeof(twcBufferToSend));
        ulBytesReturned = sizeof(twcBufferToSend);
        RtlZeroMemory(twcBufferToSend, sizeof(twcBufferToSend));
    }
    else 
    {
        /* User buffer is too small */
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        ulBytesReturned = 0;
    }
clean_up:
    return ntStatus;
}

/*++
 * @name StoreDeviceObject
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS TraverseDriverObjectList(IN PDEVICE_OBJECT pDeviceObject,
                                  IN PIRP pIrp,
                                  IN PUNICODE_STRING pusDeviceToFind, 
                                  OUT PDEVICE_OBJECT *ppDeviceReturned)
{
    NTSTATUS              ntStatus             = STATUS_SUCCESS;
    NTSTATUS              ntLocalStatus        = STATUS_SUCCESS;
    HANDLE                hDirectory           = NULL;
    POBJECT_NAMETYPE_INFO pBuffer              = NULL;
    PDRIVER_OBJECT        pDriverObject        = NULL;
    PIO_STACK_LOCATION    pStackLocation       = NULL;
    PDEVICE_OBJECT        pRetDeviceObject     = NULL; 
    ULONG                 ulContext            = 0;
    ULONG                 ulReturnedLength     = 0;
    OBJECT_ATTRIBUTES     ObjectAttributes;
    UNICODE_STRING        unDriverNameSpace;

    *ppDeviceReturned = NULL;

    /* Initialize the UNICODE_STRING with driver namespace */
    RtlInitUnicodeString(&unDriverNameSpace, L"\\Driver");
    
    /* Intialize object attributes that discribe this driver namespace */
    InitializeObjectAttributes(&ObjectAttributes, &unDriverNameSpace, OBJ_CASE_INSENSITIVE, NULL, NULL);

    /* Get function pointers */
    pZwOpenDirectoryObject = (PZWOPENDIRECTORYOBJECT)GetSystemRoutineAddress(L"ZwOpenDirectoryObject");
    pZwQueryDirectoryObject = (PZWQUERYDIRECTORYOBJECT)GetSystemRoutineAddress(L"ZwQueryDirectoryObject");

    /* If one of the pointers is NULL, abort */
    if (pZwOpenDirectoryObject == NULL ||
        pZwQueryDirectoryObject == NULL)
    {
        DbgPrint("Error getting function pointer \n");
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }

    /* Get a handle to the "\\Driver\\" directory object */
    ntStatus = pZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &ObjectAttributes);

    /* Check for success */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ZwQueryDirecoryObject failed with status %08x \n");
        goto clean_up;
    }

    /* Allocate memory for 200 drivers */
    pBuffer = (POBJECT_NAMETYPE_INFO)ExAllocatePoolWithTag(NonPagedPool ,sizeof(OBJECT_NAMETYPE_INFO)*200, 'drv ');
    
    /* Check if allocation succedded */
    if (pBuffer == NULL)
    {
        DbgPrint("Allocation failed \n", ntStatus);
        goto clean_up;
    }

    /* 
     * https://msdn.microsoft.com/en-us/library/bb470238%28v=vs.85%29.aspx 
     * Note that we allocate enought memory, so for all the drivers in the "\\Driver\\" directory
     * We allocate memory for 200 entries
     */
    ntStatus = pZwQueryDirectoryObject(hDirectory, 
                                       pBuffer, 
                                       sizeof(OBJECT_NAMETYPE_INFO)*200,
                                       TRUE,
                                       TRUE,
                                       &ulContext, 
                                       &ulReturnedLength); 

    /* Check if query succeded */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ZwQueryDirectoryObject failed with status %08x \n", ntStatus);
        goto clean_up;
    }

    do 
    { 
        /* 
         * Go to the next entry.
         * RestartScan must be FALSE, so we continue with the list
         */
        ntStatus = pZwQueryDirectoryObject(hDirectory, 
                                           pBuffer, 
                                           sizeof(OBJECT_NAMETYPE_INFO)*200,
                                           TRUE,
                                           FALSE,
                                           &ulContext, 
                                           &ulReturnedLength);

        if (NT_SUCCESS(ntStatus))
        {
            ntLocalStatus = FindDeviceObject(pDeviceObject, 
                                             pIrp, 
                                             &pBuffer->ObjectName,
                                             pusDeviceToFind,
                                             &pRetDeviceObject);
            if (NT_SUCCESS(ntLocalStatus) && pRetDeviceObject != NULL)
            {
                /* We found the device object we are looking for */
                *ppDeviceReturned = pRetDeviceObject;
                
                /* Free the buffer */
                ExFreePoolWithTag(pBuffer,  'drv ');
                
                  /* Exit from while */
                goto clean_up;
            }
        }
   }while (NT_SUCCESS(ntStatus));
        
    /* Free the memory */
    ExFreePoolWithTag(pBuffer,  'drv ');
clean_up:
    /* Our exit condition is a unsucessful status, so we will return STATUS_SUCCESS every time */
    return ntStatus;
}

/*++
 * @name StoreDeviceObject
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks 
 *
 *--*/
NTSTATUS FindDeviceObject(IN PDEVICE_OBJECT pDeviceObject,
                          IN PIRP pIrp,
                          IN PUNICODE_STRING pusDriverObject,
                          IN PUNICODE_STRING pusDeviceToFind,
                          OUT PDEVICE_OBJECT *ppReturnedDeviceObject)
{
    NTSTATUS                  ntStatus             = STATUS_SUCCESS;
    PDRIVER_OBJECT            plDriverObject       = NULL;
    PDEVICE_OBJECT            plDeviceObject       = NULL;
    POBJECT_TYPE              pIoDriverObjectType  = NULL;
    PDEVICE_EXTENSION         pDeviceExtension     = NULL;
    POBJECT_NAME_INFORMATION  pObjectNameInfo      = NULL;
    UNICODE_STRING            unDriverNameSpace    = {0};
    UNICODE_STRING            unDriverName         = {0};
    ULONG                     ulReturnedLength     = 0;
    ULONG                     ulReturnedBytes      = 0;
    
    *ppReturnedDeviceObject = NULL;

    /* Get a pointer to device extension */
    pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;

    /* Get a pointer to the system routine */
    pObReferenceObjectByName = (POBRRFERENCEOBJECTBYNAME)GetSystemRoutineAddress(L"ObReferenceObjectByName");

    /* Why we can use "extern POBJECT_TYPE IoDriverObjectType" ?*/
    pIoDriverObjectType = (POBJECT_TYPE)ExAllocatePoolWithTag(NonPagedPool, sizeof (100), 'drv ');
   

    /* Check for valid object pointer */
    if (pObReferenceObjectByName == NULL)
    {
        DbgPrint("Error getting ObReferenceObjectByName function pointer: 0%08x\n", ntStatus);
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }

    /* Always reinitialize unicode data strucure, so we can reuse every time we call this function */
    pDeviceExtension->unDriverName.Length        = 0;
    pDeviceExtension->unDriverName.MaximumLength = 0x100;
      
    /* Set up the path for the driver we are looking for */
    RtlAppendUnicodeToString(&pDeviceExtension->unDriverName, L"\\Driver\\");
   
    /* Copy rest of the path from user mode */
    RtlAppendUnicodeToString(&pDeviceExtension->unDriverName, pusDriverObject->Buffer);
    
    /* Get the DRIVER_OBJECT from Object Manager */
    ntStatus = pObReferenceObjectByName(&pDeviceExtension->unDriverName,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        FILE_ALL_ACCESS,
                                        (POBJECT_TYPE)pIoDriverObjectType,
                                        KernelMode,
                                        NULL, 
                                        (PVOID*)&plDriverObject);

    /* Check for valid status and Driver Object */
    if (!NT_SUCCESS(ntStatus) || plDriverObject == NULL)
    {
        DbgPrint("Driver object is NULL\n");
        goto clean_up;
    }
        
    /* Get the function pointer */
    pObQueryNameString = (POBQUERYNAMESTRING)GetSystemRoutineAddress(L"ObQueryNameString");

    /* Check if function pointer is valid */
    if (pObQueryNameString == NULL)
    {
        DbgPrint("Failed to get ObQueryNameString function pointer \n");
        goto clean_up;
    }

    /* Loop throught the list of device objects */
    for (plDeviceObject = plDriverObject->DeviceObject; plDeviceObject != NULL; plDeviceObject = plDeviceObject->NextDevice)
    {
        /* Get first the length of the name */
        pObQueryNameString(plDeviceObject, NULL, 0, &ulReturnedBytes);
        pObjectNameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, ulReturnedBytes, 'list');

        /* Check if allocation succeded */
        if (pObjectNameInfo == NULL)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto clean_up;
        }

        /* Zero it */
        RtlZeroMemory(pObjectNameInfo, ulReturnedBytes);
        
        /* Get the name string */
        ntStatus = pObQueryNameString(plDeviceObject, pObjectNameInfo, ulReturnedBytes, &ulReturnedLength);


        /*
         * Check for success and valid pointer 
         * Function can return STATUS_SUCCESS with NULL buffer, if we check on 
         * status value, we risk to derefernce a NULL pointer
         * Documentation say this: "If the given object is unnamed, or if the object name was not successfully acquired, 
         * ObQueryNameString sets Name.Buffer to NULL and sets Name.Length and Name.MaximumLength to zero."
         * http://fsfilters.blogspot.ro/2010/11/obquerynamestring-can-return-names-with.html
         */
        if (NT_SUCCESS(ntStatus) && pObjectNameInfo->Name.Buffer != NULL)
        {
            DbgPrint("Device Name: %ws\n", pObjectNameInfo->Name.Buffer);

            /* Check if names are equal */
            if (RtlEqualUnicodeString((PCUNICODE_STRING)pObjectNameInfo, pusDeviceToFind, FALSE) == TRUE)
            {
                /* We found the device object we looking for */
                *ppReturnedDeviceObject = plDeviceObject;

                /* Exit from for */
                goto clean_up;
            }
        }
        else 
        {
            DbgPrint("Unknow device name\n");
        }

        /* Free the allocated memory */
        ExFreePoolWithTag(pObjectNameInfo, 'list');
    }
clean_up:
    return ntStatus;
}

/*++
 * @name StoreDeviceObject
 *
 * @description 
 *
 * @param 
 *        
 * @return 
 *
 * @remarks This function must be called before "DeviceProperty"
 *
 *--*/
NTSTATUS StoreDeviceObject(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS           ntStatus             = STATUS_SUCCESS;
    PDEVICE_EXTENSION  pDeviceExtension     = NULL;
    ULONG              ulInputBufferLength  = 0;
    ULONG              ulOutputBufferLength = 0;
    PIO_STACK_LOCATION pStackLocation       = NULL;
    
    /* Get current stack location */
    pStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Get the length of the input and output buffer */
    ulInputBufferLength = pStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Check if we have a valid length and buffer is not longer than allocated buffer */
    if (ulInputBufferLength == 0 || wcslen((PWCHAR)pIrp->AssociatedIrp.SystemBuffer) > 90)
    {
        DbgPrint("Error input buffer to small");
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }
    /* Get a pointer to the device extension */
    pDeviceExtension = (PDEVICE_EXTENSION) pDeviceObject->DeviceExtension;
    
    /* Always reinitialize unicode data strucure, so we can reuse every time we call this function */
    pDeviceExtension->usDeviceName.Length        = 0;
    pDeviceExtension->usDeviceName.MaximumLength = 0x100;
      
    /* Set up the path for the device we are looking for */
    RtlAppendUnicodeToString(&pDeviceExtension->usDeviceName, L"\\Device\\");

    /* Copy rest of the path from user mode */
    RtlAppendUnicodeToString(&pDeviceExtension->usDeviceName, (PWCHAR)pIrp->AssociatedIrp.SystemBuffer);

    /* 
     * Get the device object by name 
     * Note that we need to dereference the file object from the "Unload" routine
     * in order to unload the driver, otherwise our driver can't be unloaded.
     * We dereference just after "IoGetDeviceObjectPointer", so the problem that
     * can arise is that our device can disappear from memory suddenly.
     */
    ntStatus = IoGetDeviceObjectPointer(&pDeviceExtension->usDeviceName,
                                        FILE_READ_DATA,
                                        &pDeviceExtension->pFileObject,
                                        &pDeviceExtension->pDeviceObject);
    
    /*
     * If getting the device object failed, list all drivers loaded on the system
     * and try to this device object in the list of each device object list
     */
    if (!NT_SUCCESS(ntStatus))
    {
        /* Always set file object to NULL, to prevent call form ObDerefernceObject */
        pDeviceExtension->pFileObject = NULL;

        /* Try on hard way 
         * @TODO: File object with this metod is NULL, it may cause problems
         */
          ntStatus = TraverseDriverObjectList(pDeviceObject, 
                                              pIrp,
                                              &pDeviceExtension->usDeviceName,
                                              &pDeviceExtension->pDeviceObject);
        
        /*Check for success */
        if (!NT_SUCCESS(ntStatus))
        {
            DbgPrint("Failed to get the device object");
        }
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
 * @return 
 *
 * @remark
 *
 *--*/
NTSTATUS PrintDeviceInformation(IN PDEVICE_OBJECT pDeviceObject,
                                IN PIRP pIrp, 
                                IN BOOLEAN boDirectInfo)
{
    NTSTATUS           ntStatus             = STATUS_SUCCESS;
    PDEVICE_EXTENSION  pDeviceExtension     = NULL;
    ULONG              ulInputBufferLength  = 0;
    ULONG              ulOutputBufferLength = 0;
    PIO_STACK_LOCATION pStackLocation       = NULL;
    
    /* Get current stack location */
    pStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Get the length of the input and output buffer */
    ulOutputBufferLength = pStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    /* Check if we have a valid length and buffer is valid */
    if (ulOutputBufferLength == 0 || pIrp->AssociatedIrp.SystemBuffer == NULL)
    {
        DbgPrint("PrintDeviceInformation: Output buffer error\n");
        ntStatus = STATUS_UNSUCCESSFUL;
        goto clean_up;
    }
    /* Get a pointer to the device extension */
    pDeviceExtension = (PDEVICE_EXTENSION) pDeviceObject->DeviceExtension;
    
    /* 
     * Check if we have valid arguments 
     * File object can be NULL
     */
     if (pDeviceExtension->pDeviceObject == NULL)
     {
         ntStatus = STATUS_UNSUCCESSFUL;
         goto clean_up;
     }

    if (boDirectInfo ==  TRUE)
    {
        /* Send IOCTL to driver to get information */
        ntStatus = DirectDeviceInformation(pDeviceExtension->pDeviceObject, 
                                           pIrp,
                                           pDeviceExtension->pFileObject);
    }
    else 
    {
        /* Use IoGetDevicePropery to get information */
        ntStatus = DeviceProperty(pDeviceExtension->pDeviceObject, 
                                  pIrp,
                                  pDeviceExtension->pFileObject);
    }

    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to get information \n");
    }
    
    /* Release the reference on FILE_OBJECT, take extra care to not dereference two times */
    if (pDeviceExtension->pFileObject)
        ObDereferenceObject(pDeviceExtension->pFileObject);
clean_up:
    return ntStatus;
}