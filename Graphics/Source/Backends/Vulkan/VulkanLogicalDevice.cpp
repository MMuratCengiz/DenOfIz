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

#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanCommandList.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanSwapChain.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanTextureResource.h>
#include <algorithm>

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

using namespace DenOfIz;

// ReSharper disable once CppParameterMayBeConst
static VKAPI_ATTR VkBool32 VKAPI_CALL g_DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void * /*pUserData*/ )
{
    switch ( messageSeverity )
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        DLOG( INFO ) << pCallbackData->pMessage;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG( INFO ) << pCallbackData->pMessage;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG( WARNING ) << pCallbackData->pMessage;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG( FATAL ) << pCallbackData->pMessage;
        break;
    }
    return VK_FALSE;
}

// clang-format off
const std::unordered_map<std::string, bool> VulkanLogicalDevice::g_optionalLayers =
{
#ifndef NDEBUG
    { "VK_LAYER_KHRONOS_validation", true }
#endif
};

const std::vector<const char*> VulkanLogicalDevice::g_requiredInstanceExtensions =
{
};

const std::vector<const char*> VulkanLogicalDevice::g_optionalInstanceExtensions =
{
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_GGP)
    VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_VI_NN)
    VK_NN_VI_SURFACE_EXTENSION_NAME,
#endif
#if defined( _DEBUG )
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
};

const std::vector<const char *> VulkanLogicalDevice::g_requiredDeviceExtensions =
{
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    // Maintenance Extensions
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_MAINTENANCE2_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    // Dynamic Rendering, required we do not support render passes currently.
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
    // Used to pass Viewport and Scissor count.
    VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
#if __APPLE_CC__
    "VK_KHR_portability_subset"
#endif
};

const std::vector<const char *> VulkanLogicalDevice::g_optionalDeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // Ray Tracing
    VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
};
// clang-format on

void VulkanLogicalDevice::CreateDevice( )
{
    VK_CHECK_RESULT( volkInitialize( ) );

    m_context = std::make_unique<VulkanContext>( );

    VkApplicationInfo appInfo{ };
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "DenOfIz";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{ };
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    InitInstanceExtensions( );
    std::vector<const char *> layers;
    InitSupportedLayers( layers );

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ };
    if ( m_supportedLayers.contains( "VK_LAYER_KHRONOS_validation" ) )
    {
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = g_DebugCallback;
        createInfo.pNext                     = &debugUtilsCreateInfo;
    }

    std::vector<const char *> enabledExtensions( m_enabledInstanceExtensions.size( ) );
    std::copy( m_enabledInstanceExtensions.begin( ), m_enabledInstanceExtensions.end( ), enabledExtensions.begin( ) );

    createInfo.enabledExtensionCount   = enabledExtensions.size( );
    createInfo.ppEnabledExtensionNames = enabledExtensions.data( );
    createInfo.enabledLayerCount       = layers.size( );
    createInfo.ppEnabledLayerNames     = layers.data( );

    vkCreateInstance( &createInfo, nullptr, &m_context->Instance );
    volkLoadInstance( m_context->Instance );

    if ( m_supportedLayers.contains( "VK_LAYER_KHRONOS_validation" ) )
    {
        DLOG( INFO ) << "Enabling Vk Validation Layers.";
        VK_CHECK_RESULT( vkCreateDebugUtilsMessengerEXT( m_context->Instance, &debugUtilsCreateInfo, nullptr, &m_debugMessenger ) );
    }
}

/// <summary> Iterates through the requested extensions and checks appends them into result if found. </summary>
void CollectExtensions( const std::vector<VkExtensionProperties> &availableExtensions, const std::vector<const char *> &requestedExtensions,
                        std::unordered_set<const char *> &result, const bool failOnMissing )
{
    for ( const char *requiredExtension : requestedExtensions )
    {
        bool found = false;
        for ( const VkExtensionProperties &extension : availableExtensions )
        {
            if ( strcmp( extension.extensionName, requiredExtension ) == 0 )
            {
                result.insert( requiredExtension );
                found = true;
                break;
            }
        }

        if ( !found )
        {
            if ( failOnMissing )
            {
                LOG( ERROR ) << "Missing Required Extension: " << requiredExtension;
            }
            else
            {
                LOG( WARNING ) << "Missing Optional Extension: " << requiredExtension;
            }
        }
    }
}

