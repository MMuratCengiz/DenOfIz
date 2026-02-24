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

#include <deque>
#include <unordered_set>
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"
#include "VulkanBindGroupLayout.h"
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
        static const std::vector<const char *>             g_debugInstanceExtensions;

        std::vector<VkLayerProperties>     m_availableLayers;
        VkDebugUtilsMessengerEXT           m_debugMessenger = nullptr;
        DenOfIz_DeviceConstants            m_deviceConstants{ };
        DenOfIz_PhysicalDeviceCapabilities m_deviceCapabilities{ };
        std::unordered_set<const char *>   m_supportedLayers;
        std::unordered_set<const char *>   m_enabledDeviceExtensions;
        std::unordered_set<const char *>   m_enabledInstanceExtensions;

        std::unique_ptr<VulkanContext>      m_context;
        std::vector<DenOfIz_PhysicalDevice> m_physicalDevices;
        std::unique_ptr<InternalAutoSync>   m_autoSync;
        DenOfIz_LogicalDeviceDesc           m_deviceDesc;
        std::deque<std::string>             m_deviceNameStorage;

    public:
        VulkanLogicalDevice( ) = default;

        void                        CreateDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) override;
        DenOfIz_PhysicalDeviceArray ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override;

        void                         WaitIdle( ) override;
        [[nodiscard]] VulkanContext *GetContext( ) const;

        // Factory methods
        ICommandQueue       *CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc ) override;
        ICommandListPool    *CreateCommandListPool( const DenOfIz_CommandListPoolDesc &desc ) override;
        IPipeline           *CreatePipeline( const DenOfIz_PipelineDesc &desc ) override;
        IPipelineCache      *CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc ) override;
        ISwapChain          *CreateSwapChain( const DenOfIz_SwapChainDesc &desc ) override;
        IRootSignature      *CreateRootSignature( const DenOfIz_RootSignatureDesc &desc ) override;
        IInputLayout        *CreateInputLayout( const DenOfIz_InputLayoutDesc &desc ) override;
        IBindGroupLayout    *CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) override;
        IBindGroup          *CreateBindGroup( const DenOfIz_BindGroupDesc &desc ) override;
        IFence              *CreateFence( ) override;
        ISemaphore          *CreateSemaphore( ) override;
        IBuffer             *CreateBuffer( const DenOfIz_BufferDesc &desc ) override;
        ITexture            *CreateTexture( const DenOfIz_TextureDesc &desc ) override;
        ISampler            *CreateSampler( const DenOfIz_SamplerDesc &desc ) override;
        IQueryPool          *CreateQueryPool( const DenOfIz_QueryPoolDesc &desc ) override;
        ITopLevelAS         *CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &createDesc ) override;
        IShaderLocalData    *CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &createDesc ) override;

        ~VulkanLogicalDevice( ) override;

    private:
        void InitDeviceExtensions( );
        void InitInstanceExtensions( );
        void InitSupportedLayers( std::vector<const char *> &layers );

        void SetupQueueFamilies( ) const;
        void CreateLogicalDevice( );

        [[nodiscard]] bool                                 ValidateLayer( const std::string &layer ) const;
        void                                               InitializeVma( ) const;
        void                                               CreateDeviceInfo( const VkPhysicalDevice &physicalDevice, DenOfIz_PhysicalDevice &deviceInfo );
        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> CreateUniqueDeviceQueueCreateInfos( ) const;
        void                                               DestroyDebugUtils( ) const;
    };

} // namespace DenOfIz
