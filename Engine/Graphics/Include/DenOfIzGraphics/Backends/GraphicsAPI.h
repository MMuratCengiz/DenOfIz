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
#include "Interface/ISemaphore.h"
#include "Interface/IShader.h"

namespace DenOfIz
{

    class GraphicsAPI
    {
    public:
        static void SetAPIPreference(APIPreference preference)
        {
            GfxGlobal::GetInstance()->SetAPIPreference(preference);
        }

        static std::unique_ptr<ILogicalDevice> CreateLogicalDevice(GraphicsWindowHandle *window)
        {
            std::unique_ptr<ILogicalDevice> logicalDevice = nullptr;

#ifdef BUILD_VK
            if ( IsVulkanPreferred() )
            {
                LOG(INFO) << "Graphics API: Vulkan.";
                logicalDevice = std::make_unique<VulkanLogicalDevice>();
            }
#endif
#ifdef BUILD_DX12
            if ( IsDX12Preferred() )
            {
                LOG(INFO) << "Graphics API: DirectX12.";
                logicalDevice = std::make_unique<DX12LogicalDevice>();
            }
#endif
#ifdef BUILD_METAL
            if ( IsMetalPreferred() )
            {
                LOG(INFO) << "Graphics API: Metal.";
                // TODO
                return nullptr;
            }
#endif
            DZ_ASSERTM(logicalDevice != nullptr, "No supported API found for this system.");
            logicalDevice->CreateDevice(window);
            return logicalDevice;
        }

        static void ReportLiveObjects()
        {
#ifndef NDEBUG
#if defined(BUILD_DX12)
            {
                wil::com_ptr<IDXGIDebug1> dxgi_debug;
                if ( SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgi_debug.addressof()))) )
                {
                    dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
                }
            }
#endif
#endif
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