void VulkanLogicalDevice::InitDeviceExtensions( )
{
    uint32_t                           count = 0;
    std::vector<VkExtensionProperties> availableProperties;
    vkEnumerateDeviceExtensionProperties( m_context->PhysicalDevice, nullptr, &count, nullptr );
    availableProperties.resize( count );
    vkEnumerateDeviceExtensionProperties( m_context->PhysicalDevice, nullptr, &count, availableProperties.data( ) );

    CollectExtensions( availableProperties, g_requiredDeviceExtensions, m_enabledDeviceExtensions, true );
    CollectExtensions( availableProperties, g_optionalDeviceExtensions, m_enabledDeviceExtensions, false );
}

void VulkanLogicalDevice::InitInstanceExtensions( )
{
    uint32_t                           count = 0;
    std::vector<VkExtensionProperties> availableProperties;
    vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
    availableProperties.resize( count );
    vkEnumerateInstanceExtensionProperties( nullptr, &count, availableProperties.data( ) );

    CollectExtensions( availableProperties, g_requiredInstanceExtensions, m_enabledInstanceExtensions, true );
    CollectExtensions( availableProperties, g_optionalInstanceExtensions, m_enabledInstanceExtensions, false );
}

void VulkanLogicalDevice::InitSupportedLayers( std::vector<const char *> &layers )
{
    uint32_t                       count = 0;
    std::vector<VkLayerProperties> layerProperties;
    vkEnumerateInstanceLayerProperties( &count, nullptr );
    layerProperties.resize( count );
    vkEnumerateInstanceLayerProperties( &count, layerProperties.data( ) );

    for ( const VkLayerProperties prp : layerProperties )
    {
        if ( auto layerPair = g_optionalLayers.find( prp.layerName ); layerPair != g_optionalLayers.end( ) )
        {
            m_supportedLayers.insert( prp.layerName );
            layers.emplace_back( layerPair->first.c_str( ) );
            LOG( INFO ) << "Found Enabled Layer: " + layerPair->first;
        }
    }
}

InteropArray<PhysicalDevice> VulkanLogicalDevice::ListPhysicalDevices( )
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices( m_context->Instance, &count, nullptr );
    std::vector<VkPhysicalDevice> devices( count );
    vkEnumeratePhysicalDevices( m_context->Instance, &count, devices.data( ) );
    InteropArray<PhysicalDevice> result( count );
    DZ_ASSERTM( count > 0, "No Vulkan Devices Found." );
    DZ_ASSERTM( count < 4, "Too many devices, consider upgrading library limits." );

    int index = 0;
    for ( const auto &device : devices )
    {
        PhysicalDevice deviceInfo{ };
        CreateDeviceInfo( device, deviceInfo );
        result.SetElement( index, deviceInfo );
    }

    return result;
}

void VulkanLogicalDevice::CreateDeviceInfo( const VkPhysicalDevice &physicalDevice, PhysicalDevice &deviceInfo ) const
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures( physicalDevice, &deviceFeatures );

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );

    uint32_t                             queueFamilyCount;
    std::vector<VkQueueFamilyProperties> localQueueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );
    localQueueFamilies.resize( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, localQueueFamilies.data( ) );

    uint32_t extensionPropertyCount;
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionPropertyCount, nullptr );
    std::vector<VkExtensionProperties> extensions( extensionPropertyCount );
    vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionPropertyCount, extensions.data( ) );

    deviceInfo.Id   = deviceProperties.deviceID;
    deviceInfo.Name = deviceProperties.deviceName;

    // Todo actually read these from somewhere:
    deviceInfo.Properties.IsDedicated = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    for ( VkExtensionProperties extension : extensions )
    {
        if ( strcmp( extension.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME ) == 0 )
        {
            deviceInfo.Capabilities.RayTracing = true;
        }
    }

    VkPhysicalDeviceFeatures2 deviceFeatures2{ };
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2( physicalDevice, &deviceFeatures2 );

    deviceInfo.Capabilities.ComputeShaders  = true;
    deviceInfo.Capabilities.GeometryShaders = deviceFeatures2.features.geometryShader;
    deviceInfo.Capabilities.Tessellation    = deviceFeatures2.features.tessellationShader;
    deviceInfo.Capabilities.HDR             = m_enabledInstanceExtensions.contains( VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME );
    deviceInfo.Capabilities.Tearing         = true;

    deviceInfo.Constants.ConstantBufferAlignment   = deviceProperties.limits.minUniformBufferOffsetAlignment;
    deviceInfo.Constants.BufferTextureAlignment    = deviceProperties.limits.optimalBufferCopyOffsetAlignment;
    deviceInfo.Constants.BufferTextureRowAlignment = deviceProperties.limits.optimalBufferCopyRowPitchAlignment;
}

