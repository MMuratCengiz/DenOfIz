/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ITopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalContext.h"

namespace DenOfIz
{
    class MetalTopLevelAS : public ITopLevelAS
    {
    private:
        MetalContext                                     *m_context;
        DenOfIz_TopLevelASDesc                                    m_desc;
        id<MTLAccelerationStructure>                      m_accelerationStructure;
        MTLInstanceAccelerationStructureDescriptor       *m_descriptor;
        id<MTLBuffer>                                     m_headerBuffer;
        id<MTLBuffer>                                     m_instanceBuffer;
        id<MTLBuffer>                                     m_scratch;
        MTLAccelerationStructureUserIDInstanceDescriptor *m_instanceDescriptors;
        std::vector<uint32_t>                             m_contributionsToHitGroupIndices;
        std::vector<id<MTLResource>>                      m_indirectResources;
        NSMutableArray                                   *m_blasList;

    public:
        MetalTopLevelAS( MetalContext *context, const DenOfIz_TopLevelASDesc &desc );
        void UpdateInstanceTransforms( const DenOfIz_UpdateTransformsDesc &desc ) override;
        ~MetalTopLevelAS( ) override = default;

        [[nodiscard]] id<MTLAccelerationStructure>                      AccelerationStructure( ) const;
        [[nodiscard]] id<MTLBuffer>                                     HeaderBuffer( ) const;
        size_t                                                          NumInstances( ) const;
        [[nodiscard]] id<MTLBuffer>                                     InstanceBuffer( ) const;
        [[nodiscard]] id<MTLBuffer>                                     Scratch( ) const;
        [[nodiscard]] MTLAccelerationStructureDescriptor               *Descriptor( );
        const std::vector<id<MTLResource>>                             &IndirectResources( ) const;
        [[nodiscard]] MTLAccelerationStructureUserIDInstanceDescriptor *InstanceDescriptors( );
        size_t                                                          BuildNumBytes( ) const override;
    };
} // namespace DenOfIz
