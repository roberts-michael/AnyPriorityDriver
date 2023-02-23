#include "IORequest.h"
#include <wdm.h>

namespace
{

static UNICODE_STRING s_ucsSymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\AnyPriorityDriver");

} // namespace (unnamed)

/// <summary>
/// Unload routine for the driver
/// </summary>
void OnDriverUnload(PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("DriverUnload: Enter\n"));

	if (nullptr == pDriverObject)
	{
		KdPrint(("DriverUnload: pDriverObject is NULL\n"));
		return;
	}

	// First, attempt to remove the symbolic link we created
	NTSTATUS ntStatus{ IoDeleteSymbolicLink(&s_ucsSymbolicLinkName) };

	if (false == NT_SUCCESS(ntStatus))
	{
		KdPrint(("DriverUnload: Failed IoDeleteSymbolicLink (%ld)\n", ntStatus));
		return;
	}

	// Then, attempt to remove any device created by the driver
	if (nullptr != pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}

	KdPrint(("DriverUnload: Exit\n"));
}

/// <summary>
/// Entry point for the driver
/// </summary>
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pucsRegistryPath)
{
	// Device created under the object manager Device namespace
	static UNICODE_STRING s_ucsDeviceName = RTL_CONSTANT_STRING(L"\\Device\\AnyPriorityDriver");

	UNREFERENCED_PARAMETER(pucsRegistryPath);
	KdPrint(("DriverEntry: Enter\n"));

	if (nullptr == pDriverObject)
	{
		KdPrint(("DriverEntry: pDriverObject is NULL\n"));
		return STATUS_INVALID_PARAMETER_1;
	}

	/*
	* First, register unload and any driver major functions
	*/

	pDriverObject->DriverUnload = OnDriverUnload;

	// NOTE: I've opted here to use a single dispatcher for each major driver function
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = Driver::IORequest::Dispatcher;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = Driver::IORequest::Dispatcher;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Driver::IORequest::Dispatcher;

	/*
	* Next, create a device so that the driver may be communicated with. Drivers
	* receive communication over their registered devices (read, write, IOCTL, etc)
	*/

	PDEVICE_OBJECT pDeviceObject{ nullptr };

	NTSTATUS ntStatus{ IoCreateDevice(pDriverObject, 0, &s_ucsDeviceName, FILE_DEVICE_UNKNOWN,
		0, FALSE, &pDeviceObject) };

	if (false == NT_SUCCESS(ntStatus) || nullptr == pDeviceObject)
	{
		KdPrint(("DriverEntry: Failed to create driver device (%ld %p)\n", ntStatus, pDeviceObject));
		return ntStatus;
	}

	/*
	* Lastly, create a symbolic link for the driver device. A symbolic link is needed because
	* the Win32 APIs over in UM won't work via device name.
	* 
	* A client can then use this symbolic link over in UM with APIs such as CreateFileW
	* 
	* NOTE: \\??\\AnyPriorityDriver in KM equivalent to \\\\.\\AnyPriorityDriver in UM
	*/

	ntStatus = IoCreateSymbolicLink(&s_ucsSymbolicLinkName, &s_ucsDeviceName);

	if (false == NT_SUCCESS(ntStatus))
	{
		KdPrint(("DriverEntry: Failed IoCreateSymbolicLink (%ld)\n", ntStatus));

		// Previously created device is deleted upon error
		IoDeleteDevice(pDeviceObject);

		return ntStatus;
	}

	KdPrint(("DriverEntry: Exit\n"));
	return ntStatus;
}