void VulkanLogicalDevice::LoadPhysicalDevice( const PhysicalDevice &device )
{
    DZ_ASSERTM( m_context->PhysicalDevice == VK_NULL_HANDLE, "A physical device is already selected for this logical device. CreateFromSDLWindow a new Logical Device." );
    m_selectedDeviceInfo          = device;
    m_context->SelectedDeviceInfo = device;

    uint32_t count = 0;
    vkEnumeratePhysicalDevices( m_context->Instance, &count, nullptr );
    std::vector<VkPhysicalDevice> devices( count );
    vkEnumeratePhysicalDevices( m_context->Instance, &count, devices.data( ) );

    for ( const VkPhysicalDevice &physicalDevice : devices )
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );

        if ( deviceProperties.deviceID == device.Id )
        {
            m_context->PhysicalDevice = physicalDevice;
        }
    }

    DZ_ASSERTM( m_context->PhysicalDevice != VK_NULL_HANDLE, "Invalid DeviceID provided." );

    CreateLogicalDevice( );
    InitializeVma( );

    VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo{ };
    graphicsCommandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsCommandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphicsCommandPoolCreateInfo.queueFamilyIndex = m_context->QueueFamilies[ VulkanQueueType::Graphics ].Index;
    vkCreateCommandPool( m_context->LogicalDevice, &graphicsCommandPoolCreateInfo, nullptr, &m_context->GraphicsQueueCommandPool );

    VkCommandPoolCreateInfo transferCommandPoolCreateInfo{ };
    transferCommandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferCommandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    transferCommandPoolCreateInfo.queueFamilyIndex = m_context->QueueFamilies[ VulkanQueueType::Copy ].Index;
    vkCreateCommandPool( m_context->LogicalDevice, &transferCommandPoolCreateInfo, nullptr, &m_context->TransferQueueCommandPool );

    VkCommandPoolCreateInfo computeCommandPoolCreateInfo{ };
    computeCommandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    computeCommandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    computeCommandPoolCreateInfo.queueFamilyIndex = m_context->QueueFamilies[ VulkanQueueType::Compute ].Index;
    vkCreateCommandPool( m_context->LogicalDevice, &computeCommandPoolCreateInfo, nullptr, &m_context->ComputeQueueCommandPool );

    m_context->RayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 properties{ };
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties.pNext = &m_context->RayTracingProperties;
    vkGetPhysicalDeviceProperties2( m_context->PhysicalDevice, &properties );
}

void VulkanLogicalDevice::SetupQueueFamilies( ) const
{
    auto exists = [ & ]( const VulkanQueueType bit ) -> bool { return m_context->QueueFamilies.contains( bit ); };

    uint32_t                             count = 0;
    std::vector<VkQueueFamilyProperties> properties;
    vkGetPhysicalDeviceQueueFamilyProperties( m_context->PhysicalDevice, &count, nullptr );
    properties.resize( count );
    vkGetPhysicalDeviceQueueFamilyProperties( m_context->PhysicalDevice, &count, properties.data( ) );

    uint32_t index = 0;
    for ( const VkQueueFamilyProperties &property : properties )
    {
        const bool hasGraphics = ( property.queueFlags & VK_QUEUE_GRAPHICS_BIT ) == VK_QUEUE_GRAPHICS_BIT;
        const bool hasTransfer = ( property.queueFlags & VK_QUEUE_TRANSFER_BIT ) == VK_QUEUE_TRANSFER_BIT;
        const bool hasCompute  = ( property.queueFlags & VK_QUEUE_COMPUTE_BIT ) == VK_QUEUE_COMPUTE_BIT;

        if ( hasGraphics && !exists( VulkanQueueType::Graphics ) )
        {
            m_context->QueueFamilies[ VulkanQueueType::Graphics ] = QueueFamily{ index, { static_cast<VkQueueFlags>( property.queueFlags ) } };
        }
        else if ( hasTransfer && !exists( VulkanQueueType::Copy ) )
        {
            // Try to fetch a unique transfer queue
            m_context->QueueFamilies[ VulkanQueueType::Copy ] = QueueFamily{ index, { static_cast<VkQueueFlags>( property.queueFlags ) } };
        }
        else if ( hasCompute && !exists( VulkanQueueType::Compute ) )
        {
            m_context->QueueFamilies[ VulkanQueueType::Compute ] = QueueFamily{ index, { static_cast<VkQueueFlags>( property.queueFlags ) } };
        }
        ++index;
    }

    if ( !exists( VulkanQueueType::Copy ) )
    {
        m_context->QueueFamilies[ VulkanQueueType::Copy ] = m_context->QueueFamilies[ VulkanQueueType::Graphics ];
    }
}

