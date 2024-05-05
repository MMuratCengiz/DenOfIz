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

#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/VulkanDevice.h>
#include "SDL_vulkan.h"

using namespace DenOfIz;

static VKAPI_ATTR VkBool32 VKAPI_CALL g_DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData )
{
    Verbosity verbosity;

    switch ( messageSeverity )
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

    LOG( verbosity, "VulkanDevice", pCallbackData->pMessage );
    return VK_FALSE;
}

void VulkanDevice::LoadExtensionFunctions()
{
    const vk::DynamicLoader dl;

    const auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>( "vkGetInstanceProcAddr" );

    VULKAN_HPP_DEFAULT_DISPATCHER.init( vkGetInstanceProcAddr );
}

void VulkanDevice::CreateDevice( SDL_Window *window )
{
    LoadExtensionFunctions();

    m_Context = std::make_unique<VulkanContext>();
    m_Context->Window = window;

    vk::ApplicationInfo appInfo{ "DenOfIz", VK_MAKE_VERSION( 1, 0, 0 ), "No Engine", VK_MAKE_VERSION( 1, 0, 0 ), VK_API_VERSION_1_3, };

    vk::InstanceCreateInfo createInfo{ {}, &appInfo };

    uint32_t sdlExtensionCount;
    if ( !SDL_Vulkan_GetInstanceExtensions( m_Context->Window, &sdlExtensionCount, nullptr ) )
    {
        LOG( Verbosity::Critical, "VulkanDevice", SDL_GetError() );
    }

    std::vector<const char *> extensions( sdlExtensionCount );
    if ( !SDL_Vulkan_GetInstanceExtensions( m_Context->Window, &sdlExtensionCount, extensions.data() ) )
    {
        LOG( Verbosity::Critical, "VulkanDevice", SDL_GetError() );
    }

    std::vector<const char *> layers;
    InitSupportedLayers( layers );

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
    if ( m_SupportedLayers.contains( "VK_LAYER_KHRONOS_validation" ) )
    {
        debugUtilsCreateInfo = GetDebugUtilsCreateInfo();
        createInfo.pNext = &debugUtilsCreateInfo;

        extensions.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }

    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    if ( !layers.empty() )
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
    }

    m_Context->Instance = createInstance( createInfo );
    VULKAN_HPP_DEFAULT_DISPATCHER.init( m_Context->Instance );

    auto extensionProperties = vk::enumerateInstanceExtensionProperties( nullptr );
    for ( vk::ExtensionProperties prp : extensionProperties )
    {
        this->m_SupportedExtensions[ prp.extensionName ] = true;
    }

    if ( m_SupportedLayers.contains( "VK_LAYER_KHRONOS_validation" ) )
    {
        InitDebugMessages( debugUtilsCreateInfo );
    }

    CreateSurface();
    m_Context->ShaderCompiler.Init();
}

void VulkanDevice::InitSupportedLayers( std::vector<const char *> &layers )
{
    const auto layerProperties = vk::enumerateInstanceLayerProperties();

    for ( vk::LayerProperties prp : layerProperties )
    {
        auto layerPair = m_EnabledLayers.find( prp.layerName );

        if ( layerPair != m_EnabledLayers.end() )
        {
            m_SupportedLayers[ prp.layerName ] = true;
            layers.emplace_back( layerPair->first.c_str() );
        }
    }
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanDevice::GetDebugUtilsCreateInfo() const
{
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

    debugUtilsCreateInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning );

    debugUtilsCreateInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance );

    debugUtilsCreateInfo.setPfnUserCallback( g_DebugCallback );
    return debugUtilsCreateInfo;
}

Result<Unit> VulkanDevice::InitDebugMessages( const vk::DebugUtilsMessengerCreateInfoEXT &createInfo )
{
    const auto instance = static_cast<VkInstance>(m_Context->Instance);

    const auto createDebugUtils = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" ));

    const auto createInfoCast = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(createInfo);

    if ( createDebugUtils == nullptr || createDebugUtils( instance, &createInfoCast, nullptr, &m_DebugMessenger ) != VK_SUCCESS )
    {
        return Error( "Failed to initialize debugger!" );
    }

    return Success( {} );
}

std::vector<SelectableDevice> VulkanDevice::ListDevices()
{
    const auto devices = m_Context->Instance.enumeratePhysicalDevices();
    std::vector<SelectableDevice> result;

    for ( const auto &device : devices )
    {
        vk::PhysicalDevice physicalDevice = device;

        SelectableDevice selectableDevice{};

        CreateDeviceInfo( physicalDevice, selectableDevice.Device );

        selectableDevice.Select = [=, this]()
        {
            SelectDevice( device );
        };

        result.push_back( selectableDevice );
    }

    return result;
}

