/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/QueryPool.h"
#include "../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{
    class IQueryPool : public IDeviceObject
    {
    public:
        virtual ~IQueryPool( ) = default;

        virtual DenOfIz_QueryType GetType( ) const                    = 0;
        virtual uint32_t          GetNumQueries( ) const              = 0;
        virtual double            GetTimestampFrequency( )            = 0;
        virtual DenOfIz_QueryData GetQueryData( uint32_t queryIndex ) = 0;
    };
} // namespace DenOfIz