void VulkanLogicalDevice::CreateLogicalDevice( )
{
    SetupQueueFamilies( );

    const std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = CreateUniqueDeviceQueueCreateInfos( );

    VkPhysicalDeviceFeatures features{ };
    features.samplerAnisotropy = true;
    features.sampleRateShading = true;
    if ( m_context->SelectedDeviceInfo.Capabilities.Tessellation )
    {
        features.tessellationShader = true;
    }
    if ( m_context->SelectedDeviceInfo.Capabilities.GeometryShaders )
    {
        features.geometryShader = true;
    }

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{ };
    rayQueryFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    rayQueryFeatures.rayQuery = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures{ };
    rayTracingFeatures.sType              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingFeatures.rayTracingPipeline = VK_TRUE;
    rayTracingFeatures.pNext              = &rayQueryFeatures;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeature{ };
    accelerationStructureFeature.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeature.accelerationStructure = VK_TRUE;
    accelerationStructureFeature.pNext                 = &rayTracingFeatures;

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeature{ };
    bufferDeviceAddressFeature.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    bufferDeviceAddressFeature.bufferDeviceAddress = VK_TRUE;
    if ( m_context->SelectedDeviceInfo.Capabilities.RayTracing )
    {
        bufferDeviceAddressFeature.pNext = &accelerationStructureFeature;
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeature{ };
    extendedDynamicStateFeature.sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    extendedDynamicStateFeature.extendedDynamicState = VK_TRUE;
    extendedDynamicStateFeature.pNext                = &bufferDeviceAddressFeature;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{ };
    dynamicRenderingFeature.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeature.dynamicRendering = VK_TRUE;
    dynamicRenderingFeature.pNext            = &extendedDynamicStateFeature;

    InitDeviceExtensions( );
    std::vector<const char *> enabledExtensions( m_enabledDeviceExtensions.size( ) );
    std::copy( m_enabledDeviceExtensions.begin( ), m_enabledDeviceExtensions.end( ), enabledExtensions.begin( ) );

    VkDeviceCreateInfo createInfo{ };
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>( deviceQueueCreateInfos.size( ) );
    createInfo.pQueueCreateInfos       = deviceQueueCreateInfos.data( );
    createInfo.enabledExtensionCount   = enabledExtensions.size( );
    createInfo.ppEnabledExtensionNames = enabledExtensions.data( );
    createInfo.enabledLayerCount       = 0;
    createInfo.ppEnabledLayerNames     = nullptr;
    createInfo.pEnabledFeatures        = &features;
    createInfo.pNext                   = &dynamicRenderingFeature;

    vkCreateDevice( m_context->PhysicalDevice, &createInfo, nullptr, &m_context->LogicalDevice );
    volkLoadDevice( m_context->LogicalDevice );

    m_context->Queues[ VulkanQueueType::Graphics ]     = VkQueue{ };
    m_context->Queues[ VulkanQueueType::Compute ]      = VkQueue{ };
    m_context->Queues[ VulkanQueueType::Presentation ] = VkQueue{ };
    m_context->Queues[ VulkanQueueType::Copy ]         = VkQueue{ };

    vkGetDeviceQueue( m_context->LogicalDevice, m_context->QueueFamilies[ VulkanQueueType::Graphics ].Index, 0, &m_context->Queues[ VulkanQueueType::Graphics ] );
    vkGetDeviceQueue( m_context->LogicalDevice, m_context->QueueFamilies[ VulkanQueueType::Compute ].Index, 0, &m_context->Queues[ VulkanQueueType::Compute ] );
    vkGetDeviceQueue( m_context->LogicalDevice, m_context->QueueFamilies[ VulkanQueueType::Presentation ].Index, 0, &m_context->Queues[ VulkanQueueType::Presentation ] );
    vkGetDeviceQueue( m_context->LogicalDevice, m_context->QueueFamilies[ VulkanQueueType::Copy ].Index, 0, &m_context->Queues[ VulkanQueueType::Copy ] );
    m_context->SelectedDeviceInfo.Capabilities.DedicatedCopyQueue =
        m_context->QueueFamilies.at( VulkanQueueType::Copy ).Index != m_context->QueueFamilies.at( VulkanQueueType::Graphics ).Index;
    m_context->DescriptorPoolManager = std::make_unique<VulkanDescriptorPoolManager>( m_context->LogicalDevice );
}

void VulkanLogicalDevice::InitializeVma( ) const
{
    VmaVulkanFunctions vmaVkFunctions{ };
    vmaVkFunctions.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
    vmaVkFunctions.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
    vmaVkFunctions.vkAllocateMemory                    = vkAllocateMemory;
    vmaVkFunctions.vkBindBufferMemory                  = vkBindBufferMemory;
    vmaVkFunctions.vkBindImageMemory                   = vkBindImageMemory;
    vmaVkFunctions.vkCreateBuffer                      = vkCreateBuffer;
    vmaVkFunctions.vkCreateImage                       = vkCreateImage;
    vmaVkFunctions.vkDestroyBuffer                     = vkDestroyBuffer;
    vmaVkFunctions.vkDestroyImage                      = vkDestroyImage;
    vmaVkFunctions.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vmaVkFunctions.vkFreeMemory                        = vkFreeMemory;
    vmaVkFunctions.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vmaVkFunctions.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vmaVkFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vmaVkFunctions.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vmaVkFunctions.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vmaVkFunctions.vkMapMemory                         = vkMapMemory;
    vmaVkFunctions.vkUnmapMemory                       = vkUnmapMemory;
    vmaVkFunctions.vkCmdCopyBuffer                     = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo allocatorInfo = { };
    allocatorInfo.vulkanApiVersion       = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice         = m_context->PhysicalDevice;
    allocatorInfo.device                 = m_context->LogicalDevice;
    allocatorInfo.instance               = m_context->Instance;
    allocatorInfo.pVulkanFunctions       = &vmaVkFunctions;
    allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator( &allocatorInfo, &m_context->Vma );
}

std::vector<VkDeviceQueueCreateInfo> VulkanLogicalDevice::CreateUniqueDeviceQueueCreateInfos( ) const
{
    std::unordered_map<uint32_t, bool>   uniqueIndexes;
    std::vector<VkDeviceQueueCreateInfo> result;

    for ( std::pair<VulkanQueueType, QueueFamily> key : m_context->QueueFamilies )
    {
        if ( !uniqueIndexes.contains( key.second.Index ) )
        {
            float priority = key.first == VulkanQueueType::Graphics || key.first == VulkanQueueType::Presentation ? 1.0f : 0.9f;

            VkDeviceQueueCreateInfo &queueCreateInfo = result.emplace_back( VkDeviceQueueCreateInfo{ } );
            queueCreateInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex         = key.second.Index;
            queueCreateInfo.queueCount               = 1;
            queueCreateInfo.pQueuePriorities         = &priority;
            uniqueIndexes[ key.second.Index ]        = true;
        }
    }

    return result;
}

void VulkanLogicalDevice::WaitIdle( )
{
    vkDeviceWaitIdle( m_context->LogicalDevice );
}

VulkanLogicalDevice::~VulkanLogicalDevice( )
{
    DestroyDebugUtils( );

    if ( m_context == nullptr )
    {
        return;
    }
    m_context->DescriptorPoolManager.reset( );

    vkDestroyCommandPool( m_context->LogicalDevice, m_context->TransferQueueCommandPool, nullptr );
    vkDestroyCommandPool( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, nullptr );
    vkDestroyCommandPool( m_context->LogicalDevice, m_context->ComputeQueueCommandPool, nullptr );

    vmaDestroyAllocator( m_context->Vma );
    vkDestroyDevice( m_context->LogicalDevice, nullptr );
    vkDestroyInstance( m_context->Instance, nullptr );

    volkFinalize( );
}

void VulkanLogicalDevice::DestroyDebugUtils( ) const
{
    DZ_RETURN_IF( m_debugMessenger == nullptr );

    if ( const auto deleteDebugUtils = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr( m_context->Instance, "vkDestroyDebugUtilsMessengerEXT" ) ) )
    {
        deleteDebugUtils( m_context->Instance, m_debugMessenger, nullptr );
    }
}

