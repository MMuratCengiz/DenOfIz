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

#include <string>
#include <unordered_map>
#include <vector>

#include "../../../../Graphics/Internal/DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"

namespace DenOfIz
{
    constexpr uint32_t INVALID_TEXTURE_HANDLE = UINT32_MAX;

    struct BistroVertex
    {
        DenOfIz_Float4 Position;
        DenOfIz_Float4 Normal;
        DenOfIz_Float2 UV;
        DenOfIz_Float4 Tangent;
        DenOfIz_Float4 Bitangent;
    };

    struct BistroDrawData
    {
        uint32_t VertexOffset;
        uint32_t IndexOffset;
        uint32_t NumIndices;
        uint32_t NumVertices;
    };

    struct BistroObject
    {
        uint32_t         ObjectIndex;
        DenOfIz_Float4x4 Transform;
    };

    struct BistroMaterialData
    {
        uint32_t AlbedoTextureHandle;
        uint32_t NormalTextureHandle;
        uint32_t MetallicRoughnessTextureHandle;
        uint32_t EmissiveTextureHandle;
        uint32_t OcclusionTextureHandle;
        uint32_t Padding[ 3 ];
    };

    struct BistroMaterial
    {
        std::string MaterialRef;
        uint32_t    AlbedoTextureHandle;
        uint32_t    NormalTextureHandle;
        uint32_t    MetallicRoughnessTextureHandle;
        uint32_t    EmissiveTextureHandle;
        uint32_t    OcclusionTextureHandle;
    };

    struct BistroMeshInfo
    {
        uint32_t IndexOffsetBytes;
        uint32_t UVAttributeOffsetBytes;
        uint32_t NormalAttributeOffsetBytes;
        uint32_t TangentAttributeOffsetBytes;
        uint32_t BitangentAttributeOffsetBytes;
        uint32_t PositionAttributeOffsetBytes;
        uint32_t AttributeStrideBytes;
        uint32_t MaterialInstanceId;
    };

    struct BistroData // Convenience
    {
        BistroDrawData DrawData;
        BistroObject   Object;
        std::string    MaterialRef;
        uint32_t       MaterialIndex;
        DenOfIz_Float3 MinBounds;
        DenOfIz_Float3 MaxBounds;
    };

    class Bistro
    {
        std::string bistroDir       = std::string( DZ_EXAMPLES_BISTRO_DIR );
        std::string m_bistroRawPath = bistroDir + "/bistro.gltf";

        DenOfIz_Buffer m_materialBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_vertexBuffer   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_indexBuffer    = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_meshInfoBuffer = DENOFIZ_NULL_HANDLE;

        std::vector<BistroData>                   m_bistroData;
        std::vector<BistroMaterial>               m_bistroMaterials;
        std::vector<DenOfIz_Texture>              m_textureHandles;
        std::unordered_map<std::string, uint32_t> m_texturePathToHandle;

    public:
        explicit Bistro( DenOfIz_LogicalDevice logicalDevice );
        ~Bistro( );

        [[nodiscard]] DenOfIz_Buffer                 GetMaterialBuffer( ) const;
        [[nodiscard]] DenOfIz_Buffer                 GetVertexBuffer( ) const;
        [[nodiscard]] DenOfIz_Buffer                 GetIndexBuffer( ) const;
        [[nodiscard]] DenOfIz_Buffer                 GetMeshInfoBuffer( ) const;
        [[nodiscard]] const std::vector<BistroData> &GetBistroData( ) const;
        [[nodiscard]] size_t                         GetNumSubmeshes( ) const;
        [[nodiscard]] size_t                         GetNumMaterials( ) const;
        [[nodiscard]] DenOfIz_TextureArray           GetTextures( );
    };
} // namespace DenOfIz
