/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class IBuffer : public IDeviceObject
    {
    public:
        virtual ~IBuffer( ) = default;

        // Allowed only on CPU visible resources
        virtual void *MapMemory( )   = 0;
        virtual void  UnmapMemory( ) = 0;

        [[nodiscard]] virtual size_t      NumBytes( ) const = 0;
        [[nodiscard]] virtual const void *Data( ) const     = 0;

        [[nodiscard]] virtual DenOfIz_ByteArray GetData( ) const                                              = 0;
        virtual void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) = 0;
        // Always maps memory, the caller aligns bufferOffset if needed.
        virtual void WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) = 0;
    };
} // namespace DenOfIz
