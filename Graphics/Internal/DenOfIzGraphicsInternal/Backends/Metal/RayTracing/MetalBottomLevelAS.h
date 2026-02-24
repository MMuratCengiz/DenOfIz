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

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IBottomLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"

namespace DenOfIz
{
    class MetalBottomLevelAS : public IBottomLevelAS
    {
    private:
        MetalContext                                                 *m_context;
        DenOfIz_BottomLevelASDesc                                     m_desc;
        id<MTLBuffer>                                                 m_scratch;
        id<MTLAccelerationStructure>                                  m_accelerationStructure;
        MTLPrimitiveAccelerationStructureDescriptor                  *m_descriptor;
        NSMutableArray<MTLAccelerationStructureGeometryDescriptor *> *m_geometryDescriptors;
        MTLAccelerationStructureInstanceOptions                       m_options;
        std::vector<id<MTLResource>>                                  m_indirectResources;
        DenOfIz_HitGroupType                                           m_hitGroupType;

    public:
        MetalBottomLevelAS( MetalContext *context, const DenOfIz_BottomLevelASDesc &desc );
        ~MetalBottomLevelAS( ) override = default;

        [[nodiscard]] id<MTLAccelerationStructure>            AccelerationStructure( ) const;
        [[nodiscard]] id<MTLBuffer>                           Scratch( ) const;
        [[nodiscard]] MTLAccelerationStructureDescriptor     *Descriptor( );
        [[nodiscard]] MTLAccelerationStructureInstanceOptions Options( ) const;
        [[nodiscard]] const DenOfIz_HitGroupType              &GeometryType( ) const;
        [[nodiscard]] const std::vector<id<MTLResource>>     &IndirectResources( ) const;
        size_t                                                BuildNumBytes( ) const override;

    private:
        MTLAccelerationStructureTriangleGeometryDescriptor    *InitializeTriangles( const DenOfIz_ASGeometryDesc &geometry );
        MTLAccelerationStructureBoundingBoxGeometryDescriptor *InitializeAABBs( const DenOfIz_ASGeometryDesc &geometry );
    };
} // namespace DenOfIz
