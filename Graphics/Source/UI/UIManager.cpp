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

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/UI/ClayRenderer.h>
#include <DenOfIzGraphics/UI/ClayWrapper.h>
#include <DenOfIzGraphics/UI/UIManager.h>

using namespace DenOfIz;

struct UIManager::Impl
{
    std::unique_ptr<TextRenderer> TextRenderer;
    std::unique_ptr<ClayWrapper>  ClayWrapper;
    std::unique_ptr<ClayRenderer> ClayRenderer;
    bool                          InFrame = false;
};

UIManager::UIManager( const UIManagerDesc &desc ) : m_desc( desc ), m_impl( std::make_unique<Impl>( ) )
{
    TextRendererDesc textRendererDesc{ };
    textRendererDesc.LogicalDevice = m_desc.LogicalDevice;
    m_impl->TextRenderer           = std::make_unique<TextRenderer>( textRendererDesc );
    m_impl->TextRenderer->Initialize( );

    ClayWrapperDesc clayWrapperDesc{ };
    clayWrapperDesc.Width = desc.Width;
    clayWrapperDesc.Height = desc.Height;

    m_impl->ClayWrapper = std::make_unique<ClayWrapper>( clayWrapperDesc );
    ClayRendererDesc clayRendererDesc{ };
    clayRendererDesc.LogicalDevice      = m_desc.LogicalDevice;
    clayRendererDesc.TextRenderer       = m_impl->TextRenderer.get( );
    clayRendererDesc.RenderTargetFormat = m_desc.RenderTargetFormat;
    clayRendererDesc.NumFrames          = m_desc.NumFrames;
    clayRendererDesc.MaxNumQuads        = m_desc.MaxNumQuads;
    clayRendererDesc.MaxNumMaterials    = m_desc.MaxNumMaterials;
    clayRendererDesc.Width  = m_desc.Width;
    clayRendererDesc.Height = m_desc.Height;

    m_impl->ClayRenderer = std::make_unique<ClayRenderer>( clayRendererDesc );
    m_impl->ClayWrapper->SetMeasureTextFunction( [ this ]( const InteropString &text, const ClayTextDesc &config ) -> ClayDimensions
                                                 { return m_impl->ClayRenderer->MeasureText( text, config ); } );
}

UIManager::~UIManager( )
{
    m_impl->ClayRenderer.reset( );
    m_impl->ClayWrapper.reset( );
    m_impl->TextRenderer.reset( );
}

void UIManager::BeginFrame( const float width, const float height ) const
{
    if ( m_impl->InFrame )
    {
        LOG( WARNING ) << "BeginFrame called while already in frame";
        return;
    }

    m_impl->ClayWrapper->SetLayoutDimensions( width, height );
    m_impl->ClayWrapper->BeginLayout( );
    m_impl->InFrame = true;
}

void UIManager::EndFrame( ) const
{
    if ( !m_impl->InFrame )
    {
        LOG( WARNING ) << "EndFrame called without BeginFrame";
        return;
    }

    m_impl->InFrame = false;
}

void UIManager::OpenElement( const ClayElementDeclaration &declaration ) const
{
    m_impl->ClayWrapper->OpenElement( declaration );
}

void UIManager::CloseElement( ) const
{
    m_impl->ClayWrapper->CloseElement( );
}

void UIManager::Text( const InteropString &text, const ClayTextDesc &desc ) const
{
    m_impl->ClayWrapper->Text( text, desc );
}

void UIManager::SetPointerState( const Float_2 position, const ClayPointerState state ) const
{
    m_impl->ClayWrapper->SetPointerState( position, state );
}

void UIManager::UpdateScrollContainers( const bool enableDragScrolling, const Float_2 scrollDelta, const float deltaTime ) const
{
    m_impl->ClayWrapper->UpdateScrollContainers( enableDragScrolling, scrollDelta, deltaTime );
}

bool UIManager::PointerOver( const uint32_t id ) const
{
    return m_impl->ClayWrapper->PointerOver( id );
}

ClayBoundingBox UIManager::GetElementBoundingBox( const uint32_t id ) const
{
    return m_impl->ClayWrapper->GetElementBoundingBox( id );
}

uint32_t UIManager::HashString( const InteropString &str, const uint32_t index, const uint32_t baseId ) const
{
    return m_impl->ClayWrapper->HashString( str, index, baseId );
}

void UIManager::Render( ICommandList *commandList, const uint32_t frameIndex ) const
{
    const auto renderCommands = m_impl->ClayWrapper->EndLayout( );
    m_impl->ClayRenderer->Render( commandList, renderCommands, frameIndex );
}

void UIManager::SetViewportSize( const float width, const float height ) const
{
    m_impl->ClayWrapper->SetLayoutDimensions( width, height );
    m_impl->ClayRenderer->SetViewportSize( width, height );
}