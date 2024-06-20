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
#include "../Common/GraphicsWindowHandle.h"
#include "ICommandList.h"
#include "IDescriptorTable.h"
#include "IFence.h"
#include "IPipeline.h"
#include "IResource.h"
#include "IRootSignature.h"
#include "ISemaphore.h"
#include "ISwapChain.h"

namespace DenOfIz
{

    class ILogicalDevice
    {
    protected:
        PhysicalDeviceInfo m_selectedDeviceInfo;

    public:
        virtual ~ILogicalDevice() = default;

        virtual void CreateDevice(GraphicsWindowHandle *window) = 0;
        virtual std::vector<PhysicalDeviceInfo> ListPhysicalDevices() = 0;
        virtual void LoadPhysicalDevice(const PhysicalDeviceInfo &device) = 0;
        virtual bool IsDeviceLost() = 0;
        virtual void WaitIdle() = 0;

        inline const PhysicalDeviceInfo &SelectedDeviceInfo() { return m_selectedDeviceInfo; };

        // Factory methods
        virtual std::unique_ptr<ICommandList> CreateCommandList(const CommandListCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<IPipeline> CreatePipeline(const PipelineCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<ISwapChain> CreateSwapChain(const SwapChainCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<IRootSignature> CreateRootSignature(const RootSignatureCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<IInputLayout> CreateInputLayout(const InputLayoutCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<IDescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<IFence> CreateFence() = 0;
        virtual std::unique_ptr<ISemaphore> CreateSemaphore() = 0;
        virtual std::unique_ptr<IBufferResource> CreateBufferResource(std::string name, const BufferCreateInfo &createInfo) = 0;
        virtual std::unique_ptr<ITextureResource> CreateImageResource(std::string name, const ImageCreateInfo &createInfo) = 0;
    };

} // namespace DenOfIz
