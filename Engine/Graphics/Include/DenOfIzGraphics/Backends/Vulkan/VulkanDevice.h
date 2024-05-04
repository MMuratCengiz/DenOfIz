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

#include "VulkanContext.h"
#include "VulkanSurface.h"
#include "VulkanPipeline.h"
#include <DenOfIzCore/Logger.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include "DenOfIzGraphics/Backends/Interface/IDevice.h"

namespace DenOfIz
{

class VulkanDevice
{
private:
	const std::unordered_map<std::string, bool> ENABLED_LAYERS {
#if DEBUG
//			{ "VK_LAYER_KHRONOS_validation", true }
#endif
	};

	const std::vector<const char*> REQUIRED_EXTENSIONS{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			// Maintenance Extensions
			VK_KHR_MAINTENANCE1_EXTENSION_NAME,
			VK_KHR_MAINTENANCE2_EXTENSION_NAME,
			VK_KHR_MAINTENANCE3_EXTENSION_NAME,
			// Dynamic Rendering
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
			VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
			// Used to pass Viewport and Scissor count.
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
#if __APPLE_CC__
			,"VK_KHR_portability_subset"
#endif
	};

	const std::vector<QueueType> QueueTypes = {
			QueueType::Graphics,
			QueueType::Transfer,
			QueueType::Presentation
	};

	VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
	std::unordered_map<std::string, bool> SupportedExtensions;
	std::unordered_map<std::string, bool> SupportedLayers;

	std::unique_ptr<VulkanContext> Context;
	std::unique_ptr<VulkanSurface> RenderSurface;
public:
	VulkanDevice() = default;

	void CreateDevice(SDL_Window* window);
	std::vector<SelectableDevice> ListDevices();
	void SelectDevice(const vk::PhysicalDevice& device);


	void WaitIdle();
	uint32_t GetFrameCount() const;
	VulkanContext* GetContext() const;
	ImageFormat GetSwapChainImageFormat();

	~VulkanDevice();

private:
	void CreateRenderSurface();

	Result<Unit> InitDebugMessages(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
	void InitSupportedLayers(std::vector<const char*>& layers);
	vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsCreateInfo() const;
	static void LoadExtensionFunctions();

	void SetupQueueFamilies();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateImageFormat();

	void InitializeVMA();
	static std::unordered_map<std::string, bool> DefaultRequiredExtensions();
	static void CreateDeviceInfo(const vk::PhysicalDevice& physicalDevice, DeviceInfo& deviceInfo);
	std::vector<vk::DeviceQueueCreateInfo> CreateUniqueDeviceCreateInfos();
	void DestroyDebugUtils() const;
};

}

