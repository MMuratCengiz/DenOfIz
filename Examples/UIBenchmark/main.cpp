/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/UI/UI.h"
#include "DenOfIzGraphics/Utilities/Engine.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace
{
    std::atomic<uint64_t> Allocations{ 0 };
}

void *operator new( const size_t size )
{
    Allocations.fetch_add( 1, std::memory_order_relaxed );
    if ( void *p = std::malloc( size > 0 ? size : 1 ) )
    {
        return p;
    }
    throw std::bad_alloc( );
}

void *operator new[]( const size_t size )
{
    return operator new( size );
}

void operator delete( void *p ) noexcept
{
    std::free( p );
}

void operator delete[]( void *p ) noexcept
{
    std::free( p );
}

void operator delete( void *p, size_t ) noexcept
{
    std::free( p );
}

void operator delete[]( void *p, size_t ) noexcept
{
    std::free( p );
}

namespace
{
    using Clock = std::chrono::steady_clock;

    constexpr uint32_t Width       = 2560;
    constexpr uint32_t Height      = 1600;
    constexpr float    Scale       = 2.0f;
    constexpr uint32_t NumFrames   = 3;
    constexpr uint16_t InterFont   = 1;
    constexpr uint32_t NumCards    = 12;
    constexpr uint32_t NumItems    = 80;
    constexpr uint32_t NumLogLines = 60;
    constexpr uint32_t Captured[]  = { 0, 1, 50, 240, 260, 280 };

    constexpr const char *Paragraph = "Text is shaped once and laid out again only when its width changes. Every glyph is drawn from its outline, "
                                      "so it stays sharp at any size, and the curves are compiled the first time a glyph is drawn. Long words like "
                                      "internationalisation and Supercalifragilisticexpialidocious wrap where the layout allows it.";
    constexpr const char *Scripts[] = {
        "English · العربية · עברית · हिन्दी · ไทย · 日本語 · 한국어 · Ελληνικά · Русский",
        "中文 · 日本語の文章 · 한국어 문장 · ภาษาไทย · বাংলা · தமிழ் · ქართული",
        "قرأت 3 كتب في Book World اليوم · הוא אמר (hello world) ויצא",
    };
    constexpr const char *Emoji   = "😀 👍🏽 🇳🇱 👨‍👩‍👧 🦄 🌈 🔥 ✅ 🚀 ☕";
    constexpr float       Sizes[] = { 9, 11, 13, 16, 24 };

    DenOfIz_UIColor Rgb( const uint32_t rgb, const uint8_t alpha = 255 )
    {
        return DenOfIz_UIColor{ static_cast<uint8_t>( rgb >> 16 ), static_cast<uint8_t>( rgb >> 8 ), static_cast<uint8_t>( rgb ), alpha };
    }

    DenOfIz_StringView View( const char *text )
    {
        return DenOfIz_StringView{ text, strlen( text ) };
    }

    struct Phase
    {
        std::vector<double> Samples;

        void Add( const double ms )
        {
            Samples.push_back( ms );
        }
        double Median( )
        {
            std::sort( Samples.begin( ), Samples.end( ) );
            return Samples.empty( ) ? 0.0 : Samples[ Samples.size( ) / 2 ];
        }
        double Mean( ) const
        {
            double sum = 0.0;
            for ( const double s : Samples )
            {
                sum += s;
            }
            return Samples.empty( ) ? 0.0 : sum / static_cast<double>( Samples.size( ) );
        }
    };

    double MsBetween( const Clock::time_point a, const Clock::time_point b )
    {
        return std::chrono::duration<double, std::milli>( b - a ).count( );
    }

    class Benchmark
    {
        DenOfIz_GraphicsApi      m_api         = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice    m_device      = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking m_tracking    = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandQueue     m_queue       = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandListPool  m_pool        = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandList      m_commandList = DENOFIZ_NULL_HANDLE;
        DenOfIz_Fence            m_fence       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture          m_target      = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer           m_readback    = DENOFIZ_NULL_HANDLE;
        uint32_t                 m_rowPitch    = 0;
        DenOfIz_Font             m_inter       = DENOFIZ_NULL_HANDLE;
        DenOfIz_UI               m_ui          = DENOFIZ_NULL_HANDLE;
        DenOfIz_TextRenderer     m_text        = DENOFIZ_NULL_HANDLE;
        uint32_t                 m_sidebarId   = 0;
        uint32_t                 m_mainId      = 0;
        uint32_t                 m_logId       = 0;
        std::vector<std::string> m_strings;
        char                     m_format[ 128 ] = { };

