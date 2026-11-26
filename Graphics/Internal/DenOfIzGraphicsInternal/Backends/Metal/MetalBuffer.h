/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <string>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalBuffer final : public IBuffer
    {
    private:
        MetalContext      *m_context{ };
        DenOfIz_BufferDesc m_desc;
        std::string        m_debugName;
        id<MTLBuffer>      m_buffer{ };
        MTLResourceUsage   m_usage{ };
        MTLDataType        m_dataType{ };
        uint32_t           m_numBytes     = 0;
        void              *m_mappedMemory = nullptr;

    public:
        MetalBuffer( MetalContext *context, const DenOfIz_BufferDesc &desc );
        ~MetalBuffer( ) override;
        const id<MTLBuffer>    &Instance( ) const;
        const MTLResourceUsage &Usage( ) const;
        const MTLDataType      &Type( ) const;
        void                   *MapMemory( ) override;
        void                    UnmapMemory( ) override;

        [[nodiscard]] size_t      NumBytes( ) const override;
        [[nodiscard]] const void *Data( ) const override;

        [[nodiscard]] DenOfIz_ByteArray GetData( ) const override;
        void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) override;
        void                            WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) override;
    };

} // namespace DenOfIz
