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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanDevice.h>
#include "SDL_vulkan.h"

using namespace DenOfIz;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	Verbosity verbosity;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		verbosity = Verbosity::Debug;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		verbosity = Verbosity::Information;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		verbosity = Verbosity::Warning;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		verbosity = Verbosity::Critical;
		break;
	}

	LOG(verbosity, "VulkanDevice", pCallbackData->pMessage);
	return VK_FALSE;
}

void VulkanDevice::LoadExtensionFunctions()
{
	const vk::DynamicLoader dl;

	const auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void VulkanDevice::CreateDevice(SDL_Window* window)
{
	LoadExtensionFunctions();

	Context = std::make_unique<VulkanContext>();
	Context->Window = window;

	vk::ApplicationInfo appInfo{ "DenOfIz", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3, };

	vk::InstanceCreateInfo createInfo{{}, &appInfo };

	uint32_t sdlExtensionCount;
	if (!SDL_Vulkan_GetInstanceExtensions(Context->Window, &sdlExtensionCount, nullptr))
	{
		LOG(Verbosity::Critical, "VulkanDevice", SDL_GetError());
	}

	std::vector<const char*> extensions(sdlExtensionCount);
	if (!SDL_Vulkan_GetInstanceExtensions(Context->Window, &sdlExtensionCount, extensions.data()))
	{
		LOG(Verbosity::Critical, "VulkanDevice", SDL_GetError());
	}

	std::vector<const char*> layers;
	InitSupportedLayers(layers);

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
	if (SupportedLayers.find("VK_LAYER_KHRONOS_validation") != SupportedLayers.end())
	{
		debugUtilsCreateInfo = GetDebugUtilsCreateInfo();
		createInfo.pNext = static_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(&debugUtilsCreateInfo);

		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
	if (!layers.empty())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
	}

	Context->Instance = vk::createInstance(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(Context->Instance);

	auto extensionProperties = vk::enumerateInstanceExtensionProperties(nullptr);
	for (vk::ExtensionProperties prp : extensionProperties)
	{
		this->SupportedExtensions[prp.extensionName] = true;
	}

	if (SupportedLayers.find("VK_LAYER_KHRONOS_validation") != SupportedLayers.end())
	{
		InitDebugMessages(debugUtilsCreateInfo);
	}

	CreateSurface();
	Context->ShaderCompiler.Init();
}

void VulkanDevice::InitSupportedLayers(std::vector<const char*>& layers)
{
	auto layerProperties = vk::enumerateInstanceLayerProperties();

	for (vk::LayerProperties prp : layerProperties)
	{
		auto layerPair = ENABLED_LAYERS.find(prp.layerName);

		if (layerPair != ENABLED_LAYERS.end())
		{
			SupportedLayers[prp.layerName] = true;
			layers.emplace_back(layerPair->first.c_str());
		}
	}
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanDevice::GetDebugUtilsCreateInfo() const
{
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

	debugUtilsCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);

	debugUtilsCreateInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

	debugUtilsCreateInfo.setPfnUserCallback(debugCallback);
	return debugUtilsCreateInfo;
}

Result<Unit> VulkanDevice::InitDebugMessages(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
	auto instance = static_cast<VkInstance>(Context->Instance);

	auto createDebugUtils = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	auto createInfoCast = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(createInfo);

	if (createDebugUtils == nullptr || createDebugUtils(instance, &createInfoCast, nullptr, &DebugMessenger) != VK_SUCCESS)
	{
		return Error("Failed to initialize debugger!");
	}

	return Success({});
}

std::vector<SelectableDevice> VulkanDevice::ListDevices()
{
	auto devices = Context->Instance.enumeratePhysicalDevices();
	std::vector<SelectableDevice> result;

	for (auto& device : devices)
	{
		vk::PhysicalDevice physicalDevice = device;

		SelectableDevice selectableDevice{};

		CreateDeviceInfo(physicalDevice, selectableDevice.Device);

		selectableDevice.Select = [=, this]()
		{
			SelectDevice(device);
		};

		result.push_back(selectableDevice);
	}

	return result;
}

void VulkanDevice::CreateDeviceInfo(const vk::PhysicalDevice& physicalDevice, DeviceInfo& deviceInfo)
{
	vk::PhysicalDeviceFeatures2 deviceFeatures;
	vk::PhysicalDeviceProperties deviceProperties;

	std::vector<vk::QueueFamilyProperties> localQueueFamilies;

	physicalDevice.getProperties(&deviceProperties);
	physicalDevice.getFeatures2(&deviceFeatures);

	uint32_t queueFamilyCount;
	physicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);
	physicalDevice.getQueueFamilyProperties(&queueFamilyCount, localQueueFamilies.data());

	auto extensions = physicalDevice.enumerateDeviceExtensionProperties(nullptr);

	deviceInfo.Name = std::string(deviceProperties.deviceName.data());
	deviceInfo.Properties.IsDedicated = true; // todo
	deviceInfo.Capabilities.DedicatedTransferQueue = true; // todo
}

void VulkanDevice::SelectDevice(const vk::PhysicalDevice& device)
{
	// Todo break if already initialized
	Context->PhysicalDevice = device;

	CreateLogicalDevice();
	InitializeVMA();
	CreateImageFormat();
	CreateRenderSurface();

	vk::CommandPoolCreateInfo graphicsCommandPoolCreateInfo{};
	graphicsCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	graphicsCommandPoolCreateInfo.queueFamilyIndex = Context->QueueFamilies[QueueType::Graphics].index;

	vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{};
	transferCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	transferCommandPoolCreateInfo.queueFamilyIndex = Context->QueueFamilies[QueueType::Transfer].index;

	Context->GraphicsQueueCommandPool = Context->LogicalDevice.createCommandPool(graphicsCommandPoolCreateInfo);
	Context->TransferQueueCommandPool = Context->LogicalDevice.createCommandPool(transferCommandPoolCreateInfo);
}

void VulkanDevice::SetupQueueFamilies()
{
	auto exists = [&](QueueType bit) -> bool
	{
		return Context->QueueFamilies.find(bit) != Context->QueueFamilies.end();
	};

	auto localQueueFamilies = Context->PhysicalDevice.getQueueFamilyProperties();

	uint32_t index = 0;
	for (const vk::QueueFamilyProperties& property : localQueueFamilies)
	{
		bool hasGraphics = (property.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
		bool hasTransfer = (property.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;

		if (hasGraphics && !exists(QueueType::Graphics))
		{
			Context->QueueFamilies[QueueType::Graphics] = QueueFamily{ index, VkQueueFlags(property.queueFlags) };
		}
		else if (hasTransfer && !exists(QueueType::Transfer))
		{ // Try to fetch a unique transfer queue
			Context->QueueFamilies[QueueType::Transfer] = QueueFamily{ index, VkQueueFlags(property.queueFlags) };
		}

		vk::Bool32 presentationSupport = Context->PhysicalDevice.getSurfaceSupportKHR(index, Context->Surface);

		if (presentationSupport && !exists(QueueType::Presentation))
		{
			Context->QueueFamilies[QueueType::Presentation] = QueueFamily{ index, VkQueueFlags(property.queueFlags) };
		}

		++index;
	}

	if (!exists(QueueType::Transfer))
	{
		Context->QueueFamilies[QueueType::Transfer] = Context->QueueFamilies[QueueType::Graphics];
	}
}

void VulkanDevice::CreateLogicalDevice()
{
	SetupQueueFamilies();

	std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos = CreateUniqueDeviceCreateInfos();

	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;
	features.sampleRateShading = true;
	features.tessellationShader = true;

	vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeature(VK_TRUE);
	vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature(VK_TRUE, &extendedDynamicStateFeature);

	vk::DeviceCreateInfo createInfo{};
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
	createInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = REQUIRED_EXTENSIONS.data();
	createInfo.pEnabledFeatures = &features;
	createInfo.pNext = &dynamicRenderingFeature;

	Context->LogicalDevice = Context->PhysicalDevice.createDevice(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(Context->LogicalDevice);

	Context->Queues[QueueType::Graphics] = vk::Queue{};
	Context->Queues[QueueType::Presentation] = vk::Queue{};
	Context->Queues[QueueType::Transfer] = vk::Queue{};

	Context->LogicalDevice.getQueue(Context->QueueFamilies[QueueType::Graphics].index, 0, &Context->Queues[QueueType::Graphics]);
	Context->LogicalDevice.getQueue(Context->QueueFamilies[QueueType::Presentation].index, 0, &Context->Queues[QueueType::Presentation]);
	Context->LogicalDevice.getQueue(Context->QueueFamilies[QueueType::Transfer].index, 0, &Context->Queues[QueueType::Transfer]);
}

void VulkanDevice::InitializeVMA()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorInfo.physicalDevice = Context->PhysicalDevice;
	allocatorInfo.device = Context->LogicalDevice;
	allocatorInfo.instance = Context->Instance;

	vmaCreateAllocator(&allocatorInfo, &Context->Vma);
}

void VulkanDevice::CreateSurface()
{
	auto instance = static_cast<VkInstance>(Context->Instance);
	auto surface = static_cast<VkSurfaceKHR>(Context->Surface);
	SDL_Vulkan_CreateSurface(Context->Window, instance, &surface);
	Context->Surface = vk::SurfaceKHR(surface);
}

std::unordered_map<std::string, bool> VulkanDevice::DefaultRequiredExtensions()
{
	std::unordered_map<std::string, bool> result;
	result[VK_KHR_SWAPCHAIN_EXTENSION_NAME] = true;
	return result;
}

std::vector<vk::DeviceQueueCreateInfo> VulkanDevice::CreateUniqueDeviceCreateInfos()
{
	std::unordered_map<uint32_t, bool> uniqueIndexes;
	std::vector<vk::DeviceQueueCreateInfo> result;

	for (std::pair<QueueType, QueueFamily> key : Context->QueueFamilies)
	{
		if (uniqueIndexes.find(key.second.index) == uniqueIndexes.end())
		{
			float priority = key.first == QueueType::Graphics || key.first == QueueType::Presentation ? 1.0f : 0.9f;

			result.emplace_back(vk::DeviceQueueCreateInfo{ vk::DeviceQueueCreateFlagBits(), key.second.index, 1, &priority });

			uniqueIndexes[key.second.index] = true;
		}
	}

	return result;
}

void VulkanDevice::WaitIdle()
{
	Context->LogicalDevice.waitIdle();
}

void VulkanDevice::CreateRenderSurface()
{
	Context->LogicalDevice.waitIdle();

	auto* renderSurfacePtr = new VulkanSurface{ Context.get() };
	this->RenderSurface = std::unique_ptr<VulkanSurface>(renderSurfacePtr);
}

VulkanDevice::~VulkanDevice()
{
	RenderSurface.reset();
	DestroyDebugUtils();

	if (Context == nullptr)
	{
		return;
	}

	Context->LogicalDevice.destroyCommandPool(Context->TransferQueueCommandPool);
	Context->LogicalDevice.destroyCommandPool(Context->GraphicsQueueCommandPool);
	Context->LogicalDevice.destroyCommandPool(Context->ComputeQueueCommandPool);

	Context->Instance.destroySurfaceKHR(Context->Surface);
	vmaDestroyAllocator(Context->Vma);
	Context->LogicalDevice.destroy();
	Context->Instance.destroy();

	Context->ShaderCompiler.Destroy();
}

void VulkanDevice::DestroyDebugUtils() const
{
	if (DebugMessenger == VK_NULL_HANDLE)
	{
		return;
	}

	auto instance = static_cast<VkInstance>(Context->Instance);
	auto deleteDebugUtils = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (deleteDebugUtils)
	{
		deleteDebugUtils(static_cast<VkInstance>(instance), DebugMessenger, nullptr);
	}
}

VulkanContext* VulkanDevice::GetContext() const
{
	return Context.get();
}

void VulkanDevice::CreateImageFormat()
{
	auto surfaceFormats = Context->PhysicalDevice.getSurfaceFormatsKHR(Context->Surface);
	auto presentModes = Context->PhysicalDevice.getSurfacePresentModesKHR(Context->Surface);

	auto presentMode = vk::PresentModeKHR::eImmediate;
	for (auto mode : presentModes)
	{
		if (mode == vk::PresentModeKHR::eImmediate)
		{
			presentMode = mode;
		}
	}

	auto surfaceFormat = vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	for (auto format : surfaceFormats)
	{
		if (format.format == vk::Format::eB8G8R8A8Unorm /*&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear*/ )
		{
			surfaceFormat = format;
		}
	}

	Context->SurfaceImageFormat = ImageFormat::B8G8R8A8Unorm;
	Context->ColorSpace = surfaceFormat.colorSpace;
	Context->PresentMode = presentMode;
}

uint32_t VulkanDevice::GetFrameCount() const
{
	return Context->SwapChainImages.size();
}

ImageFormat VulkanDevice::GetSwapChainImageFormat()
{
	return Context->SurfaceImageFormat;
}