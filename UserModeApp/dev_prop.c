#include "driver.h"
#include "ioctl.h"


/* Print device description */
VOID DeviceDescriptionFunction(PVOID pDeviceProperty)
{
	wprintf(L"DeviceDescription: %ws \n", pDeviceProperty);
}

/* Print hardware ID */
VOID HardwareIDFunction(PVOID pDeviceProperty)
{
	wprintf(L"HardwareID: %ws \n", pDeviceProperty);
}

/* Print device compatible IDs */
VOID CompatibleIDsFunction(PVOID pDeviceProperty)
{
	wprintf(L"CompatibleIDs: %ws \n", pDeviceProperty);
}

/* Print boot configuration resources */
VOID BootConfigurationFunction(PVOID pDeviceProperty)
{
	//wprintf(L"BusNumber: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->BusNumber);
     //wprintf(L"InterfaceType: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->InterfaceType);
}

/* Print translated resources */
VOID BootConfigurationTranslatedFunction(PVOID pDeviceProperty)
{
	//wprintf(L"BusNumber: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->BusNumber);
	//wprintf(L"InterfaceType: %X \n",((PCM_RESOURCE_LIST)pDeviceProperty)->List->InterfaceType); 
}
/* Print class name  */
VOID ClassNameFunction(PVOID pDeviceProperty)
{
	wprintf(L"ClassName: %ws \n", pDeviceProperty);
}

/* Print GUID */
VOID ClassGuidFunction(PVOID pDeviceProperty)
{
}

/* Print hardware reg key */
VOID DriverKeyNameFunction(PVOID pDeviceProperty)
{
	wprintf(L"DriverKeyName: %ws \n", pDeviceProperty);
}


/* Print manufacture */
VOID ManufacturerFunction(PVOID pDeviceProperty)
{
	wprintf(L"Manufacturer: %ws \n", pDeviceProperty);
}

/* Print freindly name */
VOID FriendlyNameFunction(PVOID pDeviceProperty)
{
	wprintf(L"FriendlyName: %ws \n", pDeviceProperty);
}

/* Print driver location */
VOID LocationInformationFunction(PVOID pDeviceProperty)
{
	wprintf(L"LocationInformation: %ws \n", pDeviceProperty);
}

/* Print PDO name */
VOID PhysicalDeviceObjectNameFunction(PVOID pDeviceProperty)
{
	wprintf(L"PhysicalDeviceObjectName: %ws \n", pDeviceProperty);
}

/* Print bus GUID */
VOID BusTypeGuidFunction(PVOID pDeviceProperty)
{
}

/* Print legacy bus type */
VOID LegacyBusTypeFunction(PVOID pDeviceProperty)
{
}

/* Print bus number function */
VOID BusNumberFunction(PVOID pDeviceProperty)
{
	wprintf(L"BusNumber: %X \n", *(PULONG)pDeviceProperty);
}

/* Print enumerator name */
VOID EnumeratorNameFunction(PVOID pDeviceProperty)
{
	wprintf(L"EnumeratorName: %ws \n", pDeviceProperty);
}

/* Print device address */
VOID AddressFunction(PVOID pDeviceProperty)
{
	wprintf(L"Address: %X \n", *(PULONG)pDeviceProperty);
}

/* Print UI number */
VOID UINumberFunction(PVOID pDeviceProperty)
{
	wprintf(L"UINumber: %X \n", *(PULONG)pDeviceProperty);
}

/* Print install state */
VOID InstallStateFunction(PVOID pDeviceProperty)
{
}

/* Printf removal policy */
VOID RemovalPolicyFunction(PVOID pDeviceProperty)
{
}

/* Print driver resource requirements */
VOID ResourceRequirementsFunction(PVOID pDeviceProperty)
{
}

/* Print allocated resource function */
VOID AllocatedResourcesFunction(PVOID pDeviceProperty)
{
}

/* Print continer ID function */
VOID ContainerIDFunction(PVOID pDeviceProperty)
{    
}

/* Initialize table of function pointers */
VOID (*FunctionTable[])(PVOID pDeviceProperty) = 
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

VOID DisplayDeviceProperty(HANDLE hDevice)
{
	ULONG ulCounter = 0; 
	DWORD lpBytesReturned;
	PWCHAR pwcOutputDriverName;
	
	pwcOutputDriverName = (PWCHAR)malloc(sizeof(WCHAR)*BUFF_LENGTH);
	
	for (ulCounter = 0; ulCounter < 23; ulCounter++)
	{
		RtlZeroMemory(pwcOutputDriverName, sizeof(WCHAR)*BUFF_LENGTH);
		if (DeviceIoControl(hDevice, 
                              IOCTL_GET_DEVICE_INFO, 
	                         &ulCounter, 
	                         sizeof(ULONG),
	        	               pwcOutputDriverName,
         			          sizeof(WCHAR)*BUFF_LENGTH,
		         	          &lpBytesReturned,
           	               NULL))
		{
			(*FunctionTable[ulCounter])(pwcOutputDriverName);
		}
		else
		{
			//printf("Function[%d] failed with status: 0x%08X\n", ulCounter, GetLastError());
		}
	}
	free(pwcOutputDriverName);
}