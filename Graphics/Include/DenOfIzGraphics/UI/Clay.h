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

#pragma once

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <functional>
#include <memory>
#include "ClayRenderer.h"

namespace DenOfIz
{
    struct DZ_API ClayWrapperDesc
    {
        ILogicalDevice *LogicalDevice                  = nullptr;
        TextRenderer   *TextRenderer                   = nullptr;
        Format          RenderTargetFormat             = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames                      = 3;
        uint32_t        MaxNumQuads                    = 2048;
        uint32_t        MaxNumMaterials                = 128;
        uint32_t        Width                          = 1024;
        uint32_t        Height                         = 1024;
        uint32_t        MaxNumElements                 = 8192;
        uint32_t        MaxNumTextMeasureCacheElements = 16384; // Maybe remove
    };

    class Clay
    {
        std::unique_ptr<ClayRenderer> m_renderer;

    public:
        explicit Clay( const ClayWrapperDesc &desc );
        ~Clay( );

        void SetLayoutDimensions( float width, float height ) const;
        void SetPointerState( Float_2 position, ClayPointerState state ) const;
        void UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime ) const;

        void BeginLayout( ) const;
        void EndLayout( ICommandList *commandList, uint32_t frameIndex ) const;

        void OpenElement( const ClayElementDeclaration &declaration ) const;
        void CloseElement( ) const;

        void Text( const InteropString &text, const ClayTextDesc &desc ) const;

        uint32_t HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;

        bool            PointerOver( uint32_t id ) const;
        ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;

        bool IsInitialized( ) const
        {
            return m_initialized;
        }

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        bool                  m_initialized;

        friend class ClayRenderer;
        friend class UIManager;
    };
} // namespace DenOfIz
