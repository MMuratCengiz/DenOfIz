/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Buffer.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ITopLevelAS.h"

namespace DenOfIz
{
    class DX12TopLevelAS final : public ITopLevelAS
    {
        DX12Context                                        *m_context;
        std::unique_ptr<DX12Buffer>                         m_instanceBuffer;
        std::unique_ptr<DX12Buffer>                         m_buffer;
        std::unique_ptr<DX12Buffer>                         m_scratch;
        D3D12_CPU_DESCRIPTOR_HANDLE                         m_asSrvHandle{ };
        std::vector<D3D12_RAYTRACING_INSTANCE_DESC>         m_instanceDescs;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;

    public:
        DX12TopLevelAS( DX12Context *context, const DenOfIz_TopLevelASDesc &desc );
        ~DX12TopLevelAS( ) override = default;
        [[nodiscard]] D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags( ) const;
        [[nodiscard]] size_t                                              NumInstances( ) const;
        [[nodiscard]] const DX12Buffer                                   *InstanceBuffer( ) const;
        [[nodiscard]] const DX12Buffer                                   *GetDX12Buffer( ) const;
        [[nodiscard]] DX12Buffer                                         *Buffer( ) const;
        [[nodiscard]] const DX12Buffer                                   *Scratch( ) const;
        void                                                              UpdateInstanceTransforms( const DenOfIz_UpdateTransformsDesc &desc ) override;
        size_t                                                            BuildNumBytes( ) const override;
    };
} // namespace DenOfIz