void VulkanDevice::CreateDeviceInfo( const vk::PhysicalDevice &physicalDevice, DeviceInfo &deviceInfo )
{
    vk::PhysicalDeviceFeatures2 deviceFeatures;
    vk::PhysicalDeviceProperties deviceProperties;

    std::vector<vk::QueueFamilyProperties> localQueueFamilies;

    physicalDevice.getProperties( &deviceProperties );
    physicalDevice.getFeatures2( &deviceFeatures );

    uint32_t queueFamilyCount;
    physicalDevice.getQueueFamilyProperties( &queueFamilyCount, nullptr );
    physicalDevice.getQueueFamilyProperties( &queueFamilyCount, localQueueFamilies.data() );

    auto extensions = physicalDevice.enumerateDeviceExtensionProperties( nullptr );

    deviceInfo.Name = std::string( deviceProperties.deviceName.data() );
    deviceInfo.Properties.IsDedicated = true; // todo
    deviceInfo.Capabilities.DedicatedTransferQueue = true; // todo
}

void VulkanDevice::SelectDevice( const vk::PhysicalDevice &device )
{
    // Todo break if already initialized
    m_Context->PhysicalDevice = device;

    CreateLogicalDevice();
    InitializeVma();
    CreateImageFormat();
    CreateRenderSurface();

    vk::CommandPoolCreateInfo graphicsCommandPoolCreateInfo{};
    graphicsCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    graphicsCommandPoolCreateInfo.queueFamilyIndex = m_Context->QueueFamilies[ QueueType::Graphics ].Index;

    vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{};
    transferCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    transferCommandPoolCreateInfo.queueFamilyIndex = m_Context->QueueFamilies[ QueueType::Transfer ].Index;

    m_Context->GraphicsQueueCommandPool = m_Context->LogicalDevice.createCommandPool( graphicsCommandPoolCreateInfo );
    m_Context->TransferQueueCommandPool = m_Context->LogicalDevice.createCommandPool( transferCommandPoolCreateInfo );
}

void VulkanDevice::SetupQueueFamilies() const
{
    auto exists = [&]( const QueueType bit ) -> bool
    {
        return m_Context->QueueFamilies.contains( bit );
    };

    const auto localQueueFamilies = m_Context->PhysicalDevice.getQueueFamilyProperties();

    uint32_t index = 0;
    for ( const vk::QueueFamilyProperties &property : localQueueFamilies )
    {
        const bool hasGraphics = (property.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
        const bool hasTransfer = (property.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;

        if ( hasGraphics && !exists( QueueType::Graphics ) )
        {
            m_Context->QueueFamilies[ QueueType::Graphics ] = QueueFamily{ index, static_cast<VkQueueFlags>(property.queueFlags) };
        }
        else if ( hasTransfer && !exists( QueueType::Transfer ) )
        {
            // Try to fetch a unique transfer queue
            m_Context->QueueFamilies[ QueueType::Transfer ] = QueueFamily{ index, static_cast<VkQueueFlags>(property.queueFlags) };
        }

        const vk::Bool32 presentationSupport = m_Context->PhysicalDevice.getSurfaceSupportKHR( index, m_Context->Surface );

        if ( presentationSupport && !exists( QueueType::Presentation ) )
        {
            m_Context->QueueFamilies[ QueueType::Presentation ] = QueueFamily{ index, static_cast<VkQueueFlags>(property.queueFlags) };
        }

        ++index;
    }

    if ( !exists( QueueType::Transfer ) )
    {
        m_Context->QueueFamilies[ QueueType::Transfer ] = m_Context->QueueFamilies[ QueueType::Graphics ];
    }
}

void VulkanDevice::CreateLogicalDevice() const
{
    SetupQueueFamilies();

    const std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos = CreateUniqueDeviceCreateInfos();

    vk::PhysicalDeviceFeatures features{};
    features.samplerAnisotropy = true;
    features.sampleRateShading = true;
    features.tessellationShader = true;

    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeature( VK_TRUE );
    const vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature( VK_TRUE, &extendedDynamicStateFeature );

    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
    createInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_RequiredExtensions.size());
    createInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();
    createInfo.pEnabledFeatures = &features;
    createInfo.pNext = &dynamicRenderingFeature;

    m_Context->LogicalDevice = m_Context->PhysicalDevice.createDevice( createInfo );
    VULKAN_HPP_DEFAULT_DISPATCHER.init( m_Context->LogicalDevice );

    m_Context->Queues[ QueueType::Graphics ] = vk::Queue{};
    m_Context->Queues[ QueueType::Presentation ] = vk::Queue{};
    m_Context->Queues[ QueueType::Transfer ] = vk::Queue{};

    m_Context->LogicalDevice.getQueue( m_Context->QueueFamilies[ QueueType::Graphics ].Index, 0, &m_Context->Queues[ QueueType::Graphics ] );
    m_Context->LogicalDevice.getQueue( m_Context->QueueFamilies[ QueueType::Presentation ].Index, 0, &m_Context->Queues[ QueueType::Presentation ] );
    m_Context->LogicalDevice.getQueue( m_Context->QueueFamilies[ QueueType::Transfer ].Index, 0, &m_Context->Queues[ QueueType::Transfer ] );
}

void VulkanDevice::InitializeVma() const
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice = m_Context->PhysicalDevice;
    allocatorInfo.device = m_Context->LogicalDevice;
    allocatorInfo.instance = m_Context->Instance;

    vmaCreateAllocator( &allocatorInfo, &m_Context->Vma );
}

