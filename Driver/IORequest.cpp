#include "IORequest.h"
#include "IOCTL.h"

namespace Driver::IORequest
{

NTSTATUS Dispatcher(PDEVICE_OBJECT pDeviceObject, PIRP pIoRequestPacket)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	KdPrint(("Driver::IORequest::Dispatcher: Enter\n"));

	if (nullptr == pIoRequestPacket)
	{
		KdPrint(("Driver::IORequest::Dispatcher: pIoRequestPacket is NULL\n"));
		return STATUS_INVALID_PARAMETER_2;
	}

	// Varies by certain requests, but unused here. Default as zero
	pIoRequestPacket->IoStatus.Information = 0;

	NTSTATUS ntStatus{ STATUS_DRIVER_INTERNAL_ERROR };

	/*
	* IRP parameters are passed via IO stack. The first order of business is getting
	* a pointer to the current IRP stack so that we may retrieve and use the
	* arguments passed to us
	*/
	PIO_STACK_LOCATION pIoStackLocation{ IoGetCurrentIrpStackLocation(pIoRequestPacket) };

	if (nullptr == pIoStackLocation)
	{
		KdPrint(("Driver::IORequest::Dispatcher: pIoStackLocation is NULL\n"));

		pIoRequestPacket->IoStatus.Status = ntStatus;
		IoCompleteRequest(pIoRequestPacket, IO_NO_INCREMENT);

		return ntStatus;
	}

	// Dispatch on major function (CLOSE, CREATE, DEVICE_CONTROL, etc)
	switch (pIoStackLocation->MajorFunction)
	{
		/*
		* Necessary for a driver client to open a handle for the driver device
		*/
		case IRP_MJ_CLOSE:
		case IRP_MJ_CREATE:
			/*
			* Allow unconditionally. One may also block access to the driver device
			* by who is attempting to open it
			*/
			ntStatus = STATUS_SUCCESS;
			break;

		// IOCTL
		case IRP_MJ_DEVICE_CONTROL:
			ntStatus = Driver::IOCTL::Dispatch(*pIoRequestPacket, *pIoStackLocation);
			break;

		default:
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
	}

	pIoRequestPacket->IoStatus.Status = ntStatus;

	/*
	* When an IRP has finished processing, use IoCompleteRequest to signal that the
	* request has been handled.
	* 
	* IRPs may also be passed along a chain of drivers, but that is beyond
	* scope here
	*/
	IoCompleteRequest(pIoRequestPacket, IO_NO_INCREMENT);

	KdPrint(("Driver::IORequest::Dispatcher: Exit\n"));
	return ntStatus;
}

}
