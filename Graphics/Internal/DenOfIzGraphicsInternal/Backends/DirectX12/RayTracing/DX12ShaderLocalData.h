/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12LocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderLocalData.h"

namespace DenOfIz
{
    class DX12ShaderLocalData final : public IShaderLocalData
    {
        DX12Context                *m_context;
        DenOfIz_ShaderLocalDataDesc m_desc;
        DX12LocalRootSignature     *m_layout;
        std::vector<Byte>           m_data;

    public:
        DX12ShaderLocalData( DX12Context *context, const DenOfIz_ShaderLocalDataDesc &desc );
        void Begin( ) override;
        void Cbv( uint32_t binding, IBuffer *bufferResource ) override;
        void Cbv( uint32_t binding, const DenOfIz_ByteArrayView &data ) override;
        void Srv( uint32_t binding, const IBuffer *bufferResource ) override;
        void Srv( uint32_t binding, const ITexture *textureResource ) override;
        void Uav( uint32_t binding, const IBuffer *bufferResource ) override;
        void Uav( uint32_t binding, const ITexture *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] uint32_t    DataNumBytes( ) const;
        [[nodiscard]] const Byte *Data( ) const;

    private:
        void EncodeTexture( uint32_t index, const ITexture *texture );
        void EncodeBuffer( uint32_t index, const IBuffer *buffer );
        void EncodeVA( uint32_t index, ID3D12Resource *resource );
    };
} // namespace DenOfIz
