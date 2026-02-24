/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <deque>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "VulkanContext.h"

typedef struct DenOfIz_VulkanLogicalDevice
{
    std::unique_ptr<DenOfIz::VulkanContext> Context;
    std::vector<DenOfIz_PhysicalDevice>     PhysicalDevices;
    std::deque<std::string>                 DeviceNameStorage;
    DenOfIz_LogicalDeviceDesc               DeviceDesc;
} DenOfIz_VulkanLogicalDevice;

void DenOfIz_VulkanLogicalDevice_Create( DenOfIz_VulkanLogicalDevice **outImpl, const DenOfIz_LogicalDeviceDesc *desc );
void DenOfIz_VulkanLogicalDevice_Destroy( DenOfIz_VulkanLogicalDevice *impl );
