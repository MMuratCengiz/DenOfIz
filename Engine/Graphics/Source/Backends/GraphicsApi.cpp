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

#include <DenOfIzGraphics/Backends/GraphicsApi.h>

using namespace DenOfIz;

GraphicsApi::GraphicsApi( const APIPreference &preference ) : m_apiPreference( preference )
{
}

GraphicsApi::~GraphicsApi( )
{
    ReportLiveObjects( );
}

std::unique_ptr<ILogicalDevice> GraphicsApi::CreateLogicalDevice( ) const
{
    std::unique_ptr<ILogicalDevice> logicalDevice = nullptr;

#ifdef BUILD_VK
    if ( IsVulkanPreferred( ) )
    {
        LOG( INFO ) << "Graphics API: Vulkan.";
        logicalDevice = std::make_unique<VulkanLogicalDevice>( );
    }
#endif
#ifdef BUILD_DX12
    if ( IsDX12Preferred( ) )
    {
        LOG( INFO ) << "Graphics API: DirectX12.";
        logicalDevice = std::make_unique<DX12LogicalDevice>( );
    }
#endif
#ifdef BUILD_METAL
    if ( IsMetalPreferred( ) )
    {
        LOG( INFO ) << "Graphics API: Metal.";
        logicalDevice = std::make_unique<MetalLogicalDevice>( );
    }
#endif
    DZ_ASSERTM( logicalDevice != nullptr, "No supported API found for this system." );
    logicalDevice->CreateDevice( );
    return logicalDevice;
}

std::unique_ptr<ILogicalDevice> GraphicsApi::CreateAndLoadOptimalLogicalDevice( ) const
{
    std::unique_ptr<ILogicalDevice> logicalDevice = CreateLogicalDevice( );

    // Todo something smarter
    for ( const PhysicalDevice &device : logicalDevice->ListPhysicalDevices( ) )
    {
        if ( device.Properties.IsDedicated )
        {
            logicalDevice->LoadPhysicalDevice( device );
            return std::move( logicalDevice );
        }
    }

    logicalDevice->LoadPhysicalDevice( logicalDevice->ListPhysicalDevices( ).front( ) );
    return std::move( logicalDevice );
}

std::unique_ptr<ShaderProgram> GraphicsApi::CreateShaderProgram( const std::vector<ShaderDesc> &shaders ) const
{
    ShaderProgramDesc programDesc{ };
    programDesc.Shaders = shaders;
#if defined( WIN32 )
    if ( m_apiPreference.Windows == APIPreferenceWindows::DirectX12 )
    {
        programDesc.TargetIL = TargetIL::DXIL;
    }
    else
    {
        programDesc.TargetIL = TargetIL::SPIRV;
    }
#elif defined( __APPLE__ )
    if ( m_apiPreference.OSX == APIPreferenceOSX::Metal )
    {
        programDesc.TargetIL = TargetIL::MSL;
    }
    else
    {
        LOG( ERROR ) << "No supported API found for this system.";
    }
    programDesc.TargetIL = TargetIL::MSL;
#elif defined( __linux__ )
    if ( m_apiPreference.Linux == APIPreferenceLinux::Vulkan )
    {
        programDesc.TargetIL = TargetIL::SPIRV;
    }
    else
    {
        LOG( ERROR ) << "No supported API found for this system.";
    }
#endif
    auto *program = new ShaderProgram( programDesc );
    return std::unique_ptr<ShaderProgram>( program );
}

void GraphicsApi::ReportLiveObjects( ) const
{
#ifndef NDEBUG
#if defined( BUILD_DX12 )
    {
        wil::com_ptr<IDXGIDebug1> dxgi_debug;
        if ( SUCCEEDED( DXGIGetDebugInterface1( 0, IID_PPV_ARGS( dxgi_debug.addressof( ) ) ) ) )
        {
            dxgi_debug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS( DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL ) );
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

    LOG( ERROR ) << "No supported API found for this system.";
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