void VulkanDevice::CreateSurface() const
{
    const auto instance = static_cast<VkInstance>(m_Context->Instance);
    auto surface = static_cast<VkSurfaceKHR>(m_Context->Surface);
    SDL_Vulkan_CreateSurface( m_Context->Window, instance, &surface );
    m_Context->Surface = vk::SurfaceKHR( surface );
}

std::unordered_map<std::string, bool> VulkanDevice::DefaultRequiredExtensions()
{
    std::unordered_map<std::string, bool> result;
    result[ VK_KHR_SWAPCHAIN_EXTENSION_NAME ] = true;
    return result;
}

std::vector<vk::DeviceQueueCreateInfo> VulkanDevice::CreateUniqueDeviceCreateInfos() const
{
    std::unordered_map<uint32_t, bool> uniqueIndexes;
    std::vector<vk::DeviceQueueCreateInfo> result;

    for ( std::pair<QueueType, QueueFamily> key : m_Context->QueueFamilies )
    {
        if ( !uniqueIndexes.contains( key.second.Index ) )
        {
            float priority = key.first == QueueType::Graphics || key.first == QueueType::Presentation ? 1.0f : 0.9f;

            result.emplace_back( vk::DeviceQueueCreateFlagBits(), key.second.Index, 1, &priority );

            uniqueIndexes[ key.second.Index ] = true;
        }
    }

    return result;
}

void VulkanDevice::WaitIdle()
{
    m_Context->LogicalDevice.waitIdle();
}

void VulkanDevice::CreateRenderSurface()
{
    m_Context->LogicalDevice.waitIdle();

    auto *renderSurfacePtr = new VulkanSurface{ m_Context.get() };
    this->m_RenderSurface = std::unique_ptr<VulkanSurface>( renderSurfacePtr );
}

VulkanDevice::~VulkanDevice()
{
    m_RenderSurface.reset();
    DestroyDebugUtils();

    if ( m_Context == nullptr )
    {
        return;
    }

    m_Context->LogicalDevice.destroyCommandPool( m_Context->TransferQueueCommandPool );
    m_Context->LogicalDevice.destroyCommandPool( m_Context->GraphicsQueueCommandPool );
    m_Context->LogicalDevice.destroyCommandPool( m_Context->ComputeQueueCommandPool );

    m_Context->Instance.destroySurfaceKHR( m_Context->Surface );
    vmaDestroyAllocator( m_Context->Vma );
    m_Context->LogicalDevice.destroy();
    m_Context->Instance.destroy();

    m_Context->ShaderCompiler.Destroy();
}

void VulkanDevice::DestroyDebugUtils() const
{
    if ( m_DebugMessenger == VK_NULL_HANDLE )
    {
        return;
    }

    const auto instance = static_cast<VkInstance>(m_Context->Instance);

    if ( const auto deleteDebugUtils = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" )) )
    {
        deleteDebugUtils( instance, m_DebugMessenger, nullptr );
    }
}

VulkanContext *VulkanDevice::GetContext() const
{
    return m_Context.get();
}

void VulkanDevice::CreateImageFormat() const
{
    const auto surfaceFormats = m_Context->PhysicalDevice.getSurfaceFormatsKHR( m_Context->Surface );
    const auto presentModes = m_Context->PhysicalDevice.getSurfacePresentModesKHR( m_Context->Surface );

    auto presentMode = vk::PresentModeKHR::eImmediate;
    for ( const auto mode : presentModes )
    {
        if ( mode == vk::PresentModeKHR::eImmediate )
        {
            presentMode = mode;
        }
    }

    auto surfaceFormat = vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
    for ( const auto format : surfaceFormats )
    {
        if ( format.format == vk::Format::eB8G8R8A8Unorm /*&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear*/ )
        {
            surfaceFormat = format;
        }
    }

    m_Context->SurfaceImageFormat = ImageFormat::B8G8R8A8Unorm;
    m_Context->ColorSpace = surfaceFormat.colorSpace;
    m_Context->PresentMode = presentMode;
}

uint32_t VulkanDevice::GetFrameCount() const
{
    return m_Context->SwapChainImages.size();
}

ImageFormat VulkanDevice::GetSwapChainImageFormat() const
{
    return m_Context->SurfaceImageFormat;
}

#endif
