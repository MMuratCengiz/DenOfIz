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
