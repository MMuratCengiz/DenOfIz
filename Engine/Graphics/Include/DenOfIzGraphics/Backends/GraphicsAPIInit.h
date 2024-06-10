#pragma once

#ifdef BUILD_VK
#include <DenOfIzGraphics/Backends/Vulkan/VulkanLogicalDevice.h>
#endif

#ifdef BUILD_DX
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#endif

#include "Interface/IFence.h"
#include "Interface/ISemaphore.h"
#include "Interface/ILogicalDevice.h"
#include "Interface/IPipeline.h"
#include "Interface/IRenderTarget.h"
#include "Interface/IResource.h"
#include "Interface/IShader.h"

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
	APIPreferenceOSX OSX = APIPreferenceOSX::Metal;
	APIPreferenceLinux Linux = APIPreferenceLinux::Vulkan;
};

namespace DenOfIz
{

class GraphicsAPIInit
{
private:
	APIPreference preference;

public:
	inline GraphicsAPIInit(APIPreference preference)
			:preference(preference)
	{
	}

	inline std::unique_ptr<ILogicalDevice> CreateLogicalDevice(SDL_Window* window)
	{
		std::unique_ptr<ILogicalDevice> logicalDevice = nullptr;

#ifdef BUILD_VK
		if (IsVulkanPreferred())
		{
			logicalDevice = std::make_unique<VulkanLogicalDevice>();
		}
#elif BUILD_DX
		if (IsDX12Preferred())
		{
			logicalDevice = std::make_unique<DX12LogicalDevice>();
		}
#elif BUILD_METAL
		if (IsMetalPreferred())
		{
			// TODO
			return nullptr;
		}
#else
		assertm(false, "No supported API found for this system.")
#endif

		logicalDevice->CreateDevice(window);
		return logicalDevice;
	}

private:
	inline bool IsVulkanPreferred() const
	{
#ifdef _WIN32
		if (preference.Windows == APIPreferenceWindows::Vulkan)
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

#ifdef __linux__
		if ( preference.Linux == APIPreferenceLinux::Vulkan )
		{
			return true;
		}
#endif

		return false;
	}

	inline bool IsDX12Preferred() const
	{
#ifdef _WIN32
		if (preference.Windows == APIPreferenceWindows::DirectX12)
		{
			return true;
		}
#endif

		return false;
	}

	inline bool IsMetalPreferred() const
	{
#ifdef __APPLE__
		if (preference.OSX == APIPreferenceOSX::Metal)
		{
			return true;
		}
#endif

		return false;
	}
};

} // namespace DenOfIz
