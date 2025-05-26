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

#include <DenOfIzGraphics/Assets/Font/Font.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "TextLayout.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace DenOfIz
{
    struct DZ_API TextBatchDesc
    {
        IRootSignature *RendererRootSignature;
        ILogicalDevice *LogicalDevice;
        Font           *Font;
    };

    struct DZ_API AddTextDesc
    {
        uint16_t      FontSize = 32;
        InteropString Text;
        float         X                = 0.0f;
        float         Y                = 0.0f;
        Float_4       Color            = Float_4{ 1.0f, 1.0f, 1.0f, 1.0f };
        uint16_t      LetterSpacing    = 0;
        uint16_t      LineHeight       = 0;
        bool          HorizontalCenter = false;
        bool          VerticalCenter   = false;
        TextDirection Direction        = TextDirection::Auto;
    };

    // We create a separate batch for each font used.
    class TextBatch
    {
        struct FontShaderUniforms
        {
            XMFLOAT4X4 Projection;
            XMFLOAT4   TextColor;
            XMFLOAT4   TextureSizeParams; // xy: texture dimensions, z: pixel range, w: unused
        };

        TextBatchDesc   m_desc;
        ILogicalDevice *m_logicalDevice;
        Font           *m_font;

        uint32_t                  m_maxVertices        = 4096;
        uint32_t                  m_maxIndices         = 4096;
        uint32_t                  m_currentVertexCount = 0;
        uint32_t                  m_currentIndexCount  = 0;
        InteropArray<GlyphVertex> m_glyphVertices;
        InteropArray<uint32_t>    m_indexData;

        BufferDesc m_vertexBufferDesc{ };
        BufferDesc m_indexBufferDesc{ };

        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup = nullptr;
        std::unique_ptr<ITextureResource>   m_atlas;
        std::unique_ptr<IBufferResource>    m_vertexBuffer;
        Byte                               *m_vertexBufferMappedMemory;
        std::unique_ptr<IBufferResource>    m_indexBuffer;
        Byte                               *m_indexBufferMappedMemory;

        std::unique_ptr<IBufferResource> m_uniformBuffer     = nullptr;
        FontShaderUniforms              *m_uniformBufferData = nullptr;

        std::vector<std::unique_ptr<TextLayout>> m_textLayouts;
        uint32_t                                 m_currentTextLayoutIndex = 0;
        XMFLOAT4X4                               m_projectionMatrix{ };

        mutable std::vector<std::unique_ptr<TextLayout>> m_measureTextLayouts;
        mutable uint32_t                                 m_currentMeasureLayoutIndex = 0;

        ResourceTracking          m_resourceTracking;
        std::unique_ptr<ISampler> m_fontSampler = nullptr;

    public:
        DZ_API explicit TextBatch( const TextBatchDesc &desc );
        DZ_API ~TextBatch( );

        DZ_API void BeginBatch( );
        DZ_API void AddText( const AddTextDesc &desc );
        DZ_API void EndBatch( ICommandList *commandList );

        DZ_API void    SetProjectionMatrix( const Float_4x4 &projectionMatrix );
        DZ_API Float_2 MeasureText( const InteropString &text, const AddTextDesc &desc ) const;

    private:
        void UpdateBuffers( );
        void InitializeAtlas( );
    };
} // namespace DenOfIz
