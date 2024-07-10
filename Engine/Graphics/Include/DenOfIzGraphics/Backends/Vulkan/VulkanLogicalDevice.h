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
#include "VulkanBufferResource.h"
#include "VulkanCommandPool.h"
#include "VulkanContext.h"
#include "VulkanCubeMapResource.h"
#include "VulkanInputLayout.h"
#include "VulkanPipeline.h"
#include "VulkanSurface.h"

namespace DenOfIz
{

    class VulkanLogicalDevice final : public ILogicalDevice
    {
        const std::unordered_map<std::string, bool> m_enabledLayers{
#ifdef _DEBUG
            { "VK_LAYER_KHRONOS_validation", true }
#endif
        };

        const std::vector<const char *> m_requiredExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                              // Maintenance Extensions
                                                              VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_MAINTENANCE2_EXTENSION_NAME, VK_KHR_MAINTENANCE3_EXTENSION_NAME,
                                                              // Dynamic Rendering
                                                              VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                                                              VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
                                                              // Used to pass Viewport and Scissor count.
                                                              VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME
#if __APPLE_CC__
                                                              ,
                                                              "VK_KHR_portability_subset"
#endif
        };

        const std::vector<QueueType> m_queueTypes = { QueueType::Graphics, QueueType::Copy, QueueType::Presentation };

        VkDebugUtilsMessengerEXT              m_debugMessenger = VK_NULL_HANDLE;
        std::unordered_map<std::string, bool> m_supportedExtensions;
        std::unordered_map<std::string, bool> m_supportedLayers;

        std::unique_ptr<VulkanContext> m_context;

    public:
        VulkanLogicalDevice() = default;

        void                        CreateDevice(GraphicsWindowHandle *window) override;
        std::vector<PhysicalDevice> ListPhysicalDevices() override;
        void                        LoadPhysicalDevice(const PhysicalDevice &device) override;
        inline bool                 IsDeviceLost() override
        {
            return m_context->IsDeviceLost;
        }

        void                         WaitIdle() override;
        [[nodiscard]] uint32_t       GetFrameCount() const;
        [[nodiscard]] VulkanContext *GetContext() const;
        [[nodiscard]] Format         GetSwapChainImageFormat() const;

        // Factory methods
        std::unique_ptr<ICommandListPool> CreateCommandListPool(const CommandListPoolDesc &createInfo) override;
        std::unique_ptr<IPipeline>        CreatePipeline(const PipelineDesc &createInfo) override;
        std::unique_ptr<ISwapChain>       CreateSwapChain(const SwapChainDesc &createInfo) override;
        std::unique_ptr<IRootSignature>   CreateRootSignature(const RootSignatureDesc &createInfo) override;
        std::unique_ptr<IInputLayout>     CreateInputLayout(const InputLayoutDesc &createInfo) override;
        std::unique_ptr<IResourceBindGroup> CreateResourceBindGroup(const ResourceBindGroupDesc &createInfo) override;
        std::unique_ptr<IFence>           CreateFence() override;
        std::unique_ptr<ISemaphore>       CreateSemaphore() override;
        std::unique_ptr<IBufferResource>  CreateBufferResource(std::string name, const BufferDesc &createInfo) override;
        std::unique_ptr<ITextureResource> CreateTextureResource(std::string name, const TextureDesc &createInfo) override;

        ~VulkanLogicalDevice() override;

    private:
        void CreateRenderSurface();

        bool                                               InitDebugMessages(const vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
        void                                               InitSupportedLayers(std::vector<const char *> &layers);
        [[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsCreateInfo() const;
        static void                                        LoadExtensionFunctions();

        void SetupQueueFamilies() const;
        void CreateLogicalDevice() const;
        void CreateSurface() const;
        void CreateImageFormat() const;

        void                                   InitializeVma() const;
        static void                            CreateDeviceInfo(const vk::PhysicalDevice &physicalDevice, PhysicalDevice &deviceInfo);
        std::vector<vk::DeviceQueueCreateInfo> CreateUniqueDeviceCreateInfos() const;
        void                                   DestroyDebugUtils() const;
    };

} // namespace DenOfIz