    public:
        bool Init( )
        {
            DenOfIz_APIPreference preference{ };
            preference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12;
            preference.Linux   = DENOFIZ_API_PREFERENCE_LINUX_VULKAN;
            preference.OSX     = DENOFIZ_API_PREFERENCE_OSX_METAL;
            m_api              = DenOfIz_GraphicsApi_Create( &preference );
            DenOfIz_LogicalDeviceDesc deviceDesc{ };
            m_device = DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( m_api, &deviceDesc );
            if ( !DENOFIZ_HANDLE_IS_VALID( m_device ) )
            {
                return false;
            }
            DenOfIz_ResourceTracking_Create( &m_tracking );

            DenOfIz_CommandQueueDesc queueDesc{ };
            queueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
            DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &queueDesc, &m_queue );
            DenOfIz_CommandListPoolDesc poolDesc{ };
            poolDesc.CommandQueue    = m_queue;
            poolDesc.NumCommandLists = 1;
            DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &poolDesc, &m_pool );
            DenOfIz_CommandListArray lists{ };
            DenOfIz_CommandListPool_GetCommandLists( m_pool, &lists );
            m_commandList = lists.Elements[ 0 ];
            DenOfIz_LogicalDevice_CreateFence( m_device, &m_fence );

            DenOfIz_TextureDesc targetDesc{ };
            targetDesc.Width     = Width;
            targetDesc.Height    = Height;
            targetDesc.Depth     = 1;
            targetDesc.ArraySize = 1;
            targetDesc.MipLevels = 1;
            targetDesc.Format    = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
            targetDesc.Usage     = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT | DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT;
            targetDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
            targetDesc.DebugName = DENOFIZ_STRING( "UI Benchmark Target" );
            DenOfIz_LogicalDevice_CreateTexture( m_device, &targetDesc, &m_target );
            DenOfIz_ResourceTracking_TrackTexture( m_tracking, m_target, DENOFIZ_QUEUE_TYPE_GRAPHICS );

            m_rowPitch = ( Width * 4 + 255 ) & ~255u;
            DenOfIz_BufferDesc readbackDesc{ };
            readbackDesc.NumBytes  = static_cast<size_t>( m_rowPitch ) * Height;
            readbackDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
            readbackDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU_CPU;
            readbackDesc.DebugName = DENOFIZ_STRING( "UI Benchmark Readback" );
            DenOfIz_LogicalDevice_CreateBuffer( m_device, &readbackDesc, &m_readback );
            DenOfIz_ResourceTracking_TrackBuffer( m_tracking, m_readback, DENOFIZ_QUEUE_TYPE_GRAPHICS );

            DenOfIz_UIDesc uiDesc{ };
            uiDesc.LogicalDevice      = m_device;
            uiDesc.ResourceTracking   = m_tracking;
            uiDesc.RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
            uiDesc.NumFrames          = NumFrames;
            uiDesc.Width              = Width;
            uiDesc.Height             = Height;
            m_ui                      = DenOfIz_UI_Create( &uiDesc );
            DenOfIz_UI_SetDisplayScale( m_ui, Scale );
            m_inter = DenOfIz_Font_CreateFromPath( DENOFIZ_STRING( "Assets/Fonts/Inter-Regular.ttf" ), 0 );
            DenOfIz_UI_AddFont( m_ui, InterFont, m_inter );

            DenOfIz_TextRendererDesc textDesc{ };
            textDesc.LogicalDevice = m_device;
            textDesc.Width         = Width;
            textDesc.Height        = Height;
            textDesc.NumFrames     = NumFrames;
            m_text                 = DenOfIz_TextRenderer_Create( &textDesc );
            DenOfIz_TextRenderer_AddFont( m_text, m_inter, InterFont );

