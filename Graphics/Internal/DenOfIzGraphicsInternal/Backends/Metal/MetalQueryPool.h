/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/IQueryPool.h"
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalQueryPool final : public IQueryPool
    {
        MetalContext              *m_context;
        DenOfIz_QueryPoolDesc      m_desc;
        id<MTLCounterSampleBuffer> m_counterSampleBuffer;
        id<MTLBuffer>              m_visibilityResultBuffer;

    public:
        MetalQueryPool( MetalContext *context, const DenOfIz_QueryPoolDesc &desc );
        ~MetalQueryPool( ) override;

        DenOfIz_QueryType          GetType( ) const override;
        uint32_t                   GetNumQueries( ) const override;
        DenOfIz_QueryData          GetQueryData( uint32_t queryIndex ) override;
        double                     GetTimestampFrequency( ) override;
        id<MTLCounterSampleBuffer> GetCounterSampleBuffer( ) const;
        id<MTLBuffer>              GetVisibilityResultBuffer( ) const;
        void                       WriteTimestamp( uint32_t queryIndex );
    };
} // namespace DenOfIz
