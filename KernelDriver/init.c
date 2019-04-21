#include "stddcls.h"
#include "driver.h"

/*++
 * @name AddDevice
 *
 * @description Called when driver is loaded
 *
 * @param pDriverObject
 *        Pointer to a DRIVER_OBJECT data structure that represent the driver 
 *
 * @return Return a status code from ntstatus.h or defined by user,
 *         if function succeded it return STATUS_SUCCESS 
 *
 * @remarks This function at PASSIVE_LEVEL when driver is loaded. It create a device object 
 *          and a symbolic link accesible from user mode
 *
 *--*/
NTSTATUS AddDevice(IN PDRIVER_OBJECT pDriverObject)
{
    NTSTATUS          ntStatus         = STATUS_SUCCESS;
    PDEVICE_OBJECT    pDeviceObject    = NULL;
    PDEVICE_EXTENSION pDeviceExtension = NULL;
    UNICODE_STRING    usDeviceName;
    UNICODE_STRING    usDosDeviceName;

    /* Initialize device name and symbolic link names */
    RtlInitUnicodeString(&usDeviceName, L"\\Device\\DriverInfo");
    RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\DriverInfo");

    /* Function creates a device object */
    ntStatus = IoCreateDevice(pDriverObject,
                              sizeof(DEVICE_EXTENSION),
                              &usDeviceName,
                              FILE_DEVICE_UNKNOWN,
                              0, 
                              FALSE, 
                              &pDeviceObject);

    /* Check if operation was succesful */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to create a device object\n");
        return ntStatus;
    }

    /* Get a pointer to the device extension */
    pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
 
    /* 
     * Must free this buffer in driver Unload, so we don't leak any memory
     */
    pDeviceExtension->unDriverName.Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, sizeof(WCHAR)*100);
    pDeviceExtension->usDeviceName.Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, sizeof(WCHAR)*100);;

    
    /* Chose the method of comunication */
    pDeviceObject->Flags |= DO_BUFFERED_IO;

    /* Tell to IO Manger to send IRP_MJ_POWER at PASSIVE_LEVEL */
    pDeviceObject->Flags |= DO_POWER_PAGABLE;

    /* Clear this flag, so other device can attach to our or open a handle */
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    /* Create a symbolic link for comunication with user mode */
    ntStatus = IoCreateSymbolicLink(&usDosDeviceName, &usDeviceName);

    /* If the operation was unsuccesfull, delete the device object */
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("Failed to create a symbolic link\n");
        IoDeleteDevice(pDeviceObject);
    }
    return ntStatus;
}
/*++
 * @name Unload
 *
 * @description Called when driver is unloaded
 *
 * @param pDriverObject
 *        Pointer to a DRIVER_OBJECT data structure that represent the driver 
 *
 * @return VOID
 *
 * @remarks This routine is called prior unloading the driver, it does nothing,
 *          but we need to have it to suport dynamic driver unload
 *
 *--*/
VOID Unload(IN PDRIVER_OBJECT pDriverObject)
{
    DbgPrint("Driver Unload routine\n");
}

/*++
 * @name DriverEntry
 *
 * @description Driver entry point
 *
 * @param pDriverObject
 *        Pointer to a DRIVER_OBJECT data structure that represent the driver 
 *
 * @param pRegistryPath
 *        Pointer to a UNICODE_STRING data structure that represent the name of the 
 *        service key in the registry
 *
 * @return Return a status code from ntstatus.h or defined by user,
 *         if function succeded it return STATUS_SUCCESS 
 *
 * @remarks This function is called when driver loaded, at PASSIVE_LEVEL
 *          and is responsable for initializing the driver
 *
 *--*/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
                     IN PUNICODE_STRING pRegistryPath)
{
    ULONG    ulIndex = 0;
    NTSTATUS ntStatus  = STATUS_SUCCESS;

    /* Initialize the table of function pointers with a function that fail every request */
    for (ulIndex = 0; ulIndex < IRP_MJ_MAXIMUM_FUNCTION; ulIndex++)
        pDriverObject->MajorFunction[ulIndex] = DispatchDefault;

    /* This function is called prior driver unload */
    pDriverObject->DriverUnload                         = Unload;

    /* Initialize function pointers that we are interesed in */
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = DispatchCommon;
    pDriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DispatchCommon;
    pDriverObject->MajorFunction[IRP_MJ_CREATE]         = DispatchCommon;
    pDriverObject->MajorFunction[IRP_MJ_READ]           = DispatchCommon;
    pDriverObject->MajorFunction[IRP_MJ_WRITE]          = DispatchCommon;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceIoControl;

    /* This is non pnp driver, so we need to call AddDevice routine */
    ntStatus = AddDevice(pDriverObject);
    
    return ntStatus;
}