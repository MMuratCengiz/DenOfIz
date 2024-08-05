#pragma once

#ifdef BUILD_VK
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#endif

#ifdef BUILD_DX12
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#endif

#ifdef BUILD_METAL
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

        std::unique_ptr<ILogicalDevice> CreateLogicalDevice( );
        std::unique_ptr<ILogicalDevice> CreateAndLoadOptimalLogicalDevice( );
        std::unique_ptr<ShaderProgram>  CreateShaderProgram( const std::vector<ShaderDesc> &shaders );

        void ReportLiveObjects( );

    private:
        bool IsVulkanPreferred( );
        bool IsDX12Preferred( );
        bool IsMetalPreferred( );
    };

} // namespace DenOfIz
