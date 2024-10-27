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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IBottomLevelAS.h>

namespace DenOfIz
{
    class DX12BottomLevelAS final : public IBottomLevelAS
    {
        DX12Context                                        *m_context;
        std::unique_ptr<DX12BufferResource>                 m_scratch;
        std::unique_ptr<DX12BufferResource>                 m_asBuffer;
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>         m_geometryDescs;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;

    public:
                                                                          DX12BottomLevelAS( DX12Context *context, const BottomLevelASDesc &desc );
        ~                                                                 DX12BottomLevelAS( ) override = default;
        [[nodiscard]] D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags( ) const;
        [[nodiscard]] const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>  &GeometryDescs( ) const;
        [[nodiscard]] IBufferResource                                    *Buffer( ) const override;
        [[nodiscard]] const DX12BufferResource                           *Scratch( ) const;
    private:
        void InitializeTriangles( const ASGeometryTriangleDesc &triangle, D3D12_RAYTRACING_GEOMETRY_DESC& dx12Geometry ) const;
        void InitializeAABBs( const ASGeometryAABBDesc &aabb, D3D12_RAYTRACING_GEOMETRY_DESC& dx12Geometry ) const;
    };
} // namespace DenOfIz
