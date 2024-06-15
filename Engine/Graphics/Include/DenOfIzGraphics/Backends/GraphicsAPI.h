#pragma once

#ifdef BUILD_VK
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#endif

#ifdef BUILD_DX
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#endif

#include "Common/GfxGlobal.h"
#include "DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h"
#include "Interface/IFence.h"
#include "Interface/ILogicalDevice.h"
#include "Interface/IPipeline.h"
#include "Interface/IRenderTarget.h"
#include "Interface/IResource.h"
#include "Interface/ISemaphore.h"
#include "Interface/IShader.h"

namespace DenOfIz
{

    class GraphicsAPI
    {
    private:
        APIPreference preference;

    public:
        static void SetAPIPreference(APIPreference preference) { GfxGlobal::GetInstance()->SetAPIPreference(preference); }

        static std::unique_ptr<ILogicalDevice> CreateLogicalDevice(GraphicsWindowHandle *window)
        {
            std::unique_ptr<ILogicalDevice> logicalDevice = nullptr;

#ifdef BUILD_VK
            if ( IsVulkanPreferred() )
            {
                logicalDevice = std::make_unique<VulkanLogicalDevice>();
            }
#endif
#ifdef BUILD_DX12
            if ( IsDX12Preferred() )
            {
                logicalDevice = std::make_unique<DX12LogicalDevice>();
            }
#endif
#ifdef BUILD_METAL
            if ( IsMetalPreferred() )
            {
                // TODO
                return nullptr;
            }
#endif
            assertm(logicalDevice != nullptr, "No supported API found for this system.");
            logicalDevice->CreateDevice(window);
            return logicalDevice;
        }

    private:
        static bool IsVulkanPreferred()
        {
#ifdef _WIN32
            if ( GfxGlobal::GetInstance()->GetAPIPreference().Windows == APIPreferenceWindows::Vulkan )
            {
                return true;
            }
#endif

#ifdef __APPLE__
            if ( GfxGlobal::GetInstance()->GetAPIPreference().OSX == APIPreferenceOSX::Vulkan )
            {
                return true;
            }
#endif

#ifdef __linux__
            if ( GfxGlobal::GetInstance()->GetAPIPreference().Linux == APIPreferenceLinux::Vulkan )
            {
                return true;
            }
#endif

            return false;
        }

        static bool IsDX12Preferred()
        {
#ifdef _WIN32
            if ( GfxGlobal::GetInstance()->GetAPIPreference().Windows == APIPreferenceWindows::DirectX12 )
            {
                return true;
            }
#endif

            return false;
        }

        static bool IsMetalPreferred()
        {
#ifdef __APPLE__
            if ( GfxGlobal::GetInstance()->GetAPIPreference().OSX == APIPreferenceOSX::Metal )
            {
                return true;
            }
#endif

            return false;
        }
    };

} // namespace DenOfIz
