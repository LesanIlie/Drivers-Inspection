#include "stddcls.h"

/**
 * Drivers data structures
 */
typedef struct _DEVICE_EXTENSION
{
    PDRIVER_OBJECT pDriverObject;
    UNICODE_STRING usDeviceName;
    UNICODE_STRING unDriverName;
    PDEVICE_OBJECT pDeviceObject;
    PFILE_OBJECT   pFileObject;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

/**
 * Undocumented data structures
 */
typedef struct _OBJECT_NAMETYPE_INFO {                
    UNICODE_STRING ObjectName; 
    UNICODE_STRING ObjectType; 
}OBJECT_NAMETYPE_INFO, *POBJECT_NAMETYPE_INFO;  

/**
 * Driver routines
 */
NTSTATUS PrintDeviceInformation(IN PDEVICE_OBJECT pDeviceObject,
                                IN PIRP pIrp, 
                                IN BOOLEAN boDirectInfo);
NTSTATUS DispatchDefault(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS DispatchCommon(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS DispatchDeviceIoControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS CompleteRequest(IN PIRP pIrp, IN NTSTATUS ntStatus, IN ULONG_PTR ulpInformation);
PVOID GetSystemRoutineAddress(IN PCWSTR pcwName);
NTSTATUS StoreDeviceObject(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS GetDriverObjectList(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS GetDeviceObjectsFromDriverObject(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS PnpBuildIrp(IN PDEVICE_OBJECT pTargetDeviceObject, OUT PIRP *ppIrp);
NTSTATUS ForwardIrpAndWait(IN PDEVICE_OBJECT pTargetDeviceObject, IN PIRP pIrp);
NTSTATUS PnpInitPDOStack(PIO_STACK_LOCATION pIoStackLocation, 
                         PFILE_OBJECT pFileObject);
NTSTATUS DirectDeviceInformation(IN PDEVICE_OBJECT pDeviceObject,
                                 IN PIRP pIrp,
                                 IN PFILE_OBJECT pFileObject);

NTSTATUS FindDeviceObject(IN PDEVICE_OBJECT pDeviceObject,
                          IN PIRP pIrp,
                          IN PUNICODE_STRING pusDriverObject,
                          IN PUNICODE_STRING pusDeviceToFind,
                          OUT PDEVICE_OBJECT *ppReturnedDeviceObject);

NTSTATUS PnpQueryDeviceInformation(IN PDEVICE_OBJECT pTargetDeviceObject,
                                   IN PIO_STACK_LOCATION pIoSourceStackLocation,
                                   OUT PVOID *pvOutputData);
NTSTATUS ForwardedIrpCompletion(IN PDEVICE_OBJECT pTargetDeviceObject, 
                                IN PIRP pIrp,
                                IN PKEVENT Event);

NTSTATUS DeviceProperty(IN PDEVICE_OBJECT pDeviceObject,
                        IN PIRP pIrp,
                        IN PFILE_OBJECT pFileObject);

/**
 * Function pointers prototypes
 */
typedef NTSTATUS(*POBQUERYNAMESTRING)(PVOID Object,
                                      POBJECT_NAME_INFORMATION ObjectNameInfo,
                                      ULONG Length,
                                      PULONG ReturnLength);

typedef NTSTATUS(*POBRRFERENCEOBJECTBYNAME)(PUNICODE_STRING ObjectPath,
                                            ULONG Attributes,
                                            PACCESS_STATE PassedAccessState,
                                            ACCESS_MASK DesiredAccess,
                                            POBJECT_TYPE ObjectType,
                                            KPROCESSOR_MODE AccessMode,
                                            PVOID ParseContext,
                                            PVOID *ObjectPtr);

typedef NTSTATUS(*PZWQUERYDIRECTORYOBJECT)(HANDLE DirectoryHandle,
                                           PVOID Buffer,
                                           ULONG Length,
                                           BOOLEAN ReturnSingleEntry,
                                           BOOLEAN RestartScan,
                                           PULONG Context,
                                           PULONG ReturnLength);

typedef NTSTATUS(*PZWOPENDIRECTORYOBJECT)(PHANDLE DirectoryHandle,
                                          ACCESS_MASK DesiredAccess,
                                          POBJECT_ATTRIBUTES ObjectAttributes);
