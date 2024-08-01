/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanInputLayout.h"
#include "VulkanPipeline.h"

namespace DenOfIz
{

    class VulkanLogicalDevice final : public ILogicalDevice
    {
        const std::unordered_map<std::string, bool> m_enabledLayers;

        static const std::unordered_map<std::string, bool> g_optionalLayers;
        static const std::vector<const char *>             g_requiredDeviceExtensions;
        static const std::vector<const char *>             g_optionalDeviceExtensions;
        static const std::vector<const char *>             g_requiredInstanceExtensions;
        static const std::vector<const char *>             g_optionalInstanceExtensions;

        const std::vector<QueueType> m_queueTypes = { QueueType::Graphics, QueueType::Copy, QueueType::Presentation };

        std::vector<VkLayerProperties>   m_availableLayers;
        VkDebugUtilsMessengerEXT         m_debugMessenger = nullptr;
        DeviceConstants                  m_deviceConstants{ };
        PhysicalDeviceCapabilities       m_deviceCapabilities{ };
        std::unordered_set<const char *> m_supportedLayers;
        std::unordered_set<const char *> m_enabledDeviceExtensions;
        std::unordered_set<const char *> m_enabledInstanceExtensions;

        std::unique_ptr<VulkanContext> m_context;

    public:
        VulkanLogicalDevice( ) = default;

        void                        CreateDevice( ) override;
        std::vector<PhysicalDevice> ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override
        {
            return m_context->IsDeviceLost;
        }

        void                         WaitIdle( ) override;
        [[nodiscard]] VulkanContext *GetContext( ) const;

        // Factory methods
        std::unique_ptr<ICommandListPool>   CreateCommandListPool( const CommandListPoolDesc &createInfo ) override;
        std::unique_ptr<IPipeline>          CreatePipeline( const PipelineDesc &createInfo ) override;
        std::unique_ptr<ISwapChain>         CreateSwapChain( const SwapChainDesc &createInfo ) override;
        std::unique_ptr<IRootSignature>     CreateRootSignature( const RootSignatureDesc &createInfo ) override;
        std::unique_ptr<IInputLayout>       CreateInputLayout( const InputLayoutDesc &createInfo ) override;
        std::unique_ptr<IResourceBindGroup> CreateResourceBindGroup( const ResourceBindGroupDesc &createInfo ) override;
        std::unique_ptr<IFence>             CreateFence( ) override;
        std::unique_ptr<ISemaphore>         CreateSemaphore( ) override;
        std::unique_ptr<IBufferResource>    CreateBufferResource( std::string name, const BufferDesc &createInfo ) override;
        std::unique_ptr<ITextureResource>   CreateTextureResource( std::string name, const TextureDesc &createInfo ) override;
        std::unique_ptr<ISampler>           CreateSampler( std::string name, const SamplerDesc &createInfo ) override;

        ~VulkanLogicalDevice( ) override;

    private:
        void InitDeviceExtensions( );
        void InitInstanceExtensions( );
        void InitSupportedLayers( std::vector<const char *> &layers );

        void SetupQueueFamilies( ) const;
        void CreateLogicalDevice( );

        [[nodiscard]] bool                                 ValidateLayer( const std::string &layer ) const;
        void                                               InitializeVma( ) const;
        void                                               CreateDeviceInfo( const VkPhysicalDevice &physicalDevice, PhysicalDevice &deviceInfo ) const;
        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> CreateUniqueDeviceCreateInfos( ) const;
        void                                               DestroyDebugUtils( ) const;
    };

} // namespace DenOfIz