            m_sidebarId = DenOfIz_UI_HashId( DENOFIZ_STRING( "Sidebar" ), 0 );
            m_mainId    = DenOfIz_UI_HashId( DENOFIZ_STRING( "Main" ), 0 );
            m_logId     = DenOfIz_UI_HashId( DENOFIZ_STRING( "Log" ), 0 );
            for ( uint32_t i = 0; i < NumItems; ++i )
            {
                m_strings.push_back( "Item " + std::to_string( i ) + " · " +
                                     std::string( "settings,inbox,archive,drafts,projects,reports,people,billing" ).substr( i % 7 * 7, 12 ) );
            }
            for ( uint32_t i = 0; i < NumLogLines; ++i )
            {
                char line[ 160 ];
                std::snprintf( line, sizeof( line ), "[00:%02u:%02u.%03u] INFO renderer: frame %u took %.2f ms, %u draws, %u glyphs, %u KiB uploaded", i / 60, i % 60,
                               i * 37 % 1000, 1000 + i * 17, 3.0 + i % 9 * 0.37, 20 + i % 13, 4000 + i * 91, i * 13 % 512 );
                m_strings.emplace_back( line );
            }
            return true;
        }

        void Shutdown( )
        {
            DenOfIz_CommandQueue_WaitIdle( m_queue );
            DenOfIz_UI_Destroy( m_ui );
            DenOfIz_TextRenderer_Destroy( m_text );
            DenOfIz_Font_Destroy( m_inter );
            DenOfIz_ResourceTracking_UntrackBuffer( m_tracking, m_readback );
            DenOfIz_ResourceTracking_UntrackTexture( m_tracking, m_target );
            DenOfIz_Buffer_Destroy( m_readback );
            DenOfIz_Texture_Destroy( m_target );
            DenOfIz_Fence_Destroy( m_fence );
            DenOfIz_CommandListPool_Destroy( m_pool );
            DenOfIz_CommandQueue_Destroy( m_queue );
            DenOfIz_ResourceTracking_Destroy( m_tracking );
            DenOfIz_LogicalDevice_Destroy( m_device );
            DenOfIz_GraphicsApi_Destroy( m_api );
        }

        void Run( const char *name, const uint32_t frames, const bool animated )
        {
            Phase    declare, endFrame, render, gpu, total;
            uint64_t buildAllocations = 0, renderAllocations = 0;
            for ( uint32_t frame = 0; frame < frames + 60; ++frame )
            {
                const bool measured = frame >= 60;
                const auto t0       = Clock::now( );
                const auto a0       = Allocations.load( );
                BuildFrame( animated ? frame : 0, animated );
                const auto t1 = Clock::now( );
                DenOfIz_UI_EndFrame( m_ui );
                const auto t2 = Clock::now( );
                const auto a1 = Allocations.load( );
                BeginPass( );
                const auto a2 = Allocations.load( );
                const auto t3 = Clock::now( );
                DenOfIz_UI_Render( m_ui, frame % NumFrames, m_commandList );
                const auto t4 = Clock::now( );
                const auto a3 = Allocations.load( );
                Submit( false );
                const auto t5 = Clock::now( );
                if ( measured )
                {
                    declare.Add( MsBetween( t0, t1 ) );
                    endFrame.Add( MsBetween( t1, t2 ) );
                    render.Add( MsBetween( t3, t4 ) );
                    gpu.Add( MsBetween( t4, t5 ) );
                    total.Add( MsBetween( t0, t4 ) - MsBetween( t2, t3 ) );
                    buildAllocations += a1 - a0;
                    renderAllocations += a3 - a2;
                }
            }
            const double n = frames;
            std::printf( "%-9s declare %6.3f  end frame %6.3f  render %6.3f  cpu total %6.3f  submit+gpu %6.3f ms (median)  allocations/frame build %.1f render %.1f\n", name,
                         declare.Median( ), endFrame.Median( ), render.Median( ), total.Median( ), gpu.Median( ), buildAllocations / n, renderAllocations / n );
            std::printf( "%-9s declare %6.3f  end frame %6.3f  render %6.3f  cpu total %6.3f  submit+gpu %6.3f ms (mean)\n", name, declare.Mean( ), endFrame.Mean( ),
                         render.Mean( ), total.Mean( ), gpu.Mean( ) );
        }

        int Capture( const std::string &dir, const bool compare )
        {
            int mismatches = 0;
            for ( uint32_t frame = 0; frame <= Captured[ std::size( Captured ) - 1 ]; ++frame )
            {
                DenOfIz_UI_SetDisplayScale( m_ui, frame < 250 ? Scale : frame < 270 ? 1.0f : 1.5f );
                BuildFrame( frame, true );
                DenOfIz_UI_EndFrame( m_ui );
                const bool captured = std::find( std::begin( Captured ), std::end( Captured ), frame ) != std::end( Captured );
                BeginPass( );
                DenOfIz_UI_Render( m_ui, frame % NumFrames, m_commandList );
                Submit( captured );
                if ( captured )
                {
                    mismatches += WriteOrCompare( dir + "/frame" + std::to_string( frame ), compare );
                }
            }
            BeginPass( );
            DrawText( );
            Submit( true );
            mismatches += WriteOrCompare( dir + "/text", compare );
            return mismatches;
        }

        void RunTextRenderer( const uint32_t frames )
        {
            Phase    cpu, gpu;
            uint64_t allocations = 0;
            for ( uint32_t frame = 0; frame < frames + 60; ++frame )
            {
                BeginPass( );
                const auto t0 = Clock::now( );
                const auto a0 = Allocations.load( );
                DrawText( );
                const auto t1 = Clock::now( );
                const auto a1 = Allocations.load( );
                Submit( false );
                const auto t2 = Clock::now( );
                if ( frame >= 60 )
                {
                    cpu.Add( MsBetween( t0, t1 ) );
                    gpu.Add( MsBetween( t1, t2 ) );
                    allocations += a1 - a0;
                }
            }
            std::printf( "text      cpu %6.3f  submit+gpu %6.3f ms (median)  allocations/frame %.1f\n", cpu.Median( ), gpu.Median( ), static_cast<double>( allocations ) / frames );
        }

    private:
        int WriteOrCompare( const std::string &name, const bool compare ) const
        {
            const std::vector<uint8_t> rgba = ReadPixels( );
            const std::string          path = name + ".png";
            if ( !compare )
            {
                stbi_write_png( path.c_str( ), Width, Height, 4, rgba.data( ), Width * 4 );
                std::printf( "wrote %s\n", path.c_str( ) );
                return 0;
            }
            int      w = 0, h = 0, c = 0;
            uint8_t *reference = stbi_load( path.c_str( ), &w, &h, &c, 4 );
            if ( reference == nullptr || w != static_cast<int>( Width ) || h != static_cast<int>( Height ) )
            {
                std::printf( "%s: missing or wrong size\n", path.c_str( ) );
                stbi_image_free( reference );
                return 1;
            }
            uint32_t maxDiff = 0, pixels = 0;
            size_t   worst = 0;
            for ( size_t i = 0; i < rgba.size( ); i += 4 )
            {
                uint32_t diff = 0;
                for ( size_t k = 0; k < 4; ++k )
                {
                    diff = std::max<uint32_t>( diff, std::abs( rgba[ i + k ] - reference[ i + k ] ) );
                }
                worst   = diff > maxDiff ? i / 4 : worst;
                maxDiff = std::max( maxDiff, diff );
                pixels += diff > 0 ? 1 : 0;
            }
            stbi_image_free( reference );
            std::printf( "%s: max channel difference %u at %zu,%zu, %u pixels differ\n", path.c_str( ), maxDiff, worst % Width, worst / Width, pixels );
            if ( maxDiff > 0 )
            {
                stbi_write_png( ( name + "-new.png" ).c_str( ), Width, Height, 4, rgba.data( ), Width * 4 );
            }
            return maxDiff > 0 ? 1 : 0;
        }

        void DrawText( )
        {
            DenOfIz_TextRenderer_BeginBatch( m_text );
            DenOfIz_TextRenderDesc desc{ };
            desc.FontSize = 20.0f;
            desc.Color    = DenOfIz_Float4{ 1.0f, 1.0f, 1.0f, 1.0f };
            desc.X        = 16.0f;
            for ( uint32_t i = 0; i < NumLogLines; ++i )
            {
                const std::string &line = m_strings[ NumItems + i ];
                desc.Text               = DenOfIz_StringView{ line.c_str( ), line.size( ) };
                desc.Y                  = 8.0f + static_cast<float>( i ) * 26.0f;
                DenOfIz_TextRenderer_AddText( m_text, &desc );
            }
            desc.FontId = InterFont;
            desc.X      = 1320.0f;
            for ( uint32_t i = 0; i < 40; ++i )
            {
                const std::string &item = m_strings[ i ];
                desc.FontSize           = 12.0f + static_cast<float>( i % 4 ) * 4.0f;
                desc.Text               = DenOfIz_StringView{ item.c_str( ), item.size( ) };
                desc.Y                  = 8.0f + static_cast<float>( i ) * 26.0f;
                DenOfIz_TextRenderer_AddText( m_text, &desc );
            }
            desc.X = 1700.0f;
            for ( uint32_t i = 0; i < 12; ++i )
            {
                desc.FontSize = 18.0f + static_cast<float>( i % 3 ) * 10.0f;
                desc.Text     = View( i % 4 == 3 ? Emoji : Scripts[ i % std::size( Scripts ) ] );
                desc.Y        = 8.0f + static_cast<float>( i ) * 60.0f;
                DenOfIz_TextRenderer_AddText( m_text, &desc );
            }
            DenOfIz_TextRenderer_EndBatch( m_text, m_commandList );
        }

        DenOfIz_StringView Format( const char *format, ... )
        {
            va_list args;
            va_start( args, format );
            const int length = std::vsnprintf( m_format, sizeof( m_format ), format, args );
            va_end( args );
            return DenOfIz_StringView{ m_format, static_cast<size_t>( std::clamp( length, 0, static_cast<int>( sizeof( m_format ) ) - 1 ) ) };
        }

        void Label( const DenOfIz_StringView text, const float size, const DenOfIz_UIColor color, const uint16_t font = InterFont,
                    const DenOfIz_UITextWrap wrap = DENOFIZ_UI_TEXT_WRAP_NONE )
        {
            DenOfIz_UITextDesc desc{ };
            desc.FontId   = font;
            desc.FontSize = size;
            desc.Color    = color;
            desc.Wrap     = wrap;
            DenOfIz_UI_Text( m_ui, text, &desc );
        }

        static DenOfIz_UIElementDesc Box( const DenOfIz_UIDirection direction, const float gap, const float padding )
        {
            DenOfIz_UIElementDesc desc{ };
            desc.Layout.Direction = direction;
            desc.Layout.Gap       = DenOfIz_UILength_Points( gap );
            desc.Layout.Padding   = DenOfIz_UILengthEdges_All( padding );
            return desc;
        }

        static DenOfIz_UIElementDesc Scroller( const uint32_t id, const float gap, const float padding )
        {
            DenOfIz_UIElementDesc desc = Box( DENOFIZ_UI_DIRECTION_COLUMN, gap, padding );
            desc.Id                    = id;
            desc.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
            desc.Layout.Shrink         = 1.0f;
            return desc;
        }

        void BuildFrame( const uint32_t frame, const bool animated )
        {
            if ( animated )
            {
                DenOfIz_UI_SetScrollOffset( m_ui, m_sidebarId, DenOfIz_Float2{ 0.0f, static_cast<float>( frame * 3 % 900 ) } );
                DenOfIz_UI_SetScrollOffset( m_ui, m_mainId, DenOfIz_Float2{ 0.0f, static_cast<float>( frame * 2 % 700 ) } );
                DenOfIz_UI_SetScrollOffset( m_ui, m_logId, DenOfIz_Float2{ 0.0f, static_cast<float>( frame % 500 ) } );
            }
            DenOfIz_UI_BeginFrame( m_ui, 1.0f / 60.0f );

            DenOfIz_UIElementDesc root = Box( DENOFIZ_UI_DIRECTION_COLUMN, 0, 0 );
            root.Layout.Width          = DenOfIz_UIValue_Percent( 1.0f );
            root.Layout.Height         = DenOfIz_UIValue_Percent( 1.0f );
            root.Style.BackgroundColor = Rgb( 0xF4F5F7 );
            DenOfIz_UI_OpenElement( m_ui, &root );
            TopBar( frame );
            DenOfIz_UIElementDesc body = Box( DENOFIZ_UI_DIRECTION_ROW, 0, 0 );
            body.Layout.Grow           = 1.0f;
            body.Layout.Shrink         = 1.0f;
            DenOfIz_UI_OpenElement( m_ui, &body );
            Sidebar( frame );
            DenOfIz_UIElementDesc content = Box( DENOFIZ_UI_DIRECTION_COLUMN, 0, 0 );
            content.Layout.Grow           = 1.0f;
            content.Layout.Shrink         = 1.0f;
            DenOfIz_UI_OpenElement( m_ui, &content );
            Cards( frame );
            Log( );
            DenOfIz_UI_CloseElement( m_ui );
            DenOfIz_UI_CloseElement( m_ui );
            DenOfIz_UI_CloseElement( m_ui );
        }

        void TopBar( const uint32_t frame )
        {
            DenOfIz_UIElementDesc bar = Box( DENOFIZ_UI_DIRECTION_ROW, 10, 12 );
            bar.Layout.AlignItems     = DENOFIZ_UI_ALIGN_CENTER;
            bar.Style.BackgroundColor = Rgb( 0x1F2430 );
            DenOfIz_UI_OpenElement( m_ui, &bar );
            Label( View( "Benchmark" ), 24, Rgb( 0xFFFFFF ) );
            const char *buttons[] = { "File", "Edit", "View", "Selection", "Window", "Help" };
            for ( const char *button : buttons )
            {
                DenOfIz_UIElementDesc desc = Box( DENOFIZ_UI_DIRECTION_ROW, 0, 6 );
                desc.Style.BackgroundColor = Rgb( 0x2E3545 );
                desc.Style.BorderColor     = Rgb( 0x4A5468 );
                desc.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
                desc.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
                DenOfIz_UI_OpenElement( m_ui, &desc );
                Label( View( button ), 13, Rgb( 0xE8ECF4 ) );
                DenOfIz_UI_CloseElement( m_ui );
            }
            Label( Format( "%u fps", 60 + frame % 7 ), 13, Rgb( 0x9AE6B4 ) );
            DenOfIz_UI_CloseElement( m_ui );
        }

        void Sidebar( const uint32_t frame )
        {
            DenOfIz_UIElementDesc side = Scroller( m_sidebarId, 2, 8 );
            side.Layout.Width          = DenOfIz_UIValue_Points( 220 );
            side.Style.BackgroundColor = Rgb( 0xFFFFFF );
            side.Style.BorderColor     = Rgb( 0xDDE1E8 );
            side.Style.BorderWidth     = DenOfIz_UIEdges{ 0, 0, 1, 0 };
            DenOfIz_UI_OpenElement( m_ui, &side );
            for ( uint32_t i = 0; i < NumItems; ++i )
            {
                DenOfIz_UIElementDesc item = Box( DENOFIZ_UI_DIRECTION_ROW, 8, 6 );
                item.Layout.AlignItems     = DENOFIZ_UI_ALIGN_CENTER;
                item.Style.CornerRadius    = DenOfIz_UICorners_All( 4 );
                item.Style.BackgroundColor = i % 9 == frame % 9 ? Rgb( 0xE6EEFF ) : Rgb( 0xFFFFFF, 0 );
                DenOfIz_UI_OpenElement( m_ui, &item );
                DenOfIz_UIElementDesc swatch = { };
                swatch.Layout.Width          = DenOfIz_UIValue_Points( 10 );
                swatch.Layout.Height         = DenOfIz_UIValue_Points( 10 );
                swatch.Style.CornerRadius    = DenOfIz_UICorners_All( 5 );
                swatch.Style.BackgroundColor = Rgb( 0x3366CC ^ ( i * 0x13579B & 0x3F3F3F ) );
                DenOfIz_UI_OpenElement( m_ui, &swatch );
                DenOfIz_UI_CloseElement( m_ui );
                const std::string &name = m_strings[ i ];
                Label( DenOfIz_StringView{ name.c_str( ), name.size( ) }, 13, Rgb( 0x2A2F3A ) );
                DenOfIz_UIElementDesc spacer{ };
                spacer.Layout.Grow = 1.0f;
                DenOfIz_UI_OpenElement( m_ui, &spacer );
                DenOfIz_UI_CloseElement( m_ui );
                Label( Format( "%u", ( i * 7 + ( i % 5 == 0 ? frame : 0 ) ) % 1000 ), 11, Rgb( 0x6B7385 ) );
                DenOfIz_UI_CloseElement( m_ui );
            }
            DenOfIz_UI_CloseElement( m_ui );
        }

        void Cards( const uint32_t frame )
        {
            DenOfIz_UIElementDesc main = Scroller( m_mainId, 0, 16 );
            main.Layout.Grow           = 1.0f;
            DenOfIz_UI_OpenElement( m_ui, &main );
            Label( View( "Overview" ), 48, Rgb( 0x1F2430 ) );
            DenOfIz_UIElementDesc grid = Box( DENOFIZ_UI_DIRECTION_ROW, 12, 0 );
            grid.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
            grid.Layout.LineGap        = DenOfIz_UILength_Points( 12 );
            DenOfIz_UI_OpenElement( m_ui, &grid );
            for ( uint32_t i = 0; i < NumCards; ++i )
            {
                DenOfIz_UIElementDesc card = Box( DENOFIZ_UI_DIRECTION_COLUMN, 8, 14 );
                card.Layout.Basis          = DenOfIz_UIValue_Points( 300 );
                card.Layout.Grow           = 1.0f;
                card.Style.BackgroundColor = Rgb( 0xFFFFFF );
                card.Style.BorderColor     = Rgb( 0xDDE1E8 );
                card.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
                card.Style.CornerRadius    = DenOfIz_UICorners_All( 10 );
                card.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
                card.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_CLIP;
                DenOfIz_UI_OpenElement( m_ui, &card );
                Label( Format( "Card %u", i + 1 ), 16 + static_cast<float>( i % 3 ) * 4, Rgb( 0x1F2430 ) );
                Label( View( Paragraph ), Sizes[ i % std::size( Sizes ) ], Rgb( 0x4A5161 ), InterFont, DENOFIZ_UI_TEXT_WRAP_WORDS );

                DenOfIz_UIElementDesc stats = Box( DENOFIZ_UI_DIRECTION_ROW, 16, 0 );
                DenOfIz_UI_OpenElement( m_ui, &stats );
                Label( Format( "%u,%03u", ( i * 37 + frame ) % 100, ( i * 211 + frame * 7 ) % 1000 ), 20, Rgb( 0x2251CC ) );
                Label( Format( "%.1f%%", static_cast<float>( ( i * 13 + frame ) % 1000 ) / 10.0f ), 20, Rgb( 0x1C8A4A ) );
                Label( View( "requests" ), 11, Rgb( 0x8A91A0 ) );
                DenOfIz_UI_CloseElement( m_ui );

                DenOfIz_UIElementDesc track = { };
                track.Layout.Height         = DenOfIz_UIValue_Points( 6 );
                track.Style.BackgroundColor = Rgb( 0xE9ECF2 );
                track.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
                DenOfIz_UI_OpenElement( m_ui, &track );
                DenOfIz_UIElementDesc fill = { };
                fill.Layout.Width          = DenOfIz_UIValue_Percent( static_cast<float>( ( i * 17 + frame ) % 100 ) / 100.0f );
                fill.Style.BackgroundColor = Rgb( 0x3B82F6 );
                fill.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
                DenOfIz_UI_OpenElement( m_ui, &fill );
                DenOfIz_UI_CloseElement( m_ui );
                DenOfIz_UI_CloseElement( m_ui );

                Label( View( Scripts[ i % std::size( Scripts ) ] ), 14, Rgb( 0x2A2F3A ), InterFont, DENOFIZ_UI_TEXT_WRAP_WORDS );
                Label( View( Emoji ), 16 + static_cast<float>( i % 4 ) * 8, Rgb( 0x000000 ), InterFont, DENOFIZ_UI_TEXT_WRAP_WORDS );
                DenOfIz_UI_CloseElement( m_ui );
            }
            DenOfIz_UI_CloseElement( m_ui );
            Label( View( "Display" ), 96, Rgb( 0x1F2430 ) );
            DenOfIz_UI_CloseElement( m_ui );
        }

        void Log( )
        {
            DenOfIz_UIElementDesc log = Scroller( m_logId, 1, 10 );
            log.Layout.Height         = DenOfIz_UIValue_Points( 220 );
            log.Style.BackgroundColor = Rgb( 0x161A22 );
            DenOfIz_UI_OpenElement( m_ui, &log );
            for ( uint32_t i = 0; i < NumLogLines; ++i )
            {
                const std::string &line = m_strings[ NumItems + i ];
                Label( DenOfIz_StringView{ line.c_str( ), line.size( ) }, 11, Rgb( i % 11 == 0 ? 0xF6AD55 : 0xC8D0DC ), 0 );
            }
            DenOfIz_UI_CloseElement( m_ui );
        }

        void BeginPass( )
        {
            DenOfIz_CommandList_Begin( m_commandList );
            DenOfIz_ResourceTracking_TransitionTexture( m_tracking, m_commandList, m_target, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
            DenOfIz_RenderingAttachmentDesc attachment{ };
            attachment.Resource   = m_target;
            attachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
            attachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
            attachment.ClearColor = DenOfIz_Float4{ 0.0f, 0.0f, 0.0f, 1.0f };
            DenOfIz_RenderingDesc rendering{ };
            rendering.RTAttachments.Elements    = &attachment;
            rendering.RTAttachments.NumElements = 1;
            rendering.RenderAreaWidth           = static_cast<float>( Width );
            rendering.RenderAreaHeight          = static_cast<float>( Height );
            rendering.NumLayers                 = 1;
            DenOfIz_CommandList_BeginRendering( m_commandList, &rendering );
            DenOfIz_CommandList_BindViewport( m_commandList, 0.0f, 0.0f, static_cast<float>( Width ), static_cast<float>( Height ) );
            DenOfIz_CommandList_BindScissorRect( m_commandList, 0.0f, 0.0f, static_cast<float>( Width ), static_cast<float>( Height ) );
        }

        void Submit( const bool readback )
        {
            DenOfIz_CommandList_EndRendering( m_commandList );
            if ( readback )
            {
                DenOfIz_ResourceTracking_TransitionTexture( m_tracking, m_commandList, m_target, DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
                DenOfIz_ResourceTracking_TransitionBuffer( m_tracking, m_commandList, m_readback, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
                DenOfIz_CopyTextureToBufferDesc copy{ };
                copy.DstBuffer  = m_readback;
                copy.SrcTexture = m_target;
                copy.Format     = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
                copy.RowPitch   = m_rowPitch;
                copy.NumRows    = Height;
                DenOfIz_CommandList_CopyTextureToBuffer( m_commandList, &copy );
            }
            DenOfIz_CommandList_End( m_commandList );
            DenOfIz_ExecuteCommandListsDesc execute{ };
            execute.CommandLists.Elements    = &m_commandList;
            execute.CommandLists.NumElements = 1;
            execute.Signal                   = m_fence;
            DenOfIz_CommandQueue_ExecuteCommandLists( m_queue, &execute );
            DenOfIz_Fence_Wait( m_fence );
        }

        std::vector<uint8_t> ReadPixels( ) const
        {
            void *mapped = nullptr;
            DenOfIz_Buffer_MapMemory( m_readback, &mapped );
            std::vector<uint8_t> rgba( static_cast<size_t>( Width ) * Height * 4 );
            for ( uint32_t y = 0; y < Height; ++y )
            {
                const uint8_t *row = static_cast<const uint8_t *>( mapped ) + static_cast<size_t>( y ) * m_rowPitch;
                uint8_t       *dst = rgba.data( ) + static_cast<size_t>( y ) * Width * 4;
                for ( uint32_t x = 0; x < Width; ++x )
                {
                    dst[ x * 4 + 0 ] = row[ x * 4 + 2 ];
                    dst[ x * 4 + 1 ] = row[ x * 4 + 1 ];
                    dst[ x * 4 + 2 ] = row[ x * 4 + 0 ];
                    dst[ x * 4 + 3 ] = row[ x * 4 + 3 ];
                }
            }
            DenOfIz_Buffer_UnmapMemory( m_readback );
            return rgba;
        }
    };
} // namespace

int main( const int argc, char **argv )
{
    uint32_t    frames = 600;
    std::string captureDir;
    bool        compare = false;
    for ( int i = 1; i + 1 < argc; i += 2 )
    {
        const std::string arg = argv[ i ];
        if ( arg == "--frames" )
        {
            frames = static_cast<uint32_t>( std::strtoul( argv[ i + 1 ], nullptr, 10 ) );
        }
        else if ( arg == "--capture" || arg == "--compare" )
        {
            captureDir = argv[ i + 1 ];
            compare    = arg == "--compare";
        }
    }

    DenOfIz_EngineDesc engineDesc{ };
    engineDesc.LogLevel = DENOFIZ_LOG_LEVEL_WARNING;
    DenOfIz_Engine_Init( &engineDesc );
    int       result = 0;
    Benchmark benchmark;
    if ( !benchmark.Init( ) )
    {
        DenOfIz_Engine_Shutdown( );
        return 1;
    }
    if ( !captureDir.empty( ) )
    {
        result = benchmark.Capture( captureDir, compare );
    }
    else
    {
        benchmark.Run( "static", frames, false );
        benchmark.Run( "animated", frames, true );
        benchmark.RunTextRenderer( frames );
    }
    benchmark.Shutdown( );
    DenOfIz_Engine_Shutdown( );
    return result;
}
