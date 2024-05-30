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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#include "SDL_vulkan.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanDescriptorTable.h"
#include "DenOfIzGraphics/Backends/Vulkan/Resource/VulkanImageResource.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanCommandList.h"

using namespace DenOfIz;

static VKAPI_ATTR VkBool32 VKAPI_CALL g_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
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

void VulkanLogicalDevice::LoadExtensionFunctions()
{
	const vk::DynamicLoader dl;

	const auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void VulkanLogicalDevice::CreateDevice(SDL_Window* window)
{
	LoadExtensionFunctions();

	m_context = std::make_unique<VulkanContext>();
	m_context->Window = window;

	vk::ApplicationInfo appInfo{ "DenOfIz", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3, };

	vk::InstanceCreateInfo createInfo{{}, &appInfo };

	uint32_t sdlExtensionCount;
	if (!SDL_Vulkan_GetInstanceExtensions(m_context->Window, &sdlExtensionCount, nullptr))
	{
		LOG(Verbosity::Critical, "VulkanDevice", SDL_GetError());
	}

	std::vector<const char*> extensions(sdlExtensionCount);
	if (!SDL_Vulkan_GetInstanceExtensions(m_context->Window, &sdlExtensionCount, extensions.data()))
	{
		LOG(Verbosity::Critical, "VulkanDevice", SDL_GetError());
	}

	std::vector<const char*> layers;
	InitSupportedLayers(layers);

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
	if (m_supportedLayers.contains("VK_LAYER_KHRONOS_validation"))
	{
		debugUtilsCreateInfo = GetDebugUtilsCreateInfo();
		createInfo.pNext = &debugUtilsCreateInfo;

		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	else
	{
		createInfo.pNext = nullptr;
	}

	createInfo.setPEnabledExtensionNames(extensions);
	createInfo.setPEnabledLayerNames(layers);

	m_context->Instance = createInstance(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_context->Instance);

	const auto extensionProperties = vk::enumerateInstanceExtensionProperties(nullptr);
	for (vk::ExtensionProperties prp : extensionProperties)
	{
		this->m_supportedExtensions[prp.extensionName] = true;
	}

	if (m_supportedLayers.contains("VK_LAYER_KHRONOS_validation"))
	{
		InitDebugMessages(debugUtilsCreateInfo);
	}

	CreateSurface();
	m_context->ShaderCompiler.Init();
}

void VulkanLogicalDevice::InitSupportedLayers(std::vector<const char*>& layers)
{
	const auto layerProperties = vk::enumerateInstanceLayerProperties();

	for (vk::LayerProperties prp : layerProperties)
	{
		auto layerPair = m_enabledLayers.find(prp.layerName);

		if (layerPair != m_enabledLayers.end())
		{
			m_supportedLayers[prp.layerName] = true;
			layers.emplace_back(layerPair->first.c_str());
			LOG(Verbosity::Information, "VulkanDevice", "Enabled Layer: " + layerPair->first);
		}
	}
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanLogicalDevice::GetDebugUtilsCreateInfo() const
{
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

	debugUtilsCreateInfo.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);

	debugUtilsCreateInfo.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

	debugUtilsCreateInfo.setPfnUserCallback(g_DebugCallback);
	return debugUtilsCreateInfo;
}

Result<Unit> VulkanLogicalDevice::InitDebugMessages(const vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
	const auto instance = static_cast<VkInstance>(m_context->Instance);

	const auto createDebugUtils = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	const auto createInfoCast = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(createInfo);

	if (createDebugUtils == nullptr || createDebugUtils(instance, &createInfoCast, nullptr, &m_debugMessenger) != VK_SUCCESS)
	{
		return Error("Failed to initialize debugger!");
	}

	return Success({});
}

std::vector<PhysicalDeviceInfo> VulkanLogicalDevice::ListPhysicalDevices()
{
	const auto devices = m_context->Instance.enumeratePhysicalDevices();
	std::vector<PhysicalDeviceInfo> result;

	for (const auto& device : devices)
	{
		vk::PhysicalDevice physicalDevice = device;
		PhysicalDeviceInfo deviceInfo{};
		CreateDeviceInfo(physicalDevice, deviceInfo);
		result.push_back(deviceInfo);
	}

	return result;
}

void VulkanLogicalDevice::CreateDeviceInfo(const vk::PhysicalDevice& physicalDevice, PhysicalDeviceInfo& deviceInfo)
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

	deviceInfo.Id = deviceProperties.deviceID;
	deviceInfo.Name = std::string(deviceProperties.deviceName.data());

	// Todo actually read these from somewhere:
	deviceInfo.Properties.IsDedicated = true;
	deviceInfo.Capabilities.DedicatedTransferQueue = true;
	deviceInfo.Capabilities.ComputeShaders = true;
	deviceInfo.Capabilities.RayTracing = true;
	deviceInfo.Capabilities.GeometryShaders = true;
	deviceInfo.Capabilities.Tessellation = true;
}

void VulkanLogicalDevice::LoadPhysicalDevice(const PhysicalDeviceInfo& device)
{
	assertm(m_context->PhysicalDevice == VK_NULL_HANDLE, "A physical device is already selected for this logical device. Create a new Logical Device.");
	m_selectedDeviceInfo = device;
	m_context->SelectedDeviceInfo = device;

	for (const vk::PhysicalDevice physicalDevice : m_context->Instance.enumeratePhysicalDevices())
	{
		const vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
		if (deviceProperties.deviceID == device.Id)
		{
			m_context->PhysicalDevice = physicalDevice;
		}
	}

	assertm(m_context->PhysicalDevice != VK_NULL_HANDLE, "Invalid DeviceID provided.");

	CreateLogicalDevice();
	InitializeVma();
	CreateImageFormat();
	CreateRenderSurface();

	vk::CommandPoolCreateInfo graphicsCommandPoolCreateInfo{};
	graphicsCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	graphicsCommandPoolCreateInfo.queueFamilyIndex = m_context->QueueFamilies[QueueType::Graphics].Index;

	vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{};
	transferCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	transferCommandPoolCreateInfo.queueFamilyIndex = m_context->QueueFamilies[QueueType::Transfer].Index;

	m_context->GraphicsQueueCommandPool = m_context->LogicalDevice.createCommandPool(graphicsCommandPoolCreateInfo);
	m_context->TransferQueueCommandPool = m_context->LogicalDevice.createCommandPool(transferCommandPoolCreateInfo);

	// Todo find a better approach
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo { vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1000, 1000 };
	std::vector<vk::DescriptorPoolSize> poolSizes = { { vk::DescriptorType::eUniformBuffer, 1000 }, { vk::DescriptorType::eCombinedImageSampler, 1000 } };
	descriptorPoolCreateInfo.setPoolSizes(poolSizes);
	m_context->DescriptorPool = m_context->LogicalDevice.createDescriptorPool(descriptorPoolCreateInfo);
}

void VulkanLogicalDevice::SetupQueueFamilies() const
{
	auto exists = [&](const QueueType bit) -> bool
	{
		return m_context->QueueFamilies.contains(bit);
	};

	const auto localQueueFamilies = m_context->PhysicalDevice.getQueueFamilyProperties();

	uint32_t index = 0;
	for (const vk::QueueFamilyProperties& property : localQueueFamilies)
	{
		const bool hasGraphics = (property.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
		const bool hasTransfer = (property.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;

		if (hasGraphics && !exists(QueueType::Graphics))
		{
			m_context->QueueFamilies[QueueType::Graphics] = QueueFamily{ index, { static_cast<VkQueueFlags>(property.queueFlags) }};
		}
		else if (hasTransfer && !exists(QueueType::Transfer))
		{
			// Try to fetch a unique transfer queue
			m_context->QueueFamilies[QueueType::Transfer] = QueueFamily{ index, { static_cast<VkQueueFlags>(property.queueFlags) }};
		}

		const vk::Bool32 presentationSupport = m_context->PhysicalDevice.getSurfaceSupportKHR(index, m_context->Surface);

		if (presentationSupport && !exists(QueueType::Presentation))
		{
			m_context->QueueFamilies[QueueType::Presentation] = QueueFamily{ index, { static_cast<VkQueueFlags>(property.queueFlags) }};
		}

		++index;
	}

	if (!exists(QueueType::Transfer))
	{
		m_context->QueueFamilies[QueueType::Transfer] = m_context->QueueFamilies[QueueType::Graphics];
	}
}

void VulkanLogicalDevice::CreateLogicalDevice() const
{
	SetupQueueFamilies();

	const std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos = CreateUniqueDeviceCreateInfos();

	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;
	features.sampleRateShading = true;
	features.tessellationShader = true;

	vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeature(VK_TRUE);
	const vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature(VK_TRUE, &extendedDynamicStateFeature);

	vk::DeviceCreateInfo createInfo{};
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
	createInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	createInfo.setPEnabledExtensionNames(m_requiredExtensions);
	createInfo.pEnabledFeatures = &features;
	createInfo.pNext = &dynamicRenderingFeature;

	m_context->LogicalDevice = m_context->PhysicalDevice.createDevice(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_context->LogicalDevice);

	m_context->Queues[QueueType::Graphics] = vk::Queue{};
	m_context->Queues[QueueType::Presentation] = vk::Queue{};
	m_context->Queues[QueueType::Transfer] = vk::Queue{};

	m_context->LogicalDevice.getQueue(m_context->QueueFamilies[QueueType::Graphics].Index, 0, &m_context->Queues[QueueType::Graphics]);
	m_context->LogicalDevice.getQueue(m_context->QueueFamilies[QueueType::Presentation].Index, 0, &m_context->Queues[QueueType::Presentation]);
	m_context->LogicalDevice.getQueue(m_context->QueueFamilies[QueueType::Transfer].Index, 0, &m_context->Queues[QueueType::Transfer]);
}

void VulkanLogicalDevice::InitializeVma() const
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	allocatorInfo.physicalDevice = m_context->PhysicalDevice;
	allocatorInfo.device = m_context->LogicalDevice;
	allocatorInfo.instance = m_context->Instance;

	vmaCreateAllocator(&allocatorInfo, &m_context->Vma);
}

void VulkanLogicalDevice::CreateSurface() const
{
	const auto instance = static_cast<VkInstance>(m_context->Instance);
	auto surface = static_cast<VkSurfaceKHR>(m_context->Surface);
	SDL_Vulkan_CreateSurface(m_context->Window, instance, &surface);
	m_context->Surface = vk::SurfaceKHR(surface);
}

std::vector<vk::DeviceQueueCreateInfo> VulkanLogicalDevice::CreateUniqueDeviceCreateInfos() const
{
	std::unordered_map<uint32_t, bool> uniqueIndexes;
	std::vector<vk::DeviceQueueCreateInfo> result;

	for (std::pair<QueueType, QueueFamily> key : m_context->QueueFamilies)
	{
		if (!uniqueIndexes.contains(key.second.Index))
		{
			float priority = key.first == QueueType::Graphics || key.first == QueueType::Presentation ? 1.0f : 0.9f;

			result.emplace_back(vk::DeviceQueueCreateFlagBits(), key.second.Index, 1, &priority);

			uniqueIndexes[key.second.Index] = true;
		}
	}

	return result;
}

void VulkanLogicalDevice::WaitIdle()
{
	m_context->LogicalDevice.waitIdle();
}

void VulkanLogicalDevice::CreateRenderSurface()
{
	m_context->LogicalDevice.waitIdle();

//	auto* renderSurfacePtr = new VulkanSurface{ m_context.get() };
//	this->m_renderSurface = std::unique_ptr<VulkanSurface>(renderSurfacePtr);
}

VulkanLogicalDevice::~VulkanLogicalDevice()
{
//	m_renderSurface.reset();
	DestroyDebugUtils();

	if (m_context == nullptr)
	{
		return;
	}

	m_context->LogicalDevice.destroyDescriptorPool(m_context->DescriptorPool);
	m_context->LogicalDevice.destroyCommandPool(m_context->TransferQueueCommandPool);
	m_context->LogicalDevice.destroyCommandPool(m_context->GraphicsQueueCommandPool);
	m_context->LogicalDevice.destroyCommandPool(m_context->ComputeQueueCommandPool);

	m_context->Instance.destroySurfaceKHR(m_context->Surface);
	vmaDestroyAllocator(m_context->Vma);
	m_context->LogicalDevice.destroy();
	m_context->Instance.destroy();

	m_context->ShaderCompiler.Destroy();
}

void VulkanLogicalDevice::DestroyDebugUtils() const
{
	if (m_debugMessenger == VK_NULL_HANDLE)
	{
		return;
	}

	const auto instance = static_cast<VkInstance>(m_context->Instance);

	if (const auto deleteDebugUtils = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")))
	{
		deleteDebugUtils(instance, m_debugMessenger, nullptr);
	}
}

VulkanContext* VulkanLogicalDevice::GetContext() const
{
	return m_context.get();
}

void VulkanLogicalDevice::CreateImageFormat() const
{
	const auto surfaceFormats = m_context->PhysicalDevice.getSurfaceFormatsKHR(m_context->Surface);
	const auto presentModes = m_context->PhysicalDevice.getSurfacePresentModesKHR(m_context->Surface);

	auto presentMode = vk::PresentModeKHR::eImmediate;
	for (const auto mode : presentModes)
	{
		if (mode == vk::PresentModeKHR::eImmediate)
		{
			presentMode = mode;
		}
	}

	auto surfaceFormat = vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	for (const auto format : surfaceFormats)
	{
		if (format.format == vk::Format::eB8G8R8A8Unorm /*&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear*/ )
		{
			surfaceFormat = format;
		}
	}

	m_context->SurfaceImageFormat = ImageFormat::B8G8R8A8Unorm;
	m_context->ColorSpace = surfaceFormat.colorSpace;
	m_context->PresentMode = presentMode;
}

uint32_t VulkanLogicalDevice::GetFrameCount() const
{
	return m_context->SwapChainImages.size();
}

ImageFormat VulkanLogicalDevice::GetSwapChainImageFormat() const
{
	return m_context->SurfaceImageFormat;
}

std::unique_ptr<ICommandList> VulkanLogicalDevice::CreateCommandList(const CommandListCreateInfo& createInfo)
{
	VulkanCommandList* commandList = new VulkanCommandList(m_context.get(), createInfo);
	return std::unique_ptr<ICommandList>(dynamic_cast<ICommandList*>(commandList));
}

std::unique_ptr<IPipeline> VulkanLogicalDevice::CreatePipeline(const PipelineCreateInfo& createInfo)
{
	VulkanPipeline* pipeline = new VulkanPipeline(m_context.get(), createInfo);
	return std::unique_ptr<IPipeline>(pipeline);
}

std::unique_ptr<ISwapChain> VulkanLogicalDevice::CreateSwapChain(const SwapChainCreateInfo& createInfo)
{
	VulkanSwapChain* swapChain = new VulkanSwapChain(m_context.get(), createInfo);
	return std::unique_ptr<ISwapChain>(swapChain);
}

std::unique_ptr<IRootSignature> VulkanLogicalDevice::CreateRootSignature(const RootSignatureCreateInfo& createInfo)
{
	VulkanRootSignature* rootSignature = new VulkanRootSignature(m_context.get(), createInfo);
	return std::unique_ptr<IRootSignature>(rootSignature);
}

std::unique_ptr<IDescriptorTable> VulkanLogicalDevice::CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo)
{
	VulkanDescriptorTable* descriptorTable = new VulkanDescriptorTable(m_context.get(), createInfo);
	return std::unique_ptr<IDescriptorTable>(descriptorTable);
}

std::unique_ptr<IBufferResource> VulkanLogicalDevice::CreateBufferResource(std::string name, const BufferCreateInfo& createInfo)
{
	VulkanBufferResource* bufferResource = new VulkanBufferResource(m_context.get(), createInfo);
	bufferResource->Name = name;
	return std::unique_ptr<IBufferResource>(bufferResource);
}

std::unique_ptr<IImageResource> VulkanLogicalDevice::CreateImageResource(std::string name, const ImageCreateInfo& createInfo)
{
	VulkanImageResource* imageResource = new VulkanImageResource(m_context.get(), createInfo);
	imageResource->Name = name;
	return std::unique_ptr<IImageResource>(imageResource);
}

std::unique_ptr<IFence> VulkanLogicalDevice::CreateFence()
{
	VulkanFence* fence = new VulkanFence(m_context.get());
	return std::unique_ptr<IFence>(fence);
}

std::unique_ptr<ISemaphore> VulkanLogicalDevice::CreateSemaphore()
{
	VulkanSemaphore* semaphore = new VulkanSemaphore(m_context.get());
	return std::unique_ptr<ISemaphore>(semaphore);
}
