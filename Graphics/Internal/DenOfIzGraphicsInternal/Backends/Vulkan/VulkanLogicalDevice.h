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

#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include <unordered_set>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanInputLayout.h"

#ifdef _WIN32
#undef CreateSemaphore
#endif

namespace DenOfIz
{

    class VulkanLogicalDevice final : public ILogicalDevice
    {
        static const std::unordered_map<std::string, bool> g_optionalLayers;
        static const std::vector<const char *>             g_requiredDeviceExtensions;
        static const std::vector<const char *>             g_optionalDeviceExtensions;
        static const std::vector<const char *>             g_requiredInstanceExtensions;
        static const std::vector<const char *>             g_optionalInstanceExtensions;

        std::vector<VkLayerProperties>   m_availableLayers;
        VkDebugUtilsMessengerEXT         m_debugMessenger = nullptr;
        DeviceConstants                  m_deviceConstants{ };
        PhysicalDeviceCapabilities       m_deviceCapabilities{ };
        std::unordered_set<const char *> m_supportedLayers;
        std::unordered_set<const char *> m_enabledDeviceExtensions;
        std::unordered_set<const char *> m_enabledInstanceExtensions;

        std::unique_ptr<VulkanContext> m_context;
        std::vector<PhysicalDevice>     m_physicalDevices;

    public:
        VulkanLogicalDevice( ) = default;

        void                    CreateDevice( ) override;
        PhysicalDeviceArray     ListPhysicalDevices( ) override;
        void                    LoadPhysicalDevice( const PhysicalDevice &device ) override;
        bool                         IsDeviceLost( ) override;

        void                         WaitIdle( ) override;
        [[nodiscard]] VulkanContext *GetContext( ) const;

        // Factory methods
        ICommandQueue       *CreateCommandQueue( const CommandQueueDesc &desc ) override;
        ICommandListPool    *CreateCommandListPool( const CommandListPoolDesc &desc ) override;
        IPipeline           *CreatePipeline( const PipelineDesc &desc ) override;
        ISwapChain          *CreateSwapChain( const SwapChainDesc &desc ) override;
        IRootSignature      *CreateRootSignature( const RootSignatureDesc &desc ) override;
        IInputLayout        *CreateInputLayout( const InputLayoutDesc &desc ) override;
        IResourceBindGroup  *CreateResourceBindGroup( const ResourceBindGroupDesc &desc ) override;
        IFence              *CreateFence( ) override;
        ISemaphore          *CreateSemaphore( ) override;
        IBufferResource     *CreateBufferResource( const BufferDesc &desc ) override;
        ITextureResource    *CreateTextureResource( const TextureDesc &desc ) override;
        ISampler            *CreateSampler( const SamplerDesc &desc ) override;
        ITopLevelAS         *CreateTopLevelAS( const TopLevelASDesc &desc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const BottomLevelASDesc &desc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const ShaderBindingTableDesc &desc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const LocalRootSignatureDesc &createDesc ) override;
        IShaderLocalData    *CreateShaderLocalData( const ShaderLocalDataDesc &createDesc ) override;

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
        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> CreateUniqueDeviceQueueCreateInfos( ) const;
        void                                               DestroyDebugUtils( ) const;
    };

} // namespace DenOfIz
