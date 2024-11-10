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
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderLocalDataLayout.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderLocalData.h>

namespace DenOfIz
{
    class DX12ShaderLocalData final : public IShaderLocalData
    {
        DX12Context            *m_context;
        ShaderLocalDataDesc        m_desc;
        DX12ShaderLocalDataLayout *m_layout;
        std::vector<Byte>       m_data;

    public:
        DX12ShaderLocalData( DX12Context *context, const ShaderLocalDataDesc &desc );
        void Begin( ) override;
        void Cbv( uint32_t binding, const IBufferResource *bufferResource ) override;
        void Cbv( uint32_t binding, const InteropArray<Byte> &data ) override;
        void Srv( uint32_t binding, const IBufferResource *bufferResource ) override;
        void Srv( uint32_t binding, const ITextureResource *textureResource ) override;
        void Uav( uint32_t binding, const IBufferResource *bufferResource ) override;
        void Uav( uint32_t binding, const ITextureResource *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] uint32_t    DataNumBytes( ) const;
        [[nodiscard]] const Byte *Data( ) const;

    private:
        void EncodeTexture( uint32_t index, const ITextureResource *texture );
        void EncodeBuffer( uint32_t index, const IBufferResource *buffer );
        void EncodeVA( uint32_t index, ID3D12Resource *resource );
    };
} // namespace DenOfIz
