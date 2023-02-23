/*
* Example client for AnyPriorityDriver. Accepts a TID and a priority as user input
* and sends that to the driver component over IOCTL
*/

#include "Driver/Interface.h" // Shared IOCTL definitions

#include <iostream>
#include <stdio.h>
#include <string>

int main(int iArgc, char** ppszArgv)
{
	using Input = Driver::Interface::IOCTL::SetThreadPriority::Input;
	using IOCTL = Driver::Interface::IOCTL;

	UNREFERENCED_PARAMETER(iArgc);
	UNREFERENCED_PARAMETER(ppszArgv);

	std::cout << "AnyPriorityDriver Client\n";

	// Data to be sent to the driver over IOCTL
	Input driverInput;
	ZeroMemory(&driverInput, sizeof(Input));

	std::string strInput;
	int iResult{ 0 };

	// First, gather user input
	do
	{
		std::cout << "Enter TID: ";
		std::cin >> strInput;

		iResult = sscanf_s(strInput.c_str(), "%u", &driverInput.ThreadId);
	}
	// Apparently, sscanf accepts negative numbers as being unsigned
	while (1 != iResult || true == strInput.starts_with('-'));

	do
	{
		std::cout << "Enter Priority (1-31): ";
		std::cin >> strInput;

		iResult = sscanf_s(strInput.c_str(), "%u", &driverInput.Priority);
	}
	while (1 != iResult || 0 >= driverInput.Priority || 32 <= driverInput.Priority);

	std::cout << "Setting priority of TID " << driverInput.ThreadId << " to " << driverInput.Priority << "...\n";

	// Next, open the driver device
	HANDLE hDriverDevice{ CreateFileW(Driver::Interface::DeviceName, GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr) };

	if (INVALID_HANDLE_VALUE == hDriverDevice)
	{
		std::cerr << "Error: Failed to open driver device handle\n";
		std::cerr << "GetLastError: " << GetLastError() << "\n";

		return 1;
	}

	DWORD dwBytesReturned{ 0 };

	// Lastly, send the data over to the driver over IOCTL
	if (FALSE == DeviceIoControl(hDriverDevice, IOCTL::SetThreadPriority::ControlCode,
		&driverInput, sizeof(Input), nullptr, 0, &dwBytesReturned, nullptr))
	{
		std::cerr << "Error: Failed to send driver IOCTL\n";
		std::cerr << "GetLastError: " << GetLastError() << "\n";

		// Close the previously opened driver device handle
		CloseHandle(hDriverDevice);
		return 1;
	}

	std::cout << "Success" << std::endl;

	// Close the previously opened driver device handle
	CloseHandle(hDriverDevice);

	return 0;
}
