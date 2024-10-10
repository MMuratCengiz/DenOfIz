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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include "MaterialData.h"

namespace DenOfIz
{
    /// <summary> AssetData class is a container class for asset information, reduces clutter. </summary>
    struct AssetDataDesc
    {
        ILogicalDevice    *Device;
        BatchResourceCopy *BatchCopy;
        GeometryData       GeometryData;
    };

    class AssetData
    {
    private:
        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        MaterialData                    *m_materialData;
        uint32_t                         m_numVertices = 0;
        uint32_t                         m_numIndices  = 0;

    public:
        AssetData( AssetDataDesc &desc );
        void             UpdateMaterialData( MaterialData *materialData );
        IBufferResource *VertexBuffer( ) const;
        IBufferResource *IndexBuffer( ) const;
        MaterialData    *MaterialData( ) const;
        uint32_t         NumVertices( ) const;
        uint32_t         NumIndices( ) const;
    };
} // namespace DenOfIz
