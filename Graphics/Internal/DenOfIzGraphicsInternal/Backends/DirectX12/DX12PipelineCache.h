/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IPipelineCache.h>
#include "DX12Context.h"

namespace DenOfIz
{
    class DX12PipelineCache final : public IPipelineCache
    {
        DX12Context                        *m_context         = nullptr;
        wil::com_ptr<ID3D12PipelineLibrary> m_pipelineLibrary = nullptr;

    public:
        DX12PipelineCache( DX12Context *context, const DenOfIz_PipelineCacheDesc &desc );
        ~DX12PipelineCache( ) override;

        size_t GetDataNumBytes( ) override;
        bool   GetData( DenOfIz_ByteArray &data ) override;

        ID3D12PipelineLibrary *GetLibrary( ) const;
    };

} // namespace DenOfIz