VulkanContext *VulkanLogicalDevice::GetContext( ) const
{
    return m_context.get( );
}

bool VulkanLogicalDevice::ValidateLayer( const std::string &layer ) const
{
    for ( const VkLayerProperties &available : m_availableLayers )
    {
        if ( layer == available.layerName )
        {
            return true;
        }
    }

    return false;
}

bool VulkanLogicalDevice::IsDeviceLost( )
{
    return m_context->IsDeviceLost;
}

ICommandListPool *VulkanLogicalDevice::CreateCommandListPool( const CommandListPoolDesc &createInfo )
{
    return new VulkanCommandPool( m_context.get( ), createInfo );
}

IPipeline *VulkanLogicalDevice::CreatePipeline( const PipelineDesc &createInfo )
{
    return new VulkanPipeline( m_context.get( ), createInfo );
}

ISwapChain *VulkanLogicalDevice::CreateSwapChain( const SwapChainDesc &createInfo )
{
    return new VulkanSwapChain( m_context.get( ), createInfo );
}

IRootSignature *VulkanLogicalDevice::CreateRootSignature( const RootSignatureDesc &createInfo )
{
    return new VulkanRootSignature( m_context.get( ), createInfo );
}

IInputLayout *VulkanLogicalDevice::CreateInputLayout( const InputLayoutDesc &createInfo )
{
    return new VulkanInputLayout( createInfo );
}

