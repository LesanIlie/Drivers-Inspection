
#include "ioctl.h"
#include "driver.h"

#define BUFF_LENGTH 8192


void DisplaySpecificInformation(HANDLE hDevice, DEVICE_INFO_ENUM enType)
{
	switch(enType)
	{
	    case DeviceProperty:
		    DisplayDeviceProperty(hDevice);
		    break;
	    case DirectInformation:
		    DisplayDirectInfo(hDevice);
		    break;
	    default:
		    /* Should never reach here */
		    printf("Bad request\n");
		    break;
	}
}
int main(int argc, char* argv[])
{	
	DWORD lpBytesReturned;
	ULONG i = 0;
	ULONG Counter = 0;
	PWCHAR pwcInputDriverName;
	PWCHAR pwcOutputDriverName;
	DEVICE_INFO_ENUM enDevInfo = Nothing;

	pwcInputDriverName = (PWCHAR)malloc(sizeof(WCHAR)*BUFF_LENGTH);
	pwcOutputDriverName = (PWCHAR)malloc(sizeof(WCHAR)*BUFF_LENGTH);

	HANDLE hdevice = CreateFileA("\\\\.\\DriverInfo", 
		                        GENERIC_READ | GENERIC_WRITE,
						    FILE_SHARE_READ | FILE_SHARE_WRITE,
						    NULL,
						    OPEN_EXISTING,
						    0/*FILE_FLAG_OVERLAPPED*/,
						    NULL);
	
	/*=======================================================================================*/
	if (hdevice == INVALID_HANDLE_VALUE)
	{
		printf("Unable to open FILEIO device - error %d\n", GetLastError());
		return 1;
	}
	
	if (DeviceIoControl(hdevice,
	    IOCTL_GET_DRIVER_LIST, 
	    pwcInputDriverName, 
	    sizeof(WCHAR)*BUFF_LENGTH,
	    pwcOutputDriverName, 
	    sizeof(WCHAR)*BUFF_LENGTH,
	    &lpBytesReturned,
	    NULL))
	{
		unsigned int uiDeviceLength = 0;
		unsigned int uiPrintCounter = 0;
		unsigned int uiNewLinePrint = 1;
		for (i = 0; i < lpBytesReturned/sizeof(WCHAR) && pwcOutputDriverName[i]; i++)
		{
			uiDeviceLength++;
			wprintf(L"%c", pwcOutputDriverName[i]);
			if (pwcOutputDriverName[i] == 0x0020)
			{
				for (uiPrintCounter = 0 ; uiPrintCounter < 20 - uiDeviceLength; uiPrintCounter++)
				{
					printf(" ");
				}
				uiNewLinePrint++;
				uiDeviceLength = 0;
				
				if (uiNewLinePrint == 4)
				{
					uiNewLinePrint = 1;
					printf("\n");
				}
			}
		}
	}
	else 
	{
		printf("Function failed with status %x \n", GetLastError());
		goto clean_up;
	}
	/*=======================================================================================*/
     RtlZeroMemory(pwcInputDriverName, sizeof(WCHAR)*BUFF_LENGTH);
	printf("\nEnter the driver name: ");
	wscanf(L"%ls", pwcInputDriverName);
	if (DeviceIoControl(hdevice, 
		               IOCTL_GET_DEVICE_LIST,
					pwcInputDriverName, 
					sizeof(WCHAR)*BUFF_LENGTH,
					pwcOutputDriverName,
					sizeof(WCHAR)*BUFF_LENGTH,
					&lpBytesReturned,
					NULL))
	{
		for (i = 0; i < lpBytesReturned/sizeof(WCHAR) && pwcOutputDriverName[i]; i++)
		{
			wprintf(L"%c", pwcOutputDriverName[i]);
			if (pwcOutputDriverName[i] == 0x0020)
				printf("\n");
		}
	}
	else 
	{
		printf("Function failed with status %x \n", GetLastError());
		goto clean_up;
	}

	/*=======================================================================================*/
     RtlZeroMemory(pwcInputDriverName, sizeof(WCHAR)*BUFF_LENGTH);
	printf("\nEnter the device object: ");
	wscanf(L"%ls", pwcInputDriverName);
	if (DeviceIoControl(hdevice, 
		               IOCTL_STORE_DEVICE, 
	                    pwcInputDriverName, 
	           		sizeof(WCHAR)*BUFF_LENGTH,
		    			pwcOutputDriverName,
             		     sizeof(WCHAR)*BUFF_LENGTH,
			         	&lpBytesReturned,
           			NULL))
	{
			/* Display required information */
			DisplaySpecificInformation(hdevice, DeviceProperty);
	}
	else 
	{
		printf("Last function failed with status %x \n", GetLastError());
	}
clean_up:
	free(pwcInputDriverName);
	free(pwcOutputDriverName);

	CloseHandle(hdevice);
	return 0;
}

