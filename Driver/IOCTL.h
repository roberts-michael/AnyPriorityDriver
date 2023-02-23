#pragma once
#include <wdm.h>

namespace Driver::IOCTL
{

/// <summary>
/// Dispatches an IOCTL request to its respective handler
/// </summary>
/// <param name="ioRequestPacket">Reference to the request IRP</param>
/// <param name="ioStackLocation">Reference to the request IO stack</param>
/// <returns>NTSTATUS from handler. Otherwise, STATUS_NOT_IMPLEMENTED</returns>
NTSTATUS Dispatch(IRP& ioRequestPacket, IO_STACK_LOCATION& ioStackLocation);

}
