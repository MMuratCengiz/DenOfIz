#pragma once

#ifdef BUILD_VK
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#endif

#ifdef BUILD_DX12
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#endif

#ifdef BUILD_METAL
#include <DenOfIzGraphics/Backends/Metal/MetalLogicalDevice.h>
#endif

#include "Interface/ILogicalDevice.h"

namespace DenOfIz
{
    enum class APIPreferenceWindows
    {
        DirectX12,
        Vulkan
    };

    enum class APIPreferenceOSX
    {
        Metal,
        Vulkan
    };

    enum class APIPreferenceLinux
    {
        Vulkan
    };

    struct DZ_API APIPreference
    {
        APIPreferenceWindows Windows = APIPreferenceWindows::DirectX12;
        APIPreferenceOSX     OSX     = APIPreferenceOSX::Metal;
        APIPreferenceLinux   Linux   = APIPreferenceLinux::Vulkan;
    };

    /// <summary>
    /// A class that provides a factory for creating API agnostic structures
    /// Currently creates a logical device and a shader program
    /// </summary>
    class DZ_API GraphicsApi
    {
        APIPreference m_apiPreference;

    public:
        explicit GraphicsApi( const APIPreference &preference );
        ~GraphicsApi( );

        [[nodiscard]] ILogicalDevice *CreateLogicalDevice( ) const;
        void                          LogDeviceCapabilities( PhysicalDevice gpuDesc ) const;
        [[nodiscard]] ILogicalDevice *CreateAndLoadOptimalLogicalDevice( ) const;
        [[nodiscard]] InteropString   ActiveAPI( ) const;

        static void ReportLiveObjects( );

    private:
        bool IsVulkanPreferred( ) const;
        bool IsDX12Preferred( ) const;
        bool IsMetalPreferred( ) const;
    };

} // namespace DenOfIz
