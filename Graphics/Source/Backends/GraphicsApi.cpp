/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"

#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifdef BUILD_VK
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanLogicalDevice.h"
#endif

#ifdef BUILD_DX12
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12LogicalDevice.h"
#ifdef _WIN32
#include <dxgidebug.h>
#endif
#endif

#ifdef BUILD_METAL
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalLogicalDevice.h"
#endif

using namespace DenOfIz;

namespace DenOfIz
{
    class GraphicsApi
    {
        DenOfIz_APIPreference m_apiPreference;

    public:
        explicit GraphicsApi( const DenOfIz_APIPreference &preference );
        ~GraphicsApi( );

        [[nodiscard]] ILogicalDevice    *CreateLogicalDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) const;
        void                             LogDeviceCapabilities( DenOfIz_PhysicalDevice gpuDesc ) const;
        [[nodiscard]] ILogicalDevice    *CreateAndLoadOptimalLogicalDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) const;
        [[nodiscard]] DenOfIz_StringView ActiveAPI( ) const;

        static void ReportLiveObjects( );
    };
} // namespace DenOfIz

bool DenOfIz_GraphicsApi_IsVulkanPreferred( const DenOfIz_APIPreference *apiPreference )
{
    if ( apiPreference->Windows == DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN )
    {
#ifdef _WIN32
        return true;
#endif
    }

    if ( apiPreference->OSX == DENOFIZ_API_PREFERENCE_OSX_VULKAN )
    {
#ifdef __APPLE__
        return true;
#endif
    }

    if ( apiPreference->Linux == DENOFIZ_API_PREFERENCE_LINUX_VULKAN )
    {
#ifdef __linux__
        return true;
#endif
    }

    return false;
}

bool DenOfIz_GraphicsApi_IsDX12Preferred( const DenOfIz_APIPreference *apiPreference )
{
    if ( apiPreference->Windows == DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12 )
    {
#ifdef _WIN32
        return true;
#endif
    }

    return false;
}

bool DenOfIz_GraphicsApi_IsMetalPreferred( const DenOfIz_APIPreference *apiPreference )
{
    if ( apiPreference->OSX == DENOFIZ_API_PREFERENCE_OSX_METAL )
    {
#ifdef __APPLE__
        return true;
#endif
    }

    return false;
}

GraphicsApi::GraphicsApi( const DenOfIz_APIPreference &preference ) : m_apiPreference( preference )
{
    std::atexit( ReportLiveObjects );
}

GraphicsApi::~GraphicsApi( ) = default;

ILogicalDevice *GraphicsApi::CreateLogicalDevice( const DenOfIz_LogicalDeviceDesc &desc ) const
{
    ILogicalDevice *logicalDevice = nullptr;

#ifdef BUILD_VK
    if ( DenOfIz_GraphicsApi_IsVulkanPreferred( &m_apiPreference ) )
    {
        spdlog::info( "Graphics API: Vulkan." );
        logicalDevice = new VulkanLogicalDevice( );
    }
#endif
#ifdef BUILD_DX12
    if ( DenOfIz_GraphicsApi_IsDX12Preferred( &m_apiPreference ) )
    {
        spdlog::info( "Graphics API: DirectX12." );
        logicalDevice = new DX12LogicalDevice( );
    }
#endif
#ifdef BUILD_METAL
    if ( DenOfIz_GraphicsApi_IsMetalPreferred( &m_apiPreference ) )
    {
        spdlog::info( "Graphics API: Metal." );
        logicalDevice = new MetalLogicalDevice( );
    }
#endif
    if ( logicalDevice == nullptr )
    {
        spdlog::critical( "No graphics API available for this platform in this build" );
        return nullptr;
    }
    logicalDevice->CreateDevice( desc );
    return logicalDevice;
}

void GraphicsApi::LogDeviceCapabilities( const DenOfIz_PhysicalDevice gpuDesc ) const
{
    std::string deviceName( gpuDesc.Name.Chars, gpuDesc.Name.NumChars );
    spdlog::info( "Loaded device: {}", deviceName.c_str( ) );
    spdlog::info( "Device Capabilities:" );
    spdlog::info( "Dedicated GPU {}", ( gpuDesc.Properties.IsDedicated ? "Yes" : "No" ) );
    spdlog::info( "Available Memory {} MB", gpuDesc.Properties.MemoryAvailableInMb );
    spdlog::info( "Dedicated Transfer Queue: {}", ( gpuDesc.Capabilities.DedicatedCopyQueue ? "Yes" : "No" ) );
    spdlog::info( "Compute Shaders: {}", ( gpuDesc.Capabilities.ComputeShaders ? "Yes" : "No" ) );
    spdlog::info( "Ray Tracing: {}", ( gpuDesc.Capabilities.RayTracing ? "Yes" : "No" ) );
    spdlog::info( "Tearing: {}", ( gpuDesc.Capabilities.Tearing ? "Yes" : "No" ) );
}

