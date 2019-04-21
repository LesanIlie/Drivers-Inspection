#include "stddcls.h"
#include "driver.h"
#include "ioctl.h"

extern ULONG ulBytesReturned;
/*++
 * @name DispatchDeviceIoControl
 *
 * @description Dispatch routine
 *
 * @param pDeviceObject
 *        Pointer to the target DEVICE_OBJECT
 *
 * @param pIrp
 *        Pointer to IRP send to this device
 *
 * @return Return a status code from ntstatus.h or defined by user,
 *         if function succeded it return STATUS_SUCCESS 
 *
 * @remarks This function handle diferit types ioctls
 *
 *--*/
NTSTATUS DispatchDeviceIoControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    NTSTATUS           ntStatus             = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIoStackLocation     = NULL;
    ULONG              ulCtlCode            = 0;

    /* Get current stack location */
    pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);

    /* Get parameters from stack */
    ulCtlCode = pIoStackLocation->Parameters.DeviceIoControl.IoControlCode;

    switch (ulCtlCode)
    {
        case IOCTL_GET_DRIVER_LIST:
            ntStatus = GetDriverObjectList(pDeviceObject, pIrp);
            if (!NT_SUCCESS(ntStatus))
            {
                DbgPrint("IOCTL GetDriverObjectList \n");
            }
            break;
        case IOCTL_GET_DEVICE_LIST:
            ntStatus = GetDeviceObjectsFromDriverObject(pDeviceObject, pIrp);
            if (!NT_SUCCESS(ntStatus))
            {
                DbgPrint("IOCTL GetDeviceObjectsFromDriverObject \n");
            }
            break;
        case IOCTL_STORE_DEVICE:
            ntStatus = StoreDeviceObject(pDeviceObject, pIrp);
            if (!NT_SUCCESS(ntStatus))
            {
                DbgPrint("IOCTL StoreDeviceObject \n");
            }
            break;
        case IOCTL_GET_DEVICE_INFO:
            ntStatus = PrintDeviceInformation(pDeviceObject, pIrp, FALSE);
            if (!NT_SUCCESS(ntStatus))
            {
                DbgPrint("IOCTL PrintDeviceInformation \n");
            }
            break;
        case IOCTL_GET_DIRECT_DEVICE_INFO:
            ntStatus = PrintDeviceInformation(pDeviceObject, pIrp, TRUE);
            if (!NT_SUCCESS(ntStatus))
            {
                DbgPrint("IOCTL Direct PrintDeviceInformation \n");
            } 
            break;
        default: 
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    return  CompleteRequest(pIrp, ntStatus, ulBytesReturned);
}