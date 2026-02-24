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
