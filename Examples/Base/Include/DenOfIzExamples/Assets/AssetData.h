/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "../Interop.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Data/Geometry.h"
#include "MaterialData.h"

namespace DenOfIz
{
    struct AssetDataDesc
    {
        DenOfIz_LogicalDevice     Device{ };
        DenOfIz_BatchResourceCopy BatchCopy{ };
        DenOfIz_GeometryData      GeometryData{ };
    };

    class AssetData
    {
        DenOfIz_Buffer m_vertexBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_indexBuffer  = DENOFIZ_NULL_HANDLE;
        MaterialData  *m_materialData = nullptr;
        uint32_t       m_numVertices  = 0;
        uint32_t       m_numIndices   = 0;

    public:
        DZ_EXAMPLES_API explicit AssetData( const AssetDataDesc &desc );
        DZ_EXAMPLES_API ~AssetData( );
        DZ_EXAMPLES_API void                         UpdateMaterialData( MaterialData *materialData );
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Buffer VertexBuffer( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Buffer IndexBuffer( ) const;
        [[nodiscard]] DZ_EXAMPLES_API MaterialData  *Material( ) const;
        [[nodiscard]] DZ_EXAMPLES_API uint32_t       NumVertices( ) const;
        [[nodiscard]] DZ_EXAMPLES_API uint32_t       NumIndices( ) const;
    };
} // namespace DenOfIz
