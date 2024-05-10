#pragma once

#include <DenOfIzCore/Common.h>

#ifdef BUILD_VK
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRenderPass.h>
#endif

#ifdef BUILD_DX
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#endif

#include "ILock.h"
#include "ILogicalDevice.h"
#include "IPipeline.h"
#include "IRenderTarget.h"
#include "IResource.h"
#include "IShader.h"

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
    APIPreferenceWindows Windows;
    APIPreferenceOSX OSX;
    APIPreferenceLinux Linux;
};

namespace DenOfIz
{

    class RHI
    {
    private:
        APIPreference preference;

    public:
        inline RHI(APIPreference preference) : preference(preference) {}

        inline std::unique_ptr<ILogicalDevice> CreateLogicalDevice()
        {
#ifdef BUILD_VK
            if ( IsVulkanPreffered() )
            {
                return make_unique<VulkanLogicalDevice>();
			}
#elif BUILD_DX
            if ( IsDX12Preffered() )
            {
                return make_unique<DX12LogicalDevice>();
			}
#elif BUILD_METAL
            if ( IsMetalPreffered() )
            {
				// TODO
                return nullptr;
			}
#else
            assert(true, "No supported API found for this system.")
#endif
        }

    private:
        inline bool IsVulkanPreffered() const
        {
#ifdef _WIN32
            if ( preference.Windows == APIPreferenceWindows::Vulkan )
            {
				return true;
            }
#endif
			
#ifdef __APPLE__
            if ( preference.OSX == APIPreferenceOSX::Vulkan )
            {
				return true;
            }
#endif
			
#ifdef __linux__ || __unix__
            if ( preference.Linux == APIPreferenceLinux::Vulkan )
            {
				return true;
            }
#endif

            return false;
        }

        inline bool IsDX12Preffered() const
        {
#ifdef _WIN32
            if ( preference.Windows == APIPreferenceWindows::DirectX )
            {
				return true;
            }
#endif

            return false;
        }

        inline bool IsMetalPreffered() const
        {
#ifdef __APPLE__
            if ( preference.OSX == APIPreferenceOSX::Metal )
            {
				return true;
            }
#endif

            return false;
        }
    };

} // namespace DenOfIz