IResourceBindGroup *VulkanLogicalDevice::CreateResourceBindGroup( const ResourceBindGroupDesc &createInfo )
{
    return new VulkanResourceBindGroup( m_context.get( ), createInfo );
}

IBufferResource *VulkanLogicalDevice::CreateBufferResource( const BufferDesc &createInfo )
{
    return new VulkanBufferResource( m_context.get( ), createInfo );
}

ITextureResource *VulkanLogicalDevice::CreateTextureResource( const TextureDesc &createInfo )
{
    return new VulkanTextureResource( m_context.get( ), createInfo );
}

IFence *VulkanLogicalDevice::CreateFence( )
{
    return new VulkanFence( m_context.get( ) );
}

ISemaphore *VulkanLogicalDevice::CreateSemaphore( )
{
    return new VulkanSemaphore( m_context.get( ) );
}

ISampler *VulkanLogicalDevice::CreateSampler( const SamplerDesc &createInfo )
{
    return new VulkanSampler( m_context.get( ), createInfo );
}

ITopLevelAS *VulkanLogicalDevice::CreateTopLevelAS( const TopLevelASDesc &desc )
{
    return new VulkanTopLevelAS( m_context.get( ), desc );
}

IBottomLevelAS *VulkanLogicalDevice::CreateBottomLevelAS( const BottomLevelASDesc &desc )
{
    return new VulkanBottomLevelAS( m_context.get( ), desc );
}

IShaderBindingTable *VulkanLogicalDevice::CreateShaderBindingTable( const ShaderBindingTableDesc &desc )
{
    return new VulkanShaderBindingTable( m_context.get( ), desc );
}

IShaderRecordLayout *VulkanLogicalDevice::CreateShaderRecordLayout( const ShaderRecordLayoutDesc &createDesc )
{
    // TODO
    return nullptr;
}

IShaderRecordData *VulkanLogicalDevice::CreateShaderRecordData( const ShaderRecordDataDesc &createDesc )
{
    // TODO
    return nullptr;
}
