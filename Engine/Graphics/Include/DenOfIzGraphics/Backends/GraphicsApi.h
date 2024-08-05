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

#include "Interface/IFence.h"
#include "Interface/ILogicalDevice.h"
#include "Interface/IPipeline.h"
#include "Interface/IRenderTarget.h"
#include "Interface/ISemaphore.h"
#include "Interface/IShader.h"

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

    struct APIPreference
    {
        APIPreferenceWindows Windows = APIPreferenceWindows::DirectX12;
        APIPreferenceOSX     OSX     = APIPreferenceOSX::Metal;
        APIPreferenceLinux   Linux   = APIPreferenceLinux::Vulkan;
    };
    /// <summary>
    /// A class that provides a factory for creating API agnostic structures
    /// Currently creates a logical device and a shader program
    /// </summary>
    class GraphicsApi
    {
    private:
        APIPreference m_apiPreference;

    public:
        explicit GraphicsApi( const APIPreference &preference );
        ~GraphicsApi( );

        std::unique_ptr<ILogicalDevice> CreateLogicalDevice( ) const;
        std::unique_ptr<ILogicalDevice> CreateAndLoadOptimalLogicalDevice( ) const;
        std::unique_ptr<ShaderProgram>  CreateShaderProgram( const std::vector<ShaderDesc> &shaders ) const;

        void ReportLiveObjects( ) const;

    private:
        bool IsVulkanPreferred( ) const;
        bool IsDX12Preferred( ) const;
        bool IsMetalPreferred( ) const;
    };

} // namespace DenOfIz
