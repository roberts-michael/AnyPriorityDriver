/*
* Shared driver interface between UM and KM
*/

#pragma once

#ifdef _DRIVER

#include <ntdef.h>

#else // _DRIVER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#endif // User-mode

namespace Driver::Interface
{

/// <summary>
/// Device type. Useful for hardware-based drivers. For software-based drivers,
/// can be anything. Microsoft recommends starting at 0x8000
/// </summary>
static constexpr ULONG const DeviceType{ 0x8000 };

/// <summary>
/// Symbolic link name created by the driver for its device. Used with APIs
/// such as CreateFileW to open a handle to the driver device
/// </summary>
static constexpr LPCWSTR const DeviceName{ L"\\\\.\\AnyPriorityDriver" };

/// <summary>
/// Helper constexpr to formulate a device IO control code (IOCTL)
/// </summary>
/// <param name="ulFunction">Unique function ID. Recommended minimum is 0x800</param>
/// <returns>A device IO control code</returns>
constexpr ULONG MakeIoControlCode(ULONG ulFunction)
{
	/*
	* Condensed from CTL_CODE macro for access FILE_ANY_ACCESS and method METHOD_NEITHER
	*/
	return (DeviceType << 16) | (ulFunction << 2) | 3;
}

struct IOCTL
{
	/// <summary>
	/// IOCTL which can set the priority of a given thread to any arbitrary value
	/// in the range of 1 to 31
	/// </summary>
	struct SetThreadPriority
	{
		static constexpr ULONG const ControlCode{ MakeIoControlCode(0xa01) };

		struct Input
		{
			/// <summary>
			/// Target thread ID
			/// </summary>
			ULONG ThreadId;
			/// <summary>
			/// Priority 1-31
			/// </summary>
			LONG Priority;
		};
	};

};

}
