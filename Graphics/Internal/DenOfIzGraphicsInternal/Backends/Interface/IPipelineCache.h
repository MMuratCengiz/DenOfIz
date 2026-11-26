/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/PipelineCache.h"

namespace DenOfIz
{
    class IPipelineCache
    {
    public:
        virtual ~IPipelineCache( ) = default;

        virtual size_t GetDataNumBytes( )                 = 0;
        virtual bool   GetData( DenOfIz_ByteArray &data ) = 0;
    };

} // namespace DenOfIz