ILogicalDevice *GraphicsApi::CreateAndLoadOptimalLogicalDevice( const DenOfIz_LogicalDeviceDesc &desc ) const
{
    ILogicalDevice *logicalDevice = CreateLogicalDevice( desc );
    if ( logicalDevice == nullptr )
    {
        return nullptr;
    }
    const DenOfIz_PhysicalDeviceArray devices = logicalDevice->ListPhysicalDevices( );
    if ( devices.NumElements == 0 )
    {
        const DenOfIz_StringView api = ActiveAPI( );
        spdlog::critical( "No GPU supporting {} found", std::string( api.Chars, api.NumChars ) );
        delete logicalDevice;
        return nullptr;
    }
    int bestDeviceIndex = 0;
    int bestScore       = -1;
    for ( uint32_t i = 0; i < devices.NumElements; ++i )
    {
        const DenOfIz_PhysicalDevice &device = devices.Elements[ i ];
        int                           score  = 0;
        if ( device.Properties.IsDedicated )
        {
            score += 10000;
        }
        score += static_cast<int>( device.Properties.MemoryAvailableInMb );
        if ( device.Capabilities.RayTracing )
        {
            score += 1000;
        }

        if ( device.Capabilities.MeshShaders )
        {
            score += 500;
        }

        if ( device.Capabilities.DedicatedCopyQueue )
        {
            score += 100;
        }

        if ( device.Capabilities.VariableRateShading )
        {
            score += 50;
        }

        if ( score > bestScore )
        {
            bestScore       = score;
            bestDeviceIndex = static_cast<int>( i );
        }
    }

    const DenOfIz_PhysicalDevice &selectedDevice = devices.Elements[ bestDeviceIndex ];

    logicalDevice->LoadPhysicalDevice( selectedDevice );
    if ( logicalDevice->IsDeviceLost( ) )
    {
        const DenOfIz_StringView api = ActiveAPI( );
        spdlog::critical( "The selected GPU could not be initialized for {}", std::string( api.Chars, api.NumChars ) );
        delete logicalDevice;
        return nullptr;
    }
    LogDeviceCapabilities( selectedDevice );

    return logicalDevice;
}

DenOfIz_StringView GraphicsApi::ActiveAPI( ) const
{
#ifdef BUILD_VK
    if ( DenOfIz_GraphicsApi_IsVulkanPreferred( &m_apiPreference ) )
    {
        return DENOFIZ_STRING( "Vulkan" );
    }
#endif
#ifdef BUILD_DX12
    if ( DenOfIz_GraphicsApi_IsDX12Preferred( &m_apiPreference ) )
    {
        return DENOFIZ_STRING( "DirectX12" );
    }
#endif
#ifdef BUILD_METAL
    if ( DenOfIz_GraphicsApi_IsMetalPreferred( &m_apiPreference ) )
    {
        return DENOFIZ_STRING( "Metal" );
    }
#endif
    return DENOFIZ_STRING( "Unknown" );
}

void GraphicsApi::ReportLiveObjects( )
{
#ifdef BUILD_DX12
#ifdef _WIN32
    IDXGIDebug1 *dxgiDebug = nullptr;
    if ( SUCCEEDED( DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &dxgiDebug ) ) ) )
    {
        dxgiDebug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL );
        dxgiDebug->Release( );
    }
#endif
#endif
}

#define GRAPHICS_API_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::GraphicsApi, handle )

extern "C"
{

    DenOfIz_GraphicsApi DenOfIz_GraphicsApi_Create( const DenOfIz_APIPreference *preference )
    {
        if ( preference == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        GraphicsApi *graphicsApi = new GraphicsApi( *preference );
        return DENOFIZ_TO_HANDLE( graphicsApi );
    }

    DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( graphicsApi ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        DenOfIz_LogicalDeviceDesc        defaultDesc = { };
        const DenOfIz_LogicalDeviceDesc *actualDesc  = desc != NULL ? desc : &defaultDesc;

        ILogicalDevice *device = GRAPHICS_API_IMPL( graphicsApi )->CreateLogicalDevice( *actualDesc );
        return DENOFIZ_TO_HANDLE( device );
    }

    void DenOfIz_GraphicsApi_LogDeviceCapabilities( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_PhysicalDevice *gpuDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( graphicsApi ) || gpuDesc == NULL )
        {
            return;
        }
        GRAPHICS_API_IMPL( graphicsApi )->LogDeviceCapabilities( *gpuDesc );
    }

    DenOfIz_LogicalDevice DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( DenOfIz_GraphicsApi graphicsApi, const DenOfIz_LogicalDeviceDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( graphicsApi ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        DenOfIz_LogicalDeviceDesc        defaultDesc = { };
        const DenOfIz_LogicalDeviceDesc *actualDesc  = desc != NULL ? desc : &defaultDesc;

        ILogicalDevice *device = GRAPHICS_API_IMPL( graphicsApi )->CreateAndLoadOptimalLogicalDevice( *actualDesc );
        return DENOFIZ_TO_HANDLE( device );
    }

    void DenOfIz_GraphicsApi_ActiveAPI( DenOfIz_GraphicsApi graphicsApi, DenOfIz_StringView *outActiveAPI )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( graphicsApi ) || outActiveAPI == NULL )
        {
            return;
        }
        *outActiveAPI = GRAPHICS_API_IMPL( graphicsApi )->ActiveAPI( );
    }

    void DenOfIz_GraphicsApi_ReportLiveObjects( )
    {
        GraphicsApi::ReportLiveObjects( );
    }

    void DenOfIz_GraphicsApi_Destroy( DenOfIz_GraphicsApi graphicsApi )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( graphicsApi ) )
        {
            return;
        }
        delete GRAPHICS_API_IMPL( graphicsApi );
    }
}
