#pragma once

#include <wdm.h>

namespace Driver::IORequest
{

/// <summary>
/// Dispatches IRPs received by the driver to their respective handler
/// </summary>
/// <param name="pDeviceObject">Driver device pointer</param>
/// <param name="pIoRequestPacket">IO request packet pointer</param>
/// <returns>NTSTATUS from handler. Otherwise, STATUS_NOT_IMPLEMENTED</returns>
NTSTATUS Dispatcher(PDEVICE_OBJECT pDeviceObject, PIRP pIoRequestPacket);

}
