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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderRecordLayout.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderRecordData.h>

namespace DenOfIz
{
    class DX12ShaderRecordData : public IShaderRecordData
    {
    private:
        DX12Context            *m_context;
        ShaderRecordDataDesc    m_desc;
        DX12ShaderRecordLayout *m_layout;
        std::vector<Byte>       m_data;

    public:
        DX12ShaderRecordData( DX12Context *context, const ShaderRecordDataDesc &desc );
        void Begin( ) override;
        void Cbv( uint32_t index, const IBufferResource *bufferResource ) override;
        void Cbv( uint32_t index, const InteropArray<Byte> &data ) override;
        void Srv( uint32_t index, const IBufferResource *textureResource ) override;
        void Srv( uint32_t index, const ITextureResource *textureResource ) override;
        void Srv( uint32_t index, const InteropArray<Byte> &data ) override;
        void Uav( uint32_t index, const IBufferResource *textureResource ) override;
        void Uav( uint32_t index, const ITextureResource *textureResource ) override;
        void Sampler( uint32_t index, const ISampler *sampler ) override;
        void End( ) override;
    };
} // namespace DenOfIz
