/*
Blazar Engine - 3D Game Engine
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
#ifdef BUILD_VK

#include "VulkanContext.h"
#include "VulkanSurface.h"
#include "VulkanPipeline.h"
#include "Resource/VulkanCubeMapResource.h"
#include "Resource/VulkanBufferResource.h"
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"

namespace DenOfIz
{

class VulkanLogicalDevice final : public ILogicalDevice
{
	const std::unordered_map<std::string, bool> m_enabledLayers{
#ifdef _DEBUG
			{ "VK_LAYER_KHRONOS_validation", true }
#endif
	};

	const std::vector<const char*> m_requiredExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			// Maintenance Extensions
														 VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_MAINTENANCE2_EXTENSION_NAME, VK_KHR_MAINTENANCE3_EXTENSION_NAME,
			// Dynamic Rendering
														 VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
														 VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
			// Used to pass Viewport and Scissor count.
														 VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME
#if __APPLE_CC__
			,"VK_KHR_portability_subset"
#endif
	};

	const std::vector<QueueType> m_queueTypes = { QueueType::Graphics, QueueType::Transfer, QueueType::Presentation };

	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	std::unordered_map<std::string, bool> m_supportedExtensions;
	std::unordered_map<std::string, bool> m_supportedLayers;

	std::unique_ptr<VulkanContext> m_context;

public:
	VulkanLogicalDevice() = default;

	void CreateDevice(SDL_Window* window) override;
	std::vector<PhysicalDeviceInfo> ListPhysicalDevices() override;
	void LoadPhysicalDevice(const PhysicalDeviceInfo& device) override;
	inline bool IsDeviceLost() override {
		return m_context->IsDeviceLost;
	}

	void WaitIdle() override;
	[[nodiscard]] uint32_t GetFrameCount() const;
	[[nodiscard]] VulkanContext* GetContext() const;
	[[nodiscard]] ImageFormat GetSwapChainImageFormat() const;

	// Factory methods
	std::unique_ptr<ICommandList> CreateCommandList(const CommandListCreateInfo& createInfo) override;
	std::unique_ptr<IPipeline> CreatePipeline(const PipelineCreateInfo& createInfo) override;
	std::unique_ptr<ISwapChain> CreateSwapChain(const SwapChainCreateInfo& createInfo) override;
	std::unique_ptr<IRootSignature> CreateRootSignature(const RootSignatureCreateInfo& createInfo) override;
	std::unique_ptr<IDescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) override;
	std::unique_ptr<IFence> CreateFence() override;
	std::unique_ptr<ISemaphore> CreateSemaphore() override;
	std::unique_ptr<IBufferResource> CreateBufferResource(std::string name, const BufferCreateInfo& createInfo) override;
	std::unique_ptr<IImageResource> CreateImageResource(std::string name, const ImageCreateInfo& createInfo) override;

	~VulkanLogicalDevice() override;

private:
	void CreateRenderSurface();

	Result<Unit> InitDebugMessages(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
	void InitSupportedLayers(std::vector<const char*>& layers);
	[[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsCreateInfo() const;
	static void LoadExtensionFunctions();

	void SetupQueueFamilies() const;
	void CreateLogicalDevice() const;
	void CreateSurface() const;
	void CreateImageFormat() const;

	void InitializeVma() const;
	static void CreateDeviceInfo(const vk::PhysicalDevice& physicalDevice, PhysicalDeviceInfo& deviceInfo);
	std::vector<vk::DeviceQueueCreateInfo> CreateUniqueDeviceCreateInfos() const;
	void DestroyDebugUtils() const;
};

}

#endif
