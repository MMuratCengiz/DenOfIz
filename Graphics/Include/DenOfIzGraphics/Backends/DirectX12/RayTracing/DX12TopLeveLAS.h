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
#include <DenOfIzGraphics/Backends/Interface/RayTracing/ITopLevelAS.h>

namespace DenOfIz
{
    class DX12TopLevelAS : public ITopLevelAS
    {
    private:
        DX12Context                                        *m_context;
        std::unique_ptr<DX12BufferResource>                 m_instanceBuffer;
        std::unique_ptr<DX12BufferResource>                 m_buffer;
        std::unique_ptr<DX12BufferResource>                 m_scratch;
        D3D12_CPU_DESCRIPTOR_HANDLE                         m_asSrvHandle;
        std::vector<D3D12_RAYTRACING_INSTANCE_DESC>         m_instanceDescs;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_flags;

    public:
        DX12TopLevelAS( DX12Context *context, const TopLevelASDesc &desc );
        ~DX12TopLevelAS( ) override = default;
        [[nodiscard]] D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags( ) const;
        [[nodiscard]] const size_t                                        NumInstances( ) const;
        [[nodiscard]] const DX12BufferResource                           *InstanceBuffer( ) const;
        [[nodiscard]] const DX12BufferResource                           *DX12Buffer( ) const;
        [[nodiscard]] IBufferResource                                    *Buffer( ) const override;
        [[nodiscard]] const DX12BufferResource                           *Scratch( ) const;
        void                                                              Update( const TopLevelASDesc &desc ) override;
    };
} // namespace DenOfIz
