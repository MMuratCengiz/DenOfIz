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

#include "DenOfIzGraphics/Backends/GraphicsApi.h"

#ifdef BUILD_VK
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanLogicalDevice.h"
#endif

#ifdef BUILD_DX12
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12LogicalDevice.h"
#endif

#ifdef BUILD_METAL
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalLogicalDevice.h"
#endif


using namespace DenOfIz;

GraphicsApi::GraphicsApi( const APIPreference &preference ) : m_apiPreference( preference )
{
    std::atexit( ReportLiveObjects );
}

GraphicsApi::~GraphicsApi( ) = default;

ILogicalDevice *GraphicsApi::CreateLogicalDevice( ) const
{
    ILogicalDevice *logicalDevice = nullptr;

#ifdef BUILD_VK
    if ( IsVulkanPreferred( ) )
    {
        LOG( INFO ) << "Graphics API: Vulkan.";
        logicalDevice = new VulkanLogicalDevice( );
    }
#endif
#ifdef BUILD_DX12
    if ( IsDX12Preferred( ) )
    {
        LOG( INFO ) << "Graphics API: DirectX12.";
        logicalDevice = new DX12LogicalDevice( );
    }
#endif
#ifdef BUILD_METAL
    if ( IsMetalPreferred( ) )
    {
        LOG( INFO ) << "Graphics API: Metal.";
        logicalDevice = new MetalLogicalDevice( );
    }
#endif
    if ( logicalDevice == nullptr )
    {
        throw std::runtime_error( "No supported API found for this system." );
    }
    logicalDevice->CreateDevice( );
    return logicalDevice;
}

void GraphicsApi::LogDeviceCapabilities( const PhysicalDevice gpuDesc ) const
{
    LOG( INFO ) << "Loaded device: " << gpuDesc.Name.Get( );
    LOG( INFO ) << "Device Capabilities:";
    LOG( INFO ) << "Dedicated GPU " << ( gpuDesc.Properties.IsDedicated ? "Yes" : "No" );
    LOG( INFO ) << "Available Memory " << gpuDesc.Properties.MemoryAvailableInMb << "MB";
    LOG( INFO ) << "Dedicated Transfer Queue: " << ( gpuDesc.Capabilities.DedicatedCopyQueue ? "Yes" : "No" );
    LOG( INFO ) << "Compute Shaders: " << ( gpuDesc.Capabilities.ComputeShaders ? "Yes" : "No" );
    LOG( INFO ) << "Ray Tracing: " << ( gpuDesc.Capabilities.RayTracing ? "Yes" : "No" );
    LOG( INFO ) << "Tearing: " << ( gpuDesc.Capabilities.Tearing ? "Yes" : "No" );
}

ILogicalDevice *GraphicsApi::CreateAndLoadOptimalLogicalDevice( ) const
{
    ILogicalDevice *logicalDevice = CreateLogicalDevice( );

    const InteropArray<PhysicalDevice> &devices = logicalDevice->ListPhysicalDevices( );
    for ( int i = 0; i < devices.NumElements( ); ++i )
    {
        if ( const PhysicalDevice &device = devices.GetElement( i ); device.Properties.IsDedicated )
        {
            logicalDevice->LoadPhysicalDevice( device );
            LogDeviceCapabilities( device );
            return logicalDevice;
        }
    }

    const auto gpuDesc = devices.GetElement( 0 );
    logicalDevice->LoadPhysicalDevice( gpuDesc );
    LogDeviceCapabilities( gpuDesc );

    return logicalDevice;
}

InteropString GraphicsApi::ActiveAPI( ) const
{
    if ( IsVulkanPreferred( ) )
    {
        return "Vulkan";
    }
    if ( IsDX12Preferred( ) )
    {
        return "DirectX12";
    }
    if ( IsMetalPreferred( ) )
    {
        return "Metal";
    }
    return "Undefined";
}

void GraphicsApi::ReportLiveObjects( )
{
#ifndef NDEBUG
#if defined( BUILD_DX12 )
    {
        if ( wil::com_ptr<IDXGIDebug1> dxgi_debug; SUCCEEDED( DXGIGetDebugInterface1( 0, IID_PPV_ARGS( dxgi_debug.addressof( ) ) ) ) )
        {
            const HRESULT hr = dxgi_debug->ReportLiveObjects( DXGI_DEBUG_ALL, static_cast<DXGI_DEBUG_RLO_FLAGS>( DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL ) );
            DX_CHECK_RESULT( hr );
        }
    }
#endif
#endif
}

bool GraphicsApi::IsVulkanPreferred( ) const
{
    if ( m_apiPreference.Windows == APIPreferenceWindows::Vulkan )
    {
#ifdef _WIN32
        return true;
#endif
    }

    if ( m_apiPreference.OSX == APIPreferenceOSX::Vulkan )
    {
#ifdef __APPLE__
        return true;
#endif
    }

    if ( m_apiPreference.Linux == APIPreferenceLinux::Vulkan )
    {
#ifdef __linux__
        return true;
#endif
    }

    return false;
}

bool GraphicsApi::IsDX12Preferred( ) const
{
    if ( m_apiPreference.Windows == APIPreferenceWindows::DirectX12 )
    {
#ifdef _WIN32
        return true;
#endif
    }

    return false;
}

bool GraphicsApi::IsMetalPreferred( ) const
{
    if ( m_apiPreference.OSX == APIPreferenceOSX::Metal )
    {
#ifdef __APPLE__
        return true;
#endif
    }

    return false;
}
