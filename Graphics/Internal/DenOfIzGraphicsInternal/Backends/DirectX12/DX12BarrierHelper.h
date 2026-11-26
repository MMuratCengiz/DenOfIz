/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DX12Context.h"
#include "DenOfIzGraphics/Backends/Interface/PipelineBarrierDesc.h"

namespace DenOfIz
{
    class DX12BarrierHelper
    {
    public:
        static void ExecuteResourceBarrier( const DX12Context *context, ID3D12GraphicsCommandList7 *commandList, const DenOfIz_QueueType &queueType,
                                            const DenOfIz_PipelineBarrierDesc *barrier );

    private:
        static void ExecuteEnhancedResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const DenOfIz_QueueType &queueType, const DenOfIz_PipelineBarrierDesc *barrier );
        static void ExecuteLegacyResourceBarrier( ID3D12GraphicsCommandList7 *commandList, const DenOfIz_PipelineBarrierDesc *barrier );
        static bool NeedsGlobalUavSync( const DenOfIz_PipelineBarrierDesc *barrier );
    };
} // namespace DenOfIz
