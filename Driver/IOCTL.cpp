#include <ntifs.h>

#include "IOCTL.h"
#include "Interface.h"

namespace
{

/// <summary>
/// IOCTL to set the priority of a given thread to any value 1-31
/// </summary>
NTSTATUS SetThreadPriority(IRP& ioRequestPacket, IO_STACK_LOCATION& ioStackLocation)
{
	using Input = Driver::Interface::IOCTL::SetThreadPriority::Input;

	UNREFERENCED_PARAMETER(ioRequestPacket);
	KdPrint(("SetThreadPriority: Enter\n"));

	/*
	* First, validate that we've received the correct size of data structure for
	* this IOCTL
	*/
	if (sizeof(Input) != ioStackLocation.Parameters.DeviceIoControl.InputBufferLength)
	{
		KdPrint(("SetThreadPriority: Input buffer size mismatch\n"));
		return STATUS_INVALID_PARAMETER;
	}

	/*
	* Continue to check that the input data buffer is not an invalid pointer
	*/
	if (nullptr == ioStackLocation.Parameters.DeviceIoControl.Type3InputBuffer)
	{
		KdPrint(("SetThreadPriority: Input buffer is NULL\n"));
		return STATUS_INVALID_PARAMETER;
	}

	Input* pInput{ reinterpret_cast<Input*>(ioStackLocation.Parameters.DeviceIoControl.Type3InputBuffer) };

	/*
	* Ensure priority is valid from LOW_PRIORITY (0) + 1 to HIGH_PRIORITY (31)
	*/
	if (0 >= pInput->Priority || 32 <= pInput->Priority)
	{
		KdPrint(("SetThreadPriority: Priority value is invalid\n"));
		return STATUS_INVALID_PARAMETER;
	}

	/*
	* Given thread ID must be converted to a PETHREAD. PETHREAD is a data structure used internally
	* by the OS. KeSetPriorityThread accepts PKTHREAD. Conveniently, PKTHREAD and PETHREAD
	* are interchangeable
	*/
	PETHREAD pThread{ nullptr };
	NTSTATUS ntStatus{ PsLookupThreadByThreadId(UlongToHandle(pInput->ThreadId), &pThread) };

	if (false == NT_SUCCESS(ntStatus) || nullptr == pThread)
	{
		// Failed to find the thread
		KdPrint(("SetThreadPriority: Failed PsLookupThreadByThreadId (%ld %p)\n", ntStatus, pThread));
		return ntStatus;
	}

	/*
	* NOTE: At this point, we've acquired the PETHREAD object and incremented its reference count. This
	* reference count must be decremented when finished, otherwise we have a resource leak
	*/

	// The RV for KeSetPriorityThread is the previous priority and not of importance
	static_cast<void>(KeSetPriorityThread(reinterpret_cast<PKTHREAD>(pThread), pInput->Priority));

	// Decrement the reference count of pThread now that we've finished
	ObDereferenceObject(pThread);

	KdPrint(("SetThreadPriority: Exit\n"));
	return ntStatus;
}

} // namespace (unnamed)

namespace Driver::IOCTL
{

NTSTATUS Dispatch(IRP& ioRequestPacket, IO_STACK_LOCATION& ioStackLocation)
{
	KdPrint(("Driver::IOCTL::Dispatch: Enter\n"));

	// Default to STATUS_NOT_IMPLEMENTED
	NTSTATUS ntStatus{ STATUS_NOT_IMPLEMENTED };

	/*
	* Attempt to dispatch the request to an appropriate IOCTL handler, based on the
	* request IoControlCode parameter
	* 
	* IOCTL for each request type defined in Interface.h and set by client
	*/
	switch (ioStackLocation.Parameters.DeviceIoControl.IoControlCode)
	{
		case Driver::Interface::IOCTL::SetThreadPriority::ControlCode:
			ntStatus = SetThreadPriority(ioRequestPacket, ioStackLocation);
			break;

		// No handler was found
		default:
			break;
	}

	KdPrint(("Driver::IOCTL::Dispatch: Exit\n"));
	return ntStatus;
}

}
