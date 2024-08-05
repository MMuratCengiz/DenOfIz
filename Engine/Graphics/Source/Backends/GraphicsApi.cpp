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

std::unique_ptr<ILogicalDevice> GraphicsApi::CreateLogicalDevice( )
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
    if ( IsMetalPreferred( preference ) )
    {
        LOG( INFO ) << "Graphics API: Metal.";
        // TODO
        return nullptr;
    }
#endif
    DZ_ASSERTM( logicalDevice != nullptr, "No supported API found for this system." );
    logicalDevice->CreateDevice( );
    return logicalDevice;
}

std::unique_ptr<ILogicalDevice> GraphicsApi::CreateAndLoadOptimalLogicalDevice( )
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

std::unique_ptr<ShaderProgram> GraphicsApi::CreateShaderProgram( const std::vector<ShaderDesc> &shaders )
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
    programDesc.TargetIL = TargetIL::MSL;
#elif defined( __linux__ )
    programDesc.TargetIL = TargetIL::SPIRV;
#endif
    ShaderProgram *program = new ShaderProgram( programDesc );
    return std::unique_ptr<ShaderProgram>( program );
}

void GraphicsApi::ReportLiveObjects( )
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

bool GraphicsApi::IsVulkanPreferred( )
{
#ifdef _WIN32
    if ( m_apiPreference.Windows == APIPreferenceWindows::Vulkan )
    {
        return true;
    }
#endif

#ifdef __APPLE__
    if ( m_apiPreference.OSX == APIPreferenceOSX::Vulkan )
    {
        return true;
    }
#endif

#ifdef __linux__
    if ( m_apiPreference.Linux == APIPreferenceLinux::Vulkan )
    {
        return true;
    }
#endif

    return false;
}

bool GraphicsApi::IsDX12Preferred( )
{
#ifdef _WIN32
    if ( m_apiPreference.Windows == APIPreferenceWindows::DirectX12 )
    {
        return true;
    }
#endif

    return false;
}

bool GraphicsApi::IsMetalPreferred( )
{
#ifdef __APPLE__
    if ( m_apiPreference.OSX == APIPreferenceOSX::Metal )
    {
        return true;
    }
#endif

    return false;
}
