/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Buffer.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IBottomLevelAS.h"

namespace DenOfIz
{
    class DX12BottomLevelAS final : public IBottomLevelAS
    {
        DX12Context                                        *m_context;
        std::unique_ptr<DX12Buffer>                         m_scratch;
        std::unique_ptr<DX12Buffer>                         m_asBuffer;
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>         m_geometryDescs;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;

    public:
        DX12BottomLevelAS( DX12Context *context, const DenOfIz_BottomLevelASDesc &desc );
        ~DX12BottomLevelAS( ) override = default;
        [[nodiscard]] D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags( ) const;
        [[nodiscard]] const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>  &GeometryDescs( ) const;
        [[nodiscard]] DX12Buffer                                         *Buffer( ) const;
        [[nodiscard]] const DX12Buffer                                   *Scratch( ) const;
        size_t                                                            BuildNumBytes( ) const override;

    private:
        void InitializeTriangles( const DenOfIz_ASGeometryTriangleDesc &triangle, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const;
        void InitializeAABBs( const DenOfIz_ASGeometryAABBDesc &aabb, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const;
    };
} // namespace DenOfIz
