/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IPipelineCache.h>
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalPipelineCache final : public IPipelineCache
    {
        MetalContext        *m_context       = nullptr;
        id<MTLBinaryArchive> m_binaryArchive = nil;

    public:
        MetalPipelineCache( MetalContext *context, const DenOfIz_PipelineCacheDesc &desc );
        ~MetalPipelineCache( ) override;

        size_t GetDataNumBytes( ) override;
        bool   GetData( DenOfIz_ByteArray &data ) override;

        id<MTLBinaryArchive> GetArchive( ) const;
    };

} // namespace DenOfIz
