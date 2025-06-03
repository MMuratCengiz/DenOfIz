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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ITopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"

namespace DenOfIz
{
    class MetalTopLevelAS : public ITopLevelAS
    {
    private:
        MetalContext                                     *m_context;
        TopLevelASDesc                                    m_desc;
        id<MTLAccelerationStructure>                      m_accelerationStructure;
        MTLInstanceAccelerationStructureDescriptor       *m_descriptor;
        id<MTLBuffer>                                     m_headerBuffer;
        id<MTLBuffer>                                     m_buffer;
        id<MTLBuffer>                                     m_instanceBuffer;
        id<MTLBuffer>                                     m_scratch;
        MTLAccelerationStructureUserIDInstanceDescriptor *m_instanceDescriptors;
        std::vector<uint32_t>                             m_contributionsToHitGroupIndices;
        std::vector<id<MTLResource>>                      m_indirectResources;
        NSMutableArray                                   *m_blasList;

    public:
        MetalTopLevelAS( MetalContext *context, const TopLevelASDesc &desc );
        void UpdateInstanceTransforms( const UpdateTransformsDesc &desc ) override;
        ~MetalTopLevelAS( ) override = default;

        [[nodiscard]] id<MTLAccelerationStructure>                      AccelerationStructure( ) const;
        [[nodiscard]] id<MTLBuffer>                                     HeaderBuffer( ) const;
        size_t                                                          NumInstances( ) const;
        [[nodiscard]] id<MTLBuffer>                                     InstanceBuffer( ) const;
        [[nodiscard]] id<MTLBuffer>                                     Scratch( ) const;
        [[nodiscard]] MTLAccelerationStructureDescriptor               *Descriptor( );
        const std::vector<id<MTLResource>>                             &IndirectResources( ) const;
        [[nodiscard]] MTLAccelerationStructureUserIDInstanceDescriptor *InstanceDescriptors( );
    };
} // namespace DenOfIz
